/**
 * Lazy MCP loading for OpenVB's Pi harness.
 *
 * Philosophy: a large capability set must not mean a large permanent prompt.
 *
 * - `math` MCP tools stay permanently declared (exposure "direct" in .pi/mcp.json).
 * - Every other MCP server is configured with exposure "deferred" in .pi/mcp.json,
 *   so its tools are registered but never declared to the model by default.
 * - This extension keeps a local, model-invisible registry of those hidden MCP
 *   tools and exposes three tiny permanent tools:
 *
 *     mcp_tools_search(query, limit?)      semantic search; REPLACES the transient set
 *     mcp_tools_enable(tools, replace?)    exact activation for deterministic workflows
 *     mcp_tools_reset(server?)             drop transient tools (one server or all)
 *
 * - Transient activations replace each other; the active MCP tool set never
 *   accumulates across a long session.
 *
 * Search is plain local lexical scoring (token overlap over tool name, server,
 * description, parameter names and descriptions). No embeddings, no LLM.
 */

import type { ExtensionAPI, ToolInfo } from "@earendil-works/pi-coding-agent";
import { Type } from "typebox";

const MCP_PREFIX = "mcp__";
/** MCP tools under this server stay permanently active and are never managed here. */
const PERMANENT_MCP_SERVERS = new Set(["math"]);
/** Defensive: never keep built-in discovery tools active (disabled via .pi/settings.json). */
const NEVER_ACTIVE = new Set(["tool_search", "codemode"]);

const DEFAULT_LIMIT = 4;
const MAX_LIMIT = 8;

interface HiddenTool {
	name: string;
	server: string;
	tool: string;
	description: string;
	paramsText: string;
	nameTokens: Set<string>;
	serverTokens: Set<string>;
	descTokens: Set<string>;
	paramTokens: Set<string>;
	nameLower: string;
	descLower: string;
}

/* ------------------------------ tokenization ------------------------------ */

const STOP_WORDS = new Set([
	"a", "an", "and", "are", "as", "at", "be", "by", "for", "from", "in",
	"is", "it", "of", "on", "or", "that", "the", "this", "to", "with",
]);

function stem(term: string): string {
	if (term.length > 4 && term.endsWith("ies")) return `${term.slice(0, -3)}y`;
	if (term.length > 4 && /(ches|shes|sses|xes|zes)$/.test(term)) return term.slice(0, -2);
	if (term.length > 3 && term.endsWith("s") && !term.endsWith("ss")) return term.slice(0, -1);
	return term;
}

function tokenize(text: string): string[] {
	return text
		.replace(/([a-z0-9])([A-Z])/g, "$1 $2")
		.replace(/([A-Z]+)([A-Z][a-z])/g, "$1 $2")
		.toLowerCase()
		.split(/[^a-z0-9]+/)
		.filter((t) => t.length > 0 && !STOP_WORDS.has(t))
		.map(stem);
}

function tokenSet(text: string): Set<string> {
	return new Set(tokenize(text));
}

/* ------------------------------ registry build ---------------------------- */

/** Collect property names and description strings from a parameter schema, recursively. */
function schemaText(schema: unknown, parts: string[]): void {
	if (typeof schema !== "object" || schema === null) return;
	const obj = schema as Record<string, unknown>;
	if (typeof obj.description === "string") parts.push(obj.description);
	if (typeof obj.title === "string") parts.push(obj.title);
	if (typeof obj.const === "string") parts.push(obj.const);
	if (Array.isArray(obj.enum)) parts.push(obj.enum.filter((v) => typeof v === "string").join(" "));
	const props = obj.properties;
	if (typeof props === "object" && props !== null) {
		for (const [name, value] of Object.entries(props as Record<string, unknown>)) {
			parts.push(name);
			schemaText(value, parts);
		}
	}
	schemaText(obj.items, parts);
	for (const key of ["anyOf", "oneOf", "allOf"]) {
		const variants = obj[key];
		if (Array.isArray(variants)) for (const variant of variants) schemaText(variant, parts);
	}
	const additional = obj.additionalProperties;
	if (typeof additional === "object" && additional !== null) schemaText(additional, parts);
}

function splitMcpName(name: string): { server: string; tool: string } | undefined {
	if (!name.startsWith(MCP_PREFIX)) return undefined;
	const rest = name.slice(MCP_PREFIX.length);
	const sep = rest.indexOf("__");
	if (sep <= 0) return undefined;
	return { server: rest.slice(0, sep), tool: rest.slice(sep + 2) };
}

function buildRegistry(pi: ExtensionAPI): HiddenTool[] {
	const tools: ToolInfo[] = pi.getAllTools();
	const registry: HiddenTool[] = [];
	for (const tool of tools) {
		const parsed = splitMcpName(tool.name);
		if (!parsed) continue;
		if (PERMANENT_MCP_SERVERS.has(parsed.server)) continue;
		const description = tool.description ?? "";
		const paramParts: string[] = [];
		schemaText(tool.parameters, paramParts);
		const paramsText = paramParts.join(" ");
		registry.push({
			name: tool.name,
			server: parsed.server,
			tool: parsed.tool,
			description,
			paramsText,
			nameTokens: tokenSet(`${parsed.tool} ${parsed.server}`),
			serverTokens: tokenSet(parsed.server),
			descTokens: tokenSet(description),
			paramTokens: tokenSet(paramsText),
			nameLower: `${parsed.server}__${parsed.tool}`.toLowerCase(),
			descLower: description.toLowerCase(),
		});
	}
	registry.sort((a, b) => a.name.localeCompare(b.name));
	return registry;
}

/* --------------------------------- scoring -------------------------------- */

function overlap(haystack: Set<string>, term: string): boolean {
	return haystack.has(term);
}

function scoreTool(entry: HiddenTool, terms: string[]): number {
	let score = 0;
	for (const term of terms) {
		if (overlap(entry.nameTokens, term)) score += 6;
		else if (entry.nameLower.includes(term)) score += 3;
		if (overlap(entry.serverTokens, term)) score += 4;
		if (overlap(entry.descTokens, term)) score += 2;
		else if (entry.descLower.includes(term)) score += 1;
		if (overlap(entry.paramTokens, term)) score += 1;
	}
	return score;
}

function searchRegistry(registry: HiddenTool[], query: string, limit: number): HiddenTool[] {
	const terms = tokenize(query);
	if (terms.length === 0) return [];
	const scored = registry
		.map((entry) => ({ entry, score: scoreTool(entry, terms) }))
		.filter((row) => row.score > 0)
		.sort((a, b) => b.score - a.score || a.entry.name.localeCompare(b.entry.name));
	return scored.slice(0, limit).map((row) => row.entry);
}

/* ------------------------------- extension -------------------------------- */

export default function lazyMcp(pi: ExtensionAPI) {
	/** MCP tools this extension currently keeps declared (transient set). */
	const transient = new Set<string>();

	const isManaged = (name: string): boolean => {
		const parsed = splitMcpName(name);
		if (!parsed) return false;
		return !PERMANENT_MCP_SERVERS.has(parsed.server);
	};

	/** Make active = (current active minus managed) + transient. Replaces, never accumulates. */
	const syncActive = (): { added: string[]; removed: string[] } => {
		const active = pi.getActiveTools();
		const kept = active.filter((name) => !isManaged(name) && !NEVER_ACTIVE.has(name));
		const additions = [...transient].filter((name) => !kept.includes(name)).sort();
		const next = [...kept, ...additions];
		const removed = active.filter((name) => !next.includes(name));
		if (removed.length > 0 || additions.length > 0) pi.setActiveTools(next);
		return { added: additions, removed };
	};

	const registryOf = (): HiddenTool[] => buildRegistry(pi);

	const describe = (entry: HiddenTool): string => {
		const firstLine = entry.description.split("\n")[0].trim();
		const short = firstLine.length > 160 ? `${firstLine.slice(0, 157)}...` : firstLine;
		return `- ${entry.name}\n  ${short}`;
	};

	const applyActivation = (names: string[], reason: string): string => {
		const previous = [...transient].sort();
		transient.clear();
		for (const name of names) transient.add(name);
		const { added, removed } = syncActive();
		const registry = registryOf();
		const lines: string[] = [];
		for (const name of names) {
			const entry = registry.find((tool) => tool.name === name);
			lines.push(entry ? describe(entry) : `- ${name}`);
		}
		const header =
			names.length === 0
				? "Activated 0 MCP tools."
				: `Activated ${names.length} MCP tool${names.length === 1 ? "" : "s"} (${reason}). Full schema${names.length === 1 ? " is" : "s are"} available on the next turn.`;
		const changes: string[] = [];
		if (removed.length > 0) changes.push(`removed: ${removed.join(", ")}`);
		if (previous.length > 0 && names.length > 0) changes.push("previous transient set replaced");
		const tail = changes.length > 0 ? `\n(${changes.join("; ")})` : "";
		return [header, ...lines].join("\n") + tail;
	};

	/* ------------------------------ mcp_tools_search ----------------------------- */

	pi.registerTool({
		name: "mcp_tools_search",
		label: "MCP tools search",
		description:
			"Describe a capability you need (e.g. 'analyze entropy of a binary region') and activate a small set of currently hidden MCP tools " +
			"(IDA, ImHex, Microsoft Learn, codegraph, devenv). The matched tools become callable on the next turn. " +
			"Each call REPLACES the previously activated MCP tools, so request everything you need in one search.",
		promptSnippet: "Activate a few hidden MCP tools by describing the capability needed",
		promptGuidelines: [
			"Use mcp_tools_search to load IDA, ImHex, Microsoft Learn, codegraph, and devenv MCP tools; only activate what the current step needs.",
		],
		parameters: Type.Object({
			query: Type.String({
				description: "Natural-language capability description, e.g. 'decompile a function' or 'search MSDN for Win32 CreateFile'.",
			}),
			limit: Type.Optional(
				Type.Number({ description: `Maximum tools to activate (default ${DEFAULT_LIMIT}, max ${MAX_LIMIT}).` }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, params, signal) {
			if (signal?.aborted) throw new Error("cancelled");
			const limit = Math.max(1, Math.min(MAX_LIMIT, params.limit ?? DEFAULT_LIMIT));
			const registry = registryOf();
			if (registry.length === 0) {
				return {
					content: [{ type: "text", text: "No hidden MCP tools are registered right now. MCP servers may still be connecting or are offline (check /mcp). IDA servers require the devenv ida-*-headless processes; ImHex requires imhex-headless." }],
					details: { activated: [] },
				};
			}
			const matches = searchRegistry(registry, params.query, limit);
			if (matches.length === 0) {
				const servers = new Map<string, number>();
				for (const entry of registry) servers.set(entry.server, (servers.get(entry.server) ?? 0) + 1);
				const listing = [...servers.entries()].map(([server, count]) => `${server} (${count})`).join(", ");
				return {
					content: [{ type: "text", text: `No hidden MCP tools matched \"${params.query}\".\nHidden servers/tools available: ${listing}.\nRetry with different wording, or use mcp_tools_enable with an exact tool name.` }],
					details: { activated: [], query: params.query },
				};
			}
			const names = matches.map((entry) => entry.name);
			const text = applyActivation(names, `search: ${params.query}`);
			return { content: [{ type: "text", text }], details: { activated: names, query: params.query } };
		},
	});

	/* ------------------------------ mcp_tools_enable ----------------------------- */

	pi.registerTool({
		name: "mcp_tools_enable",
		label: "MCP tools enable",
		description:
			"Activate exact hidden MCP tools by name, replacing the current transient set. Use full names like mcp__ida_original_f3__decompile " +
			"(see mcp_tools_search or /lazy_mcp for what exists). Pass an empty list to deactivate all transient MCP tools.",
		parameters: Type.Object({
			tools: Type.Array(Type.String(), {
				description: "Full MCP tool names (mcp__<server>__<tool>) or short tool names when each is unique.",
			}),
			replace: Type.Optional(
				Type.Boolean({ description: "Replace the current transient set (default true). If false, merge into it. Keep the total small (<= 8)." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, params, signal) {
			if (signal?.aborted) throw new Error("cancelled");
			const registry = registryOf();
			const resolve = (requested: string): string | undefined => {
				if (requested.startsWith(MCP_PREFIX)) return registry.some((e) => e.name === requested) ? requested : undefined;
				const hits = registry.filter((e) => e.tool === requested || e.name === requested);
				return hits.length >= 1 ? hits[0]!.name : undefined;
			};
			const resolved: string[] = [];
			const unknown: string[] = [];
			for (const requested of params.tools) {
				const name = resolve(requested.trim());
				if (name && !resolved.includes(name)) resolved.push(name);
				if (!name) unknown.push(requested);
			}
			if (resolved.length === 0) {
				return {
					content: [{ type: "text", text: unknown.length > 0
						? `Unknown or not-yet-registered MCP tools: ${unknown.join(", ")}. Nothing activated. Servers may still be connecting; check /mcp or /lazy_mcp.`
						: "No tools requested." }],
					details: { activated: [], unknown },
					isError: unknown.length > 0,
				};
			}
			if (resolved.length > MAX_LIMIT) {
				return {
					content: [{ type: "text", text: `Refusing to activate ${resolved.length} tools at once (budget ${MAX_LIMIT}); request a smaller set.` }],
					details: { activated: [] },
					isError: true,
				};
			}
			if (params.replace === false) {
				const merged = new Set(transient);
				for (const name of resolved) merged.add(name);
				if (merged.size > MAX_LIMIT) {
					return {
						content: [{ type: "text", text: `Merging would put ${merged.size} transient MCP tools in context (budget ${MAX_LIMIT}). Use a replacing mcp_tools_enable instead, or mcp_tools_reset first.` }],
						details: { activated: [] },
						isError: true,
					};
				}
				transient.clear();
				for (const name of merged) transient.add(name);
				syncActive();
				const text = `Merged MCP tools. Transient set now: ${[...transient].sort().join(", ") || "(empty)"}`;
				return { content: [{ type: "text", text }], details: { activated: [...transient] } };
			}
			const text = applyActivation(resolved, "explicit enable");
			const result: string[] = [text];
			if (unknown.length > 0) result.push(`Not found (skipped): ${unknown.join(", ")}`);
			return { content: [{ type: "text", text: result.join("\n") }], details: { activated: resolved, unknown } };
		},
	});

	/* ------------------------------ mcp_tools_reset ------------------------------ */

	pi.registerTool({
		name: "mcp_tools_reset",
		label: "MCP tools reset",
		description: "Deactivate lazy MCP tools: all transient ones, or only those of one server. Keeps the permanent tool set (core tools, math MCP, lazy MCP controls).",
		parameters: Type.Object({
			server: Type.Optional(Type.String({ description: "Only drop transient tools of this MCP server (e.g. ida_original_f3). Omit to drop all." })),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, params, signal) {
			if (signal?.aborted) throw new Error("cancelled");
			if (params.server) {
				const parsedPrefix = `${MCP_PREFIX}${params.server}__`;
				for (const name of [...transient]) if (name.startsWith(parsedPrefix)) transient.delete(name);
			} else {
				transient.clear();
			}
			const text = applyActivation([...transient].sort(), params.server ? `reset of ${params.server}` : "reset");
			return { content: [{ type: "text", text }], details: { activated: [...transient] } };
		},
	});

	/* ------------------------------- commands/events ------------------------------ */

	pi.registerCommand("lazy_mcp", {
		description: "Show hidden, active, and permanently active MCP tool counts per server",
		handler: async (_args, ctx) => {
			const registry = registryOf();
			const active = new Set(pi.getActiveTools());
			const byServer = new Map<string, { hidden: number; active: number }>();
			const add = (server: string, isActive: boolean) => {
				const row = byServer.get(server) ?? { hidden: 0, active: 0 };
				if (isActive) row.active += 1;
				else row.hidden += 1;
				byServer.set(server, row);
			};
			for (const entry of registry) add(entry.server, active.has(entry.name));
			for (const name of active) {
				if (!isManaged(name)) continue;
				const parsed = splitMcpName(name);
				if (parsed) add(parsed.server, true);
			}
			const lines = [...byServer.entries()]
				.sort((a, b) => a[0].localeCompare(b[0]))
				.map(([server, row]) => `${server}: ${row.active} active / ${row.hidden} hidden`);
			const permanentMcp = [...active].filter((n) => n.startsWith(MCP_PREFIX) && !isManaged(n));
			const summary = [
				`Lazy MCP: ${registry.length} hidden tools, ${transient.size} transiently active (${[...transient].join(", ") || "none"}).`,
				`Permanent MCP: ${permanentMcp.length > 0 ? permanentMcp.join(", ") : "none connected"}`,
				...lines,
			];
			ctx.ui.notify(summary.join("\n"), "info");
		},
	});

	// Adopt transient tools that are still declared when a session starts/resumes,
	// so state reconstruction never double-counts or accidentally drops them.
	pi.on("session_start", () => {
		for (const name of pi.getActiveTools()) if (isManaged(name)) transient.add(name);
		syncActive();
	});

	// Servers connect asynchronously; keep the registry view fresh (it is rebuilt on
	// demand anyway) and prune transient entries whose tools were withdrawn.
	pi.on("mcp_servers_change", () => {
		const registered = new Set(pi.getAllTools().map((tool) => tool.name));
		for (const name of [...transient]) if (!registered.has(name)) transient.delete(name);
		syncActive();
	});
}

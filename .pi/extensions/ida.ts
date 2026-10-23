/**
 * `ida` - one direct tool for BOTH headless IDA MCP servers (OpenVB harness)
 *
 * Replaces the lazy-MCP activation dance for IDA work (mcp_tools_search that
 * keeps surfacing codegraph instead of IDA, "tool not found" after set
 * replacement, transient sets). Ghidra proved the shape: ONE tool, actions as
 * parameters, target as a REQUIRED parameter.
 *
 * - target: "original" | "reimpl"  - the ORIGINAL_REFERENCE vs
 *   REIMPLEMENTATION_UNDER_TEST discipline moves from skill prose into the
 *   schema. A call cannot happen without choosing.
 * - action: health | list_tools | call | batch | save
 * - list_tools introspects the server's real tool names and argument schemas
 *   once per session - nothing here hardcodes a guessed vocabulary.
 * - call/batch run `tools/call` over the servers' streamable-HTTP MCP
 *   endpoint (port = $OPENVB_IDA_ORIG_MCP_PORT / $OPENVB_IDA_REIMPL_MCP_PORT,
 *   default 8745/8746, path /mcp?ext=dbg from .pi/mcp.json). The servers
 *   serialize requests on their own port, so batch ops are safe.
 * - save performs the standing-rule save_database + disk-mtime proof + census
 *   log line (artifacts/idb-guard-census.log), same script as the persistence
 *   guard.
 *
 * Persistence interplay: the idb-persistence-guard extension watches
 * tool_call events for this tool too, so writes made through `ida` arm the
 * 5s-idle autosave exactly like direct mcp__ida_* calls. If the guard is
 * removed, save becomes YOUR responsibility again (see batch-2/3).
 *
 * Gating: `ida` is an uncovered custom tool, so pi-verdict adjudicates each
 * call (batch = ONE adjudication for up to 25 ops - friendlier latency than
 * 25 individual MCP tool calls).
 */

import * as fs from "node:fs";
import * as path from "node:path";
import { Type } from "typebox";
import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

interface Target {
	name: string;
	portEnv: string;
	defaultPort: string;
	idbEnv: string;
	idbDefault: string;
}

const TARGETS: Record<string, Target> = {
	original: {
		name: "ida_original_f3",
		portEnv: "OPENVB_IDA_ORIG_MCP_PORT",
		defaultPort: "8745",
		idbEnv: "OPENVB_ORIG_IDB",
		idbDefault: "artifacts/F3.exe.i64",
	},
	reimpl: {
		name: "ida_reimplementation_openvb",
		portEnv: "OPENVB_IDA_REIMPL_MCP_PORT",
		defaultPort: "8746",
		idbEnv: "OPENVB_REIMPL_IDB",
		idbDefault: "build/F3.exe.i64",
	},
};

const MAX_OPS = 25;
let baseCwd = process.cwd();
const RESULT_CAP = 50_000;

const SAVE_SCRIPT = [
	"import json, ida_loader, idautils, idc",
	"_p = ida_loader.get_path(ida_loader.PATH_TYPE_IDB)",
	"_ok = bool(ida_loader.save_database(_p, 0))",
	"_all = list(idautils.Functions())",
	"_named = sum(1 for ea in _all if not idc.get_func_name(ea).startswith('sub_'))",
	"print(json.dumps({'saved': _ok, 'functions': len(_all), 'named': _named}))",
].join("\n");

// ---------------------------- streamable-HTTP MCP client ----------------------------

interface Session {
	url: string;
	sessionId: string | null;
}

const sessions = new Map<string, Session>();

function endpointOf(t: Target): string {
	return `http://127.0.0.1:${process.env[t.portEnv] || t.defaultPort}/mcp?ext=dbg`;
}

async function mcpRequest(t: Target, body: Record<string, unknown>, timeoutMs: number, retry = true): Promise<Record<string, unknown>> {
	const url = endpointOf(t);
	let sess = sessions.get(t.name);
	if (!sess) {
		sess = await mcpInitialize(t, url);
		sessions.set(t.name, sess);
	}
	const headers: Record<string, string> = {
		"content-type": "application/json",
		accept: "application/json, text/event-stream",
		...(sess.sessionId ? { "mcp-session-id": sess.sessionId } : {}),
	};
	const res = await fetch(url, {
		method: "POST",
		headers,
		body: JSON.stringify(body),
		signal: AbortSignal.timeout(timeoutMs),
	});
	if (res.status === 404 || res.status === 400) {
		if (retry) {
			sessions.delete(t.name); // stale session id: re-handshake once
			return mcpRequest(t, body, timeoutMs, false);
		}
	}
	if (!res.ok) throw new Error(`HTTP ${res.status} from ${t.name}: ${(await res.text()).slice(0, 200)}`);
	const text = await res.text();
	const dataLine = text.split("\n").filter((l) => l.startsWith("data:")).pop();
	const payload = JSON.parse((dataLine ? dataLine.slice(5) : text).trim() || "{}");
	if (payload.error) throw new Error(`rpc ${payload.error.code}: ${payload.error.message}`);
	return payload.result ?? {};
}

async function mcpInitialize(t: Target, url: string): Promise<Session> {
	const res = await fetch(url, {
		method: "POST",
		headers: { "content-type": "application/json", accept: "application/json, text/event-stream" },
		body: JSON.stringify({
			jsonrpc: "2.0",
			id: 0,
			method: "initialize",
			params: {
				protocolVersion: "2025-03-26",
				capabilities: {},
				clientInfo: { name: "ida-tool", version: "1.0.0" },
			},
		}),
		signal: AbortSignal.timeout(10_000),
	});
	if (!res.ok) throw new Error(`initialize HTTP ${res.status} for ${t.name} - is the headless server running? (devenv up / ida-orig-headless, ida-reimpl-headless)`);
	const sessionId = res.headers.get("mcp-session-id");
	await fetch(url, {
		method: "POST",
		headers: {
			"content-type": "application/json",
			accept: "application/json, text/event-stream",
			...(sessionId ? { "mcp-session-id": sessionId } : {}),
		},
		body: JSON.stringify({ jsonrpc: "2.0", method: "notifications/initialized" }),
		signal: AbortSignal.timeout(10_000),
	}).catch(() => undefined);
	return { url, sessionId };
}

function textOf(result: Record<string, unknown>): string {
	const content = (result.content || []) as { type?: string; text?: string }[];
	return content
		.filter((c) => c.type === "text")
		.map((c) => c.text || "")
		.join("");
}

// Same server-side ledger dump as the guard (single implementation in the
// guard file; duplicated here deliberately tiny so action=save leaves evidence).
const ledgerDumpScript = (outPath: string): string => [
	"import json, os, time, idautils",
	`_out = r"${outPath}"`,
	"os.makedirs(os.path.dirname(_out), exist_ok=True)",
	"_skip = ('sub_','loc_','unk_','off_','asc_','byte_','word_','dword_','qword_','flt_','dbl_','align_','noret_','seg_','doubleword_')",
	"_names = []",
	"for ea, nm in sorted(idautils.Names()):",
	"    if not nm.startswith(_skip):",
	"        _names.append((hex(ea), nm))",
	"_meta = {'ts': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()), 'entries': len(_names), 'note': 'EA->name ledger via ida action=save'}",
	"_f = open(_out, 'w')",
	"_f.write(json.dumps(_meta) + chr(10))",
	"for ea, nm in _names:",
	"    _f.write(json.dumps({'ea': ea, 'name': nm}) + chr(10))",
	"_f.close()",
	"print(json.dumps({'ledger': _out, 'entries': len(_names)}))",
].join("\n");

function ledgerDir(): string {
	return path.resolve(baseCwd, process.env.OPENVB_LEDGER_DIR || "artifacts/ledgers");
}

function idbPathOf(t: Target): string {
	return process.env[t.idbEnv] ? path.resolve(baseCwd, process.env[t.idbEnv]!) : path.resolve(baseCwd, t.idbDefault);
}

function diskProof(t: Target, census?: Record<string, unknown>): string {
	let m: number | undefined;
	try {
		m = fs.statSync(idbPathOf(t)).mtimeMs;
	} catch {
		/* missing */
	}
	const proven = m !== undefined;
	try {
		const dir = path.dirname(idbPathOf(t));
		fs.mkdirSync(dir, { recursive: true });
		fs.appendFileSync(
			path.join(dir, "idb-guard-census.log"),
			JSON.stringify({
				ts: new Date().toISOString(),
				server: t.name,
				via: "ida-tool",
				disk: idbPathOf(t),
				mtime: m ? new Date(m).toISOString() : null,
				proven,
				...(census ? { census } : {}),
			}) + "\n",
		);
	} catch {
		/* log best effort */
	}
	return m ? new Date(m).toISOString() : "disk-file-not-visible";
}

// ---------------------------------- the tool ----------------------------------

const Params = Type.Object({
	target: Type.Union([Type.Literal("original"), Type.Literal("reimpl")], {
		description: "REQUIRED. original = authoritative ida_original_f3 reference; reimpl = validation target only",
	}),
	action: Type.Union([
		Type.Literal("health"),
		Type.Literal("list_tools"),
		Type.Literal("call"),
		Type.Literal("batch"),
		Type.Literal("save"),
	]),
	tool: Type.Optional(Type.String({ description: "server tool name (action=call) - verify with list_tools first" })),
	args: Type.Optional(Type.Record(Type.String(), Type.Unknown(), { description: "arguments for the server tool" })),
	ops: Type.Optional(
		Type.Array(
			Type.Object({ tool: Type.String(), args: Type.Optional(Type.Record(Type.String(), Type.Unknown())) }),
			{ maxItems: MAX_OPS },
		),
	),
	timeoutSeconds: Type.Optional(Type.Number({ minimum: 1, maximum: 600, description: "per-op timeout, default 120" })),
});

export default function (pi: ExtensionAPI) {
	pi.registerTool({
		name: "ida",
		label: "IDA",
		description:
			"Run headless IDA Pro actions on a target database. First use per target: action=list_tools to discover the server's real tool names/schemas. Then call (one op) or batch (up to 25 sequential ops in one round trip). health checks the server. save performs the mandatory save_database + disk-mtime census. NEVER infer original behavior from target=reimpl.",
		promptSnippet: "Direct access to the two headless IDA MCP servers (target: original|reimpl required; health/list_tools/call/batch/save)",
		promptGuidelines: [
			"Before doing IDA work, decide the target explicitly; default is original. Use list_tools once per session; never guess server tool names.",
			"Batch related queries into one ida call (up to 25 ops) instead of many single calls.",
			"After IDB mutations (renames, comments, types), the persistence guard autosaves; call action=save explicitly before risky restarts.",
		],
		parameters: Params,
		async execute(_toolCallId, params, _signal, _onUpdate, _ctx) {
			const t = TARGETS[params.target];
			const timeoutMs = (params.timeoutSeconds ?? 120) * 1000;
			const normArgs = (v: unknown): Record<string, unknown> => {
				if (v && typeof v === "object") return v as Record<string, unknown>;
				if (typeof v === "string") {
					try {
						return JSON.parse(v) as Record<string, unknown>;
					} catch {
						return {};
					}
				}
				return {};
			};
			const result: { content: { type: "text"; text: string }[]; isError?: boolean } = {
				content: [{ type: "text", text: "" }],
			};
			const emit = (txt: string, isError = false) => {
				result.content[0].text = txt.slice(0, RESULT_CAP) + (txt.length > RESULT_CAP ? "\n[truncated]" : "");
				result.isError = isError;
				return result; // every execute path MUST return the result object
			};
			try {
				switch (params.action) {
					case "health": {
						sessions.delete(t.name);
						const sess = await mcpInitialize(t, endpointOf(t));
						sessions.set(t.name, sess);
						const tools = await mcpRequest(t, { jsonrpc: "2.0", id: 1, method: "tools/list" }, timeoutMs);
						emit(
							JSON.stringify({
								server: t.name,
								endpoint: endpointOf(t),
								alive: true,
								sessionId: sess.sessionId ? "yes" : "none (stateless)",
								toolCount: ((tools.tools || []) as unknown[]).length,
							}),
						);
						break;
					}
					case "list_tools": {
						const tools = await mcpRequest(t, { jsonrpc: "2.0", id: 1, method: "tools/list" }, timeoutMs);
						const compact = ((tools.tools || []) as { name: string; description?: string; inputSchema?: { properties?: Record<string, unknown> } }[]).map(
							(x) => ({
								name: x.name,
								params: x.inputSchema?.properties ? Object.keys(x.inputSchema.properties) : [],
								help: (x.description || "").split("\n")[0].slice(0, 100),
							}),
						);
						emit(JSON.stringify(compact, null, 1));
						break;
					}
					case "save": {
						const r = await mcpRequest(
							t,
							{ jsonrpc: "2.0", id: 2, method: "tools/call", params: { name: "py_eval", arguments: { code: SAVE_SCRIPT } } },
							timeoutMs,
						);
						const txt = textOf(r);
						let census: Record<string, unknown> | undefined;
						try {
							// py_eval wraps output as {"result":..., "stdout":..., "stderr":...}
							const outer = JSON.parse(txt.trim());
							const inner = typeof outer.stdout === "string" ? outer.stdout : txt;
							census = JSON.parse(String(inner).trim().split("\n").pop() || "");
						} catch {
							try {
								census = JSON.parse((txt.trim().split("\n").pop() || "").slice(0, 400));
							} catch {
								/* census optional */
							}
						}
						const mtime = diskProof(t, census);
						let ledger = "";
						if (census && census.saved === true) {
							const out = path.join(ledgerDir(), `${t.name}-save-${new Date().toISOString().replace(/[:.]/g, "-")}.jsonl`);
							try {
								const lr = await mcpRequest(t, {
									jsonrpc: "2.0",
									id: 3,
									method: "tools/call",
									params: { name: "py_eval", arguments: { code: ledgerDumpScript(out) } },
								}, timeoutMs);
								ledger = lr.isError ? `ledger FAILED: ${textOf(lr).slice(0, 120)}` : out;
								if (!lr.isError) {
									try {
										const all = fs.readdirSync(ledgerDir()).filter((f) => f.endsWith(".jsonl")).sort();
										for (const old of all.slice(0, Math.max(0, all.length - 20))) fs.unlinkSync(path.join(ledgerDir(), old));
									} catch { /* prune best effort */ }
								}
							} catch (e) {
								ledger = `ledger FAILED: ${e instanceof Error ? e.message : String(e)}`;
							}
						}
						emit(
							JSON.stringify(
								{
									server: t.name,
									census,
									ledger,
									disk_mtime: mtime,
									note: "save requested through ida tool; guard watches this call too",
								},
								null,
								1,
							),
							!!r.isError,
						);
						break;
					}
					case "call": {
						if (!params.tool) return emit("action=call requires tool=<name from list_tools>", true);
						// (return stays explicit above; never return emit's void)
						const r = await mcpRequest(t, {
							jsonrpc: "2.0",
							id: 2,
							method: "tools/call",
							params: { name: params.tool, arguments: normArgs(params.args) },
						}, timeoutMs);
						return emit(textOf(r), !!r.isError);
					}
					case "batch": {
						if (!params.ops?.length) return emit("action=batch requires ops=[{tool,args}...] (max 25)", true);
						const out: { i: number; tool: string; ok: boolean; result?: string; error?: string }[] = [];
						for (let i = 0; i < params.ops.length; i++) {
							const op = params.ops[i];
							try {
								const r = await mcpRequest(t, {
									jsonrpc: "2.0",
									id: 2,
									method: "tools/call",
									params: { name: op.tool, arguments: normArgs(op.args) },
								}, timeoutMs);
								const txt = textOf(r);
								if (r.isError) {
									out.push({ i, tool: op.tool, ok: false, error: txt.slice(0, 400) });
									break; // ordering matters: a failed op invalidates the rest
									}
									out.push({ i, tool: op.tool, ok: true, result: txt });
							} catch (err) {
								out.push({ i, tool: op.tool, ok: false, error: err instanceof Error ? err.message : String(err) });
								break; // ordering matters: stop after a failed op
							}
						}
						emit(JSON.stringify(out, null, 1));
						break;
					}
				}
			} catch (err) {
				emit(`ida ${params.target} ${params.action}: ${err instanceof Error ? err.message : String(err)}`, true);
			}
			return result;
		},
	});

	pi.on("session_start", (_e: unknown, ctx: { cwd: string }) => {
		baseCwd = ctx.cwd;
		sessions.clear();
	});
}

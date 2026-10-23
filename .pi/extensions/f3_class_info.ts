/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_class_info.ts.
 */

import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";
import { Type } from "typebox";
import { existsSync } from "node:fs";
import { readFile } from "node:fs/promises";
import { dirname, join, resolve } from "node:path";

function dataFile(startDir: string): string {
	const fromEnv = process.env.OPENVB_F3_JSON;
	if (fromEnv) return fromEnv;
	let dir = resolve(startDir);
	for (;;) {
		const candidate = join(dir, "artifacts", "F3.json");
		if (existsSync(candidate)) return candidate;
		const parent = dirname(dir);
		if (parent === dir) break;
		dir = parent;
	}
	throw new Error(
		`F3.json not found at or above ${startDir}; set OPENVB_F3_JSON to override (expected artifacts/F3.json).`,
	);
}

function norm(v: unknown): string {
	return String(v ?? "").toLowerCase();
}

function findStructure(structures: Record<string, unknown>, query: string): [string, any] | null {
	const q = norm(query);

	for (const [key, s] of Object.entries(structures) as [string, any][]) {
		if (norm(key) === q || norm(s.name) === q || norm(s.demangled_name) === q) {
			return [key, s];
		}
	}

	for (const [key, s] of Object.entries(structures) as [string, any][]) {
		if (norm(key).includes(q) || norm(s.name).includes(q) || norm(s.demangled_name).includes(q)) {
			return [key, s];
		}
	}

	return null;
}

export default function (pi: ExtensionAPI) {
	pi.registerTool({
		name: "f3_class_info",
		label: "F3 class info",
		description:
			"Return detailed F3 class/struct information including size, members, methods, and vftables.",
		promptSnippet: "Detailed F3 class/struct info (members, methods, vftables)",
		parameters: Type.Object({
			class: Type.String({
				description:
					"Class/struct name, demangled name, or mangled key, e.g. ActionMoveTo or .?AUActionMoveTo@@",
			}),
			include_members: Type.Optional(
				Type.Boolean({ description: "Include members. Defaults to true." }),
			),
			include_methods: Type.Optional(
				Type.Boolean({ description: "Include methods. Defaults to true." }),
			),
			include_vftables: Type.Optional(
				Type.Boolean({ description: "Include vftables. Defaults to true." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, args, signal, _onUpdate, ctx) {
			if (signal?.aborted) throw new Error("cancelled");
			const db = JSON.parse(await readFile(dataFile(ctx.cwd), "utf8"));
			const structures = db.structures ?? {};

			const found = findStructure(structures, args.class);
			if (!found) {
				return {
					content: [{ type: "text" as const, text: `No class/struct found for: ${args.class}` }],
					details: undefined,
				};
			}

			const [key, s] = found;
			const result: any = {
				key,
				name: s.name,
				demangled_name: s.demangled_name,
				size: s.size,
				member_count: Object.keys(s.members ?? {}).length,
				method_count: Object.keys(s.methods ?? {}).length,
				vftable_count: Object.keys(s.vftables ?? {}).length,
			};

			if (args.include_members ?? true) result.members = s.members ?? {};
			if (args.include_methods ?? true) result.methods = s.methods ?? {};
			if (args.include_vftables ?? true) result.vftables = s.vftables ?? {};

			return {
				content: [{ type: "text" as const, text: JSON.stringify(result, null, 2) }],
				details: undefined,
			};
		},
	});
}

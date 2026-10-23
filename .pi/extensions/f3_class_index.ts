/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_class_index.ts.
 * The OpenCode original remains the source of truth for the OpenCode harness.
 */

import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";
import { Type } from "typebox";
import { existsSync } from "node:fs";
import { readFile } from "node:fs/promises";
import { dirname, join, resolve } from "node:path";

/**
 * OpenCode hardcoded the absolute artifact path. In Pi, allow OPENVB_F3_JSON to
 * override and otherwise locate artifacts/F3.json at or above the session cwd.
 */
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

function text(v: unknown): string {
	return String(v ?? "").toLowerCase();
}

export default function (pi: ExtensionAPI) {
	pi.registerTool({
		name: "f3_class_index",
		label: "F3 class index",
		description:
			"Search the F3 object/type database for classes and structs by mangled name, demangled name, or prefix.",
		promptSnippet: "Search F3 classes/structs by name",
		parameters: Type.Object({
			query: Type.Optional(
				Type.String({ description: "Optional search text, e.g. Action, Audio, Gfx, cls_0x" }),
			),
			limit: Type.Optional(
				Type.Number({ description: "Maximum number of results to return. Defaults to 50." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, args, signal, _onUpdate, ctx) {
			if (signal?.aborted) throw new Error("cancelled");
			const db = JSON.parse(await readFile(dataFile(ctx.cwd), "utf8"));
			const structures = db.structures ?? {};

			const q = text(args.query);
			const limit = args.limit ?? 50;

			const rows = Object.entries(structures)
				.filter(([key, s]: [string, any]) => {
					if (!q) return true;
					return (
						text(key).includes(q) ||
						text(s.name).includes(q) ||
						text(s.demangled_name).includes(q)
					);
				})
				.slice(0, limit)
				.map(([key, s]: [string, any]) => ({
					key,
					name: s.name,
					demangled_name: s.demangled_name,
					size: s.size,
					member_count: Object.keys(s.members ?? {}).length,
					method_count: Object.keys(s.methods ?? {}).length,
					vftable_count: Object.keys(s.vftables ?? {}).length,
				}));

			return {
				content: [
					{
						type: "text" as const,
						text: JSON.stringify(
							{
								file: db.filename,
								total_matches_returned: rows.length,
								results: rows,
							},
							null,
							2,
						),
					},
				],
				details: undefined,
			};
		},
	});
}

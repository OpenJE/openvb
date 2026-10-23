/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_vftable_index.ts.
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

function text(v: unknown): string {
	return String(v ?? "").toLowerCase();
}

export default function (pi: ExtensionAPI) {
	pi.registerTool({
		name: "f3_vftable_index",
		label: "F3 vftable index",
		description:
			"Search F3 vftables by class, vftable address, entry offset, or target method EA.",
		promptSnippet: "Search F3 vftables by class, address, entry target",
		parameters: Type.Object({
			query: Type.Optional(
				Type.String({ description: "Search text, e.g. 0x6940e4, 0x55b450, ActionMoveTo" }),
			),
			class: Type.Optional(Type.String({ description: "Optional class/struct filter." })),
			limit: Type.Optional(
				Type.Number({ description: "Maximum number of results. Defaults to 50." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, args, signal, _onUpdate, ctx) {
			if (signal?.aborted) throw new Error("cancelled");
			const db = JSON.parse(await readFile(dataFile(ctx.cwd), "utf8"));
			const structures = db.structures ?? {};

			const q = text(args.query);
			const classFilter = text(args.class);
			const limit = args.limit ?? 50;
			const results: any[] = [];

			const done = () => ({
				content: [{ type: "text" as const, text: JSON.stringify({ results }, null, 2) }],
				details: undefined,
			});

			for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
				const classText = [classKey, s.name, s.demangled_name].map(text).join(" ");
				if (classFilter && !classText.includes(classFilter)) continue;

				for (const [vftableEa, vf] of Object.entries(s.vftables ?? {}) as [string, any][]) {
					const entries = Object.entries(vf.entries ?? {}).map(
						([slot, entry]: [string, any]) => ({
							slot,
							...entry,
						}),
					);

					const haystack = [
						classKey,
						s.name,
						s.demangled_name,
						vftableEa,
						vf.ea,
						vf.vftptr,
						vf.length,
						...entries.flatMap((e: any) => [e.slot, e.ea, e.name, e.type, e.demangled_name]),
					]
						.map(text)
						.join(" ");

					if (q && !haystack.includes(q)) continue;

					results.push({
						class_key: classKey,
						class_name: s.name,
						demangled_name: s.demangled_name,
						vftable_ea: vftableEa,
						vftable: vf,
					});

					if (results.length >= limit) return done();
				}
			}

			return done();
		},
	});
}

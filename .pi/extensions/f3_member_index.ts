/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_member_index.ts.
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
		name: "f3_member_index",
		label: "F3 member index",
		description:
			"Search F3 class/struct members by class name, member name, offset, type, referenced struct, or usage address.",
		promptSnippet: "Search F3 members by class, name, offset, type, usage",
		parameters: Type.Object({
			query: Type.Optional(
				Type.String({
					description: "Search text, e.g. mbr_0x14, 0x14, vftptr, Action, 0x55b450",
				}),
			),
			class: Type.Optional(Type.String({ description: "Optional class/struct filter." })),
			limit: Type.Optional(
				Type.Number({ description: "Maximum number of results. Defaults to 100." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, args, signal, _onUpdate, ctx) {
			if (signal?.aborted) throw new Error("cancelled");
			const db = JSON.parse(await readFile(dataFile(ctx.cwd), "utf8"));
			const structures = db.structures ?? {};

			const q = text(args.query);
			const classFilter = text(args.class);
			const limit = args.limit ?? 100;
			const results: any[] = [];

			const done = () => ({
				content: [{ type: "text" as const, text: JSON.stringify({ results }, null, 2) }],
				details: undefined,
			});

			for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
				const classText = [classKey, s.name, s.demangled_name].map(text).join(" ");

				if (classFilter && !classText.includes(classFilter)) continue;

				for (const [offset, m] of Object.entries(s.members ?? {}) as [string, any][]) {
					const haystack = [
						classKey,
						s.name,
						s.demangled_name,
						offset,
						m.name,
						m.offset,
						m.type,
						m.struc,
						...(m.usages ?? []),
					]
						.map(text)
						.join(" ");

					if (q && !haystack.includes(q)) continue;

					results.push({
						class_key: classKey,
						class_name: s.name,
						demangled_name: s.demangled_name,
						offset,
						member: m,
					});

					if (results.length >= limit) return done();
				}
			}

			return done();
		},
	});
}

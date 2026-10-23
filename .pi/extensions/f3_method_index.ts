/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_method_index.ts.
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
		name: "f3_method_index",
		label: "F3 method index",
		description:
			"Search F3 class/struct methods by method name, EA/address, type, class name, or demangled name.",
		promptSnippet: "Search F3 methods by name, EA, type, class",
		parameters: Type.Object({
			query: Type.Optional(
				Type.String({
					description: "Search text, e.g. ctor, dtor, virt_meth, 0x55b450, ActionMoveTo",
				}),
			),
			type: Type.Optional(
				Type.String({
					description: "Optional method type filter, e.g. ctor, dtor, meth, virt_meth, deldtor.",
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
			const typeFilter = text(args.type);
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

				for (const [ea, method] of Object.entries(s.methods ?? {}) as [string, any][]) {
					if (typeFilter && text(method.type) !== typeFilter) continue;

					const haystack = [
						classKey,
						s.name,
						s.demangled_name,
						ea,
						method.ea,
						method.name,
						method.demangled_name,
						method.type,
					]
						.map(text)
						.join(" ");

					if (q && !haystack.includes(q)) continue;

					results.push({
						class_key: classKey,
						class_name: s.name,
						demangled_name: s.demangled_name,
						ea,
						method,
					});

					if (results.length >= limit) return done();
				}
			}

			return done();
		},
	});
}

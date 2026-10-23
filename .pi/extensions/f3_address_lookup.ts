/**
 * Native Pi tool, ported 1:1 from .opencode/tools/f3_address_lookup.ts.
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

function normAddr(v: unknown): string {
	return String(v ?? "").toLowerCase();
}

export default function (pi: ExtensionAPI) {
	pi.registerTool({
		name: "f3_address_lookup",
		label: "F3 address lookup",
		description:
			"Look up an F3 address across methods, vftables, vftable entries, member usages, and member references.",
		promptSnippet: "Look up an F3 address across methods, vftables, members",
		parameters: Type.Object({
			address: Type.String({ description: "Address to find, e.g. 0x55b450" }),
			limit: Type.Optional(
				Type.Number({ description: "Maximum number of matches. Defaults to 100." }),
			),
		}),
		annotations: { readOnlyHint: true, openWorldHint: false },
		async execute(_toolCallId, args, signal, _onUpdate, ctx) {
			if (signal?.aborted) throw new Error("cancelled");
			const db = JSON.parse(await readFile(dataFile(ctx.cwd), "utf8"));
			const structures = db.structures ?? {};

			const needle = normAddr(args.address);
			const limit = args.limit ?? 100;
			const matches: any[] = [];

			function add(match: any) {
				if (matches.length < limit) matches.push(match);
			}

			for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
				const classInfo = {
					class_key: classKey,
					class_name: s.name,
					demangled_name: s.demangled_name,
				};

				for (const [offset, m] of Object.entries(s.members ?? {}) as [string, any][]) {
					if (normAddr(offset) === needle || normAddr(m.offset) === needle) {
						add({ kind: "member_offset", ...classInfo, offset, member: m });
					}

					for (const usage of m.usages ?? []) {
						if (normAddr(usage) === needle) {
							add({ kind: "member_usage", ...classInfo, offset, member: m });
						}
					}
				}

				for (const [ea, method] of Object.entries(s.methods ?? {}) as [string, any][]) {
					if (normAddr(ea) === needle || normAddr(method.ea) === needle) {
						add({ kind: "method", ...classInfo, ea, method });
					}
				}

				for (const [vftableEa, vf] of Object.entries(s.vftables ?? {}) as [string, any][]) {
					if (normAddr(vftableEa) === needle || normAddr(vf.ea) === needle) {
						add({ kind: "vftable", ...classInfo, vftable_ea: vftableEa, vftable: vf });
					}

					for (const [slot, entry] of Object.entries(vf.entries ?? {}) as [string, any][]) {
						if (normAddr(entry.ea) === needle) {
							add({
								kind: "vftable_entry",
								...classInfo,
								vftable_ea: vftableEa,
								slot,
								entry,
							});
						}
					}
				}
			}

			return {
				content: [
					{
						type: "text" as const,
						text: JSON.stringify(
							{ address: args.address, match_count: matches.length, matches },
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

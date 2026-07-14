# ./devenv/opencode/tool/f3-member-index.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/artifacts/F3.json";
in {
  opencode.tools.f3_member_index = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function text(v: unknown): string {
      return String(v ?? "").toLowerCase()
    }

    export default tool({
      description: "Search F3 class/struct members by class name, member name, offset, type, referenced struct, or usage address.",
      args: {
        query: tool.schema.string().optional().describe("Search text, e.g. mbr_0x14, 0x14, vftptr, Action, 0x55b450"),
        class: tool.schema.string().optional().describe("Optional class/struct filter."),
        limit: tool.schema.number().optional().describe("Maximum number of results. Defaults to 100."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const q = text(args.query)
        const classFilter = text(args.class)
        const limit = args.limit ?? 100
        const results: any[] = []

        for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
          const classText = [classKey, s.name, s.demangled_name].map(text).join(" ")

          if (classFilter && !classText.includes(classFilter)) continue

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
            ].map(text).join(" ")

            if (q && !haystack.includes(q)) continue

            results.push({
              class_key: classKey,
              class_name: s.name,
              demangled_name: s.demangled_name,
              offset,
              member: m,
            })

            if (results.length >= limit) {
              return JSON.stringify({ results }, null, 2)
            }
          }
        }

        return JSON.stringify({ results }, null, 2)
      },
    })
  '';
}

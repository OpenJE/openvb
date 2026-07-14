# ./devenv/opencode/tool/f3-vftable-index.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/artifacts/F3.json";
in {
  opencode.tools.f3_vftable_index = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function text(v: unknown): string {
      return String(v ?? "").toLowerCase()
    }

    export default tool({
      description: "Search F3 vftables by class, vftable address, entry offset, or target method EA.",
      args: {
        query: tool.schema.string().optional().describe("Search text, e.g. 0x6940e4, 0x55b450, ActionMoveTo"),
        class: tool.schema.string().optional().describe("Optional class/struct filter."),
        limit: tool.schema.number().optional().describe("Maximum number of results. Defaults to 50."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const q = text(args.query)
        const classFilter = text(args.class)
        const limit = args.limit ?? 50
        const results: any[] = []

        for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
          const classText = [classKey, s.name, s.demangled_name].map(text).join(" ")
          if (classFilter && !classText.includes(classFilter)) continue

          for (const [vftableEa, vf] of Object.entries(s.vftables ?? {}) as [string, any][]) {
            const entries = Object.entries(vf.entries ?? {}).map(([slot, entry]: [string, any]) => ({
              slot,
              ...entry,
            }))

            const haystack = [
              classKey,
              s.name,
              s.demangled_name,
              vftableEa,
              vf.ea,
              vf.vftptr,
              vf.length,
              ...entries.flatMap((e: any) => [e.slot, e.ea, e.name, e.type, e.demangled_name]),
            ].map(text).join(" ")

            if (q && !haystack.includes(q)) continue

            results.push({
              class_key: classKey,
              class_name: s.name,
              demangled_name: s.demangled_name,
              vftable_ea: vftableEa,
              vftable: vf,
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

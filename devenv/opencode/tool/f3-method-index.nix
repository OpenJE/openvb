# ./devenv/opencode/tool/f3-method-index.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/artifacts/F3.json";
in {
  opencode.tools.f3_method_index = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function text(v: unknown): string {
      return String(v ?? "").toLowerCase()
    }

    export default tool({
      description: "Search F3 class/struct methods by method name, EA/address, type, class name, or demangled name.",
      args: {
        query: tool.schema.string().optional().describe("Search text, e.g. ctor, dtor, virt_meth, 0x55b450, ActionMoveTo"),
        type: tool.schema.string().optional().describe("Optional method type filter, e.g. ctor, dtor, meth, virt_meth, deldtor."),
        class: tool.schema.string().optional().describe("Optional class/struct filter."),
        limit: tool.schema.number().optional().describe("Maximum number of results. Defaults to 100."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const q = text(args.query)
        const typeFilter = text(args.type)
        const classFilter = text(args.class)
        const limit = args.limit ?? 100
        const results: any[] = []

        for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
          const classText = [classKey, s.name, s.demangled_name].map(text).join(" ")
          if (classFilter && !classText.includes(classFilter)) continue

          for (const [ea, method] of Object.entries(s.methods ?? {}) as [string, any][]) {
            if (typeFilter && text(method.type) !== typeFilter) continue

            const haystack = [
              classKey,
              s.name,
              s.demangled_name,
              ea,
              method.ea,
              method.name,
              method.demangled_name,
              method.type,
            ].map(text).join(" ")

            if (q && !haystack.includes(q)) continue

            results.push({
              class_key: classKey,
              class_name: s.name,
              demangled_name: s.demangled_name,
              ea,
              method,
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

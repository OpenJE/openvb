# ./devenv/opencode/tool/f3-class-index.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/artifacts/F3.json";
in {
  opencode.tools.f3_class_index = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function text(v: unknown): string {
      return String(v ?? "").toLowerCase()
    }

    export default tool({
      description: "Search the F3 object/type database for classes and structs by mangled name, demangled name, or prefix.",
      args: {
        query: tool.schema.string().optional().describe("Optional search text, e.g. Action, Audio, Gfx, cls_0x"),
        limit: tool.schema.number().optional().describe("Maximum number of results to return. Defaults to 50."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const q = text(args.query)
        const limit = args.limit ?? 50

        const rows = Object.entries(structures)
          .filter(([key, s]: [string, any]) => {
            if (!q) return true
            return (
              text(key).includes(q) ||
              text(s.name).includes(q) ||
              text(s.demangled_name).includes(q)
            )
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
          }))

        return JSON.stringify({
          file: db.filename,
          total_matches_returned: rows.length,
          results: rows,
        }, null, 2)
      },
    })
  '';
}

# ./devenv/opencode/tool/f3-class-info.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/F3.json";
in {
  opencode.tools.f3_class_info = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function norm(v: unknown): string {
      return String(v ?? "").toLowerCase()
    }

    function findStructure(structures: any, query: string) {
      const q = norm(query)

      for (const [key, s] of Object.entries(structures) as [string, any][]) {
        if (
          norm(key) === q ||
          norm(s.name) === q ||
          norm(s.demangled_name) === q
        ) {
          return [key, s]
        }
      }

      for (const [key, s] of Object.entries(structures) as [string, any][]) {
        if (
          norm(key).includes(q) ||
          norm(s.name).includes(q) ||
          norm(s.demangled_name).includes(q)
        ) {
          return [key, s]
        }
      }

      return null
    }

    export default tool({
      description: "Return detailed F3 class/struct information including size, members, methods, and vftables.",
      args: {
        class: tool.schema.string().describe("Class/struct name, demangled name, or mangled key, e.g. ActionMoveTo or .?AUActionMoveTo@@"),
        include_members: tool.schema.boolean().optional().describe("Include members. Defaults to true."),
        include_methods: tool.schema.boolean().optional().describe("Include methods. Defaults to true."),
        include_vftables: tool.schema.boolean().optional().describe("Include vftables. Defaults to true."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const found = findStructure(structures, args.class)
        if (!found) {
          return `No class/struct found for: ''${args.class}`
        }

        const [key, s] = found
        const result: any = {
          key,
          name: s.name,
          demangled_name: s.demangled_name,
          size: s.size,
          member_count: Object.keys(s.members ?? {}).length,
          method_count: Object.keys(s.methods ?? {}).length,
          vftable_count: Object.keys(s.vftables ?? {}).length,
        }

        if (args.include_members ?? true) result.members = s.members ?? {}
        if (args.include_methods ?? true) result.methods = s.methods ?? {}
        if (args.include_vftables ?? true) result.vftables = s.vftables ?? {}

        return JSON.stringify(result, null, 2)
      },
    })
  '';
}

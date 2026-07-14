# ./devenv/opencode/tool/f3-address-lookup.nix

{ config, ... }:
let
  nodeModules = "${config.devenv.root}/.opencode/node_modules";
  dataFile = "${config.devenv.root}/artifacts/F3.json";
in {
  opencode.tools.f3_address_lookup = /* typescript */ ''
    import { tool } from "${nodeModules}/@opencode-ai/plugin/dist/tool.js"
    import { readFile } from "node:fs/promises"

    const DATA_FILE = "${dataFile}"

    function normAddr(v: string): string {
      return String(v ?? "").toLowerCase()
    }

    export default tool({
      description: "Look up an F3 address across methods, vftables, vftable entries, member usages, and member references.",
      args: {
        address: tool.schema.string().describe("Address to find, e.g. 0x55b450"),
        limit: tool.schema.number().optional().describe("Maximum number of matches. Defaults to 100."),
      },
      async execute(args) {
        const raw = await readFile(DATA_FILE, "utf8")
        const db = JSON.parse(raw)
        const structures = db.structures ?? {}

        const needle = normAddr(args.address)
        const limit = args.limit ?? 100
        const matches: any[] = []

        function add(match: any) {
          if (matches.length < limit) matches.push(match)
        }

        for (const [classKey, s] of Object.entries(structures) as [string, any][]) {
          const classInfo = {
            class_key: classKey,
            class_name: s.name,
            demangled_name: s.demangled_name,
          }

          for (const [offset, m] of Object.entries(s.members ?? {}) as [string, any][]) {
            if (normAddr(offset) === needle || normAddr(m.offset) === needle) {
              add({ kind: "member_offset", ...classInfo, offset, member: m })
            }

            for (const usage of m.usages ?? []) {
              if (normAddr(usage) === needle) {
                add({ kind: "member_usage", ...classInfo, offset, member: m })
              }
            }
          }

          for (const [ea, method] of Object.entries(s.methods ?? {}) as [string, any][]) {
            if (normAddr(ea) === needle || normAddr(method.ea) === needle) {
              add({ kind: "method", ...classInfo, ea, method })
            }
          }

          for (const [vftableEa, vf] of Object.entries(s.vftables ?? {}) as [string, any][]) {
            if (normAddr(vftableEa) === needle || normAddr(vf.ea) === needle) {
              add({ kind: "vftable", ...classInfo, vftable_ea: vftableEa, vftable: vf })
            }

            for (const [slot, entry] of Object.entries(vf.entries ?? {}) as [string, any][]) {
              if (normAddr(entry.ea) === needle) {
                add({
                  kind: "vftable_entry",
                  ...classInfo,
                  vftable_ea: vftableEa,
                  slot,
                  entry,
                })
              }
            }
          }
        }

        return JSON.stringify({
          address: args.address,
          match_count: matches.length,
          matches,
        }, null, 2)
      },
    })
  '';
}

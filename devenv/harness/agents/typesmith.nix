# ./devenv/harness/agents/typesmith.nix

{ lib, ... }:
let
  name = "typesmith";
  description = "Type reconstruction specialist for signatures, stack vars, structs, enums, and IDA naming cleanup when type uncertainty blocks analysis.";
  prompt = lib.strings.removeSuffix "\n" ''
    You are the Van Buren type reconstruction specialist.

    Your job is to resolve type uncertainty that blocks behavioral understanding.

    Responsibilities:
    - Infer or refine function signatures.
    - Reconstruct stack variables, locals, structs, enums, and field meanings.
    - Suggest conservative renames when evidence supports them.
    - Use IDA MCP type and stack tools rather than guessing.

    Rules:
    - Favor minimal, evidence-backed type changes.
    - Prefer refining existing IDA types and OOAnalyzer-derived structure over replacing them wholesale.
    - Treat existing type/class ownership as strong prior evidence unless fresh evidence clearly contradicts it.
    - Distinguish safe type assertions from tentative ones.
    - Return blocking ambiguities explicitly.

    Return:
    1. Current type state vs proposed refinements
    2. Why they fit the evidence
    3. Remaining ambiguity or contradiction
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    description: ${description}
    mode: subagent
    model: lmstudio/qwen3.6-35b-a3b@q4_k_xl
    temperature: 0.1
    permission:
      "*": deny
      read: allow
      glob: allow
      grep: allow
      list: allow
      edit:
        "*": deny
      skill:
        "type-recovery": allow
        "engine-game-boundary": allow
        "binary-faithfulness": allow
      bash:
        "*": deny
      "ida-reimpl_idalib_open": allow
      "ida-reimpl_idalib_switch": allow
      "ida-reimpl_idalib_current": allow
      "ida-reimpl_idalib_list": allow
      "ida-reimpl_idalib_unbind": allow
      "ida-reimpl_idalib_health": allow
      "ida-reimpl_idalib_warmup": allow
      "ida-reimpl_lookup_funcs": allow
      "ida-reimpl_decompile": allow
      "ida-reimpl_disasm": allow
      "ida-reimpl_stack_frame": allow
      "ida-reimpl_search_structs": allow
      "ida-reimpl_read_struct": allow
      "ida-reimpl_xrefs_to_field": allow
      "ida-reimpl_export_funcs": allow
      "ida-reimpl_infer_types": allow
      "ida-reimpl_list_globals": allow
      "ida-reimpl_get_global_value": allow
      "ida-reimpl_int_convert": allow
  '';
in {
  opencode.agents.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}

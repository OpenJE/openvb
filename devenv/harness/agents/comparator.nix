# ./devenv/harness/agents/comparator.nix

{ lib, ... }:
let
  name = "comparator";
  description = "Original-versus-reimplementation comparison specialist that escalates from binary surface evidence up through pseudocode and runtime confirmation.";
  prompt = lib.strings.removeSuffix "\n" ''
    You compare the original Van Buren demo against the rebuilt reimplementation.

    Your job:
    - Compare the requested target across the original and rebuilt sides.
    - Escalate from cheap evidence to expensive evidence only when needed.
    - Ensure the original side is using `OPENVB_ORIG_IDB` when available.
    - Ensure the rebuilt side is using `OPENVB_REIMPL_IDB` when available, otherwise open `OPENVB_REIMPL_EXE`.
    - Use the original-side IDA tools for the packaged demo and the reimplementation-side IDA tools for the rebuilt binary.
    - Call out the first stable divergence point when you can localize it.
    - Request lockstep debugging only when static comparison remains inconclusive.

    Rules:
    - Use the comparison ladder skill.
    - Prefer symbolic/function alignment over raw address equality.
    - Distinguish exact match, structural match, semantic match, and mismatch.
    - Do not jump to runtime debugging if lower-fidelity evidence is already decisive.

    Return:
    1. Scope compared
    2. Comparison level used
    3. Alignment result
    4. Evidence
    5. First known divergence or why comparison is inconclusive
    6. Recommended next action
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    description: ${description}
    mode: subagent
    hidden: true
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
        "comparison-ladder": allow
        "database-targeting": allow
        "binary-faithfulness": allow
        "function-reconstruction": allow
        "type-recovery": allow
      task:
        "decompiler": allow
        "typesmith": allow
        "auditor": allow
        "lockstep-debugger": allow
      bash:
        "*": deny
      "ida-orig_idalib_open": allow
      "ida-orig_idalib_switch": allow
      "ida-orig_idalib_current": allow
      "ida-orig_idalib_list": allow
      "ida-orig_idalib_unbind": allow
      "ida-orig_idalib_health": allow
      "ida-orig_idalib_warmup": allow
      "ida-orig_lookup_funcs": allow
      "ida-orig_list_funcs": allow
      "ida-orig_list_globals": allow
      "ida-orig_imports": allow
      "ida-orig_decompile": allow
      "ida-orig_disasm": allow
      "ida-orig_xrefs_to": allow
      "ida-orig_xrefs_to_field": allow
      "ida-orig_callees": allow
      "ida-orig_get_bytes": allow
      "ida-orig_get_int": allow
      "ida-orig_get_string": allow
      "ida-orig_get_global_value": allow
      "ida-orig_stack_frame": allow
      "ida-orig_read_struct": allow
      "ida-orig_search_structs": allow
      "ida-orig_find_regex": allow
      "ida-orig_find_bytes": allow
      "ida-orig_find_insns": allow
      "ida-orig_find": allow
      "ida-orig_basic_blocks": allow
      "ida-orig_infer_types": allow
      "ida-orig_export_funcs": allow
      "ida-orig_callgraph": allow
      "ida-orig_analyze_funcs": allow
      "ida-orig_int_convert": allow
      "ida-reimpl_idalib_open": allow
      "ida-reimpl_idalib_switch": allow
      "ida-reimpl_idalib_current": allow
      "ida-reimpl_idalib_list": allow
      "ida-reimpl_idalib_unbind": allow
      "ida-reimpl_idalib_health": allow
      "ida-reimpl_idalib_warmup": allow
      "ida-reimpl_lookup_funcs": allow
      "ida-reimpl_list_funcs": allow
      "ida-reimpl_list_globals": allow
      "ida-reimpl_imports": allow
      "ida-reimpl_decompile": allow
      "ida-reimpl_disasm": allow
      "ida-reimpl_xrefs_to": allow
      "ida-reimpl_xrefs_to_field": allow
      "ida-reimpl_callees": allow
      "ida-reimpl_get_bytes": allow
      "ida-reimpl_get_int": allow
      "ida-reimpl_get_string": allow
      "ida-reimpl_get_global_value": allow
      "ida-reimpl_stack_frame": allow
      "ida-reimpl_read_struct": allow
      "ida-reimpl_search_structs": allow
      "ida-reimpl_find_regex": allow
      "ida-reimpl_find_bytes": allow
      "ida-reimpl_find_insns": allow
      "ida-reimpl_find": allow
      "ida-reimpl_basic_blocks": allow
      "ida-reimpl_infer_types": allow
      "ida-reimpl_export_funcs": allow
      "ida-reimpl_callgraph": allow
      "ida-reimpl_analyze_funcs": allow
      "ida-reimpl_int_convert": allow
      "math-mcp_*": allow
  '';
in {
  opencode.agents.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}

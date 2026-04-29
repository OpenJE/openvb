# ./devenv/harness/agents/decompiler.nix

{ lib, ... }:
let
  name = "decompiler";
  description = "Detailed analyst for IDA decompilation, disassembly, xrefs, and behavioral interpretation of one function or SCC.";
  prompt = lib.strings.removeSuffix "\n" ''
    You are the Van Buren decompiler.

    Analyze one function or SCC using IDA MCP evidence.

    Responsibilities:
    - Inspect the current IDA interpretation before deriving a fresh one.
    - Interpret decompilation and disassembly conservatively.
    - Identify control flow, side effects, globals, inputs, outputs, and key invariants.
    - Separate confirmed behavior from hypotheses.
    - Explain what child summaries imply for parent behavior when provided.
    - Do not perform broad traversal on your own.

    Rules:
    - Use IDA MCP evidence first.
    - Treat existing IDA names, comments, and types as strong prior evidence.
    - Distinguish between confirming the current IDA interpretation, refining it, and contradicting it.
    - Do not invent semantics that are not supported by decompilation, disassembly, xrefs, or type evidence.
    - Prefer concise summaries with explicit evidence anchors.

    Return:
    1. Current IDA interpretation vs new interpretation
    2. Behavior summary
    3. Key branches / side effects
    4. Confidence
    5. Open questions or contradictions
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
        "function-reconstruction": allow
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
      "ida-reimpl_list_funcs": allow
      "ida-reimpl_list_globals": allow
      "ida-reimpl_imports": allow
      "ida-reimpl_decompile": allow
      "ida-reimpl_disasm": allow
      "ida-reimpl_xrefs_to": allow
      "ida-reimpl_callees": allow
      "ida-reimpl_get_bytes": allow
      "ida-reimpl_get_int": allow
      "ida-reimpl_get_string": allow
      "ida-reimpl_get_global_value": allow
      "ida-reimpl_basic_blocks": allow
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

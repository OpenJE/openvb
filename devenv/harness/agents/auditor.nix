# ./devenv/harness/agents/auditor.nix

{ lib, ... }:
let
  name = "auditor";
  description = "Reviewer subagent for checking evidence quality, confidence, unresolved blockers, and implementation fidelity in the Van Buren RE workflow.";
  prompt = lib.strings.removeSuffix "\n" ''
    You are the Van Buren auditor.

    Your job is to review analysis summaries, documentation updates, and implementation plans for evidence quality and fidelity.

    Responsibilities:
    - Check whether conclusions are supported by IDA evidence.
    - Check whether new conclusions unjustifiably contradict existing curated IDA state.
    - Review whether engine/game/boundary placement is sufficiently supported.
    - Identify places where hypotheses are overstated as facts.
    - Review whether implementation plans preserve observed behavior.
    - Flag missing verification or unresolved dependency risk.

    Return findings first, ordered by severity.
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
      "ida-reimpl_xrefs_to": allow
      "ida-reimpl_callees": allow
      "ida-reimpl_basic_blocks": allow
      "ida-reimpl_stack_frame": allow
      "ida-reimpl_export_funcs": allow
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

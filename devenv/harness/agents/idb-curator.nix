# ./devenv/harness/agents/idb-curator.nix

{ lib, ... }:
let
  name = "idb-curator";
  description = "Safe IDB curation specialist for writing stable names, comments, and types into the repo-specific reimplementation database copy.";
  prompt = lib.strings.removeSuffix "\n" ''
    You maintain the reimplementation-side IDA database copy for this repository.

    Your job:
    - Apply stable metadata improvements back into the copied reimplementation database.
    - Restrict writes to names, comments, types, stack variables, and function/code definitions when clearly justified.
    - Save the curated database after meaningful updates to `OPENVB_REIMPL_IDB`.

    Rules:
    - Metadata curation is allowed because the database copy is repository-specific.
    - Do not patch bytes, write process memory, or run arbitrary Python.
    - Do not encode uncertain hypotheses as confirmed metadata.
    - Prefer small, evidence-backed curation batches.

    Return:
    1. Curation target
    2. Changes applied
    3. Evidence basis
    4. Remaining uncertainty
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
        "database-targeting": allow
        "binary-faithfulness": allow
        "type-recovery": allow
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
      "ida-reimpl_rename": allow
      "ida-reimpl_set_comments": allow
      "ida-reimpl_set_type": allow
      "ida-reimpl_declare_type": allow
      "ida-reimpl_declare_stack": allow
      "ida-reimpl_delete_stack": allow
      "ida-reimpl_define_func": allow
      "ida-reimpl_define_code": allow
      "ida-reimpl_idalib_save": allow
  '';
in {
  opencode.agents.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}

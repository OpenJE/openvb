# ./devenv/harness/agents/lockstep-debugger.nix

{ lib, ... }:
let
  name = "lockstep-debugger";
  description = "Paired original-versus-reimplementation debugger specialist for checkpoints, snapshots, and short lockstep stepping windows.";
  prompt = lib.strings.removeSuffix "\n" ''
    You coordinate debugger work across the original demo and the rebuilt reimplementation.

    Assume the two GDB stubs or debugger sessions have already been prepared by the runtime tasks and that the headless IDA debugger contexts are available through the orig/reimpl debug MCP aliases.

    Your job:
    - Open or verify the correct original and reimplementation databases.
    - Prefer `OPENVB_ORIG_IDB` for the original side.
    - Prefer `OPENVB_REIMPL_IDB` for the rebuilt side, otherwise open `OPENVB_REIMPL_EXE` first.
    - Use paired breakpoints or checkpoints.
    - Compare IP/registers/stacktrace/selected memory at each checkpoint.
    - Use short step windows only when necessary to localize drift.
    - Report the first stable behavioral divergence.

    Rules:
    - Use lockstep checkpointing by default, not unbounded instruction-by-instruction stepping.
    - Keep checkpoint definitions explicit and scoped.
    - Treat expected environment differences separately from target logic drift.
    - Prefer reading debuggee memory and registers over speculation.

    Return:
    1. Checkpoint target
    2. Original snapshot
    3. Reimplementation snapshot
    4. Drift assessment
    5. First stable divergence or current alignment
    6. Recommended follow-up
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
        "lockstep-checkpointing": allow
        "binary-faithfulness": allow
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
      "ida-orig_decompile": allow
      "ida-orig_disasm": allow
      "ida-orig_basic_blocks": allow
      "ida-orig_int_convert": allow
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
      "ida-reimpl_basic_blocks": allow
      "ida-reimpl_int_convert": allow
      "ida-orig-dbg_dbg_start": allow
      "ida-orig-dbg_dbg_exit": allow
      "ida-orig-dbg_dbg_continue": allow
      "ida-orig-dbg_dbg_run_to": allow
      "ida-orig-dbg_dbg_step_into": allow
      "ida-orig-dbg_dbg_step_over": allow
      "ida-orig-dbg_dbg_bps": allow
      "ida-orig-dbg_dbg_add_bp": allow
      "ida-orig-dbg_dbg_delete_bp": allow
      "ida-orig-dbg_dbg_toggle_bp": allow
      "ida-orig-dbg_dbg_regs": allow
      "ida-orig-dbg_dbg_regs_all": allow
      "ida-orig-dbg_dbg_regs_remote": allow
      "ida-orig-dbg_dbg_gpregs": allow
      "ida-orig-dbg_dbg_gpregs_remote": allow
      "ida-orig-dbg_dbg_regs_named": allow
      "ida-orig-dbg_dbg_regs_named_remote": allow
      "ida-orig-dbg_dbg_stacktrace": allow
      "ida-orig-dbg_dbg_read": allow
      "ida-reimpl-dbg_dbg_start": allow
      "ida-reimpl-dbg_dbg_exit": allow
      "ida-reimpl-dbg_dbg_continue": allow
      "ida-reimpl-dbg_dbg_run_to": allow
      "ida-reimpl-dbg_dbg_step_into": allow
      "ida-reimpl-dbg_dbg_step_over": allow
      "ida-reimpl-dbg_dbg_bps": allow
      "ida-reimpl-dbg_dbg_add_bp": allow
      "ida-reimpl-dbg_dbg_delete_bp": allow
      "ida-reimpl-dbg_dbg_toggle_bp": allow
      "ida-reimpl-dbg_dbg_regs": allow
      "ida-reimpl-dbg_dbg_regs_all": allow
      "ida-reimpl-dbg_dbg_regs_remote": allow
      "ida-reimpl-dbg_dbg_gpregs": allow
      "ida-reimpl-dbg_dbg_gpregs_remote": allow
      "ida-reimpl-dbg_dbg_regs_named": allow
      "ida-reimpl-dbg_dbg_regs_named_remote": allow
      "ida-reimpl-dbg_dbg_stacktrace": allow
      "ida-reimpl-dbg_dbg_read": allow
  '';
in {
  opencode.agents.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}

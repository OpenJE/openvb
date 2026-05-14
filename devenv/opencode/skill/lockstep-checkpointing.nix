# ./devenv/opencode/skill/lockstep-checkpointing.nix

{ lib, ... }:
let
  name = "lockstep-checkpointing";
  description = "workflow for paired original-vs-reimplementation debugger checkpoints, snapshots, and short lockstep stepping windows.";
  prompt = lib.strings.removeSuffix "\n" ''
    When driving both binaries side by side:
    - Prefer paired checkpoints over unbounded instruction stepping.
    - Use matched breakpoints or symbolic checkpoints first.
    - Compare instruction pointer, general registers, stack trace, and selected memory/global regions.
    - Escalate to short step windows only after checkpoint drift is observed.

    Rules:
    - Keep checkpoints narrow and evidence-backed.
    - Record the first stable drift point rather than narrating every single step.
    - Treat environment noise and unrelated initialization drift separately from target behavior drift.

    opencode-openje integration:
    Before starting lockstep debugging, register both original and reimpl functions via `re_function_register`. Record checkpoint drift findings via `re_worker_submit`. Use `re_edge_add` to capture call relationships discovered during debugging. If drift is confirmed, update function status to `worker_done` with evidence.
    Lockstep checkpointing primarily serves `re-discovery` and `re-analysis` categories.

    ## Agentic Debugging with IDA MCP

    All debugger tools require the `?ext=dbg` extension on the ida-pro-mcp server. Available tool families:
    - Control: dbg_start, dbg_exit, dbg_continue, dbg_run_to, dbg_step_into, dbg_step_over
    - Breakpoints: dbg_bps, dbg_add_bp, dbg_delete_bp, dbg_toggle_bp
    - Registers: dbg_regs, dbg_regs_all, dbg_regs_remote, dbg_gpregs, dbg_gpregs_remote, dbg_regs_named, dbg_regs_named_remote
    - Stack & Memory: dbg_stacktrace, dbg_read, dbg_write

    ### Prerequisites
    Both IDA MCP servers must be running with `?ext=dbg`. The GDB stub processes (ida-orig-gdb, ida-reimpl-gdb) must be ready. Each debug session targets one IDA instance at a time—cannot debug original and reimpl simultaneously through the same MCP server.

    ### Starting a Debug Session
    Call `dbg_start()` on the appropriate MCP server:
    - Original binary: use `ida-original-exe`
    - Reimplementation: use `ida-reimplemented-exe`
    IDA connects to the remote GDB stub and suspends the target process.

    ### Setting Breakpoints
    First locate the target address via `ida-original-exe_decompile(addr)` or `ida-original-exe_disasm(addr)`. Then call `dbg_add_bp(addrs)` to set breakpoints. For paired checkpoints, set matching breakpoints on both IDA instances before running.

    ### Running to Checkpoints
    Use `dbg_run_to(addr)` to advance execution to a known checkpoint. Prefer matched breakpoints over unbounded stepping. Each `dbg_run_to` call halts when the address is reached or a breakpoint triggers.

    ### Comparing State at Checkpoints
    After hitting a breakpoint, compare state side by side between original and reimpl:
    - `dbg_gpregs()` — compare EIP/RIP, EAX/RAX, EBX/RBX, etc.
    - `dbg_stacktrace()` — compare call stack frames and return addresses
    - `dbg_read({regions: [{addr, size}]})` — compare memory regions at the same virtual addresses

    Record mismatches as evidence of drift.

    ### Stepping for Drift Investigation
    After detecting drift, use `dbg_step_into()` or `dbg_step_over()` in SHORT windows (5-10 instructions max). Do not step unboundedly. Record the FIRST stable drift point where behavior diverges and cannot be explained by environment differences.

    ### Recording Findings
    Use `re_worker_submit` with evidence from `dbg_gpregs`, `dbg_stacktrace`, `dbg_read`. Include register values, stack frames, and memory bytes that show the divergence. Use `re_edge_add` to record call relationships discovered during debugging. When drift is confirmed and documented, update function status to `worker_done` with evidence attached.

    ### Ending a Session
    Call `dbg_continue()` to let the process run to completion or next breakpoint. Then `dbg_exit()` to detach. The GDB stub process stays running and can be reconnected to for a new session.

    ### Concurrency
    Only one debug session per IDA instance at a time. Do not attempt simultaneous debugging of original and reimpl through the same MCP server. Coordinate sessions by closing one before opening the other.

    ### Refresh After Rebuild
    When the reimplementation EXE is rebuilt (via `msvc-build`), both the IDA MCP session and the GDB stub become stale. Refresh them with idalib session management — no process restart needed for IDA:

    1. **Close the old IDA session**: `ida-reimplemented-exe_idalib_close(session_id="<old_session_id>")`
    2. **Open the fresh EXE**: `ida-reimplemented-exe_idalib_open(input_path="./build/F3.exe")` — IDA auto-analyzes and creates a new `.i64` database
    3. **Restart the GDB stub**: `devenv processes restart ida-reimpl-gdb` — the stub must restart because `winedbg` loads the binary into memory at startup; there is no hot-reload

    The original side (`ida-orig-*`) never needs refreshing — `F3.exe` never changes. After refresh, call `dbg_start()` on the new session to resume debugging. Any addresses from the old session must be re-acquired via `decompile` or `disasm` since the reimpl IDB is ephemeral and regenerated from scratch.
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    name: ${name}
    description: ${description}
  '';
in {
  opencode.skills.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}

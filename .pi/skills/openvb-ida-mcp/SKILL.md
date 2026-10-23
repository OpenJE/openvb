---
name: openvb-ida-mcp
description: How to use the IDA Pro MCP servers in OpenVB (original F3 reference vs reimplementation), including database paths, target selection, lazily loaded tools, concurrency, and persistent analysis notes. Use for any disassembly, decompilation, xref, type, renaming, debugging, or binary-analysis task.
---

# OpenVB IDA MCP workflow

## Targets and paths

Two headless IDA MCP servers exist (started by `devenv up` / devenv processes):

- `ida_original_f3` (port `$OPENVB_IDA_ORIG_MCP_PORT`, default 8745): the original proprietary
  `F3.exe` demo. Canonical database: `$OPENVB_ORIG_IDB` (normally `./artifacts/F3.exe.i64`),
  executable `$OPENVB_ORIG_EXE`. This is the authoritative reverse-engineering reference.
- `ida_reimplementation_openvb` (port `$OPENVB_IDA_REIMPL_MCP_PORT`, default 8746): the rebuilt
  OpenVB `F3.exe`. Canonical database: `$OPENVB_REIMPL_IDB` (normally `./build/F3.exe.i64`),
  executable `$OPENVB_REIMPL_EXE`. Validation/comparison/debug target only.

Remote-GDB stub ports (for IDA's debugger setup, not MCP ports): original
`$OPENVB_ORIG_GDB_PORT` = **31437**, reimplementation `$OPENVB_REIMPL_GDB_PORT` = **31338**.
The original is **no longer 31337** - ImHex's Network Interface hardcodes that number and cannot
be moved, so the stub moved instead (measured 2026-10-21, harness item 39). Always read the env
var rather than quoting a remembered port; winedbg stubs are single-client, so nothing else may
TCP-probe them first.

The reimplementation database is ephemeral: it is deleted before each MSVC rebuild and regenerated
from the fresh executable. Never rely on it persisting; persist durable findings in the original
IDB or in repo docs (`docs/f3/*`).

## Target discipline

- Before any IDA MCP call, decide explicitly: ORIGINAL_REFERENCE (`ida_original_f3`) or
  REIMPLEMENTATION_UNDER_TEST (`ida_reimplementation_openvb`). Default to ORIGINAL_REFERENCE.
- Never infer original behavior from the reimplementation; it may be incomplete, stubbed, or stale.
- When comparing: inspect `ida_original_f3` first, record the original behavior, then inspect
  `ida_reimplementation_openvb`. Never substitute one server for the other because names or
  addresses look similar.

## Primary tool surface: `ida`

Use the direct `ida` tool (`.pi/extensions/ida.ts`) instead of the lazy-MCP route for IDA work:

1. `ida(target=..., action="list_tools")` once per session - the servers expose ~59 tools; never
   guess names/schemas (args differ from what you expect: `decompile` takes `addr`, `lookup_funcs`
   takes `queries`, `rename` takes `batch`, `py_eval` takes `code`).
2. `ida(target=..., action="call", tool=..., args=...)` or `action="batch", ops=[...]` - up to 25
   sequential ops in one round trip; a batch stops at the first failed op (ordering matters).
3. `ida(target=..., action="save")` - standing-rule save + disk-mtime census (the persistence
   guard autosaves 5s after writes anyway; call this before risky restarts).

`target` is required - the original-vs-reimpl discipline is enforced by the schema, not prose.
Writes through `ida` arm the persistence guard exactly like direct `mcp__ida_*` calls. The
`dbg_*` debugger tools are reachable through `ida` too (`list_tools` shows them); the two servers
can still be lazy-loaded via mcp_tools_search if you prefer per-tool calls, but that path is
deprecated in favor of `ida`. Each `ida` call is adjudicated by pi-verdict as one gray-zone
request (a batch = ONE adjudication, cheaper than N).

## Loading tools lazily (fallback path)

IDA tools are not declared until you load them:

1. `mcp_tools_search("decompile function at address")` (or similar) activates a small relevant
   subset from the connected IDA servers.
2. If nothing matches, retry with different wording or `mcp_tools_enable` with exact
   `mcp__ida_original_f3__<tool>` names; `/lazy_mcp` lists servers, hidden and active counts.
3. Each new `mcp_tools_search`/`mcp_tools_enable` replaces the previous transient set. When you
   switch target or activity, search again for the tools you now need.
4. If IDA servers show as disconnected, the devenv processes are not running (`devenv up`, or
   check `ida-orig-headless` / `ida-reimpl-headless`); use `/mcp` to inspect/reconnect.

## Concurrency and state

- Treat each `.i64` as single-writer: one editing session per database. The MCP server serializes
  requests on its own port, but do not run a second IDA process (GUI, idalib, idat -S) against the
  same `.i64` while the MCP server holds it.
- **The headless IDA MCP servers do NOT auto-save.** Renames/comments/type edits live only in the
  server process until an explicit `ida_loader.save_database(...)` (via `py_eval`). A devenv/ImHex
  restart reloads the on-disk `.i64` and silently rolls back every uncommitted-to-disk write — this
  erased batch-2/batch-3 (223 renames) plus earlier name edits when the 2026-10-21 13:13 restart
  reloaded `artifacts/F3.exe.i64` (found by GoldYak's pre-batch-4 baseline census: T2 read 26.37%
  instead of the registered 23.93%). STANDING RULE: every IDB-writer wave ends with explicit
  `save_database` + an mtime check + one census dump proving disk state == live state; mid-wave
  checkpoint saves after large restore/apply phases. ENFORCED AUTOMATICALLY: the
  `idb-persistence-guard` extension (`.pi/extensions/idb-persistence-guard.ts`) AUTOSAVES: 5s after
  the last IDA-mutating MCP call (and at every settle) it calls `py_eval -> save_database` over the
  server's own MCP endpoint, proves the disk mtime moved, and appends a function/named-count census
  line to `artifacts/idb-guard-census.log` — unsaved waves cannot depend on agent memory. If
  autosave fails (server down, unexpected schema), the agent is NOT allowed to settle quietly: it
  gets bounded follow-up reminders (2 + one final warning) with the failure reason and the manual
  save+census steps. `/idb_guard` status, `/idb_guard save [server]` forces, `/idb_guard ack <server>`
  clears only after you independently verified disk state. Never use the ack to skip the proof. GOLD STANDARD (proven post-T-055 re-up): the
  real test is names present on a FRESH server after process restart — save mtime + fresh-dump match
  + re-query the respawned server at 3 known EAs; "save returned True" alone is the exact error that
  erased batch-2/3. Ledgers with per-function name+EA tables are
  the recovery source — keep them committed, and never reconstruct lost writes from memory alone.
- The two servers hold different databases, so original and reimplementation can be analyzed
  concurrently; debugger sessions are per-server (one active debugger per server, `dbg_start` ...
  `dbg_exit`), and the two targets debug independently.
- Do not use `idalib_switch`/`idalib_open`/`idalib_unbind` to retarget a server; each server is
  pinned to its canonical database. Refresh the reimplementation after a rebuild only via its own
  documented close/open flow, and never refresh the original after reimplementation builds.
- Persist analysis deliberately: renaming functions/globals, comments, and setting types in the
  database are how findings survive across turns without keeping everything in context. Make
  meaningful edits; do not scatter speculative names. Prefer editing `ida_original_f3` for
  durable RE metadata (the reimpl DB disappears on rebuild).

## Companion local data

For quick structural lookups without IDA, prefer the native Pi tools (`f3_class_index`,
`f3_class_info`, `f3_member_index`, `f3_method_index`, `f3_vftable_index`, `f3_address_lookup`)
over spinning up IDA queries.

---
name: openvb-reimpl-workflow
description: OpenVB build/run/compare loop for the F3 reimplementation - msvc-build, f3-reimpl-run, lockstep debugging, cppcheck, and where the durable project docs and shared RE skill files live. Use when building, running, testing, or comparing the reimplementation against the original F3 demo.
---

# OpenVB reimplementation workflow

## Build and run (all are devenv scripts on `$PATH`; `--help`/behavior via reading `devenv/scripts/*.nix`)

Session start: run the `doctor` tool (`/doctor`) once - one table covering IDA servers,
original IDB, ledgers, ghidra home, wine-prefix triad (dxvk verdict, userdirs, staged rht),
and reimpl IDB freshness; `[FAIL]` rows carry the fix command.

- `msvc-build` / `msvc-build-debug` / `msvc-build-tracing`: build `build/F3.exe` with MSVC inside
  the Docker image (`$MSVC_IMAGE`); needs docker (rootless ok).
- `f3-reimpl-run` / `f3-reimpl-run-debug` / `f3-reimpl-run-trace`: run the rebuilt executable under
  Wine (`$OPENVB_REIMPL_EXE`).
- `f3demo-run` / `f3demo-run-debug`: run the original F3 demo (`$OPENVB_ORIG_EXE`) under Wine.
- `f3-lockstep-debug`: paired lockstep debugging (original GDB stub port `$OPENVB_ORIG_GDB_PORT`,
  reimplementation `$OPENVB_REIMPL_GDB_PORT`).
- `cppcheck-all`, `cppcheck-openvb`, `cppcheck-openje`, `cppcheck-tracing`: static analysis.

## Wine prefix state (three ways this repo's boot lies, and the tools that prove it)

- Prefixes live in `.devenv/state/{f3-reimpl,f3demo}-wineprefix`; they are disposable and DO get
  recreated, which silently resets all three things below at once.
- Save folder: a fresh `wineboot` symlinks `drive_c/users/$USER/{Documents,Desktop,...}` to `$HOME`;
  if the host dir is missing the link dangles and `F3::SetupSaveDirectory` fatals with
  "Unable to create save folder." `f3-wine-userdirs-fix [prefix]` heals it (only `-L && ! -e` links).
  All `f3-*-run*`/`lockstep`/`run-all-tests` scripts call it already.
- Data: `JE::ResourceSystem::Init` opens `<cwd>\resource.rht`, so a run script MUST stage the demo
  root and `cd` into `.devenv/state/f3-reimpl-data` or it fatals "Couldn't open hashtable file".
  `f3-reimpl-run-data` and `f3-reimpl-run-trace` do this; `f3-reimpl-run` does not (bare variant).
- Render: `f3-wine-dxvk status|arm|disarm|ensure [prefix]` classifies the d3d8 path **by hash** against
  `$OPENVB_DXVK_X32` (declared in `devenv/environment/openvb.nix`) and reports ARMED / INERT /
  IMPOSTOR / CLEAN / PARTIAL. A dll merely *named* d3d8.dll in system32 proves nothing - the prefix
  shipped Wine's own builtins there for a day and every "DXVK" measurement was wined3d (harness
  item 68). Arm = real DXVK device, magenta frame on the wall. `ensure` (added 2026-10-23) is the
  machine-portable self-heal: it re-arms from THIS host's declared source whenever the verdict is
  not ARMED, and every wine-booting script (f3-*-run*, f3demo-run*, lockstep) now calls it - so a
  prefix armed by the other laptop, or orphaned by a nixpkgs/lock bump, repairs itself at next boot
  instead of rendering silently through the Wine builtin.
- devenv scripts never hot-reload into a live shell: `devenv-reload --check NAME` then
  `source /tmp/openvb-dev-env.sh` (harness item 1).
- `run-all-tests` MUST be the devenv-env resolution. A shell that never sourced the captured env
  falls back to the base-PATH shim from an OLDER devenv build, whose suite table predates the
  current dependency rows - observed 2026-10-24: it compiled 9 of the ResourceSystem row's 39 TUs
  and reported false LNK2019 reds with the suite's own symbols missing. Symptom signature: the
  link only saw the row's FIRST handful of impl TUs plus `$SRC`. The runner's own stale-payload
  guard ("WARNING the installed run-all-tests is STALE vs tree", it names the payload store path)
  is the tell; verify with `command -v run-all-tests` before and after sourcing the env and trust
  the one the devenv env provides.
- Standard CMake layout in `build/`; `cmake.toml` + `cmkr` generate the CMake files.

After a reimplementation rebuild, `$OPENVB_REIMPL_IDB` is deleted/regenerated; refresh only the
`ida_reimplementation_openvb` server when comparing against a fresh build (see the
`openvb-ida-mcp` skill). Never treat reimplementation behavior as proof of original behavior.

## Comparison ladder (escalate only when needed)

1. Surface: `f3_class_index` / `f3_class_info` / `f3_member_index` / `f3_method_index` /
   `f3_vftable_index` / `f3_address_lookup` on the F3 object database (fast, no IDA process).
2. Structure/types: IDA `ida_original_f3` via lazily loaded MCP tools.
2b. Cross-decompiler check: the `ghidra` tool (pi-ghidra) on the SAME function in original and
    reimplementation, then text-diff the two pseudo-code outputs (bash `diff -u` on saved files).
    Agreement across Hex-Rays + Ghidra is much stronger evidence than either alone; when they
    disagree, suspect analysis state first (run a fresh analysis, check function boundaries).
    First use may need `/ghidra-setup <ghidra-dir>`; inside devenv the binaries are on PATH
    (`ghidra`, `analyzeHeadless` -> nixpkgs ghidra 12.x with bundled JDK).
3. Pseudocode/disassembly: `decompile`/`disasm` on the original; then compare the reimplementation.
4. Runtime: lockstep checkpoints / debugger sessions on the respective targets.

Surface-coverage metric (audited loop target, NOT a fidelity proof): `python3 tools/reimpl-coverage.py`
prints the % of demangled F3.json classes that exist as declarations in src/. Current baseline 14.14%.

## Durable knowledge

- `docs/f3/*` and `docs/f3.md` are the durable project analysis docs; update them when findings
  are durable. Database-side persistence (renames/comments/types) belongs in the original IDB.
- EA->name recovery ledgers land automatically in `artifacts/ledgers/*.jsonl` (guard autosave and
  `ida action=save`); COMMIT them - they are the recovery source after any restart/rollback.
- Next-target selection: `python3 tools/reimpl-coverage.py --report N` ranks uncovered original
  classes by evidence weight (member usages + methods) from the frozen F3.json.
- Shared RE guidance files used by the OpenCode harness are readable on disk under
  `.opencode/skills/*/SKILL.md` (comparison-ladder, binary-faithfulness, type-recovery,
  lockstep-checkpointing, etc.). Read the relevant one on demand; do not edit them from Pi.
- `STYLE.md` governs C++ style; `.clang-tidy`/`cppcheck-*` enforce parts of it.

## Parallel agents (implementer-A/B + discovery) — pi-messenger + pi-intercom (protocol v3)

Session start, in order: (1) `pi_messenger` `status`/`list`/`feed` — the mesh is the live
channel (auto-registered via `.pi/pi-messenger.json`, `scopeToFolder` so only this repo's agents
show); (2) `tasks/README.md` board — claim by committing the file edit
(`assignee`/`status: in-progress`) **and** `pi_messenger({action:"reserve", paths:[...]})` the
files you are about to write, before you write them; (3) respect lane ownership in the board's
table — a reservation is a claim inside your lane, not a claim on someone else's.
`intercom({action:"ask"})` when you need a peer's answer *inside this turn* (it blocks and
returns the reply as the tool result); `pi_messenger` `send` for notifications.
`chat/from-A.md` + `chat/from-B.md` are the frozen archive of the old `C-<n>` threads: read on
request, do not start new threads there (protocol: `chat/README.md`).

The discovery agent stays file-based on purpose (their output is committed evidence, and they
may not run pi): poll `discoveries/` with `ls -t discoveries | head` every session, treat it as
evidence-backed but verify claims that gate your edits, and append replies only to
`discoveries/RESPONSES-from-reimpl-agent.md`, reference documents by filename (session numbers
collide). IDA MCP is shared: read-only unless a task grants rename rights for your lane; announce
renames on the mesh. End every task with both repos clean (submodule first, parent pointer
second). Launch with `PI_AGENT_NAME=<role> pi --name <role>` so `send to:` / `ask to:` stay
stable across restarts.

Memory (recall) runs through Engram: `mem_save` a verified conclusion/gotcha with a
keyword-dense title and a citation to the `discoveries/` file + address; `mem_search` before
re-deriving one. Retrieval is BM25/FTS and `match_mode:"all"` is AND, so query with keywords or
use `match_mode:"any"` for recall. Binary + `ENGRAM_DATA_DIR` come from devenv (shared store
between agents); `.engram/config.json` is tracked, `.engram/chunks/` is not. Evidence still
lands in `discoveries/` — a memory cites it, it never replaces it.

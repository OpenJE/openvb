# Harness operator guide (pi RE harness — openvb)

Start here every session: `/doctor` (or "run doctor") — 9 read-only checks; [FAIL] rows carry the fix command.

## Comparison ladder
1. `f3_class_index|f3_class_info|f3_member_index|f3_method_index|f3_vftable_index|f3_address_lookup` — frozen OOAnalyzer DB (2024-04-04; staleness vs live IDB is by design; f3-json-check guards provenance).
2. `ida` tool — ONE tool, REQUIRED `target: original|reimpl` (original = authority; reimpl = validation only, never infer original behavior from it).
   - `ida(target=original, action=health)` then `action=list_tools` once per session — real schemas (decompile→addr, lookup_funcs→queries, rename→batch, py_eval→code). Never guess.
   - `action=call` single op; `action=batch` ≤25 ops, stops at first failure (ordering matters).
   - `action=save` explicit save+census (normally unnecessary — see guard below).
2b. Cross-decompiler: same function via `ghidra` tool + `ida` decompile, `diff -u` the pseudo-code. Agreement = strong evidence; disagreement = suspect analysis state first.
3. Runtime: `f3-reimpl-run-data`, `f3-lockstep-debug`, `f3demo-run` — all call `f3-wine-dxvk ensure` at boot: prefix self-rearms from THIS host's dxvk (covers laptop-to-laptop drift and nixpkgs bumps). Verdict line goes to stderr.

## Persistence (automatic — know it exists, then forget it)
- idb-persistence-guard: 5s after any IDA-mutating wave goes quiet (and at settle) it saves via py_eval→save_database, proves the .i64 mtime moved, and writes an EA→name ledger to artifacts/ledgers/*.jsonl + census line to artifacts/idb-guard-census.log. COMMIT these — they are the recovery source (see batch-2/3).
- If autosave can't run, the agent gets bounded follow-up reminders with the failure reason; `/idb_guard [save|ack <server>]`.

## Supervised work (GLLA)
- `/goal "<objective; done when ...>"` — completion is a CLAIM audited by a detached verifier; chat "done" means nothing.
- `/list "<obj>", ...` — audited queue; activate/order freely.
- `/loop start "..." measure="python3 tools/reimpl-coverage.py 2>/dev/null" direction=max` — metric loop over surface coverage (baseline 14.14%).
- `python3 tools/reimpl-coverage.py --report N` — next-target queue ranked by evidence weight (member usages).

## Delegation
- subagent tool: bounded fan-out (Jev picks the model per task; needs jevRouting apiKey in ~/.pi/subagent.json). Explore/review = read-only → safe to run many against the shared IDA servers (servers serialize). Writes need worktrees; IDB writes stay lane-owned.
- Parallel sessions (pi-messenger/intercom, protocol v3): lane ownership, board claims by commit, rename announcements. Sessions own days; subagents own minutes.

## Models (enabledModels scope = /model picker + cycling; --list-models shows full catalog by design)
- default: opencode-go/glm-5.3-flash (agentic workhorse). qwen3.8-flash = parallel/cache lane.
- bulk: deepseek-v4-flash · huge inputs: gpt-6-luna (1.05M, $0.10 in) · vision: claude-haiku-5-5 · escalation: deepseek-v4-pro · audit ceiling: claude-sonnet-5-5 · free: deepseek-v4.1-flash-free, minimax-m3.1-flash-free · local GPU: battlemage/qwen38-flash-next, dsv4-flash.
- /jev <prompt> routes one prompt (tiers above); jev_decide for typed calls; pi-verdict gray-zone classifier = opencode/jev-1.13 (resolves via registry — classifiers are not valid enabledModels patterns).
- Manually selecting an out-of-scope model auto-adds it to enabledModels (self-healing).

## Gating (pi-verdict)
Deterministic deny floor (sudo, force-push, curl|sh, ...) → user rules (~/.pi/agent/config/pi-verdict.json — EDIT OUTSIDE PI; the gate self-protects) → Jev classifier. ignoreTools covers f3_*/mem_*/mcp_tools_*/mesh. Uncovered custom tools (ida, ghidra, doctor, subagent) are classifier-adjudicated; a batch = ONE adjudication.

## Two hosts
Everything under .pi/extensions, .pi/skills, .pi/mcp.json, .pi/settings.json + devenv/ is tracked: pull → devenv reload → pi /reload. Credentials (auth.json, ~/.pi/subagent.json, pi-verdict config, jev-model-router.json) are machine-local.

## Memory
mem_search before re-deriving a conclusion; mem_save after fixes/decisions/discoveries; evidence lands in discoveries/ + docs/f3 (memory cites it, never replaces it).

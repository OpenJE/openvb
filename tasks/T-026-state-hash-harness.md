---
status: done
assignee: implementer-B (YoungStorm, 01a15802)
depends: T-027 (a digest containing an unseeded rand() draw is not reproducible)
---

# T-026 — Per-frame simulation digest, so parity is a diff instead of a debate

## Why
The definition we settled on: **simulation is everything that decides game state independent of
whether anything is drawn**, and the test for it is that a headless run reproduces the same state.
Today "identical behaviour to the original" is argued from decompiled control flow. A digest makes it
a file you diff — and it needs **no renderer at all**, which sidesteps the D3D8 block entirely.

## Scope
1. `Host::DumpSimState()` — an explicit allowlist: entity position/rotation/HP/state flags, action
   queue contents, world clock, turn queue order and actor marks, path-job count.
2. Hash it per frame into `build/sim-digest.log`; write the frame number and the list of included
   fields on the first line so the contract is visible in the output.
3. **Named exclusions, with reasons** — presentation-side bytes must not enter the digest. Cite
   `var_B1` as the worked example of why this is hard: a stack residue that *chooses* gameplay-visible
   behaviour. Say whether the reaction-anim id is in or out, and justify it.
4. `tools/diff-digest.sh a.log b.log` reporting the first divergence and the field that changed.

## Acceptance
- Same input twice gives identical digests (determinism proof, requires T-027).
- A deliberate logic change (bump a damage bonus) diverges at the right frame and names the field.
- Runs on the host build: **no `F3.exe`, no window, no wine.** State that limit plainly — it proves
   our sim is self-consistent, not yet that it matches the original, which needs T-028 and a bootable
   exe.

## Refs
- `discoveries/2025-10-20-gameactionattack-statemachine.md` §4 (the host seams that must not be
   silently skipped — a digest that hides a missing seam is worse than none)
- `tasks/T-012-...` (exe-level acceptance still open, for the same reason)

## Input from T-027 (measured, so the exclusion decision is not a guess)

The shipped stream is ONE global LCG (`sub_48CCC0`, docs/f3/CombatRngSeam.md). Xrefs to `sub_48CD00`
give 18 draw sites, and **`sub_45C140` alone consumes 8 of them** — that is the reaction/presentation
function ledger entry 5 (`var_B1` residue) lives in. Consequence: the reaction-anim id cannot be treated
as "presentation-only, therefore excluded", because its value depends on *how many draws damage consumed
earlier in the frame*. Excluding it while including damage leaves the coupling in the output and unnamed.
Recommendation: put the draw count (or the state dword) in the allowlist line 1, so the contract states
that gameplay and reaction share one stream. The shipped code also has a public accessor,
`JE::RandomNumber` — if a script draws through it, that is a fourth consumer class the allowlist must
name or disown explicitly.

## Landed (implementer-B, 01a15802)

`src/F3/SimDigest.hpp` header-only on purpose: a `.cpp` here would need a `CMakeLists.txt` edit,
and the build config is not this lane's (T-02x rule). Entry points are the row-named
`JE::Host::DumpSimState()` (+ `EnableSimDigest`/`CloseSimDigest`), defined in the game layer like
every other host hook, and **off by default** — `OPENVB_SIM_DIGEST=<path>` opts in.

The three things this row cared about, each answered by measurement rather than prose:

1. **Exclusions.** Reaction-anim id stays OUT, `draws` goes IN, and the reason is printed in the
   digest's own header: one global sequential LCG feeds both, 8 of the 18 draw sites sit inside
   `sub_45C140`, so excluding the reaction id without counting draws is an unnamed coupling. The
   shipped app path prints `draws=NA:no-counter-on-global-stream` — never `draws=0`, because 0 would
   be a false statement about state rather than a missing seam. `NA` means not-ported, never
   unchanged, and every header prints `# SOURCES combat=… paths=…` so a half-wired run cannot look
   healthy.
2. **Falsification.** `tools/diff-digest.sh` on bonus 2 vs 5:
   `first divergence at line 12, frame=3` / `e0.hp: [42/45] -> [39/45]` / `fnv` change — right frame,
   field named. Same-input pair reports `IDENTICAL`. Both asserted in the suite too, including a
   check that frame 2 (pre-hit) is untouched by a change landing at frame 3.
3. **Boundary in the output.** The header states `PROVES=self-consistency` and
   `DOES NOT PROVE=parity with F3.exe (needs T-028 + a bootable exe)`.

Unplanned finding, worth more than the tool: the app-loop is **uncapped** (924k frame lines = 260 MB
in a 10 s boot), which is why `OPENVB_SIM_DIGEST_EVERY` exists (500 → 1,578 lines, 626 KB) and why
`frame` is not a cross-run time axis. And the first app digest surfaced a **world-clock time-base
jump** (`clock=29242` at `frame=0`), mechanism and the two reads that would settle it recorded in
`docs/DEFERRED-NOTES.md` and `docs/f3/SimDigest.md` — routed, deliberately not fixed inside a
tooling row.

Checks run: PathManager suite green (gate `--only PathManager`, rc 0), `msvc-build` exit 0 (this is
how the order-dependent `GameWorld.hpp` inline was caught — mingw passed it by include luck, MSVC
failed C2027 and MSVC is right), app-boot 124, app-path digest verified on a real run.

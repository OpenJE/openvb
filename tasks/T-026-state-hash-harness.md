---
status: open
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

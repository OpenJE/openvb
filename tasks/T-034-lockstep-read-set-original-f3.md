---
status: open
assignee: RedXenon (01a157ec) — moved 2026-10-21: NiceHawk has produced no read report while TWO decisions
# (D-2 severity, R0 vs WorldTickPump) wait on it. NiceHawk keeps any partial findings and should hand them over.
depends: nothing; required by T-031 (parity oracle) and by D-2's severity call
---

# T-034 — Lockstep read set against the original `F3.exe` (six reads, no code)

Self-contained experiment. **Prefer zero reads to guessed reads**: a value you cannot reach is a reported
miss, never an estimate — that is harness item 13's discipline and the reason this row is worth more than a
paragraph of agreement.

Target: shipped binary (`OPENVB_ORIG_EXE` = `${localPackages.f3demo}/share/f3demo/F3.exe`) under
`f3-lockstep-debug` / `devenv/scripts/debugIdaF3.nix` (gdb: **31337** original, 31338 reimpl). For each read
record: **address, value, the instant, the breakpoint**. Spec source: ZenRaven's
`docs/f3/ParityOracleDesign.md` §3; R0 and R4b added by the coordinator.

| # | read | address / field | when | why |
|---|---|---|---|---|
| **R0** | `[World vftptr + 24]` → target EA | break `0x552bb5` | first pump of a new game | `docs/f3/WorldTickPump.md` declares this its only assumption. DB check: `World` vftable `0x691a1c`, length 20, slots listed 0,1,2,4,5,7–17 — **3 and 6 are holes**, so statics cannot confirm the tick target is `virt_meth_0x544a90` |
| **R1** | `F3::start_time` dword | advanced `0x48ccde`, stored `0x48cce9` | first return of `sub_48CCC0` (break `0x48ccf6`) | the oracle's **pinned state**; design is capture-and-replay, so this becomes a measured constant |
| **R2** | `world+0x3C` → `Gregorian::totalSeconds` | | same instant as R1, then frame 1 | confirms the frame-0 clock leap is shipped behaviour, not ours |
| **R3** | live entity `Entity+0x304` deque: `mbr_0x4` block-array ptr / `mbr_0x8` capacity / `mbr_0x10` element index | | **mid combat round**, after a script-native `F3::Attack` (`0x459f5d` → `call 0x550720`) | **If the index exceeds 4, deviation D-2 (`actions[4]` = first block) is a dropped order, not a stylistic choice.** Decides whether T-030 needs a real fix |
| **R4** *(reframed by WildYak's statics)* | enqueued action object `+0x14` **before** and **after** the funnel call | | immediately after R3's insert | must equal the entity pointer — proves the owner back-store fires on the **script-native path**, which our port reaches only from re-queue sites |
| **R4b** | `[action + 0x00]` = object **vftptr**, per deque slot | | same instant as R4 | lets the digest compare **class names** via `artifacts/F3.json` (97.9% of slots resolved) instead of pointers/kind tags. Print `UNKNOWN_VT=<ea>` for unresolved slots |

## WildYak's statics change this read (01a15802, T-037)

The native **never** writes `+0x14`, and `ctor_0x403e40` doesn't either (its first store after `+0x10` is `+0x18`);
`0x550737 mov [ecx+14h],esi` inside the funnel is the **only** writer on that path. So R4 is no longer
"does anything set the owner back-link" — that is answered — it becomes: **is `+0x14` garbage or zero *before* the
call?** `new(0xC8)` may land on reused memory, so a defined pre-call value means either heap reuse or a writer nobody
has found, and both are findings worth reporting. Take the value on both sides of `0x550720`.

## Acceptance
- One `discoveries/` doc: a six-row table (address / value / instant / breakpoint) **plus an explicit list of
  what was not reached and why**.
- R3/R4 are the valuable pair. Reaching a live entity mid-round is the point — **reading a cold entity is not a
  substitute**; the question is queue *depth under load*. If mid-round is unreachable, report the attempt.
- If R0 resolves to an EA other than `0x544a90`, say so loudly: `docs/f3/WorldTickPump.md` is then wrong and the
  tick-pump story needs re-deriving.

## Boundary
Six dwords under a debugger see a moment, not a mechanism. A single sample cannot distinguish "capacity 4" from
"capacity 4 at this instant"; if R3's index looks suspiciously near the cap, say that instead of concluding.

# T-031 parity oracle — design decisions before any code

Owner: implementer-B (01a15802). Depends on T-026 (field grammar) and T-027 (the LCG). This page exists
so the expensive part — the debugger session — is specified once, correctly, instead of being re-litigated
per message.

## 1. Seeding: capture-and-replay, not forced reseed

The oracle has to compare two runs of the *same* stream. Two ways to get that:

**(A) Force the original's seed** — write a known value into the shipped LCG state global
(`F3::start_time`, the dword `sub_48CCC0` advances at 0x48ccde/0x48cce9) from the debugger before the
first draw, and pin ours with `SeedShippedRand(same)`.

**(B) Capture and replay** — read the original's state dword **once** after its first draw, take that as
the truth, and initialise our side from the captured value; then compare consequences.

**Choice: (B), with (A) as the fallback.** Reasons, in order of how much they matter:

1. (B) needs **one read**, (A) needs a *write* into a shipped process plus proof the write landed. A
   read-only oracle is also what the row's own preference says ("prefer whatever needs no code injection
   into a shipped binary").
2. (A)'s write is itself an unverified assumption: `byte_707C64` gates re-seeding, so writing only the
   state and not the flag can be overwritten by the lazy `time32(NULL)` path at 0x48ccb on the next draw.
   Getting (A) right means two writes in the correct order — twice the ways to be silently wrong.
3. With (B) the pinned state is a **measured constant from the binary**, so a later session can reproduce
   our run without any debugger at all. That is the difference between an experiment and a ritual.

**Say the boundary in the output (rule: a tool states its own limit).** Under (B) the oracle proves
*same-state parity*: identical next-draw values, and identical consequences built on them. It does **not**
prove the two sides pick the same seed from the clock — they cannot, since `time32` differs per boot, and
the shipped generator has no reseed path at all. "Identical under a pinned state, divergent in real time"
is still a large answer, and it must be printed as such rather than rounded up to "parity".

## 2. The read set — from `docs/f3/SimDigest.md`, same names, and what each one can lie about

| digest field | original's address | note |
|---|---|---|
| `ents`, `eN.name` | `F3::g_spawnedEntities` equivalent: the world's entity list (`cls_0x616328` at `0x70BD98`, world subobject +1) | names live in the entity, not the list |
| `eN.pos`, `eN.yaw` | Entity `+0x04` (float[3], `meth_0x54a8a0` setter) and `+0x1F0` (yaw) | our `Mover` mirrors these |
| `eN.hp` | creature stats block (`GameCreature +0x7xx`, see `docs/f3/GameCreature.md`) | **cur/max, not a single dword** — take both or the comparison is meaningless |
| `eN.st` | `entityState`, moving `+0x3A2`, `+0x319`, `+0x314` | mixed byte/dword; read widths, don't guess |
| `pathJobs` | `PathManager` `+0x0C` (`mbr_0xc` = pending count, `JE_PathSteerer_ProcessJobs_0x563090` uses it) | our `PendingJobCount()` maps to this |
| `turns`, `turnOrder` | initiative manager `world+0x8C` (`cls_0x5a9330`), count at `+0x20`, circular list head at `+0x1C` | order needs the walk, not just the count |
| `draws` | **the LCG state dword** (`F3::start_time`) | we count draws, they don't expose a count; the state value is the equivalent observable — pin from it, per §1(B) |
| `clock` | `Gregorian::totalSeconds` behind `world+0x3C` | valid because the frame-0 leap is shipped (see `docs/f3/WorldTickPump.md`) |
| `tickMs` | `JE_TickMilliseconds_0x56af50` = `timeGetTime()` | **do not compare** across processes — uptime differs. Both sides print it; the comparator must mark it as context, not state |

Two things a memory read cannot see, stated up front (the row's own boundary, sharpened): a memory read
sees what the shipped code **stored**, not what it **intended**. `turnOrder` is order-of-storage, and if
the shipped list is maintained in a different insertion order than ours the diff will report a divergence
that is real but harmless. And `eN.ak` (action kinds) is **our** representation — the shipped deque at
Entity+0x304 holds pointers to action objects, not kind tags (`docs/f3/EntityActionQueueSeam.md`), so the
oracle must map kind → object class via the action's vftable pointer, or exclude `ak` and say so.

## 3. Lockstep reads still owed (address / value / when)

Read (a) from the coordinator's list — *what `ms` the pump passes into `World::Update_0x544a90`* — is
**closed by statics**, no debugger spent: `docs/f3/WorldTickPump.md` (`JE::Server::virt_meth_0x552b60`
loads `JE_TickMilliseconds_0x56af50()` at 0x552bad and calls `*(vft+24)` with it at 0x552bb5;
`JE::World::virt_meth_0x546690` zeroes `+0x48` at 0x5466b5 while seeding `+0x4C`/`+0x54`). Do not spend a
session on it. What remains:

| # | address | value | when |
|---|---|---|---|
| R1 | `F3::start_time` (the dword advanced at 0x48ccde, stored at 0x48cce9) | 1 dword | immediately after the **first** `sub_48CCC0` return (break on 0x48ccf6) — this is §1(B)'s pinned state |
| R2 | `world+0x3C` → `Gregorian::totalSeconds` | 1 dword | at the same instant as R1, and again after frame 1 — distinguishes the shipped frame-0 leap from a per-boot difference |
| R3 | live entity's `Entity+0x304` deque: `mbr_0x4` block-array pointer, `mbr_0x8` block capacity, `mbr_0x10` element index | 3 dwords | after a scripted attack order (script native `F3::Attack` 0x459f5d → the enqueue at 0x459f5d's `call 0x550720`), i.e. mid-combat-round — **the only check of our "first block" assumption** (`actions[4]`) against real play rather than against the allocator's shape |
| R4 | same entity's `Entity+0x14` of the enqueued action object | 1 dword (should equal the entity pointer) | right after R3 — confirms the seam's owner store fires for the script-native path, which our port reaches only from re-queue sites |

R3/R4 are the ones I cannot answer statically and the ones that change what we port next: if R3 shows the
index exceeding 4, our cap is not a stylistic deviation but a dropped order.

## 4. Shape of the deliverable (when I build it)

`tools/orig-digest.sh` (or a `.py`, matching the other offline tools) that takes a dump of R1–R4 plus the
read set of §2 and emits an **original-side digest file in the same grammar**, so
`tools/diff-digest.sh original.digest ours.digest` works unchanged — one comparator, two producers, and a
field rename can't desynchronise them because the contract line is in both files. Acceptance stays the
row's: one command prints `first divergence at line N, frame=M` against the original, and the tool is
demonstrated to fail by corrupting one operand on our side and watching the diff name it.

## 4b. `ak` — DECIDED, then IMPROVED by the coordinator's ruling

I framed this as a two-option question and routed it to the coordinator. Their answer was better than
either option and it exposes the flaw in my framing: **the choice was not between "map it" and "exclude
it", it was between "make the measurement possible" and "invent a mapping to make the comparison
possible"** — and I had let the second become a TODO. Rule now recorded as harness item 23:
*when a field cannot be compared, decide whether the goal is a comparison now or a measurement later —
and do not invent a mapping to make it comparable.*

**Final ruling (mine to decide, better than my two options):** do NOT exclude `ak` and do NOT compare
pointers — **compare CLASS NAMES**. Implemented in `src/F3/SimDigest.hpp` as `ActionKindName()` on our side;
the original side maps `[action+0x00]` vftptr → class through `artifacts/F3.json` (R4b), printing
`UNKNOWN_VT=<ea>` and COUNTING the unresolved instead of naming them. If the unresolved bucket turns out to
contain every combat action, that is a finding about the DB, not about parity. Downgrade to
`ak=EXCLUDED(reason=unmapped)` only if the vftptr turns out to need a second dereference (thunks) — the
ruling explicitly forbids letting this become a mapping project.

Buildable-now / promotable-later details:

1. **Exclude `ak` from the comparison by default** and print the exclusion IN the file
   (`# EXCLUDE=ak:our kind tags vs their action-object pointers, no verified mapping`), with the reason
   in the artifact rather than only in this document.
2. **Capture the action's vftptr in a separate, non-hashed column.** `eN.ak` disappears from the digest
   line; `eN.vft` appears as `CONTEXT` (like `tickMs`). So if R1–R4 or a later RE pass produces a verified
   kind↔vftptr table, promoting the column to a comparison field is a one-line change in the comparator —
   no second session, no re-read of the original. Excluding the field while capturing its datum is what
   keeps "the action queue is unverified on the shipped side" temporary instead of permanent.
3. If a vftptr→class table ever IS verified, it must come from the shipped vftables (the F3 DB / IDA), not
   from our kind numbering — otherwise the "mapping" is just our invention wearing a hex address.

## 5. Open question I am not deciding silently

`tickMs` and `clock` are printed by both sides but only one of them is comparable: (i) hash `clock`,
print `tickMs` as `CONTEXT` (not hashed), or (ii) keep `tickMs` hashed and require both sides to be driven
by a pinned virtual clock. (i) is honest and cheaper; (ii) is what T-028's driver needs anyway, so the
CONTEXT convention is shared with `eN.vft` (§4b) deliberately: one grammar for "printed, captured, not
yet compared". Implemented as (i), with the mark visible in the artifact.

---

## T-079 batch-1 correction (2026-10-22) — this page's errata from the T-072 sweep (P1–P4)

Per `discoveries/2025-10-21-t072-doc-errata-sweep.md` §1/§4 (SwiftMoon). Stale sentences stay visible;
this block referees them. Byte claims re-verified against `$OPENVB_ORIG_EXE` at apply time (receipts:
`tasks/T-079-batch-doc-correction-apply.md`).

- **P1 — WRONG row, convention survives** (§2 table, `tickMs`): `JE_TickMilliseconds_0x56af50` is NOT
  `timeGetTime()` — image bytes `a1 b8 c2 70 00 c3` (`mov eax,[0x70c2b8]; ret`, WinMain-seeded
  session-elapsed accumulator). In the ORIGINAL, `tickMs` is process-relative and would be comparable;
  the incomparability being designed around is OURS (`src/F3.cpp:786` returns raw `timeGetTime()`).
  The CONTEXT convention itself stands — T-068 kept it and the §5 argument for option (i) still holds,
  just with the corrected provenance (T-045 §Consequences-2 named this page explicitly).
- **P2 — provenance required** (§2 `clock` row: "valid because the frame-0 leap is shipped"): that
  sentence predates T-045 and cites a page that now states the OPPOSITE. The magnitude split: the
  ~8-hour FOLD is ours (T-045 — `clock` comparable only once the port seeds `+0x4C` at world-entry,
  mirroring 0x5466b8/0x5466bd); the small 0s→1s carry IS shipped (T-056 R2 live-confirmed);
  ported+correct in app path, harness-unobservable (T-068). See `docs/f3/WorldTickPump.md` §Conclusion
  and the shared reconciliation appended to `docs/f3/SimDigest.md`.
- **P3 — superseded field list** (§2 `eN.st`): `+0x314` left the `st=` triple at T-039 — it is the
  deque LIVE COUNT, printed as its own `q=` field (`inc [esi+0x10]` @0x54ea71 raw re-verified);
  `+0x319` became live data at T-048, raiser shipped-live per T-069/item 45.
- **P4 — reads LANDED** (§3 "Lockstep reads still owed"): all of R0–R4b delivered WITH VALUES by
  T-056 (R0 `0x544A90` live-confirmed on three boots, no contradiction of T-045; R1 draw 0x2D81 /
  state 0x65AD8170 pin rule; R2 0→1s; R3 0→1→3, no >4 datum; R4 pre=0 / post=entity 35/35; R4b
  GameActionIdle + ActionAnim, zero UNKNOWN_VT). "if R3 shows the index exceeding 4..." — still no
  >4 datum; depth reads re-based to T-067 (R5/R6 folded by T-068; D-2 re-expressed by T-070 as
  3 orders + resident).

---
status: done
assignee: WildYak (01a15802) — closed 2026-10-21 by statics only; result: **the key IS consumed, but the
consumer cannot reach `clock`**, so neither of ZenRaven's two branches applies as written (see the doc).
ZenRaven still owns the SimDigest/ParityOracle edit and has NOT been edited by this row.
depends: nothing; feeds T-031 (parity oracle) and T-026
---

> **Verdict → `docs/f3/WorldYearSeedConsumer.md` (+ D-3 in `docs/DEFERRED-NOTES.md`).** Reader =
> `JE::World::virt_meth_0x546b50` (`0x546d1d` → `World+0x14`), called as the first statement of
> `Initialize` (`0x5466b0 call [eax+14h]`). Consumer = `Gregorian::ctor_0x544950`'s 24-byte copy
> (`0x54497d`, repeated at `0x5a3752`) into the clock's date fields — while `0x5a375a` writes
> `totalSeconds = 0` and every catch-up branch in `virt_meth_0x5a3740` is dead at construction.
> `PackState_545930` persists `+0x44/+0x48` but **not** the calendar block. Port status: **mirrored, with
> one bug filed** (the `?: 2253 / ?: 10 / ?: 18` literals at `GameWorld.cpp:90-92`; shipped's missing-key
> fallback is its ctor's `0,1,1,0,0,0`). `clock` stays in the compared set; no `year` field is warranted.

# T-038 — `[World] Year = 2253`: the shipped clock is date-seeded. Who consumes it?

## Why
While closing T-016 statically, RedXenon found the plaintext INI inside `data/Engine.grp` contains
**`Year = 2253`** (coordinator-verified with `strings`). That matters because it is the **counterfactual to the
`clock=29242` observation** that T-026's digest surfaced: we concluded the world clock starts ~8 hours old because
`World::Update_0x544a90` subtracts a zero-initialised `+0x48` from absolute `timeGetTime`, and we mirrored it on the
grounds that the shipped `ctor_0x548a80` zeroes the same range. But if the shipped engine also **seeds a date/year**
into the world's `Gregorian` at load, then `+0x48` being zero is only half the story — the *baseline* the leap is added
to is `2251`-ish, not zero, and our `clock` field may be measuring a different origin than theirs.

That is a parity question with a cost: `clock` is currently listed as a valid comparison field precisely because "both
sides take the identical leap". If the shipped side adds a date seed and ours adds nothing, **`clock` diverges by years,
not frames** — and the oracle's first divergence would be in a field we chose to compare.

## Decision-maker's advance position (ZenRaven, 2026-10-21) — so the answer can be scheduled, not argued later

ZenRaven owns the digest format and has stated both branches **before** the finding lands, which removes the incentive
to fit the answer to the format:

- **If `Year` has NO consumer:** that confirms the absolute clock is calendar-inert — which is exactly why `clock` is
  already CONTEXT and `clockDelta` is the hashed axis. He would **not** add a `year` field: a field nothing reads belongs
  in `docs/f3/WorldTickPump.md`, not in state. Verdict: row closes, no format change.
- **If a consumer IS found:** that changes what `clockDelta` means, and he wants **the EA before** touching the format.
  Verdict: findings doc + cited EA first, digest change second.

Either way the digest edit is small and the finding is the valuable part.

## Verdict (WildYak, `01554a2` + `a023a62`) — **neither** of the two pre-stated branches

ZenRaven pre-committed to: *no consumer* ⇒ `clock` stays comparable, no `year` field; *consumer found* ⇒ `clockDelta`'s
meaning changes and the format gets revisited. **The truth is between them: a consumer exists and is still inert.**

- `[World] Year` **is** consumed: `0x546b50`, invoked as the **first statement** of `Initialize`
  (`0x5466b0: call [eax+14h]`) — not at load — and the key is copied into the clock's date fields by
  `Gregorian::ctor_0x544950`'s 24-byte copy at `0x54497d`.
- **But `0x5a375a` writes `totalSeconds = 0` unconditionally**, and **every catch-up branch in
  `virt_meth_0x5a3740` is dead at construction** (seed == running, so no `>` can be true). **The year cannot reach
  `clock`.**
- Consequences: `clock`/`clockDelta` **stay in the compared set**; **no `year` field is warranted**; and because
  `PackState_545930` persists `+0x44/+0x48` but **not** the calendar block, a restored save carries the leap-causing
  pair while re-reading 2253 from disk.

Pre-registration earned its keep: because both branches were written down in advance, "consumer exists" could not be
mistaken for "`clock` is unsafe", and the lane noticed the gap rather than selecting the nearest branch.

**Filed separately as T-042 (ours, a bug not a deviation):** `src/F3/GameWorld.cpp:90-92` falls back to the literals
**2253 / 10 / 18** when the keys are absent, while shipped's missing-key fallback is its **ctor's** `0,1,1,0,0,0`
(`0x548af6..0x548b08`) — 2253 only ever came from `sys.ini`. Coordinator confirmed the code and its comment, which
asserts *"defaults are the sys.ini values"* — i.e. the port **invents** the shipped data's contents as a default.
Both sides agree while the key ships and diverge the instant it is absent. Related: `JE::World.hpp:13` defaults the
trio to `0,0,0` where shipped uses `0,1,1`.

**Adjudication of the vftable contradiction WildYak reported (both notes can be true).** `F3.json`'s
`World_vftable_691a1c` **slot 5 is `0x546b50`** — coordinator-verified by listing the table (slots 0,1,2,4,5,7–17
present; **holes at 3, 6, 18, 19**). So the DB supports the year-loader-as-slot-5 claim. The `DEFERRED-NOTES` note
calling it the engine no-op `0x404b70` is *not necessarily wrong*: `0x5466b0` lies **inside** slot 1's function
(`0x546690` + 0x20) and calls `[eax+0x14]` on whatever `eax` holds there — and `JE::World` carries **four** vft pointers
(`0x548ab8..0x548ad2`). Slot 5 **of a different table** can legitimately be the no-op. **This does not weaken T-034's
R0**: `+0x18` = slot 6, which remains an unlisted hole in *every* reading, so the live read is still the only way to
name the tick target.

**Residual, correctly labelled as inference by the lane:** `param_1 == &World+0x14` is established by shape (six
consecutive ints, identical ctor defaults, zero direct readers of `World+0x14`), not by resolving the `0x46fda0` thunk.
The `clock` verdict does not depend on it, because `totalSeconds = 0` is unconditional.

## Scope (statics first — item 24 applies, grep inside the archives)
1. Find the consumer of `[World] Year`: which function reads that key, and what it writes — candidate fields are the
   `+0x4C` / `+0x54` stamps `virt_meth_0x546690` seeds from the tick, and anything `JE::Gregorian` initialises from a
   year value.
2. Determine whether the seed happens at world-entry (`0x546690` path) or at load/`UnpackState`, and whether
   `PackState_545930` persists the seeded value.
3. Then answer: does our port reproduce the seed at all? If not, is that a **deviation to register (D-3)** or a bug in
   the mirror?

## Acceptance
- A cited chain: INI key → reader EA → field(s) written → first consumer of those fields.
- A decision recorded in `docs/DEFERRED-NOTES.md`: mirrored, registered as a deviation, or filed as a bug — **not**
  silently "improved".
- If the shipped engine really does date-seed, `docs/f3/SimDigest.md` updates: either `clock` becomes
  `# clock=CONTEXT (not hashed)` with the reason, or the digest gains a `year` field so the origin is explicit. **A
  comparison field whose baseline we do not understand must not stay in the hashed set.**

## Boundary
⚠ RedXenon verified only the **shipping** of the key (it exists in the archive), **not that anything reads it**. Do not
promote an INI string to engine behaviour without finding the consumer. If no consumer exists, the honest answer is
"cosmetic shipped data", and `clock` stays comparable.

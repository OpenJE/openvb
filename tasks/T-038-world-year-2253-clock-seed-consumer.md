---
status: open
assignee: ZenRaven (owns SimDigest + the time-base question)
depends: nothing; feeds T-031 (parity oracle) and T-026
---

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

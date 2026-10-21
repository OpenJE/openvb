---
status: done
verdict: shipped data never trips it (0/57)
priority: P3 (small, and it decides whether a ledger entry is observable at all)
assignee: FastArrow (research)
depends: T-015 (the parser and the flag positions are theirs)
type: data-read
---

# T-019 — Does any shipped record actually trip "can't be both consumed and thrown"?

## Why this is worth a sitting

`docs/DEFERRED-NOTES.md` ledger entry 4 is the only defect the shipped binary *announces*:
`GameWeapon::ReadAttackModeData` raises `FatalError("A weapon can't be both consumed and thrown.")`
when the v5 flag pair is both set. The reimpl rule is **crash parity** — reproduce the fatal error,
do not tolerate the combination. But crash parity is untestable in one specific situation: if **no
shipping record ever sets both flags**, we can never observe the path, and any implementation of it
is unverified code that looks verified.

So the question is purely data, and it changes what we can claim:

- **No record trips it** ⇒ the check is dead code in shipped content. We still mirror it (a
  weapon we author, or a mod, can trip it), but we mark it **unobservable in the demo slice** and
  T-012/T-013 must not depend on it.
- **Some record trips it** ⇒ the original would crash loading that weapon, which is a far stronger
  claim: it means either the flag decoding is different from our model, or shipped data is
  inconsistent with the code that reads it. Either outcome invalidates an assumption somewhere and
  is worth finding out before anyone builds on the parser.

## Steps

1. Enumerate weapon records carrying a GWAM/GIWP chunk through the reader you already own
   (`GameWeapon::ReadAttackModeData`), version **v5** only — the pair is v5-gated.
2. For each, read the two flag bytes at the offsets your B9 grammar establishes and report the count
   where both are set.
3. Report the check's own reachability with it: is the `FatalError` site dominated only by the v5
   path, or can a v<5 record reach an equivalent assertion?

## Acceptance

A one-line verdict in the B9 file (or a new section): **N of M shipped v5 weapon records set both
flags**, naming any that do. Plus one sentence on what the answer forbids or permits in the
reimpl. A zero answer is the expected and perfectly good result — say "0, so crash parity is
mirrored but unobservable in this content" and close the row.

## References

- `discoveries/2025-10-20-t015-gwam-record-premap.md` (B9) — flag positions, v5 gate, parser
- `docs/DEFERRED-NOTES.md` shipped-defect ledger, entry 4

## Verdict (FastArrow B11, `c2c5db1`) — 0 of 57, and the earlier 3 candidates were my own artifact

With the corrected operand mapping from the frame pass, **consumed = 0 on all 57 shipped v5 GWAM
records**, so the `FatalError` never fires on shipped content. B10's three candidates are explicitly
retracted as a mapping artifact rather than reinterpreted — the lane published its own falsification.
The verdict is also recorded as **robust to the one remaining thrown-operand ambiguity**, so the
residual slot-table work cannot overturn it.

Consequence for the port, keeping the mirror rule intact: ledger entry 6 stays as shipped behaviour and
should still be implemented **as shipped** — the crash is reachable by any weapon that does trip it
(mods, authored data, or a future record), it is simply **unobservable in this demo slice**. So:
implement, cite, and do not write a test that asserts a trip we cannot produce. T-015's slot table and
creature-twin parity remain open; that is separate work, not this verdict.

## Notes (coordinator)

Chosen for you because you are already warm on exactly these bytes and nobody else can answer it
without re-deriving the grammar. It is deliberately small; do not let it turn into an audit of the
whole item table.

## Finalization (PureKnight 01a15cb7-e27e, T-015 remainder pass, 2025-10-21)

The 0/57 verdict is now **unconditional** and its mapping **corrected**: the guard operands are wire fields
**#16 (payload+54)** and **#17 (payload+55)** — independent u8 reads, bool-normalized at `0x414b42`/`0x414b61`
(weapon) and `0x40ddf`-block (twin); trip iff both != 0. B11 §2's “bytes 2/3 of the #10 dword / 0x4a never
written” reading is refuted by the instruction stream. Corrected census over the same 57 committed chunks:
**(#16,#17) = (0,0) on 57/57 → 0 trips** (the {(0,0)×54,(0,1)×3} histogram was the mis-mapped pair).
The port rule for custom/modded records is now byte-exact. Details: `discoveries/2025-10-21-t015-slottable-twin-parity.md` §2.

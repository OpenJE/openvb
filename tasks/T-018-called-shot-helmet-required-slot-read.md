---
status: done
assignee: PureKnight `01a15cb7` (was IronStorm, departed)
---

# T-018 — One data read: a shipped helmet record's `GameItem+0xA8` required-slot vector

Statics are finished for called-shot naming; this is the last gap. FastArrow's B8 pair table
(`sub_469430`) collapses req{2..5} -> phys2 and reserves phys0/phys1 for req0/req1, so row B is one
requirement class and {5,6} = FEET/WAIST is structurally impossible. Prediction: a helmet's `+0xA8`
vector **contains 1**.

- Contains 1 -> head-family naming stands, T-010c closes.
- Anything else -> the requirement-class reading is wrong somewhere, and that is the more valuable
  answer; report which assumption breaks.

Either result is a deliverable. Do not extend into a general item-table audit.

## Result (PureKnight 01a15cb7-e27e, 2025-10-21)
**CONFIRMS: helmet's +0xA8 required-slot vector = {1}** — stored, not computed. GITM payload stream
(tag+12): `u32 type, u32 x, u32 N, N×i32 values` parsed at `sub_410B80` `0x410beb/0x410c19/0x410c3c` into the
container anchored `GameItem+0xA8` (inserts at `this+0xA8/+0xAC`), consumed by `GameInventory::Equip_46be40`
(snapshot `0x46befa`, iterate→`sub_469430` at `0x46bf33`/`0x46c03f`). Shipped bytes: `0x7c4` (Hea* art, head-worn)
N=1 vec=(1); suits `0x3e4` N=1 vec=(0); 5 texture variants N=0 vec=(). Full read + hex in
`discoveries/2025-10-21-t018-helmet-reqslot-vector-read.md`. Side finding for T-025 owners: gitm.csv
`equip`/`eqslot` columns are actually req-count / reqvec[0] mislabels (schema note §4) — session35's conclusion
survives unchanged because its "eqslot" WAS reqvec[0]. T-010c stays closed.

## Coordinator pin (2026-10-21, before the new owner starts) — non-implication to hold
The row's question is the **`GameItem+0xA8` required-slot vector as it materializes in memory**. The
`_ARM.grp` on-disk scalar equip-slot (`GITM+24`, session 35's read) is NOT known to be the same field:
a scalar equip slot and a slot-*vector* can disagree, and one reading does not settle the other.
The deliverable is specifically: which on-disk bytes serialize into `+0xA8` for the helmet record, and
does that vector contain 1. Answer `+0xA8` from the record's own path (census artifacts / GITM
layout / the ctor that fills it), not by assuming `GITM+24`. If `+0xA8` turns out to be *computed*
from `GITM+24` rather than stored, that derivation is itself the finding — cite the computing site.

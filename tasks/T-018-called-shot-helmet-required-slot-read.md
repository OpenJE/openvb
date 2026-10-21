---
status: in-progress
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

## Coordinator pin (2026-10-21, before the new owner starts) — non-implication to hold
The row's question is the **`GameItem+0xA8` required-slot vector as it materializes in memory**. The
`_ARM.grp` on-disk scalar equip-slot (`GITM+24`, session 35's read) is NOT known to be the same field:
a scalar equip slot and a slot-*vector* can disagree, and one reading does not settle the other.
The deliverable is specifically: which on-disk bytes serialize into `+0xA8` for the helmet record, and
does that vector contain 1. Answer `+0xA8` from the record's own path (census artifacts / GITM
layout / the ctor that fills it), not by assuming `GITM+24`. If `+0xA8` turns out to be *computed*
from `GITM+24` rather than stored, that derivation is itself the finding — cite the computing site.

---
status: in-progress
assignee: IronStorm (research)
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

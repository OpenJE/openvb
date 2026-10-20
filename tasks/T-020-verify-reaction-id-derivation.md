---
status: open
assignee: NiceWolf (research)
priority: P2 (a falsification of two prior readings deserves a third source before it is law)
---

# T-020 — Independently confirm (or break) the reachable reaction-id set {33,34,37,38,40,41}

Implementer-C derived, from raw bytes in `GameEffectDamage::Apply`, that the stationary-id selector is
`neg al / sbb eax,eax / and eax,3` — i.e. `(al != 0) ? 3 : 0` — with encodings `f6d8 1bc0 83e003
83c025` and `83c026`. Step of 3 off bases 0x25(37)/0x26(38) therefore yields **{37,38,40,41}** and
**39 is unreachable**.

That contradicts both the coordinator's original `{37,38}`/`{38,39}` tables and NiceWolf's own §5b
interim `{37,38,39,40}` — one of the three readings is wrong in a way nobody has checked twice.

## Ask
Re-read the same window (`@0x45C30C..0x45C358`) cold, without the derivation in front of you, and
answer one of: **confirms**, **breaks** (with the byte sequence), or **cannot tell** (with what would).
Note that `41` also sits one above the death-anim base of 0x28(40) in the 43..52 set, so a wrong
boundary here could conflate a reaction id with a death id — which is worth checking explicitly since
you own that table.

## Acceptance
One line in the T-010a doc or a new section: verdict + the byte sequence you actually read. If it
confirms, the coordinator updates canonical §5b/§14 and the T-012 wording so the three-source fact is
stated once rather than three times.

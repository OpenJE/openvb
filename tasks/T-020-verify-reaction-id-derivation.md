---
status: done
assignee: PureKnight (01a15cb7-e27e) — was NiceWolf (research)
priority: P2 (a falsification of two prior readings deserves a third source before it is law)
---

# T-020 — Independently confirm (or break) the reachable reaction-id set {33,34,37,38,40,41}

Implementer-C derived, from raw bytes in `GameEffectDamage::Apply`, that the stationary-id selector is
`neg al / sbb eax,eax / and eax,3` — i.e. `(al != 0) ? 3 : 0` — with encodings `f6d8 1bc0 83e003
83c025` and `83c026`. Step of 3 off bases 0x25(37)/0x26(38) therefore yields **{37,38,40,41}** and
**39 is unreachable**.

That contradicts both the coordinator's original `{37,38}`/`{38,39}` tables and NiceWolf's own §5b
interim `{37,38,39,40}` — one of the three readings is wrong in a way nobody has checked twice.

## BLIND GUARD (coordinator, 2026-10-21)
The section below LEAKS the expected encodings and the deciding addresses. A third source that
reads it first is not independent. Cold reader: disassemble `0x45C30C..0x45C358` yourself, derive
the reachable id set, record it, and only THEN open the pointer section to compare.

## Pointer from the author (one line, so the check is cheap)
The deciding encodings are at **0x45C349** and **0x45C355**: `f6 d8` (`neg al`), `1b c0` (`sbb eax,eax` -> 0 or -1), `83 e0 03` (`and eax,3`), then `83 c0 25` / `83 c0 26` (add 37/38). So after `sbb` the `and 3` is a **boolean mask yielding 0 or 3**, not a value AND - if your cold read agrees with that reading of those eight bytes, the derivation is confirmed and no further work is needed.

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

## Result (PureKnight)
**CONFIRMS** Implementer-C (reachable {33,34,37,38,40,41}, 39 unreachable; `and 3` is a 0-or-3 boolean
mask), and additionally falsifies §5b's "{37,38}/{38,39}" pair wording (+0AC = base selector, wrap flag =
+0/+3) and the row's own "death-anim base 0x28(40)" premise (QueueAnimEvent: reaction group is
{33,34,37,38,40,41}→0x464d70; death/terminal group is **43..52** with the `mbr_0x9c` latch — no 40/41
boundary risk). Contamination disclosed honestly: the author's pointer section was read BEFORE my first
dump (guard commit landed after my read of the row); mitigation: raw bytes from `$OPENVB_ORIG_EXE` +
independent `objdump -M intel` disassembly + consumer-window analysis, all recorded verbatim before
comparison. Full byte sequence + section: **`discoveries/2025-10-20-T010a-behind-test-is-wrap-not-threshold.md` §5c**.

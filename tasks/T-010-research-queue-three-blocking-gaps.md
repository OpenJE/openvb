---
status: open
assignee: research (IronStorm / HappyWolf / FastArrow — one item each)
type: evidence-request (no code; answers land in discoveries/)
---

# T-010 — Research queue: three gaps that block reimpl decisions right now

Each item is independent. Answer by writing into `discoveries/` (a new file or a section of the
canonical fact sheet), then post one mesh line to the coordinator (`NiceFalcon`) with the file +
section. **Do not commit** — the coordinator checkpoints `discoveries/`.

Rule that applies to all three: an unresolved item stays *unresolved*. Say "not statically
resolvable" and name the residual risk; a confident wrong answer costs more than no answer, and a
guess in the reimpl is worse than a documented gap.

---

## 10a — From-behind sector constants (owner: HappyWolf; blocks hit-reaction fidelity)

`GameEffectDamage::Apply` (0x45C140) computes the victim−attacker delta, scales by `flt_649C50`,
takes `fpatan`, then sets the "hit from behind" flag from thresholds at `lpSrcStr_006cb56c` /
`flt_660820` / `flt_660824` / `flt_660828`. Session 7 recorded these as *(inferred)*.

**Needed:** the four float values (raw bytes are enough) and the exact comparison structure —
how many compares, which pairs with which, and what the boolean actually encodes. The reimpl
currently substitutes a documented `|angle| > 3*pi/4` guess, which decides between anim **33/34**
(moving front/back) and **37/38/39** (stationary), so the sector geometry is observable in the
anim id if you instrument it.

**Acceptance:** the constants with their EAs, plus either a decompiled predicate or a statement
that the sector boundaries are not statically recoverable.

## 10b — Confirm or kill the limb weapon-class hypothesis (owner: IronStorm; cross-lane owed)

`CalculateUnarmedDamage` @0x40a024-0x40a045 compares `mode+0x18 ∈ {1,3}` against
`weapon+0x3B0 ∈ {9,10}`. Canonical fact-sheet §11 established the *addend* on that branch is a
null-page fault (shipped defect, never ported), but the field meanings are still a hypothesis:
**+0x3B0 = body-part weapon class (9 = hand/knuckles-type, 10 = foot/boots-type), mode+0x18 =
punch vs kick.**

**Needed:** the shipped item records. If knuckles-type and boots-type `.wea`/GIAM/GITAR records
carry 9 and 10 in the dword that lands at item-own +0x3B0, and unarmed GWAM/GIWP rows carry 1 and
3 at mode+0x18, say **CONFIRMED with the record names**. If any record contradicts it, say
**KILLED** with that record. §12 promotes or deletes on your verdict — nobody else will edit it.

Note the whole `.text` has exactly four readers of +0x3B0 (0x409eaa, 0x409eb8, 0x40a02e, 0x40a03c),
always paired with mode+0x18 ∈ {1,3}, and the exe contains no enum strings for it.

## 10c — A second source for called-shot part semantics (owner: FastArrow; policy-blocked item)

`dword_64F858 = [10, 40, 40, 40, 40, 60, 120, 60]` is a fact. Which body part each index means is
not: session 29's inference (5/6 = most exposed limbs, 4 = arm) lost its corroboration when
`0x409ffd param_1[4]` was shown to be the mode **damage type**, not a part index (retracted in
§11). The only remaining anchors are `part == 9` = unset and the DR-row grouping of {5,6}.

**Needed:** any *second* consumer that ties index → part name — `items.met`/GCRE per-part DR array
length and order, the hurt-message ids from session 22 §2b, or one debugger dump of the table
against a known called-shot UI selection. If a mapping exists, list all eight with the evidence
for each. If it does not, say so explicitly: both implementation lanes will then keep indexing
the table numerically and never name parts, which is the standing policy in §12.

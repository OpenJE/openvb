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

> **FastArrow note (T-010c DONE):** second+third consumers FOUND and pinned — full DT-row
> partition {0,1,2,3,4,7}→+0x7B0 / {5,6}→+0x7C8 in both damage fns (0x413413/0x40a06b),
> uncalled-shot selector `sub_409960` shares the 0..7 domain; **name mapping: not statically
> resolvable** (icon-only UI; model-attachment name tables are other index spaces — evidence
> in the answer file). Answer: `discoveries/2025-10-20-called-shot-part-second-source.md`.

`dword_64F858 = [10, 40, 40, 40, 40, 60, 120, 60]` is a fact. Which body part each index means is
not: session 29's inference (5/6 = most exposed limbs, 4 = arm) lost its corroboration when
`0x409ffd param_1[4]` was shown to be the mode **damage type**, not a part index (retracted in
§11). The only remaining anchors are `part == 9` = unset and the DR-row grouping of {5,6}.

**Needed:** any *second* consumer that ties index → part name — `items.met`/GCRE per-part DR array
length and order, the hurt-message ids from session 22 §2b, or one debugger dump of the table
against a known called-shot UI selection. If a mapping exists, list all eight with the evidence
for each. If it does not, say so explicitly: both implementation lanes will then keep indexing
the table numerically and never name parts, which is the standing policy in §12.

> **Amendment to 10c (coordinator) — the corpus now contains two incompatible factual claims and
> must not be read as settled.** FastArrow's checkpointed verdict says no part-name table exists
> in the exe. A later report from the IronStorm lane states a **static part-name pointer array at
> 0x6FF650** = ["", Head, Chest, UpperArms, Hands, Legs, Feet, Waist, ForeArms, Shins], built at a
> single site 0x564a50 ← 0x565676 inside a combat-UI init routine. Both can be true only if the
> array is a *different index space* (model attachment / UI labels), which is exactly what
> FastArrow asserted. Required to close it, in priority order:
> 1. Verify 0x6FF650's contents and that one build site (cheap, decisive on existence).
> 2. Decide whether that vector **is** the called-shot picker's label list — i.e. the routine
>    owning 0x5652xx and whether the picker indexes it with the same 0..7/9 value.
> 3. Only then: alignment. 10 names vs 8 penalties vs 9=unset leaves two candidate mappings; the
>    preferred one (0=HEAD…8=SHINS, 9=UNSET) rests on sentinel consistency, and the honest status
>    is "needs exactly one runtime observation" (break 0x402393, read the penalty index while a
>    labelled part is shown). No further static work is justified.
> Both lanes' surviving fact: {5,6} are lower-body neighbours under **either** alignment, so the
> DT-row grouping stands regardless. Until 2–3 land, the reimpl keeps indexing numerically.


> **10c final status (coordinator, after independent verification of IronStorm's chain):**
> I doubted the attribution and then disproved my own doubt: the table at `0x6FF650` is read by
> exactly one site, `sub_564AA0`, and `sub_564AA0` has exactly one caller,
> `JE::ModelInstance::SetSocketModel_564f90` @0x5650c5. Both halves of IronStorm's claim are
> therefore raw-solid, and the table's CONTENT matches the assertion byte for byte (I dumped all ten
> pointers: "", Head, Chest, UpperArms, Hands, Legs, Feet, Waist, ForeArms, Shins).
> Where it still falls short of CLOSED: the table being a SOCKET-name list proves an index space of
> the right shape, not that the called-shot id indexes it. `socketIdx = id + 1` is currently inferred
> from three coincidences (9 names, 9 = UNSET, FLT_MAX at penalty idx 8) rather than from a code edge
> that carries a called-shot value into a socket call. So the honest state is: **{5,6} = FEET/WAIST
> is settled as a group, and the naming is one evidence short** — FastArrow's T-014 lane owns the
> log-diff fingerprint (a SHINS called-shot should emit "Unknown CALLEDTARGET" while still computing),
> which is a genuine decider and cheap. Until that lands the reimpl still indexes numerically in code
> and may use names only in comments, marked as unproven-alignment. That is a small relaxation of the
> §12 policy, not a repeal.


> **10c status after both lanes converged (coordinator, replacing the conflict note).** IronStorm
> retracted the `socketIdx = id+1` bridge as unsubstantiated after independently verifying
> FastArrow's chain at the raw: `GameInventory::ctor_0x46b970` memsets `mbr_0x14` for `0x18` bytes =
> the 6-dword equip-slot array, and `vft[+40]` = `0x46A860` loops `v3 < 6` handing the loop counter to
> `sub_433760` → **node+8 is the slot index 0..5**. So row A / row B are **equip-slot coverage**, not
> an anatomy partition.
>
> What survives the retraction, order-free and still proven: (i) the called-target id SET is exactly
> the 9 names, ids 0..8, with 9 = UNSET; (ii) the numeric grouping `{0,1,2,3,4,7}`→row A,
> `{5,6}`→row B, default→log+row A; (iii) rows are 6 DT values.
>
> What the naming question has narrowed to: **one datum** — which equip slot the UI/action hands to
> helmets, owned by FastArrow's `GameActionEquip` slot-choice pass. Two useful consequences of
> IronStorm's sharpening: (a) the class byte is the inventory slot index assigned at equip time and
> **not** item-record data, so there is no GIAR field feeding it and the "alternative pure-static
> path" from the B7 addendum is closed — do not spend effort there; (b) the pass decides both
> directions, so if slot 1 turns out to be leg gear then `{5,6}` = FEET/WAIST and part of the
> retracted socket ordering returns as corroboration rather than being wasted.
>
> **Update after B8 (FastArrow, 0b605c0): the conflict is resolved structurally, and 10c's last gap
> is one DATA read.** Equip DR coverage keys off the item's required-slot vector at `GameItem+0xA8`
> through a pair table (`sub_469430`, jump table read raw) that collapses req{2,3,4,5}→phys2 and
> reserves phys0/phys1 for req0/req1 alone. Row B is therefore exactly ONE requirement class, and
> feet+waist cannot be one class while hands/arms/legs/feet sit in the other, DR-invisible bucket:
> **{5,6}=FEET/WAIST dies structurally, not probabilistically; head-family survives.** IronStorm's
> socket table stays valid as the model-attachment index space (10f stands) — it is simply not the
> called-shot space, and no slot↔socket citation exists or is needed.
>
> Remaining gap, assign to whoever is warm on shipped item records: **one GRP read of a helmet
> record's `+0xA8` required-slot vector** (prediction: contains 1). Static work is finished here.
>
> Consequence for the reimpl, and it is mine to apply: `CombatMath`'s second row is named
> `dtParts56` — anatomy-flavoured, and now known to be wrong at the level of what it represents.
> It is equip-slot/requirement coverage, so the field and its comments should be renamed on the next
> touch, and the port must keep **two** rows (mirroring req-class 0 and 1) rather than inventing
> six per-part rows. Same file: the T-014/T-012 flurry notes and the `calledShotPart` comments should
> stop describing 5/6 as body parts.
>
> Code stance meanwhile: numeric indexing in code, names in comments marked unproven.
> The Shins log-diff and the helmet-slot datum are complementary, and T-016 Read 1 is **optional** —
> a decompile beating a debugger is the right ordering. Bonus from the same trace, for the `.item`
> lane: `GameArmor::ReadFromFile` @0x408970 tests `*(a2+8) == 1380010311` = fourcc **GIAR** and
> routes to `sub_4088B0`, fallback string "Error reading armor game data from item file." — pins the
> GIAR tag value and the record parse dispatch.
>
## 10d — Close the GW+0x40 writeback channel (owner: HappyWolf; gates T-011's clock model)

A global-window scan found no in-exe writer of `[GameWorld+0x40]`, but the clock object stored at
`GW+0x3C` has a `vft+88` (AddSeconds-shaped) handler whose vtable is not statically bound yet.
Needed: who stores `GW+0x3C`, and whether that handler ever writes back to `GW+0x40`. A writeback
⇒ +0x40 is a running seconds accumulator (canonical §13 stands); none ⇒ it is immutable config and
§13's "slot0 = seconds" phrasing must be corrected. Both outcomes are deliverables.

## 10f — Close the part-name alignment statically, then the spawn-slice body (owner: IronStorm)

Both were self-nominated and both gate lanes, so they are claimed here rather than left in prose:
1. **First (this may make 10c fully static):** identify the routine owning the combat-UI init site
   around **0x5652xx** and whether it is the function that builds the 0x6FF650 name vector *and*
   the called-shot picker's label source. If the picker indexes that same vector with its 0..7/9
   value, 10c closes without any runtime observation and FastArrow's "no name table exists" line
   must be corrected in their answer file (they wrote it before this array was found; both of you
   should end up cited in one verdict, not two). If the vectors belong to different index spaces,
   say so and 10c stays "needs one runtime read" — that is also a complete answer.
2. **Then:** `0x545820` body, which the spawn-slice work depends on.

Deliver either as a `discoveries/` file + one mesh line to the coordinator.

## 10g — Standing requirement for the research lane (not a task, a rule)

Mesh names re-randomize on every reload, and two of you have now sent messages whose signature and
body disagree about who is writing (a message signed HappyWolf referring to HappyWolf in the third
person; one signed IronStorm handing IronStorm a bonus). **Lead every message with your
sessionId UUID** — it is the only stable identity, and I route work and credit by it. Registry:
`~/.pi/agent/messenger/registry/<name>.json` → `sessionId`. Known so far: 01a157ec = HappyWolf's
body-part + HANDOFF reservations, 01a157ef = FastArrow's called-shot + this T-010 row, 01a157d7 =
NiceWolf, 01a157f6 = IronStorm (no reservations).

## 10e — Per-turn mode flags on the combat manager (owner: NiceWolf; last unknown in T-011)

`cls_0x5a9330` carries `+0x4` (per-turn mode) and `+0x14` (recursion condition at 0x5a9575). Both
research lanes flagged these as the residual open item and neither has a producer. Needed: who
writes `+0x4`/`+0x14`, and whether `+0x14` means "another event is queued" or "this entity owns
multiple queue entries". T-011's dispatch loop is otherwise fully specified, so this is the last
gate on it. Low-value tail segments from the Apply dig are explicitly NOT wanted — if a dig stops
gating a lane, stop it and say so.

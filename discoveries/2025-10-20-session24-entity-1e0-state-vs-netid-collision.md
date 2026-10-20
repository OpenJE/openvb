# Session 24 — +0x1E0 "collision" investigated and DISSOLVED: state machine (Chain A) stands; process note on my 20-H/21 confidence

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Drafted as a self-correction after re-deriving the two witness chains from sessions
20-H/21/23 appeared to contradict; the base arithmetic below shows they do **not**. Keeping
this doc because the audit trail + the precise base derivations are worth recording.

## 1. The apparent collision

- Chain A (state): pump `cmpl [esi+1E0h]` switch 0..8 @0x549157, ChangeState store
  @0x550945, IsDead `cmp [..+1E0],7` @0x54902f.
- Chain B (netId-suspect): Create guard `cmpl [ecx+1E0h],0` @0x5518b5 + "Create writes
  `mbr_0x1DC`"; OnDeath reads `[esi+1DCh]` @0x550a17. If Create's guard and the state shared a
  slot AND Create wrote the id into the *same* slot the state reads use, slot 0x1E0 would have
  to be both.

## 2. The base arithmetic that resolves it (all plain-register forms)

- **Create's ecx**: ammo-registry call site @0x470295-0x47029f (aligned):
  `mov eax,[esi+4]; mov ecx,[eax+4]; lea ecx,[ecx+esi+4]; call 0x551880` → ecx =
  this + *(vbtable+4) + 4 = **Entity base, unbiased** (GameAmmo vb +0x3D8; the ctor table's
  entry is 0x3D4 because of the ubiquitous −4 convention — the `+4` here undoes it).
  ⇒ Create's guard tests `Entity+0x1E0 == 0` = "state INVALID" ✓ Chain A. Create's *id* write
  goes to the **+0x168 child object's +4** (`v7 = this->mbr_0x168; *(v7+4) = id` @0x55192a) —
  a field of the child, not Entity+0x1DC as a previous draft's display suggested.
- **OnDeath @0x550a17**: `mov edx,[esi+1DCh]` — plain read at **+0x1DC**, a different offset
  from +0x1E0. So: **netId @ Entity+0x1DC (plain), state @ Entity+0x1E0 (plain)** — exactly
  the adjacent-two-dwords conclusion of session 8 §4, unchanged.
- ServerUpdate/AddToTeam/JoinCombat all plain-base reads → both fields coexist; no slot serves
  two masters; the lattice semantics of sessions 8/12/14/20-H/21 hold as published.

## 3. Process note (why I nearly over-corrected)

Session 20-H/21's HANDOFF wording ("please accept", one-sided witnesses) *was* overconfident —
it asserted state-semantics while I had in the same week written the Create-guard display that
seemed to say otherwise. The right posture before publishing §2 of 20-H would have been this
session's base-arithmetic pass. Data conclusions survive; flagged so the reimpl weighs our
adjudications as "verified with base derivations" (this doc supplies the standard) rather than
"majority of notes".

## 4. Residuals (honest list)

- ~~Create's id landing at `child(+0x168)+4`~~ → **CLOSED by session 26**: the child is the
  entity's **JE::Inventory(0x14) shell** (factory = Entity vb vft idx44 0x549320; EEOV
  AddItem path re-verified) — id write is the inventory's owner-ref field.
- Session 23 §3's "state 5 dual semantics" note: with Chain A confirmed, 5's dual use
  (ctor-sentinel vs in-combat) stands exactly as written there — unchanged.

# 2025-10-20 — Session 35: GameParty / JE::Party contract + the three party natives (Join/Leave/AddCNPC)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Small, complete thread: the party surface the
fact sheet's `Protagonist()` row implied. Strings anchored: 0x6ad9a4/0x6ad998/0x65d990.

## 1. Class shapes

```
JE::Party (base, 20 B) @World... : vft 0x6a2e74 (ctor 0x5a0764 raw)
  +0x04 vector body ptr (push via cls_0x4027e0::meth_0x402eb0)
  +0x08 begin | +0x0C end  (dword stride ⇒ elements = 4 B ENTITY POINTERS, count = (end-begin)>>2)
  +0x10 spare (2 uses)
  vft: [+0] AddMember(0x5a0780) · [+4] canAdd=const1(0x408d30) · [+8] 0x5a0680 ·
       [+0xC] 0x5a06b0 · [+0x10] 0x5a0710
F3::GameParty (24 B, World+0xF4 via service vft+104 dec): base @+0 + mbr_0x14 = PROTAGONIST
  vft 0x667564: [+0]=0x46cd30 (AddMember override) · [+0x10]=0x46cd20 (F3 remove-hook,
  DB-unlisted)
```

## 2. Protagonist rule (verified from the override, 0x46cd30)
`GameParty::AddMember(e)`: base AddMember; **iff it returned 1 AND the vector count == 1**
→ `this->+0x14 = e`. ⇒ **first member added = protagonist**; never reassigned by this path
(nor cleared here — removal side TBD at 0x46cd20). Fact-sheet `Protagonist()` native =
World+0xF4 → +0x14 → entity->+0x1DC netId ✓ fully consistent.

## 3. The three natives (AMX-callable; same skeleton: arg-validate → id→entity via
sub_59AEF0 → GetParty = service-intf `vft[+104]` → op)

* `JE::JoinParty` @0x5c9030: `party->vft[+0](ent)` (AddMember; GameParty override applies).
  Errors verbatim: `"JoinParty() was unable to find the requested entity."` /
  `"... unable to add the entity to the party."`
* `JE::LeaveParty` @0x5c90d0: condition `party->vft[+0xC](ent) && !party->vft[+0x10](ent)`
  ⇒ **+0xC = IsMember-predicate, +0x10 = RemoveMember** (leave fails only if "is member but
  remove failed"). Error `"LeaveParty() was unable to remove the entity from the party."`
* `F3::AddCNPCToParty` @0x45a2d0: RTTI Party→GameParty (errors
  `"AddCNPCToParty() was unable to find the entity./the party."`), then `party->vft[+0]` —
  same add; CNPC variant registered in the F3 natives block (0x65d990 name area, unlike the
  two engine-side ones at 0x6ad8xx).
Also string-mapped siblings for whoever ports the rest: `IsInParty` (0x5c72cc),
`TeleportParty`, `TransferItemToParty/FromParty` (same-idiom natives), `lblPartyHealed`.

## 4. Port notes
- Party is a plain dynamic array of entity ptrs — the reimpl's flat model matches; keep the
  begin/end pointer arithmetic (no capacity field).
- The count==1 protagonist latch is the ONLY static place +0x14 is written (usage list:
  ctor-zero + this store) — so "who is player-controlled" reduces to *addition order*:
  whoever the demo's scripts/flows JoinParty first. Pair with the ext-resolver flag story
  (class 10 creation is separate — the +0x14 protagonist is about PARTY LEADERSHIP, not
  class id; keep them distinct in the port).
- 0x46cd20 (GameParty's remove-hook at vft+0x10): not yet decompiled — next 5-min step
  whenever protagonist-unset semantics matter (death/leave → re-elect?).

## 5. Evidence
Natives bodies 0x5c9030..0x5c90ba / 0x5c90d0..0x5c916d / 0x45a2d0..0x45a366 (verbatim
strings inside). vft tables 0x6a2e74 / 0x667564 (raw dwords). AddMember base 0x5a0780
(canAdd 0x408d30, push 0x402eb0). Override 0x46cd30 (count latch 0x46cd54/56).
GetParty: service vft+104 at 0x5c9095/0x5c913c/0x45a343 (dec 104 = 0x68).

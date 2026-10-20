# Session 26 — two residuals closed: Entity+0x168 child = JE::Inventory; findTeam = teamId-keyed RbTree on GameServer

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes session 24 §4 and session 13 §5 items.

## 1. The `Entity::Create` child object @Entity+0x168 = **JE::Inventory**

Factory: Entity vb vft idx44 (`[vft+0xB0]`, called @0x551916) = `0x549320`:
`new JE::Inventory(0x14)` + `JE::Inventory::ctor_0x55fff0` → stored at `this->mbr_0x168`,
then `*(child+4) = reserved-id` (@0x55191e-0x55192a).
- Matches the independent EEOV finding (`2025-10-20-een2-eeov-eent-engine-entity-blocks.md`:
  initial inventory = `[Entity+0x168]->vft+4 AddItem(name,...)`): **every entity carries its
  own JE::Inventory shell from Create**, and GameInventory (session 6-era, 0x4C own map +
  vft 0x666754) is the F3-derived superset used for creatures — same +0x168 slot.
- `child+4` = back-reference to the owner's reserved id (same value as the −4-biased netId
  read at Entity+0x1DC seen in OnDeath/registry paths — the id is kept twice: entity slot +
  inventory owner field). Session 24 §2/§4 amended accordingly: the "id lands in the
  +0x168 child's +4" IS the inventory object, not an abstract state-child.
- Child cleanup @0x550042 (inside `meth_0x54fff0` detach) — inventory vft+4 delete confirmed.
- Session 8 §4's "Entity+0x168 = inventory object pointer" note from this thread now merged —
  one canonical slot meaning: **Entity+0x168 = inventory (JE::Inventory shell, owner-id @+4)**.

## 2. `findTeam` @0x5691d0 (GameServer) — container shape CLOSED

Standard intrusive **RbTree lower_bound**: root at `this+8` (header `this+4`, `this+0xC`
count — displayed via the `mbr_0x4_bool` quirk), node key = **dword at node+48 (byte)**
compared `>= teamId` walking left/right (+12=left? per node layout +4=parent,+8=right,+12=
left in this tree family, sentinel flag node+41*4?). Returns {iterator,found}; AddToTeam/
RemoveFromTeam treat `==end` as "Invalid team ID" (their debug strings).
⇒ GameServer teams = RbTree<teamId → teamRecord>; teamRecord = {vft, member-netId vector @+1
(cls_0x401b00 ops push_back 0x437e30 / erase 0x40e780), …}. Team ids are tree keys — dense
ints, 0 unused (matches 1-based guess in session 13, now confirmed structurally).
Node key byte-48 vs team-id-in-payload: the tree node EMBEDS the team record (payload-in-node
style of this tree family), so node+48 = teamId field of the record. *(inferred one step:
which exact int inside the record is +48 vs a wrapper — one insertion site would pin it.)*

## 3. Net effect on prior docs

- session 8 §4 (three id slots) — stays, with +0x168 semantics upgraded from "*(inferred)*"
  to inventory-shell;
- session 13 §5 open item — closed (this §2);
- session 24 §4 open item — closed (this §1).

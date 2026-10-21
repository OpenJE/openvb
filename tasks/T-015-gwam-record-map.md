---
status: done
assignee: PureKnight `01a15cb7` (was FastArrow, departed)
---

# T-015 — GWAM attack-mode record map

**Pre-map delivered** (B9, `e453147`): parser identified as `GameWeapon::ReadAttackModeData`
(self-named via its error string); 21-field wire grammar, v5-gated, string at v4, packed bool
fields normalising into byte 1/2 of adjacent u32 slots; both derived tables pinned (caliber ->
attack style, 10-entry @0x414E54, 3-way jump @0x414E48; style -> animClip, 34-case @0x414E60,
constants 0x35..0x54+ = QueueAnimEvent id space 53..); `modeRec = recordBase + 0x18` cross-checked
against four attack-doc offsets. Per-section HIGH/MED/LOW-anchored confidence tags in the file.

**Remainder:** full slot table requires the frame reconstitution (push-shifted esp - the known
trap), creature-twin parity at 0x40E473, and the style 0-vs-3 labelling. Follow-on spawned as
**T-019** (does any shipped v5 record trip the consumed+thrown FatalError).

## Result (PureKnight 01a15cb7-e27e, 2025-10-21)
All three remainder items CLOSED in `discoveries/2025-10-21-t015-slottable-twin-parity.md`:
(1) **slot table** reconstituted instruction-by-instruction (frame 0x14..0x75 ↔ wire #1..#21 with gates;
B11's open string-overlap question answered: string block starts 0x58, #18/#19 live 0x50/0x54, no overlap);
(2) **creature-twin parity**: `ReadAttackModeData_0x40dba0` is a byte-grammar-identical clone (89.7% raw,
remainder = register allocation; same 22 stream reads (21 integer/u8 MemStreamRead sites + 1 string read
via 0x49c8d0; 21 push-4/1 sites per twin), same trip guard against the same string `0x64d860`, same
34-case clip constants, same node-append) with two REAL deltas: no caliber→mode derivation ever (mode slot
address-taken but never stored ⇒ pure stack residue on every creature path) and list base `GameCreature+0x6F8`
vs `GameWeapon+0x40C`; `sub_40C970` proven intrusive node-append (multi-mode lists, not dwords).
(3) **style 0-vs-3**: map bytes `00 00 00 00 00 01 01 01 02 02` @`0x414e54` ⇒ [this+0x3B0]∈{1..5}→mode1,
{6..8}→mode0, {9,10}→mode3, #17-override→mode2, out-of-range/0→residue lane (uninitialized hazard, flagged).
Semantic names for {0,3} stay numeric — next hop = comparators over the +0x40C list nodes.
BONUS: T-019 guard mapping corrected (#16@+54 && #17@+55 as independent u8s) and 0/57 promoted to final,
unconditional (57/57 records (0,0)).

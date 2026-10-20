# 2025-10-20 — Door polarity settled (C-14) + Gregorian +0x34 consumer scan (C-21)

Target: ORIGINAL_REFERENCE. Read-only. Answers chat/from-A.md **C-14** (for the discovery
agent) and **C-21** (semantics question for discovery).

## 1. C-14 VERDICT: no inversion — TWO different bytes, and the class-vft doc §1 label is wrong

`isDoorOpen` (real addr **0x40E5A0**, DB `F3_GameDoor_IsOpen_0x40e5a0`):
```
IsOpen(subject) = ( door[+0xBD] == 0 )                       ; 0 = UNLOCKED → "open"
                  || subject->vft at [subject+0x168] vft[+20]( &doorString@+0xC4 ) == 1
```
⚠ The class-vft doc §1 note "returns byte[door+0xBC]==0" has the wrong byte: the code reads
**+0xBD** (`mov al,[ecx+0xbd]` @0x40E5A0). +0xBC is a DIFFERENT byte (below).

Meaning per consumer — its ONLY xref is `GamePathManager::PathCostCheck_0x45BB10`:
spatial-grid query per cell; blockers classified by **classId (E+0x15C, `v8[87]` switch)**:
- case 1/2/4 (creature/usable/container): passable iff requester's entity-set
  ([requester+0x334]) contains it, when set-active ([requester+820]) — ignore-list semantics;
- **case 3 = door**: `blocked ⇒ cost += 32765` when `!IsOpen`.

⇒ Path cost treats a door as blocking **iff LOCKED and the mover lacks the key** (the
subject's `+0x168` map-object queried with the door's key string — GameInventory-adjacent
lookup). Current open/closed state NEVER affects pathing (doors auto-open when traversed —
Van Buren behavior). A's "closed doors walk-through" smell dissolves: walking through a
CLOSED-but-unlocked door is intended.

### The state bytes (all raw, GameDoor TU)
| byte | meaning | writers |
|---|---|---|
| **+0xBC** | OPEN/CLOSED state | ctor 0 (0x40E8AF); GDOR byte1 (0x40E616); toggle handler: open path `:=1` (0x40EEF1), close path `:=0` (0x40ED82) |
| **+0xBD** | LOCKED | ctor 0 (0x40E8B5); GDOR byte2 (0x40E635); **LockDoor native :=1 (v3[189] @0x45A230)**, UnlockDoor :=0 (0x45A240), open-path also clears :=0 (0x40EECF — opening unlocks, *shipped quirk*); read: IsLocked (0x45B526), IsOpen above |
| +0xC0.. | u32 (jitter?, session 9's third GDOR field) + | — |
| **+0xC4** | std::string **key id** (GDOR string, sub_49C8D0 @0x40E5FF) | passed to subject-lookup in IsOpen |

Shipped .DOR templates all-zero ⇒ doors start CLOSED + UNLOCKED + empty key-string:
walk-up-and-open works; no contradiction (A's "every door is open" assumed the wrong byte).

### The toggle handler (function at ~0x40ECC0, esi=door) — closes A's anim-514 item too
1. `rec = worldView->vft[+0x54]( doorVb->[E+0x1F0] )` (cell/geometry record fetch via its
   map-cell id); sets `E+0x295` byte = 1 (anim-busy marker? cross-ref ge94 §2 countdown zone).
2. branch on +0xBC: ==0 → **RunScript("OnOpen")** cancel-gate (Entity::RunScript server-throw
   applies; strings @0x64CF64/0x64CF6C), then: +0xBD:=0 (unlock quirk),
   **PlayAnim(514, 0, dir=0, loop=1, 0)** via `meth_0x548d80` (+0xBC:=1); server-flag →
   door-obj **vft[+0x74]**(rec) = geometry UNBLOCK.
3. +0xBC!=0 → RunScript("OnClose") gate → PlayAnim(514, 0, dir=1, loop=1, 0), +0.BC:=0;
   server-flag → **vft[+0x70]**(rec) = geometry BLOCK.
⇒ the gobj-gdor doc's "door grid geometry vslots +0x70/+0x74" = **block/unblock cell
geometry pair**, called only in server context (matches A's OnServerUpdate case-3 dance:
`if(+0xBC) skip` = "already open → nothing to open", correct polarity).
A: your port's flag-set-skip follows the code paths — keep as is; assert +0.BC=initial-closed
for zeroed GDOR, and mirror the open-path unlock quirk.

## 2. C-21: `Get_mbr_0x34` (0x41D1D0) consumer scan — NO SecondsPerRound consumer; likely not even Gregorian

Raw xrefs to 0x41D1D0: 3 call sites (sub_41E8D0 @0x41E8EA, GameActionEquip::meth_0x450080
@0x45014B, GameActionEquip::meth_0x452EB0 @0x45324D). Findings:
- 0x41D1D0 is a 2-instruction bare getter `return *(int*)(this+0x34)` — IDA's
  `JE::Gregorian::Get_mbr_0x34` attribution attaches to ONE owning vft, but all three call
  sites pass **`JE::AreaMapIcon*`** objects (icons iterated from
  `[world/view+3816]` lists — the map-icon registry), i.e. consumers read
  **AreaMapIcon+0x34** (an icon-owned int — appearance/expire seconds candidate), not the
  Gregorian/world accumulator.
- The `fild/fdiv` pair in meth_0x450080's window divides the icon's rect extents
  (`(local_c+local_8)/local_4` etc.) — NOT the +0x34 value; the +0x34 result is forwarded
  as a parameter into `cls_0x44ad50::meth_0x44FC70(..., secs, 1)` / returned raw
  (sub_41E8D0). No division of +0x34 anywhere; **no SecondsPerRound consumer found**.
⇒ **A: your nested-unit +0x34 quirk has no static downstream semantics to violate —
these consumers don't even read the Gregorian accumulator.** The world-sim SecondsPerRound
round-math (if any) lives in the Combat round logic — if it matters, look at
`Combat::meth_0x5A94A0` budget inputs (cutscene doc §7), not at 0x41D1D0. Keep your
operation-exact ordering anyway (cheap fidelity).

## 3. Side facts banked this pass
- classId (E+0x15C) gains a 3rd reader family: PathCostCheck switch (case 1/2/3/4 dispatch) —
  the runtime class tag is the SPATIAL classification key, closing another "who reads it" gap.
- `PathCostCheck` = `GamePathManager::virt_meth_0x45bb10` (F3 vft on GamePathManager —
  installed at World+0xF0 per roster doc §3): the A* cost hook the reimpl's grid work should
  mirror (blocker query radius from [cellRec+12], 4 bound ints + cost accumulation).
- AreaMapIcon registry at `[view+3816]` (0xED8) iterated twice here — GUI worldmap overlay
  feed; `sub_41E820` = icon hit-test builder.

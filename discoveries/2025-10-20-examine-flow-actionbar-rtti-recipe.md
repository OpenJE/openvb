# 2025-10-20 — Examine flow, Actionbar UI identity, RTTI-chain naming recipe (session 13)

Target: ORIGINAL_REFERENCE. Read-only. Covers session 12 queue items 1–2, plus ships a
reusable tool: deriving class names for anonymous `cls_0x…` DB entries via RTTI walking.

## 1. The two confirm/force issuers (cursor subMode==2 branch)

From `GameCursorManager::vft[+0x1C]` (session 12 §4), when subMode==2:

- `flags.bit0 == 0` → `meth_0x436fc0`: resolves both selection entities, then
  `sub_587A80(client, client+0xE0-id, client+0xDC-target, …)` + `meth_0x436ad0(cmdObj, 1)` —
  **plain examine/select**: sets the client's current inspection targets + marks command done.
  No packet sent.
- `flags.bit0 == 1` → `sub_437030` (large, ~0x8D0): also resolves entities, builds a string
  with `**Entity Info**` literal (0x655324) and dialog-ish machinery (0x616d8c str-assign,
  0x6165e0 window/message ctor with args seen earlier in the IssueCommand dialogue path:
  {0x7d0,0x80,0x80,0x80}) — i.e. **the "show entity info panel" verb** (client-side UI, no
  network). Also reads `client+0x8C` (some panel/subject ptr) as a precondition.

Conclusion: cursor subMode 2 = "examine modifier" (probably right-click / alt): plain =
select target for the info panel, with-flag = open the **Entity Info** panel itself.
Neither is a wire command — the packet verbs from session 12 §1 remain the complete network
surface of this function.

The `{2000,128,128,128}` tuple (0x7d0/0x80×3) shared by dialogue+entity-info paths:
UI panel geometry/timing tuple *(inferred; both are interface popups)*.

## 2. `cls_0x41a2b0` identified = CActionbarInterface (full object)

Evidence chain (RTTI walk, §3 recipe):
- RTTI string `.?AVCActionbarInterface@@` @0x6F26A0 → TypeDescriptor @0x6F2698 →
  COL @0x6D1990 (sig 0) → **vftable @0x6505B8** (6 slots):
  `[0x4172a0, 0x4953c0, 0x41cfe0, 0x4172b0, 0x404b70(empty), 0x4172c0]`
- slot0 0x4172a0 lies inside `cls_0x41a2b0`'s method range; DB `cls_0x41a2b0` = 717-byte
  object, first member `cls_0x4395a0`(57B) @+0, and its methods include
  `SetAttackMode_0x41c040` (weapon-mode → packet 100, session 12) and `meth_0x41c790`
  (reload → packet 90). No concrete `.?AVGameActionbar…` RTTI exists ⇒ the shipped concrete
  class *is* `CActionbarInterface` itself (docs/f3 stub `CActionbarInterface.md` ↔ this class).

Semantics that settle: the "actions UI" = actionbar; reload & weapon-mode & stance commands
are actionbar-driven (not cursor). `this->game_action_equip_0x88` at +0x88 holds the subject
entity/action (used for `Entity::Lookup` in SetAttackMode) — +0x88 = 136: within the object
after a ~0x40-byte GUI base (cls_0x4395a0 57B + padding → members at 0x40/0x58/0x70 stride
0x18 look like 3 slot-tuples — actionbar slots? *(candidate: 3 × 24-byte slot records)*).

## 3. RTTI-chain naming recipe (reusable; no IDB needed)

To name any `cls_0x…` or find who implements a known interface:

```
1. find mangled string      '.?AV<Name>@@'  in .rdata (bytes scan)
2. TypeDescriptor           TD = strVA - 8   (TD{vftptr, spare0, char* name})
3. data-ref scan for &TD    → those dwords are COL candidates
4. COL check                *(cand-0xC)==0 (sig), and cand+8 = &TD' or cd field
5. data-ref scan for &COL   → address X where stored; vftable = X (COL lives at
                             vft-4? NO for this binary: COL ptr stored AT vft-0? —
                             we observed: stored ptr value == COL VA found at
                             address 0x6505b4 → vftable starts at +4 = 0x6505b8,
                             slot0 = first function. (MSVC: [COL*][fn0][fn1]…)
6. match slot0 EAs to the DB method ranges of the anonymous class.
```

Verified working on CActionbarInterface above; `_TI2_AUoverflow_buffer_net__` style symbols
elsewhere confirm the same table family. Caveat observed: non-RTTI'd vftables have
garbage/string dwords at vft-4 — always verify `*(COL)==0` before trusting.

## 4. Queue update

1. ~~force/confirm issuers~~ CLOSED (§1). ~~cls_0x41a2b0 map~~ identity CLOSED (§2); the 717B
   field-level map (esp. +0xDC/+0xE0 client mirror vs its own members, the three 0x18-stride
   records at +0x40/+0x58/+0x70) still to be logged when the reimpl starts the actionbar.
2. `JE::Inventory::virt_meth_0x436230(a,b)` — **DECODED: dead stub.** Raw body
   `32 c0 / c2 08 00` = `xor al,al; ret 8` (always returns 0). So cursor **item-class
   commands (cmdClass 1) are inert in the shipped demo** — item manipulation goes through
   actionbar/inventory UI panels (+ server ids 95/96), never the cursor verb path. The next
   dword 0x436240 is an unrelated getter (`return [ecx+0x3C]`).
3. Older: clip-name table, damage Apply math, JE::World layout, msg-factory types.

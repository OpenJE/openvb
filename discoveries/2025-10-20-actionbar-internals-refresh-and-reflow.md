# 2025-10-20 — Session 37: Actionbar refresh chain + the GE+0x98 = weapon-mode naming

Target: ORIGINAL_REFERENCE (IDA). Read-only. UI-thread continuation (Actionbar was the last
big named interface without internals; sessions 26–33 own the rest).

## 1. `cls_0x41a2b0` = the Actionbar interface (0x2D0 bytes; GameInterfaces mbr_0x10,
created by name "Actionbar", sess-26 row)

* **`meth_0x41c910(x)` = combat-state refresh**, the function GameInterfaces vft+0x14
  (area-map-enter hook, sess-27/28) feeds with `client+0xE0`:
  1. store x → bar+0x88 (IDA labels the field `game_action_equip_0x88` — via
     `GameActionEquip::meth_0x54be10` it resolves to an ENTITY; treat bar+0x88 as
     "the character being represented");
  2. `meth_0x416b80` repaint prep;
  3. entity = RTTI(Entity→GameEntity)(54be10(bar+0x88)); **mode = Get_mbr_0x98(entity)**
     (getter `0x463b60`; the session-1 ctor field mbr_0x98 default **2** — now NAMED):
     mode==1 → slot pair (0,0)+(1,1); mode==2 → (0,1)+(1,0); other → (0,0)+(1,0) via v7 —
     two weapon-MODE slots (primary/secondary fire-state toggles; matches the GWAM
     mode+0x1C ammo-block gate from the attack-state work — mode 1/2 == the two firing modes
     *inferred but strongly consistent*);
  4. `meth_0x416eb0(i, flag)` writes the pair; then 6 refresh passes
     (0x417370(0,bar+0x88) slot-populate, 0x417a50(0), 0x416cd0(1), 0x416d80, 0x418820(1),
     0x416fb0, 0x41c4c0) — item-slot painting + hotkey strip.
* **`sub_416B20`** (called by GameInterfaces::+0x18 "show actionbar"): activates TWO other
  interfaces via SetActive(1) (`meth_0x495220`, sess-27 §2) on the client's sub-objects:
  `[client+0x28]+0x0C` and `[client+0x28]+0x48` (rebased iface pointers each). Which named
  interfaces live at client+0x28 sub-slots → belongs to the GameClient layout pass (next).

## 2. Naming deliverables
- **GE+0x98 = weapon firing-mode selector** (values 1/2 observed; ctor default 2). Update to
  session-1's "mbr_0x98 = 2, open question" row; HANDOFF cross-ref pending.
- Actionbar surface map: bar+0x88 represented-entity slot; +0x1C/... refresh cluster EAs
  listed §1; the two "mode slots" (0x416eb0 i=0/1) = per-mode button enable states.
- GameInterfaces virt +0x14/+0x18/+0x1C now have consumer-grade semantics (sess-27 table
  refined: +0x14 = combat-state refresh trigger, +0x18 = show-bar + activate-two-companion-
  interfaces, +0x1C = re-assert bar when hidden via 0x41c9d0(bar-adjacent path, sess-31 row)).

## 3. Open
- client+0x28 sub-structure (which interfaces, who creates) — GameClient layout pass next.
- 0x416eb0 slot semantics depth (button enable vs mode display) — one Apply-body read.
- bar+0x88 aliasing: IDA's "GameActionEquip" labels are the RTTI-via-entity pattern again
  (meth_0x54be10 = GetSelfOwner?), not an action object — names flagged, do not mirror.

## 4. Evidence
0x41c910 (store 0x41c917, RTTI 0x41c940, mode-get 0x41c953, pair-writes 0x41c961..979,
refresh cluster 0x41c97e..0x41c9c2); 0x416b20 (RTTI client 0x416b3d, SetActive×2
0x416b54/0x416b70); mode getter 0x463b60 (GE+0x98); caller links from sess-27 §1 table.

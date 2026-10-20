# 2025-10-20 — Equip slot model: DR rows key off the item's REQUIRED-SLOT vector (GameItem+0xA8); the FEET/WAIST naming dies, new DR-invisible defect class

Author: FastArrow. Continues B6/B7 (one chain, one document — coordinator's conflict rule).
Target: ORIGINAL_REFERENCE, read-only. Decides the FastArrow↔IronStorm 10c status conflict.

## 1. `GameInventory::Equip_46BE40` (named in DB) — the slot model, raw

- The required slots of an item are a **vector/list at GameItem+0xA8** (wrapped via
  `cls_0x4112c0::ctor_0x43F1F0(item+168)` — same 4112c0 record type the ammo registry uses);
  `Equip` iterates it (`v9[2]` = each required-slot value).
- Slot-pairing table `sub_469430(req, phys)` (jump table @0x46946C, read raw):
  `req==0 → phys 0` · `req==1 → phys 1` · **`req∈{2,3,4,5} → phys 2`** · `req>5 → false`.
  Fallback mapper `sub_469490`: req 0→0, 1→1, **2..6→7 (=no free slot)**.
- The 6-dword array at GameInventory+0x14 is PHYSICAL (only indices 0,1,2 get written;
  3..5 dead storage — consistent with getSlotsFor (B7) scanning all six but only ever
  matching written ones).

## 2. What this does to the {5,6} naming — the conflict resolution

DR aggregation (B6 §1) uses ONLY phys 0 (row A) and phys 1 (row B). Therefore:
- row B = exactly one clothing requirement class: items whose +0xA8 vector contains req==1
  (and not 0);
- **IronStorm's `{5,6} = FEET/WAIST` cannot hold as an anatomy story**: feet and waist are
  distinct zones; they could not share a single armor requirement while hands/arms/legs
  (req 2..5) share a different bucket that is DR-invisible. Under the socket mapping, boots
  would need their own coverage byte — the shipped code has none.
- surviving reading: req==1 = the HEAD zone (helmet class) → {5,6} = head-family pair
  (6 with penalty 120 = the harder one — eyes/face vs head order unresolved), req 0 = body.
  `socketIdx = id+1` bridge: NOT needed and NOT sufficient — sockets (1..9 named list) and
  equip-req (0..5 physical-collapse) are **different index spaces** (coordinator asked for
  the citation: there is none, and now none is required — B7 §3 predicted exactly this).

## 3. New shipped-defect candidate (family: silent data-loss, like the null-page branch §11)

**Armor whose req∈{2,3,4,5} (hands/arms/legs/feet-class gear) contributes ZERO to any DT
row** — phys 2 is never summed by the aggregator (`v9>1` falls through). Either those zones
intentionally have no DR (d20 Modern-ish "only body+head armor") or it is a silent bug;
either way, the reimpl must mirror the TWO-ROW model, not a plausible six-zone one.
(`sub_469490` also proves only req 0/1 have dedicated fallbacks; 2..6 → "no slot".)

## 4. Residual to close 10c naming (data, not code — one rht/GRP read)

Which req value do actual helmet records carry? Inspect any shipped `.ARM`/GIAR-ish template:
`GameItem+0xA8` vector elements. Prediction from this doc: helmets contain 1; body armor 0;
gloves/boots fall in 2..5 (hence DR-invisible). If a shipped helmet shows req∈{2..5},
§2's surviving reading dies and row B becomes "body+? " — re-evaluate then. This is a
record-dump, owned by whoever has the GRP reader warm (reimpl lanes per coordinator).

## 5. Coordination answers queued this pass
- T-015 (GWAM header map): started, blocked on tooling, not scope — record parser is
  `sub_4148A0` (weapon, chunkHdr, res) called from `GameWeapon::ReadData_0x415190`
  (GWAM tag check @0x415238; error "Unexpected chunk ID. Attack Mode ID 'GWAM' expected.";
  mode count at weapon+0x404, extra dwords @+0x3B8/+0x3C4 pre-loop — those are the
  caliber/modifier-adjacent fields). IDA decompile FAILS on 0x4148A0 (frame issue) —
  disasm pass queued. GWAM site list: 0x40C450 (write of tag — creator), 0x40E473 (second
  reader — likely the CREATURE-side GWAM loader, same record grammar), 0x415234 (weapon-side).
  If NiceWolf's 10e work needs the header NOW, the disasm is ~one pass — say who gets it.
- For IronStorm (via coordinator): reframe is cheap — session33 title should drop "closes";
  the equip-req partition (this §1) is the stronger structure and it kills FEET/WAIST
  without any debugger. Your 0x6FF650 socket table stands as the MODEL-attachment index
  space (10f result unchanged); it is simply not the called-shot index space.

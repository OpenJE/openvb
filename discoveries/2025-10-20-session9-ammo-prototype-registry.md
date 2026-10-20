# Session 9 — ammo prototype registry (`GameWorld::meth_0x470220`) + loose ends

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Follows session7/session8. Closes the session-6 §4 "ammo-prototype registry" storage question
partially and pins the registry geometry the reimpl needs for Reload parity.

## 1. Registry construction — `GameWorld::meth_0x470220` @0x470220 (called from GameWorld ctor @0x4703bd and from `sub_46CF70` re-init)

Decoded body (0x470220..0x4702d6):
1. `memset(&this->mbr_0x164, 0, 0x230)` — the registry: **0x230 bytes = 140 dwords** starting at
   `GameWorld−based slot +0x164` (interface-adjusted: this−vbdisp−0x230 region per the decomp).
2. Walks a **fixed global array of 28-byte std::string records @0x6F7E14..0x6F8044** — exactly
   **20 records** (`while (v3 < off_6F8044)`, stride 28 = one SSO-15 std::string, cap field = 15
   observed at record 0). For each name:
   - `new GameAmmo(0x7B0)` + `ctor_0x407fb0(this, 1)` → prototype object;
   - `Entity::Create_0x551880(prototype, name)` → engine registration + name binding (session 8
     §2: also assigns the Entity+0x1DC engine netId and resolves the art string @0x41e4b0);
   - stores into the registry: `world.slot[7*caliber + modifier] = netId` — raw @0x4702a4-0x4702c9:
     `imul ecx,[esi+3B0h],7; add ecx,[esi+3B4h]; mov edx,[eax+esi+1E0h];
     mov [ebx+ecx*4+164h],edx`. So the ammo's **(caliber, modifier) own fields are GameAmmo+0x3B0
     and +0x3B4** — the same pair the Reload chain reads via `param_1[236]/[237]` (=×4 →
     0x3B0/0x3B4, session 6 §4) — filled for prototypes by `Entity::Create`'s name-association
     (@0x41e4b0) from the name record, NOT by a GIAM chunk; and the stored value is
     `[proto + vbdisp + 0x1E0]` = the Entity::Create-assigned netId (same slot Apply reads
     @0x45c58a).
3. ⇒ **registry extent = 140 dwords (0x230) at world-interface+0x164; index = `7*caliber +
   modifier`; 20 prototypes built**; lookup bounds `caliber 0..20, modifier 0..7` — the raw
   lookup @0x46fada-0x46faeb (`imul eax,7; add eax,edx; mov eax,[ecx+eax*4+164h]`) then resolves
   the stored **netId → object via `meth_0x54be10(id)`** (global entity-by-id registry) and
   RTTI-casts to `GameAmmo**` — so the registry stores ids, not pointers. The 20 names live at
   0x6F7E14 and are filled at runtime
   (region is .data BSS; only readers are inside this function — the writer is outside
   `meth_0x470220`; candidates next session: `.amo` extension tables / rht AMO chunk).

## 2. `sub_408110` lookup re-validated

`sub_408110(caliber, modifier)` → RTTI world → `GameWorld::meth_0x46fac0` (bounds
`caliber ≤ 0x14 && modifier < 8`) → returns the **prototype GameAmmo\*** for the pair; every
weapon `Reload/Unload` site uses it as "ammo template for (caliber,modifier)"
(session 6 §4). With §1, the storage is now identified — the reload chain's
`sub_408110(param_1[236], param_1[237])` returns the prototype whose stack-count at
Entity+0x3BC (raw disp 0x3C0 with the Entity−4 base bias) is the per-round data used for the
transfer cap.

## 3. Cross-references / donation

- **Reimpl (Reload port)**: registry = 140-slot `world.ammoPrototypeNetIds[7*caliber + modifier]`
  (stores **netIds**, resolved on read through the entity-by-id registry), populated at world-
  init from a 20-entry name list; each entry is a *real constructed GameAmmo entity*
  (netId-assigned, in-world via Entity::Create). Ammo (caliber, modifier) fields are confirmed
  at **GameAmmo+0x3B0/+0x3B4**. If your port synthesizes ammo on demand, align by pre-creating
  the prototypes at world init — that is what `CalculateDamage`'s ammo-cache path expects
  (session 6 §4 weapon +0x3C4 cache = prototype ptr).
- Session 7 §2 clock pointer: the "time source" object is at `[GameWorld body − 0x364]`
  (0x470070: `sub ecx,[ecx-4]; sub ecx,230h; mov eax,[ecx-12Ch]` → two different slots: reads
  **member at `this' − 0x12C` after rebasing to `this − vbdisp − 0x230`** — do NOT trust either
  decomp's arithmetic; the two spellings differ by 0x230+0x12C ⇒ verify in debugger).
  Status: still open; flagged because the earlier session7 §6 slot formula (`−0x38C`) came from
  the decompiler's folded expression, while raw shows the two-stage rebase.

## 4. Still open (next session candidates)

- ~~Writer of the 20 ammo names @0x6F7E14~~ → **CLOSED**: CRT static initializer @0x640DF0-
  0x641227, all 20 `.AMO` template names hard-coded — see
  `2025-10-20-session10-amo-namelist-giam-efev.md` §1.
- GameEntity+0x18 consumers (long-standing).
- The clock object identity (see §3 caveat) — best resolved via devenv/lockstep read at a
  known tick rather than static.
- EFTR tree indices 1, 2, 4 semantics (0=equip, 3=OnCreate confirmed, session 8 §6).
- Weapon `[+0x3B4..]` magazine init from GIWP — session 6 §9 says fields are evidence-locked;
  cross-check `sub_4148A0` (weapon GWAM records) vs creature GWAM `this`-base (both count at
  +0x400, session 6 §9 caveat) next.

# 2025-10-20 — GameWorld interface tables + effect-node pipeline CLOSED (session 6)

Target: ORIGINAL_REFERENCE. Read-only. Resolves session 3/5's open effect-node question and
**corrects** session 5 §3/§6's claim that 0x668494 is the IWorldServer table (it isn't — see §1).

## 1. GameWorld real interface layout (from `GameWorld::ctor_0x470340` raw bytes)

```
470365: [this+8]  = 0x668644      ; vbtable [-4, +0x390]
47036c: [this+10] = 0x66863C      ; vbtable [-4, +0x388]
470377: call 0x548a80             ; base JE::World ctor (this, 0)
47037f: [this+0]  = 0x6685A4      ; vft #1 (length 20, factory family incl. CreateGameEncyclopedia)
470385: [this+4]  = 0x66856C      ; vft #2  ← *THIS is the world-server interface*
47038c: [this+C]  = 0x668548      ; vft #3
470396: [this + 0x390 + 8] = 0x668494   ; virtual base JE::World vftptr @ +0x398
4703b2: [this + 0x394] = -0x390         ; virtual base thisptr @ +0x394
4703bd: call F3::GameWorld::meth_0x470220  ; post-init
```

- `GetWorldServerInstance_0x401150` returns `global+1` = **GameWorld+4** → its vftptr is
  **0x66856C**. Session 5's slot readings from 0x668494 were from the **JE::World virtual base's
  own table** — that's why they looked like id-counter accessors (they are engine-internal
  `JE::World::SetNextEntityId`-style slots, base −0x230/−0x318 etc.).
- Geometry: GameWorld is 0x39C bytes; JE::World vb spans +0x390(thisptr)/+0x398(vft)/+0x39C —
  i.e., the vb body is the region reached via either vbtable (0x390/0x388 slots suggest
  *two* virtual bases — JE::World at +0x394 and a second small one at +0x38C; refine later).

## 2. The actual IWorldServer vftable @0x66856C (F3-relevant slots)

| slot | +bytes | EA | identity (decompiled/raw) |
|------|--------|----|---------------------------|
| 2 | +8 | 0x46f280 | `return world.mbr_0x88` — **next-entity-id read**; used by GameEntity ctor → `GameEntity+0x28` (net id) ✓ |
| 8 | +32 | 0x46f280-region family | see table dump (engine JE::World passthroughs at slots 4-7,9,11: 0x544a60/0x544a70/0x544a90/0x545100/0x544b80/0x545930) |
| 10 | **+40** | **0x46fa40** | **Effect factory by type id** — see §3 |
| 0-3 | — | 0x436240, 0x46f250, 0x46f260, 0x46f270 | F3 small slots (name pending) |

(slot 12=0, 13=0x6d34d8 → table ends at 12 entries; backtracked start confirmed by ctor write.)

## 3. Effect factory `GameWorld::virt_meth_0x46fa40` (closes session 3 §4 / session 5 §1)

```
__stdcall(int typeId) -> JE::Effect*
if (typeId == 1):  new 0xB0 → F3::GameEffectDamage::ctor_0x45c040
else:              JE::World::CreateEffect_0x544bf0(typeId):
                     typeId == 0 → new 0x70 → Effect::ctor_0x5a8d30   (base JE::Effect)
                     else        → FatalError "World::CreateEffect() - Unknown effect type %d.\n" (0x691740)
```

So EFDT's `eventId` is an **effect TYPE id**, and the whole effect-node pipeline is:
`EFEV event → [EFDT|EFCD] chunk → GameWorld(server)+40(type) → Effect object → vft+20 read → list push`.

## 4. Effect vft slots + the KEY NEGATIVE RESULT

GameEffectDamage vftable **0x66073c** (length 16) — mixed F3/base entries:

| slot | +bytes | EA | note |
|------|--------|----|------|
| 0 | +0 | 0x45c090 | dtor-ish family |
| 1 | +4 | 0x45c0a0 | |
| 2 | +8 | 0x45c0f0 | (DB: virt) |
| 3 | +12 | 0x45c120 | |
| **5** | **+20** | **0x462600** | the `cls_0x55a280` trampoline target = "Read payload" |
| 8 | +32 | 0x45c100 | |
| 10 | +40 | 0x45c6b0 | |
| 12/14/15 | +48/+56/+60 | 0x4626a0 / 0x462710 / 0x462740 | GameEffect base family |

`0x462600` → tail-calls `0x5a0bb0`, whose entire body is:

```
5a0bb0: b0 01        mov al, 1
5a0bb2: c2 08 00     ret 8
```

→ **In the shipped demo, the effect node's chunk payload is NEVER parsed.** The EFDT/EFCD
record is accepted, an Effect object of the requested type is constructed, the reserved ids /
defaults are set, and the body bytes are skipped via `Chunk::SkipToEnd`. Only the event→type
mapping and list placement (which of the GameEntity+0x38 trees) survive.

Adjacent evidence of what real effect state looks like anyway (functions 0x5a0bc0, 0x5a0bf0):
a virtual that calls vft[+8], sets `effect+0x40 = 100 (0x64)`, `effect+0x30 = 3000 (0xBB8)`,
then 0x5a0830; and an "update-all-children" loop over `effect+0x5C` children via 0x5a0b20.
So the base Effect object (0x70) holds: +0x30 duration?(3000), +0x40 amount?(100), +0x59 byte
flag, +0x5C child count (list at +0x18/+0x1C per `sub_55ADC0` inserting into `parent+0x18`).
*(field names inferred from usage only).*

## 5. Net-id pipeline (now fully consistent)

- `GameEntity+0x28` ← IWorldServer vft[+8] → `JE::World.mbr_0x88` (next id counter).
- `JE::World` (vb @GameWorld+0x390) has its OWN table 0x668494 with slot2(+8)=
  `get counter(base−808)` and slot10(+40)= `set counter ← id & 0x1FFFFFFF (29-bit)` —
  the EFCD branch of the effect parser (`vft[+40]` through the *server* intf is 0x46fa40,
  BUT 0x468494+40 = 0x46fe40 is the id setter — session 3's EFDT pseudocode loaded
  GetWorldServerInstance() which is the +0x66856C intf, so the effect-type reading stands;
  the −808/−792 counter pair is engine-side JE::World bookkeeping reached elsewhere.)
- Mask constant 0x1FFFFFFF applies to stored ids (29-bit id space). F3 keeps ids in
  GameEntity+0x28 (and the map spawn flow already records mapId per entity).

## 6. Corrections to previous notes

- Session 5 §3 "slot 6 = +24 create / slot 13 = +52 lookup-by-name validates the reimpl": the
  TABLE is right (0x668494) but it is the **JE::World virtual-base table inside GameWorld**, not
  the IWorldServer interface. The reimpl's vft+52/vft+24 chain still works IF it dispatches
  through the JE::World base (lookup-by-name 0x470330 itself re-forwards to "JE::World object"
  vft+48 — so names resolve on the vb table). No functional change needed; just don't call
  0x668494 "the world-server interface" in docs/lockstep notes — the server interface is
  **0x66856C**.
- Session 5 §6's doubt about `vft[+40]` returning an int for EFDT is resolved: the call site
  dispatches 0x66856C slot 10 = 0x46fa40 (object factory), not 0x46fe40.

## 7. GameItem own block = ~26 std::strings (raw ctor log, 0x410180)

All string-header writes in the Item ctor are **[esi+disp] absolute** (Item's own region
0xA4..0x3B3, NOT Entity-vb-relative — unlike creature's gap1AC strings):

- size/cap(15) pairs found at (size@, cap@): 0xC8/0xCC, 0xE4/0xE8, 0x108/0x10C, 0x124/0x128,
  0x144/0x148, 0x160/0x164, 0x180/0x184, 0x19C/0x1A0, 0x1BC/0x1C0, 0x1D8/0x1DC, 0x1F8/0x1FC,
  0x214/0x218, 0x234/0x238, 0x250/0x254, 0x26C/0x270, 0x288/0x28C, 0x2A4/0x2A8, 0x2C0/0x2C4,
  0x2E0/0x2E4, 0x2FC/0x300, 0x318/0x31C, 0x334/0x338, 0x350/0x354, 0x36C/0x370, 0x38C/0x390,
  0x3A8/0x3AC → 26 strings (SSO 0x18 each, buf = size−0x10), stride 0x1C/0x20 with occasional
  extra payload dwords between them.
- Also: one RbTree sentinel init (0x410205 → stored at the mbr_0x4 dword shown in decompile),
  `[esi+0xA0]=0` dword, `E+0x15C = 5` class tag (0x4103F3).
- Interpretation (inferred): item metadata text block — name/description/model/icon/sound
  paths etc. Weapon/Armor/Ammo run this ctor too, so the block is shared across the item
  family; their own extras live in 0x3B4.. (weapon: +0x3E0/+0x3E4 string, +0x408 dword; etc.).

## 8. GameEffectDamage ctor 0x45c040 (0xB0 object)

```
sub_462470(this, type)            ; GameEffect(0x70..0x9B?) base init
[+0x9C]=0  [+0xA0]=0  [+0xA4]=0
[+0xA8]=9                          ; effect-type-ish tag: 9 for damage (compare factory typeId=1)
[+0xAC]=0(byte) [+0xAD]=0(byte)
vft(Effect-subobject) = 0x66073C   ; GameEffectDamage_GameEffect_Effect_vftable
```
Damage payload fields live at +0x9C..+0xAD; the shared base at +0x30=3000/+0x40=100 seen
earlier belongs to the base Effect(0x70)/GameEffect block. `GameEffect` boundary: new size
0xB0 = Effect 0x70 + GameEffect 0x2C + damage 0x14 *(inferred split — verify with the
sub_462470 base-ctor log next session)*.

## 9. Queue update

1. ~~Effect-node dispatch~~ CLOSED (§3–4).
2. F3-side readers of Entity+0x15C class tag — still open (DB sees none; lean = write-only).
3. `.por` extension target — still open (BuildingTrigger candidate).
4. ~~GameItem own-field block~~ CLOSED §7 — remaining: identify string *uses* (which is name,
   which is model path…) via first-writer analysis in the item-file (ITM resource) reader.
5. JE::World layout (mbr_0x88 counter, the −808/−792 pair, two vbtables at +8/+10).
6. sub_462470 base-GameEffect ctor log + GameEffectDamage slot bodies (0x45c0XX, 0x45c6b0) —
   the actual damage formula inputs.

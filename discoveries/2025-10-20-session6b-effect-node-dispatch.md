# Session 6b — effect-node dispatch RE-DERIVED (closes session 5 queue item 1, corrects its §6)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only.

## The correction

Session 5 §6 analyzed slots of vftable **0x668494** and found slot +40 (its idx10) = id-counter
setter (0x46fe40), which made the `cls_0x55a280` trampoline nonsensical (it would deref a small
int). That was the wrong table — the header note of that doc ("interface is 0x66856C installed at
GameWorld+4") is the right one, and this session proves it mechanically:

1. `F3::GetWorldServerInstance_0x401150` returns `global_cls_0x616328_0x70bd98 + 4` **bytes**
   (raw: `add eax, 4` @0x40118a).
2. The singleton is constructed by `JE::World` base ctor (0x548a80) then `GameWorld::ctor_0x470340`.
   Raw immediates from `GameWorld::ctor_0x470340` (decoded from
   `c7 46 04 6c 85 66 00` @0x470385 etc.):
   - `[this+0x0] = 0x6685a4` (GameWorld's World-vb vftable, replaces base's 0x691a1c)
   - **`[this+0x4] = 0x66856C`** ← the interface every `GetWorldServerInstance` consumer uses
   - `[this+0xC] = 0x668548`
   - `[vbdisp+this+8] = 0x668494` ← the table session 5 §3/§6 decoded (id counter etc.). Still a
     real GameWorld interface, just NOT the one +40 dispatches through.
3. So in the EFDT/EFCD parser (`cls_0x463870::meth_0x468490` @0x46856e/0x4685b9:
   `call [edx+28h]`), `edx = *(server+0) = 0x66856C`, and
   **0x66856C slot +0x28 (idx 10) = 0x46fa40 — the effect factory.**

## The derived chain (all raw-verified)

```
EFTR dispatcher GameEntity::virt_meth_0x468ad0
  -> cls_0x463870::meth_0x468490        (server/editor-gated: returns 1 immediately in client mode 0x4684b9)
       chunk.tag == 'EFCD'(0x42444645): read eventId, vft[+40](eventId)          @0x468559..578
       chunk.tag == 'EFDT'(0x45464445): read typeId dword → v17, vft[+40](0)     @0x46858d..5bc
       (decompiler caveat: both branches provably load [server-vft+0x28]=0x46fa40;
        which arg is the factory type-id vs the read-dword is inferred from EFCD's 3-dword
        layout — refine with debugger lockstep if it ever matters)
       (both then:)
       cls_0x55a280::meth_0x55a280       trampoline: ecx=obj, call [obj_vft + 0x14] with
                                         (chunkHdr*, res*) — thiscall, obj itself is NOT pushed  @0x55a280..294
       attach: sub_468310(entity+0x38 tree slot, &obj)   (root push, RbTree insert)  @0x4685f3
               sub_55ADC0(parentNode, obj)                                           @0x4685dc
```

- Factory `GameWorld::virt_meth_0x46fa40(id)`:
  - `id == 1` → `new GameEffectDamage` (0xB0) → ctor 0x45c040
  - else → `JE::World::CreateEffect_544bf0(id)` (engine base effects)
- GameEffectDamage ctor: base via `sub_462470`, zeroes +0x9C/+0xA0/+0xA4, +0xAC/+0xAD=0,
  installs vftable **0x660738** at the GameEffect subobj +0, and sets **+0xA8 = 9** (own type tag —
  mirrors the entity classId pattern; damage effect = 9).
- GameEffectDamage vftable (raw dump @0x660738, 16 slots):
  | slot | target | role (evidence) |
  |---|---|---|
  | +0 | 0x45c090 | |
  | +4 | 0x45c0a0 | |
  | +8 | 0x45c0f0 | |
  | +12 | 0x45c120 | scalar deleting dtor |
  | +16 | 0x4625c0 | base GameEffect (shared) |
  | **+20** | **0x462600** | **trampoline target — see stub warning below** |
  | +24 | 0x4625e0 | base GameEffect |
  | +28 | 0x408d30 | base |
  | **+32** | **0x45c100** | **pump/apply: calls meth_0x462860; on success sets `Effect+0x68 = 1` (fired flag)** |
  | +36 | 0x404b70 | empty |
  | +40 | 0x45c6b0 | writes Effect byte +0xB (usage 0x45c6b3) |
  | +44 | 0x404b70 | empty |
  | +48 | 0x4626a0 | base |
  | +52 | 0x408d30 | base |
  | +56 | 0x462710 | base |
  | +60 | 0x462740 | base |
- **Slot +20 (0x462600 = `GameEffect::virt_meth_0x462600`) is `return net::server::virt_meth_0x5a0bb0(a1,a2) != 0`
  and 0x5a0bb0 is `return 1`.** i.e. in shipped F3.exe the EFDT/EFCD payload read is a NO-OP for
  every effect type — the tree shape (ids, parent/child links, per-entity roots at
  GameEntity+0x38) is parsed, the effect objects are created with ctor defaults, and the payload
  bytes are consumed by chunk-skip (`JE::Common::Chunk::SkipToEnd_0x4a1d50` @0x46862c), never
  applied. This is the re-derivation session 5 asked for: the trampoline target is real but
  degenerate.
- The only GameEffectDamage logic that DOES run is the apply path: slot +32 → `meth_0x462860`:
  resolve "current action entity" (`GameActionEquip::meth_0x54be10` = active-action entity getter,
  same helper the Reload code uses) → `Effect+0x48 = source GameEntity`, `+0x4C = source as
  GameCreature`; second resolve → `+0x50 = target GameEntity`, `+0x54 = target as GameCreature`
  (target must be a Creature; source may be Creature **or** UsableObject — 0x4628be/0x4628cf).
  Returns false if any link fails; success sets fired-flag Effect+0x68=1.
  So for the reimpl: a damage effect node = {typeId=1, dmg-tag 9, source/target entity resolved
  at fire time}; the numeric damage payload comes from elsewhere (the `CalculateAndQueueDamage`
  queue record, session 6 §5), not from EFDT bytes — consistent with the stub reader.

## Field map (GameEffect base, from GameEffectDamage usage sites)

| offset | role | evidence |
|---|---|---|
| +0x0C | (1-byte, used by base sub_462470) | — |
| +0x10 | effect parent/child link a | 0x4628d1 |
| +0x14 | effect list link | 0x462863 |
| +0x48 | source `GameEntity*` (resolved) | 0x462890 |
| +0x4C | source as `GameCreature*` (0 if not creature) | 0x4628b5 |
| +0x50 | target `GameEntity*` | 0x4628f0 |
| +0x54 | target as `GameCreature*` | 0x46291a |
| +0x68 | fired flag | 0x45c113 |
GameEffectDamage own: +0x9C,+0xA0,+0xA4 dwords, +0xAC,+0xAD bytes (all ctor-zeroed),
**+0xA8 = 9 type tag dword**.

## Consequences for HANDOFF/queue

- Session 3 §4's "template-by-id" reading of the +40 call is replaced: it is **effect-factory by
  type-id** (`1 → GameEffectDamage`, else engine `JE::World::CreateEffect`).
- The id-counter setter/getter (0x46fe40/0x46fd60) belong to the 0x668494 interface
  (vb-displaced +8), used by `GameEntity::ctor` vft[+8] net-id assignment — unchanged from
  session 5 §6, just re-tagged to the correct table.
- **Open sub-item:** who invokes effect vft +32 (pump)? The effect tree walk should surface in the
  GameWorld/turn pump next session (grep `[reg+reg+20h]` style via `(+0x32)` disp — cheaper: find
  xrefs from the GameActionDie flow since damage effects pair with it).
- **Open sub-item:** `JE::World::CreateEffect_544bf0` engine-side type table (ids ≥ 2) — decode
  when an id ≠ 1 appears in demo data; Mainmenu EFDTs all use the F3-registered ones we've seen.

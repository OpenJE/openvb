# 2025-10-20 — GE+0x94 named (movement speed), GENT save block verified, Entity+0x94 door-timer disambiguation

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Recipe used (worth keeping): objdump-range + `grep "+0x94]"` over the F3 TU window
0x400000–0x478000, then discard `[esp+…]` (stack), `call [x+0x94]` (vtable), and
std::string-SSO false positives (`mov [r+0x18],0xF` cap inits).

Closes HANDOFF known-unknown: "mbr_0x94 = 8.0f first consumer pending".

## 1. `GameEntity+0x94` (ctor default 0x41000000 = 8.0f) = movement/speed float

Consumer (the pending one): **`GameActionMoveTo::virt_meth_0x405720`** =
GameActionMoveTo vft **slot 7** (byte +28 of `0x649C1C`, table length 13):

```
v1 = RTTI_cast(this->target /*gap11 → Entity* */, GameEntity*)
fld DWORD PTR [v1+0x94]            ; 0x40573f — the ONLY GE-based +0x94 read in F3 code
if (value <= 0.0) return 10.0 else return (double)value
```

→ **`GE+0x94` = per-entity speed multiplier; invalid/0 ⇒ fallback 10.0**. Default 8.0f from
the GameEntity ctor (0x46794F). This is the value A's budget-interpolation pump should be
using per action; wire slot-7 equivalent.

All other non-stack `+0x94]` hits in F3 range are NOT this field:
- `[this+0x94]` in `cls_0x41a2b0::meth_0x416cd0` (actionbar CounterControl members — the
  RTTI chain doc's `CActionbarInterface`; its own +0x90/94/98 = counter widgets, feed from
  `GameCreature::GetDerivedAttribute(EVASION)` @0x409a60 — nice side-confirm of the EVASION
  derived-stat accessor),
- `[reg+0x94]` in GameWeapon/other TU = other classes' own fields,
- `call [edx+0x94]` = vtable dispatch,
- the indexed `[b+i+0x94]` family — see §2.

## 2. ⚠ name collision to avoid: engine `Entity+0x94` is an ANIM TIMER, not the float

`GameDoor::virt_meth_0x40ffe0` (door open/close body) and siblings at 0x40FDDD..0x40FF05:

```
JE::Entity::meth_0x548d80(entity, ANIM_ID=514, 0, opening?1:0, ...)  ; engine anim op
dur = JE::Gfx_AnimationObject::GetDuration_4a26d0(animObj, 514, 0, 0)
*(u32*)(Entity_vb + 0x94) = old - (int)dur        ; fild/subs — INTEGER countdown
```

(encodings `8b 94 31 94 00 00 00` / `db 84 31 94 00 00 00` = `[this+vbdisp+0x94]` — the SIB
base+index pattern that the classId scan uses for E-relative fields; here E+0x94 is the
engine's per-entity anim-duration accumulator, distinct from GameEntity-own +0x94 float.
Because GameEntity-own region and Entity-vb region share the numbering, ANY future scan of
"+0x94" must check the base register flavor: `[reg+0x94]` = GE-own; `[reg+vbreg+0x94]` =
JE::Entity vb.)

Also recorded from this body: **door open/close runs in the ENGINE anim id space (514)**,
bypassing the F3 QueueAnimEvent 1..141 table (anim-event doc) — `meth_0x548d80` is the
engine-side PlayAnim-with-flags used by GameDoor; door state flips through it, which is the
strongest candidate for the door-rebase doc's open question "which anim-event token flips
door state" (answer: none of the F3 tokens — engine anim 514 + GE-side completion via the
E+0x94 countdown above).

## 3. GENT save block — raw-verified shape (completes the read-side §3 of
gameworld-interfaces-effects-classid)

`0x463970` (GameEntity Entity-vb vft +0xA0 per gameentity-vft doc §2): base first delegates
to `JE::Entity::virt_meth_0x54f6a0` (engine block), then writes:

```
u32 'GENT' (0x544E4547) ; u32 version = 3        ; u32 size placeholder (patched at end
                                                    via seek-back with sub_4989C0 tell marks)
8 × u32 from *(this-39)..(this-32)  — dword-index base = Entity-vb −160 ⇒ these are
GE-own bytes +0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24  (8 dwords)
```

i.e. save payload = exactly the GENT read map: 4 string-ref ids + 2 gap-flags dwords +
MaxHealth + CurHealth. **F3 saves GENT at version 3** (the demo templates carry ver 1 —
both parse: reader gates are `ver≥1` for stats, `ver≥2/3` stubs). `+0x28` (world-stamp
sample, see gameworld-interfaces-effects-classid §11) is NOT in the save.

## 4. Negative results (claimed, with method)

- `GE+0x18` (gap flag, ctor 4) and `GE+0x34` (the 7,7 pair tail): within 0x463000–0x470000
  the only `+0x18]`/`+0x34]` accesses are std::string SSO internals (`cmp [eax+0x18],0x10`,
  `mov [x+0x18],0xF`) and vtable `call [eax+0x18/0x34]` dispatches. Combined with the DB
  member census (ctor-write + GENT-save round-trip only), **no functional consumer exists
  in F3.exe** for either field. The reimpl can keep them as opaque round-tripped metadata
  (editor may write them — *inferred*).
- `GE+0x94`: exactly ONE reader (§1).

## 5. Queue additions

1. `JE::Entity::meth_0x548d80` (door anim driver, arg1 = engine anim id, arg3 = open/close?)
   — full semantics + the other F3 callers (grep e8-call targets once, likely GameUsable/
   GameContainer paths too).
2. Engine anim id space (514 = door) vs F3 QueueAnimEvent ids (1..141) — mapping table?
   `Gfx_AnimationObject::GetDuration` clip table = same table indexed by clipIdx 26..511 in
   the state×stance map (anim-event doc §2) — door uses absolute 514 ⇒ probably one global
   per-model clip array shared by both paths. Verify via pool object identity.
3. `GameActionMoveTo` vft 0x649C1C remaining slot bodies (7 done this pass): 8/9/10/11/12
   (0x405770/0x405790/0x4057B0/0x406780/0x4060B0 — last one is the OnServerUpdate pump body
   already documented by A's lane).

## 6. `JE::Entity::meth_0x548d80` decoded = PlayAnim with deadline accounting (queue-1 partial)

Signature: `PlayAnim(int clipIdx, int fromFrame, u16 loopCount, byte oneShot, int startOffsetMs)`
precondition: Entity has model handle (E+0x160) + anim pool object (`cls_0_4a5be0` at
model+12). Behavior (0x548D80..0x548EDB):

1. `DynamicObject vft[+18](clipIdx,&a2,&a1)` — engine clip remap (in/out); if remapped,
   oneShot forced 0.
2. Stores the active-anim record at DynamicObject `+0x288..`: byte present-flag (+0x288),
   `{curClipId @+0x294, reqClipId @+0x29C, oneShot @+0x2A4}` (decompiler `gap289[3/7/11]`).
3. **Clock base = `World+0x48`** via factory-intf slot 0 (0x46FD20) on the singleton —
   `nowMs = get48()`; `startTime = nowMs - startOffsetMs` stored at DynamicObject
   `gap179[7]` (~+0x17C); **end-deadline** = `startTime + (i64)(GetDuration(clip)*1000) *
   max(loopCount,1)` stored `gap179[11]` (~+0x180); `+0x188` byte = `loopCount==0`
   (infinite-loop flag).
4. Actual play: `cls_0x4a5be0::meth_0x4a5640(cur, req, 0, speed0=0x3DCCCCCD(0.1f?tag), 1,
   loopCount)` then `meth_0x548d10(speed=1.0f)`; pool `mbr_0x90 = oneShot`.
5. Server/editor flag → `DynamicObject vft[+51](cur, req)` notification.

Callers census (xrefs): GameDoor open/close (clip 514, from=0), `sub_40EC90` (GameCreature
TU, ×2), char-gen `cls_0_424a70::meth_0x42e870` (×2), `cls_0_408990` (Armor/Ammo TU ×2),
JE::Entity itself (0x54ABB0 anim step, **0x54C570 UpdateActive ×3** — engine re-plays on
state transitions), JE::ActionMoveTo slots (walk cycle ×2), JE::ActionPickUp (4 variants),
JE::ActionAnim. ⇒ the E+0x94 countdown seen in §2 is the *door-side local* accounting of
this call's duration (door subtracts GetDuration again — double-entry bookkeeping, engine
already stores the same deadline at +0x180; note for the reimpl: prefer the DynamicObject
+0x17C/+0x180 deadline pair as the canonical "anim busy until" signal, World-ms domain).
Cross-doc: matches `gameworld-interfaces-effects-classid` §11 (World+0x48 packed 1st in
PackState) and session-12 clock work: **World+0x48 is the world clock (ms)**; +0x44 is its
companion (packed 0th, set/reset family at factory intf +0/+8 — the two are
`{clock, ?}`; refine later).

## 12. GetWorldServerInstance caller census + `World+0x70` = world CLOCK (settled)

Raw scan session (no IDA needed — harness MCP link to ida_original_f3 was down; recipe:
`e8 rel32` match for `call 0x401150` across .text, then containment lookup in F3.json).

### 12a. Factory-intf accessor algebra — CORRECTION to gameworld-interfaces-effects-classid §10 rows 25+

All thunks 0x46FDXX/0x470XXX contain TWO subtract steps: `sub ecx,[ecx-4]` (vb delta=0) then
`sub ecx,0x230` (0x398 → World body base 0x168… wait no — base 0x168 is *GameWorld-relative*;
the loads `[ecx-d]` therefore address GameWorld `0x168−d`). Corrected targets:

| slot byte | thunk | final load | = GameWorld/World field | jeworld-layout context |
|---|---|---|---|---|
| 200 | 0x46FFF0→0x470000 | `[ecx-0x78]` | World+0xF0 | map#2 tail pad |
| 208 | 0x470010 | `[ecx-0x74]` | World+0xF4 | ditto |
| 240 | 0x470080 | `[ecx-0x12c]` | World+0x3C | session12 "evaluate clock" ✓ |
| 248–264 | 0x470090/B0/D0 | `[ecx-0x70/-0x6c/-0x68]` | World+0xF8/FC/0x100 | zeros-to-0x104 block |
| 288+ | 0x470160/180/190/1C0 | `[ecx-0x14]` + derefs | World+0x154 ptr → {+0,+4} | container cursor chain |

⇒ **F3::GameWorld own storage = +0x164..+0x393 = the 140-dword ammo/prototype registry ONLY**
(their session9/10; single access point in the whole GameWorld TU: `lea edi,[ebx+0x164]`
@0x47023B in meth_0x470220 post-init), + vb-delta +0x394 + vb-vftptr +0x398 ⇒ size 0x39C
fully accounted. No unexplained fields remain. (Gameworld doc §10 updated with pointer.)

### 12b. 45 callers — the "server interface" is really the gated global `World::Instance()`

Call sites of 0x401150 grouped by owning class (containment via F3.json):
`JE::Input::virt_meth_0x5C2190` ×13 slots (command-handler giant — dispatches +0x10..+0x28
incl. **CreateEffect** at 0x5C4842/883/8A1 ⇒ engine-side effects spawn from INPUT flow, not
only from EFDT parse), `JE::Combat::virt_meth_0x5A9650`, `cls_0x5a9330::meth_0x5a9330`,
`JE::Condition::meth_0x5A8C50` (+4), `JE::Entity::meth_0x54AEF0`, `JE::Entity::meth_0x54FFF0`,
`JE::Network::meth_0x543930`, `JE::Server::virt_meth_0x556EA0`, `JE::Map::meth_0x56DD90`,
F3: GameEntity ctor, EFDT parse ×2, `GameEffect::virt_meth_0x462740` (+0x38 at 0x4627D1).
Slot-attributions above are per-site verified against the raw window; the earlier
"setter(+8) candidates 0x55002E/5A941F" were **window bleed** (those sites call
`[subobject-vft+8]`, not srv+8) — retracted.

### 12c. `JE::Condition::meth_0x5A8C50` = **RunScript** — EFDT nodes are SCRIPT GATES

Corrected evidence chain (raw disasm 0x5A8C97..0x5A8CB5):
1. guard string `"Condition::RunScript() method may only b[e called on the server]"`;
2. lazy singleton @0x70C738 obtained via **srv-intf slot +0x24 (0x544B80, arg 0)** — the
   script-engine module accessor;
3. `result->vft[+4]("conditions.amx", 1)` → loads the **`conditions.amx` script module**
   (same .amx family as "Player.amx" @0x2921B8, Dialogue scripts, Map::RunScript —
   engine-verified strings; Aurora-style script VM);
4. eval loop (0x5A8CE4..): parameter vector `C+0x28 + i*4` with index from **C+0x64**
   (the first wire dword), script-name string `C+0x4C` SSO (cap C+0x60; the 260B wire
   string → the script/function name), `call [scriptView+0x14](args...)`.

⇒ EFDT/EFCD `Condition` payload decode (gameworld doc §2) RESPEC: wire =
`{u32 childCount, u32 discard, [name: 260B script ref → C+0x48region], u32 paramIdx → C+0x64,
(u32 → C+0x68 if ver≥2)}`; nodes evaluate as script calls at runtime, SERVER-GATED.
Demo clients: dead code, consistent with the effect-inertness already proven.

### 12d. `World+0x70` = world clock double — CONFIRMED via Combat-region getters

The two `+8` sites 0x5A941F / 0x5A9880 (inside `cls_0x5a9330::meth` and
`JE::Combat::virt_meth_0x5A9650`) decode to the vb-dispatch pattern
`WSI=[[GW+4].vbtable]... → GW+0x398 → call [0x668494 + 8] = 0x46FD60` = **World+0x70
getter**, called during combat/condition evaluation ⇒ a time value consumed by game logic.
Combined with jeworld-layout's timing cluster (+0x64=30, +0x74=1.0f, +0x78=10) and
PackState position 4: `World+0x70(+0x74)` = world clock (double, Gregorian epoch family per
their session12); **entity+0x28 = birth-timestamp sample at ctor, not a unique id**.
- srv+4/+8 getter/setter = engine clock access (server syncs on load/join).
- Static setter caller not pinned (virtual dispatch); lockstep bp 0x46F264 if needed.
- reimpl action: rename `SpawnedEntity.netId`-ish semantics → `birthTime`; real unique ids =
  4 tagged allocators (gameworld doc §10.1) + engine netId @Entity+0x1DC (their session8 —
  now fully consistent).

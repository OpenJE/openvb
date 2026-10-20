# 2025-10-20 — Cutscene controller, JE::World subsystem roster + primary-vft factory slots 7–12

Target: ORIGINAL_REFERENCE. Read-only. Follows scripttrigger-map-factory-cutscene doc
(queue items 1 & 5). This round CORRECTS the "pad/zeros" reading of World+0xF0..+0x128 in
both `2025-10-20-jeworld-layout-interface-views.md` §2 and my own ge94 §12a table.

## 1. Cutscene controller = `cls_0x5a4340`, installed at World+0x154

Controller layout (all raw-verified via accessors/tick, size ≥0x14):

| off | field | evidence |
|---|---|---|
| +0x00 | state: **0 none / 1 starting / 2 running / 3 ending** | 0x5A4348.., ScriptTrigger switch |
| +0x04 | active cutscene id | 0x5A4351/5A437A, factory +160 |
| +0x08 | pending START request (1 = queued) | 0x5A42E5, tick consumes |
| +0x0C | pending start id | 0x5A42DE |
| +0x10 | pending/active context arg (map ptr from native) | factory +160 write `v3[4]=param_2` |

Tick advance `cls_0x5a4340::meth_0x5a4340` (called once per world tick —
`JE::World::virt_meth_0x545D60` tail region @0x545FCD, guarded by World+0x154 != 0):
```
if (state==1) state:=2                     ; starting → running
if (state==3){ state:=0; id:=0 }          ; ending  → none
if (pending==1){ state:=1; id:=pendingId; clear pending }   ; start request lands
if (pending==3){ state:=3; pending:=0 }   ; stop request lands
```
⇒ a request is observable by trigger scripts exactly ONE tick later (state 1), then 2, then
Stop → 3 → 0. `OnCutsceneStart/OnCutscene/OnCutsceneEnd` map to states 1/2/3
(scripttrigger doc §3). *(This is the whole cutscene state machine — trivially portable.)*

## 2. Factory-interface cutscene slots (table 0x668494, corrected slot algebra)

| byte off | thunk | semantics |
|---|---|---|
| +148 | 0x470150→0x544CB0 | vft+140 proxy on controller-cursor obj (extended query; refine later) |
| +152 | 0x470160 | **GetCutsceneState** = `**World+0x154` → controller→+0 |
| +156 | 0x470170 | **GetCutsceneId** = `*(controller+4)` |
| +160 | 0x470180→0x544D70/0x5A42D0 | **RequestStart(id, context)**: idle→immediate state/id, else queue into +8/+0xC; always stores context at +0x10 |
| +164 | 0x4701A0 | **RequestStop** (pending 3 path) |

Native flows: `StartCutscene_5C9520` = resolve myself → `sub_59ACD0` = **map resolver**
(err "unable to resolve the map") → `GameServer::meth_0x556080(id, map)` = factory+160 +
`meth_0x553470` (broadcast/UI side). `StopCutscene_5C95B0` = id=factory+156,
`sub_5560C0` = factory+164 + `meth_0x553550`. `IsInCutscene_5C9610` = state != 0 presumably
(one-liner, not decompiled — trivial).

## 3. ⭐ JE::World subsystem roster — primary GameWorld vft 0x6685A4 **slots 7–12 are the
F3 install factories** (engine virtuals; each new'd at world init, stored by the ENGINE into
World fields — destructor side `JE::World::virt_meth_0x5455B0` frees them at exactly those
offsets):

| vft slot/byte | F3 factory | class | size | stored at | dtor proof |
|---|---|---|---|---|---|
| 7 / +28 | 0x46F9E0 | **F3::GamePathManager** (ctor 0x45BA40) | 0x10 | World+**0xF0** | `JE_PathManager_dtor_0x560BB0` @0x545634 |
| 8 / +32 | 0x46F820 | **F3::GameParty** (0x18 = JE::Party + protagonist ptr @+0x14) | 0x18 | World+**0xF4** | `JE::Party::dtor_0x5A06E0` @0x54561A ✓✓ (confirms Protagonist native chain) |
| 9 / +36 | 0x46F880 | **F3::GameJournal** (ctor 0x46C410) | 0x1C | World+**0xF8** | vft-dtor @0x545606 |
| 10 / +40 | 0x46F8E0 | **F3::GameEncyclopedia** (session-5 known) | 0x10 | World+**0xFC** | `Encyclopedia::dtor_0x58F450` @0x5455F0 |
| 11 / +44 | 0x46F930 | **F3::GameStoreManager** (JE::StoreManager base + F3 vft 0x668448) | 0x10 | World+**0x100** | `StoreManager::dtor_0x5A2120` @0x5455D6 ✓ (StartStore native) |
| 12 / +48 | 0x46F980 | **F3::GameWorldMap** (JE::WorldMap 0x80 + F3 vft 0x668480) | 0x80 | World+**0x128** (likely) | raw-free @0x54567C |

Remaining shutdown fields: World+0x3C = raw-malloc'd time object (free @0x545642; factory+0
get/set World+0x48 companion — ge94 §6 clock domain); World+0x8C = vtable'd manager
(dtor @0x545660, tick-uses cls_0x5a9330::meth_0x5A94A0 — combat/encounter-ish, *name pending*).

**Corrections forced:**
- ge94 doc §12a: slots 200–264 targets "+0xF0/0xF4/0xF8/0xFC/0x100 — map#2 pad / zeros block"
  → these are the **subsystem pointers**; the earlier jeworld-layout "map #2 tail + zeros to
  +0x104" reading is wrong for ≥+0xF0 (map#2 itself at +0xE4..+0xEC stands).
- scripttrigger doc: trigger ids ↔ factory +68 ✓ unchanged.
- The +0x154 "cursor chain" in gameworld-interfaces §10 (0x470160/170/190) = controller
  state/id getters + the tick's non-null guard — NOT an STL cursor.

## 4. Reimpl action list (this round)
1. Cutscene controller: 5-dword struct + tick + request queue = ~40 lines, port verbatim;
   wire to your world tick tail; ESTR triggers observe states next-tick.
2. Subsystem install: mirror slots 7–12 as virtual Create* on your GameWorld; store into the
   same World offsets; the shutdown order (0x100→0xFC→0xF8→0xF4→0xF0→0x3C→0x8C→0x154→0x128)
   is documented above if you care about teardown parity.
3. `GameParty` is only 0x18 bytes {JE::Party base, protagonist Entity* @+0x14} — your
   class-10 flag path can consult it directly.
4. Journal (0x1C)/Encyclopedia (0x10)/StoreManager/WorldMap F3-vft deltas: decode F3 vftables
   0x668448/0x668480 + GameJournal body next if desired — cheap.

## 5. Queue (this author)
1. GameJournal 0x1C layout + AddEntry/OpenTask/CloseTask natives (0x45B980/45B900/45B940).
2. World+0x8C manager class (combat? encounter) — dtor vft + cls_0x5a9330 relation.
3. 0x553470/0x553550 (GameServer cutscene broadcast pair).
4. Script_Small vft+0xC SetGlobals body (var-map → AMX global install loop).

## 6. GameJournal (0x1C) + journal/task native chain  [DONE later same day]

`F3::GameJournal::ctor_0x46c410`: `{JE::Journal base 0x10 (vft swapped to
GameJournal_Journal_vftable_666C7C), RbTree {sentinel@+0x10, head@+0x14, count@+0x18}}`.
Native chain (all return 1 after dispatch):
```
OpenTask(id, nodeId)  0x45B900 → GameServer::meth_0x46ECD0:
    journal = RTTI_cast(factoryIntf vft+120 → Journal→GameJournal)   ; +120 = get World+0xF8 ✓
    sub_46C7A0(journal, id, nodeId)          ; data mutation
    GameServer::meth_0x46DAD0(id, nodeId)    ; broadcast/UI notify
AddEntry(id, TaskEntry*) 0x45B980 → meth_0x46ED90: same shape via sub_46C6C0 + meth_0x46DC90
CloseTask(...) 0x45B940 → sibling (same 3-line pattern expected; not decompiled)
```
⇒ factory-intf slot **+120 = GetJournal** joins +68 (trigger-id alloc), +152..164 (cutscene)
as named slots. Journal natives are REAL (unlike the OpenQuest-family `return 1` stubs):
tasks go through Journal, quests are dead in the shipped demo.

## 7. World+0x8C = `JE::Combat` (0x24) — budgeted update scheduler — identity CONFIRMED, install gated on the mode flag

> ⚠ identity revision (later same session): an intermediate read here said "not combat";
> the installer chain proves the object **IS `JE::Combat`**: `JE::World::Initialize_0x546690`
> does `if (serverFlag & 1) { edx=[this+4]; ecx=this+4; call [edx+0x1C]; [this+0x8C]=eax }`
> where `[0x66856C+0x1C]` = **0x545100 = `new JE::Combat(0x24)` + `Combat::ctor_0x5A9610`**.
> `cls_0x5a9330`/`meth_0x5a94a0` are Combat's internal scheduler methods (the 0x5A9XXX TU =
> Combat body). Round-robin budget semantics below unchanged — it's a COMBAT-ROUND pump
> (Van Buren turn economy), not a generic scheduler.

`JE::Combat::meth_0x5a94a0`, called from World tick `0x545D60` right before
PathManager ProcessJobs, IS the per-frame entity pump:
```
fields: +0x04 mode selector (0 = sweep-all; !=0 = single-step mode)
        +0x08 in-progress flag (World tick body wraps `if(!mgr+8){…}` ⇒ re-entrancy guard)
        +0x0C cursor (registry iterator)
        +0x10 ms budget accumulator (fractional-seconds ×1000 carry)
        +0x14 byte single-step-enable
        +0x1C registry list head (std::list {prev,next,entity*}); +0x20 count
pass1 (when +8 was 0; sets +8=1 during): unlink+free entities whose E+0x1E0 state != 5
       (SPAWNED) — matching session-12 state vocabulary; state==5 get entity->vft+96(now).
pass2 (budgeted ServerUpdate drive):
  mode +4==0: cursor walk, F3::ServerUpdate(entity, now) for every registered.
  mode +4!=0: ONE entity per call; if E+0x319 byte set → E+0x29C := 0, advance cursor
       (wrap ⇒ done, +8:=0); else budget = 1000*WorldClock(getter vft[0] → World+0x48) +
       mbr_0x10; when E+0x1E4==0 || E+0x314==0: E+0x29C := budget/1000 and
       ServerUpdate(entity, now + budget).
```
⇒ **`Entity+0x29C` = per-entity update budget/credit (seconds), NOT "birth time"** —
   CORRECTS entity-script-binding §?(time-natives note) / scripting-abi §4's reading that
   E+0x29C is an entity timestamp: the time natives' `entity+668` value is the SAME field,
   consumed as elapsed-time — so it is entity-local elapsed credit, dual purpose.
⇒ demo live-pump contract = state-5 registry + vft+96 + F3::ServerUpdate with
   budget-shifted timestamps (dovetails reimpl session-14 frame-pump work; the +0x29C
   semantics is the correction to carry).

## 8. `Script_Small::SetGlobals` (vft+0xC) = AMX public-variable preload  [DONE]

Body `0x5A4A80(script, map<string,u32>*)`: if blob loaded, walk map nodes;
`sub_5CCBE0` = **amx_FindPubVar**(name,&idx) → `sub_5CE350` = **amx_GetAddr**(idx,&addr) →
`*addr = value`. Warnings: "Variable %s not found while preloading script %s.",
"Unable to resolve address of variable %s while preloading script %s.",
"Attempt to preload script variables into an unloaded script.". Loaded-module name kept at
Script+0x08 string (used by logs). ⇒ trigger initial-vars land in AMX **publics** (not
DefineGlob) — the `[Globals]` blocks visible in Scripts.grp strings are the script-side
declarations of this layer.

## 9. Remaining queue
1. ~~0x553470/0x553550 (GameServer cutscene broadcast pair)~~ **CLOSED by discovery — see 2025-10-20-session27-cutscene-pair-packets-77-78.md** (AMX StartCutscene/StopCutscene natives → world-vb vft+160/+164 + packet 77 {sceneId,map} / 78 {} broadcasts).
2. CloseTask sibling confirm. ~~pending~~ **DONE**: 0x45B940 → `GameServer::meth_0x46ED30` —
   exact OpenTask pattern (journal op + broadcast), §6 closed.
3. ~~Registry list ownership~~ **CLOSED — §10.**
4. ⚠ **OPEN RUNTIME QUESTION for the reimpl (flag-semantics, high impact):**
   `JE::World::Initialize` installs Combat (World+0x8C) ONLY when
   `[global@0x70BD90]->int_0x4 & 1` is set, and `GetWorldServerInstance_0x401150` THROWS
   otherwise. Which mode does the demo boot in? Static evidence is ambiguous
   (sys.ini has a [Server] section = local-server hosting likely ⇒ flag=1 ⇒ Combat pump LIVE,
   entity+0x28 stamps nonzero, EFDT parse active); several prior conclusions
   ("server-gated ⇒ inert in demo") flip if flag=1. One gdb read of [obj+4] at
   0x5B4F30-return answers it; please report. If flag=1: the demo IS a local-server run and
   the Combat round-scheduler is the real movement/attack pump — the reimpl's
   `World::ServerUpdate_0x545d60`-driven movement slice is then CORRECT as built.
5. `Year()` mode-0 reads `[World+0x100]->vft[+20]` as resident calendar — clashes with the
   boot-verified "+0x100 = GameStoreManager" row (slot-base arithmetic suspect after the
   COL-at-+8 layout at 0x668448). mode-1 is fully resolved (see scripting-abi §4):
   `new JE::Gregorian(0x38)` over the epoch block **World+0x14..0x28** (0x544D90) + setTime
   (vft+88, E+0x29C) + getYear (vft+20). Re-check +0x100's runtime vft at your GetYear bp.

### 9b. Initialize extras (same function, 0x546690 prologue, raw)
`World+0x48=0; World+0x4C = 0x56AF50(); World+0x50=0; World+0x54 = 0x56AF50()` — two
small clock/stat objects created at world-Initialize (0x56AF50 = the TickMilliseconds-region
allocator; session-12's wall-clock neighbor). Secondary Combat install/replacement path:
inline setters at 0x548D3A/0x548D4A — ⚠ RETRACTED in-session: those belong to
`cls_0x4a5be0::set-anim-speed` (its own +0x8C = float speed 1.0 from PlayAnim), NOT World.

## 10. Scheduler ⇄ global entity list identity + registration path (queue-3 closed)

Evidence assembled from raw stores + the xref census of `byte_70BE0C` (global entity list
sentinel) and `+0x70BE10` (count):
- **Registration happens in `JE::Entity::ctor_0x5513A0` @0x551813..0x551851** — entities
  self-link at CONSTRUCTION (parallel track's entity-ctor doc already pinned the link
  mechanics: `sub_433760(byte_70BE0C, byte_70BE0C[1], &rec)` list insert; their verdict
  "authoritative LIVE-ENTITY list" stands). No state-gated insertion exists — the scheduler
  filters state==5 at pump time (pass1 skips/unlinks otherwise; unlink+`--count` sites:
  scheduler pass1 for state==8 REMOVED nodes at 0x545DA5../5a94da).
- The scheduler's `+0x1C` "registry head" and the global list share the node shape
  `{prev,next,Entity*}` and the same counter byte `70BE10` decrement pattern ⇒
  **same list object** (scheduler holds a pointer to the sentinel + a cursor). *(strongly
  implied; the constructor writing +0x1C = &byte_70BE0C was not directly sighted —
  World+0x8C instance creation site remains open, likely GameServer init; queue item).*
- `byte_70BE24` = **second** global list (same node shape) walked by the World tick body
  (`0x545E71..`) for the *non-state-5* ServerUpdate pass — i.e. the engine keeps TWO lists:
  the pumping registry (70BE0C) and a still/limbo list (70BE24, entities skipping the main
  budget pump when `state != 5` … pattern `if (v14[120] != 5) ServerUpdate(...)`).
  ⚠ refine: which lifecycle state migrates entities between 70BE0C/70BE24 — one raw pass
  next session.
- `JE::Entity::meth_0x54EF30` = **add-to-limbo(70BE24)**, idempotent via entity flag byte
  (**E+0x1E8**, set at 0x54EF3C; also cleared per 0x54BF01/0F readers) and sets global
  `byte_70BDE8 = 1` (limbo-dirty). CALLERS = the migration answer: `Entity::ChangeState
  0x5507F0@0x550954`, `Entity::AddAction 0x550720@0x55073C`, `Entity::SetMode
  0x550750@0x550768` (the ctor's mode=5 call!), effect-apply `0x5506C0@0x550713`, and
  `virt_meth_0x552420@0x55249B/699` (entity setup/load) — i.e. every lifecycle
  transition pushes the entity onto the limbo list; the World tick drains it (non-state-5
  entities get vft+96 + F3::ServerUpdate there — the "still pumps" for CREATED/ACTIVATING/
  DYING states), while Combat's registry pass covers SPAWNED ones.
- `cls_0x5a9330::meth_0x5a9330(mode)` (Combat broadcast helper — see §7 identity): walks 70BE0C calling
  per-entity handlers gated on byte fields (`E+0x296`-ish pattern `gap0[662]`/`gap188[270]` —
  IDA indexing ambiguous, refine when its callers surface; only caller = scheduler step mode).
- Consumers of the census (GameClient meth_0x4615F0, Server virt_meth_0x5552C0/0x556EA0,
  Map/World init 0x546870=ReloadConfig, camera commands CommandCameraReset/SetTarget,
  0x54BE10 netId-resolver) — all walk the SAME global list ⇒ the reimpl's registry should be
  one list + the 0x54BE10 by-id resolver, matching engine behavior. *(their entity-ctor doc
  covers ctor anatomy; this section only adds the scheduler linkage.)*

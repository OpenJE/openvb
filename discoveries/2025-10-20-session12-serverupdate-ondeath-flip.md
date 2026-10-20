# Session 12 — `F3::ServerUpdate` pump dispatch decoded: the 6→7 DEAD flip located

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes: session 8 §7 "who flips state 6→7"; session 9 §4 same item; refines
`2025-10-20-serverupdate-pump-unpackstate-state-census.md` §1 with the exact slot dispatch +
correction of `door-rebase-correction-changestate-machine.md` §callers claim that 0x550ad0
"itself changes state" (it does not — see §3).

## 1. `F3::ServerUpdate` @0x549130-0x549217 — per-entity frame pump = state machine driver

Raw dispatch (function body @0x549157; `esi` = entity interface base, `[esi]` = Entity vft):
```
switch (state @Entity+0x1E0)                      ; 0x549157 mov eax,[esi+1E0h]; cmp 8; jumptable
  0: throw Common::Exception("Trying to update entity when its state ...")   ; INVALID never pumps
  1: vft[+0xB8] (idx46 = 0x550970)  ; CREATED  -> RunScript + ChangeState(4)
  2: vft[+0xC0] (idx48 = 0x550a50)  ; DESTROY  -> RunScript "OnDestroy" + ChangeState(8)
  3: vft[+0xC4] (idx49 = 0x550a90)  ; CREATED-ALT -> RunScript + ChangeState(4)
  4,5: vft[+0xC8] (idx50 = 0x54c570) ; ACTIV/SPAWNED -> normal per-frame update
  6: vft[+0xBC] (idx47 = 0x5509b0)  ; DYING -> **OnDeath handler** (see §2)  <-- the 6→7 edge
  7: if byte [esi+0x1BC] != 0: vft[+0xC8]        ; DEAD still pumps while world-link alive
  8: vft[+0xD0] (idx52 = 0x550ad0)  ; REMOVED -> deep update/cleanup tail; also
                                     ; writes 0 to Entity+0x2D4 every pump case (loc_5491E2)
```
(All dispatch pushes `edi` = frame-delta param; head @0x549130: `edi = param_1` then
`vft[+0x68](idx26 = 0x54ab50)(param)` runs BEFORE the state switch = pre-update hook; tail
loc_5491E2 writes `Entity+0x2D4 = param_1` (frame delta timestamp) on every case.)

## 2. OnDeath handler `JE::Entity::OnDeath_0x5509b0` (vft idx47, slot +0xBC)

Raw @0x5509b0-0x550a41 (IDA owner-label currently wrong — it merges this into
`F3::GameAmmo::virt_meth_0x4083d0`, another owner-misattribution like anim-doc §):
1. `JE::Entity::RunScript(this, "OnDeath", 1, &out)` @0x5509d1 — script hook with the pump's
   frame param.
2. Server gate: if world-server flag clear → throw
   `"Invalid attempt to retrieve world server instance."` @0x5509e5 (so 6→7 IS server-only;
   client-mode demo entities never flip to 7 — consistent with the ctor-raw-5 frozen-state
   finding of session 8).
3. `obj = worldServer(0x66856C-iface) vft[+0x84](this->Entity+0x1DC netId)` @0x550a14-0x550a2a
   — world-server **by-netId lookup** of the dying entity's registry entry.
4. `meth_0x56a1a0(obj)` @0x550a2f — mark/deregister that entry.
5. `this->ChangeState(7)` @0x550a38 — **the one and only ChangeState(7) call site in the
   binary** (raw scan of all 11 direct E8 callers + no indirect SetMode(7) sites found this
   session; validates "6→7 only" lattice rule).

## 3. Caller-set corrections

- `door-rebase-correction-changestate-machine.md` listed 0x550998/0x550a78/0x550ab8 as "inside
  `JE::Entity::virt_meth_0x550ad0` — the engine server update, so the pump itself changes
  state". **Correction:** those three call sites are inside the lifecycle wrappers
  (Activate@0x550970→4, OnDestroy@0x550a50→8, OnLoad@0x550a90→4); `0x550ad0` (vft idx52) is a
  big per-frame update body containing NO ChangeState calls (decompiled fully this session,
  0x550ad0..0x551284: model-instance anim ticks, octree water queries, timed-object lists
  `Entity+0x3C4/0x3CC/0x33C/0x390+20` — it is the REMOVED-state cleanup/tick path). The pump
  that *selects* the wrappers is `F3::ServerUpdate` (§1), not 0x550ad0.
- Session 8 §2's table (ChangeState callers and pushed values) verified again at byte level:
  push values = {4,7,8,4,1,2,4(cond),6,4,4,5} — the `push 5` case @0x5a987b (JE::Combat region)
  is new info: a combat-flow path legally sets SPAWNED(5) through ChangeState — chase only if
  the reimpl ports combat init.

## 4. Reimpl-actionable summary

- Model death as: HP<=0 or DT-death → ChangeState(6) (Kill) → next pump tick, state-6 arm runs
  OnDeath: fire `OnDeath` script hook → registry deregister → ChangeState(7). In client-mode
  parity, state stays where you put it (server gate throws otherwise — the original never
  reaches 7 in the shipped demo).
- Corpse persistence rule: state 7 pumps normal update (+0xC8) **iff Entity+0x1BC world-link
  byte != 0** — corpses vanish when their grid link clears.
- `Entity+0x2D4` is set to the pump's frame-delta param on every update (loc_5491E2) — last-
  update-time accumulator; note for the live-pump work (their current step).

## 5. Time source (session 7 §6 item) — CLOSED at usable granularity

- `GameWorld::virt_meth_0x470070` (world-vb vft+116) raw @0x470070: `sub ecx,[ecx-4];
  sub ecx,230h; mov eax,[ecx-12Ch]` — returns the pointer member at **World(vb−0x230)−0x12C**.
  Same slot used by `JE::World::ReloadConfig_0x546870` @0x546998 (`mov ecx,[esi-12Ch];
  call [ecx-vft+8]`) — the World's **calendar/clock singleton**.
- The class is `JE::Gregorian` (size 0x38 as allocated here; the DB's 344-byte struct is the
  full F3-derived variant; vft 0x69166c @0x544xxx cluster) — the
  engine's in-world time/date manager (name from RTTI string "Gregorian"). **CONFIRMED same
  session later: `JE::World::Update@0x544a90`'s seconds-carry sink `World+0x3C` is this very
  object — created via vb-vft idx4 factory (GameWorld override 0x46fda0 → sub_544D90 →
  `new Gregorian(0x38)`, ctor 0x544950) in World::Initialize — see
  `2025-10-20-session14-frame-pump-chain.md` §2/§4.** Evaluate (0x462620)
  therefore arms `Effect+0x90 = Entity+0x2A0 + Gregorian-time` — effects schedule on the
  **in-world calendar**, not wall-clock ms. Slot map observed: vft+8 = `Get_mbr_0x28`
  (0x544a10, scalar getter — likely epoch ms), vft+12 (0x544a30) decompiles as
  `return byte_6FF4F3[a1]` — the decompiler likely mis-modeled `this`; treat exact slot
  semantics as *(inferred)* pending a debugger read.
- Additionally: `JE::TickMilliseconds_0x56af50` = `return dword_70C2B8` — a **wall-clock ms
  accumulator** maintained by `F3::ProcessMessagesAndUpdateTime` @0x56af7c
  (`dword_70C2B8 += timeGetTime() − dword_70C2BC` in the message-loop idle branch; also
  referenced 0x56af6b). Consumers include World init/change-state, entity update vft+4
  (0x54dea0), PathManager::ProcessJobs, Graphics/Camera, Client mouse picking. The reimpl's
  live-pump can use `timeGetTime()` deltas identically.

## 6. Still open

- `GameEntity+0x18` consumers — parallel doc confirms save-only round-trip; no gameplay reader
  exists in F3.exe (likely a BIFF1/2 editor-era field). *(inferred, low value to chase further)*
- ~~Evaluate clock object at World−0x364~~ → CLOSED §5 (Gregorian singleton at
  World(vb−0x230)−0x12C; exact vft-slot semantics for the current-time read need a debugger pin).
- What `vft[+0x84]` (by-netId lookup) and `meth_0x56a1a0` do exactly on 0x66856C — decode next
  if server registry parity matters.

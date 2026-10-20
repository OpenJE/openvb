# 2025-10-20 — Session 11: ServerUpdate state dispatch, Entity::UnpackState wire snapshot, +0x1E0 reader census

Target: ORIGINAL_REFERENCE. Read-only. Names the state-machine values via the per-state
pump switch (completes the lattice in
[door-rebase-correction-changestate-machine](2025-10-20-door-rebase-correction-changestate-machine.md)
§3 and the drivers in A's
[session8-entity-lifecycle-state-die](2025-10-20-session8-entity-lifecycle-state-die.md)).

## 1. `F3::ServerUpdate` (JE::Entity per-frame pump) @0x549130-ish — switch on Entity+0x1E0

Raw behavior (decompile @0x549157, function head ~0x549130):

```
if (vft[+104](dt) ) {                      ; pre-update gate virtual
  if (*(this+0x3A0)) {                     ; byte at raw +928 = Entity+0x3A0 = "pump enabled"?
    switch (state@Entity+0x1E0):
      0 → throw "Trying to update entity when its state is invalid"
      1 → vft[+184](dt)                    ; state1 update   (post-Create, pre-OnCreate)
      2 → vft[+192](dt)                    ; state2 update   (DESTROYING corridor)
      3 → vft[+196](dt)                    ; state3 update   (alt-created variant)
      4,5 → if (Entity+0x1BC) vft[+200](dt) ; ACTIVE updates (5 = ctor-birth state)
      6 → vft[+188](dt)                    ; DYING update (Kill()→6)
      7 → if (Entity+0x1BC) vft[+200](dt)  ; DEAD still animates (corpse)
      8 → vft[+0xD0] (0x550ad0)                  ; REMOVED deep tick/cleanup (per session-12
                                                 ; ondeath-flip doc §1; earlier "noop" here
                                                 ; was WRONG — corrected, raw jumptable idx52)
      default → throw "Unknown state in Entity::ServerUpdate()\n"
    dt store: Entity+0x2D4 = pump param (raw loc_5491E2, every case — session-12 doc;
    supersedes this doc's earlier raw +181/+182 byte-offset reading)
    *(acc raw 182) += vft[+44](this)       ; per-entity anim-time accumulator (getter 0x408080
                                           ; returns [E+0x2D0])
  }
}
```
⇒ state working names (consistent with all writers/reads found by both agents):
**0=INVALID, 1=CREATED, 2=DESTROYING, 3=CREATED-ALT (client-only?), 4=ACTIVATED,
5=SPAWNED (ctor raw-state), 6=DYING (only legal →7), 7=DEAD (still pumps vft+200!), 8=REMOVED.**
The 4/5/7 gate on `Entity+0x1BC` non-null = the linked world-object handle (Create() sets
`Entity+0x16C = vft[+180]()` server-side per A's lifecycle doc; +0x1BC sibling — A should map
this in the grid work: vft[+176]/[+180] region @0x5518xx Create tail).

## 2. `JE::Entity::UnpackState(this, srcId, stream)` @0x54e690 — entity network snapshot IN

- Guard: throws `"UnpackState() method may only be called on invalid entities."` unless
  **state == 0** (@0x54e6ac..0x54e6d0) → deserialization happens only into INVALID entities
  (pairs with Create-from-network flow).
- Stream readers are `meth_0x45e500` (u32), `meth_0x45e5e0` (string), `meth_0x45e460` (byte)
  on the packet reader object (IDA mislabels it GameActionEquip — ownership-label caveat,
  same class of misattribution flagged for `GameActionEquip::meth_0x4630xx` in RESPONSES).
- Field order of the WIRE SNAPSHOT (this is the network format, mirror of PackState):

```
1  string                      → Entity name (gap1AC[20] = Entity+0x1C0)
2  u32                          → Entity+0x1E0 STATE  (written DIRECTLY, bypassing ChangeState)
3  u32, u32                     → Entity+0x1E4, +0x1EC  (⚠ +0x1EC == reimpl mapId ✓ matches!)
4  3×u32 (v9/v10/v11)           → SetPos via meth_0x54a8a0   (== reimpl pos @+4.. floats ✓)
5  3×u32 (v12/v13/v14)          → SetRot via meth_0x549fe0
6  string                       → Entity+0x228-ish (gap1AC[108])  ; art/model token candidate
7  string                       → gap1AC[136] (Entity+0x250)      ; EEN2 string #2 twin
8  string                       → gap1AC[164] (Entity+0x26C)      ; EEN2 string #3 twin
9  u32, u32                     → DynamicObject gap289[3],[7] (Entity+0x294/0x298-ish)
10 byte, byte                   → gap179[7], gap179[11]
11 byte-flags ×2                → gap289[13],[14]
12 u32 ×2                       → gap289[15],[19]
13 byte flag: if==1: ensure [Entity+0x168] (vft[+44] create-if-null) → child->vft[+36](stream)
14 model load: meth_0x54dc50(name-string-from-step-6)               ; the .veg/.amx bind
15 SetPos(v9..), SetRot(v12..)
16 srcId stored → Entity+0x1D0 (gap1AC[48])
```
⚠ offsets marked "ish": IDA's gap1AC/gap289 indexing folded through DynamicObject base —
the *order* is byte-level fact, per-field absolute offsets should be confirmed with a
lockstep dump before writing to them (this is A's lane anyway).

**Why this matters for the reimpl**: the EME2 map-record path plausibly feeds THIS unpacker
(create invalid entity → UnpackState with position/rot/state/mapId → Create → ChangeState),
which explains why EME2 records carry pos/rot/mapId/flag directly and why the ctor bakes
state 5 (server spawn) while net-scheduled entities arrive via state-0+UnpackState. Test in
the SpawnMove harness: does the original's Default_PC spawn path call UnpackState (breakpoint
0x54e690)?

## 3. +0x1E0 reader census (corrected scan; 29 raw hits, false-positive class identified)

Engine-side (JE::Entity own): 0x549157 (pump switch §1), 0x5507fa (ChangeState validation),
0x5518b5 (Create guard `==0`), 0x551b60-area (Destroy guard `!=0`), 0x54ab0c/0x54ab53
(`virt_meth_0x54ab00`: can-update predicate = `vft[+25] || state∈{1,2,3,6} || +0x2F0 || +0x2F4[32]`),
0x54a657, 0x54e6ac (UnpackState guard §2), 0x550998-wrapper reads via ChangeState.
GUI/interface cluster 0x4e4cdc/0x4e62ca/0x4e64a8/0x4e6dca/0x4f2xxx/0x4f3b09/0x4f4bdc/0x4f6fae/
0x5129a6 — interface screens gating on entity state (B's lane; likely inventory/dialogue
"dead?" checks).
⚠ **False-positive class found**: `8b 8? ?e0 01 00 00` matches include Map dtor 0x57af9b
(`mov ecx,[eax+78h]`-adjacent bytes) — disp32 scans need SIB/modrm context checks; the raw-scan
recipes in earlier docs now carry this caveat.

## 4. Answers for the implementers
- **A**: §1 gives you the per-state virtual slots (184/192/196/188/200) — your pump
  implementation should dispatch exactly this way; the `Entity+0x1BC` gate + dt-store at
  +0x3A0 pump-enable byte and the raw +181/+182 dt/accumulator pair ("+= vft[+44](this)") is
  the entity-local clock your Action layer
  timeouts can share. §2 is the deserialization order to mirror for net/load spawns.
- **B**: the state==7 corpse-pumps detail matters for death UI; the 0x4f2xxx readers are your
  on-ramp to name the interface-side state uses.

## 5. Evidence index — pass 1
Pump 0x549130..0x549217 (switch @0x549166, throws 0x549177/0x549207, vft dispatch calls
0x549191..0x5491d7, time accum 0x5491e2..0x5491ef).

## 6. Addendum (session 12): the per-state handler slots RESOLVED (engine defaults vs F3 overrides)

**Note: `2025-10-20-session12-serverupdate-ondeath-flip.md` (parallel lane) covers this ground
with rawer evidence (jumptable indices, 0x2D4 dt store, the sole-ChangeState(7) proof). Read
that first; what follows survives as (a) the engine-vs-GameEntity override table §6 (their doc
covers defaults only) and (b) the UpdateActive §6.1 section map.**

⚠ Slot values must be read from each class's **Entity-vb vft** (GameEntity 0x666044 etc.,
class-vft-matrix table) — the engine table 0x692084 holds only the JE::Entity defaults.
For GameEntity (dword offsets = byte offsets on the Entity interface vft):

| state | slot | engine default | GameEntity override | behaviour |
|-------|------|----------------|---------------------|-----------|
| 1 CREATED | +184 | 0x550970: RunScript **OnCreate** → ChangeState(4) | 0x467ad0 → sub_467640 **ACTIVATE** (vft-decode doc §2.2) | F3 entities skip OnCreate script; activate = ApplyOnCreateEffects + animState=5 + desync QueueAnimEvent(1,rand) |
| 2 DESTROYING | +192 | 0x550a50: RunScript **OnDestroy** → ChangeState(8) | not overridden (GE +0xC0 = engine default) | same as engine |
| 3 CREATED-ALT | +196 | 0x550a90: RunScript **OnLoad** → ChangeState(4) | +0xC4 → not overridden | load-spawned entities (net/loaded path) |
| 6 DYING | +188 | 0x5509b0 = full **OnDeath**: RunScript OnDeath (server-gated, throws "Invalid attempt…"), WorldServer vft[+132] create-action, `GameActionEquip::meth_0x56a1a0(a, entity.mbr@+119raw)`, ChangeState(**7**) | 0x467a60 → re-thunk of the SAME body | death handler = DYING-state pump |
| 4/5/7 | +200 | **0x54c570** (below) | +0xC8 → not overridden (corpse animates with same body) | active per-frame |

(Slot arithmetic: +184 = 0xB8 etc. against 0x666044 dump in the vft-decode doc §1.1 —
verify ±160/4 confusion once in the IDB; the four engine bodies themselves are confirmed.)

**Activate→state-4 path resolved**: activate's tail `meth_0x550720` @0x550720 is NOT a state
setter — it pushes the activation record into the per-entity **encounter ring at Entity+0x304**
(`cls_0x54e070::meth_0x54ea00` push; `rec+20 = this`) and calls `meth_0x54ef30` (same post-hook
ChangeState uses). `JE::Combat` flush (0x5a941a/0x5a9440/0x5a987b, A's lifecycle doc §2) drains
those and calls ChangeState(4). ⇒ **state1→4 for F3 goes via encounter flush, not the pump.**

### 6.1 `Entity::UpdateActive` @0x54c570 (states 4/5/7 per-frame body) — section map
```
1. ACTION QUEUE drain: ring cursor Entity+0x2E0/0x2EC/0x2F0 (4-slots-per-chunk indexed list):
   per action obj: meth_0x55afb0() ; if done-virtual vft[+0] → erase + meth_0x55a570, set
   Entity+0x171 byte=1 (queue-changed flag?)
2. MESSAGE queue: if Entity+0x37C list non-empty → store msg id @gap384, RunScript
   **"OnMessage"**(msg args), unlink+free node   @0x54c643..0x54c6c4
3. CUTSCENE/UPDATE scripts (gated on game-options global_area_map_icons[0]->int_0x18+28900):
   service-intf vft[+152]() mode: 1→**OnCutsceneStart**, 2→**OnCutscene**, 3→**OnCutsceneEnd**
   (each with {prevmode, dt}); mode 0 → **OnUpdate(dt)**   @0x54c6d7..0x54c777
4. PATH/GRIDJOB list @Entity+0x2F4+16: per-entry `sub_55A1C0(job, dt, budget)` decrement from
   Entity+0x17C(?) budget float; completed entries freed; stuck flag byte gap2F4[36]/[37]
   @0x54c77c..0x54c86d
5. if `vft[+27]` done && !gap2F4[36] → meth_0x54bf00 (movement-finished post hook)  @0x54c883
6. POSITION RECORD QUEUE: 24-byte records {p0..p4 + byte} in deque Entity+0x1AC[0..16]:
   if byte flag Entity+0x188: pop head → meth_0x548d80(x,y,z,?,?) = SETPOS push   @0x54c891..0x54c93d
   else timed variant: service vft[0]() vs schedule fields Entity+0x184?? (gap179[7]/[11]) →
   pop when due; if queue empty: vft[+20]/vft[+22] movement-result hooks → meth_0x548d80 @0x54c952..0x54ca2b
```
⇒ For A's live pump: **this single function is the state-4/5/7 contract** — action queue,
OnMessage/OnUpdate/OnCutscene* script surface (names verbatim from 0x54c747..0x54c777),
pathjob budget drain, and the pos-record deque feeding `meth_0x548d80`. The queue-empty
movement hooks (vft+20/+22 on the DynamicObject table — F3 thunks at 0x467e30→Entity::meth_0x467be0
region) are how MOVE-completion notifies actions: same table your ActionMoveTo reimpl calls.

## 7. Evidence index addendum
Engine defaults 0x550970 (OnCreate→4 @0x55098f/0x550998), 0x5509b0 (OnDeath chain @0x5509d1..
0x550a38), 0x550a50 (OnDestroy→8), 0x550a90 (OnLoad→4); encounter push 0x550720
(ring@+0x304 via 0x54ea00, hook 0x54ef30); UpdateActive 0x54c570 sections 1–6 at
0x54c597/0x54c643/0x54c6d7/0x54c77c/0x54c883/0x54c891; slot dump table 0x692084+{b8,bc,c0,
c4,c8} vs GameEntity-vb 0x666044 entries from the vft-decode doc.
Pass-1 residual (restored):
UnpackState 0x54e690 (guard 0x54e6ac,
state write 0x54e733, child unpack 0x54e921..0x54e944, model bind 0x54e95a). can-update
0x54ab00 (@0x54ab0c/0x54ab30). Raw scan method + false-positive note §3.

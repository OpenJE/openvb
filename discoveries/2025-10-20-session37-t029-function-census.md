# Session 37 — T-029 function census: the denominator is 8,876 game functions (10,173 total in the IDB); 46.7% semantically named, 26.4% type-stub, 24.6% still `sub_` — plus the 171-entry DB/IDA disagreement

Target: ORIGINAL_REFERENCE IDA MCP (`ida_original_f3`, route A) + `artifacts/F3.json` (route B).
Read-only. Re-measurable; every number below states its route.

## 1. Route A — full IDB enumeration via `mcp__ida_original_f3__list_funcs`

Route: paginated `{count:1500, offset:0..9000}`, 7 pages, terminating page returned
`next_offset: null` at 1,173 items. Integrity checks: 10,173 items collected, **10,173 unique
addresses (zero duplicates), monotonic address ordering across page boundaries** (verified at
each of the 6 joins). Total IDB functions = **10,173**.

**Region split (boundary rule, stated because it is a judgment):** functions at VA < **0x5FD000**
= 8,876; ≥ 0x5FD000 = 1,297. The upper run is the link tail: MSVC-CRT/D3DX static blobs
(`sub_5FFxxx`–`sub_605xxx`, mangled `?name@D3DX`, `unknown_libname_*`) and `atexit`/global-init
thunks (`call_*`, `F3::atexit_thunk_*`). Caveat proving the boundary is soft: 89 mangled CRT
names and interleaved MFC pieces (e.g. `?GetFirstVisibleTabNum@CMFCTabCtrl@@…` @**0x4066b0**,
`??0Iostream_init@@QAE@XZ` @0x4066b0 area) sit *inside* game text — statically linked fragments
placed early by the linker. For the coverage denominator below I keep all 8,876 (honest: the
CRT-in-text residue is <1.5% of it and inflates "unnamed", i.e. conservative for us).

Name-class buckets over the whole IDB (route A, by name pattern):
`Namespace::Class::name`-form **6,747** · `sub_` **2,496** (2,177 in game region) ·
plain descriptive names (e.g. `JE_IntSqrt_5601F0`, `F3::CommandPipBoy`) **582** (125 in game
region) · C-mangled `?…` **139** · thunks `j_`/`call_` **40** · `unknown_libname_` **134** ·
`nullsub_` **35**.

**Game-region (8,876) three-tier split — the quotable coverage:**
| tier | definition | count | share |
|---|---|---|---|
| **T1 semantic** | `JE::`/`F3::`/`net::`-namespaced + hand plain names (`JE_`, `F3_` prefix) | **4,149** | **46.7%** |
| **T2 typed-stub** | owned by anonymous `cls_0xADDR::` classes (type applied, identity not) | **2,340** | **26.4%** |
| **T3 unnamed** | `sub_` + residue (`nullsub` 1, interleaved CRT/mangled ~228 incl. std-in-text) | **2,387** | **26.9%** |

(T2+T1 = 73.1% carry a type-DB class; the "about 3%, probably" class-count proxy badly
understates function-level identification — the 3% was **port** coverage of classes, not RE
coverage of functions. Both numbers are now measurable and they measure different things.)

## 2. Route B — `artifacts/F3.json` cross-check (scope item 2), and the disagreement IS the finding

- DB declares **6,624 unique method EAs** (1,101 structures).
- **6,453** exist as distinct functions in route A's list.
- **171 do NOT** — they are mid-function addresses inside another IDA function
  (examples: 0x411f00/0x411f10 lie within `F3::GameProjectile::virt_meth_0x411ef0`;
  0x408d30 within `F3::GameContainer::virt_meth_0x408d20`). These are vcall/branch targets the
  type-DB tooling recorded as methods; IDA legitimately merges the tails. So the two routes
  agree to **97.4%** and the 2.6% disagreement is a *known artifact class, not missing code* —
  recorded because a future census that ignores it will see 171 "vanished" functions.
- Reverse direction: **3,720** IDA functions are not DB methods (2,495 `sub_`, 632 plain, 558
  mangled, 35 nullsub) — plain free functions (console commands `F3::Command*`, natives) plus
  the unnamed mass.

## 3. Vftable slot resolution (scope item 1, 451-class question)

Route B: **2,156 vftable entries** across the DB's vftabled classes.
- **2,114** (98.1%) target an address that exists as an IDA function (route A join).
- **2,112** (97.9%) target a function whose name is not `sub_` ⇒ identified.
- Only **44** slots point at unnamed functions or nothing ⇒ "vtable-driven RE" is nearly
  complete; the fog is in free/plain code and stub classes, not dispatch tables.

## 4. Ranking "what's left" (scope item 3)

Each of the 2,177 game `sub_` bucketed by nearest preceding named function's owner class:
**1,971 distinct owner neighbourhoods** — the unnamed mass is NOT a few big blobs. Top 15:
JE::GUI 24 · F3::GameActionEquip 23 · JE::ResourceSystem 16 · JE::Entity 13 · JE::Map 13 ·
JE::World 7 · JE::Gfx_ParticleSystem 7 · JE::CEngineInterface 6 · JE::CounterControl 6 ·
F3::GameClient 5 · F3::GameEntity 5 · net::critical_section 5 · F3::GameWeapon 4 ·
F3::GameServer 4 · JE::VFX_ParameterInfo 4 (+ JE::Gfx_Water 4, JE::CSoundSystem 4,
JE::CameraControl 4, net::connection 4).
Interpretation for triage: even attacking the top 15 clusters fully only names ~150 of 2,177 —
the backlog is wide, so per-cluster sweeps (GUI, equip, resource) beat any "finish one class"
queue. The `cls_0xADDR` T2 tier is where cheap wins live: 2,340 functions already sit under a
type — naming those 300-odd classes (biggest: `cls_0x4d8d70` 138, `cls_0x50dbb0` 120,
`cls_0x55dc80` 94, `cls_0x50db20` 79, `cls_0x424a70` 54, `cls_0x4542d0` 50) converts ~80% of
one tier in a bounded number of identity decisions.

## 5. Re-measure recipe (weekly, no user help needed — answers the row's "idapython script" fallback)

1. `mcp__ida_original_f3__list_funcs {count:1500, offset:N}` iterate N=0,1500,… until the
   returned block is empty; the tool echoes the full JSON to a `/tmp/pi-mcp-*.txt` file even
   when the conversation view truncates — parse those files, not the view.
2. Unique-addr check + boundary monotonicity check as in §1.
3. Apply §1's tier rules and §2's EAs-vs-list join against `artifacts/F3.json`.
Everything is scripted-local after the MCP pulls; no idapython, no user action. (Route C,
idapython over the IDB, was NOT needed; noted so the next reader doesn't go look for it.)

## 6. One-line for the index

Game-function denominator **8,876** (of 10,173 IDB); semantic naming **46.7%**, typed-stub
**26.4%**, unnamed **26.9%**; DB/IDA method agreement **97.4%** with a 171-entry tail-alias
artifact class; vftable slot identification **97.9%**; unnamed backlog = 1,971 distinct
neighbourhoods, top cluster 24 — triage clusters/classes, not functions.

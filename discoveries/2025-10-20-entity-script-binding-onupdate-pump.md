# 2025-10-20 — Entity↔script binding lifecycle, Script_Small::Execute, OnUpdate pump, pause gate

Target: ORIGINAL_REFERENCE. Read-only. Follows `2025-10-20-scripting-abi-native-tables.md`
(same-day, this author). Queue items 1 & 3 of that doc land here; item 2 (AMX wrapper names)
partially.

## 1. `JE::Entity` script fields (raw-verified via GetScript/RunScript disasm; E = Entity-vb)

| offset | size | meaning | evidence (raw disp) |
|---|---|---|---|
| E+0x2A4 | 0x14B | **script name** std::string: buf@+0x2A4, **size@+0x2B4**, cap@+0x2B8 | 0x54AF0F size test; 0x54AF41 cap cmp |
| E+0x2BC | 4 | **`JE::Script_Small*`** (lazy, cached) | 0x54AF01/0x54AF3B store of CreateScript result |
| E+0x2C0 | 1 | "no script / load failed — don't retry" byte | set 0x54AF1F/0x54AF93; guard 0x54AEF3 |
| E+0x2C4 | ~0x8 | secondary attach string/data (passed to script vft+0xC when present) | 0x54AFBE lea 0x2C4 → `call [vft+0xC]` |
| E+0x2CC | 1 | attach-presence byte for the above | 0x54AFB4 test |
| E+0x1DC | 4 | entity **netId** — feeds Script_Small owner arg | 0x54AF2D `mov ecx,[esi+0x1DC]` → CreateScript(ecx) |

`JE::Entity::GetScript_54aef0` algorithm: if failure-byte → 0; if cached ptr → return; if name
size==0 → set failure-byte, return 0; else `Script_Small* = srvView->vft[+0x24](E+0x1DC)`
(= `CreateScript(ownerNetId)` — the +0x24 slot from the ABI doc §1; owner goes to Script+0x04),
`script->vft[+4](name, 1)` (Load), on success: if E+0x2CC set →
`script->vft[+0xC](&E+0x2C4)`; on failure: dtor script, cache=0, failure-byte=1,
log `"Entity::GetScript() Unable to load script: %s\n"`.

`JE::Entity::RunScript_54afe0(name, out, a3, a4)`: **throws
"Entity::RunScript() method may only be called by the server." when the server/editor flag is
clear** (demo ⇒ never called in practice — pump sites below are server-side paths), else
`GetScript` + `script->vft[+0x14](name, out, a3, a4)` = Execute (ABI doc slot 5).

⇒ **"myself" global == the entity's netId** (E+0x1DC → Script_Small+0x04 → AMX global tag
0x464C534D, LE bytes "MSLF"). Cross-pins: `Protagonist()` native returns exactly that number
(ABI doc §2); Year()/time natives space-test `myself & 0xE0000000` against it;
`0x54BE10` = netId→entity object resolver.

## 2. `Script_Small::Execute` (0x5A5A90, vft+0x14) semantics

```
if (!blob +0xD0) return 0
dbgState = amx_GetGlobal(tag 0x54415453, LE bytes "STAT")   ; only when mbr_0x24 (debug) set:
    state 2 → return 0 (script halted by debugger)
    state 4 → set state 1 (auto-resume) then single-call path
    state 3 → amx step-execute (sub_5CCE50(&amx,out,-2,0,…))
if (pauseGate(global[0]->int_0x18).meth(ownerId /*+0x04*/)) return 0   ; §3
name resolution:
    if strcmp(name,"OnUpdate")==0 and FQUE queue global (tag 0x45555146, LE "FQUE", = &Script+0xC0)
       has head:  effName = queued head string (advance via sub_580770 when not "busy"
                   meth_0x430800==0)                        ; TIME-SLICED script queue
    else effName = name
amx_FindPublic(&amx, effName, &idx)  ; sub_5CCA80 — nonzero ⇒ function absent ⇒ return 0 SILENTLY
rc = amx_Exec(&amx, out, idx, argc/args…)  ; sub_5CE310
return rc==0 || rc==12                     ; 12 = ERR_NOTIMPLEMENTED tolerated? *(inferred)*
```

AMX core wrapper identification (mechanical, pending string-level confirm):
`sub_5CC810`=amx_Init, `sub_5CCDE0`=amx_Register, `sub_5CCD00`=amx_Const/DefineGlob(set),
`sub_5CCCC0`=amx_GetGlobal, `sub_5CCA80`=amx_FindPublic, `sub_5CE310`=amx_Exec(_ex),
`sub_5CE320`=set-debug-hook, `sub_5CCE50`=amx-step/abort control.

## 3. Pause/cutscene gate — `cls_0x5529e0::meth_0x5529e0(ownerId)`

Raw (`0x5529E0`): `return (mbr_0x70e5 && !mbr_0x70e6) || (mbr_0x70e8 && mbr_0x70e8 != owner)`.
Instance reachable as `subsystem_mgr[0]->int_0x18` (the DB's `global_area_map_icons[0]` is a
mislabel — same object F3 uses for the server-flag, AreaMapIcon::Get_mbr_0x4 getter family).
Semantics *(inferred, matches natives)*: +0x70e5 = "scripts paused", +0x70e6 = "exception
owner/allow", +0x70e8 = **cutscene lock owner id** — while nonzero, ONLY that entity's scripts
execute. Fed by `StartCutscene/StopCutscene/IsInCutscene` natives (0x5C9520/5C95B0/5C9610 —
decode bodies next if needed).

## 4. OnUpdate pump sites (who calls RunScript)

`"OnUpdate"` @0x691FD4 has 4 code xrefs:
1. **`JE::Entity::virt_meth_0x54C570` (UpdateActive)** @0x54C770–77:
   `RunScript(this, "OnUpdate", &out@esp+0x18, [esp+0x20])` then iterates the vector
   E+0x304/0x30C/0x310/0x314 (their session-12 UpdateActive body — this is inside the
   "OnUpdate script hook" step of the documented pump contract).
2. `JE::Map::meth_0x56E9E0` — map-level script OnUpdate (MAP scripts; same 4-call shape).
3. `Script_Small::Execute` itself (strcmp literal).
4. **`JE::ScriptTrigger::virt_meth_0x5B24D0`** — ScriptTrigger (engine trigger entity) runs
   OnUpdate; class discovered by this xref (queue: geometry of ScriptTrigger + its F3 use by
   GDOR/area triggers).

All server-flag-gated (throw in client mode) ⇒ **entity scripts never execute in the demo** —
consistent with every prior "server-gated" finding. ⚠ CORRECTION (same-day, see
`2025-10-20-scripttrigger-map-factory-cutscene.md` §3): **trigger scripts DO run in client
mode** via `ScriptTrigger::Update-dispatch` (OnUpdate/OnCutscene*/OnCutsceneStart/
OnCutsceneEnd) — the server-throw applies to `Entity::RunScript` only. Reimpl: implement the
storage (E+0x2A4..0x2CC) + lazy GetScript + no-op Execute path; full AMX VM only needed for
parity if you ever run server/editor mode **or if shipped Scripts.grp contains ESTR modules**.

## 5. `QueueFunction` closed loop

Native `QueueFunction_5ca4f0`: validates name <260, `sub_59AB20` extracts the AMX string arg,
gets FQUE global (= `&Script+0xC0`, the `cls_0x50db20` queue installed at ctor-registration),
pushes `{SSO string}` via `cls_0x50db20::meth_0x4c60a0`. Consumed per OnUpdate by Execute's
head-pop (§2) — **exactly one queued call per update when the queue isn't "busy"**
(meth_0x430800 busy check). `ClearFunctionQueue_5c9f80` = queue clear. This is the engine's
script time-slicing model (Van Buren "run next N bytes per tick" convention, *inferred*).

## 6. Questions for the reimpl agent

1. Do you want `ScriptTrigger` (E-class, OnUpdate-owning engine entity) mapped next? Its
   presence explains engine trigger volumes independently of F3 GDOR/GCON.
2. If you mirror the E+0x2A4..0x2CC script block in your entity, keep the failure byte
   semantics (`load once, never retry`) — the original logs and nulls rather than retrying.
3. `rc==12` tolerated in Execute — flag if your AMX port hits a code-12 path; likely
   `ERR_RANGE` from abort() and treated as clean stop *(inferred)*.

## 7. Queue (this author, next)
1. Cutscene natives bodies (0x5C9520/5B0/610) → exact writes to cls_0x5529e0 fields.
2. `cls_0x50db20` queue class full geometry (head/size/busy at +0/+4/+0xC?, meth_0x430800).
3. ScriptTrigger anatomy + who constructs it (EEN2/EENT link?).
4. Where script names come from in files (EENT? EEN2 tail?) — tie to een2-eeov-eent doc.

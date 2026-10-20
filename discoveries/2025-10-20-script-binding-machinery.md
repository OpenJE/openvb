# 2025-10-20 — Entity↔script binding machinery; Script_Small factory; tokenizer located (session 28)

Target: ORIGINAL_REFERENCE. Read-only, native IDA. Opens the scripting seam flagged in
sessions 11/12/27. Independently corroborates the reimpl's `entity+0x2BC = script ptr`
finding (their gameentity doc) from the engine side.

## 1. `JE::Entity::GetScript_0x54aef0` — the whole binding in one function

Entity script block (Entity-view offsets; matches session-18 ctor gap289 region):
```
E+0x2A4..0x2BA  std::string  script NAME (SSO; cap check at +0x2B7)
E+0x2BB?        byte flag    "eligible for load" (ctor param path; gate at 0x54af0f)
E+0x2BC         dword        SCRIPT POINTER (cached; = reimpl's 0x2BC ✓)
E+0x2C0         byte         "load failed/permanent" memo (set 1 on error & on give-up)
E+0x2C8..?      block        EXPORTED script variables (filled by vft[+12] export call)
```
Flow:
```
if (failed-memo) return 0
if (script) return script
if (!eligible) { failed=1; return script }                    // lazy-load once
id  = *(E+0x1DC)                                              ; ★ entity INSTANCE id = script key
script = IWorldServer(ws 0x66856C) vft[+36] (slot 9, 0x544B80)(id)
     → JE::World::virt_0x544b80: `new Script_Small(0xE0)` + Script_Small::ctor_0x5a7240(id)
cache at E+0x2BC
if (script->vft[+4](name, 1))          ; bind name → load script resource
     if (E+has-vars-flag) script->vft[+12](&E+varblock)   ; export script locals into entity
else LogDebug("Entity::GetScript() Unable to load script: %s"), delete (vft[0](1)), cache=0
```
⇒ **ws vft+36 is a per-entity Script_Small FACTORY, not a registry** (re-reading session 11
§1 case-15: its "mgr" wording is now corrected — case-15 attaches scripts exactly through
this path; `sub_436250` = "already has script?" reads E+0x2BC ✓ reimpl).

## 2. Script class surface (from call sites; RTTI-named classes exist)

`JE::Script_Small` (0xE0) — ctor(id); vft slots observed in use:
`+0` delete · `+4` Load/bind(name,bool) · `+12` ExportVars(target-block) · `+16` **CallFunction(name,…)**
(the talk path: session 12's `script->vft[+16]("GetInfo")`, returns bool "handled", +20 fetches the
result value — the `(*(*v50+24))("GetInfo",&out)` in server-handler id 81).
DB also carries `Script_Big` (full script flavor) and `ScriptListener` — sweep their method
lists next round.

## 3. Tokenizer located (script compiler side)

`0x56c0xx` (big frame 0x138, decompiler bails — heavy SEH): script LEXER diagnostics in
situ: `'<Bad Token>'` (0x699590), `"Couldn't find variable %s"` (0x69959c) around string-
assembly calls into the SSO helpers ⇒ ESF-style tokenizer/variable resolution lives ~0x56b750
region. This is where scripted combat effects (session 21 open: who else fills damage) will
land — scripts CALL into game funcs by name/id; the dispatcher is still to be found
(start from xrefs to Script_Big methods).

## 4. Cross-session ledger updated

- Session 11 §1 case-15 wording: "mgr" → "Script_Small factory" (corrected here).
- Session 12 §talk + session 21: script call surface now has a real class name & slots.
- Session 27 'Strings' literal @0x4c5eef: candidate for script string tables — check when
  walking Script_Big.

## 5. Queue (scripting line)

1. Script_Big vs Script_Small method diff (DB vftables) → the VM surface.
2. Script→game function dispatcher ("call" opcode → engine function table) — the thread that
   answers "who fills GameEffectDamage outside the attack path".
3. Script resource format (load path behind vft+4) — pairs with reimpl rht work.
4. Standing blocked items unchanged (runtime experiments with reimpl).

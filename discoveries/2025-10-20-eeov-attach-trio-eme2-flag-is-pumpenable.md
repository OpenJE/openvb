# 2025-10-20 — Session 22: EEOV attach trio NAMED + EME2 flag byte = pumpEnabled(0x3A0); EMAP header cross-ref

Target: ORIGINAL_REFERENCE (IDA). Read-only. Continues the map-load thread
(session 21); closes two of its §4 open items.

## 1. ⭐ The EME2 record flag byte is `Entity+0x3A0` — the "pump enabled" byte

Session 21 §2 step 7 stored the record's u8 flag at `v8->gap389[23]` (EA 0x57417e). Base
arithmetic: gap389 starts at Entity+0x389 (decompiler's DynamicObject-relative layout), +23 →
**Entity+0x3A0**. That is the exact slot the reimpl's C-12 message says it models as
`pumpEnabled` (and the engine's ServerUpdate pre-gate reads `[esi+0x3A0]` before the state
switch — session 11 §1 / session 16 doc's engine default table). ⇒ **The EME2 per-record flag
byte = "this entity pumps"; map records with flag=0 spawn non-updating (frozen) entities.**
Reimpl: if your `record.flag` is stored but not wired into your pump gate, wire it — Mainmenu
records all carry 1, Tutorial may not (worth a walk). Also corrects session 21 §4 item 3
(+0x3B0 guess → +0x3A0 exact).

## 2. EEOV attach trio — semantics resolved (session 14 §5 item)

- `sub_552A20` @0x552a20 = **global attach-instance counter**: `if (++dword_70BE60) return it;
  else dword=1; return 1` — wraps away from 0. One id per attach registration (whole-game).
- `JE::Entity::meth_0x5512b0(strName, id, flag)` @0x5512b0 = build attach instance:
  `sub_4E1DE0()` allocs, `sub_4E2AD0(..., strName, v9=flt[6FF0D8])` → **VFX_VEGInstance-style
  object** (IDA types elsewhere as `JE::VFX_VEGInstance` — see footstep `.veg` usage of the
  same helper in the GE OnAnimEvent dispatcher, session 5 §1.1 +0xA4 FL/FR branch), then
  `meth_0x54ef90(this, inst, 1, 1, id, flag)` = register-attached-object on the entity.
- ⇒ EEOV v0's `m × { string → attach }` block = **per-entity attached visual effects**
  (`.veg` names), each stamped with a global unique id — same machinery the footstep FX use
  at runtime. This is what the map-side EEOV `m` loop populates (and the legacy EENT twin).
- Same call trio also feeds the `sub_552A20(1, ...)` shape seen at 0x552287: args (int 1) —
  the counter ignores them (stdcall cleanup), confirming pure-counter role.

## 3. EMAP header reader 0x56c9d0 — cross-reference (B territory, one confirmation)

Field order (verbatim, versions gated): `u32 Map+0x28; string +0x2C; string +0x4C; string
+0x68; string +0x90; v≥1: 3 fog bytes (with the rotate-then-+0x116=0x80 quirk); v≥2: bool
+0x1D8; v≥3: 4 bytes →+0x113..116; v≥4: bool +0x1D9; v≥5: bool +0x1B1, 4 bytes → composite
+0x1B4, u32×3 +0x1B8/BC/C0, then mirror block +0x1C4..+0x1D4`. This is the function whose
`0xFFFFFF00` constant trap implementer-B already documented from the opposite side
(their from-B note) — **matches exactly**; no delta to report, just the pin that the mirror
writes (+0x1CC/1D0/1D4 = copies of +0x1B8..C0, and float-mirror at +0x1C8 of the composite)
are part of shipped behavior (their CopyFloats-bug fix must preserve them). Four header
STRINGS (+0x2C/4C/68/90) = candidates name/description/music/… — the reimpl's EMAP v5 reader
should already store them; names TBD by consumers (UI/docbook).

## 4. EMEN(18) vs EME2(19) diff — DONE (closes session-21 §4 item 1) [VERIFIED against full decompile]

EMEN @0x573bf0 opens with `"Warning: reading obsolete entity chunk from map file.\n"` — the
map-side twin of EENT (obsolete, pre-EME2). Actual differences from EME2:
```
Create(ent, map, name, 0, 0)          ; flag params hard-0 (no record flag)
NO pump-flag u8 byte                  ; → Entity+0x3A0 left at engine default in this path
pos 3×u32 → SetPosition               ; SAME as EME2 (my earlier claim of "no pos/rot" was
rot 3×f32 ×57.295776 → SetRotation    ; WRONG — corrected before publishing)
NO nested EEOV chunk                  ; no spawn-time contents/attach via EEOV
v ≥ 1: string → meth_0x54de00(ent, s)  ; script attach (SetScript?) — EME2 has no equivalent
v ≥ 2: u8   → meth_0x548d60(ent, b)   ; pump/processing-enable setter (same helper EEOV v0
                                       ; calls at 0x5521e1)
register Map+0x134/0x138              ; identical tail
```
⇒ obsolete path = EME2 minus EEOV/flag, plus versioned script-string + pump-byte fields.
Reimpl: port only if old maps appear; the two `meth_0x54de00`/`meth_0x548d60` are useful
named engine setters either way (script bind + pump-enable, both reachable from EEOV too).

## 5. Open (map-load thread)
- What the 4 EMAP header strings feed (UI/quest/log) — consumers scan.
- EMEF map-effect body 0x578fb0 + EMTR trigger 0x575050 (script-volume machinery — probably
  pairs with their cutscene-controller doc territory; claim check before starting).
- `meth_0x54ef90` (attach register body) — CLOSED by session 25 doc
  `2025-10-20-attach-register-entity-vfx-maps.md` (attach maps at Entity+0x338/0x344/0x350;
  UpdateActive synthesis; ⚠ Entity+0x1DC dual-reading open there §4)

## 6. Evidence index
Flag byte write 0x57417e; pump gate read in ServerUpdate (session 11 doc §1) and engine
default 0x54ab50 can-update (session 16 doc). Counter 0x552a20 (byte_70BE60), attach
0x5512b0 (helpers 0x4E1DE0/0x4E2AD0, register 0x54ef90 @0x5512e0). EMAP 0x56c9d0 (version
gates 0x56ca4f/AD1/B75/BA4; compositing 0x56cc4d; mirrors 0x56ccb5..0x56cce0).

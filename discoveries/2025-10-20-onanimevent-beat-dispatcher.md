# 2025-10-20 — OnAnimEvent beat dispatcher: token→action-commit table (session 20)

Target: ORIGINAL_REFERENCE. Read-only, native IDA. `sub_467E90` = `GameEntity` vft+0xA4
(OnAnimEvent(token-record), sessions 1/8 flagged it) — fully decoded. Resolves the
reimpl's queued "when does an attack actually apply damage at the DM beat" question:
**the beats only COMMIT actions / spawn presentation; DM just flags the running
GameActionAttack.**

## 1. Common shape

Record = `{..., int@+0x10, {SSO-string@+0x18, cap@+0x2C}}` (reimpl's "token record" from
gameentity-vft-decode §4 — same object). Current action fetched via
`sub_4677E0(this+vbadj−160)` = entity's active Action*, then RTTI-cast to the concrete
GameAction class per token. **Every commit is action-identity-guarded** (wrong action
running ⇒ no-op).

## 2. Token table

| token | gate | cast target | commit | semantics |
|-------|------|------------|--------|-----------|
| `DM` | **server/editor flag only** | `GameActionAttack` | `attack->byte +0x65 = 1` | "damage beat reached" flag on the running attack — damage *application* happens elsewhere (action step reads this flag) |
| `RL` | none | `GameActionAttack` | `sub_4017D0(action, rec->int@+0x10)` | reload beat, passes record int (round count? timing?) |
| `PU` | none | `GameActionPickUp` | `sub_407170(action)` | pick-up commit at anim beat |
| `EP` | none | `GameActionEquip` else `GameActionSetActiveWeapon` | `meth_0x405160` / `meth_0x407710` | equip / weapon-drawn commit |
| `UP` | none | `GameActionUnequip` else `GameActionSetActiveWeapon` | `meth_0x407bc0` / `meth_0x4077b0` | unequip / holster commit |
| `FL` / `FR` | none | — (no action needed) | see §3 | **footstep VFX**: left/right foot |
| others | — | — | ignored | |

## 3. FL/FR footstep chain (first fully-wireless client effect found)

```
v26 = *(GE − 160 + 0x14C)          ; entity field (+0x14C in this coord) — gait/anim id?
v28 = world3rdIntf(sub_401130)->vft[+84](v26)      ; lookup (surface/biome?)*
v29 = sub_574F20(v28, pos, "FX_FootL_Hum_Sand_Base_01.veg" | "FX_FootR_..._01.veg", 0, 1)
     ; create VEG instance (same pool as demo weather, session 11/19)
attach = *(*( *(GE+0x20-ish[vbadj+192-160]) +16 )) ; entity's scene-node/decal-owner
sub_4E0F70(attach, layer= 12(L) / 7(R), instance...)
sub_59F850(instance, 1, ...)                        ; start
```
Footstep FX are HARDCODED to the `Hum_Sand` set in this build (demo), keyed off FL/FR
anim events — the session-1 §5 `.rdata` strings now fully explained.

## 4. Consequences

- **DM semantics settled**: at the damage keyframe the SERVER side flips
  `GameActionAttack+0x65` byte; actual HP application must be driven by the attack action's
  own step reading that flag (next target: GameActionAttack step/`meth_0x4023c0` family —
  callers of CancelAttack/FinishAttack already named in DB from session 2 evidence).
- Client-only beats (PU/EP/UP/RL/FL/FR) = presentation commits → the reimpl's action layer
  needs the SAME two-way split: state commit on authority, cosmetic commit everywhere.
- `sub_4677E0` = `GameEntity::GetCurrentAction` (name it in notes) — central seam.

## 5. Queue update

1. GameActionAttack step body reading +0x65 → the damage application + effect-instance
   creation path (closes +0x15C-adjacent `+0x160/164`... and the 0x45c0c0 a2[+0x9C] origin).
2. Footstep biome string table (is FX_FootL..._Sand configurable per surface? one lookup
   at vft+84 suggests yes; demo fixed).
3. Standing: World trees attribution, clip names, damage Apply math (0x45c420+),
   mode-writer runtime watch.

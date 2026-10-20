# Session 17 — to-hit subsystem decoded (closes the reimpl's last combat-depth item) + GetTotalSkill formulas

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Answers `RESPONSES-from-reimpl-agent.md` ("Only remaining combat depth: to-hit internals
(0x402260)") from `attack-state-machine-damage-formula.md`. Also notes: their other open item,
"Apply's math tail 0x45c420+", was already fully decoded in
`2025-10-20-session7-effect-pipeline-and-damage-apply.md` §3 (.veg-per-DT table, voice switch,
OnDamage script event, GameActionDie creation) — cross-reference before spending more time there.

## 1. `GameActionAttack::RollToHit_0x402260` — the wrapper

```
chance = unarmed ? GameCreature::GetToHitUnarmed_0x409e30(...)
                 : GameWeapon::CalculateToHit_413560(...)
rand   = RandRange(0, 100)                      ; sub_48CD00
log    : "Hit/Miss: Chance = %d, Random = %d"   ; cls_0x558740 debug channel (0x80/128/0x80)
return chance >= rand                            ; HIT when chance ≥ roll (d100 style)
```

## 2. Weapon dispatcher `CalculateToHit_413560`

- mode record = `weapon+0x408` (`*(this+258)` dword-idx form ⇒ byte +0x408; the modeRec their
  formula doc references); NULL/absent mode → 0 + `"Recieved a NULL pointer in critical data."`
- `modeRec[+8] != 0` → **ranged** path (`CalculateRangedToHit_413150`), else **melee**
  (`CalculateMeleeToHit_412fe0`).

## 3. Melee to-hit `CalculateMeleeToHit_412fe0`

```
att = attacker GetTotalSkill(MELEE) + desc[10]        ; desc = attack descriptor (session 6 §5)
if desc[7] (uses-ammo): att += ammo(prototype).toHitBonus at +0x3C4   ; (964/4 = byte 0x3C4)
ev  = target->GetDerivedAttribute(EVASION)            ; creature only, else skip
range = GetEffectiveRange_412f50(desc)                ; see §5
if 3D-dist(attacker,target) > range: return 0         ; out of range = 0% chance
if part != 9: ev += PART_PENALTY[part]                ; table §6
chance = clamp(att − ev, 0, 95)
```
(The FatalError string reads "CalculateMeleeToHit - Couldn't find ammo..." @0x413067.)

## 4. Ranged to-hit `CalculateRangedToHit_413150`

- `desc[2] == 2` → **throwing**: skill = `GetTotalSkill(MELEE)`, range limit =
  attacker-derived `THROWING_RANGE (=3*STR)`;
  else **firearms**: `GetTotalSkill(FIREARMS)` + weapon effective range.
- ammo bonus same as §3 (+ammo+0x3C4).
- distance penalty (percent):
  ```
  v11 = (range >= 11) ? 200 / ((range − 10) / 5) : 0          ; per-range "units", integer div
  v14 = max(0, floor(0.2 * dist − 2))                          ; −2 − (dist × −0.2)
  penalty = v11 * v14
  ```
- `chance = clamp(skill + desc[10] + ammoBonus − (EVASION + PART_PENALTY[part≠9]) − penalty, 0, 95)`.
- **no** early-0 on out-of-range for guns (distance penalty instead); throwing path DOES use the
  same `GetEffectiveRange`-vs-dist gate? — no: range var is the derived THROWING_RANGE value but
  compared only through the penalty math above. *(inferred for throwing clamp; raw shows
  identical v9 usage for both sub-branches)*.

## 5. `GetEffectiveRange_412f50(desc)`

- no desc → mode record's own (weapon+0x408) ... pattern:
  `desc==NULL ? modeRec : desc`;
- `desc[7]==0` (no ammo): return `desc[11]` (base range field);
- `desc[7]` set and (`desc[7] > weapon+0x3B8`(capacity idx251→ +0x3B8) or `desc[8] > weapon+0x408?`(idx253=+0x3F4))
  → return **0** (bad ammo state);
- else `desc[11] + ammo[+0x3C8]` (ammo range modifier, idx242×4=968=+0x3C8).
- Out-of-range check returns chance 0 in melee/unarmed paths.
- Roll op (implementer-A's open "hit-test" assumption, now resolved): roll = RandRange(0,100)
  **inclusive 0..100 (101 outcomes)** @0x4022f4, hit = `chance >= roll` @0x402393.

## 6. Body-part aim penalty tables — IDENTICAL dwords

`PART_PENALTY[9]` at **0x64D4C0** (unarmed site) and **0x64F858** (weapon sites) both:
```
[0]=10  [1]=40 [2]=40 [3]=40 [4]=40  [5]=60  [6]=120  [7]=60  [8]=0x7F7FFFFE(~FLT_MAX)
```
part value 9 = "no called shot" → no penalty added (branch skipped). The +120 entry [6] = head
*(inferred — matches called-shot difficulty; cross-check the called-shot UI order in
body-part-table doc)*. [8] FLT_MAX = sentinel, never indexed with ints (only 0..7 + 9 used).
Two copies exist (unarmed vs weapon translation units) — same values; keep ONE table.

## 7. `GameCreature::GetTotalSkill_409830(skill)` — raw + attribute bonus (F2 formulas)

`raw = meth_0x409820(skill)` (stored skill), then:
| skill | total |
|---|---|
| FIREARMS, MELEE | raw + 50 |
| UNARMED | raw + END + 2·STR |
| BARTER, PERSUASION | raw + 3·CHA |
| DECEPTION | raw + INT + 2·CHA |
| MECHANICS, MEDIC | raw + PER + 2·INT |
| OUTDOORSMAN | raw + END + 2·PER |
| SCIENCE | raw + 3·INT |
| SNEAK, SECURITY | raw + AGI + 2·PER |
| STEAL | raw + PER + 2·AGI |
| unknown | debug "GameCreature::GetTotalSkill() - UNKNOWN SKILL: %d" |
Attribute slots = the confirmed SPECIAL block +0x760..+0x778 (session 6 §6) — the `+50` on
combat skills is the in-game "+50%" display convention baked into the math.

## 8. For the reimpl (implement-now summary)

```
toHit(attacker,target,desc,part):
  att = desc[10] + totalSkill(mode==throw? MELEE : weapon.mode[8]? FIREARMS : MELEE-or-unarmed)
  if usesAmmo: att += ammoPrototype.toHit(+0x3C4); range += ammoPrototype.range(+0x3C8)
  ev  = part<9 ? EVASION + PART_PENALTY[part] : EVASION
  penalty = ranged ? (200/((effRange−10)/5)) * max(0, 0.2*dist−2) : 0   ; melee: effRange<dist ⇒ 0
  chance = clamp(att − ev − penalty, 0, 95)
  hit = chance >= RandRange(0,100)
```
Plus the `"Hit/Miss: Chance = %d, Random = %d"` debug line (0x402316 area) — keep it; it's your
lockstep diffing signal for combat parity, same channel as the "Damage for %d queued." line.

## 9. Open follow-ups (low priority)

- ~~`meth_0x409820(skill)` storage table offset~~ → CLOSED: raw skill array =
  **GameCreature+0xE0 + 4·skillIdx** (`return *(this+8+216+4n)` @0x40982b) — sits right after
  the 7-stat block at +0xA0 from the GCRE docs (0xA0+0x3C = 0xDC, then skills at 0xE0: the
  +0xE0.. region holds 31 skill dwords until ~0x15C).
- Roll op confirmed & range corrected (see §4); DT-row selector = PART (session 6 §5 patched);
  base-skill array raw-locked **GameCreature+0xE0** (`mov eax,[ecx+eax*4+0E0h]` @0x409824) —
  ⚠ implementer-B's C-32 "216 (0xD8)" dropped the +8 gap-base of the decomp display; use 0xE0.
- Attack-descriptor field map consolidated: `[2]`=attack kind (2=throw), `[3]==1`=?
  (flag used in queue record), `[4]`=body part, `[6]`=style (1 punch/3 kick, per §4 of
  session 6 doc ammo interplay), `[7]`=uses-ammo, `[8]`=ammo-in-mag?, `[9]`=skill bonus,
  `[10]`=to-hit adder, `[11]`=range, `[14]/[15]`=damage dice count/sides, `+56..60` per
  their mode-rec doc.

## 10. CORRECTION — GameWeapon absolute layout, raw-verified (supersedes session 6 §4 cache
rows and casts doubt on the §9 GIWP rebase)

While pinning the to-hit ammo fields I re-checked the raw instruction encodings (objdump) and
found the decompiler's `*(this + N)` displays had been read with inconsistent units in earlier
notes. **Raw-truth weapon own-block layout (byte offsets, every one from a `mov/cmp [reg+disp]`
encoding in 0x412f79-0x413223 / 0x4136a4-0x4137cd / 0x407460s):**

| offset | field | raw evidence |
|---|---|---|
| +0x3B4 | primary caliber | `mov 0x3b4(weapon)` push @0x412f9c (ammo-lookup arg) |
| +0x3B8 | primary modifier | `mov 0x3b8` paired arg @0x412f96 |
| +0x3C0 | secondary caliber | `mov 0x3c0(weapon)` in UnloadSecondary cmps (decomp idx240×4) |
| +0x3C4 | secondary modifier | idx241×4 = 0x3C4 |
| +0x3EC | **primary CURRENT rounds** | `cmp desc[7], [w+0x3EC]` gate @0x412f79; reload `+= v7` at idx251×4 |
| +0x3F0 | **primary CAPACITY** | `cap − cur` @0x413b10 (idx252×4) |
| +0x3F4 | secondary CURRENT | `cmp desc[8], [w+0x3F4]` @0x412f84 |
| +0x3F8 | secondary capacity (inferred from trio) | |
| +0x424 | primary ammo-prototype CACHE ptr | `mov 0x424(esi)` @0x412f8c — **NOT +0x3C4** as session 6 §4 said |
| +0x428 | secondary cache ptr | UnloadSecondary `*(this+266)`→0 = idx266×4 |
Ammo own: to-hit bonus **+0x3C4** (`add 0x3c4(ammo),%esi` @0x413088/0x413223 — confirmed),
range modifier **+0x3C8**, pierce/armor **−0x3BC**, damage-type **−0x3B8**, material **−0x3B0**
per earlier raw reads (unchanged).

Consequences:
- session 6 §4 table rows `+0x3BC magazine capacity`, `+0x3C0 current`, `+0x3C4 cache`,
  `+0x3EC/+0x3F4 reload-delay` are **WRONG** → replace with the table above.
- session 6 §9's GIWP field-map assumed reader-base == weapon-base arithmetic; with the cache
  at +0x424 (BEYOND the 0x3B0s block end I had drawn at 0x408), the `sub_412CD0` reader's
  `this` is NOT the ammo-adjacent base I assumed. **Re-derive the GIWP wire→absolute mapping
  before implementing weapon load** (their port plan) — recipe: the reader writes at
  `this+944/952/956/960/964/968(str)/...` with `this = weapon + K`, solve K from ONE known
  anchor: `sub_4148A0` writes mode records — the cache/cur/cap fields above (+0x3EC..+0x428)
  are NOT wire-loaded (runtime state), so match a wire field to a raw-documented absolute
  offset (e.g. primary caliber 0x3B4 ← reader +944 ⇒ K = −0x14? ⇒ reader base = weapon−0x14).
  Flagged as session-18 follow-up; do not trust §9 rows until then.
- The to-hit §3/§4 formulas above ARE raw-corrected (ammo bonus +0x3C4, range mods
  desc[11]+ammo[+0x3C8], gates +0x3EC/+0x3F4).
- **RESOLVED NEXT ROUND:** the "GIWP rebase unresolved" flag is closed — reader base == weapon
  base (dispatch `lea ecx,-0x430(%esi)` + Weapon Entity-vb = +0x430). See
  `2025-10-20-session18-weapon-layout-final-giwp.md` for the final table (+0x3EC/+0x3F0
  primary cur/cap ARE serialized; GWAM count at +0x404; caches +0x424/+0x428 runtime-only).

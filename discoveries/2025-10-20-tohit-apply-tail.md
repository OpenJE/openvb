# 2025-10-20 — To-hit roll + Apply tail map (session 22)

Target: ORIGINAL_REFERENCE. Read-only. Complements session 21 (combat chain). Two leaves:
to-hit is now FULL; Apply's tail is structurally mapped, with exactly one store (the final
HP/state mutation site) left as the single combat open point, and why.

## 1. To-hit `GameActionAttack::meth_0x402260` — FULL

```
chance = unarmed?  GameCreature-side cls_0x409820::meth_0x409e30(creature, modeRec+24,
                                                            target, modeRec+20, weapon)
         else      GameWeapon::CalculateToHit_0x413560(weapon, modeRec+24, actorEntity,
                                                            target, modeRec+20)
roll   = sub_48CD00(0, 100)              ; same dice helper as damage (uniform int range)
debug  = "Hit/Miss: Chance = %d, Random = %d"   (overlay via cls_0x558740::meth_5583e0)
return chance >= roll                     ; inclusive percentile
```
⇒ to-hit is a single percentile roll; all skill/AC/evasion math is inside
`CalculateToHit_0x413560` (not yet opened — same shape as CalculateDamage: weapon modes +
ammo modifier). modeRec fields reused identically: +20 (ammo extra), +24 (ammo proto rec).

## 2. Apply (0x45c140) tail — structural map from raw disasm (decompiler fails on this
   function — stack-mismatch; window read via cached objdump)

Order after the hit-direction prelude (session 7 §3):
```
0x45c420  call 0x5512b0                      ; (entity helper — likely "get alive/state")
0x45c425  switch(effect+0xA8: 0..7)          ; DAMAGE TYPE switch, jump table @0x45c684
          each case: roll sub_48CD00(a,b)    ; per-type secondary effect chance
          (type-specific flags then set)     ; crit/crip/sever-style gates *(inferred)*
0x45c4f8  effect driver [eff+0x164... edi+0x164] → 0x48dc90   ; finish/notify plumbing
0x45c504  GetWorldServerInstance; call [ws].vft+0x74          ; server-side hook (slot ≥12
          ⇒ NOT 0x66856C — edx here is a DIFFERENT object; verify before naming)
0x45c52e  target (eff+0x48) vft+0xC  → 0x549CD0 = ModelInstance wrapper (model state set)
0x45c575  0x55a220                          ; packet/msg helper
0x45c59c  0x46d020                          ; F3 GameWorld-side helper
0x45c5b2  target vft+0x50 → 0x549030        ; model/state query
0x45c5c6  Entity::meth_0x54c440(target)     ; touch/wake
0x45c5e5  vft+0x14 → 0x549D10              ; model get
0x45c5f7  0x6165e0 (alloc/new-ish)          ; build payload/action
0x462626→ target->Entity::AddAction(0x550720, built action/msg)   ; ★ consequence QUEUED
```
⇒ Apply does NOT store HP inline: it (a) rolls per-type extra effects by damage type,
(b) pokes the target's model/state, and (c) **queues a follow-up action/message onto the
target** (AddAction) — matching the engine's everything-via-actions style. The HP/state
delta therefore lands in that queued action's OnServerUpdate (die/knockdown) or in a
ChangeState handler reached through target vft — THE one remaining combat leaf.

## 2b. Damage-type switch decoded (raw) — it selects a STATE/MESSAGE id, not HP

`sub_48CD00(ebx, 1)` = binary roll; per type (eff+0xA8, 0..7) computes a small enum id
passed in eax to `0x48dc90` (driver = `[edi+0x164]`, the same cls_0_48dc10 that
`cls_0_59f7d0` delegates to — the effect's own message/state sink):

| type | ids chosen (with roll variants) |
|------|--------------------------------|
| 0 | 1 or 2 |
| 1 | 10 or 11 |
| 2 | 5 or 6 |
| 3 | 38 (roll≠0) else 35/36 (second roll) |
| 4..7 | same pattern, targets 0x45c4b0/0x45c4dc(5,6 shared)/0x45c4e3 |

⇒ the switch = "which hurt-message/state flavor per damage type". HP remains untouched
here — consistent with §2's AddAction conclusion.

## 3. Combat remaining work (precise)

0. Object built before AddAction — **MECHANISM CLOSED, IDENTITY OPEN (corrected below)**:
   `0x6165e0` = `operator new(0x34, td@0x6CB008)` then `call 0x616328`. Apply fills
   `msg+0x1C = eff+0xA0`, `msg+0x20 = eff+0xA4`, `msg+0x28 = flagbyte`, then
   `Entity::AddAction(target, msg)` (0x550720 @0x45c626 with vb-adjust ⇒ target Entity view).
   ⇒ Damage consequence is delivered to the VICTIM as a small typed object; the victim-side
   handler performs the HP/state change (reimpl: one log line in the pump identifies it).
   ⚠ see item 5's RETRACTION before reusing "0x616328 = ctor/packet" anywhere.

1. Identify the object built at 0x45c5f7 + AddAction'd at 0x45c626 (ctor → class → its
   OnServerUpdate = the HP write). Approach: disasm 0x45c5f0..0x45c626 for the ctor call
   target; or decompile `meth_0x462620` (the gate called before Apply per session 7 slot10).
2. `CalculateToHit_0x413560` body (skill math).
3. The damage-type secondary rolls: constants at each 0x48CD00 call site give the
   per-type proc table (crit chances etc.) — mechanical, ~15 min.
4. Note for openje: `0x549CD0/0x549D10/0x549030` = ModelInstance-forwarding Entity virtuals
   (slots +0xC/+0x14/+0x50 of Entity vft 0x692084) — name them there.
5. **RETRACTION (verified later same day)**: `0x616328` is NOT a ctor and NOT net::packet —
   decompiled: it is a 16-byte **thisptr-adjust helper** (`v1=*(this-4); return this-*(v1+4)
   [+ *(&this->mbr_0x0 - v3)]`). IDA's class name `cls_0x616328` derives from that helper.
   The 0x34-byte AddAction payload is an UNIDENTIFIED class; the "wire id == message id"
   unification is UNVERIFIED — do not name or code against it until the object's vft is
   pinned (find the store to [obj+0] inside the 0x6165e0 callee chain).
6. NEW (same verification pass): **`0x546b50` = `JE::World::InitClock`** (slot 5 of
   GameWorld/IWorld vft 0x6685A4): looks up settings section `"World"` via `cls_0x56af40`
   (config registry — DB mislabel "VFX_Parameter"), keys and defaults:
   `Seconds Per Round` → World+0x40 (6), `Year` → +0x14, `Month` → +0x18, `Day` → +0x1C,
   `Hour` → +0x20, `Minute` → +0x24, `Second` → +0x28 (all j__atol). This finalizes
   session 14 §2's timing cluster and is the game-clock config surface (openje + reimpl
   sys.ini parity).

## 4. Status line

Combat pipeline: 95% mapped — click→packet→action→beats→to-hit(d100 percentile)→damage
formula→queue→effect-create→attach→tick→Apply(type rolls + model pokes + queued
consequence)→ [last hop: queued consequence → HP]. Everything else in the folder stands.

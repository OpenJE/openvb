# 2025-10-20 — T-014: the record `+0x19` mark is a per-record UNARMED snapshot, written by CalculateAndQueueDamage at queue time

Task: T-014 (board). Author: FastArrow (01a157ef). Target: ORIGINAL_REFERENCE, read-only.
Resolves the §5 caveat of `aoe-reaction-flags-ac-ad-answer.md` (db0ae6a): "the +0x19 mark's
writer not traced". Short answer: **there is no separate marking step — the field is the
action's `+0x71` (ResolveWeaponAndAttackMode out-param, the attack doc's own
"unarmed flag (from resolve) @0x402410" row) snapshotted into the record when the hit is
queued.**

## 1. Writer chain (raw)

`GameActionAttack::CalculateAndQueueDamage_0x404060`:
1. @0x4040A8 branch reads `mbr_bool_0x71` to pick unarmed-vs-weapon damage path
   (`CalculateUnarmedDamage_409f90` vs `CalculateDamage_413310`);
2. @0x40412A–0x404134: `LOBYTE(v7) = this->mbr_bool_0x71; … v22 : char = v7` — the snapshot
   goes into the stack record block (`v20[4]`@esp+14h, then v21@esp+24h, **v22@esp+25h**,
   v23@esp+26h, v24@esp+28h);
3. @0x404187 `cls_0x55dc80::meth_0x403D60(&action-list-triplet@+0xB4, key = {victim netId
   [a3+0x1DC], attacker netId})` → per-victim node; @0x4041A1 `sub_4B9AD0(begin, end, v20)`
   appends the block as a list node;
4. Node payload base = node+0x08 (finisher reads damage at `[node+0x08]` @0x4038B3 — matches
   v20[0]@esp+14h), therefore **esp+25h (v22) = node+0x19**. The finisher's count-scan
   `BYTE2(this->mbr_int_0x14)` @0x4037F0 reads exactly that byte.

So the finisher's `local_11 = (count(records with node+0x19) ≥ 2)` is: **"≥2 UNARMED hits
queued in this batch"** — and its product, first-hit `eff+0xAC = 1`, selects the heavier
stationary reaction pair {38,39}: the shipped semantic is a *flurry* rule, not a generic
multi-hit rule. Ranged volleys (bool_71 clear) never set +0xAC regardless of count.

## 2. Non-writer disambiguation (so nobody re-chases it)

The other two byte-stores to +0x19 in the TU — **0x404B4F** (`mov BYTE[eax+0x19],dl` in the
0x404B40 copy helper) and **0x404D03** (`mov cl,[esi+0x19]; mov [eax+0x19],cl` @0x404D00-03) —
belong to the **GameActionInitiateDialogue(0x24) clone-and-requeue path** (new 0x24 @0x404CE3,
copy-ctor call @0x404CF1, `AddAction 0x550720` @0x404D0A after
`SetPosition 0x55B350 + [x+0x30]=5` walk-to-retry at 0x404CC2-0xD1). InitiateDialogue's own
+0x18/+0x19 are its {whose-DLG, target} bytes — different struct, same offsets. Anyone
grep'ing `+0x19` stores will hit these; they are not volley records.

## 3. Consequences

- Reimpl (UltraBear's volley-batch model): the +0xAC condition must be
  `queuedUnarmedCount >= 2` (snapshot per record), not "≥2 records" — a three-shot WEAPON
  volley must stay {37,38}, only a ≥2 unarmed flurry upgrades to {38,39}. Observable:
  unarmed multi-hit anim id differs from weapon multi-hit under identical counts.
- The attack doc §1 line "+0x71 unarmed flag (from resolve)" is now load-bearing in two
  places (damage path select @0x4040A8 and reaction flavour via this mark) — keep it.
- My aoe answer §4 table row "+0x17 unarmed flag (string-source select)" gets an
  **unverified flag**: 0x40386B's `HIBYTE(mbr_int_0x14)` maps to a record byte whose
  node↔stack alignment I could not pin two independent ways; the +0x19/+0x11 identity here
  is the pinned one. (Not edited in the committed file — this doc is the delta, per protocol.)

## 4. Residual (one gdb/ASSERT line would close it 100%)

node+0x18 (record byte esp+24h = `modeRec*+0x0C == 1`, from v6[3] via the a2+0x18 sub-mode
object) has a second writer at 0x404D0F (`mov BYTE[esi+0x18],1`) in the same
dialogue-clone path — different struct again, same offset-aliasing trap as §2. If the reimpl
instruments, watch for node+0x19 == action+0x71 at the 0x4041A1 insert to confirm the
stack↔node base (esp+14h → +0x08) that §1 step 4 derives.

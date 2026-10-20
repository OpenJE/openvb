# Session 36 — T-010b verdict: KILLED. weapon+0x3B0 is a never-written dword (ctor sweep writes +0x3E0..+0x428, never +0x3B0; GIWP wire doesn't reach it; zero data writers) ⇒ reads 0 ⇒ the {9,10} limb-class guard is permanently-dead shipped code. Bonus: full shipped GITM taxonomy + eqslot table for all 51 weapons/tools/armor

Target: ORIGINAL_REFERENCE + shipped data (`build/data/`). Read-only. Verdict owed to
T-010b ("confirm or KILL against shipped item records — not a maybe").

## 1. The verdict chain (four independent legs, all raw)

Hypothesis was: weapon+0x3B0 = body-part weapon class (9 hand-type / 10 foot-type), read at
the four sites 0x409eaa/0x409eb8/0x40a02e/0x40a03c paired with mode+0x18 ∈ {1,3}.

- **L1 — no data writer exists:** whole-image stores to the field = ZERO (the four known
  accesses are loads; my session-18-era .text scans + this pass found no `mov [reg+3B0h]`
  store anywhere).
- **L2 — the constructor never initializes it:** `GameWeapon::ctor_0x414510` writes
  indices 236..266 = runtime +0x3E0..+0x428 (the ammo/magazine block) and
  `Entity::ctor(this+1072)`; nothing in the +0x218..+0x3D8 region (the GIWP wire's territory
  — session 18 map). ⇒ +0x3B0 keeps whatever the allocation gave it; every weapon object
  shares the same zeroed source (proto clone from a zero-initialized registry slot —
  session 9/18 clone path). Treat as constant 0.
- **L3 — shipped records contradict a class field:** all 51 weapon records dumped from
  `_WEA.grp`: the value 9/10 appears in **8 different payloads at 8 DIFFERENT GIWP offsets**
  (0x20 22Autoloader, 0x44 Baseballbat, 0x4c Minigun, 0x50 Hatchet, 0x80 Shiv, 0x98 Crowbar,
  0xa8 Machete, 0xe8 CombatKnife). A per-weapon class field would be ONE offset; eight
  names × eight offsets = unrelated small values (damage/counts), i.e. **no candidate field
  exists in the wire at all.**
- **L4 — no limb weapons shipped:** none of the 51 names is a knuckles/boots-type item.

⇒ **KILLED.** The guard `weapon+0x3B0 ∈ {9,10}` never fires for any loadable configuration;
combined with the already-ledgered fault at 0x40a045 on the branch's tail, the whole
{1,3}×{9,10} comparison block is dead shipped code. Reimpl guidance: keep the comparison
(mirror, cite, never improve — defect-ledger rule) with the constant-0 field or omit it with
a comment citing this file; **do NOT invent a limb-weapon-class field or wire slot.** §12 of
canonical should delete/downgrade the hypothesis entry to "killed — see session36".

## 2. Bonus — shipped GITM taxonomy (new, for the record-format lanes)

`GITM.type` (payload +12) across the demo set: **0 = tools/misc** (First_Aid_Kit, Lockpicks,
Tool_Kit), **2 = armor** (all `_ARM.grp`), **4 = weapons** (all `_WEA.grp`).
`GITM.equip` (u8 @+20) = can-be-worn flag (props: 0). `GITM.eqslot` (s32 @+24):
**0 = body armor, 1 = head armor, 2 = weapons AND tools** (all 51 weapons + 3 tools = 2)
⇒ equip-slot vocabulary: 0 torso / 1 head / 2 (weapon|tool hand) / 3..5 = the DR-invisible
rest (belt/back/eyes? — no shipped record uses 3..5; consistent with B7 "slots 2..5
DR-ignored" being harmless by data).
Full weapon roster dumped (51 records: guns/melee/throwables incl. 9mmMachinePistol2/4 and
APOLLO variants) — name list available if a prototype-table lane needs it.

## 3. Method note (harness-item-13 applied)

This was owed as a "maybe" and cost ~4 tool calls once the data question was asked first:
"could on-disk data answer this?" — yes, and it did better than the debugger route would
have (all 51 records, not one instance). Recording here because the coordinator's item-13
rule is what redirected me to it.

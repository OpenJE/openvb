# Session 15 — callers of GameEffectDamage vft[+8] (0x45c0c0) pinned: it's an effect-CLONE, and the runtime damage value never goes through it

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Answers `2025-10-20-entity-update-updater-maps.md` §4 item 1 (+§3 lead) and
`2025-10-20-onanimevent-beat-dispatcher.md` §5 item 1, using session 7/8 results + fresh
verification this session.

## 1. The +8 contract, re-pinned (all direct evidence this session)

`0x45c0c0` = `GameEffectDamage::InitFrom(args*)`:
- base `GameEffect::virt_0x4624f0(args)` @0x4624f0: `JE::Effect::virt_0x55ac10` (copies arg
  bytes +8..+0xB and the child/parent/sibling link dwords at +0xC/+0x10/+0x14, +0x28..+0x34)
  then **bulk copy args[+0x48..+0x98] → this+0x48..0x98 (21 dwords)**;
- then `this+0x9C = args[+0x9C]` (damage amount).
⇒ vft[+8] is a **prototype-clone ("init-from") per class**, NOT a tick, NOT entity-derived.
The `a2` argument is a **GameEffect instance of the same class** (an EFTR-tree prototype node),
never a GameEntity — the `a2[+0x9C] == GE+0x9C anim-lock` coincidence in their §3 is REFUTED
by the call sites below (wrong object family; GameEntity+0x9C is a byte, the clone source is
the 0xB0-byte effect where +0x9C is the dword damage).

## 2. Every caller (vft-slot dispatch sites for the GameEffectDamage family)

| site | context | arg passed | when it matters |
|---|---|---|---|
| `ApplyOnCreateEffects_466fe0` @0x467056 (`call [vft+8](newEff, node)`) | entity activation, per EFTR **tree 3** root node | factory(1)-created temp clone-of **file prototype node** | demo: prototype +0x9C = 0 (EFDT payload reader is the no-op stub `GameEffect::virt_0x462600` — session 6b) ⇒ clones damage 0 |
| `ApplyEquipEffects_46ba30` @0x46bacb | equip/unequip, EFTR **tree 0** | same pattern (clone file prototype) | same demo-inertness |
| runtime combat — **does NOT call +8** | `GameActionAttack::meth_0x403770` (the step body reading the `+0x65` DM-flag, consuming the `mbr_0xc0` queued-damage list) | builds `eff = factory(1)` then **DIRECT field stores**: `eff+0x9C = rec.dmg` @0x4038b6, `+0xA0 = rec.dtype` @0x4038c2, `+0xA4 = rec.material` @0x4038c8, `+0xA8 = rec.voice` @0x4038d1, `+0xAC/+0xAD` bytes = last-record/strike flags @0x4038e8-0x4038f7; entity refs via `meth_0x55a360`(setEntity) + indexed `meth_0x55a3e0(0..3)` setters; then `Entity::meth_0x5506c0(victim, eff)` fires it | **this is the whole damage-application dataflow** they asked for |

So: cloned-from-file effects = demo-inert (0 damage, stub payloads); real damage effects are
runtime objects with direct writes — the clone slot +8 is only ever seen with prototype args.
`JE::Effect::virt_0x55ac10` doubling as "+8 body" for the engine base Effect explains the
updater-map completion semantics they saw in 0x5506c0 (`call [child_vft+8](child, dt)` for the
0x33C/0x354 maps passes `dt` — same slot number, DIFFERENT contract per child family — their
"polymorphic update-or-configure" reading is correct; the EFFECT family uses it as clone-init
via the two Apply* paths, the TIMED-CHILD family (VFX/attachments) as tick).

## 3. `+0x65` DM-flag consumer loop (their open #1, fully)

`GameActionAttack::meth_0x403770` (the attack action's own step; called from the action
pipeline): iterates the `mbr_0xc0` queue built by `CalculateAndQueueDamage_0x404060`
(session 6 §5 records: {damage, part, attackerPerk, weaponOrFist, targetId}); for each record
with `owner` (victim via `meth_0x54be10` lookup — that helper is **entity-by-id from the
active-action context**, seen everywhere): server-flag gate @0x4037b5 (client-mode: would
throw ⇒ the whole consumption path is server-side), builds the fresh effect as §2, evaluates
via `0x5506c0`, advances list cursor via the byte at attack+0x65 (their flag) choosing
prev/next traversal (0x40391a-0x403961); clears the queue via `cls_0x55dc80::meth_0x4035d0`
(list destroy) at 0x403981. This also closes the loop on the queue container: the same
`(&action+0xbc)`-cluster list that `CalculateAndQueueDamage` inserts into via the
`0x4b9ad0`/`0x401ef0` list-append idiom (@0x404187-0x4041a9) is the one iterated and destroyed
here — one pending-damage record list per attack action (the exact field spelling differs
between the two decompiles due to thiscall-base artifacts; both land in the action's +0xC0s
container cluster — treat +0xBC..+0xCC as {list header, cursor, count} until a debugger pin).

## 4. For the reimpl (directly actionable)

- Mirror the split: **file EFTR effects = templates cloned at activation; runtime damage =
  new object with direct field writes** (one struct, two producers). Do not route runtime
  damage through any clone/init-from-args path.
- Their `entity-update-updater-maps` §2 pump order stays valid; the updater maps' children
  vft[+8](dt) tick contract applies to the *timed children* (VFX/particles), not effects.
- The DM beat → `attack+0x65` → step-consume loop: the flag ALSO selects list traversal
  direction in the consumption loop (§3) — replicate both roles or multi-hit attacks will
  consume in the wrong order.

## 5. Cross-links

- Answers: `2025-10-20-entity-update-updater-maps.md` §4.1/§3-lead;
  `2025-10-20-onanimevent-beat-dispatcher.md` §5.1.
- Built on: session6b (stub reader), session7 (Apply body), session8 §6 (firing sites).

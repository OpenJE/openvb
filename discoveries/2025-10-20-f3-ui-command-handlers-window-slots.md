# 2025-10-20 — Session 39 (T-017): F3 UI command handlers — runtime windows live on the CLIENT, not the singleton table

Target: ORIGINAL_REFERENCE (IDA). Read-only. The branch the coordinator predicted is real,
and it splits two mechanisms that were about to be conflated. Handlers sampled: EscapeMenu
(0x45ec00), PipBoy (0x45ec70), InventoryWindow (0x45eac0), OptionsWindow (0x45eb70).

## 1. Shared skeleton (all four)
```
client   = [*0x70BE08]+0x2C                     (sess-28 client global; NOTE: *0x70BD90
                                                  variant also seen — same +0x2C pattern,
                                                  two globals, see §4)
blk      = *(client+0xA0)                       ⭐ RUNTIME WINDOW-SLOT BLOCK (boot-zeroed in
                                                  GameClient ctor 0x45e7f3..0x45e820)
slot A   = blk[0x18]  ("main" window, e.g. inventory/pipboy body)      [dword idx 6]
slot B   = blk[0x44]  (companion window — re-opened when A closes)     [idx 17]
slot C   = blk[0x4C]  (cls_0x442bf0 options/pipboy-window)             [idx 19]
gate     = *(client+0x9C)  + sub_592630 = "no modal up" predicate
entity   = *(client+0xE0)  (represented entity — sess-37 actionbar row; bound into window
                             content via 0x455dc0 (pipboy) / 0x43f4c0 (inventory))
```
Protocol (verbatim pattern): if `IsActive(iface)` (Get+0x20 @0x406a50 after vb-rebase):
**exclusive close** — SetActive(A,0) [+ optional dismiss 0x4952a0 / 0x4433e0(1)] → reopen
companions (B,C SetActive 1). else: **open** — SetActive(A,1). SetActive = `meth_0x495220`
(sess-27 §2). None of the four touch the GameInterfaces singleton table (sess-26) — the
boot-registered 16 stay a *lookup registry*; runtime window state is entirely the
client+0xA0 block.

## 2. EscapeMenu adds the ROOT link (answers the coordinator's cross-link question)
`0x45ec5e`: on the close path it calls **`cls_0x4b28a0::meth_0x4b4f00(&GUI_ROOT(0x707E48),
rebased iface)`** — the same root-bootstrap seen at GameInterfaces vft+0x08 (sess-27):
insert/refresh in root watcher-set +0xA0 + stale-modal clear. So EscapeMenu's window is the
one that participates in the root's modal/watcher machinery; inventory/pipboy/options do NOT
(plain client-slot exclusivity). ⇒ **Two distinct "name in, object out" mechanisms, confirmed
and distinguishable** (for T-013): console dispatch = command-name → fixed handler fn-ptr
(RegisterCommand map, sess-38) with objects at FIXED client slots; texture-registry acquire
= name → RbTree node (`++node[44]`). Not shared; do not port a common "ui registry".

## 3. Handler-size honesty (per rule): all four sampled handlers are REAL (48–140 bytes,
branching). No new stubs. Stub census unchanged: the 4 return-stub commands (sess-38).

## 4. Residuals
- ~~⚠ `*0x70BE08` vs `*0x70BD90` two-globals ambiguity~~ **RESOLVED same-day (data-file
  lesson!)**: raw loads census — `0x70BD90` is the only loaded base (253 direct loads, first
  site = GetWorldServerInstance 0x401150); `0x70BE08` is never a load-target — it appears
  as a `mov ecx,imm` this-argument (0x551839) = a STATIC embedded object the engine entity
  ctor registers into (distinct from the heap GameInterfaces of sess-36 `new(0x64)` — three
  neighbouring globals, three roles),
  and every sess-39 "`global_area_map_icons[0]`" site literally encodes `mov edi,
  [eax+0x2C]` off the 0x70BD90 value (verified bytes @0x45eac9/0x45ec05: `8b 78 2c 8b …
  a0`). So ONE client global: `[clientbase]+0x2C`, exactly sess-28's statement; the
  0x70BE08 label is IDA display sugar (base+disp folding / misnomer family). No check needed.
- blk identity: who ALLOCATES client+0xA0's block & fills A/B/C with which boot ifaces
  (pipboy=blk+0x18-or-0x4C?) — next pass, likely GameWorld::Initialize UI wiring.
- gate@+0x9C `sub_592630` predicate body unverified; companion semantics (B vs C) by call
  sites only.
- Remaining F3 handlers un-sampled: AttackModeCycle 0x45ee60, FeedbackWindow 0x45ee00,
  GameOptions 0x45edb0, HotKeyOptions 0x45edc0, LoadGameOptions 0x45edd0,
  MultiplayerOptions 0x45ede0, UseSkill 0x45ee30, VideoOptions 0x45edf0, WeaponToggle
  0x45eed0, ActionbarWindow 0x45ea50, OptionsWindow-sibling 0x45eb70 done; engine tier
  StartMap 0x58db80 / CreateEntity 0x58dda0 / Warp 0x58b880 are T-013/spawn-adjacent,
  NOT yet opened (deliberately).

## 5. Evidence
Skeleton EAs: blk loads 0x45ec7c/0x45eacc/0x45ec08/0x45eb78; exclusivity close 0x45ecc1 /
open 0x45eb58; dismiss 0x4952a0 @0x45ec4c; 4433e0(1) @0x45ebac; companion reopen
0x45ecdb/0x45ebbc; entity bind 0x455dc0/0x43f4c0; root call 0x45ec5e; ctor block-zero
0x45e7f3..0x45e820 (mbr_0xa0 census, sess-38 DB).

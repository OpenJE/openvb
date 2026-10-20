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

## 6. Follow-up (T-017 pass 2): the window block is EMBEDDED, not installed — consumers mapped
- `client+0xA0` has **no install store anywhere** (0 hits for disp32-0xA0 stores in the whole
  GameClient code band) ⇒ `blk` is an inline ~0x50-byte array of iface ptrs in the 964-byte
  client, ctor-zeroed — slots: confirmed idx 3..19 (bytes +0x0C..+0x4C) = 11 window slots
  (s39's A/B/C was the sampled subset, not the whole block).
- **`sub_45E6C0` = restore-ALL-windows**: re-SetActive(1) over every non-null slot (+ hooks
  0x430760 on slot idx9, 0x443400 on idx19). Callers: **`recv_packet_461a10` @0x461e58**
  (a packet restores the whole stack — fact-sheet handler, likely disconnect/dialogue-end
  recovery class), `sub_433BE0` (Feedback-adjacent), thunk `sub_46EDF0`.
- **`GameClient::virt_meth_0x45e7f0`** = forward-to-windows tick: `blk[+0x18]→0x43f5e0
  (inventory content tick)`, `blk[+0x10]→0x41c4c0` (**actionbar final repaint — the tail of
  sess-37's 0x41c910 chain ✓ closes that loop**), `blk[+0x28]→0x458710(args)`.
- ⇒ slot FILLERS are per-handler/per-packet stores at `client+0xA0+idx*4` (disp 0xA8..0xEC
  band) — next pass enumerates writers per slot; note probable intersection with IronStorm's
  SetFocusEntity adoption path (host vft+84/+0x54) if slots are filled on entity-focus
  changes rather than at window-open.

## 7. Evidence (addendum)
0x45E6C0 (slot loads 0x45e6d4..0x45e70c, restores 0x45e710..0x45e7dc); callers xrefs
(0x433e76/0x461e58/0x46edfb); forwarder 0x45e7f0 (three calls 0x45e7fc/0x45e80a/0x45e818);
zero-hit install census described §6 line 1.

## 8. T-017 pass 3: the slot fillers — ONE function, and it's the adoption seam
Census: inside the whole GameClient code band (0x45e000..0x462400), exactly **three stores**
touch client+0xA0..0xF0 — and all three live in **`GameClient::meth_0x4615f0`** (0x2f6):
- `client+0xB8` ← [arg+8] (blk window idx6 = the pipboy/inventory "main window") @0x461622
- `client+0xDC` ← [arg+0x34] @~0x4616xx region (pending candidate)
- `client+0xE0` ← [client+0xDC] **only after `54be10(0xDC)->[+0x1E4] == [client+0xB8]`**
  @0x46169c — an OWNER-STAMP equality check (+0x1E4 = the protagonist field from
  `FastArrow:B8`, if that offset reads right from this view)
Flow: switch on [arg+8]-kind (0 → default fill after `vft[+0xD4](1)` = engine SetFocus-ish;
2 → verify+stamp as above, then actionbar `0x41c9d0` via blk[+0x10]; other paths branch on
byteflags at [arg+4]). ⇒ **+0xE0 = "promoted represented entity"** (sess-27/37's actionbar
source explained: it updates only when focus-entity AND ownership-to-current-window align);
+0xB8 = active window. Caller of 0x4615f0: NOT enumerated (tool budget) — check in
`recv_packet_461a10` or the engine adoption path; this is IronStorm's SetFocusEntity
neighbourhood — coordinated stop: no further writes from me in 0x461xxx until they confirm.
Caveat on the census: `mov [reg+disp8]` addressing (regs holding client/blk bases outside
the code band, or [blkbase+0x18]-style) is NOT covered by this disp32 scan — the three-store
claim is scoped to disp32-in-client-band, stated so nobody over-reads it.

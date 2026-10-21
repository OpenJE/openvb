# 2025-10-21 — T-010 §10e: the combat manager's +0x4/+0x14 producers — both provably constant; `meth_0x5a9330` is dead code in the shipped image

PureKnight 01a15cb7-e27e. ORIGINAL_REFERENCE (`ida_original_f3` + raw-EXE objdump/scans; no IDB writes).
Class: `JE::Combat` (DB key `cls_0x5a9330`, size 36 = 0x24, vft `0x6a684c`, constructed by `ctor_0x5a9610`
at `0x545132` inside `JE::World::virt_meth_0x545100`, ticked by `meth_0x5a94a0` from
`JE::World::virt_meth_0x545d60` @`0x545e0d` — the T-011 seam).

## Verdict (the last T-011 gate)

1. **`+0x14` has NO producer of any non-zero value anywhere in the shipped image.** Every writer stores 0:
   ctor `0x5a962e`, reset `0x5a9603`, and both exits of `meth_0x5a9330` itself (`0x5a9371`, `0x5a93b0`).
   Scans (bounded, self-tested — item 35): byte stores `[reg+14]` disp8: 515 sites across `.text`, 8 with
   imm==1, ALL owned by foreign classes (PathSteerer 0x563261/0x5633c9, GUI 0x4abbf1/0x4bbb70/0x4c666e,
   CRT tails 0x5e99de/0x5e9d1f/0x5e9d8f — offset coincidences); `[reg+14]` disp8+disp32 with mod=10: 0 sites;
   dword stores: 1106, none owned by any Combat method; `lea [reg+14]` producers: 0 in Combat band, 0 in World
   band. Self-test anchors: known call `0x5a957f` and known store `0x5a9371` were both RECOVERED by the scans.
2. **`+0x4` is therefore pinned to 0 on every reachable path.** Its writers: ctor `0x5a961d`(0),
   reset `0x5a95fa`(0) — reachable; `meth_0x5a9330` `0x5a936e/0x5a93ad`(= its param) — **unreachable**, because
   the ONLY call of `meth_0x5a9330` in the entire image is `0x5a957f`, gated at `0x5a9578` by
   `mov al,[esi+14h]; test al,al; jz skip` (E8-rel32 full-text scan: exactly 1 site, self-tested against the
   known 0x5a957f). `meth_0x5a9330` is not virtual: vft `0x6a684c` slot 3 = `0x5a9320` is a `ret 4` stub
   followed by nop-sled, so vtable dispatch cannot reach it either.
3. Consumer semantics confirmed for the port: `0x5a951d cmp [esi+4], 0` — mode==0 is the ALWAYS path
   (0x5a9525 queue loop); the non-zero path (0x5a955a) is unreachable at runtime for the same reason.

## What `meth_0x5a9330(v)` WOULD do (for the mirror; it never runs on shipped data)

Walks the global entity list (`byte_70BE0C` sentinel — same list as `Server::virt_meth_0x555380`), and for every
entity with `[entity+0x296] != 0` (combat-queued flag):
- `v==1`: `Entity+0x297` latch → `cls_0x548ee0` = **combat-JOIN**: sets `[e+0x297]=1`, acquires a handle via the
  world-server/map-manager global `0x70bd98` vft-factory and stores it at `[e+0x298]` (`0x548ef4..0x548f03`);
- `v!=1` (v==0 from the param path): `cls_0x548f30` = **combat-LEAVE**: `[e+0x297]=0`, releases `[e+0x298]`.
Then `this->+4 = v`, `this->+0x14 = 0`. Both helpers (`0x548ee0/0x548f30`) have exactly one caller each —
`meth_0x5a9330` (xref-verified) — so they are equally dead on shipped data.

## The "+0x14 = another-event-queued vs owns-multiple-entries" question

**Neither reading is supported by any producer, because there is no producer.** Code-shape intent: +0x14 is a
one-shot RESCAN/REARM latch — read once at `0x5a9575` immediately after an explicit-time event (entity `+0x319`
path at `0x5a955a`, the same latch family as the T-048 +0x319 raiser) pops, and if armed, the next dispatch runs
the full-entity combat join/leave sweep, re-pins the cursor (+4 = time param passed through `push ecx` at
`0x5a957c` = the update's time arg) and disarms itself (`+0x14=0` in both exits). The engine that would arm it
is not present in this image: the arming site exists only as a design seam. For the port: model
`mode(+4)=0`, `rearm(+0x14)=0` as constants with the branch structure mirrored (mirror-and-cite);
do NOT invent an arming path — no door in the shipped surface arms it (this is the same discipline as the
consumed+thrown never-fires ledger entry, T-019).

## Reset/install chain (completeness)

- `JE::Combat::meth_0x5a95f0` (RESET: sub_5a9440 + zero +4/+8/+0x10/+0x14) is called from
  `dtor_0x5a9a10` and from `sub_546870` (World UI-cluster teardown path reached via
  `F3::GameWorld::virt_meth_0x46ffa0` region `0x46fcc7`) — i.e., on world unload/destruct, consistent with
  save/load or world-switch reset.
- Update entry: `meth_0x5a94a0` sole caller `JE::World::virt_meth_0x545d60` @`0x545e0d` — T-011's dispatch owner.

## Tooling discipline note

Two scans this session FAILED EMPTY on known positives (prior full-text `89/C7 disp32==0xA0` store scan;
and the first `+14` scan attempt was correctly bounded only after adding the disp8 form). Every negative claim
in this doc comes from a scan that also RECOVERED a known positive (`0x5a957f` call, `0x5a9371` store), and the
"never armed" conclusion is additionally cross-checked by three independent writer forms (byte-imm, byte-reg,
dword) + pointer-producer (lea) sweeps + vftable stub inspection.

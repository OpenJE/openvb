---
status: open
assignee: implementer-B (YoungStorm)
depends: M-001 slice 1 (device first), then this unblocks texture-bound skins
---

# T-013 — Engine texture registry: `cls_0x55dc80` @byte_70B200 (T-007's tail)

> **STATIC TAIL DISPATCHED (coordinator LoudDragon, 2026-10-21, after T-007 closure `4fbe65e`)**
> — KeenStorm claims the RE-only tail NOW: (a) Fragment `+0x2C` semantics via all 11 export
> usages (0x4b1750/0x4b1760/0x4b1c28/0x4b1cdf/0x4b1d43/0x4b1d92/0x4b1ee6/0x4b213a/0x4b2384/
> 0x4b24ec + reader); (b) the `+0xC0→+0xDC/+0xF8/+0x114` broadcast trio's READERS
> (0x4b1a0b/0x4b1b34/0x4b200b/0x4b262a etc) — derived rects vs per-part clones; (c) the exact
> bytes reaching the `+0xDC` probe key (name-buffer construction 0x4b21e8..0x4b2208,
> [handle+0x20]/[handle+0x2C] concat shape). Deliverable:
> `discoveries/2025-10-21-t013-static-tail-plus0x2c-broadcast-keyform.md`, byte-discipline as
> T-007 (anchor to EAs/raw bytes, never names), `ida_original_f3` READ-ONLY (no IDB writes),
> `[DISCOVERY-BATCH ready]` on completion. The PORT half of this row stays unassigned — this
> claim covers the static tail only.

## Scope
Port the registry that `SkinTable`/`SkinDefaults` acquire textures through: id = `node[3]`,
use count at `+44`, list head `mbr_0x28`; slots `+0xE0` add (`0x5c0940`), `+0xD8` (`0x5c0930`),
`+0xC4` acquire-by-name (`0x5c0a90`, a 14-arg SEH forwarder the decompiler fails on).

## Measured target, not a blank slate (from T-007)
Texture misses per shipped skin are **0 / 0 / 3 / 54 / 0** (`SkinTable::TextureMisses`), and all 54
`areamap.ini` textures are absent from the shipped index - so every AreaMap border record must take
the **quiet miss** (id 0, return 0). The original draws nothing there either. A port that invents
error handling for that case diverges.

## Do NOT assume (relayed from HappyWolf's T-017, settled answer)
"Name in, object out" is **two mechanisms** in this engine - do not unify them:
- console commands: name -> fixed handler -> **fixed window slot** (client+0xA0 block)
- this registry: name -> **RbTree node**, acquire-by-name, `++node[44]`, quiet id-0 miss
The one exception worth knowing: `CommandEscapeMenu`'s close path calls the session-27 root
bootstrap `0x4b4f00`, so that window participates in GUI-root modal/watcher machinery while
inventory/pipboy/options use plain client-slot exclusivity.

## Narrowing updates (coordinator, 2026-10-21) — T-007 G2 + the port already shrank this row
- **Registry internals are now STATICALLY CONFIRMED** (committed: `discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md` §G2 b/d): id = `node+0x0C` (`v6[3]` @0x51f664), name string at `node+0x10`, use count at `obj+44` (bump @0x51f679), intrusive list head at manager `mbr_0x28` (walk @0x51f58f, wrap @0x51f63d), id→object map at manager `+0x24/+0x28` (lookup @0x51f160). Slot semantics settled: `+0xC4`=0x5c0a90 GET-OR-CREATE index (negative = load failure, logged; callers clamp), `+0xD8`=0x5c0930 handle→object (invalid-handle logged), `+0xDC`=0x5c0970 lookup-only probe (`*out=id` on hit / 0 on miss, bumps usecount, NEVER inserts). `0x707CE4` = Gfx_Engine singleton (sole store @0x5bfe33, vft `0x6ae7ec`); its three slots are thin delegations to the `0x70B200` manager — class now minted `JE::Gfx_Texture_Manager` (T-033 b6, closed-by-cross-lane-evidence).
- **The key-form question is now NARROWER than "stem vs name.tga" overall** (via KeenStorm's T-007 port work, parent `2b17631`): the resource-name resolver (`ResolveResourceName_0x499f10`/`ResourceExists_0x49abd0` analogue) STRIPS the extension before the rht probe — so `Demand("<stem>.tga")` legitimately succeeds and the shipped chain converts those records. The open question is ONLY the registry probe key at `+0xDC`/the `cls_0x55dc80` node+0x10 string form (what bytes reach the manager), which one lockstep observation (T-052 lane) answers.
- **The behavior half of the seam is ALREADY PORTED** — `GUI::TextureSlots` / `BindFragmentTexture` / `ResolveSkinTexture` (openje `a166dfa`) implement the reader-shape and LoadDefault-shape chains against our `ResourceSystem`, each shape asserted separately (probe-hit skips demand; miss → Demand(name,1) → +0xC4 on resolved name → negative-clamp-still-releases → store; gate variant clears id on null object). This row's port work is the REAL registry (node layout above) plugged into that existing seam — not a from-zero design.
- REMAINING from the old scope: `TextureMisses 0/0/3/54/0` reproduction once the real registry exists; Fragment+0x2C dword meaning; the +0xC0→+0xDC/+0xF8/+0x114 broadcast targets' role (port reproduces the COPY per ruling; a future consumer analysis may still be owed).

## Acceptance
Registry add/acquire/release round-trips with use-count accounting; the 57 measured misses reproduce
exactly; `GUILoading` stays green; MSVC clean (the struct/class U-V rule applies to any new forward
declaration here).

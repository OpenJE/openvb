---
status: open
assignee: implementer-B (YoungStorm)
depends: M-001 slice 1 (device first), then this unblocks texture-bound skins
---

# T-013 — Engine texture registry: `cls_0x55dc80` @byte_70B200 (T-007's tail)

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

## Acceptance
Registry add/acquire/release round-trips with use-count accounting; the 57 measured misses reproduce
exactly; `GUILoading` stays green; MSVC clean (the struct/class U-V rule applies to any new forward
declaration here).

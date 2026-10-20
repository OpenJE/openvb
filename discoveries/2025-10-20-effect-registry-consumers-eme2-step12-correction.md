# 2025-10-20 — Session 24: effect-registry consumers found; EME2 step-12 CORRECTION (name/script ritual)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Closes session 23 §3's "+0x24C consumers" item
with the promised SIB-aware sweep, and corrects one of my own readings in session 21 §2.

## 1. `Map+0x24C/0x250` effect-registry consumers (SIB-aware disp32 sweep: 14 sites)

- **Loader** (session 23): insert + dup-guard @0x5792ab..0x579396.
- **Stop-all sweep — `JE::Map::meth_0x579760`**: three map-section resets (0x5712A0,
  0x577290(10.0f,1.0f), 0x5736C0) then `for node in *(Map+0x250): MapEffect::meth_0x5b2da0(node.value)`
  — 0x5b2da0 = per-effect **stop/reset** virtual. Its ONLY caller: `sub_547C30` @0x547e2a
  (JE engine map-transition routine — effect teardown on map leave; matches the engine
  region's World-level switch flow).
- **Registry destruction** in the `JE::Map` dtor @0x57ae1a..0x57ae41
  (`cls_0x4d8d70::meth_0x571be0` on the map member then free+null the head) — full dtor also
  reveals the ORDERED teardown list of the map's registries: children list [75] (EMEF-effect
  children via 0x56cf50 cleanup), slots [33/34/35] sub-structures freed via sub_579BB0,
  sub-block resets 0x56e7e0/0x56e770/0x571000/0x571320/0x574430/0x5724f0/0x572560, registry
  maps at element-dwords [148](=+0x250 effects), [145](=+0x254? next map), [142], [135],
  [129], [126], [123], [120], plus 7 string/list members — usable as the map-unload parity
  checklist for the reimpl teardown (order matters for dangling refs).
- F3-side hits 0x40d1d8/0x410305 are unrelated per-instance SSO strings (DynamicObject +0x24C
  cap field), not the map registry — noted to save the next reader a wild goose chase.

## 2. ⚠ CORRECTION to session 21 §2 step 12 (my earlier "post-create children" guess)

`meth_0x409cb0` (called at 0x5741f9) is a **2-instruction SSO string accessor**:
`return (cap@Entity+0x24C < 16) ? &Entity+0x238 : [Entity+0x238]` — a string DATA pointer for
the entity string living at Entity+0x238 (EEN2/EENT-era string slot — adjacent to +0x234).
So EME2 step 12–13 = **copy that string into a local list head; if a sibling count field is
non-zero, call entity->meth_0x54b050** — a name/script-string post-spawn ritual, not "children"
handling. Its ONLY xref (besides its own vft slot) is the EME2 handler. Session 21's step-12
row now annotated; the entity-string identity at +0x238 is open (candidates: script name /
display name — EENT wrote +0x234-adjacent string #4).

## 3. Practical
- A/B reimpl map-unload parity: §1's dtor order list is the checklist; the stop-sweep is
  triggered from the engine transition function (0x547C30) — your "change map" path should
  call effect-stop-all BEFORE freeing registries.
- No further map-chunk thread items left open except: attach-register body `meth_0x54ef90`
  (entity VEG attach list — FX work), the four EMAP header strings' consumers, and the
  Entity+0x238 identity above.

## 4. Evidence
Sweep method: SIB-aware disp32 scan for 0x24C/0x250 excluding esp-based SIB (recipe worth
keeping: `op modrm(mod=10,rm!=4) disp32` + `op modrm(…,rm=4) sib disp32`, basef==5 skip).
Sites: sweep 0x572564/0x57455f = other-class stack/window coincidences (verified: [esp+…]
forms — excluded on inspection); registry: loader §1 refs; sweep 0x579760 (loop 0x579781..
0x5797ac, caller xref 0x547e2a); dtor 0x57ae1a..0x57ae41 (full dtor 0x57ac20..0x57b287);
accessor 0x409cb0 (cmp 0x409cb7 / return arms), sole caller 0x5741f9.

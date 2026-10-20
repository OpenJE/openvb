# 2025-10-20 — Session 23: EMEF map-effect wire format + Map effect registry (Map+0x24C)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Closes the last session-21 §4 map-chunk item
(EMEF); EMTR/trigger factory was taken by the parallel
`2025-10-20-scripttrigger-map-factory-cutscene.md` (no overlap — that doc covers slots 17's
trigger objects; this covers slot 27's effects).

## 1. `JE::Map::ReadMapEffect_0x578fb0` — the EMEF chunk record

```
string   instance-name           ; unique per map (see §2)
u32      effect-id      → MapEffect+0x20
f32 ×2   vec-a          → MapEffect+0x24/+0x28      (position? pair with +0x2C)
u32      scalar         → MapEffect+0x2C
f32 ×2   vec-b          → MapEffect+0x30/+0x34      (extent/radius/second point)
u8    n; n × string    → string-list                ; effect parameters (name/value strings)
u8     flag            → MapEffect+0x48
```
Construction: `operator new(0x5C)` → `JE::MapEffect::ctor_0x5b2fa0(map)` (parent map stored),
instance-name copied to `MapEffect+0x04` (SSO/string), the n parameter strings appended into
the `MapEffect+0x38` string-collection (`cls_0x50db20::meth_0x4c60a0` per list node) ⇒
MapEffect = 0x5C bytes: {vft, map*, name@+4, +0x20 id, +0x24/28 A-vec, +0x2C u32,
+0x30/34 B-vec, +0x38 params list, +0x48 byte flag}.

## 2. Duplicate-guarded map effect registry

Lookup in **`Map+0x24C` (map→string→MapEffect* registry; head member +0x250)** via
`cls_0x4d8d70::meth_0x4cbdf0` + string-compare `meth_0x41e4b0`; a hit throws
`JE::Common::Exception("Duplicate instance name in Map::ReadMapEffect(): %s.")` and the
half-built MapEffect is destroyed (`vft[0](me,1)`). Insert:
`*meth_0x5755f0(&Map+0x24C, name) = me` (find-or-add slot then store).
⇒ effect instance names are the lookup key for everything that later starts/stops map FX
(their cutscene dispatcher + `MapEffect` consumers in je docs `MapEffect.md` — cross-check
field names there before porting).

## 3. Map-load thread status after this
Session 21 §4 items: EMEN ✓, EEOV trio ✓, flag byte ✓ (=+0x3A0 pump), EMAP ✓, EMTR ✓
(parallel doc), **EMEF ✓ this doc**. Remaining map-thread openers:
- attach-register body `meth_0x54ef90` (entity-side list of attached VEG instances + per-frame
  update site) — pairs with FX work, still open;
- the four EMAP header strings' consumers (name/desc/music?);
- MapEffect runtime: who iterates `Map+0x24C` and fires effects (trigger/cutscene linkage
  probably — check the scripttrigger doc's dispatcher).
- `Map+0x134/0x138` entity registry iteration order at unload (destroy order matters for the
  reimpl teardown; session 21 §2 step 13 list).
- ⚠ Consumer-scan note: first disp32-0x24C sweep was inconclusive — every hit was an
  `[esp+0x24C]` stack-slot coincidence and the scan MISSED SIB base+disp32 forms (the
  reader's own `lea edi,[esi+SIB,0x24C]` didn't match). A SIB-aware sweep is the correct next
  step for locating effect-registry readers; left open rather than half-claimed.

## 4. Evidence
0x578fb0 whole-body (record reads 0x579036..0x579153; ctor 0x5791d3/0x5791fa; params loop
0x579280..0x57928b; dup-check 0x5792ab..0x579383; registry insert 0x579396; temp cleanup
0x5793ae). MapEffect fields per §1 EAs. Registry head fields `mbr_0x24c/mbr_0x250`.

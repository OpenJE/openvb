# 2025-10-20 — Session 14: v1 12-string array = BODY PARTS, item CopyFrom (26 strings), +0x18 declared write-only

Target: ORIGINAL_REFERENCE. Read-only. Closes the "12/13 string-array semantics" item from
`gcre-versions-gwam-queueanimevent-table` §1.1 and continues
[creature-copyfrom-gcre-saver-writer](2025-10-20-creature-copyfrom-gcre-saver-writer.md).

## 1. Creature slot +0xE0 body `sub_40AE60` = per-part model/armor registration ⇒ the arrays are PART data

Strings hardcoded in-order @0x40af29..0x40b233: **"Chest", "Feet", "Hands", "Head", "Legs",
"UpperArms", "Waist", "ForeArms", "Shins"** — the canonical Fallout body-part set (9 named +
3 more only via the file arrays). Behaviour:

```
a8 = engine attachment/list object at (Entity−1700B region)   ; ⚠ units, see §4 note
if creature-part-count present:
   clear all part slots (loop cls_0x49db40::meth_0x49db40(i,0))
   add 9 defaults: meth_0x49dd80(list, {"Chest"|"Feet"|...}, idx 0..8)   ; named PART registry
   then loop 12 records @Creature+0x2BC + 0x1C*i: if record.hasString (v6[5]!=0) →
        meth_0x49dd80(list, record-string, continues idx)                 ; @0x40b26f..0x40b287
        ⇒ stride-7-dword (=0x1C) iteration over EXACTLY the GCRE-v1 12×0x1C string array
final loop i=1..12: if record @Creature+0x1E0?? (this−232−8 dwords, stride 14) non-empty:
        resolve name→slot via sub_564F20(i, &nameStr, &out)               ; model bone/attach lookup
        a8->vft[+4](a8, i, out, nameStr)                                  ; register part↔bone
```
⇒ **GameCreature+0x2BC 12×0x1C strings = EXTRA/PART name table (overrides & extensions of the
9 default body parts)**, registered per spawn into an engine part list; the second block
(stride 0x38 at this−232·4…) iterates 12 more named records resolving to model attachments —
consistent with the **13×0x38 pair-array @+0x428** being per-part/per-slot model-attachment
{name, ?} pairs *(part-slot mapping; medium confidence — the two loops' bases differ by class
units, mark ⚠)*.

Consequences: armor DT-per-part arrays (+0x7B0/+0x7C8, **6 entries each**, per GCRE v3 mirror
sets and the damage-formula doc) and the body-part registry (9 defaults + up to 12 file
entries) are DIFFERENT capacities — so the DR arrays are NOT one-per-part as literally counted;
they likely group parts (e.g. torso/limbs/head) or index an armor-slot class. The exact
mapping is unproven; what IS solid is the part-name ORDER for the registry: Chest=0, Feet=1,
Hands=2, Head=3, Legs=4, UpperArms=5, Waist=6, ForeArms=7, Shins=8, file-extras 9..11.
Pin that enum order in the reimpl; ~~treat the 6-slot DR arrays as a separate index space until
a consumer proves the grouping~~ **RESOLVED (T-010c arc; 10c CLOSED)**: the DR indexing keys
on EQUIP SLOT, not anatomy — FastArrow's aggregation reads the equip node's class byte with a
6-dword array at `GameInventory+0x14..+0x2C`, and IronStorm's shipped-data read confirmed
`GITM.eqslot = 1` for every head-worn piece vs `0` for suits (eqslot IS the slot: scalar, not
vector). The +0x7B0/+0x7C8 creature-side sets then live in that same slot-index space (not
separately asserted here). Proven: row B = head-family, so {5,6} are the two head-family ids
of six equip slots; still inferred: which of 5/6 is HEAD vs EYES, the rest of the id order —
code stays numeric, comments may name the head-family pair with the pairing marked inferred.
This closes the caveat raised in this section; see the B8→B9 chain for the derivation.

## 2. GameItem CopyFrom @0x411330 (xref of GE CopyFrom body) — the 26-string block confirmed

RTTI Entity→GameItem, then ~13×{byte flag, string CopyAssign} pairs mirrored (dest
this−740..this−5, stride 0x1C/28-dword records; matches "GameItem = 26 own std::strings"
= 13 records × {flag+string} + 2 extra strings @this−29/−22 region) + tail GE CopyFrom
@0x41164d. This is the +0x28 override for GameItem (and 0x411ee4 shows GameProjectile's own
chain — IDA merged it into the dtor region, treat as projectile CopyFrom tail). Combined with
creature's §2-of-previous-doc CopyFrom: **the CopyFrom virtual exists across the family and
mirrors each class's file-loaded blocks** ⇒ A: template-instantiate = create + vft[+0x28]
is near-certain the shipped mechanism (three classes implement it; who CALLS the slot is the
remaining link — engine side, `call [reg+28h]` scan pending, see §4).

## 3. `GameEntity+0x18` — FORMAL VERDICT: write-only in the shipped build

Evidence: (a) ctor default 4 @0x467935; (b) GENT dword[4] load; (c) GENT save round-trip;
(d) NOT touched by any of the 58 decoded Entity-vb vft bodies (vft-decode doc §1.1); (e) two
consumer scan passes found no reader — the decisive one being that the disp8 forms of
`[E−0x8C]` and `[GE+0x74]` are byte-identical (`74`), so no positive disambiguation is
possible, and every *resolvable* access site (via the 0x666044-body census and the
entity-file readers) has been inventoried without one. ⇒ Reimpl: model it as
`gent.flags0` / reserved, **never read**; deviation risk zero. (If a future lockstep dump shows
a runtime reader, revisit — declaration is scan-based, not proof-based.)

## 4. Still open / handoffs
- **A**: slot +0x28 CALLER scan (`call dword ptr [reg+28h]` with entity-second-arg heuristic)
  — engine-side; say if you want me to run it or take it with the grid work.
- The part list object (Entity−1700/attachment region): `cls_0x49db40` meth_0x49dd80 =
  "addPart(name, idx)", `cls_0x49caf0::meth_0x49caf0` = count — engine-side part container;
  slot bodies of `v8->vft[+4]` = register-part-bone. If your render layer needs part→model
  bone mapping, THIS is its source.
- 0x409be0 (creature +0x60 body) still unnamed.
- ⚠ unit note: dest-bases in this function are _DWORD*/cls_ pointers with −N·4 arithmetic;
  I cite instruction EAs throughout; A should re-derive absolute offsets via one creature
  instance in the debugger (single dump of 0x2BC..0x2D8 will settle it).

## 5. Evidence index
Part defaults 0x40af29("Chest")/0x40af93(Feet)/0x40aff3(Hands)/0x40b053(Head)/0x40b0b3(Legs)/
0x40b113(UpperArms)/0x40b173(Waist)/0x40b1d3(ForeArms)/0x40b233(Shins); 12-loop 0x40b269..0x40b287
(this−338 dwords, stride 7); attachment loop 0x40b296..0x40b352; sub_564F20 0x40b2fb.
Item CopyFrom 0x411330 (RTTI 0x411348, tail 0x41164d); projectile chain 0x411ee4.
GE+0x18: ctor 0x467935, GENT f5 slot (entity-file-format §1), scan census §3(d)(e).

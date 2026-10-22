# 2025-10-21 — R-VFM: VFX_FormatManager anatomy — vft census, load/parse chain, .veg conformance, size erratum

**Agent:** SwiftMoon `01a16092` (PARK pull, lane contract §2) · **Coordinator:** LoudDragon `01a15f82`
**Mode:** read-only — raw PE parse of `$OPENVB_ORIG_EXE` + aligned `objdump -D -b binary -m i386
-M intel --adjust-vma=0x400000` + `artifacts/F3.json` joins; zero IDB, zero writes.
**Kit law:** T-083 self-test-first; items 40–49; DIRTY-until-checkpoint; claim commit `a390c27`.

## 0. Self-test gate — T-073's six anchored facts, re-read from raw BEFORE any classification

| # | T-073 claim | raw result |
|---|---|---|
| 1 | dtor stores vft 0x67396C @0x4FD29D | `c7 06 6c396700` @0x4FD29D ✓ |
| 2 | load/parse stores same vft @0x4FD7AD | `c7 45 00 6c396700` (via `[ebp]`, ebp=ecx @0x4FD7A9) ✓ |
| 3 | sole caller = static-init thunk `mov $0x70811c,%ecx; call 0x4fd780` | @0x641980–0x641989 ✓ (immediately followed by `push 0x645E60; call 0x61700A` = atexit registration ✓) |
| 4 | atexit 0x645E60 → dtor 0x4FD280 | `b9 1c817000 / e9 …4fd280` ✓ |
| 5 | getter `0x4FB040: mov eax,0x70811c; ret` has exactly ONE caller | image-wide e8-scan: 1 hit — 0x48EF71 (VFX_VEGInstance::meth_0x48EE30 region) ✓ |
| 6 | vftable-slots.tsv L2213 idx0=0x4FD660 for 0x67396C | raw `[0x67396C]=0x4FD660` ✓ |

Gate 6/6 ⇒ the dumper reading 0x67396C is the one T-073 certified.

## 1. THE VFT CENSUS — the anatomy answer nobody had: **the table is exactly ONE slot**

Raw walk from 0x67396C: slot 0 = **0x4FD660**; cell 1 @0x673970 = `0x656E6F6E` = ASCII `'none'`
— not a code target; the table ENDS at length 1 and 0x673968 holds the COL pointer.
* **0x4FD660 = scalar deleting destructor** (body: `mov esi,ecx; call 0x4FD280; test [flag],1 →
  push esi; call 0x6162E0` (operator delete)). So VFX_FormatManager is a **single-virtual class**:
  its only polymorphic member is the destructor. Every other method (load 0x4FD780, dtor 0x4FD280,
  getter 0x4FB040, FormatEntry grow 0x4FD680/0x4FD310) is non-virtual — the facade maps by direct
  call, not dispatch.
* RTTI (item-42): vft−4 → COL 0x6D5038 (COL+0x0=0 ⇒ primary), COL+0xC → TD 0x6FBBC4, inline name
  `.?AVVFX_FormatManager@@` ✓ — one COL, one class, no sharing.
* **Export agrees**: F3.json `.?AVVFX_FormatManager@@` vft 0x67396C `length 1, entries 1` — and
  the idx-0 EA matches. No census erratum on this table (unlike 0x6ae7ec's hole).
* One slot classified against §0 gate: **D→O, role-named** — slot0 = deleting dtor; the "6 unknowns"
  question doesn't arise: there are zero unnamed slots.

## 2. Object anatomy — members, teardown order, and the TRUE SIZE (the erratum, landed)

Load-time zero-stores (0x4FD7C1–0x4FD7E0) + dtor teardown (0x4FD2A3–0x4FD2CB):

| member | content | evidence |
|---|---|---|
| +0x00 | vftptr 0x67396C | stores 0x4FD29D/0x4FD7AD |
| +0x04/+0x08/+0x0C | `vector<FormatEntry>` {begin,end,cap}, element **0x14**={tag, nested-vector@+4} | dtor frees via 0x4FD150 on `[esi+4]`; grow math `imul 0x66666667 … sar 3` (÷20) @0x4FD695/0x4FD6B4 |
| +0x10 | zeroed by load (4th container dword / flag) | 0x4FD7C7 |
| +0x18..+0x24 | `vector<VFX_ParameterInfo>` header, element **0x114** (RTTI 273-byte VFX_ParameterInfo, ctor 0x4FB2D0 — T-073) | dtor `lea ecx,[esi+18h]; call 0x4FB560` @0x4FD2B3/BB (the family's ONLY view-const site); load zeros +0x1C/20/24 |
| +0x28 | third container (parameter member; destroyed FIRST via 0x4FB510) | 0x4FD2AE |
| +0x2C/+0x30/**+0x34** | scalars zeroed by load | 0x4FD7DA/7DD/**7E0** |

**SIZE ERRATUM:** max store is `[ebp+0x34]` (4-byte write @0x4FD7E0) ⇒ object span **≥ 0x38 = 56
bytes**. F3.json `.?AVVFX_FormatManager@@` says **size 44** — 12 bytes short of its OWN sibling
record `cls_0x4fd780` which says **size 56** (the same object under two export identities —
OOAnalyzer epoch-split, the T-057/0x50dbb0 pattern). Row for T-040's list: named-record 44 = WRONG,
live ≥ 56, export self-contradicts by 12 across its own split records. (Per T-040, 56 is the floor,
not a proof — no store exceeds +0x37 so no larger evidence exists; state floor-vs-truth, not truth.)

## 3. Load/parse chain (what 'dtor-is-actually-load' really does)

```
CRT static-init  0x641980  mov ecx,0x70811C ; call 0x4FD780        [+ atexit(0x645E60)]
VFM::load_0x4FD780  re-arms vft; zeroes +8..+0x34;
   ├ 0x52FB10() → 0x776 = 1910  (constant — entry bound of the big schema string-array @0x6FCA18)
   ├ loop i: push [0x6FCA18 + i*4]  (1910-slot SSO string array: enum member names, effect names,
   │        types 'VFX_Vector', defaults '0 0 0' … — the .veg schema corpus, .data-initialized)
   │        0x52FA30(str) → strcmp-walk of the 14-entry name table @0x6FE82C (0xE bound at
   │        0x52FA56; 0x61A73D = strcmp) → index 0..13, −1 on miss
   │        0x4FCD00(this,&index)                                     per-entry registrar
   └ registrar body: 0x52FB20() → 0x2B = 43  (BUILTIN schema-entry count — constant)
        loop 43 entries [0x6FC968 + j*4] (0x2B via 0x52FB20 — the builtin name/type/default
        schema strings: 'General','std::string','Name',''@0x64D7D8,'enum VFX_EffectType',
        'EffectType', …); per string: inline-flag byte==1 → SSO assign
        (0x401BD0 (ptr,len) / 0x401B20), delimiters " \t\n\r,;" @0x673950 through
        0x4FDCF0/0x4FDD90 scanners, element ctor 0x4FB2D0 (VFX_ParameterInfo 0x114),
        typed defaults via 0x4FF1B0 ('false'@0x671EAC at the bool branch 0x4FC891; '0'@0x661160
        at the float branch 0x4FC972; 12 call sites total) — '0 0 0'@0x67C114 is a SCHEMA-ROW
        default for VFX_Vector (data-observed, not a verified 0x4FF1B0 argument), insert 0x4FC280 → grow path 0x4FD680 → 0x4FD310 (element 0x14 math).
```
BUILTIN SCHEMA TABLE (raw dump 0x6FC968/0x6FCA18 — strings confirmed, FIELD ORDER NOT):
the arrays are consecutive SSO-string descriptors whose members include, verbatim in .rdata:
`General`, `std::string`, `Name`, `""`@0x64D7D8, `enum VFX_EffectType`, `EffectType`,
`VFXEFFECTTYPE_PARTICLESYSTEM`, `Emitter`, `enum VFX_AABBType`, `BoundingBoxType`,
`VFXAABB_OPTIMIZED`, `VFX_Vector`, `_OptimizedAABBMin`, `0 0 0`, `_OptimizedAABBMax`,
`MinBoundingBox` … — a (name/type/default/…)-tuple layout is CONSISTENT with the dump but the
quadruple-vs-triple field order is not pinned by any read this sweep → **FINAL-unknown (order)**;
the values themselves are all quoted raw. VFX_ParameterInfo carries exactly 4 string fields
(+0x34/6C/A4/DC per T-073 §2a, own-read of its member table not re-run) — a four-tuple
reading is the shape the element size supports, still an inference, labelled as one.
**Cross-row bonus:** the `""` default is the SAME pooled empty-string object T-078 just settled
(0x64D7D8) — the builtin schema table consumes it as an empty-string default at a THIRD site,
independently confirming "pooled `""`, referenced as default everywhere" (the gcre 'default-token'
reading again vindicated — T-078's disposition survives a second appearance).
Consumer side (accepted, re-verified): the format registry is read by the VEG-instance band
(0x48EE30→getter 0x4FB040) and property application at 0x5011A0–0x501220: `find('Loop')@0x4FA3C0 →
bool @0x4FFB60`, `find('MinStartTime') → float fstp`, `find('MaxStartTime') → float` — defaults on
miss: byte 0 / float 0 (0x5011E5/0x50120D).

## 4. .veg conformance vs `van-buren-docs/File-Format-Specifications.md` §3 (both directions)

| # | spec says | shipped does | verdict |
|---|---|---|---|
| C1 | file header signature char[8] `"VEG V1.1"` + int32 count + byte[12] | contiguous `'VEG V1.1'` NOWHERE in image (byte-search), but checked as TWO 4-byte tokens `'VEG '`@0x673A30 + `'V1.1'`@0x673A28 at **0x50136D/0x501383** via stream-read `0x4B8130` | **CONFORMS** (split-token implementation); note the spec's own C# reader *skips without validating* — the engine is STRICTER than the reference implementation |
| C2 | block header `"VFX V1.0"`(8) + byte[8] unknown + int32 propertyCount | `'VFX '`+`'V1.0'` read/compared as u32 pair @0x4F9994(`0x20584656`)/0x4F99B2(`0x302e3156`) then 8-byte skip then `0x4F9D40` (count) — second string-form twin @0x4F9A15–0x4F9A4E | **CONFORMS**; contiguity again two 4-byte pools (0x673900/0x6738F8) |
| C3 | §3.2 type encodings incl. float(4B IEEE), bool(1B: **0xDC=true**/0x00=false), VFX_Byte, VFX_Vector(3f), Vector2(2f), TextureUVs(4f), Color(3B), Resource/enum/string variable | type-name literals ALL present: float@0x673958, bool@0x673960, `VFX_TriggeredEffect`@0x673978, `VFX_TextureUVs`@0x67399C, `VFX_Resource`@0x6739B8, `VFX_Color`@0x6739C8, `VFX_Rotation`@0x6739D4, `VFX_Vector`@0x6739F0, `VFX_Vector2`@0x6739E4, `enum`@0x673A08, `std::string`@0x6739FC; **binary 0xDC bool-true tested @0x500C5F (`and al,0xDC`… consumer band)** | **CONFORMS** — full §3.2 vocabulary engine-side |
| C4 | property examples e.g. `EmitterShape`, value=type-dependent | `EmitterShape`@0x6796C8, `Emitter`@0x67C0C4, `RibbonOrigin`@0x673A38, `MaxSegmentsPerFoot`@0x673A48, `MinSegmentsPerFoot`@0x673A5C, `MaxWidth`@0x673A70, `MinStartTime`/`MaxStartTime`/`Loop` in the schema band | **CONFORMS** (spec's documented property names are the engine's own literals — doc and binary share provenance) |
| C5 | textual source form `VEG { … }` | `'VEG {'` absent; delimiters `>`,`=`,`" \t\n\r,;"` (0x673948/4C/50) referenced from the shared tokenizer (0x4FDCF0/90 …, also 0x523D95 = the skin-ini tokenizer, 0x52E655 = VFX-parameter INI band) | **text form is editor-side** (compiled .veg ships; engine carries a text tokenizer shared with the ini layers — consistent, no contradiction) |
| C6 | — | **0x4F9A00 string-form checker has ZERO direct e8 callers** (vft/thunk reach unresolved by this scan) | FINAL-unknown (reachability only; the bytes are read and conformed above) |

## 5. Bucket summary + successor

* Census: 1/1 slots classified (role-named deleting dtor) — a trivially complete census BECAUSE the
  class is single-virtual; the "anatomy" is therefore member/chain-based (§2–3), not dispatch-based.
* U-rows with reasons: 0x4F9A00 caller-set (above); 0x501xxx band's class identity (property
  APPLICATOR — likely VFX_VEGCache/instance sibling: it new(0xC)-allocates a triple at 0x5011BB and
  takes the getter's registry — one routing hint, NOT a mint claim, item 41/44 disciplines).
* **Successor proposal for PARK (contract §3):** R-VFM-2 (queued): "VFX_ParameterInfo (273B, vft
  0x673944) field anatomy + VEG-cache 0x501xxx band ownership" — the applicator band is the last
  unattributed member of this epoch and feeds the cursor/audio port era like R-CS.

## 6. Hygiene

Read-only honored (image+JSON only; no 8745; no IDB; no writes outside my two files). Every EA above
re-read raw this sweep; T-073 reused ONLY as the self-test yardstick (§0), never as a citation
source for claims (all §2–5 claims carry own-read EAs). Size question reported floor-vs-export,
not truth-vs-export, per T-040. Deliverable LEFT DIRTY; row flip relayed.

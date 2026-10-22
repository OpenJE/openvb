---
status: done
assignee: WildZenith `01a16092` (PULL per lane contract, PARK id R-VEG-E proposed by SwiftMoon in R-VFM-2 sec 3-successor, dispatched by LoudDragon `01a15f82` 2026-10-21 after R-AMX accept 84a9dad)
type: research (read-only; discoveries DIRTY-until-checkpoint)
depends: R-VFM, R-VFM-2 (both accepted — ParameterInfo grid, applicator band, the two FINAL-unknowns this row closes)
---

# R-VEG-E — VFX_Parameter value-cache roles + Holder 0x28-row anatomy + value-emit family

## Objective
Close the two FINAL-unknowns R-VFM-2 left with reasons printed, plus the family attribution:
1. **Value cache +0x20/+0x24/+0x28** (VFX_Parameter 52B base subobjects): the three -1 sentinels'
   roles — which is last-value, which is min/max clamp (R-VFM-2 proved live block {Loop byte,
   Min f32, Max f32} at HOLDER+0x30 — is the Parameter cache the same triple in different dress?),
   settle by READER/Writer containment per field, not type-guess.
2. **Holder 0x28-row element anatomy**: find-walks showed stride-0x28 rows; deliver the row field
   map via the row-ctor / row-allocator site (name string @+0x00? + what fills +0x04..+0x27).
3. **0x500Bxx-0x500F9x value-emit family**: the emit/eval functions that READ the cache + write
   instance values — attribution matrix: which type (0x6C/A4/DC subobject class + the encoded
   0xDC bool from R-VFM-2's self-errata) dispatches to which emit fn, EA-pinned.
   Completeness METHOD: containment (this lane's R-AMX method) — the family = exactly the call
   graph reachable from the applicator band's dispatch slots; state the anchor set + why closed.

## Deliver
`discoveries/2025-10-21-r-veg-e-value-cache-emit-family.md` (DIRTY): cache-field role table
(each dword: writers[], readers[], sentinel semantics, verdict), row anatomy table, emit-family
attribution matrix + closure argument, named port relevance (feeds audio/cursor port era per
PARK note — state what the port needs, invent nothing).

## Acceptance
Every claim EA-anchored; raw bytes over decompiler for all field roles (item 43); family closure
method stated with limits (item 41); R-VFM-2's yardsticks re-confirmed not re-guesses (0x28
stride, 52B Parameter, -1 sentinels); zero IDB writes; collisions honored (KeenStorm product,
HappyEagle harness, writer-seat tools/ files untouched — incl. the current tools/ecx-flow.py
unstaged foreign edit: NOT touched, NOT committed by this lane).

## Steps
1. Claim (this file + relay). 2. Field-role sweep: member_index + raw mov-sweep on 0x20/24/28(%reg)
in the 0x4F9000-0x502300 band; classify writer/reader chains. 3. Row-ctor hunt via allocator
(new(0x28)) xrefs in band. 4. Emit family: xrefs graph closure from vft slots of Holder/VFX
class; attribute per-type dispatch. 5. Write doc DIRTY, flip done, commit task-file-only, relay.

## References
discoveries/2025-10-21-r-vfm2-parameterinfo-applicator.md (banked), R-VFM doc, T-073, harness
items 40-47 (+52 honest-subtraction ethos), openvb-ida-mcp skill.


## Notes (closure)

**DONE 2026-10-21 (WildZenith 01a16092).** Deliverable: discoveries/2025-10-21-r-veg-e-value-cache-emit-family.md (DIRTY). Closes both R-VFM-2 FINAL-unknowns + the band attribution, all raw-pinned:
(1) FIELD ROLES: +0x20 = TYPE enum 0..0xE (15-name vocab table @0x6fbbe8: int,VFX_Byte,float,bool,enum,std::string,VFX_Vector,VFX_Vector2,VFX_Rotation,VFX_Color,VFX_Resource,VFX_Target,VFX_TextureUVs,VFX_SnapBone,VFX_TriggeredEffect; -1 unset; parse 0x4fdcf0->0x50062e, 13 canonical writers 0x4fdfff-0x4fe1d2, switch in serializer +0x500c04 + every getter). +0x24 = INLINE-DEFAULT float bits (0x4fdd90 second-token atof via 0x52fa70, -1 none; consumed ONLY by stringifier @0x500827/0x500868 + assign copy - emit takes defaults as CALL ARGS, mov 0x4(%esp)/ret-4 shapes). +0x28/2C/30 = raw byte-buffer trio {size,alloc,ptr} (setter 0x4fdea0 shrink-free, deep-assign 0x4fdf50 rep movsl+and3/movsb, all typed readers deref [+0x30] per type: flds/movb/movzbl/movl/0x502120). Reset 0x4fe1fa writes the -1-trio = {type,default,size} - NOT three homogeneous caches (R-VFM-2 wording refined in doc sec 4).
(2) HOLDER ANATOMY: name-keyed red-black TREE cls_0x4f9b90 (count @holder+8 limit 0x3333332 length_error 'map/set<T> too long' @0x4fa8f3; insert 0x4fa8d0 + rotations 0x4f9b90/0x4f9cb0; descent 0x4f9db0 with CASE-INSENSITIVE _stricmp @0x4f9df8). Node = new(0x60) @0x4fa803: {L+0,P+4,R+8, VALUE node+0x0C = {SSO name +0x04..+0x1B, VFX_Parameter @node+0x28}, color +0x5C, end-sentinel +0x5D}. THE '0x28-ROW' RE-TIMED: lea esi,[eax+0x28] @0x4fa450 = &node.parameter RETURN, not a stride - applicator flow (0x5011A0 -> getters on &param) re-verified with correct base; all R-VFM-2 EAs stand.
(3) EMIT BAND: 0x500bc0-0x500fbf = ONE serializer function (ret $8 @0x500fbf, JT data @0x500fc2+), sole callers VFX_ParameterInfo::meth_0x501000 @0x5010b2/0x5010e5 (ParameterInfo SAVE: name + the three embedded Params). Width map {int4,Byte1,float4,bool1(0x00/0xDC ENCODED - R-VFM-2 claim reconfirmed),enum-str,string,vec12,vec2-8,rot12,color3,resource-str,target-str,uv16,snapbone-str,triggered4} via 0x4B8130/0x4B8340; load mirror 0x4B80E0 in parse zone @0x5006xx. Closure = xref-complete writers + prologue/ret boundary scan; limits stated.
(4) BONUS FINDS: live-IDB mislabel Win95IsDebuggerPresent = toBool wrapper (body @0x4ffb6b proves !=0 test) - flag for annotation tier; off-by-four string-base question in Parameter ctor-vs-assign (0x5006f0 vs 0x4ffcc0) named+left-open with reconciling hypothesis (base +0x04, size @+0x14, cap @+0x18).
(5) PORT NAMED: doc sec 1+3 = complete .veg parameter wire spec in one page - 15-type index order + widths + bool encoding + inline-default parse semantics + case-insensitive keying (tree shape is impl-choice, hash-map relief named NOT obligated). Liftable verbatim by P-CMBT-era doc.
Posture: read-only honored (zero IDB connections), raw-over-decompiler, refinements of banked wording delivered as PROPOSED annotations (no edits to others' rows), foreign tools/ecx-flow.py unstaged edit untouched, doc DIRTY.

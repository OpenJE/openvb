# R-VEG-E — VFX_Parameter field roles, the Holder "0x28-row" anatomy, and the value-emit band

**Agent:** WildZenith (sessionId `01a16092`) · **Date:** 2026-10-21 · **Row:** R-VEG-E (claim `66149d2`, PARK pull proposed by SwiftMoon via R-VFM-2 successor) · **Method:** read-only; raw objdump over the md5-pinned exe primary; native f3_* + complete xref censuses; containment for closures. R-VFM/R-VFM-2 yardsticks re-confirmed, wording refined where bytes overrode (listed in §4).

## 1. THE FIELD ROLES (+0x20 / +0x24 / +0x28–0x30) — settled by writer/reader containment

The 52/56-byte `VFX_Parameter` base (ctor `JE::VFX_Parameter::ctor_0x5006f0`, vft `0x673a20`) is NOT three homogeneous value caches. Raw-pinned roles:

**+0x20 = TYPE ENUM (0..0xE; −1 = unset/unrecognized).**
- The 15-name vocabulary table lives at **`0x6fbbe8`** (dwords → `int`@0x66a8e4, `VFX_Byte`@0x673a10, `float`@0x673958, `bool`@0x673960, `enum`@0x673a08, `std::string`@0x6739fc, `VFX_Vector`@0x6739f0, `VFX_Vector2`@0x6739e4, `VFX_Rotation`@0x6739d4, `VFX_Color`@0x6739c8, `VFX_Resource`@0x6739b8, `VFX_Target`@0x6739ac, `VFX_TextureUVs`@0x67399c, `VFX_SnapBone`@0x67398c, `VFX_TriggeredEffect`@0x673978) — this is the exact canonical index numbering both parsers and emitters use.
- Parse: `0x4fdcf0` tokenizes against this table (scan `cmp $0xf` bound, not-found ⇒ `or $0xffffffff` @0x4fdd4f) — stored `[edi+0x20]` @0x50062e.
- Canonical writers: the 13 per-type sites `0x4fdfff..0x4fe1d2` (`movl $K,0x20(%esi)` each, every one verified raw: types 0,1,2,3,5,6,7,8,9,a,c,d,e) — these are the default-apply writers; type 4 (enum)/b/f path variants complete the set.
- Readers: EVERY typed consumer switches on it — serializer @0x500c04, the getter family (0x4ff970/0x4fe280/0x4fe450/0x4fe690/0x4fe880/0x4feb20/0x4feef0 = int/string/vec3/vec2/rot/color readers), `Win95IsDebuggerPresent`-mislabeled `toBool` wrapper `0x4ffb60` (IDA name wrong, body = `meth_0x4ff970(...) != 0` @0x4ffb6b).
- Sentinels: ctor `-1` @0x500736; reset `-1` @0x4fe1fa; unrecognized-parse `-1` @0x4ff516.

**+0x24 = INLINE-DECLARED DEFAULT (float bits; −1 = none).**
- Writer: `0x4fdd90` = the second-token parser — skips the type-name token, and iff a second token follows (`test %eax,%eax; je ⇒ -1` @0x4fde2c), float-converts it (`0x52fa70` @0x4fde04); stored `[edi+0x24]` @0x50065a. So `VFX_Byte 200` in a .veg/vfxt text ⇒ type=1, +0x24=bits(200.0).
- Consumers: the stringifier/description path ONLY — @0x500827 (`push [esi+0x24]` → `0x52fb30` int-format) and @0x500868 (re-read for the tail) — plus raw copy by assign `0x4ffce1`. The EMIT switch never reads it: per-type default writers take the default as a CALL ARGUMENT (`mov 0x4(%esp),%al … ret 4` shape @0x4fe010-0x4fe029), not from +0x24. −1 semantics = "no inline default declared".
- Verdict for R-VFM-2's open item: **+0x24 is NOT a second value cache; it is the parse-time default annotation** — which is why no typed reader distinguished it.

**+0x28 / +0x2C / +0x30 = the raw byte-buffer trio {size, allocated, data-ptr}.**
- Setter `0x4fdea0` (1 arg = new size): frees when shrinking (`cmp %eax,0x2c(%ebx); jle` @0x4fdf2f-0x4fdf37 → helper `0x4fde50`), `rep movsd`-style buffer ops @0x4fdf1d; assign-into `0x4fdf50` (2 args = size+srcptr): shrink-check + `rep movsl` + `and $3; rep movsb` tail @0x4fdf89-0x4fdf90 — **byte-size arithmetic, proving buffer semantics** (not a pair of caches).
- ctor zeros all three @0x500729-0x50072f; reset writes `{+0x20=-1, +0x24=-1, +0x28=-1}` + zeroes +0x2C/+0x30 @0x4fe1fa-0x4fe206 (the −1 in +0x28 = "buffer size unknown/unset" — distinct from ctor's 0).
- All typed readers resolve values through **[+0x30] dereferenced per [+0x20]**: float `flds (%[+0x30])` @0x4fe3c7/0x4ffa03, bool byte `mov (%[+0x30])` @0x4ff9ca, `movzbl` @0x4ff9b1, int dword @0x4ff9eb, VFX_Resource/Target wrapper `Get_mbr_0x0_502120(this=[+0x30])` @0x4fe3a5.
- The per-type default writers (0x4fdff0 family) BUILD this buffer: `push $WIDTH; call 0x4fdf50` with the value in a stack local + then tag `[+0x20]=type` — widths `{$4 int/float, $1 byte/bool, $0xC vec3, …}` = the emit widths of §3.

⇒ Consolidated table (Parameter-relative):

| off | role | ctor | reset | parse-writer | reader use |
|---|---|---|---|---|---|
| +0x20 | type enum 0..0xE | −1 | −1 | 0x50062e (via 0x4fdcf0) + 13 canonical sites | switch in serializer + every getter |
| +0x24 | inline-default float bits | −1 | −1 | 0x50065a (via 0x4fdd90) | stringifier ONLY (0x500827/0x500868) |
| +0x28 | buffer SIZE | 0 | −1 | via 0x4fdea0 | shrink-cmp 0x4fdf2f; stringifier |
| +0x2C | buffer allocated | 0 | 0 | via 0x4fdf50 | realloc/shrink |
| +0x30 | buffer data ptr | 0 | 0 | via 0x4fdf50 | ALL typed reads (dereferenced) |

## 2. THE HOLDER ELEMENT ANATOMY — a name-keyed red-black TREE; the "0x28-row" is a node offset

The container (`VFX_ParameterHolder`, vft 0x67390C, find `0x4FA3C0`) is **`cls_0x4f9b90` = std::map/set-style tree**, NOT a 0x28-stride vector:

- Header fields: `[holder+0x04]` = tree head-node handle; `[holder+0x08]` = **count** (post-increment `++this->mbr_0x8` @0x4fa979; miss-test `cmp 0x8(%esi),%eax` @0x4fa420; count limit `>= 0x3333332` ⇒ throws `length_error("map/set<T> too long")` vft 0x647934 @0x4fa8f3-0x4fa94c).
- Node = **`new(0x60)`** @0x4fa803 (node ctor `sub_4FA7E0`, 5 args L/parent/R/key/color): `{+0x00 left, +0x04 parent, +0x08 right, +0x0C VALUE(0x54), +0x5C color byte (set 0/1 through RB-fixup), +0x5D end-sentinel byte (walked `while(!*(node+93))` @0x4f9dba)}`.
- VALUE @node+0x0C = `{SSO name string (+0x04..+0x1B within value: buf16, size @+0x14, cap @+0x18=15 — value-relative; written by ctor_0x4f9fb0 @0x4f9fd2-0x4f9fe9), VFX_Parameter @value+0x1C = node+0x28}`.
- **The "0x28 row stride" re-timed:** find's return path is `lea esi,[eax+0x28]` @0x4FA450 where `eax` = found node ⇒ **&node.parameter** — the parameter's OFFSET inside the node, which the earlier walk read as an element stride. Descent `0x4f9db0`: `_stricmp(node.key, query) >= 0` ⇒ go left `*v2` else right `*(v2+8)` @0x4f9df8-0x4f9e01 — **case-insensitive name lookup** (matches .veg semantics: names are matched without case).
- Insert `0x4fa8d0` = `sub_4FA7E0` node build + link (three insert positions: root/head-`*mbr_0x4`, before `param_3[0]`, after `param_3[2]` — head-node circular list semantics @0x4fa97c-0x4fa9ae) + RB rebalance/rotation with color flips (`meth_0x4f9b90` left-rotate, `meth_0x4f9cb0` right-rotate @0x4fa9f4-0x4faa63).
- Erase/link fixups @0x4fa480+ (`mov %eax,0x4(%eax)` self-linking = head relink).

Consequences that stand as written: R-VFM-2's applicator flow (`0x5011A0`: new(0xC) live block ← find('Loop')→0x4FFB60→[blk+0], find('MinStartTime')→0x4FE280→fstp [blk+4], find('MaxStartTime')→[blk+8], defaults 0/0.0 @0x5011E1-0x50120D) re-verifies with the CORRECT base: the getters receive `&node.parameter` and read its §1 fields. The "defaults proven" statement stands.

## 3. THE VALUE-EMIT BAND 0x500Bxx–0x500F9x — one serializer + its closure

Band decomposition (function-granular, item 43):
- `0x500150`–`0x5004c0`(tail thunk `mov %edi,%edi`-adjacent) = **typed stringifier** family (per-type `%d/%f`-style formatting feeding emit case 4/5/a/b strings; toDescription zone @0x5004C0-0x500b7e shares tail blocks 0x500b00/0x500b08 with it — boundary noted, interior not walked further than the +0x24/+0x30 reads named in §1).
- `0x5005xx–0x5006e8` = parse/load zone: type+default token stores into +0x20/+0x24 @0x50062e/0x50065a, wire read `0x4B80E0($4/$1)` right after @0x500664 — the load-side counterpart.
- `0x5006f0` = `VFX_Parameter::ctor` (copy-ctor shape: zeros fields, calls assign `0x4ffcc0` @0x50073f).
- **`0x500bc0`–`0x500fbf` = THE EMITTER — one function** (`ret $8` @0x500fbf; jump-table data from `0x500fc2` — verified no second head in 0x500b80-0x500fbf). R-VFM-2's "0x500C5F within the DB-unattributed function" = this: IDA attributes it `JE::VFX_ParameterInfo::meth_0x500bc0`. Its callers = **exactly 2**, both inside `JE::VFX_ParameterInfo::meth_0x501000` (0x501000+0x14d) @0x5010b2/0x5010e5 — the ParameterInfo SAVE path (emits name-string via `0x4B8340` + the three embedded Parameter subobjects +0x6C/+0xA4/+0xDC through the serializer, plus flags).

Emit width map (per `case` of the switch on +0x20, writer `0x4B8130(buf, width, 1, dest, cursor)` / `0x4B8340` for strings — all raw from the decompile + push constants):

| type idx | name | wire | value source (getter) |
|---|---|---|---|
| 0 | int | 4B | 0x4ff970 (`[+0x30]` dword) |
| 1 | VFX_Byte | 1B | 0x4ff970 byte path |
| 2 | float | 4B | 0x4fe280 (`flds`) |
| 3 | bool | 1B **encoded 0x00/0xDC** | neg/sbb/and chain @0x4ff9ce-0x500c5b (R-VFM-2 encoding claim: same, base corrected) |
| 4 | enum | name string | stringifier via 0x4ffd10 temp + 0x4e1900/0x4b8340 |
| 5 | std::string | token string | 0x4ffd10 |
| 6 | VFX_Vector | 12B (3 f32) | 0x4fe450 |
| 7 | VFX_Vector2 | 8B | 0x4fe690 |
| 8 | VFX_Rotation | 12B | 0x4fe880 |
| 9 | VFX_Color | 3B (RGB) | 0x4feb20 |
| 10 | VFX_Resource | name string | 0x500760 temp path |
| 11 | VFX_Target | name string | 0x4ffd10 path (caseB) |
| 12 | VFX_TextureUVs | 16B | 0x4feef0 |
| 13 | VFX_SnapBone | string | 0x500150 path (caseD) |
| 14 | VFX_TriggeredEffect | 4B id | 0x4ff970 int |

Closure method (item 41, limits stated): band = complete xref sets of the two wire writers (0x4B8130/0x4B8340 — emit sites cluster at 0x500c04-0x500f90 + load-mirror 0x4B80E0 at 0x5006xx) + function-boundary scan (prologue/ret raw walk 0x500400-0x501100); attribution = containment within those heads. NOT claimed: every interior block of the stringifier family (only the §1-relevant reads sampled).

## 4. REFINED WORDING on the banked R-VFM-2 lines (all its EAs stand; two words change)

1. "value-cache dwords +0x20/+0x24/+0x28" → **{type, inline-default, buffer-size}** — three roles, not three caches (§1 table). The −1 tri-store they sampled is the RESET writing type=-1, default=-1, size=-1 @0x4fe1e0.
2. "holder's OWN 40-byte entry rows, element 0x28" → **0x60 tree node, parameter at node+0x28**; `lea esi,[eax+0x28]` @0x4fa450 was the &param return, not a stride advance (§2). Their find-by-name/count/dup-find conclusions are unaffected; the container being a name-keyed RB tree with CASE-INSENSITIVE `_stricmp` @0x4f9df8 is the sharper fact.
3. Minor: "toBool 0x4FF970 / toFloat 0x4FE280" — roles correct; note the live-IDB name of the toBool wrapper is mislabeled `Win95IsDebuggerPresent` (body @0x4ffb6b proves `!= 0` test).
4. R-VFM-2's "0x500C5F bool ENCODED (not merely tolerated)" — reconfirmed inside §3's serializer case 3; encoding produced, not compared.

## 5. Port relevance — named, no invention

- §1's field table + §3's width/vocabulary map = **the complete .veg parameter wire spec** in one page: 15 types at index order pinned to the 0x6fbbe8 table, per-type widths {4,1,4,1,str,str,12,8,12,3,str,str,16,str,4}, bool 0x00/0xDC encoding, inline-default parse semantics ("TYPE VALUE" token pair, −1 absent), and the case-insensitive name keying. Whatever the port's VFX layer reads or writes, these tables are the conformance target — SwiftMoon's P-CMBT-era doc can lift §1+§3 verbatim.
- The Holder's tree shape is an implementation choice, NOT wire-visible — the port may key by hash-map given the `_stricmp` case-insensitivity preserved; named as relief, not obligation.
- Named unknown left open (not this row's scope, small): value-string interior off-by-four (`+0x04` assign-base vs `+0x08` ctor NUL) between `0x4ffcc0` and `0x5006f0` — string base +0x04 with size@+0x14/cap@+0x18 reconciles both; flagged for annotation tier if a lane wants the exact struct name.

## 6. Posture

Read-only honored (zero IDB connections/writes; image + F3.json + /tmp scratch). Raw-over-decompiler on every offset verdict (§1/§2/§3 byte-quoted); the three FINAL-unknowns R-VFM-2 left named are closed with roles, not guesses; refinement wording for banked lines (§4) delivered as PROPOSED annotations, edits to other rows not performed by this lane. Collisions: `tools/ecx-flow.py` foreign unstaged edit untouched; product/harness files not written. Doc left DIRTY per contract.

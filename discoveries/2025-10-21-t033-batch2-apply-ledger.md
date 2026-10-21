# 2025-10-21 — T-033 batch 2: PRE-CHECKS executed (PureKnight 01a15cb7-e27e, GRANTED apply scope)

> **Tool distinction (so no later lane quotes the wrong one):** the function-tier census recipe is
> **T-029 §5** = `list_funcs` full iteration (pages parsed from the /tmp/pi-mcp-*.txt JSON) + local tier rules
> over the game region (< `0x5FD000`). `tools/data_census.py` is the **T-025 shipped-DATA census** (chunks/GWAM/GITM)
> and has nothing to do with typed-stub tiers.

Statics only, `ida_original_f3` + `$OPENVB_ORIG_EXE`. This file records the two pre-checks RedXenon's
`2025-10-20-t033-batch2-selfnaming-decisions.md` gated the mints on, plus the corrected decision table.
**No IDB rename has been written yet** — the draft table (§4) goes to the coordinator first per grant.

## 1. PRE-CHECK 1 — near-identity, `cls_0x4cb300` vs `cls_0x4d4650` → **RESULT: one class (merged), not a pair**

Evidence from the constructor bodies themselves:

- `cls_0x4cb300::ctor_0x4cb2e0` (11 instr): SSO-string header init `[+14h]=0, [+18h]=0Fh`, byte `[+4]=0`,
  then `G3D_Transform::ctor_0x4cb190(this+20h)` — a **default** construction of {string @+0x10, G3D_Transform @+0x20}, size 152.
- `cls_0x4d4650::ctor_0x4d4650` (30 instr, SEH): same `[+14h]=0,[+18h]=0Fh` + `[+4]=0` init, **plus**
  `cls_0x50db20::meth_0x401b20` = copy-string from the param, and `G3D_Transform::ctor_0x4d3780(this+20h, param+20h)` —
  a **copy/record-conversion ctor of the same 152-byte layout**.
- `G3D_Transform::ctor_0x4d3780` reads its source at `+8` (string) / `+28h..+60h` (`rep movsd` 8+6 dwords) — i.e. it is
  the transform's own copy ctor, not a second transform type. The "distinct entry points" are DISTINCT CTOR OVERLOADS.
- Consumer nails the type: `sub_4D8000(p1,p2,cls_0x50db20*)` does `new 0xA0`, stores two ptrs at +0/+4, then
  `cls_0x4d4650::ctor_0x4d4650(this+8, name)` → the node embeds the same 152-byte object at +8 (8+152=0xA0 ✓).
- Member tables agree: `.?AVG3D_Transform@@_0x20` (120B, parent=true) + string-header members at +0x14/+0x18 in BOTH.

**Verdict: `cls_0x4d4650` is not a twin class — it is a second construction site of `cls_0x4cb300`'s type.**
Therefore: mint ONE name `JE::G3DTransformHolder` on cls_0x4cb300 with cls_0x4d4650::ctor_0x4d4650 recorded as its
converting-ctor; do NOT mint the paired address-suffix alternative (the pair branch is dead). 5+1 fns = 6 retired.

## 2. PRE-CHECK 2 — alias-merge, `cls_0x40a220` vs `cls_0x4cb0a0` → **RESULT: two DIFFERENT std streams; merge check FAILS the merge**

- `cls_0x40a220::ctor_0x497bb0` installs `std::basic_fstream<char,…>::vftable_669ea8` +
  `basic_filebuf::vftable_669e64` at this+0xC (`lea ecx,[esi+58h]` = filebuf's own streambuf tail). Size 156.
- `cls_0x4cb0a0::ctor_0x4cb0a0` installs `std::strstream::vftable_671cc8` + `std::strstreambuf::ctor_0x4cb020`
  at this+0xC. Size 136, member table holds `.?AVstrstreambuf@std@@` (96B, parent=true @+0xC) AND parent
  `cls_0x496bd0` (52B @+0x0) — the two-parent overlap is exactly the MSVC `istrstream/strstream` shape
  (basic_ios + embedded strstreambuf), not corruption.
- `cls_0x40a220::ctor_0x40a220` is a trivial two-SSO-string init (no vft) — it does not construct either stream;
  that "ctor" attribution looks like helper-theft noise, not evidence of aliasing with 0x4cb0a0.

**Verdict: they are NOT two instantiations of one strstream-ish type** (fstream ≠ strstream: different vftables,
different buffers, different sizes). RedXenon's "mint `JE::StrStream` once, not twice" gate resolves to:
- `cls_0x4cb0a0` (1 fn) → mint **`std::istrstream`** family name; the shipped symbols already exist in the IDB
  (`std_strstream_vftable_671cc8`), so the safe IDB-side name is the one whose vftable is referenced —
  propose `std::strstream` **only** on cls_0x4cb0a0.
- `cls_0x40a220` (12 fns) → its dominant real body (`ctor_0x497bb0` and the 0x49xxxx cluster) constructs a
  **`std::fstream`**; propose mint `std::fstream` on cls_0x40a220, evidence = vft 669ea8/669e64 at 0x497c16/0x497c2f.
  But note: cls_0x40a220 is a size-156 class whose method set spans 0x40a220..0x605db5 — a known IDB route-A
  split (batch-1 "tail-alias artifact"). Escalate as **SPLIT-SUSPECT**: mint the name but do NOT expect the
  12-fn retirement to be clean; census must count cls_0x40a220 as named-but-noisy.

## 3. Do-not-mint / inherited rulings (unchanged)

- `cls_0x50db20`: stays anonymous (refuted composite; also appears as the converting-ctor's param type in §1 —
  a reason to keep ITS name unfixed, not to fix it).
- CEngineInterface embedders 0x558740/0x5b6400/0x4395a0/0x5b4eb0/0x5b7c10: R36-e — annotation `embeds@CEngineInterface`
  only; no base-name minted.
- 0x4468c0 / 0x56eb70 / 0x4c0190 single-embedder holders: pass the >=3-embedders filter (1 embedder each) per
  RedXenon's audit — mints stand as drafted.

## 4. DRAFT RENAME TABLE (route A = live IDB; **awaiting coordinator sign-off, no writes yet**)

| cls | new name | keyword | fns retired (claimed) | evidence EAs |
|---|---|---|---|---|
| cls_0x4cb300 | `JE::G3DTransformHolder` | struct | 5 (+1 converting ctor absorbed from cls_0x4d4650) | ctor 0x4cb2e0 / conv-ctor 0x4d4650→new 0x4D8000(0xA0=8+152) / embed `.?AVG3D_Transform@@`@+0x20 in both |
| cls_0x4468c0 | `JE::HotKeyWindowFormatTable` | struct | 2 | 3× `.?AVHotKeyWindowFormat@@` 128B stride @+0x14/+0x94/+0x114 (member-table join) |
| cls_0x56eb70 | `JE::G3DMeshHolder` | struct | 2 | `.?AVG3D_Mesh@@`@+0x70, size 268 |
| cls_0x4c0190 | `JE::GfxCameraHolder` | struct | 6 | `.?AVGfx_Camera@@`@+0x30, size 640 |
| cls_0x4cb0a0 | `std::strstream` | class | 1 | vft 0x671cc8 @0x4cb110; strstreambuf ctor 0x4cb020; parent cls_0x496bd0(ios)+embed@0xc |
| cls_0x40a220 | `std::fstream` (SPLIT-SUSPECT — name + noise note) | class | 12 nominal, count as noisy | vft 0x669ea8 @0x497c16 + 0x669e64 @0x497c2f |

Pre-registered prediction check (§ batch-2 doc): max retirable ≤49 fns; this table claims 28 clean + 12 noisy
= 40 fns = 1.62% of T2's 2,473 → within the ≤1.3pt band ONLY IF the noisy 12 are counted; if they are, band holds;
if the census counts name-route only, expect 28/2,473 = **1.13pt** — inside the band. Route B (F3.json) stays stale
by T-040; will be reported as such, not averaged.

## 5. LEDGER — APPLIED 2025-10-21 (route A = live `artifacts/F3.exe.i64` via ida_original_f3, sole writer PureKnight, post-sign-off)

Fresh BASELINE (immediately before writes, T-029 §5 recipe): total 10,173 fns, game 8,876,
**T2 = 2,340 = 26.4%** — exactly reproduces the published T-029 number (no drift; unique-addr check passed).

| old fn (per cls) | new fn name | evidence EAs | tag |
|---|---|---|---|
| cls_0x4cb300::ctor_0x4cb2e0 | JE::G3DTransformHolder::ctor_0x4cb2e0 | SSO init `mov[+18h],0Fh`+G3D_Transform::ctor_0x4cb190@+20 | struct |
| cls_0x4cb300::dtor_0x4cb300 | JE::G3DTransformHolder::dtor_0x4cb300 | class-dtor over +0x10 string + +0x20 transform | struct |
| **cls_0x4d4650::ctor_0x4d4650** | **JE::G3DTransformHolder::ctor_0x4d4650** (converting ctor, ABSORBED — pair-suffix branch died) | see WHY below | struct |
| cls_0x4cb300::meth_0x4d07f0/0x4d09b0/0x4d09d0 | JE::G3DTransformHolder::meth_* | member usage 0x4d09be.. on +0x20 embed; 1 of 5 flagged `6-unproven(check-prototype)` by t036 — retained, tagged | struct |
| cls_0x4468c0::ctor_0x4468c0, ::meth_0x4461e0 | JE::HotKeyWindowFormatTable::ctor_0x4468c0 / ::meth_0x4461e0 | 3× `.?AVHotKeyWindowFormat@@` 128B stride @+0x14/+0x94/+0x114 (usages 0x446976/0x446986/0x446999) | struct |
| cls_0x56eb70::ctor_0x56eb70, ::meth_0x56e460 | JE::G3DMeshHolder::ctor_0x56eb70 / ::meth_0x56e460 | `.?AVG3D_Mesh@@` 156B @+0x70, size 268 = 0x70+156 exact tail embed (usages 0x56e47e/0x56ebbb) | struct |
| cls_0x4c0190::ctor_0x4c0190 + 5 meth (0x4bffd0/0x424880/0x4248f0/0x424db0/0x429d20) | JE::GfxCameraHolder::* | `.?AVGfx_Camera@@` 300B @+0x30, size 640; embedders=0 (t036); all 6 `5-evidenced-method`; 4 fns are far-band (0x424xxx) but evidenced via +0x30 usage 0x429d78 | struct |
| cls_0x4cb0a0::ctor_0x4cb0a0 | **std_strstream::ctor_0x4cb0a0** | installs std_strstream_vftable_671cc8 @0x4cb110 + strstreambuf::ctor_0x4cb020 @+0xC; matches existing IDB member std_strstream::virt_meth_0x4cb140 | class (STL, V-mangled — shipped symbol) |
| cls_0x40a220::ctor_0x497bb0, ::ctor_0x499500, ::meth_0x497fb0/0x498120/0x4984e0/0x498550/0x499040 (7) | **std_basic_fstream_char::ctor_0x497bb0 … ::ctor_0x499500** | installs std_basic_fstream vft_669ea8 @0x497c16 + filebuf vft_669e64 @0x497c2f; dense band 0x497bb0..0x499500 already contains named member std_basic_fstream…::virt_meth_0x497e60 | class (STL) |

**WHY THE PAIR-SUFFIX READING DIED (condition 1, for revival attempts):** byte-level — cls_0x4d4650::ctor_0x4d4650
performs the IDENTICAL skeleton init as cls_0x4cb300::ctor_0x4cb2e0 (`[+14h]=0, [+18h]=0Fh, [+4]=0`, SSO string at +0x10),
then chains `cls_0x50db20::meth_0x401b20` (string copy) + `G3D_Transform::ctor_0x4d3780(this+20h, src+20h)` where
0x4d3780 provably reads the SOURCE's name string at src+8 and 8+6-dword float blocks at +28h/+48h — i.e. the SAME
G3D_Transform layout (vft 0x671cfc) cls_0x4cb300 embeds at +0x20. Consumer `sub_4D8000` news 0xA0 = 8 + 152 and
constructs the object at node+8, proving one 152-byte type, not a layout twin. A "pair with address suffix" would have
minted two names for one class.

**NOISE LABELS (condition 2, never silently promoted):** cls_0x40a220 was NOT renamed wholesale — t036 gate returned
R36-e composite (embedders=9, ratio 0.58). Three in-game fns STAY ANONYMOUS as helper-noise/unknown-owner:
`cls_0x40a220::ctor_0x40a220` (trivial two-SSO-string init, constructs neither stream), `::meth_0x4e3260`,
`::meth_0x5a4b80` (far scatter). Two more (0x605c76/0x605db5) are out-of-game link-tail and already owned by
std::ios_base — not counted, not renamed.

**POST-CENSUS (route A, same recipe, same file):** T2 = **2,316 = 26.09%** → delta **0.31pt / 24 fns retired**
(6+2+2+6+1+7). PRE-REGISTERED BAND (RedXenon re-registration): ≤1.3pt → **HOLDS** (band is a ceiling; small delta is
honest: batch-2 targeted only 7 classes of the ~300 anonymous). Route-B (F3.json, DB-method route): **STALE by T-040**
(no ooprolog stage 2, no re-export path) — NO delta claimed for route B; f3-json-check pins the 2024 export against
drift only. Batch-1's voided pre-registration (→~23%) is NOT the yardstick; the void-and-re-register stands as recorded.

**Escalations carried forward:** (a) cls_0x50db20 remains the name-blocked param type of a minted converting ctor —
its composite stays batch-3's problem; (b) cls_0x40a220 residual 3 fns need host-band attribution (t036 R36-e form);
(c) the `6-unproven` G3DTransformHolder::meth_0x4d07f0 wants a prototype check next pass; (d) `meth_0x401100` doc
claim in ScriptNativeActionDoors.md ("whoever calls it is a different door") is now resolved in
`docs/f3/AttackPositionOverrideDoors.md` — T-037 ledger can cite it.

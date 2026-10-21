# 2025-10-21 — T-033 batch 3 DRAFT (pre-sign-off): R36-e residuals — template families named by body, host bands where provable, vendor band escalated

PureKnight 01a15cb7-e27e. Read-only prep (census + objdump profiles + t036 traces); **no IDB writes yet**
(awaiting coordinator sign-off per grant).

## Method note
Each class got: t036 `--class` trace (categories + IDB-owner overlap), full-.text instruction profile of its
route-A functions (top callees + referenced rdata strings), and route-A prefix count. Overlap discipline per
grant: "overlap estimate" = functions the IDB already attributes elsewhere (3a/3b conflicts + std names) —
printed next to counts so the 533 upper bound can't become the story.

## Route-A counts & evidence profiles (measured, not from the pre-registration sum)

| cls | route-A fns | t036 cats | IDB-owner overlap | profile (calls/strings) |
|---|---|---|---|---|
| cls_0x4d8d70 | **142** | 35 evid / 95 clone / 5 tail | 1 (`std::locale`) | STL `map/set<T> too long`, `invalid map/set<T> iterator`, `list<T> too long`; 80×`j__free`; 57+57 of its OWN fns `0x583930/0x4c8e50` (the two generic tree helpers); 27× `cls_0x50db20::meth_0x401bd0` (string append); ctor at class base `ctor_0x4d8d70` + per-site clones |
| cls_0x50dbb0 | **120** | 15 evid / 101 clone | 3 (`JE::ResourceFileManager` — 3a) | same STL diagnostics; plus config strings `use_managed_file`,`Resources`,`file_cache_size`,`file_max_count` IN-BAND; entity-ref fallback map cited already (`meth_0x51f0f0`, EntityRefKey0x1DC §35) |
| cls_0x55dc80 | **94** | 20 evid / 69 clone | 5 (`JE::Gfx_Texture_Manager` — 3a) | same STL diagnostics only — NO host-unique string inside the family band |
| cls_0x4542d0 | **50** | 41 clone / 7 evid | 0 | same STL diagnostics; struct = two embedded `cls_0x454210` (allocator pair @+0/+C, size 24) |
| cls_0x424a70 | **54** | 50 evid / 4 unproven | 0 (1 unnamed conflict) | GUI/PLAYER band: `lblSkill%da..c`,`lblName`,`lblToolTip*` strings; calls `JE::CLoadMapInterface::meth_0x495740`(48×), `JE::ModelInstance::…`, `JE::CounterControl::…`; **`meth_0x42e870` calls `GamePlayer::ctor_0x411820` (the ONLY non-factory player constructor caller, sess doc `gameworld-interfaces-effects-classid.md` §5) — char-gen/avatar rebuild, gender texture switches, socket model tables `byte_6F29B8/6F2A08`, appearance table `byte_6F2B34`** |
| cls_0x668cd8 | **46** | 42 evid / 4 clone | 0 | **VENDOR**: string `"ID3DXMeshOptimize: Cannot do AttributeSort because %d vertex needs to …"` (verbatim D3DX9 utility text); band 0x474xxx-0x47exxx = statically-linked D3DX mesh-optimizer + `??2@YAPAXI@Z` operator-new; vft 27 slots at `0x668cd8` |
| cls_0x49c610 | **0** | R36-b | all 43 named `JE::ResourceSystem` in IDB | route-A delta = 0 by construction — declared-at-0, route-disagreement demo |

## Proposed actions (per class)

1. **cls_0x4d8d70 → `JE::StringTree`** — APPLY all 142 route-A fns. Batch-1 CONFIRMED ruling (mechanism:
   `_Tree` family, 11 copy-ctors = 11 instantiations) + this pass re-confirmed the family profile (STL
   diagnostics + string-append + 2 shared generic helpers). Ledger caveat: template family; per-instantiation
   value types unresolved; 95 clone-family fns are sibling instantiations of the same source bodies (R36-d
   semantics). Tag: `class` (MSVC `_Tree` family are `class`-mangled; `V` per DEFERRED-NOTES:389 note).
   Expected route-A retirement: 142.
2. **cls_0x50dbb0 → ESCALATE (no mint) — draft v1's "cfg subset" proposal REFUTED by push-site precision.**
   The t036 band-profile's config strings (`use_managed_file/file_cache_size/file_max_count` + section
   `Resources`) resolve, at exact push-site attribution, to **one** family function:
   `cls_0x50dbb0::meth_0x50cd40` — a `Config_Load(tree&)`-shaped body whose callee cluster is owned by the
   ALREADY-NAMED `JE::ResourceSystem` (49bf20/499900/49bc60). The 3a IDB conflicts point at
   `JE::ResourceFileManager` for 3 other fns. So the band spans ≥2 real owners through shared `_Tree`
   instantiations — renaming the 119 clones under any single host = silently promoted noise (forbidden).
   Action: escalate as `std::_Tree` family across ResourceSystem/ResourceFileManager hosts; annotation only.
3. **cls_0x55dc80 → ESCALATE (no mint).** Same family, host-unique strings absent; IDB already names the 5
   evidenced ones (`JE::Gfx_Texture_Manager`). Renaming the 69 clones to any single host would promote noise.
   Ledger annotation: `std::_Tree family; texture-manager instantiation named in IDB; remainder unresolved`.
4. **cls_0x4542d0 → ESCALATE (no mint).** `_Tree` family + allocator-pair struct; 41 clones. Same reasoning.
5. **cls_0x424a70 → `F3::GamePlayerAvatarManager`** — APPLY all 54 route-A fns. Evidence chain: sole
   non-factory `GamePlayer::ctor` caller + char-gen tables + gender/socket loops + player-skill labels;
   sess-40 doc named it "that player/avatar manager"; batch-1's blocker (member-census) satisfied by this
   profile. Tag `class`? DB key is `cls_` (no RTTI) → default `struct` + note per grant rule.
6. **cls_0x668cd8 → VENDOR ESCALATION, zero mints.** Statically linked D3DX9 optimizer/utility band.
   Recommendation to census owners (NOT an IDB rename): treat 0x474xxx..0x47exxx vendor islands as a tier
   note — these are not engine identity; naming them `D3DX::*` would inflate named-% with third-party code.
   (If the coordinator prefers, an IDB comment `vendor: D3DX9 mesh optimizer` at the vft is a bounded, honest
   write — proposed as the action instead of a rename.)
7. **cls_0x49c610 → no route-A action (0 fns); ledger records route-B-only retirement = 0 route-A delta**
   (declared at 0 per grant condition).
8. **CEngineInterface embedders (0x558740/0x5b6400/0x4395a0/0x5b4eb0/0x5b7c10)** → IDB `set_comments`
   `embeds@CEngineInterface (R36-e; base-vs-member open, batch-4)` at each ctor vft store — annotation-only,
   no names.
9. **cls_0x40a220 residual (3 fns)** → comment at `ctor_0x40a220`: `helper-noise; owner unresolved (R36-e,
   batch-2 ledger)` — labelled, not promoted.
10. **cls_0x50db20** → annotation-only as ruled (comment at vft-less ctor region? — decision: skip, batch-2
   ledger already carries it).

## Pre-registered expectation (measured route-A counts before writes; band reported after)

Apply set = 142 (StringTree) + 54 (avatar mgr) ≈ **196 fns route-A** ⇒ T2 26.09% → ≈ **23.9%** (−2.2pt).
Within my earlier band (0–8pt). Escalations (50dbb0, 55dc80, 4542d0, 668cd8-vendor, 49c610-zero) = ~310 fns
that STAY anonymous under this discipline — reported as the batch's conservative half, not hidden as a failure.
Route-B (F3.json) stale by T-040; no claim.

---

## APPLY & RESULTS — 2025-10-21 (post GO; ZenNova per-item rulings 8e49181-thread)

**Writes (route A, sole writer, mesh-announced before first write): 199 renames, all `ok:true`.**

| old prefix | new prefix | n | evidence anchor | self-test (item 35) |
|---|---|---|---|---|
| cls_0x4d8d70:: | **JE::StringTree::** | **145** (142 from pre-dump + 3 live-only found by post-census diff: 0x5478b0/0x5479a0/0x547ba0 — last role-named `insert_0x547ba0`, the World-load pool insert cited in T-041) | batch-1 CONFIRMED `_Tree` ruling + body profile (`map/set<T> too long`, `invalid map/set<T> iterator`; shared helpers 0x583930/0x4c8e50 57× in-family each; string-append callers) | live filtered `list_funcs` before/after: prefix count 142→0 remaining under old key; 139 named in dump + 3 late + 4 ≥0x5FD000 accounted |
| cls_0x424a70:: | **F3::GamePlayerAvatarManager::** | **54** (0x42e870 role-named `avatarRebuild`) | sole non-factory `GamePlayer::ctor_0x411820` caller; char-gen tables `byte_6F2B34/6F29B8/6F2A08`; gender texture pair `off_6F2B3C/6F2B40`; lblSkill/lblToolTip | **call-scan recovered BOTH known positives — factory 0x46f73f AND char-gen 0x42eb35 — exactly 2 E8 callers total** |

Tags: both families DB keys are `cls_` (no shipped RTTI) → port-side definitions must use matching keyword;
ledger notes the choice per DEFERRED-NOTES:389 trap (JE::StringTree = struct in port headers; 
F3::GamePlayerAvatarManager likewise).

**Escalations (zero renames, conservative half, all per GO rulings):** cls_0x50dbb0 120 (std::_Tree across
JE::ResourceSystem cfg reader 0x50cd40 + entity-ref fallback 0x51f0f0 + 3 already-IDB-named
JE::ResourceFileManager = R36-f conflicts); cls_0x55dc80 94 (same family, 5 already JE::Gfx_Texture_Manager);
cls_0x4542d0 50 (family + allocator pair); **cls_0x668cd8 46 = VENDOR D3DX9 band — reported SEPARATELY per
ruling: not renamed (would inflate named-% with third-party), not silently excluded (stays visible T2 line)**;
cls_0x49c610 = route-A 0 (declared-at-0; IDB already names all 43 JE::ResourceSystem — route disagreement demo,
not progress); cls_0x40a220 residual 3 (labelled noise); cls_0x50db20 79 (annotation-only per ruling).

**Post-census (route A, same T-029 §5 recipe + live filtered queries):**
- PRE (post-batch-2): T2 = 2,316 = 26.09% of 8,876. **POST: T2 = 2,124 = 23.93% → −2.16pt, 192 retired.**
- Registered target ≈23.9% — **MET** (gap vs 196-raw = 4 link-tail fns ≥0x5FD000 outside the denominator +
  1 dump-merge duplicate; explained, not averaged away).
- Vendor band line per ruling: 46 fns (cls_0x668cd8) inside T2, flagged third-party here; no recipe change
  (out-of-scope note only).
- Route-B: STALE by T-040, no delta claimed.
- Batch-4 queue now leads with t036 `OK rename candidate` classes (cls_0x6bb908 41, cls_0x6bc9d8 30,
  cls_0x41a2b0 28, cls_0x4b28a0 28, …) — 644 distinct anon classes, 2,124 fns remain.
- Cumulative identity program: T2 26.40% → 23.93% (b2+b3 = 219 route-A fns retired).

**Tooling failure disclosed (item 35):** `ida_original_f3::set_comments` rejects every input shape — the
harness delivers the argument as a STRING where the schema demands array/object (4 attempts: full array,
single object, minimal array — all `must be array/object/anyOf`). `rename` accepts the identical shape
(coercion present there), so ONLY the 13 planned comments were blocked; NOTHING was silently lost — the full
text is preserved below for the next session with a working comments path:

1. `0x668cd8`: vendor D3DX9 mesh optimizer + utility band (verbatim ID3DXMeshOptimize strings ~0x47xxxx;
   statically linked third party). NOT engine identity — report separately from named-% (T-033 batch-3 ruling).
2/3. `0x583930`, `0x4c8e50`: JE::StringTree SHARED generic helper (57 in-family callers) — one body per
   instantiation family; do not re-parent.
4. `0x40a220`: labelled helper-noise (batch-2/3) — two-SSO-string init, constructs neither stream; owner
   unresolved (R36-e composite).
5–9. `0x558740`, `0x5b6400`, `0x4395a0`, `0x5b4eb0`, `0x5b7c10`: embeds@CEngineInterface (R36-e composite;
   base-vs-member open, batch-4). No mint.
10. `0x50dbb0`: ESCALATION std::_Tree family, ≥2 real hosts (0x50cd40 cfg→ResourceSystem; 0x51f0f0
   entity-ref fallback; 3 fns IDB-named ResourceFileManager). Renaming clones = promoted noise.
11. `0x55dc80`: ESCALATION _Tree family; 5 evidenced already Gfx_Texture_Manager; no host-unique string in-band.
12. `0x4542d0`: ESCALATION _Tree family + allocator pair (2× cls_0x454210 @+0/+C).
13. `0x49c610`: route-A 0; IDB already JE::ResourceSystem ×43; route disagreement, not retirement.

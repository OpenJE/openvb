---
status: done
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: research (read-only on ida_original_f3; discoveries/ only, no commits of src/ or IDB writes)
depends: none — spawned by the T-033 batch-6 closure statement (commit `ddd39ec`: "cheap deterministic
evidence exhausted; future identity wins need NEW evidence classes... shipped-string mining in
still-unexamined regions")
---

# T-053 — Shipped-string mining over the unexamined bands (new evidence class for identity)

## Objective

The six-batch identity chain (T-033) exhausted the t036 OK-gate queue and the RTTI sweep. Remaining
T2 = 2,046 anonymous-at-IDB game functions, concentrated in known non-target families (vendor bands
46+148, _Tree/allocator 50dbb0/55dc80/50db20/4542d0 ≈ 332, n≤2 tail). This row hunts a NEW evidence
class: shipped strings in regions never mined, to mint or annotate classes beyond those families.

## Steps

1. Enumerate coverage: which `.rdata` string clusters have ALREADY been claimed by a committed
   discovery/doc (grep the discoveries/ + docs/f3 for string citations) vs which clusters sit above
   unnamed bands. Build the claim-vs-actual table (self-test: the known zones — Actionbar.INT
   names @0x41a2b0 band, Gfx_Texture_Manager strings @0x521d78, Gfx_AnimMeshInstance string
   @0x66Cxxx — must resolve to their committed bands, proving the method works on knowns).
2. Mine the UNCLAIMED clusters: for each shipped string with a `Class::Method - ` or `ns::name`
   shape, xref to its referencing function, walk to the owning band (vft store in ctor / this=
   global join), and per the accepted evidence hierarchy (singleton-join > shipped in-band string >
   membership-by-construction) decide MINT / ANNOTATE / ESCALATE. NO renames — you are READ-ONLY;
   findings are a HANDOFF-READY queue for the next T-033-style writer wave.
3. Register dead ends honestly: clusters that resolve to vendor code, CRT, or std internals get a
   one-line dismissal with the reason — they are then EXCLUDED, not left to re-mine.

## Acceptance

- `discoveries/` file with (a) the coverage table (claimed vs unclaimed string clusters, self-tested
  against three known positives), (b) a prioritized mint-queue: each candidate = string bytes + EA +
  referencing fn EA + owner-band evidence + hierarchy verdict + fns-at-stake count, (c) dismissal
  list. Zero speculative names; anything not statically resolvable stays unresolved with named risk.
- Coordinator checkpoints the file and re-dispatches the queue to a writer wave if it's worth one.

## References

- `.pi/skills/openvb-ida-mcp/SKILL.md` (target discipline, READ-ONLY + save rules — you never write)
- `discoveries/2025-10-21-t033-batch4-ledger.md` / batch5 / batch6 (evidence-hierarchy examples,
  registered escalations you must NOT target)
- `discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md` (model of byte-exhaustive claiming)

## Acceptance evidence (delivered 2025-10-21)

- **(a) Coverage table — self-tested on 3 knowns:** method (string→xref[EA + EA+1 for the
  `\xc7` boundary artifact]→containing fn→band class) reproduces Actionbar.INT@0x650ae0→CActionbarInterface,
  Gfx_Texture_Manager@0x677560, Gfx_AnimMeshInstance@0x66c430. Corpus: 422 `::`-strings; 387 sit in
  68 already-named bands (claimed), 32 touch anon fns → triaged to 0 unexplained.
- **(b) Prioritized mint-queue** in `discoveries/2025-10-21-t053-string-mining-unexamined-bands.md`:
  headline `JE::CDebugInterface` (cls_0x558740, **7** contiguous fns; ctor stores own
  `CDebugInterface_vftable_692e84` + Debug.INT + self-naming strings @0x692eac/@0x692ef0; 5 0x434xxx
  fns rejected as helper-theft) + 13 GUI window-interface ctor-band own-vft joins + 6 merge-into-
  existing-class candidates (each with string bytes + string EA + fn EA + owner-band evidence +
  hierarchy verdict + fns-at-stake). Net honest yield ≈30-31 route-A T2→T1 fns.
- **(c) Dismissal list:** `sub_5FB742` = DirectX DM/DShow error-string translator (15 strings, vendor,
  0x224f B, 490 msg-table refs, 47 callers), CRT `printf`/`debug_stream`, `std::string`, n≤2
  `Gfx_ModelInstance::AttachTo`, already-RTTI-claimed `AnimationCache::Startup`. Registered
  `_Tree`/vendor bands touched only by single-fn joins (b6 precedent).
- **READ-ONLY proven:** `artifacts/F3.exe.i64` mtime 14:26:04 (b6 save) unchanged at hand-back; zero
  renames/comments/type-edits issued. File: `discoveries/2025-10-21-t053-string-mining-unexamined-bands.md`.

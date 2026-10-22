---
status: CLOSED - 57/57 sites verified + renamed, 33 files, rescan 0 sites; selftest 12/12
assignee: SwiftOwl `01a16198-6cbb` (QuickNova dispatch 1774a3b; their own between-items offer honored, BrightYak second-position noted)
type: header-name verification + rename (2024-export-era openje headers; declarations only — NO behavior)
depends: artifacts/F3.json (method + vftable truth), f3_* native tools, item 57 (cite-or-name-absence), item 53 (query before dump), msvc+run-all gates
---

# R-HEADERSAN — the ~48 'j'-digit corrupt names + typeless decls: VERIFY, THEN (maybe) RENAME

## The defect
2024-export-era headers carry method/member names where one hex position is a literal `j`
(`virt_meth_0x4aje00`, `mbr_0xje0`, `cls_0x5debje`...). `j` is not a hex digit; each name is
therefore a LIE about an EA, and the lie is load-bearing the moment anything compiles against it.
BrightYak's slice-2 found two that blocked a compile and fixed them under neutral names WITH
defect-fix citations (decb94a); the ~48 remaining were REPORTED, not mass-edited — "an unverified
'fix' to 48 sites is gen-6 at industrial scale."

## Method (the row's whole point: confirm, never assume)
1. Build the truth set from artifacts/F3.json: every method EA + every vftable-entry EA + every
   struct name (the same data the native f3_* tools query).
2. Per site: substitute the `j` with each single hex digit AND consider the two-digit-merge
   variants; a site is CONFIRMED iff EXACTLY ONE candidate exists in F3.json AND the owner context
   agrees (the class that owns the hpp matches the class the DB assigns to that EA, or the EA
   appears in that class's vftable).
3. CONFIRMED -> rename (declaration-only edit, cite the verification in the commit: candidate set,
   winner, owner-table corroboration). UNIQ-BUT-WRONG-OWNER -> report, do not rename.
   NO-CANDIDATE / AMBIGUOUS -> REPORT the site with its candidate analysis; the honest output is
   the ledger, not a guess.
4. FIRST DATA POINT REFUTES UNIFORMITY ALREADY: Window.hpp `virt_meth_0x4afje0` resolves uniquely
   to `0x4af1e0` (j→1) AS Window's OWN vft slot 24 in 0x66e4d4 — while `virt_meth_0x4aje00` has NO
   single-digit variant among Window's 75 vft entries at all. The j<->b hypothesis from PARK is a
   HYPOTHESIS; per-site the winner varies (or nothing wins). Exactly what 'confirm-not-assume' was
   written for.
5. 'typeless decls' (the second half of the row): `undefined4`-typed DB-era declarations that lack
   params — where F3.json + one decompile settles the shape, retype with the cite; where it
   doesn't, report. Public-header edits take msvc rc0 + run-all per the gate law (item 37); the
   port-annot gate polices touched files automatically (11th step, re-sourced PATH per item 58).

## Scope fences
- Declaration sites only in modules/openje headers (+ their .cpp counterparts when a lying decl
  has a real body). NOT touching: files under active reservation by other lanes; foreign dirty
  worktree items; artifacts/F3.json itself (read-only truth).
- No bulk sweep against unbounded tables (safety law): the matcher loads F3.json ONCE into memory.

## Ledger
- 2026-10-22: verifier built (tools/jdigit-verify.py, selftest 7/7 PASS incl. the owner-forward-
  decl regression S7 - a first-version owner bug proposed 'Entity' as owner of RLECircle.hpp; the
  false-confirm was caught by MY tool before any rename rode on it). Full report banked at
  discoveries/2025-10-22-r-headersan-jsites-verdicts.md.
- FIRST RUN (pre-fix): 62 sites. AFTER the two CONFIRMED sites are renamed/folded: **57 remain:
  WRONG-OWNER 15, AMBIG 16, NONE 26, CONFIRMED 0.** The j<->b hypothesis is DEAD as a blanket
  rule - the two confirmable sites needed j->d and j->f respectively, and the other two letters
  of the flip hypothesis appear nowhere as a winner.
- RENAMED (2 sites, 2 files, zero-include headers - truth repair, not build change):
  Gfx_Engine.hpp `virt_meth_0x5bdje0` -> FOLDED into the typed `virt_meth_0x5bdee0()` decl
  (unique candidate; owner-verified: Gfx_Engine vft 0x6ae7ec slot 90, f3_address_lookup);
  cls_0x538f30.hpp rewritten from full fingerprint (class name site cls_0x538je0 -> cls_0x538f30
  = file base + DB key + exact member {2c,30,50,54} + method {f30,f50,f90,fe0} match; the export
  had ALSO dropped meth_0x538f30 entirely - restored; guard fixed).
- NOT RENAMED - what the verdicts mean for follow-on action:
  * WRONG-OWNER (15): each has ONE real-EA candidate, but under another class (e.g. Window.hpp's
    0x4aje80 -> 0x4abe80 = cls_0x4abe60's method; VFX_Model's 0x4je0c0 -> 0x4fe0c0 = the VFX
    default-writer family). These are 2024-export MIS-ATTRIBUTIONS: a rename would relocate a
    method into the wrong class - the right action per site is DELETE-WITH-CITE under the
    GameActionAttack.hpp:61 rule when the header is zero-include (all verified so far), or
    re-parent when the consuming lane shows up. Candidate + owner-of-record named per site in
    the report.
  * AMBIG (16): 2-3 in-DB candidates. Tie-break sources exist and are queued next: member
    FAMILIES (AreaMapWindow mbr_0xje0/4/8/c = {0xde0,0xee0} x4 - cross-check BOTH classes' full
    member sets, and the identical Edit.hpp family, to find the digit that keeps the 4-family
    contiguous AND present); method BANDS (VFX_Model 0x4je6x0 run vs the class's real vft).
  * NONE (26): no single-digit variant exists AT ALL - these names were never EAs or lost more
    than one digit; un-repairable by substitution. Action: named-absence ledger entries only
    (done - the report IS the naming), retire-on-consumption.
- 'typeless decls' half of the row: the bare `name;` lines are pervasive in the same DB-era
  headers (10+ files carry them; the two CONFIRMED sites were both bare-line duplicates of
  typed decls - folded). A separate pass enumerating bare decls vs F3.json method arity is
  queued; NOT conflated with the j-site sweep.
- Gates at this commit: headers are included NOWHERE (verified per file) -> no compile surface;
  run-all + msvc + annot still run as discipline (expect unchanged-green).

## CLOSURE (same day) — 57/57
- The verdict pipeline evolved per-site: single-digit only {2/16/16/28} -> discover the **je->f3
  export law** (first on ONE Window family by hand, then admitted to the candidate SET only,
  never as a blanket rename) {45/12/0} -> add same-KIND NEIGHBOR sandwich (a sorted block's
  position fingerprint = the adjacent same-kind lines; global min/max broke on Edit.hpp which
  sorts LEXICOGRAPHICALLY, mbr_0x1000 right after mbr_0x10 — the lo>hi veto now returns no
  bracket instead of a false one) + same-file DUPLICATE exclusion -> {57/0/0}, rescan 0.
- Every rule pinned by a selftest (S8 je->f3, S10 sandwich, S11 dup, S12/S12c same-kind neighbor
  + lexicographic regression, S9 template owner, S7 fwd-decl owner) — 12/12.
- The PARK hypothesis j<->b: CONFIRMED-NOT-ASSUMED, and it lost — b was never a winner anywhere.
  The row was RIGHT to refuse bulk rename: a bulk rename under the plausible-looking hypothesis
  would have mis-named 55/57 sites.
- Sweep safety: baseline per-file syntax parity BEFORE edits; 29/31 headers never self-compiled
  (DB-era include debt, pre-existing, zero-include files); post-sweep every touched file has
  ZERO own-file errors; the one worse-than-baseline file (Edit.hpp duplicate decl of
  meth_0x4c3af0 with the comment naming the true meth_0x4c4ed0 — both real in Edit's table) was
  a PRE-EXISTING export defect, caught by the parity check, fixed in-pass with cites.
- Typeless-decl conversion: bare `name;` lines retyped to `undefined4 name();` house style
  (folded into typed twins when present) — the row's second half, done at the sites the row
  touched; a FULL typeless-decl census across all DB-era headers is NOT this closure and is
  named as the successor below.
- Gates at close: run-all 11/0 app=124, port-annot GATE PASS stock 0/390, TRUE msvc rc=0 (no
  pipe), rescan TOTAL 0. openje tip 39da531 (33 per-file commits), pushed.
- SUCCESSOR (named, not smuggled): full typeless-decl sweep of the 2024-export headers (arity
  from decompile where a consumer exists; leave cited otherwise) + GUI/Base.hpp include-debt
  (LONG/FILE) if any lane ever starts consuming these headers.

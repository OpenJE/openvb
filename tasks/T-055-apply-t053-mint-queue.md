---
status: done
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — queue author promoted to writer; GoldYak left the mesh at the T-055 ping)
type: writer wave (sole WRITER on ida_original_f3; T-033 rules apply verbatim)
depends: T-053 queue (commit `5d48e35`, accepted by coordinator)
---

# T-055 — Apply the T-053 mint queue (batch-7-shaped wave, new evidence class)

## Objective

Convert CalmRaven's handoff-ready queue into durable IDB names: the GUI window-interface ctor
own-vft family (headline `JE::CDebugInterface` 7 fns + 13 ctor-band single-joins) plus the 6
merge-into-existing-class candidates. ~30 fns at stake; this wave tests whether the b6 "queue
exhausted" statement needed exactly this qualifier: exhausted for DENSITY-GATE-visible bands, not
for 1-2-fn ctor families.

## Steps / scope boundaries

1. Work ONLY from `discoveries/2025-10-21-t053-string-mining-unexamined-bands.md` §2/§3. Re-verify
   EVERY claim at write time against the live image (EA anchors, name churn assumed): own-vft store is
   SOLE-storer (re-check vft xref count before each mint), the string actually sits in the function's
   band, `.INT` name loads in the ctor.
2. HARD exclusions: the 5 `0x434xxx` helper-theft fns (CFeedback region — merge 7, NOT 12 for
   CDebugInterface); CNAS/cls_0x426720 (§named-risk, own-vft unverified — skip unless you verify
   sole-storer yourself and say so); §3 Map 0x577290 — do NOT merge until the member-vs-helper
   conflict with the effect-registry doc is reconciled in your ledger (pick a side WITH cited bytes).
3. GameWeapon 0x4148a0/0x413810: doc-claimed-never-renamed (T-015/B11) — rename-only closes a real
   tail; cite the doc pair in the ledger. ResourceFileManager singles: b6 precedent = single-fn joins
   OK, band NOT renamed.
4. The 13 §2b single-joins are NOT certified by the researcher and you don't have to certify them
   either: apply per your own sole-storer check; label-and-skip any that fail re-verification, and
   itemize accepted vs skipped in the ledger.

## Acceptance

- Ledger `discoveries/2025-10-21-t055-mint-queue-apply-ledger.md`: per-fn table (string EA / ctor EA /
  vft EA + sole-storer re-check result / verdict / rename applied), T2 BEFORE/AFTER with the delta
  itemized by family (headline 7, single-joins N/13, merges N/6), CNAS/Map/helper-theft dispositions.
- Save discipline NON-NEGOTIABLE: mid-wave checkpoint save + end-of-wave `save_database`, mtime check,
  fresh-dump verify disk==live (the standing rule; writer slot ends only after proof).
- Board: this row flipped done with evidence pasted + coordinator checkpoint of the ledger.

## References

- T-033 ledgers b4/b5/b6 (hierarchy + escalation rules), T-053 queue (scope), harness items 35/40/43
- `.pi/skills/openvb-ida-mcp/SKILL.md` (save discipline)

## Acceptance evidence (delivered 2025-10-21)

- **Ledger** `discoveries/2025-10-21-t055-mint-queue-apply-ledger.md` (commit `13d3536`): per-fn table
  (string EA / ctor EA / vft EA / sole-storer recheck / verdict / rename), accepted-vs-skipped by family,
  T2 delta itemized THREE ways. HEADER = canonical sole-storer predicate (item 41 rule 3).
- **Renamed 41 fns (T2 −34, T3 −7):** T-053 queue −23 (headline CDebugInterface **7 NOT 12**, 5 @0x434xxx
  helper-theft rejected; 14 window-interface ctor own-vft joins incl CNAS **verified not waived**; 2 _Tree
  single-fn joins, band intact) + T-054 ext −11 (8 HotKeyWindowFormatTable band fns via `-0x198` containment,
  4 Gfx_BufferManager VB/IB) + 6 merges T3→T1 (Map reconciled to **member** with cited bytes) + 1 dtor.
- **Annotation-only:** `cls_0x4b28a0` — 142/142 external `this==&0x707E48` verified, vacuous membership,
  comment set @0x4B28A0, **no rename** (28-fn band retained). T2 delta 0.
- **Census:** T2 2,046→**2,012 = 22.67%**, game denom 8,876; cumulative vs pre-b2 2,340→2,012 = **−328 exact**.
- **Save discipline:** checkpoint 15:11:11 + end-of-wave `save_database` 15:31:28; fresh-dump re-read confirms
  every name + comment persisted → **disk==live proven**. Dumps `discoveries/wavedumps/t055-{pre,post}dump.json`.
- **Residuals correct:** `cls_0x558740::`=5 (helper-theft), `cls_0x445350::`/`cls_0x507810::`/`cls_0x5067c0::`=0,
  `cls_0x4b28a0::`=28 (annotation-only), `cls_0x50dbb0::` band intact (only 2 single-fn joins out).
- IDB: **8745 held; end-of-wave save 15:31:28; full devenv re-up HELD on coordinator until wave close reported**.

## EXTENSION (coordinator, same wave, post-checkpoint): apply the T-054 adjudications
After the T-053 queue lands and you checkpoint-save, fold EpicRaven's wave-queue into the same wave
(ledger sections clearly separated by source row):
- cls_0x445350 band -> owner naming per MINT-READY chain: host cls_0x4468C0 (COptionsHotKeysInterface
  impl; vft 0x65A290 outer + interface vft 0x659DB0 at +0x198; thunk 0x4468B0; 14 sites at -0x198) —
  name both the host methods and the band entries per EpicRaven's §2, citing their doc; factory-2nd
  xref 0x445763 logged EXPECTED per corrected predicate.
- cls_0x507810 (+ cls_0x5067c0 same-object question) -> Gfx_Engine+0x30 heap member mint per §verdicts;
  naming choice is yours — no shipped string; ctor site 0x5BFFEC / install / dtor 0x5C1D41 cited.
- cls_0x4b28a0 -> ANNOTATE ONLY: comment at 0x4B28A0 recording 166/166 this==&0x707E48 singleton purity
  + vacuous membership so future waves stop hunting an owner that doesn't exist. NO rename.
Acceptance unchanged plus: T2 delta itemized THREE ways (T-053 queue / T-054 extension / annotation-only
zero-delta), and EpicRaven's discoveries doc is your citation source, not your evidence — re-verify at
write time same as everything else.

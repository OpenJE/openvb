---
status: in-progress
assignee: SwiftOwl `01a16198-6cbb` (WAVE-4 SEATED per QuickNova `16a0622` queue call: "re-triage NOW, then open WAVE-4 as seat-5"; offered in PARK since the TrueCastle death; sole IDB writer while seated - preflight save-sync FIRST, per the wave-3 protocol this file reuses verbatim)
type: WRITER WAVE — re-cut + annotation (live-vs-plan per row, item 54 law; batch save chain; disk==live verify)
depends: w3-postdump.json (freshest live census, 2026-10-21), qw2-arbitration-class1.txt (the table to refresh), discoveries/2025-10-22-getglobal-setglobal-seh-triage.md (fresh manifest feed), wave-3 ledger flags 1-3
---

# WAVE-4 — class-1 re-cut, feed-unique salvage, toBool, triage rows

## Manifest (PARK row + coordinator additions; HOLD-21 explicitly NOT here - values-era owner-chase)
- **W4-R1 class-1 REFRESH/re-cut** - wave-3 flag #1 ('table refresh suggested'): re-derive
  (hostClass, terminal-field) per row from `hosts --cls <band> --dump
  discoveries/wavedumps/w3-postdump.json` for every distinct band in qw2-arbitration-class1.txt,
  diff against the table PLAN, verdict per row (item 54: the live dump is the record, the table
  is the plan; 0x4903E0 = the reference staleness case). Output: qw2-arbitration-class1-refreshed
  + row-by-row verdict log. Renames ONLY where refreshed-derivation confirms a LIVE name that
  contradicts the dump-truth (already-applied wave-3 names are LIVE - re-applying the OLD table
  is the wave-3 near-miss; never regress).
- **W4-R2 4 feed-unique rows** - TrueCastle's banked feed (rinst-install-deriv.md) + EpicRaven
  cross-check = salvage-with-audit (their sessions dead; the R-ANNOT salvage precedent applies):
  re-verify each row's install-site cite against w3-postdump + live reads, then apply or HOLD-
  with-cites per row's own evidence strength.
- **W4-R3 Win95IsDebuggerPresent -> toBool annotation** - the R-VEG-E named mislabel (annotation-
  tier, no rename): IDB comment at the def site citing the R-VEG-E close + the value-domain
  receipt ({0,1} to-bool semantics), read-back verify.
- **W4-R4 GetGlobal/SetGlobal triage rows** (coordinator: verdicts feed this wave directly) -
  IDB annotations from discoveries/2025-10-22-getglobal-setglobal-seh-triage.md:
  sub_5A9D50 predicate (is-registered), cls_0x50db20::meth_0x5A9E60 (toInt extractor - the
  toBool sibling), globals slot +0x6C find-by-name census note on cls_0x616328 (DUAL ROLE:
  pump-timer + globals owner - re-cut check FIRST: item-54 shared-adjacency discipline, do not
  mint on a two-role read without band evidence), the SetGlobal create-fallthrough (warning-only
  @0x58e2a4), and the two OPEN items left named-not-guessed (message tail past 'It',
  meth_0x58A9A0 set-body).
- **W4-R5 R-INST reactivation line HOLSTERED** (unchanged): fires ONLY if a row outside the 74
  fails committed regen; status recorded at close either way.

## Protocol (wave-3's, verbatim duties)
1. Preflight: gate-6 equivalent = ecx-flow selftest GREEN in-code above (done, seat-time) +
   IDB save-sync BEFORE first write (live==disk via MCP idb_save + read-back spot-checks);
   if the live IDB drifted from w3-postdump beyond the wave-3 applies, that drift = new rows
   in W4-R1's verdict log (name-diff census over the 65 table EAs first).
2. Band-batches: per host band, regen-recheck + share-check per row -> rename/comment via MCP
   -> read-back per EA -> per-batch checkpoint save -> END save + w4-postdump -> reconcile
   name-diff count vs plan, delta to row list -> ledger = recovery source (full EA table).
3. IDB SOLE-WRITER: claim = this file + reservation; any other seat's IDB write request queues
   behind close or the named cliff bank.
4. Close: gates at close (repo: run-all 11+1 / msvc if ports untouched-by-IDB / annot; IDB:
   disk==live verify), ledger in discoveries/, PARK flip, relay.

## Cliff bank (if context bites mid-flight)
Progress MUST live in the commits below (protocol step = commit per batch with the verdict
log). A fresh seat resumes at the first unchecked row here:
- [x] seat + protocol file
- [x] selftest green
- [ ] live-vs-w3-postdump drift census (65 table EAs)
- [ ] W4-R1 refresh (per-band regen)
- [ ] W4-R2 salvage
- [ ] W4-R3 toBool annotation
- [ ] W4-R4 triage annotations
- [ ] W4-R5 holster note
- [ ] END save + w4-postdump + reconcile + ledger

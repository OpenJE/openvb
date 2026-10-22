---
status: done
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (WAVE-3 SEATED 9346a5f — sole IDB writer; TrueCastle rest preserved, wave-4 named for their wake)
type: WRITER WAVE — MINT tier (renames; T2 moves — forecast before apply, measured after)
depends: EpicRaven's committed class tables (qw2-arbitration-class1/2.txt, 2086aef/d2eb76a), R-CS §7.1, R-ST successor #2, TrueCastle feed (rinst-install-deriv.md)
---

# WAVE-3 — adjudicated class tables + ST-RENAME + derived-in-base + install-cited re-hosts

## Manifest (from the Q-W2b PARK row)
1. **class-1: 52 MINT-CONFIRMED** → rename each to its doc class, per-row regen re-run BEFORE apply
   (the no-self-waiver leg for this wave = `hosts --addrs <ea> --dump t064-postdump.json` + share-check).
2. **class-1: 13 RE-HOSTED** → rename to the KEY holder (install-site cites carried into comments);
   six Client→World rows (install proof @0x548BEA/BF5), JE::AreaMap@+0x60, net::connection@+0x1C, and
   the cls_0x570800/sub_595C40/sub_411330-band re-hosts (band-consistent labels; anonymity where the
   host band is itself unnamed = deliberate: name-correctness beats prefix cosmetics).
3. **class-2: STRONG-key mints** per table (0x515040, 0x515D50, 0x4A5A10, 0x523450) — table says 4,
   commit message says 3: DISCREPANCY resolved per TABLE (the regenerator output is the record; the
   count is noted in the ledger both ways — item: table-vs-prose drift, cite don't smooth).
4. **class-2: WEAK rows ×3** (0x52CC70, 0x5150C0, 0x436410): writer-discretion per the row's own
   clause — mint ONLY if band-level regen confirms, else annotate-consumer. Decision + reason per row.
5. **§3-STANDS ×2** (0x515580, 0x4A5720): no action; ledger records the stands.
6. **ST-RENAME pair**: 0x4B2DD0 → std-tier name, 0x56EE40 → std-tier name (names OUT of the
   StringTree prefix; comments already carry the truth; collision-checked against live std:: labels).
7. **0x539370** derived-in-base: `JE::CSound::meth_* → JE::CMusic::meth_*` (R-CS §7.1 proof + §9 vft note).
8. **4 feed-unique rows**: EXCLUDED pending EpicRaven's cross-check (explicitly "if they're still
   standing" — not yet ruled; relay ask status at close; they join wave-4 if cleared).

## Ambiguity gate (resolve BEFORE touching IDB; outcomes in the queue table)
- `doc=F3` (7 rows: 0x442BF0/0x4433E0/0x5813A0-C0-E0/581420-430-470/5817A0/5B5640) and `doc=JE`
  (6 rows: 0x55BF60/55BFA0/59AAA0/5B2900/5B2990/5B2C80/5B2CD0) — bare-namespace holders are NOT
  rename targets; resolve each to its concrete live class via the site EAs in the key column
  (t064-postdump containment + regen); unresolvable = HOLD-with-cites (annotation instead of guess).
- AMENDMENT-3 collision rule stands as contract: fn-level STRONG mints, §3 annotation for remainder.

## Protocol
gate 6/6 first -> save-sync before first rename (live==disk) -> band-batches (per host), each batch:
regen-recheck + share-check per row -> rename via MCP `rename` -> read-back per EA -> per-batch
checkpoint save -> END save + postdump -> T2 measured vs forecast, delta reconciled to the row list
-> ledger as recovery source (full name+EA table) -> relay + row flips.

## Close evidence (this commit)
50 renames (44 class-1 confirms+rehosts after 1 stale-row drop + 4 c2 strong + 2 std-tier + 1 CMusic), 34 comments (10 install-cited rehosts, 3 WEAK consumer verdicts, 21 HOLD-with-cites), T2 1892->1847 EXACT reconciliation, chain 00:56:49/00:57:22/00:58:16/00:59:02Z, dumps banked. Three flags banked (stale row, count drift table-authoritative, feed-unique excluded). Successors: 21-row values-era chase batch, table refresh for wave-4, feed-unique on cross-check.

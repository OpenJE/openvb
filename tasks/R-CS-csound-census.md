---
status: in-progress
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (PULL from PARK R-CS, dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: research (read-only + offline; NO IDB writes; no live-IDB queries heavier than xref-read; census feeds the cursor+audio port era)
depends: T-062 §1 (CSound cursor object, tag-sniffer, master-gate), T-086 E1 (CSound-epoch field edges into ResourceSystem), ecx-flow @90864fd (gate-first)
---

# R-CS — CSound class census: anatomy of the 0x539xxx band + full member/slot portrait

## Objective
T-062 NAMED the cursor object as a CSound (RTTI COL 0x671408 → 0x6D4098 → TD `.?AVCSound@@`) and
flagged the export's size 281 as OOAnalyzer inflation-family, but never drew the portrait. The
0x539xxx band (T-062 named-and-excluded) plus the 0x4C6xxx–0x4C8xxx CSound epoch get a full census:

1. **Instance size adjudication**: export 281 vs the 0xF8-instance evidence — locate ALL
   `new(size)`→ctor sites, pin the true size bytes-first.
2. **Member anatomy**: full member table (offset, size, role-cited where cheap, usage EAs) —
   byte-checked against the export, export treated as locator-not-authority (T-040 rule).
3. **Vft anatomy**: every CSound vft (COL/TD chain, slot count, per-slot target + role), incl. the
   loader vft[0]=0x4c82e0 idempotence-bail DEMAND path T-062 found.
4. **The 0x539xxx band**: what it actually is (T-062 'named-but-didn't-portrait' — names live in
   t064 dump), its relation to CSound (methods? helpers? separate class sharing the epoch?),
   the 3-char tag-sniffer epoch (0x4c6c30), and the Play/Enable master-gate ([0x70B250]+0x48).
5. **Allocator/cleanup chain**: new/delete wrappers, SEH pads (T-059 taxonomy), the trio-releaser
   0x401D60 if it appears, and audio-master singleton ownership.
6. **Machine verdict**: share-check @90864fd on the census band (census tier, but the print is
   free jurisprudence; feeds E1's R1/R2 discrimination from the CSound side).

## Steps
- gate: `python3 tools/ecx-flow.py selftest` 6/6 FIRST (md5-pinned a68c22f3…)
- bands from: `f3_class_info(.?AVCSound@@)`, t064-postdump live names (0x4C6xxx–0x4CAxxx,
  0x539xxx), raw objdump (pinned cache), RTTI COL/TD walks, new-size call census
- deliverable: discoveries/2025-10-21-r-cs-csound-census.md (LEFT DIRTY), per-fn rows, writer-ready

## Acceptance
discoveries file with: true instance size + the allocator-site proof; member table byte-checked;
vft slot table; 0x539xxx band classification with cited edges; cleanup-chain map; share-check print;
explicit cross-note to T-086 E1 (does the CSound side settle cached-alias-vs-theft?).
Ping [DISCOVERY-BATCH ready].

## Notes
Lane contract honored: file created AT CLAIM; PARK row relayed to coordinator (coordinator-maintained
table, not touched by lane). KeenStorm/HappyEagle files untouched. Read-only; offline-first.

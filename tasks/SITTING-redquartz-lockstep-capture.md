---
status: in-progress (ACCEPTED from offer 38a97c8 — keeper's first solo crown event; the gate was a
  working rig, the rig has been built twice from paper, the docs carry the harness shape, and the
  readings are pre-written. Declining a seat whose fire conditions I built myself would contradict
  every thesis on this board tonight.)
assignee: RedQuartz (rig keeper; ex-HappyEagle seat, died in the freeze — context on paper, see below)
depends: T-052/T-056 (harness+wall+recipe), T-067 (DXVK stack for original-exe longevity),
  T-085 §6 (bp7-bp13 ladder), R-AMX §5 (bpN1-bpN5 crosswalk), docs/f3/sitting-runbook.md (composition)
---

# THE SITTING — lockstep capture run, original F3.exe, keeper-grade rig

## Offer terms (QuickNova 38a97c8)
Re-seated to the keeper; scope when slice-2 acceptance is clear: rebuild lockstep harness per
T-052/T-056 docs, boot ORIGINAL F3.exe under keeper prefix, run the pre-armed ladder. Full IDB-READ
access, no writes. Run-identity ledger on every window (keeper law #126). Five S-PEND cells ride:
bp10 = T-050 falsifier, event-kind numerics, >4 depth, cursor +0xA4 fill, 801 cross-refs,
plus the digest fold waking WildZenith's successor row.

## Asset audit (verified this sitting, NOT assumed)
| asset | state |
|---|---|
| `docs/f3/sitting-runbook.md` | TRACKED and current (2461 B): bp7..bp13 (T-085 §6) + bpN1..bpN5 (R-AMX §5) + NEW RUNGS; docs-win rule stated (T-085/R-AMX override this file on disagreement) |
| `tools/t052-drive.py` | REPO-RESIDENT, NOT dead-with-/tmp — header: "RE-LANDED harness v3 successor... Supersedes /tmp/t052-drive{,L}.py" (md5s confirm divergence). Services bp1-bp6 + seam watch legs; fifo discipline documented (held-writer or the inferior goes debugger-LESS) |
| `tools/t034-capture.py` | documented TARGET SHAPE for the driver — read before edits |
| winedbg | `/nix/store/hf94...-wine-11.0/bin/winedbg` present |
| original exe | `$OPENVB_ORIG_EXE` = /nix/store/a1j4.../f3demo/F3.exe, READ-ONLY store (boot precedent: HappyEagle ran it from cwd=demo-root repeatedly on this exact stack) |
| rig | warm :96 (pid 18412) + cold-bootstrappable `tools/rig-g1-run.sh`; for the SITTING use the DXVK env (T-067: original-exe longevity 12 min+ CPU clean; the softpipe stack dies at bink upload 2.5-4 min) |
| /tmp salvage | `t052-driver{D,I,J,J2,L}.out`, pid files — HappyEagle run outputs = signal; DOCUMENTED, not cleaned, not relied-on (repo driver supersedes their .py ancestors) |
| xdotool | on PATH (item-60 era) — the input-injection subproject's enabler |

## Known hazards (named BEFORE chasing — each has a pre-written answer)
1. **BP-NAMING COLLISION**: driver's legacy legs are named bp1..bp10 where ITS bp7=0x581810
   (Graphics::Initialize) and bp9/bp10 = seam watch; the RUNBOOK ladder re-uses bp7..bp13 for the
   T-085 scheme (bp7=0x54c833...). The runbook SUPERSEDES earlier sketches — first work item is a
   naming reconciliation pass on the driver legs (paper+code, no IDB writes) so a relay can never
   confuse "hit bp9" across the two tables.
2. **CODE-ON-ARRIVAL legs** (T-067 addendum, banked): driver needs bp8 debounce-pair logic (0x54c864
   two-occurrence) and bp12 either-or site counts (0x54c9c1 vs 0x54c92a) before their readings are
   harvestable. "Minutes, not rest-breakers" — per the lane that wrote the note.
3. **INPUT-INJECTION GATED CELLS**: ">4 depth" and live-Attack are DEMO-SCRIPT-limited (T-067: max 3
   at 12 min crash-free; cap is the script). Those two cells require xdotool SendInput into the :96
   window — a new mechanism class, explicitly OUT of every prior sitting. Subproject S3b; other three
   cells (bp10 falsifier, event-kind numerics, cursor +0xA4 fill, + 801 cross-refs) are passive-armable.
4. **LONGEVITY**: run the ORIGINAL on the DXVK stack (registry overrides, nv32 ICD, no softpipe) —
   the wined3d-11 cs-thread bug (the 2.5-4 min SIGSEGV) is why T-056 died there; T-067 proved DXVK
   runs it 712 s CPU clean. Pairing rule from T-067: same-env addresses, cross-env constants — this
   sitting is a DXVK-env capture, label it so in the digest fold.
5. **HYGIENE CONTRACT (unchanged)**: managed winedbg pairs :31337/:31338/:31437 NEVER touched;
   sitting uses SCRATCH PORTS 31440+ ONLY; every kill environ-gated (/proc/<pid>/environ WINEPREFIX
   match); private prefix per window; IDB READ-ONLY; no src/ writes (driver = tools/, keeper's lane).

## Stage plan (each stage = one ledgered window, named TRACE2 per item 61)
- **S0** naming reconciliation: driver header map vs runbook tables; produce one unified arm-list.
- **S1** bring-up smoke: fifo-held winedbg :31440, original F3.exe under keeper prefix + DXVK env,
  driver v3 services bp1-bp6 while the ladder arms; gate = boot past intro alive (600 s budget).
- **S2** ladder harvest (passive cells): bp10 zero-or-hit (T-050 falsifier — ANY hit = loud relay),
  bpN4 kind histogram, bp13 sink census, bpN1/N2/N3/N5 push tallies, event-kind numerics at
  cursor-reload bps, cursor +0xA4 fill site, 801 cross-refs vs sightings.
- **S3** input-injection subproject (xdotool) → >4 depth + live Attack + bp5 firing; T-077 shape-A/B/C
  reading rules apply verbatim (single-discriminator = done-byte at 0x54c7f6 same-walk; pre-specified
  readings, no freelancing).
- **S4** digest fold + relay: results formatted for WildZenith-successor handoff; interpretation
  stays with the docs' pre-specified rules — the run is mechanical, the READINGS were pre-written.

## Verdict slots (fill as harvested — table-or-named-mechanism, honest-negative per cell)
- bp10 hits across a full DXVK boot: TBD  (T-050: reopen or closed-as-unobserved, both gold)
- event-kind numerics {1,3,4,5,7}: TBD
- deque count >4 observed: TBD (if no-injection: NOT-CLAIMABLE-ABSENT, script-capped, say so)
- cursor +0xA4 fill site + GameCursorManager+0xA4: TBD
- 801 cross-refs residual: TBD
- windows ledgered with exe-sha of the ORIGINAL (f3demo F3.exe — static; head at run = git provenance)

## S0 RECONCILIATION (complete — no window needed, pure paper-vs-code)
**The collision is worse than naming — it is DISPATCH-BY-NUMBER.** The driver's elif chain services
GDB numbers 1..10, which are purely the ORDER of the `break *<ea>` lines pushed into the fifo (arm
order = number assignment). Adding any runbook leg without a plan renumbers what follows and can
land, e.g., raiser 0x54c834-family under the GFX1 handler — wrong reads, confidently. Never
re-order the legacy break list without rewriting the dispatch table.

UNIFIED ARM PLAN (two-phase, numbers allocated ONCE):
| gdb# | EA | role | handler state |
|---|---|---|---|
| 1-6 | 0x48ccf6, 0x552bb5, 0x550720, 0x550741, 0x459f5d, 0x56af50 | T-034 six reads (R0-R4 + tick) — NOTE driver#3 == runbook bpN1 | EXISTING, keep arm order verbatim |
| 7-8 | 0x581810, 0x5818d0 | FindMode-era GFX probes (legacy bp7/bp8 — DEAD EAs for the ladder, but the handlers hold the numbers) | EXISTING; arm ONLY in a dedicated FindMode window, never co-armed with T-085 legs |
| 9-10 | 0x5a94a0, 0x5a955d | seam watch — driver#9 == runbook bp11, driver#10 == runbook bp10 (the falsifier, SAME numbering by luck) | EXISTING |
| 11+ | 0x54c833, 0x54c864, 0x549090, 0x54c9c1, 0x54c92a, 0x548d80, 0x54c440, 0x54ee60, 0x46f290, 0x5505F0 | T-085 bp7/bp8/bp9/bp12a/bp12b/bp13 + R-AMX bpN2/bpN3/bpN4/bpN5 (bpN1=driver#3, bpN10 falsifier=driver#10) | CODE-ON-ARRIVAL (S2): bp8-debounce pair, bp12 either-or tallies, bp13 census reads, bpN4 kind histogram; runbook bp11 re-entrancy DEPTH (mbr_0x8 read) extends driver#9's leg |
| 15+ | cursor-reload bps + window-event sites | event-kind NUMERICS + +0xA4 fill (S-PEND cells 2&4) | TO-ARM per T-067 BONUS-4/5 notes (EAs: cursor bps from T-062 docs; GameCursorManager+0xA4 = the BONUS-5 watchpoint) |

Vocabulary law for relays from now on: **quote EAs, never bare bp names** — "bp10 (0x5a955d, the
falsifier)" not "bp10". Both tables use it, the numbers diverge, the EAs do not.
Also corrected: the driver's SHAPE-OF-USE header still shows the softpipe env (wined3d era) — the
sitting arms the DXVK env per hazard 4; header gets updated at S1 commit (tools/ = keeper lane).

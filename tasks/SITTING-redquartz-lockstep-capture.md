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
- cursor +0xA4 fill site + GameCursorManager+0xA4: STATIC-CLOSED 2026-10-24 — fill = 0x58f1d3 (mov [esi+0xA4],eax) in JE::Client::virt_meth_0x58f010, the install hub; factory = slot-45 0x45e940 (new(0xB0)+GameCursorManager::ctor_0x436330) dispatched at 0x58f1cd; ctor zero-inits first (0x58e971). Evidence: discoveries/2025-10-21-t062-cursor-subsystem-and-child-notify.md item-2 addendum (9167884); live [cursorMgr vft+0x10] sighting still open for the SetCursor half
- 801 cross-refs residual: TBD
- windows ledgered with exe-sha of the ORIGINAL (f3demo F3.exe — static; head at run = git provenance)

## S1 BRING-UP VERDICT (COMPLETE — keeper window 2026-10-22T00:57→01:07, head 24f6f98-era)
Original F3.exe (sha256 3194880 B store exe, ledgered to sitting-runs.log), fifo-winedbg :31440,
keeper-grade DXVK prefix (clone of r-rig), driver v3 servicing legs 1-10 in dispatch order.
**GATE: PASS — alive 9m15s, ZERO SIGSEGV (the wined3d bink wall at 2.5-4 min left far behind),
DRIVER-END r1=4 tick=6 pump=5 funnel=12 post=35 seamA=25 seamB=0.**
Historical fingerprint MATCHED line by line: post=35 (T-056 runs C/H, T-067 run I — 4th deterministic
boot), seamA=25 (T-067 run I exactly), bp8 (0x5818d0 FindMode-fatal) 0 hits (rig law holds for the
ORIGINAL under DXVK), bp5 (live Attack) 0 (script cap, as every prior boot), net chatter 'unknown id 7'
(T-067's live-session signature). First S-PEND cell filled: **falsifier 0x5a955d = 0 hits across a
clean 9-min DXVK boot** (consistent w/ T-067; re-check after S3 injection before any T-050 wording).
Six-row read shapes captured live in values.log (world 0x7e16740, Greg 0x0a1fa8b8, +0x34=0x2e@R1-time).
Teardown: every window pid environ-gated to /tmp/sitting-redquartz/prefix before kill (stub trio took
TWO passes — launcher dies, wine-host children persist; lesson banked); managed 31338/31437 verified
STILL UP and untouched post-teardown. Driver header usage updated to the DXVK env (softpipe demoted to
legacy FindMode one-offs, never both stacks in one window).

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

## S2 window attempt 2026-10-23 15:53-16:02 (splitseat; ABORTED-ghost, torn down clean)
What was tried: the banked recipe (clone r-rig DXVK prefix, fifo + held-writer, winedbg --gdb
--port 31440 scratch, unified arm order 1-20, driver v3, 560s deadline) via nohup-detached script.
What failed: the held-writer + fifo dance does NOT survive detached launches — the gdb frontend
started (banner in dbg.log), the arm-push blocked on the fifo (no reader had opened), the held
writer vanished from the process table, and the inferior free-ran debugger-LESS (the exact ghost
family the dance exists to prevent: silent EOF detach). Arm list never landed; values.log empty.
Disposition: environ+cmdline-gated teardown of every sitting-redquartz process (winedbg 0120,
held writers, drivers); survivors 0; managed 31337/31338/31437 verified untouched throughout;
prefix clone kept for the retry (dosdevices diff vs r-rig = transient dll tmps + one F3.log only).
DURABLE CORRECTION for the next attempt: run the window FOREGROUND under a session-owned long
timeout (the prior keeper's S1 window was a foreground window; detached nohup breaks the
process-group assumptions the dance relies on). Also fixed incident-wise: the t052 driver has no
WINEPREFIX in its environ — cmdline-gating is required for it, environ alone misses it.

## S2 window attempt-2 2026-10-23 23:47 (splitseat; COMPLETE — foreground, rc=0, ladder serviced)
Executed the banked durable correction verbatim: session-owned foreground window
(`/tmp/sitting-redquartz/s2-window.sh`, 560s deadline), prefix clone reused, Xorg :96 cold-rebuilt
from the r-rig config (stale-socket guard added: X96 with no owner is rm'd before bind — attempt-2
hit it once), unified arm order 1-20 accepted by gdb (20/20 "Breakpoint N at" lines). TWO window
machinery fixes landed in the script itself, both honest: (1) the held-writer open must come AFTER
the reader's open or `exec 9>fifo` deadlocks (write-open blocks with no reader — found live, fixed;
the attempt-2a variant never reached the inferior); (2) expanded env-arrays are command words, not
assignments — `env "${WDBENV[@]}"` is the working form. ZERO SIGSEGV again; teardown two-pass,
managed 31338/31437 verified alive post-window; no 31440 stragglers.
**Legacy fingerprint MATCHED the 4th/5th replication exactly: r1=3 tick=6 pump=5 funnel=12
post=35 seamA=25 seamB=0** — the boot is deterministic under the arm plan. LADDER READINGS
(code-on-arrival legs 11-20, raw reads in values-s2.log; interpretation kept to the pre-specified
readings, no freelancing):

| leg | EA | hits | decoded reading |
|---|---|---|---|
| bp7 runbook (raiser R1) | 0x54c833 | 10 | raiser fired 10x in a no-injection boot |
| bp8 runbook (exhausted) | 0x54c864 | 101 (cap) | [ecx+0x318] reads banked raw (decode next window) |
| bp9 runbook (clearer) | 0x549090 | 100 (cap) | +0x300 reads banked raw |
| bp12a/12b either-or | 0x54c9c1 / 0x54c92a | **63 / 3** | pop path A dominates 63:3 — either-or tally LANDED, pair sequence in values log |
| bp13 sink census | 0x548d80 | 101 (cap) | [ecx+0x288] content at hit time: 68x `0x5f4d4101` (bytes `01 41 4d 5f` = `\x01AM_`), 14x 1, 9x 0xffffff01 — raw table banked, NOT interpreted here |
| bpN2 native drain | 0x54c440 | **0** | honest zero: no natives drain in a script-capped boot (consistent with R-AMX: native traffic requires script loads) |
| bpN3 sync mailbox | 0x54ee60 | **0** | honest zero, same boot shape |
| bpN4 action factory | 0x46f290 | 35 | **pushed-K histogram LIVE: 33x kind 1, 2x kind 6** — the kind-numerics S-PEND cell's ACTION-site half is filled; the window-event {1,3,4,5,7} half rides legs 15+ |
| bpN5 anim pushes | 0x5505F0 | 66 | anim deque pushes 66x; [esp+4] raw banked |
| bp11 runbook re-entrancy | 0x5a94a0 (driver#9) | 25 | **[ecx+0x8] depth byte = 0x00 on ALL 25 hits** — zero re-entrancy in a script-capped boot; >4-depth cell stays NOT-CLAIMABLE-ABSENT (correctly), injection window pending |
| bp10 falsifier | 0x5a955d (driver#10) | **0** | 2nd consecutive clean DXVK boot with 0 hits — T-050 verdict slot: closed-as-unobserved so far, re-check after S3 injection before any wording |

Also observed: bp7-legacy (0x581810 Graphics::Initialize) hit ONCE under DXVK (S1-era note said the
pair never hits — refined: bp7 hits, bp8 (the FindMode-fatal stop) does not; the "0 hits" claim in
S1 was bp8-specific). Run identity ledgered: head 8da0668, exe sha256 4652a3d0…3579b87 (SAME bytes
as S1 and as the PE-read table authority — store path differs, content identical).

**STILL OPEN for the row:** legs 15+ (cursor bps + GameCursorManager+0xA4 watchpoint, event-kind
{1,3,4,5,7}), the S3 injection window (xdotool) that the >4-depth and T-050 re-checks are gated on,
and the 801 cross-refs residual. Next attempt inherits: foreground recipe (this script), prefix
clone (dosdevices diff = transient dll tmps + one F3.log), values-s2.log for the raw-read decode tail.

## S3 window 2026-10-24 00:25 (splitseat; COMPLETE — xdotool injection LIVE, all legs serviced, rc=0)
Same foreground recipe (s3-window.sh = s2-window.sh shape), arms extended to 26 (bcf43f3), plus
synthetic mouse input: 40 rounds of xdotool mousemove+click on :96 while the ladder ran. ZERO
SIGSEGV again; teardown clean, managed pairs alive; ledgered head c8432a8, same exe sha 4652a3d0.
Legacy fingerprint held (r1=3 tick=6 pump=5 funnel=12 post=35; seamA=24 — one off the 25, within
the sampling window's arms-cap variance, noted not hidden). INJECTION READINGS (raw in
values-s2.log S3 portion — values path bug in the copied script, fixed for future windows):
- **Event-kind producer trace LANDED (BONUS-4/S-PEND cell 2)**: slot-2 dispatch entry 0x5803e0
  serviced 201×; at the PINNED BRANCH GATES (post-0x580408 load, [edi] = kind, live):
  kind 5 = 39 exclusive at the beam gate 0x580435; kind 4 = 18 + kinds 5/6/2 passing 0x580417
  (falls-through branch semantics); kind 1 = 2 sightings at 0x5804e0; **kinds 3 and 7 NEVER fired**
  — honest named absence: the synthetic mouse stream produces no drag/hover-class events.
  Dominant kind 6 (~39+ sightings) = the mouse-move class our 40×3 moves produced.
  **INSTRUMENT CORRECTION**: [edi] is NOT the kind at the ENTRY bp — edi still holds the caller's
  value until 0x580408 loads the arg (arg1 = [esp+4]); the leg-21 entry reads are stale-EDI reads
  (0x5399348 → [0x66e4d4] event-class ptr), recorded as such; the gate legs are the kind evidence.
- **SetCursor live sighting LANDED (S-PEND cell 4 live half)**: 0x437ab0 hit 20× (cap); path arg =
  `GUI_DefaultCursor.tga` (0x6561ec) ×19 + a THIRD CALLER with `DefaultCursor.tga` (0x654c80) ×1 —
  not the T-062 beam string; a new boot-path SetCursor site identity to chase (named successor).
- Verdict slots now FULLY FILLED or honest: bp10 falsifier 0 hits across 3 clean boots
  (closed-as-unobserved, T-050 gate stays); >4 depth NOT-CLAIMABLE-ABSENT (script cap, T-067
  concurring); +0xA4 static-closed + live-sighted; event-kind numerics live-traced; 801
  cross-refs = the live windows serviced only already-named EAs, so the 801 band stays the
  T-084 queue's business (named successor below).

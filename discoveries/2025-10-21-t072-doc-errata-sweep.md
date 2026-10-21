# 2025-10-21 — T-072 doc-errata sweep: docs/f3/* + docs/DEFERRED-NOTES.md vs the accepted corpus

**Agent:** SwiftMoon `01a16092` (research lane, onboarding row) · **Coordinator:** LoudDragon `01a15f82`
**Task file:** `tasks/T-072-doc-errata-sweep.md` · **Method law:** harness items 40–47
**Mode: READ-ONLY everywhere. Zero IDB writes, zero doc edits. This is a LIST, not a fix.**

## 0. Scope, yardstick, and rules honored

Inventory swept: **all 75 files of `docs/f3/*.md` + `docs/DEFERRED-NOTES.md`** (76 documents).
Yardstick = the accepted findings corpus, primarily the 2026-10-21 wave: T-007 (G1/G2), T-013
static tail, T-033 b6, T-039, T-041, T-042, T-043, T-044, T-045, T-047, T-048, T-052, T-053,
T-054, T-055, T-056 (six rows), T-057, T-058, T-059 (+ amendment), T-060, T-061, T-062 (+ two
addenda), T-063, T-064, T-066, T-068, T-069, T-070 (+ T-071 addendum on the seam doc), plus
Oct-20 canonical answers still unreconciled into the docs (C-21/C-22/C-23, session25 mode-flag,
T-010c B8 reading, death-console registry).

Item-43 discipline applied: **every refutation or anchor I could check myself was re-read from
raw bytes** of `$OPENVB_ORIG_EXE` (PE parse, image base 0x400000, `.text` vma 0x401000 = file
0x1000; recipe in §3). No doc-quotes-doc claim was promoted to STILL-TRUE without a byte or an
accepted-row citation. Nothing was edited; suggested fix shapes are advisory only.

## 1. The big find: a three-way "frame-0 leap" wording collision

The single most dangerous live confusion in the corpus is that **three docs use the phrase
"the frame-0 leap" for different quantities**:

* `SimDigest.md` / `OrigCapture.md` / `ParityOracleDesign.md` (and the DEFERRED-NOTES
  "RESOLVED by statics" section): "the leap is SHIPPED, mirror it, clock is comparable because
  both sides leap" — the **pre-T-045 verdict, kept as live text**.
* `WorldTickPump.md` (T-045) + `WorldYearSeedConsumer.md` amendment: "the ~8-hour uptime fold is
  OURS (the shipped tick is session-elapsed, the stamp `Update` subtracts is SEEDED at
  0x5466bd)" — correct about the fold.
* `2025-10-21-t056-xorg-dummy-six-rows.md` R2 (accepted): "the frame-0 clock leap IS SHIPPED
  BEHAVIOR — confirmed at the prescribed instants" (pump#1 0s → pump#3 1s).

Reconciliation the docs do not yet state: **T-056 confirmed the small shipped seconds-carry
leap (0→1s in the first pumps, and 19–21s at the R1 instant — boot-wall-offset character);
T-045 refuted only the ~8-hour uptime FOLD as shipped (that is a port artifact of
`src/F3.cpp:786` returning raw `timeGetTime()` while shipped `0x56af50` returns the accumulated
session delta).** T-068 then recorded the leap as "ported+correct in app path, UNOBSERVABLE in
harness (driver clock starts 0)". Each individual doc sentence in the collision is either
stale (pre-T-045) or missing the magnitude qualifier (T-056). Hits: S1/S2 (SimDigest), P1/P2
(Parity), O1/O2 (OrigCapture), W2 (WorldTickPump annotate), D1 (DEFERRED), plus the
`timeGetTime` byte-claim in three pages (P1/O1/S2 — raw-verified refutation, §3-1).

## 2. Per-file coverage table (every doc × verdict)

Verdicts: **ERRATA** (≥1 hit below) · **CLEAN** (claims checked, nothing stale found or only
port-nav skeleton) · **STUB** (no EA-level claims; boilerplate links/inheritance graph).
`n` = errata hits.

| file | verdict | n | hits |
|---|---|---|---|
| SimDigest.md | ERRATA | 4 | S1 S2 S3 S4 |
| ParityOracleDesign.md | ERRATA | 4 | P1 P2 P3 P4 |
| OrigCapture.md | ERRATA | 4 | O1 O2 O3 O4 |
| WorldTickPump.md | ERRATA | 2 | W1 W2 (body otherwise current) |
| GUILoadingRecon.md | ERRATA | 6 | Gr1 Gr2 Gr3 Gr4 Gr5 Gr6 |
| EntityActionQueueSeam.md | ERRATA | 4 | E1 E2 E3 E4 (T-071 addendum current) |
| CombatTurnDispatch.md | ERRATA | 2 | C2 C4 (T-069's two fixes verified present, C1) |
| ScriptNativeActionDoors.md | ERRATA | 4 | N1 N2 N3 N4 (N5 still open, honest) |
| CombatRngSeam.md | CLEAN | 0 | R2 strengthened by T-056-R1/T-068; §2 unknown honest |
| StartupAndSubsystemTable.md | ERRATA | 3 | ST1 ST2 ST3 |
| GUIStringTags.md | ERRATA | 2 | GS1 GS3 (GS2 still open, honest) |
| GamePathManager.md | ERRATA | 1 | GP1 (else CLEAN vs T-051) |
| WorldYearSeedConsumer.md | ERRATA | 2 | WY1 WY2 (⚠4 honestly open) |
| T013-registry-port-design.md | ERRATA | 4 | T1 T2 T3 T4 (T5 still open, honest) |
| EntityFlag0x1E4.md | CLEAN | 0 | current (T-041); predicate raw-verified §3-7 |
| EntityRefKey0x1DC.md | CLEAN | 0 | current (T-043) |
| AttackPositionOverrideDoors.md | ERRATA | 1 | A1 |
| GITMItemRecordLayout.md | ERRATA | 1 | GI1 |
| DEFERRED-NOTES.md | ERRATA | 13 | D1–D13 |
| CGameInterface.md, COptionsGameInterface.md, CPipBoyInterface.md, CPipBoyAddNoteInterface.md, CGameFloatingTextInterface.md, GameDialogueInterface.md, GameCSMenuInterface.md, GameCSSkillMenuInterface.md | STUB→ERRATA (class) | 8×1 | I1 (inheritance framing vs T-057 composition) |
| GameCursorManager.md | STUB | 0 | I2 (facts point to T-062; vft slot5 raw-verified §3-5) |
| GameActionIdle.md + 23 other GameAction*/Game* STUBs, Display.md | STUB | 0 | no EA claims; GameActionIdle consistent with landed port file (T-071 in progress) |

(All 75 docs/f3 files + DEFERRED-NOTES accounted for: 20 substantive audited, 8 interface stubs
in errata class I1, 47 remaining stubs with no address-level claims — the stub set is the
class-page skeleton: file links + `openje/docs` references + one mermaid graph.)

## 3. Raw-byte spot-checks (done by this sweep, `$OPENVB_ORIG_EXE`, PE-parsed, image base 0x400000)

```
0x56af50: a1 b8 c2 70 00 c3                    mov eax,[0x70c2b8]; ret  → NOT timeGetTime
0x552ba4: 8d7004 eb02 33f6 8b1e e89e8301 00 50 8bce ff5318  → lea esi,[eax+4]; call 0x56af50;
                                               push eax; mov ecx,esi; call [ebx+18h]  (+4-view slot 6)
[0x6919e4+0x18] = 0x544a90   [0x668584] = 0x544a90   [0x691a1c+0x18] = 0x404b70   [0x691a1c+0x14] = 0x546b50
[0x691a1c+0x0c] = 0x404b70  (primary slots 3 AND 6 are the retn stub; slot 5 is the calendar seeder)
ctor 0x548abb: c706 1c1a6900 / c74604 e4196900 / c7460c c0196900  → tables 0x691a1c/+4 0x6919e4/+C 0x6919c0
                                               ⇒ 0x691A20 is NOT a table head (it is primary+4)
0x54c833: c6 85 19 03 00 00 01                 mov byte [ebp+0x319],1   (raiser, shipped-live)
0x5a955d: 8a 87 19 03 00 00 84 c0              mov al,[edi+0x319]; test al,al  (consumer test)
0x549090: 8b 44 24 04 c6 81 19 03 ...          TurnPrep dt-store then +0x319 clear
0x54ea71: ff 46 10                              inc [esi+0x10]  (deque LIVE COUNT at Entity+0x314)
0x4b1760: 89 56 2c                              mov [esi+0x2C],edx  (slot-15 animation frame WRITE)
0x5a95b0: 8b 87 e4 01 00 00 ...                mov eax,[edi+0x1E4]  (inMap predicate)
Gfx_Engine ctor 0x5bfe10 head contains: a3 e47c7000 (mov [0x707CE4],eax) and c706 ece76a00 ([esi]=0x6ae7ec)
CEI vft 0x69A95C: [0x5803c0, 0x4953c0, 0x5803e0, 0x404b70, 0x404b70, 0x580360]  (slot1 = GUI::Interface
                                               0x4953c0 inheritance; slots 3/4 = ret stubs — T-061 shape ✓)
GameCursorManager vft 0x6550B4 slot5 = 0x437ab0 ✓ (T-062)
GUI::Fragment vft 0x66ea1c: slot7=0x4b20a0, slot12=0x4b1610, slot15=0x4b1720, slot16=0x4b1b00 ✓
0x4b1610 body: 8d81 c0000000 b9 04000000 ...   lea eax,[ecx+0xC0]; mov ecx,4 ✓ (float broadcast, T-062)
0x4b06a0 body: 6aff 83c140 6a00 51 ...          push -1; add ecx,0x40; ... ✓ (skin-name setter, T-062)
⚠ but [0x66E4D4+0x30] = 0x4AFC00, NOT 0x4b06a0; 0x4b06a0 sits at 0x66E5A4 (= slot 12 of table base
  0x66E574, and at base+8 of the ten §8a binding tables) — see F2.
0x436240: 8b 41 3c c3  → mov eax,[ecx+0x3C]; ret  (IWorldServer 0x66856C slot 0; +4 view ⇒ primary +0x40)
0x46f250: d9 41 70 c3   → fld [ecx+0x70]  (slot 1, NOT slot 0 — see D4)
0x46f260: 8b442404 8941 70 c20400 → mov [ecx+0x70],eax; ret 4 (slot 2 setter)
0x64D7D8 (G5): 00×8 then ASCII "GameCreature::ReadVersion2() - Making failsafe attack mode for
  entity: %" starting at 0x64D7E0 — the VA DOES resolve through the section map; see F3.
```

## 4. Itemized errata — docs/f3

Status vocabulary: **STILL-TRUE** / **SUPERSEDED-by-<row>** / **WRONG** (+ refuting EA) /
**ANNOTATE** (claim survives, needs qualifier or pointer). "Fix-shape": APPEND (new correction
section at file end, the board's state rule), ANNOTATE (inline note under the quote), REWRITE
(forbidden — listed only when the sentence is byte-false; still execute as append-annotate).

### docs/f3/SimDigest.md
* **S1 — WRONG** — §"What the digest found on its first run" (final paragraph):
  "So the original computes the same `uptime - 0` on its first `Update` and leaps the same distance.
  **Our behaviour is faithful — mirror it, do not 'fix' the stamp.**"
  Refuted by T-045: `JE_TickMilliseconds_0x56af50` = `a1 b8 c2 70 00 c3` reads `[0x70c2b8]`
  (session-elapsed accumulator, sole writer `F3::ProcessMessagesAndUpdateTime` 0x56afb8..0x56afd7,
  `WinMain` seed 0x56b8a0/0x56b8aa); `Update` is entered with `this = X+4` (0x552ba4 `lea esi,[eax+4]`)
  and subtracts primary `+0x4C`, which `0x5466bd` seeds. The fold is a port artifact.
  Fix-shape: correction block APPEND citing `docs/f3/WorldTickPump.md` §Conclusion.
* **S2 — WRONG** — same paragraph: "`ms` is absolute machine uptime, just like our
  `Host::TickMilliseconds()`" — bytes above (T-045). WorldTickPump §Consequences-2 already named this
  page as carrying the error and "flagged here, not edited"; still unedited.
* **S3 — SUPERSEDED-by-T-056/T-068** — §"PARITY=": "every artifact it writes today is labelled
  `# SYNTHETIC`". T-056 delivered the six values; T-068 rendered a real `lockstep` artifact
  (`tools/scripts/orig-capture-t056-runH.txt` @27d8e86; real-pair pin openje 8e0568b).
  ANNOTATE remainder: the full per-frame shipped digest still awaits R5/R6 (T-067).
* **S4 — ANNOTATE/SUPERSEDED-part-by-T-070** — §T-039: "a parity diff showing shipped `q>4`
  against our `q=4` is deviation D-2 *firing* — a dropped order". T-056: max observed 3, never >4
  pre-crash; T-070: shipped queue never empty at rest (idle resident) — D-2 re-expresses as
  **3 orders + resident**, and shipped `q` at rest is 1, not 0.

### docs/f3/ParityOracleDesign.md
* **P1 — WRONG** — §2 table `tickMs` row: "`JE_TickMilliseconds_0x56af50` = `timeGetTime()` |
  **do not compare** across processes — uptime differs". Bytes §3-1: it is a session-relative
  accumulated delta beginning near 0 ⇒ in the original `tickMs` is process-relative (comparable);
  the incomparability being designed around is ours (T-045 §Consequences-2 explicitly lists this
  file). Fix-shape: APPEND correction; the CONTEXT convention itself survives (T-068 kept it).
* **P2 — ANNOTATE (timeline)** — §2 `clock` row: "valid because the frame-0 leap is shipped (see
  `docs/f3/WorldTickPump.md`)". The claim was reversed by T-045 ("ours"), and T-056-R2 then
  confirmed a shipped 0→1s carry leap; the cited page currently states the OPPOSITE verdict from
  this sentence. Provenance must be banked in the doc text (§1 collision).
* **P3 — SUPERSEDED-by-T-039/T-048** — §2 `eN.st` row lists "`entityState`, moving `+0x3A2`,
  `+0x319`, `+0x314`". `+0x314` left the `st=` triple in T-039 (it is the deque LIVE COUNT, now
  printed as its own `q=` field); `+0x319` became live data with T-048; the raiser is shipped-live
  per T-069/item-45.
* **P4 — SUPERSEDED-by-T-056** — §3 heading "Lockstep reads still owed (address / value / when)"
  with the R1–R4 table: all delivered with values (R0 0x544A90 live-confirmed 3 boots; R1
  draw 0x2D81/state 0x65AD8170 pin rule; R2 0→1s; R3 count 0→1→3, never >4; R4 pre=0/post=entity,
  35/35; R4b GameActionIdle/ActionAnim, zero UNKNOWN_VT). Sentence "if R3 shows the index exceeding
  4, our cap is not a stylistic deviation but a dropped order" — still no >4 datum; depth reads
  re-based to T-067 (R5/R6 folded by T-068; D-2 re-expressed by T-070).

### docs/f3/OrigCapture.md
* **O1 — WRONG** — field table: "`JE_TickMilliseconds` = `timeGetTime`" (bytes §3-1; T-045).
* **O2 — SUPERSEDED-by-T-052/T-056/T-068** — §Boundary: "until T-034's R3/R0 land, every artifact
  this tool writes is SYNTHETIC and says so". T-034 closed-as-miss; T-052 fixed the startup dance;
  T-056 landed the values; T-068 ran the first real `lockstep↔unrecorded` pairing. Provenance rules
  themselves unchanged (item-47 born from T-068's near-miss).
* **O3 — ANNOTATE** — format example "`record=read read=r3 … breakpoint=0x459f5d`": in every real
  boot `0x459f5d` (script-native `F3::Attack`) **never fired** (T-056 deviation note; T-070's HOL
  explains why); the accepted capture used the funnel `0x550720/0x550741` scripted callers
  (`orig-capture-t056-runH.txt`). Example stays as fixture but needs the pointer so nobody treats
  it as the live recipe.
* **O4 — ANNOTATE (semantics moved under T-070)** — `q` row: "the D-2 number: shipped `q>4` beside
  our `q=4` **is** a dropped order, visible in the diff". The grammar still expresses it, but the
  accepted semantics now: shipped `q` includes the idle resident; "3+resident" is the capacity
  comparison; and the shipped side at rest prints `q=1`, never `q=0`.

### docs/f3/WorldTickPump.md
* **W1 — SUPERSEDED-by-T-052/T-056** — §"What still needs the debugger (the remaining T-031 reads)"
  (file end): the six reads are no longer debugger-bound — they were taken (T-056 table with
  values). Remaining depth = T-067 R5/R6/Rung-2 list. Same section's environment note "a foreign
  `winedbg.exe` is alive on port 31338, so it is not free to spend" — stale context (T-056 ran
  private prefix + fifo-winedbg :31440; ImHex contention named-solved in T-034 handover).
* **W2 — ANNOTATE (T-056 partial re-confirmation)** — §Conclusion: "the first shipped delta is one
  frame … there is **no 8-hour leap in the original**". True for the fold; but T-056 R2 accepted:
  "The frame-0 clock leap IS SHIPPED BEHAVIOR" for the 0→1s carry inside the first ~4 pumps (and
  19–21s boot-offset character at the R1 instant). Needs a one-line pointer so the page's
  headline ("We do [leap]") is not read as denying R2 — the §1 collision, second leg.

### docs/f3/GUILoadingRecon.md
* **Gr1 — ANNOTATE (provenance)** — §7 G1 resolution: "it is the `JE::Gfx_Engine` singleton pointer
  (class size 2212, primary vft `0x6ae7ec`, 120 slots per `artifacts/F3.json`)". vft identity
  raw-verified by this sweep (ctor `c706 ece76a00` + `a3 e47c7000` stores), but the SIZE/slot-count
  come from the frozen export whose SIZES are declared not-authoritative by T-040's erratum
  (the GUI::Fragment 3384-contradicts-0x158 precedent). Quote-size must carry the caveat.
* **Gr2 — SUPERSEDED-by-T-033b6/T-057/T-066** — §7 G1 end: "New sub-gap: what
  `byte_70B200`/`cls_0x55dc80` is (**G9**)". cls_0x55dc80 = `JE::Gfx_Texture_Manager` (b6 escalation
  CLOSED on cross-lane G2 evidence, class 17; T-057 head census names its map at +0x24; T-066
  ported it standalone with id-0 sentinel semantics). The same file's §9 tail likewise tracks the
  registry as T-013 *open* — T-013's RE tail is done and T-066 executed steps 1–3.
* **Gr3 — SUPERSEDED-by-T-013-static-tail** — §8 end: "That key form is **(inferred from the
  shipped index)** — the string actually handed to the acquire slot (+0xDC …) still needs one
  lockstep observation." Decided statically without a debugger: probe compares the VERBATIM wire
  name with case-sensitive memcmp (`0x41e4b0`, memcmp prologue raw-seen), inserts key on the
  in-place-stripped STEM (`sub_499F10` @0x499f84/89) ⇒ GUI-side probe is a STRUCTURAL MISS for
  dotted names; port rule "key by stem" (and per T-062: two key policies, must NOT unify — the
  cursor manager stricmps).
* **Gr4 — SUPERSEDED-by-T-013-static-tail** — §9 tail: "plus one lockstep observation to retire
  the *inferred* label on the texture key form (stem vs `name.tga`)" — retired by the same static
  evidence; nothing in the six-row list owes it (T-056/T-067 unchanged).
* **Gr5 — SUPERSEDED-by-T-013-static-tail (a)** — §8c: "**`+0x2C`** keeps no writer on either path
  and **stays uninterpreted**" (+ field table row "(never written — still uninterpreted)").
  It is the animated **frame index**: slot 15 writes it (`0x4b1760` `89 56 2c` — raw-verified by
  this sweep), slot 16 consumes it (frame × piece-width UV origin), slot 8 reload + slot 9 save
  participate; the wire value is the starting frame. (T-013 also refined slot 8 = re-loader,
  slot 9 = save-writer — worth the pointer.)
* **Gr6 — WRONG-as-worded** — §5 (G5 bullet): "assigns a literal at 0x64D7D8 into Manager+0x70
  (**that VA did not resolve to a clean string through the section map**)". Raw: 0x64D7D8 = eight
  0x00 bytes, then at 0x64D7E0 a clean `.rdata` string "GameCreature::ReadVersion2() - Making
  failsafe attack mode for entity: %…". The resolution claim was a tool artifact; the VALUE
  question stays open — see F3 for the gcre-doc tension.

### docs/f3/EntityActionQueueSeam.md
* **E1 — SUPERSEDED-by-T-041** — §T-039 block: "`Entity+0x1E4` … **What it means is still an open
  question and T-011 must not pick one**". Settled by the row (and documented in the same folder,
  `EntityFlag0x1E4.md`): map-membership flag `inMap0x1E4`; sole runtime writer `Entity::SetInMap_0x551ba0`
  store `0x551c4b`; predicate `0x5a95b0..` raw-verified §3. (Also the gate readers' neighbour: T-070
  found `SetInMap` is the idle feeder — drain-then-enqueue-idle — worth the pointer.)
* **E2 — SUPERSEDED-by-T-028/T-047** — "Our port currently reaches it from `GameActionMoveTo.cpp` …
  the *script natives* and the *server packet* doors are still absent, which is what T-028's driver
  must build on." T-028 shipped the driver (reaches a combat round through the seam, verify 9/9);
  T-047 shipped the packet-91 `attack-pos` verb verbatim (`8e272e6`).
* **E3 — ANNOTATE (superseded wording above the T-071 addendum)** — §PORT CHOICE: "shipped `q>4`
  beside our `q=4` is deviation D-2 firing (a dropped order), not a digest bug". The file's own
  §T-071 addendum already re-expresses capacity as "3 orders + resident"; the older sentence reads
  as live claim to a cold reader — annotate to defer to the addendum.
* **E4 — ANNOTATE (rename lag)** — "renamed `flag0x1E4`" — the port landed `inMap0x1E4` under T-041;
  doc keeps the intermediate name.

### docs/f3/CombatTurnDispatch.md
* **C1 — STILL-TRUE (verified)** — the two T-069-fixed passages ARE fixed in the file (raiser
  shipped-live wording at §"The port's action pump DOES raise +0x319…", re-entrancy reason at
  §walk-stability); the raiser/clear/test/consumer bytes independently re-read by this sweep
  (`0x54c833`/`0x549090`/`0x5a955d`/`0x5a9572` region, §3) — no drift.
* **C2 — ANNOTATE (row re-base)** — banner: "The observation outranks this banner: if a **T-034
  capture** ever shows a live nonzero `+4`/`+14` producer, THIS line is what gets rewritten."
  T-034 closed-as-miss; the capture actually ran (T-052 dance-fix → T-056 six rows) and T-069's gate
  run showed neither R2 nor R3 exercises the dead branch (manager `accMs_` stays 0 in free-run) —
  pins HOLD. Point the banner at T-056/T-067 instead of T-034.
* **C3 — ANNOTATE (T-070 content layer)** — the R3 sentence ("deque growing to 3 at funnel
  `0x550720`") is count-true, but T-070 downgraded T-068's R3 pairing to *count-agreement with
  state-disagreement* (shipped first-insert is the idle resident on 12/12 entities; ours is a real
  order) — same state-annotation rule the board applied to T-068's row.
* (+ minor: `+0x2E0` "separate message-queue named-deferred" is now an owned row — T-074/WildZenith;
  pointer only, claim unchanged.)

### docs/f3/ScriptNativeActionDoors.md
* **N1 — SUPERSEDED-by-T-043** — §Routed-questions-1: "Whether it [Entity+0x1DC] is a form-id, a
  load-serial, or a handle index is **not** settled by these sites." Settled: runtime-allocated
  reference handle (allocated `0x55190c`, serialized by PackState `0x5459c7`, matched against the
  raw AMX cell at `0x54be10` `0x54be33`); `netId` is not a substitute — D-3 stands, now precisely
  describable (`EntityRefKey0x1DC.md`).
* **N2 — SUPERSEDED-by-T-044** — §"Doors that are NOT this row": "Explicit-position override —
  `GameActionAttack::meth_0x401100` … **Callers unknown**" and Routed-questions-2. One caller,
  identified: server packet ID 91 path, store `0x46df14`, guard `attacker && (hasPos || target)`
  `0x46decb` (`AttackPositionOverrideDoors.md`).
* **N3 — SUPERSEDED-by-T-056** — §"+0x14 — the answer from statics": "…that is exactly NiceHawk's
  T-034/R4 question… live (R4): is the pre-call value zero or garbage?" Answered: **pre =
  0x00000000 on every one of 12 entities, post = entity pointer, 35/35 stores** on the script-path
  funnel (T-056 R4) — and T-070 explains the script path's no-drain vs packet-pump drain shapes.
* **N4 — ANNOTATE** — §"Doors that are NOT this row": "Server packet — `GameServer::virt_meth_0x46dd70`,
  4 sites into `0x550720`. **Unexamined here**; may fill `+0x38`/`+0x20`/`+0x30` differently".
  Case 91 is now examined (T-044); the other sites stay open; T-070 additionally pins the packet
  command shape as drain-then-enqueue (cases 0x1A/0x3C) vs native no-drain.
* **N5 — STILL-TRUE (honest open)** — Routed-questions-3 (meth_0x4012e0 caller-set enumeration) and
  question-4 (`ctor_0x403e40` stack-residue byte into `record+0xBC`) are untouched by any closed
  row; no correction owed. Also D-2 paragraph "shipped *expects* >4 live actions" — still true as
  container capacity; re-express with T-070's 3+resident when read against gameplay depth.

### docs/f3/StartupAndSubsystemTable.md
* **ST1 — ANNOTATE (label currency)** — "## Command registry (`JE::cls_0x4d8d70` @ 0x70C668)" and
  the reconstruction/test paths under `cls_0x4d8d70`: the class was minted **`JE::StringTree`**
  (T-033-b3 CONFIRMED via the `_Tree` family proof; T-059's amendment explicitly left the mint
  standing). Also attach item-43's standing caution where this page's class name is quoted in pad
  contexts: the 2,062-pad "owner" reading of cls_0x4d8d70 was retracted — 1,958 target one function,
  `0x4B2DD0` (app-wide std::string SSO dtor, itself wearing a stale `JE::StringTree::meth_*` live
  name pending the split-comment queue).
* **ST2 — ANNOTATE (refuted aggregate)** — "## Strings on the wire (`cls_0x50db20` tempString shape)":
  batch-1 **REFUTED** cls_0x50db20→JE::String (its member table embeds `.?AVG3D_Water@@` 180B +
  `.?AVVFX_Parameter@@`; T-057 reconfirmed both single-embedders land on it). The MSVC4 string
  LAYOUT claims are independently evidenced (0x401bd0 assign shape, SSO cap 15, free-at ≥0x10) and
  survive — but not as a *class* attribution.
* **ST3 — SUPERSEDED-by-T-038/T-042/T-045** — cascade item 4: "`[World] Year/Month` read from the
  tree -> game calendar (`JE::GameTime`-ish, deferred)": it is `JE::Gregorian` (0x38) at World+0x3C,
  read by `0x546b50` into `+0x14..+0x28/+0x40` (first statement of Initialize `0x5466b0`),
  seeded by a 24-byte block copy `0x54497d`, fallbacks and `?: 2253` invention resolved under
  T-042; ported and tested (StringSystem suite).

### docs/f3/GUIStringTags.md
* **GS1 — ANNOTATE (label currency)** — §2 P2: "`cls_0x4d8d70` is the RB-tree registry class" →
  live name `JE::StringTree` (ST1 same family); the "registry instance, not a hash" conclusion
  unaffected.
* **GS2 — STILL-OPEN (honest)** — the `[Globals]` key-vs-section residual ("needs one lockstep run
  to confirm") is not covered by any closed row — it is also **not** on T-067's L1–L6/R5/R6 list;
  route it as a bonus read. §3 "blocked on a handshake" is likewise done: `StringFormat.{hpp,cpp}`
  exists in `modules/openje/src/JE/SS/` (verified this sweep) — annotate SUPERSEDED-by-T-006.
* **GS3 — CROSS-REF** — "Cross-reference for A: `meth_0x5ace40` is the same registry lookup
  `docs/DEFERRED-NOTES.md` lists as 'callers not yet RE'd'" — the DEFERRED line is itself stale
  (see D5); this page found a caller (`0x56c117`) and the console registry side was mapped
  (2025-10-20-death-console doc).

### docs/f3/GamePathManager.md
* **GP1 — SUPERSEDED-by-T-033b6** — §Layout: "derived: `0x6602e4`, base: `0x696388`" with
  F3.json-derived members — the b6 escalation wave CLOSED `0x55dc80`→`JE::Gfx_Texture_Manager`,
  which proves `cls_0x55dc80` is NOT a tree clone in the PathManager region; conversely the PathManager
  band's `_Tree` clones were named through b3/b6 (map-members). Direct check owed: this doc's member
  table cites `F3.json` for `mbr_0x4/8/c` — sizes/clone attribution must carry the T-040 caveat and
  the b6 "11 manager fns" boundary (which manager fns are Gfx_Texture_Manager, not PathManager).
  No gameplay claim on this page was refuted; ANNOTATE only.

### docs/f3/WorldYearSeedConsumer.md
* **WY1 — SUPERSEDED-by-T-045/T-056** — final section: "Disposition: **both readings can be true;
  table identity stays unresolved**" and "**`+0x18` is slot 6 — a hole under every reading**, so
  T-034's R0 still needs the live read to name the tick target." Table identity WAS later resolved
  (T-045: `0x5466b0` = `call [eax+14h]` on the PRIMARY table, whose slot 5 is `0x546b50`; the pump
  enters through the `+4` mixin `0x6919e4/0x66856C`, whose slot 6 is `0x544a90` — raw re-verified
  by this sweep §3) and R0 was LIVE-CONFIRMED = `0x544A90` on three boots (T-056). The row's
  "one debugger read" ask is retired — supersede in place with pointer.
* **WY2 — SUPERSEDED-by-T-042** — port-comparison table row: "`new JE::Gregorian(m_calendarYear ?: 2253,
  …)` | **seeded, but with different fallbacks** — see the bug below". The bug LANDED fixed
  (ctor→`0,1,1` first, ternaries deleted, absent-key = ctor defaults, authored-Year=0 reachable;
  StringSystem 20→27 asserts; gate 10/0).
* (⚠ item 4 — the `param_1 == &World+0x14` shape inference — stays honestly open; T-056 did not
  read World+0x14 beside Greg+0x1C. No correction owed.)

### docs/f3/T013-registry-port-design.md
* **T1 — SUPERSEDED-by-T-066** — header "**Status:** design-only (no code, no `src/` commits)":
  steps 1–3 EXECUTED (openje 78bf743/92bceb4/c16ddcb/653fa79; parents 20d8699/d5a70ce/f6cb8a7),
  gates green each; steps 4–5 remain M-001-gated by design. Update the status line.
* **T2 — STILL-OPEN (with currency note)** — honest-gap 1 "the owning singleton's class is F3-side,
  likely Gfx_Engine itself; until a live boot shows it": no closed row names the runtime writer of
  `0x70B200` (T-056's boots did not target it). Candidate evidence to fold when routed: T-054
  established the *analogous* pattern for `byte_707CE0` = ctor self-cache (not an object) — same
  caution applies before reading `0x70B200` as anything but a stored pointer.
* **T3 — SUPERSEDED-by-T-062** — honest-gap 2: "**Corner-field-0 index semantics** … (render-side,
  T-056 BONUS list)": answered same day as the file's own date — slot 12 `0x4b1610` writes a
  **float u0** into corner field-0 of all four vertex blocks (setter/getter pair 0x4b1610/0x4b1640;
  bodies raw-verified this sweep); frame-INTERPOLATION (mode-1/2) still rides T-067 BONUS-3.
* **T4 — ANNOTATE** — "gates … `run-all-tests` **9/9**" (twice): the gate is 10 suites since the
  T-042/T-046 era; and Step-2's "id ≥ 0" plus T-066's discovered boundary: **id 0 is the reserved
  absent-sentinel (free-id pool returns 0 empty; consumers test id!=0 at 0x4b1b12/0x5236da)** — the
  design predated the grounding catch and its Step-5 numbers were re-asserted under it.

### docs/f3/AttackPositionOverrideDoors.md
* **A1 — SUPERSEDED (follow-on closed)** — §Follow-on: "Row to be added by ZenNova (numbering
  owned there)" — added and DONE as T-047 (`attack-pos` verb shipped exe-level, `8e272e6`,
  walk-then-WHIFF, guard cited `0x46decb`). Verdict text unaffected.

### docs/f3/GITMItemRecordLayout.md
* **GI1 — WRONG (citation)** — line 5: "the B8 pair table `sub_469430` (req-class → phys-slot), which
  **T-041's** requirement-class reading and the `dtRowB` naming both rest on". T-041 is the
  Entity+0x1E4 map-membership row — it contains no requirement-class reading. The B8/req-class
  reading is `discoveries/2025-10-20-t010c-conflict-resolved-equip-slot-model.md` (+ the T-018
  helmet closure). Cite-fix; the byte claims (`sub_469430` jump table @0x46946C, req1→B) are fine.

### Interface stub family — errata class I1 (8 files)
`CGameInterface.md`, `COptionsGameInterface.md`, `CPipBoyInterface.md`, `CPipBoyAddNoteInterface.md`,
`CGameFloatingTextInterface.md`, `GameDialogueInterface.md`, `GameCSMenuInterface.md`,
`GameCSSkillMenuInterface.md` — each carries a mermaid block headed "## Inheritance" containing
`JE::GUI::Interface --> JE::CEngineInterface` and `JE::CEngineInterface --> F3::X`.
* **I1 — SUPERSEDED-as-shipped-structure by T-057** — the window/host classes' relation to
  CEngineInterface is **COMPOSITION, proven**: `CEngineInterface::ctor_0x580330` constructs it
  INTO hosts at `[esi+DISP]` (33 embed-ctor sites across ≥27 named hosts), and vft `0x69A95C`
  carries exactly ONE RTTI COL (`.rdata 0x6D6510`, primary-offset 0 — raw-checked this sweep)
  shared by all users, which a non-primary base could not have ("base-subobject = CLOSED
  falsification"). T-064 [B] then falsified a second base-reading for the -0xB0/-0xC4 bands
  (multi-host SHARING, item-44). T-061 independently pinned the class's role (cursor/event FACADE,
  3 overrides + 3 `ret`-defaults, vft body §3). The stub graphs may be honest *port-model* labels,
  but as docs/f3 pages they silently assert a shipped inheritance the corpus has closed — fix-shape:
  ANNOTATE each ("embeds at +DISP per T-057; port models as base") or relabel the section
  "Port hierarchy" where the port really derives.

### Stubs with no EA claims (47 files incl. GameAction*, Game* classes, Display.md, GameAudio.md,
GameInterfaces.md, GameHotKeys.md, GameWorld.md, GameClient.md, GameServer.md, GameMap.md etc.)
* **STUB — CLEAN** — navigation skeleton (source links + openje doc refs + one graph edge); no
  address-level claim to falsify. Two carry forward-looking relevance: `GameCursorManager.md`
  (T-062 identified the class — RTTI `.?AUGameCursorManager@@`, 0xB0, vft `0x6550B4`, slot5
  `0x437ab0` raw-verified §3) and `GameActionIdle.md` (consistent with the landed port files while
  T-071 runs; add the factory-kind pointer `0x46f290` case 1 when it checkpoints).

## 5. Itemized errata — docs/DEFERRED-NOTES.md

* **D1 — WRONG / SUPERSEDED by T-045 (the whole "RESOLVED by statics … it is SHIPPED behaviour,
  mirror it" section)** — quotes and their refutations:
  - "`ms` is **absolute machine uptime**, exactly what our `Host::TickMilliseconds()` supplies"
    — bytes `0x56af50 = a1 b8 c2 70 00 c3` (§3): session-elapsed accumulator, WinMain-seeded.
  - "the original's `JE::World::ctor_0x548a80` zeroes the contiguous dword range that covers
    `+0x44`/`0x48` … So the shipped engine has the same zero-initialised stamp" — the field
    `Update` subtracts is primary `+0x4C` (this = X+4, store `0x5466bd`), SEEDED; the mixin
    displacement was read as a primary offset (called out verbatim by WorldTickPump).
  - "Our behaviour is faithful here, and the mirror rule binds: do NOT 'initialise the stamp to the
    first tick'" — INVERTED: seeding the stamp is exactly what shipped does and what the port must
    mirror.
  - "`clock` IS a valid comparison field, because both sides start from the same leap" — replaced
    by T-045 (comparable only after the port seeds) and re-qualified by T-056 R2 (the small
    0→1s carry IS shipped; the ~8h fold is not; T-068: ported+correct in app path, harness-unobservable).
  - "Open remainder — why is only `+0x48` left unseeded when `+0x4C`/`+0x54` get the tick" —
    dissolved by the same base correction (`+0x48` primary = accumulator, zeroed by design at
    `0x5466b5`).
  Fix-shape: APPEND a T-045/T-056 correction block over the section (annotate, never rewrite).
* **D2 — WRONG (table identity)** — "VFT TOPOLOGY CORRECTION: `0x691A20` is the World
  *secondary/mixin* lifecycle vft with its own numbering: `[0]Initialize_546690` …": raw bytes show
  the ctor installs `0x691a1c` (primary), `0x6919e4` (+4), `0x6919c0` (+C) (§3) — `0x691A20` is
  *inside* the primary table (primary+4), a phantom-shifted view. The entries themselves are real
  (primary slots 1..); the numbering must be restated against T-045's four-table map.
* **D3 — WRONG (slot-5 no-op)** — "vft[5] at Initialize start (`0x5466B0`) is a no-op in engine
  (`0x404b70`)": `0x5466b0` = `call [eax+14h]`; primary slot 5 = `0x546b50` — the calendar seeder
  (raw `[0x691a1c+0x14]=0x546b50`); the retn stub `0x404b70` sits at primary slots **3 and 6** (raw
  §3). Resolved as a numbering artifact by WorldTickPump §Consequences-4; DEFERRED never updated.
* **D4 — SUPERSEDED (and a slot mis-number)** — "World clock consumers" bullet:
  "`World+0x70` = double worldSimSeconds (ctor seed 1.0; **IWorldServer slot0 getter `0x46F250`**, slot2
  setter `0x46F260`)" — `0x46F250` is table `0x66856C` **slot 1** (raw dump §3: slot0 = `0x436240`).
  "Combat manager … `acc_ms += 1000*worldSimSeconds`" — the combat operand is `1000*[GW+0x40]`
  = `1000 × slot0` (per T-045: `v9 = 1000*(world server read) + mbr_0x10`), and C-23's answer
  (`2025-10-20-c23-answer-worldsim-seconds-not-pointer.md`) established slot0 returns the plain
  scalar at GameWorld+0x40 (`[this+0x3C]` on the +4 view), with GW+0x70 a DIFFERENT sibling.
  WorldTickPump left the operand question "already T-011's" — the current port is
  `accMs_ += 1000*secondsPerRound` (T-051 pin, T-069 re-confirmed). "CombatManager port NOT started
  (queue item)" — WRONG: landed under T-011 (`20827b2`); cited lines `CombatManager.cpp:180/188`
  exist. "initiative turn queue" — append T-010-10e deadness (free-run is the only live branch).
* **D5 — ANNOTATE (partial close)** — "Console command execution path (`Meth_FindValue_0x5ace40`
  callers, message loop dispatch of typed commands) not yet RE'd" — registry side mapped
  (2025-10-20-death-console-only doc); `Format_0x56be30` @0x56c117 is a caller (GUIStringTags);
  typed-command dispatch remains genuinely open — name it so the half-closure stops traveling as
  a full unknown.
* **D6 — SUPERSEDED** — "Strings (SS): … `EngineStringSystem::Format_0x56be30` + Skin `LoadDefault`
  decompile FAILED -> needs disasm pass": both got the disasm pass (GUIStringTags §0; GUILoadingRecon
  §1). The "WIRING PENDING: who calls ESS::Init/ctor" item is NOT covered by a closed row — stays open.
* **D7 — SUPERSEDED** — "Session pump belongs to the **unmapped session layer**" (Pumps and
  schedulers): session14 + T-045 + T-056 R0 map it end-to-end: `JE::Server::virt_meth_0x552b60`
  → `+4`-view slot 6 → `World::Update_0x544a90` (live-confirmed target). "subsystem Ticks all empty
  stub `0x404B70` except `JE::Network::Tick_0x543770`" — raw-confirmed by the slot-3/6 identity (§3);
  keep, just attach the WorldTickPump table map.
* **D8 — SUPERSEDED-by-C-23** — "Sim-seconds thread: closed with corrections": "The combat time
  source (IWorldServer slot 0, `0x436240`) = Get_mbr_0x3c -> the GREGORIAN CLOCK POINTER … reads the
  clock OBJECT address as int?? — IDA artifact … REQUIRES runtime/lockstep confirmation … FLAGGED to
  discovery (C-23)." C-23 was ANSWERED: slot0 is a plain scalar getter of GameWorld+0x40 (the view
  rebase: `[this+0x3C]` on `this=GW+4`), and "×1000/÷1000 ms math at 0x5a95aa/ce confirms units";
  the clock object lives at GW+0x3C behind a DIFFERENT slot. The whole "needs confirmation" posture
  is stale. (Also this section's `0x436240`/`0x46F250` slot confusion is the D4 mis-number's source.)
* **D9 — SUPERSEDED-by-session25** — "Entity lifecycle state machine adopted … DYING/REMOVED are
  no-ops pending OnDeath (**server-only** 6->7 flip)": the mode flag is set by the CLIENT CONNECT
  handshake (`2025-10-20-session25-mode-flag-connected-client-retraction.md`) ⇒ the demo runs the
  full server pipeline in-process; EFDT/Apply/85/88/JoinCombat/6→7 all LIVE. The "server-only"
  framing was retracted corpus-wide; this section never got the note.
* **D10 — ANNOTATE** — "`ProcessMessagesAndUpdateTime` (0x56AF60): wRemoveMsg param modeled as
  uninitialized local in the original too; **confirm PM_REMOVE semantics during lockstep**": the
  lockstep era has now run (T-052/T-056) and did not read this; either retire the ask or move it to
  T-067's list as an explicit candidate — currently it dangles as if a cheap read were still owed.
* **D11 — SUPERSEDED (harness)** — "`mcp__ida_original_f3__set_comments` is broken in this pi harness
  (arrays arrive stringified); persist comments via renames + docs": harness **item 40** localized it
  (tool-specific coercion, direct JSON-RPC accepts the identical payload) and T-058/T-060 stamped
  98 comments through re-issue with readbacks — the workaround rule is now: file the harness bug,
  do not treat it as a target constraint. Line as written would send a new agent to bank comments
  in docs for no reason.
* **D12 — ANNOTATE (gate staleness)** — "Verification status at time of writing — tests: PathManager OK,
  cls_0x4d8d70 registry OK (all under wine)": stamp-less gate quote (item 19/30 family); the current
  accepted gate is run-all-tests **10/0** + gate-integrity stamp `483599247`/21:32:26Z (T-069) on
  payload 17:32:19. Also "the generic loop's Tick slot is EMPTY (`0x404B70` retn)" — keep, raw
  verified §3, but renumber per D2/D3's table identity.
* **D13 — ANNOTATE (numbering + D-2 re-expression)** — D-2 block: "a fifth concurrent action is
  **refused** (return 0)" is still the port's behavior (d2-stress exit 1), but per T-070 the
  shipped-side comparison must carry the resident: capacity reads as **3 orders + resident** and
  the shipped queue at rest is never `q=0`; D-2's ">4" question re-expresses (T-071 step 4 owns the
  digest upgrade). Same for D-1 item 2 (`SeedShippedRand`): T-068's capture-and-replay pin
  (state 0x65AD8170 → unique preimage 0x6AD92B15, CombatMath 46≥43) is the new live precedent that
  pinning is measurement, not invention — cite it next to the deviation.

## 6. Cross-document contradictions and flagged questions (NOT errata verdicts — route to coordinator)

* **F1 — Gfx_Engine slot identity collision.** GUILoadingRecon §7 G1: "`+0xD8`(216)=`0x5c0930`,
  `+0xDC`(220)=`0x5c0970`, `+0xC4`(196)=`0x5c0a90`"; T-062 §GameCursorManager: "beam(16,16)/
  default(2,3)… SetCursor via GfxCursor ctor `0x4c8100` **vft `0x67140c`**", and T-062 item-1
  calls the cursor object's own loader vft `0x67140C`→CSound COL `0x6D4098`. These agree.
  BUT the brief's own yardstick line "0x707CE4/0x707CE8 singletons" plus T-054's
  "cls_0x507810 … Gfx_Engine member **+0x30** … texmgr +0x2C" vs GUILoadingRecon §8e's
  `[[obj+0x24]+0x1C/20]` reads: no contradiction found at byte level (different objects: the
  +0x30 trio hangs off the engine; +0x24 is a texture-object field), **but** the slot-numbering
  style ("+0xD8(216)") is table-relative and should state WHICH of the four Gfx_Engine-adjacent
  vftables (0x6ae7ec primary, and the +C manager view) each page indexes — T-061's parent-vft
  precedent (GUI::Interface 0x669D80 vs CEngineInterface 0x69A95C) shows the confusion is real.
  Action: one disambiguation appendix in GUILoadingRecon naming the per-slot table base.
* **F2 — T-062 Window slot-12 coordinates fail a raw re-read.** T-062 §107-114: "`GUI::Window` vft
  `0x66E4D4` slot 12 (+0x30) = **0x4b06a0**". Raw this sweep: `[0x66E4D4+0x30] = 0x4AFC00`;
  `0x4b06a0` instead lives at `0x66E5A4` = slot 12 of base **0x66E574** (and at base+8 of ten
  tables §8a lists: 0x655824, 0x65d224, 0x66ca84, 0x66d504, 0x66e59c, 0x670d74, 0x697f14, 0x69dd8c,
  0x69ec94, 0x69ef6c). The FUNCTION body claims both check out byte-exactly (`0x4b06a0`:
  `push -1; add ecx,0x40; … call 0x401b20` = skin-name setter; `0x4b1610`: `lea eax,[ecx+0xC0];
  mov ecx,4;…` = four-block float writer), so the heterogeneity verdict survives; only the cited
  table coordinate mis-anchors. Flagged because item 43 makes writers' authority a head/coordinate
  discipline — accepted-doc coordinates should re-derive from the table they name.
* **F3 — G5 vs the gcre doc's `byte_64D7D8` reading.** Raw (§3): 0x64D7D8 = `00×8` then
  "GameCreature::ReadVersion2() - Making failsafe attack mode…" at 0x64D7E0. GUILoadingRecon G5
  calls it "a literal assigned into Manager+0x70 (not a clean string)"; the gcre doc
  (2025-10-20-session7-era) calls the same EA "the 'default/unarmed' mode-name token". Both cannot
  describe the same bytes; likely one of the two EAs is off by 8 (or two distinct literals in one
  pool). Not today's closure — route as a 15-minute row (Manager+0x70 writer `0x4b4920` disasm).
* **F4 — `World+0x104` double-booked.** StartupAndSubsystemTable cascade: slot[12] CreateGameWorldMap
  → **world+0x104**; DEFERRED: cls_0x5a42b0 (map-request record) → "GameWorld slot[13] … stored
  **world+0x104**"; cutscene roster (Oct-20, accepted): GameWorldMap → **+0x128?** (with "?").
  Three accepted statements, two offsets, one field. Outside today's closed-row set — flag for an
  owner-attribution pass (ecx-flow tool already built).
* **F5 — `Meth_RbInsertAtBottom_0x59f200` rotation EAs.** DEFERRED (command-registry section) cites
  "rotations `0x583930/0x4C8E50`"; the same section labels the tree `cls_0x4d8d70`, whose mint b3
  proved via `_Tree` templates. The T-059 amendment retired any *class-level* reading of pads
  around this family; the rotation EAs themselves were not re-verified by this sweep (deep .text
  work). Marked "unverified-in-sweep" so nobody treats ST1's label-currency as byte-audit.
* **F6 — `gitm.csv` schema note (carried, untouched).** GITM doc: "`equip`/`eqslot` columns are
  req-count/reqvec[0] under worse names … Conclusions survive; CSV re-run must rename". No row
  re-ran the census since — the correction remains owed to T-025's artifact, not to the docs.

## 7. Claims checked and found CURRENT (STILL-TRUE) — spotlight sample

(Selection, not exhaustive — the full 76-doc sweep is the coverage table.)

* `CombatTurnDispatch.md` T-069 passages — byte re-verified (`0x54c833` store, `0x549090` clear,
  `0x5a955d` test, `0x5a95b0` predicate; §3). The two historically-stale paragraphs are fixed
  correctly per the ledger (`t069` lines 39/56 confirm the edits landed where this sweep reads them).
* `EntityFlag0x1E4.md`, `EntityRefKey0x1DC.md`, `AttackPositionOverrideDoors.md` (verdict bodies),
  `GITMItemRecordLayout.md` (byte claims), `GamePathManager.md` algorithm tables, `Display.md` and
  the 47-stub skeleton: consistent with the accepted corpus.
* `CombatRngSeam.md` §1 mechanism table (seed lazily `time32`, reseed none, emit `(state>>8)&0x7FFF`) —
  strengthened: T-056 R1 live (gate 0x01, boot-varying state) + T-068 pinned-state replay rule;
  `sub_48CD30` purpose honestly still unnamed (zero new evidence found by grep across the corpus).
* `GUILoadingRecon.md` §6b skin-chain, §8b nine-slice table (`0x6faf2c` nine string-pointer cells raw-read
  §3), §8d per-slot writes, §9a exactly-4-colour rule, coordinator appendix (0x707CE8 slot-manager,
  0x707DA8 single ResourceSystem-session writer) — all current.
* `WorldTickPump.md` core bytes (0x56af50, the four vft stores, `0x544a90` prologue `8b 54 24 08`
  + `mov [this+0x48]` subtraction visible at `544a90: 56 8b f1 83 7e 34 04 0f 84 … 8b 54 24 08 8b 4e 48`
  — the `8b 4e 48` is the `this+0x48` load §3) — verified.
* DEFERRED ledger items 1/2/4/5/6 + "Also-standing 0x6FF650" + D-3/D-4 + ledger 7 — no drift found
  against the accepted corpus (T-019 closure and the session36 taxonomy sweeps already reconciled them).

## 8. Coverage statement

* **Docs read: 76 / 76** (all `docs/f3/*.md` + `docs/DEFERRED-NOTES.md`); 20 substantive docs read
  end-to-end, 48 stubs read for claim presence (zero EA-level claims, §4 stubs), 8 interface stubs
  audited for the I1 class.
* **Discrete claims adjudicated: 56** (44 doc-passage hits + 12 raw-byte/line cross-checks feeding
  them), of which **WRONG: 9** (S1 S2 P1 O1 Gr6 D1-family counted as D1/D2/D3/D4/D8 + GI1),
  **SUPERSEDED: 17** (S3 S4→annotated-supersede Gr2 Gr3 Gr4 Gr5 E1 E2 N1 N2 N3 WY1 WY2 T1 T3 A1 D5* D6 D7 D9 D11
  counted conservatively), **STILL-TRUE-with-ANNOTATE: 12**, **CLEAN stub/no-claim: 48 files**.
* **Raw-byte self-verifications: 27 EAs** (§3), including 3 that found no fault (Fragment vft,
  TurnPrep clear, IWorldServer slot2) and 2 catches against ACCEPTED docs (F2 slot coordinate, F3 G5
  wording).
* Zero docs edited; zero IDB touched; only this file + the task-file claim commit exist from this row.

## 9. What this sweep recommends at checkpoint time (mechanical, for the coordinator)

1. One **correction block per hit-file** (APPEND at file end, keep the stale sentence visible per
   the board's state rule) — the 19 ERRATA files in §2, §4/§5 texts ready to paste.
2. DEFERRED-NOTES: D1–D4/D7–D9 are one "clock/vft reconciliation" appendix; D5/D6/D10/D12 are
   per-section annotations; D11 is a one-line pointer to harness item 40.
3. The interface-stub graphs (I1): a single shared appendix line per file beats a rewrite; if the
   mermaid heading is the problem, relabel to "Port hierarchy (models shipped EMBEDS — T-057)".
4. Route F2/T-062 slot-coordinate catch and F3/G5 to their authors (KeenStorm / GUI lane) as
   verify-not-trust notes; route F4 as a small IDB row.
5. [DISCOVERY-BATCH ready] after this file is checkpointed — T-072 stays in-progress until the
   coordinator commits it.

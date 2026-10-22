---
status: in-progress
assignee: JadeBear `01a1639b`-peer (pulled 2026-10-22 ~10:4x from the coordinator-maintained T-079 row; seat's coordinator was dark at claim — relay sent, no contest; predecessor applies: T-078 dispositions + item 48 landed by coordinator at fb3d03f)
type: docs-apply ONLY (zero IDB writes, zero src/, zero prefix boot; annotate-never-rewrite law)
depends: T-072 deliverable (the per-file errata LIST — discoveries/2025-10-21-t072-doc-errata-sweep.md), T-081 §5 re-key table (paste-ready), T-082 §6 corpus rule, T-057/T-064 composition proof for I1
---

# T-079 — BATCH CORRECTION APPLY (docs-errata sweep's mechanical pass, finished)

## Scope (README row's remaining "1.5 of ~6")
1. **Frame-0-leap three-way reconciliation appendix** — SimDigest.md / OrigCapture.md /
   ParityOracleDesign.md (+ WorldTickPump wording if the appendix needs the other magnitude;
   T-056 R2 0->1s carry). One reconciliation block, same text, cited to T-045/T-056 bytes
   (`0x56af50 = a1 b8 c2 70 00 c3` session-elapsed; `+0x4C` seeded store `0x5466bd`).
2. **I1 interface-stub relabels (8 files)** — CGameInterface, COptionsGameInterface,
   CPipBoyInterface, CPipBoyAddNoteInterface, CGameFloatingTextInterface, GameDialogueInterface,
   GameCSMenuInterface, GameCSSkillMenuInterface: annotate the "## Inheritance" mermaid blocks
   per T-072 §9.3 (port-model label over COMPOSITION proof — embeds at +DISP, T-057 33 sites,
   single COL 0x6D6510 offset-0).
3. **DEFERRED-NOTES.md** — D1–D4 + D7–D9 clock/vft reconciliation appendix; D5/D6/D10/D12/D13
   per-section annotations; D11 one-line pointer to harness item 40 (the old "persist in docs"
   rule is retired — file the harness bug instead). P-CMBT's 2 doc items fold here per PARK.
4. **Per-file §4 correction blocks** — GUILoadingRecon, EntityActionQueueSeam, CombatTurnDispatch,
   ScriptNativeActionDoors, StartupAndSubsystemTable, GUIStringTags, GamePathManager,
   WorldYearSeedConsumer, T013-registry-port-design, AttackPositionOverrideDoors,
   GITMItemRecordLayout — each APPENDs its §4/§5 block at file end, stale sentence stays visible.
5. **Clean-pending DONE marks + dead-pending flips** (T-041/T-043/T-044 lines etc. per §2 table).

## Laws honored
- **Verify-not-trust at APPLY time**: every byte claim being pasted gets re-checked against
  `$OPENVB_ORIG_EXE` (PE-parsed image base 0x400000) or the frozen raw dumps in the sweep §3
  before it lands — SwiftMoon's list is evidence-backed, not waived (tasks/README §5). Catches
  during apply get named in the ledger, including against the sweep.
- **Corpus rule (T-082 §6)**: cite RAW table dwords, never F3.json export slot-maps.
- **annotate, never rewrite** (board state rule); correction blocks carry the refuting EAs.
- **Slot laws**: no IDB writes (read-only image/offline tools only), no src/, no prefix — zero
  collision with WildKnight's runtime row + R-USERRE-B.
- README row flip is coordinator-only: RELAY the progress/final state; task file here is the
  working ledger.

## Ledger
- [x] claim filed (this file), reserves taken, batch order set
- [x] **batch 1 LANDED (2026-10-22)** — SimDigest S1–S4, ParityOracleDesign P1–P4, OrigCapture O1–O4,
  WorldTickPump W1–W2, each as an appended correction block (stale text left visible, annotate-never-
  rewrite honored). APPLY-TIME VERIFICATION (own PE-parsed reads of `$OPENVB_ORIG_EXE`, image base
  0x400000 — NOT copies of the sweep's §3): `0x56af50 = a1 b8 c2 70 00 c3` MATCH; seed block
  `5466b5 mov[esi+48h],ebx / 5466b8 call 0x56af50 / 5466bd mov[esi+4Ch],eax / 5466c3+5466c8 -> +0x54`;
  `0x552ba4 = 8d 70 04 eb 02 33 f6 8b 1e e8 9e 83 01 00 50 8b ce ff 53 18` (`lea esi,[eax+4]`, `call
  [ebx+18h]` = +4-view slot 6); `[0x6919e4+0x18]=0x544a90`, `[0x668584]=0x544a90`,
  `[0x691a1c+0x18]=0x404b70`, `[0x691a1c+0x14]=0x546b50`, `[0x691a1c+0x0c]=0x404b70`;
  `[0x66856C+0x00]=0x436240`, `+0x04=0x46f250`, `+0x08=0x46f260`; `[0x66E4D4+0x30]=0x4AFC00`,
  `[0x66E574+0x30]=0x4B06A0`; `0x436240=8b 41 3c c3`, `0x46f250=d9 41 70 c3`,
  `0x46f260=8b442404 8941 70 c20400`; `0x64D7D8`=8x00 then ASCII at `0x64D7E0` ("GameCreature::
  ReadVersion2() - Making failsafe attack mode f..."); ctor `0x548abb = c706 1c1a6900 c74604 e4196900
  c7460c c0196900`; raiser `0x54c833 = c6 85 19 03 00 00 01`; consumer `0x5a955d = 8a 87 19 03 00 00
  84 c0`. EVERY sweep §3 claim batch 1 pasted stands. ONE catch against the sweep's COVERAGE (logged
  in the WTP block, not rewritten): `WorldTickPump.md:205` inline comment labels `World+0x48` "the
  stamp Update subtracts" — contradicts the page's own Conclusion (`Update` subtracts `X+0x4C` via
  `this=X+4`; `X+0x48` is the accumulator zeroed there). T-072's W-list missed it (body declared
  "otherwise current"); apply-time reverify caught it. CRLF/absolute-path machinery: n/a here (docs only).
- [x] **batch 2 LANDED** — I1 ×8 interface-stub shared annotation line appended under each
  "Inheritance" mermaid (T-072 §9.3 "shared line beats rewrite"). APPLY RE-DERIVATION: raw E8
  rel32 scan of .text over `$OPENVB_ORIG_EXE` found EXACTLY 33 direct call sites of
  `JE::CEngineInterface::ctor_0x580330` (T-057's embedding count, independently reproduced);
  `[0x69A95C-4] = 0x6D6510` with COL primaryOffset=0 (one-COL non-base proof, both dwords
  raw-read); slot1=0x4953c0, slots3/4=0x404b70 (T-061 facade). CGameFloatingTextInterface's extra
  F3-side derive edges untouched (out of I1's scope). Glob-miss caught at commit time:
  COptionsGameInterface had silently dropped from the check pattern — 8/8 confirmed written
  before the commit, none after.
- [x] **batch 3 LANDED (this commit)** — DEFERRED-NOTES remaining D-items (D5 half-close, D6,
  D7 half-open split, D8, D10 retired-with-reason, D12, D13) + the coordinator-block bullet
  re-map (its third/fourth/fifth/sixth bullets = sweep D3/D4-tail/D11/D9 — a numbering drift
  caught in MY OWN new prose before the commit, gen-1 of this lane) + the two WildKnight-pinned
  regions (~114 run-data staging, ~195 pasted wine path) handled as supersession-CITES to
  d284880/9207579, not restatements. P-CMBT items 1-2 applied to CombatTurnDispatch (raiser
  section: +0x2E0 is the effect INBOX per T-074, anim deque is +0x1A8 per T-085, walk #2 of
  +0x304 raises at the tail; budget [ent+0x174] not +0x179; arm byte +0x318 not '+0x2E0+0x34';
  two-occurrence debounce per T-085 §4) + SimDigest T-048 pointer addendum. Item 3 (MapEntitySpawn.cpp
  comment) routed to the product lane (WildKnight/B2 owns that file's era). APPLY VERIFICATION:
  `0x54c828 = 8b 85 74 01 00 00`, `0x54c833/0x54c864 = c6 85 19 03 00 00 01`, `0x54c85d/0x54c86d
  = c6 85 18 03 00 00 00/01` — every byte the errata cites, re-read from the image; `8b 41 3c c3`
  at 0x436240 (D8); `[0x691a1c+0x18]=0x404b70` (D6); GUIStringTags §0 / GUILoadingRecon §1
  sections confirmed present (D7); the D12 12/0 number is attributed as the runtime lane's mesh
  report, not this lane's measurement.
- [ ] batch 4 (§4 per-file correction blocks: GUILoadingRecon, EntityActionQueueSeam,
  CombatTurnDispatch (T-072's own C2/C4 — separate from the P-CMBT block), StartupAndSubsystemTable,
  GUIStringTags, GamePathManager, WorldYearSeedConsumer, T013-registry-port-design,
  AttackPositionOverrideDoors, GITMItemRecordLayout; ScriptNativeActionDoors check vs B2 lane)
- [ ] batch 5 (pending flips / DONE marks)
- [ ] close: coverage statement + relay

## Cliff bank
Resume at the first unchecked batch; per-batch commit with the verification receipts.

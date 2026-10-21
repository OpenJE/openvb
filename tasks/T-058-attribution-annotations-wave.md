---
status: done
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — T-057 ping honored)
type: writer wave, ANNOTATION-TIER (comments only; sole IDB writer; expected T2 impact = ZERO)
depends: T-057 queue (discoveries doc committed `47582a4`, checkpointed `d1473a1`)
---

# T-058 — Apply the T-057 attribution queue as IDB annotations (273 rows self-explaining)

## Objective
Make the escalation families legible in the database itself: per-entry host-attribution comments,
the CEngineInterface composition resolution (including AMENDING batch-3's stale "base-vs-member
open" comments), and PAD classification notes. No class mints — families stay non-classes.

## Scope (source: discoveries/2025-10-21-t057-escalation-family-readjudication.md §3)
1. `instantiation-of-<HOST>@+0x<DISP>` comments on single-owner 50dbb0/4542d0 entries (§1 table).
2. `embeds@CEngineInterface@+0x<OFF>` at the ≥27 confirmed ctor-owners; ctor identity comment at
   0x580330 (`CEngineInterface::ctor, vft 0x69A95C`); AMEND the b3-reissued embedder comments
   (0x558740/0x5b6400/0x4395a0/0x5b4eb0/0x5b7c10 vicinity) replacing "base-vs-member open" with the
   CLOSED composition finding (shared COL 0x6D6510, offset-0; cite t057 §2).
3. PAD notes at the 21 cleanup-pad sites (SEH/atexit/vector-iterator).

## Reconciliation rules (EpicRaven pre-briefed, coordinator-ratified — read before flagging anything)
- EDGE-TOTALS are per-build: T-054 doc (44,690 edges/T1=314) vs T-057 (48,581/T1=317, 445350
  external=16/intra=4) are both self-consistent on the same pinned md5; quote the number against
  ITS doc's build. Flag only if a datum matches NEITHER build.
- PAD-vs-DELEG is the judgment bucket: PAD requires the pad VA itself dword-referenced in
  .rdata/.data (EH/atexit array evidence); pure intra-band tail-jmp pads are DELEG. Re-bucketing
  under that rule = taxonomy note in your ledger, not a flow mismatch. Both buckets = attributed-foreign.
- DISP immediates are byte-enumerated per ctor site — if a DISP moves, THAT is a true mismatch:
  ping the coordinator, EpicRaven arbitrates against the pinned cache with the site EA.

## Acceptance
- Ledger (writer-lane, committable): three sections matching the three scope items, per-site table
  (EA / comment text / set-verified readback), T2 before==after stated, stale-b3-comment amendments
  listed old->new with citation.
- Save discipline unchanged: mid/end save_database + mtime + dump-verify; comment presence proven
  by get_cmt readback per site (b4's 13/13 pattern).
- Any rename-worthy surprise discovered during application = NEW queue item (message me), not this wave.

## Acceptance evidence (delivered 2025-10-21)

- **Comments-only wave, T2 = 2,012 before AND after (delta 0)** — census unchanged by construction, verified
  by dump (no renames). 43 site comments applied, each `get_cmt` readback-verified (b4 13/13 → 43/43).
- **§1 AMENDMENT (highest value):** the 5 identical stale batch-3/4 comments
  (`embeds@CEngineInterface ... base-vs-member open, batch-4`) at 0x558740/0x5b6400/0x4395a0/0x5b4eb0/0x5b7c10
  rewritten to the CLOSED composition finding. Independently re-verified: each host sets its OWN primary vft
  at [esi] (0x692F58/6AC638/6561E4/65ABC4/6ACBF0 — all distinct) then constructs CEngineInterface at [esi+DISP]
  (DISPs +0xC4/+0x3C/+0x8/+0xC/+0x18 byte-exact, zero drift); COL 0x6D6510 dword-ref'd at EXACTLY one site
  (0x69A958 = vft 0x69A95C's [-4]) → composition not base. ctor identity @0x580330 + vft anchor @0x69A95C set.
  +11 additional embed-tier owners annotated (GameCSMenu/SkillMenu/CMainmenu/CMessageBox/CMiniMap/COptions/
  OptionsGame/OptionsVideo/OptionsMultiplayer/LoadingBar @+0x198 hotkey-outer).
- **§2/§4:** 18 `instantiation-of-<HOST>@+0x<DISP>` (50dbb0, 15 LIVE + 3 DELEG taxonomy-noted) + 6 chunk heads
  (4542d0). Each host-caller-verified against the live IDB.
- **§5 PAD:** 0x4bb6b0 confirmed SEH/EH-cleanup (pad 0x4BB755 dword-ref'd at .rdata 0x6E12EC), annotated. The
  0x507550 EpicRaven b4-era locator is NOT a live function head → logged as a build-locator taxonomy note
  (mechanism verified: pad 0x635653 dword-ref'd at 0x6E528C) rather than force-annotated on a non-head EA.
- Reconciliation rules honored (per-build edge totals, PAD-vs-DELEG bucket, DISP byte-check — zero drift); no
  `cls_0x4d8d70`/StringTree site touched (out-of-scope guard). T-060 PARKED, not started.
- Saves: checkpoint 15:50:18 + 15:52:37, end-of-wave `save_database` **15:57:36**; fresh-dump re-read confirms all
  comments persisted → disk==live. Ledger `discoveries/2025-10-21-t058-annotation-apply-ledger.md`. Claim-flip
  (in-progress) committed with this wave (admin beat from coordinator honored — no claim left only in worktree).

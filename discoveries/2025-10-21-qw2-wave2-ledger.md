# 2026-10-22 — Q-W2 WAVE-2a LEDGER (TrueCastle `01a16096`) — 99 MINTS APPLIED, READ BACK, SAVED

PARK pull per lane contract (claim d95ed81, GO 665e7cc). Ruleset: wave-1 verbatim + Gate A
(intersection-with-proof) + Gate B (&global Manager folds). EXE md5 a68c22f3... unchanged; IDB
`artifacts/F3.exe.i64`. Recovery: pre-flight in /tmp-captured prestate (see wavedumps/qw2-gateA-*),
post-state at discoveries/wavedumps/qw2-wave2a-poststate.txt (EA+name x99).

## Gates at write time (no self-waiving)
- Baseline SYNC save before first mutation: mtime 00:17:39.
- Preflight: 108 candidate rows, ALL live function heads, NONE already named — but 9 rows collided
  with T-084 §3's five ANNOTATE-FOREIGN class keys (doc-internal contradiction, same family as the
  0x515040 conflict) -> HELD (list below), queue 108 -> 99.
- Per band: apply -> name read-back (0 mismatches every band) -> checkpoint save
  (A 00:18:43, B 00:19:07, C 00:19:38, D 00:20:07) -> FINAL save 00:20:40 UTC, size 95273887.

## APPLIED (99 rows; per-host T2 itemization — source row = doc §2 tier-1 unless folded)
Band A (20): JE::World 12 — 0x4e7d80 0x4e7f70 0x54a610 0x54aa20 0x569f10(ctor) 0x56ae70(ctor)
0x56d7f0 0x5811a0 0x5a42b0(ctor) 0x5a4340 0x5a9330 0x5a94a0 | JE::Gregorian 8 — 0x425d30 0x4292e0
0x429bc0 0x4c6b60(Get_mbr_0x80 stem kept) 0x52c480 0x52c760 0x52c7d0 0x52c890
Band B (19): JE::Gfx_Engine 9 — 0x490120 0x50b4f0 0x50b6b0(ctor) 0x5cb510(ctor) 0x5cb740(ctor)
0x5cb790 0x5cbdb0 0x5cbe50 0x5cbe60 | JE::Gfx_ParticleSystem 5 — 0x513fc0 0x5149d0 0x515820 0x515dd0
0x51abf0 | JE::Graphics 1 — 0x4bdae0(ctor) | JE::Gfx_AnimMeshInstance 4 — 0x4a5a90 0x4a5b30
0x4a67f0(ctor) 0x5053c0
Band C (23): F3::GameActionEquip 6 — 0x43ae90 0x441d20 0x442450 0x44ad50 0x44b2a0 0x44db10 |
JE::AreaMapWindow 5 — 0x41d060 0x41d0c0 0x4203c0 0x4ae0a0 0x4ae100 | JE::Map 4 — 0x5704b0 0x572610
0x581330 0x5b1620 | JE::Client 3 — 0x4a4cd0 0x5b5b10 0x5b5c50 | JE::CounterControl 2 — 0x432550
0x432c60 | F3::GameActionOperateContainer 3 — 0x458770 0x496250 0x4aed50
Band D (24): JE::VisualEffect 5 — 0x48d8a0 0x48d8e0 0x48d910 0x48d940 0x48dc10 | net::client 5 —
0x5ab770 0x5ab930 0x5ab960 0x5aba20 0x5b9100 | JE::CSoundSystem 6 — 0x52ce60 0x52d080(ctor)
0x539d90(ctor) 0x53a180 0x53a1e0(ctor) 0x53a510(ctor) | JE::VFX_Model 3 — 0x49db70 0x49db80
0x4f45e0(ctor) | JE::ModelInstance 3 — 0x49d290 0x563c40 0x5646b0 | JE::DialogueManager 2 — 0x55e720
0x55e960
Band E (13): JE::GUI::Manager 6 GATE-B FOLDS (doc said Graphics/Label/Window/F3; folded on T-075
§1/§8 pure this==&0x707E48 evidence, per-row: 0x4b2ac0, 0x4b2c80, 0x4b3690, 0x4b4180, 0x4b4ce0 —
doc-listed &global rows — plus 0x4b4d90 from the hidden-10, raw b9-scan 3-edge) | JE::GUI::Label 4 —
0x4341f0 0x4a8c40 0x4a9070 0x4a9310 (note: live key was cls_0x401d60 — T-059 trio family; R-ST
split-annotation wave may revisit, doc tier-1 host stands) | JE::GUI::Skin 3 — 0x5226d0 0x5228b0
0x522a10
TOTAL: 99 renames; per-host T2 sum = 99.

## HELD (booked LOUDLY, not silently) — 16 rows of the announced-107 never applied
Gate-A HOLD (65 rows, as filed): untouched, arbitration/re-cut queue per coordinator ruling.
NEW §3-collision HOLD (9 rows caught at band preflight, doc tier-1 rows living under §3 foreign keys):
  0x52cc70 (under cls_0x401b00), 0x515040 (doc self-contradiction, the named case), 0x5150c0
  (cls_0x515040), 0x515580 (cls_0x40cc80), 0x515d50 (cls_0x40cc80), 0x4a5720 (cls_0x40cc80),
  0x4a5a10 (cls_0x40cc80), 0x436410 (cls_0x401b00), 0x523450 (cls_0x4dff90)
  -> route to EpicRaven/HappyStorm arbitration together with the class-(a) HOLDs: the foreign-key
  claim vs the §2 host must be reconciled per row before any of these mints.
Net vs announced GO scope (108): 99 applied + 9 collision-held. The stop-gate earned its keep twice.

## Census line
cls_-prefix: 1,991 (wave-1 close) -> **1,892 (−99, exactly the applied set)**; total fns 10,173
(unchanged). cls_0x4b28a0:: residue = 1 (the 0x619b38 CRT exclusion — intended).

## Board flips requested via relay
T-084 tier-1: 99/182 APPLIED; 65+9 HOLD rows = re-cut/arbitration list (obj/t087q/final-queue.txt,
banked at discoveries/wavedumps/qw2-gateA-queue-split.txt); tier-2 + §3 foreign batches (X-PUSH)
wait on the install-mode derivor re-bank (the finding that halved this wave's scope is now the
law: 'a queue artifact whose generator didn't ship is a queue with half its proof').

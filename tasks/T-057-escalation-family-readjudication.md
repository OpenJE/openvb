---
status: done
assignee: EpicRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — their own offer, pre-loaded data)
type: offline analysis (tools/ecx-flow.py against the md5-pinned cache; NO IDB writes ever from this lane)
depends: T-054 (the tool), T-055 in flight (any MINT-READY verdicts queue BEHIND CalmRaven's wave — writer slot is theirs)
---

# T-057 — Re-adjudicate the registered escalation families with the four-join tool

## Objective

The standing escalations are the bulk of the remaining T2 mass (50dbb0 120, 55dc80's 83 clones —
already ruled foreign-TU _Tree, 50db20 79, 4542d0 50). They were escalated under pre-T-054 evidence
rules; item 41 now makes "no owner" notes falsifiable in three independent queries. Answer them with
the tool, not re-scans.

## Scope (per EpicRaven's pre-load)

1. **cls_0x50dbb0** — clone census by stride/size-shape (repeat counts 0x329/0x1d1/0xbe/0xb7/0xd3/0x3e
   already measured) + `band --cls` owner answer + slot-dispatch census. Verdict per family: confirmed
   std::_Tree clones in named foreign owners (list the owners) vs any genuinely-unknown host.
2. **cls_0x50db20** — the CEngineInterface-embeds question (T2-12): re-run with the embed-tier proof
   (+0x84/+0x8c/+0x8d member stores keyed by owner in the flow cache) instead of the member-name guess.
3. **cls_0x4542d0** — one run, same shape.
4. Any candidate that comes out MINT-READY goes to the coordinator's QUEUE (writer slot belongs to
   T-055 until CalmRaven closes; nothing gets applied from this row directly).

## Acceptance

- discoveries file: per-family verdicts answering item-41's three queries explicitly for every
  negative; owner lists for the clone families; MINT-READY queue separated; gate-selftest-first
  wording (selftest green @ pinned md5 before any verdict).
- Board effect: T-033's "remaining T2" decomposition gets updated wording (confirmed-foreign vs
  owner-now-known), which is the census honesty the chain was closed on.

## ACCEPTANCE EVIDENCE (EpicRaven, 2026-10-21)

GATE-FIRST: `tools/ecx-flow.py selftest` 4/4 PASS on pinned EXE md5
`a68c22f3dd1c256ec9539a70e87cb434` (T1 317 / T2 4+1strong+4weak / T2b embed / T3 36) — stated
before every verdict below. Tool extended: SEH/atexit **pad-thunk recognition** (3,253 pads),
tail-jmp edges, transitive delegation, `families` mode (per-entry LIVE/DELEG/PAD/ISO + item-41
Q1/Q2 lines). `discoveries/2025-10-21-t057-escalation-family-readjudication.md` holds the tables.

Per-family verdicts, item-41's three queries ANSWERED mechanically (Q1 callers-of-callers via
DELEG chain, Q2 vft->thunk->entry slots, Q3 full .rdata+.data+.data1 scan):

1. cls_0x50dbb0 (136): 71 LIVE / 50 DELEG / 15 PAD / **0 ISO**. CONFIRMED-FOREIGN std::_Tree
   instantiation aggregate — hosts NAMED per single-owner entry: head 0x50DBB0 = ResourceSystem
   `cls_0x49c610`+0x108; `cls_0x55dc80`+0x24 (TexMgr map/list); `.?AUEntity@@`+0x394;
   `.?AVGfx_ModelInstance@@`+0xB8; `.?AVFont@GUI@@`+0x2C; `.?AUMap@@`+0xBC; +0x18 GUI-dialogue
   trio; `.?AVCEngineInterface@@` this-0x10; &0x70BE14 / &0x70BED8 pins. vft slots into band: 0.
2. cls_0x50db20 (83): 73 LIVE / 10 DELEG / 0 PAD / **0 ISO**. CONFIRMED-FOREIGN std::basic_string
   + ios-stream machinery aggregate (members incl. string ctor 0x401B20 / assign 0x401BD0).
   T2-12 SETTLEMENT: CEngineInterface ctor = 0x580330 (`movl $0x69A95C,(%esi)`); **33 embedded-ctor
   sites / >=27 named host classes** (all five batch-2 ctors confirmed at embed tier: sites
   0x55877D/0x5B644E/0x4395C2/0x5B4ED2/0x5B7C4C, plus CMainmenu/CMessageBox/CMiniMap/COptions*/
   GameCSMenu(+0xD54)/GameCSSkillMenu(+0xD58)/CFloatingText(+0x37550)/T-054 hotkey outer
   `cls_0x4468C0`+0x198). vft 0x69A95C has exactly ONE shared COL (.rdata 0x6D6510, TD
   .?AVCEngineInterface@@, offset 0) => COMPOSITION PROVEN, batch-2 base-subobject alternative is
   a CLOSED falsification. +0x84/8C/8D proven NOT CEngineInterface-specific (20+ owner store census).
3. cls_0x4542d0 (54): 23 LIVE / 25 DELEG / 6 PAD / **0 ISO** (orphan 0x4BB6B0 resolved by the new
   EH-call pad rule: handler 0x4BB755 `mov -0x14(%ebp),%ecx; call 0x4BB6B0`, dword-referenced at
   .rdata 0x6E12EC). CONFIRMED-FOREIGN mixed clone chunks; hosts named (model-map 0x4A2630 =
   Entity+0x370/+0x3C0 via Granny wrappers; AreaMap+0xA8; cls_0x4bc5f0+0x18; dialogue-epoch fan).

MINT-READY queue: NO new class mints (the census-honesty result — three largest T2 blocks are
proven non-classes). Queue to writer slot (behind T-055) = per-entry `instantiation-of-<HOST>@
+0x<DISP>` comments + `embeds@CEngineInterface@+0x<OFF>` on the >=27 ctor-owners + ctor identity
comment at 0x580330/0x69A95C + PAD/EH-cleanup notes. Board wording: T-033 "~250 foreign-family" ->
"273 attributed: 167 LIVE / 85 DELEG / 21 PAD / 0 orphan". No IDB writes; py_compile clean; 4/4
gate reproducible; README row added; discoveries file uncommitted (writer-slot discipline).

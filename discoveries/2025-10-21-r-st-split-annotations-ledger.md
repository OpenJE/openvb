# R-ST — split-annotations wave ledger (writer: HappyStorm `01a16092`, claim 10d4bf6, wave close this commit)

IDB: `artifacts/F3.exe.i64` — baseline mtime 00:20:40Z (Q-W2a close) -> save-synced 00:33:21Z BEFORE
first write (live==disk proven) -> writes -> checkpoint save 00:34:38Z -> end save **00:34:53Z**.
Dumps: `discoveries/wavedumps/rst-predump.json` / `rst-postdump.json` (schema = t0xx wave dumps).
Gate: ecx-flow selftest **6/6** @ md5 `a68c22f3…` before any write. Provenance tags in every comment.

## Proof of annotation-tier purity
- **10,173 functions pre == post; name-diff = 0** (comments only — no renames anywhere this wave).
- **T2 (startswith `cls_`): 1892 -> 1892, delta exactly 0** — annotation wave law held.
- All 6 comment sites `get_cmt` read-back verified live at 00:34:53Z state.
- (Baseline note: `startswith cls_` = 1892 vs the T-087 row's 1,991 = contains-variant metric; own
  baseline used consistently, both stated.)

## Queue [A] — T-059 split pair: VERIFIED ALREADY-LANDED, no-op (the verify-first catch of the wave)
| target | queue item | state found | action |
|---|---|---|---|
| 0x4B2DD0 (`JE::StringTree::meth_*`) | SPLIT-READING comment | FULL text present: item-43 wording, body bytes, 2,322-fan census, T-060 cite | none — verified |
| 0x56EE40 (`JE::StringTree::meth_*`) | same | FULL text present: two-SSO-block dtor, ~104-pad residual caution | none — verified |
Landed by an earlier wave (T-060-era/T-087 lineage); queue item closed as satisfied, NOT re-applied
(no duplicate provenance). Renames remain OPEN decisions (names still StringTree-wearing; T-059
declared rename a future mint-tier choice — excluded here per wave type, listed as successor).

## Queue [B] — T-086 decode notes (2 comments)
| site | kind | applied (head-line) | cites |
|---|---|---|---|
| 0x4C0190 `JE::GfxCameraHolder::ctor` | fn-head | SIX-host referencer census (all six store EAs listed: 0x5C0D19/0x5C0D9D/0x4B5845/0x42BAA7/0x43D04B/0x594038) + new(0x280)-set = sole creation; RenderManager-object ownership at +0x164 noted | T-086 §4.1 |
| 0x4A5BE0 `JE::Gfx_AnimMeshInstance::ctor` | fn-head | 4 heap ctor sites (3 VFX_Model + 1 ModelInstance per banked share-check), RTTI-anchored identity, band-SHARING = pointer fan | T-086 §4.2 |

## Queue [C] — R-CS §9 / T-086 [E2] store-site pair (4 comments)
| site | kind | applied | 
|---|---|---|
| 0x58BE40 `JE::Client::virt_meth_0x58BE40` | fn-head | CDebugInterface FACTORY identity: vft 0x69C814 slot40, new(0xF8) via 0x4A7680, image's sole `ctor_0x558740` call @0x58BEC1, name string 'Debug' @0x69CB18, S1-close statement |
| 0x58F010 `JE::Client::virt_meth_0x58F010` | fn-head | BOTH store sites named from the head per the ratified T-060 containment rule (0x58F0CF -> +0x8C, 0x58F114 -> +0x90), two instances one class |
| 0x58F0CF | instr | window #1 -> [client+0x8C] |
| 0x58F114 | instr | window #2 -> [client+0x90] |

## Exclusions honored (named)
0x539370 rename (MINT-tier, needs share-check-at-write + coordinator label call) — NOT applied.
R-CS §7.2-7.6 CSound-epoch comment batch — not one of the three named queues; successor below.

## Successor line (per lane contract)
1. **R-CS-ANN batch** (new PARK candidate): §7.2 SEH/register notes, §7.3 `[sound+4]` cached-alias
   member comment (the [E1] closure deserves its own on-object note), §7.4 class-tree notes on the
   six vfts, §7.5 cursor-as-CSound, §7.6 master-gate byte row — 5-6 cheap comment targets, one sitting.
2. **StringTree artifact-pair RENAME decision** (0x4B2DD0/0x56EE40): annotations done, names still
   mislead — mint-tier call for a future wave (share-check pre-gate; likely re-label as std::string
   helpers per T-059 amendment).
3. Wave-3 interlock: if EpicRaven's R-INST derivor re-hosts the 74-row list, the CDebugInterface
   rows should cite this ledger's factory decode (slot40/0x58BE40) as install-mode ground truth.

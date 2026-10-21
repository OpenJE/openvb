---
status: done
assignee: TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240` (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: research (READ-ONLY on ida_original_f3 — xref-weight queries only; NO IDB writes; discoveries file LEFT DIRTY until coordinator checkpoint)
depends: T-054 (registered the VACUOUS-membership verdict for cls_0x4b28a0), T-053 (string-mine method incl. \xc7-pad EA+1 xref trick), T-062 + T-064 (0x707CE4/0x707CE8 Gfx_Engine singleton anatomy — the exact pattern being reused), harness items 40-47 (method law)
---

# T-075 — Identity of the 0x707E48 GUI/skin-registry singleton (name-blocked since T-054)

## Objective
T-054 adjudicated cls_0x4b28a0's band as 142/142 (166/166 at that wave's count) pure
`this == &0x707E48` — membership VACUOUS because it is a static global with no container.
What remains unknown is **WHAT CLASS lives at 0x707E48**. This row closes that: RTTI +
single-writer store census + string mining + consumer-side naming, to a MINT-READY verdict
with a SINGLE-owner proof (item 44 machine test) or a FINAL anonymous-with-reasons verdict
with all three item-41 queries answered explicitly. Both verdicts are success; a guessed
name is failure.

## Attack plan (in order)
1. **RTTI COL->TD chain from the band's vft** — locate the vft the ctor stores at 0x707E48,
   read COL at vft-4, walk to TypeDescriptor; apply item 42: a size-0 `.?AV` names a VIEW,
   not the owner. Verify RTTI size before adopting any name.
2. **Single-writer store census (0x707CE4 Gfx_Engine pattern)** — A3-form `mov [abs],reg`
   scan across the whole image for stores of `&0x707E48` (and dword-store `C7 05 48 7E 70 00`);
   self-test the scanner against the known 0x707CE4 positive (item 35: empty + known-present
   = broken scan). A single ctor store = static-singlelet proof; multiple stores = sharing.
3. **String-mine the band** with the T-053 method incl. the \xc7-pad EA+1 xref trick; GUI/skin
   literals in the band's owned string data name the class family. Keep the dismissal list.
4. **Consumer-side naming** — 0x4b1848 window-scale reads inside GUI paint paths; map which
   named classes read through &0x707E48 and what fields they touch (vft slots, member offsets
   inside 0x707E48..0x707E48+size).

## Acceptance (mechanical)
- discoveries/2025-10-21-t075-skin-registry-identity.md (LEFT DIRTY, researcher never commits) with:
  - verdict = **MINT-READY** + explicit single-owner proof (item 44 machine test stated:
    NOT reachable via >=2 view constants AND NOT via >=2 holder fields; one host, one adjust)
    + the deciding datum (RTTI name + size check, store census count, consumer field map), OR
  - verdict = **FINAL anonymous** with the three item-41 queries answered explicitly at
    current tool strength: (a) callers-of-callers, (b) thunk-adjusted table resolution,
    (c) full .rdata + .data + .data1 vft coverage — plus named residual.
- Coverage statement (what was scanned, with bounds per item 35 rule 2).
- Zero IDB writes (mtimes prove it); coordinator checkpoint on [DISCOVERY-BATCH ready].

## Rules
- Read items 40-47 first; ecx-flow selftest gate BEFORE any verdict; byte-over-decompiler;
  EA-anchored everything; quote the scanner's known-positive self-test in the doc.
- Lanes to respect: HappyStorm (orphan hunt, same DB read-only), KeenStorm (product port),
  HappyEagle (DXVK boots at :96). Writer slot free-but-guarded — do NOT write the IDB.

## Result (TrueCastle, 2026-10-21)
**Verdict: MINT-READY — 0x707E48 is the complete object of `GUI::Manager`** (shipped RTTI
`.?AVManager@GUI@@`, TD 0x6faf80; live-DB convention `JE::GUI::Manager`).
- Deciding join: `ctor_0x4b5640` @0x4b5669 `c7 06 cc f2 66 00` stores vft **0x66f2cc** at `this+0`;
  `vft[-4]=COL 0x6d3dc8 → TD 0x6faf80 = .?AVManager@GUI@@`; CHD 0x6d3db8 maxNodes=1, MTD self-
  describing, CDOffset 0, export RTTI class size **188 == cls_0x4b28a0 size 188** (item-42 owner,
  not view). The prior "no vft store" negatives were an absolute-encoding blind spot (register-
  indirect `c7 06` inside the ctor) — item 8c family; new harness lesson filed in the doc.
- Single-owner (item 44): ctor called exactly ONCE (static-init thunk `0x641660`, the ONLY
  absolute encodings of 0x707E48 in .text are 168 `mov ecx,imm` this-loads; 0 stores/loads/lea/
  push/holder-dwords image-wide; second vft writer `0x4b52a3` = `0x4b5280` dtor vft-restore,
  same class; COL referenced only from its own `vft[-4]`).
- Scanner self-tested on 0x707CE4: all three known Gfx_Engine writers reproduced (a3@0x5bfe33,
  c7 05@0x5bedd6, 89 1d@0x5c1e24); ecx-flow selftest 5/5 green.
- Band purity re-measured: flow [A] 142/142, [B] 0 thunks-in, [C] 0/345 vft slots; string mine
  (T-053 method incl. \xc7-pad): zero shipped in-band strings — identity rests on tier-1 join.
- Writer queue: merge 27 band fns → `JE::GUI::Manager::` (EAs listed in doc §8); EXCLUDE
  `meth_0x619b38` (CRT `__getptd` tail thunk); FIX live `JE::StringTree::dtor_0x4b5280` →
  `JE::GUI::Manager::dtor_0x4b5280` (byte-proven).
- discoveries/2025-10-21-t075-skin-registry-identity.md written, LEFT DIRTY per protocol.
- Zero IDB writes; read-only list_funcs/xrefs_to only.

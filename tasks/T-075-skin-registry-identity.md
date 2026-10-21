---
status: in-progress
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

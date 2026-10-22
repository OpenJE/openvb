# 2026-10-22 — R-INST: the install-edge derivor ships green — one-class answer for +0x118, and the collision-9 resolve as shared machinery

TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240`, PARK first-pull (claim `6b906aa`). Read-only:
pinned cache + raw EXE (md5 `a68c22f3…` enforced in-tool) + committed dumps; zero IDB writes, zero
product touches. Deliverables: `tools/r-inst-deriv.py` (selftest-gated, refuses verdicts on red) +
this verdict feed (74 rows) for EpicRaven's Q-W2b arbitration. Full machine output:
`obj/t087q/rinst-74-feed.txt` (regenerable: `python3 tools/r-inst-deriv.py --run74`).

## 0. Selftests (ALL PASS — the gates are the claims)

- **S1 (the +0x118 pair, tool ANSWERS not inherits):** all five `cls_0x569f10`-family rows
  (0x569790/569970/0x569d50/0x569de0/0x569f10) resolve install-host **JE::World @+0x118**,
  votes={World:1} each, and the histogram contains **zero JE::Client embed-votes at that offset**.
  ANSWER: ONE component class, embedded ONLY by World; the doc's Client/World split is a
  caller-mode artifact (the component's methods are called BY Client-side code, which reads as
  ownership). Corollary that vindicates wave-2a: Client's three APPLIED rows (0x4a4cd0/0x5b5b10/
  0x5b5c50) are field-load members of Client proper — correct hosts stand; the six HELD "Client"
  embed-rows are World's components (+0x118 trio cluster cls_0x569f10; +0x108 trio
  0x56a6f0/56a8d0/0x5acb0 cluster cls_0x56ae70, same shape independently).
- **S2 (banked positive recovered independently):** the T2b edge (owner *ModelInstance*, this+0x44,
  target 0x56E0B0) is present in the derivor's disps — among **49 embedding hosts** total for that
  target. This is the helper-vs-component discriminator made concrete: 0x56E0B0 is a GENERIC
  initializer helper (every class's ctor calls it for its list heads), so unique-host is the WRONG
  expectation for helper-targets — S2/S3 therefore assert edge-recovery, not uniqueness, and the
  derivor flags multi-host clusters honestly.
- **S3 (blind, the five-wave shape):** target 0x4C6640, host NOT instructed → derivor recovers
  **(JE::GUI::Manager, +0x48)** from the ctor-chord edges — the register-indirect embed that hid
  from five waves of absolute-store censuses is routine material for the install join.

## 1. Verdict classes shipped to arbitration (74 rows: 65 Gate-A HOLDs + 9 collision rows)

| class | n | meaning | rows (highlights) |
|---|---|---|---|
| **ADOPT-BY-INSTALL** | 13 | unique install edge; host is NEW or re-homing vs doc | 6 Client->World component rows (above); 0x5ad7b0 doc=Map -> **JE::AreaMap @+0x60** (joins its tier-2 siblings 0x5ad7e0/5ad850 at +0x40 — coherent component story); 0x56d030/90 doc=Map -> installer cls_0x570800 (UNNAMED host — resolves the row but not the name; chain to wave-3); 0x442bf0/3e0 doc=F3 -> installer sub_443410 @-0x34 (same unnamed-host class); 0x44dcb0/fc70 doc=GameActionEquip -> installer sub_453540 (=CPipBoyAddNoteInterface vft member per T-053 §2b) @-0x18 **while wave-2a already minted 3 siblings of the same cluster as GameActionEquip** — shared-component/template shape across two hosts; recommend: follow the already-minted cluster host (GameActionEquip) and annotate the PipBoy embed edge, do NOT re-home |
| **CONFIRM-DOC** | 12 | install evidence matches the doc host | includes the +0x40/+0x4C CounterControl embed rows and the VFX/Vehicle family — safe wave-3 mint material |
| **CONTRADICTS-DOC / MULTI-HOST** | 16 | >=2 install hosts vote | nine collision rows (below) + seven embed rows with genuinely two named installers — each carries its vote histogram in the feed |
| **NO-EVIDENCE** | 33 | no install edge exists for the cluster | the rest of the 40cc80/401b00 inline machinery + field-load-only rows; these stay in caller/doc arbitration — install mode is silent, which is honest, not a refutation |

**The collision-9 answer themselves (independent of EpicRaven's derivation archaeology):** every
one of the nine sits in a cluster with **11–25 distinct install hosts** (cls_0x401b00: 25,
cls_0x40cc80: 14, cls_0x515040: 11, cls_0x4dff90: 13). That is the machine definition of SHARED
MACHINERY — §3's ANNOTATE-FOREIGN disposition is right for all nine and §2's mint rows are the
error, the item-52 contradiction resolved by evidence rather than adjudication preference. Proposal
to the arbiter: annotate-foreign x9, zero mints.

## 2. What the derivor's mode adds as a standing law (for ecx-flow graduation later)

Install edge := call site with ecx = (this+disp) [flow-model `plus/inecx` shape], owner of site =
host, target's cluster = component. Row->cluster: stored-vft (c7-imm-vft scan, item 49 patterns) >
vft-table membership > t064-key family. Host: unique vote = assign; multi vote = shared/template
annotate; helper-target (49-hosts-for-0x56E0B0 is real life) = never demand uniqueness — that
discipline is exactly what the archived doc silently lacked.

## 3. Limits of this row (stated so nobody over-buys)

- I DO NOT apply: every row above is a PROPOSAL; wave-2b/3 writer disposes (EpicRaven arbitrates
  both classes per PARK Q-W2b row).
- Unnamed installers (sub_443410, cls_0x570800) identify a host OBJECT but not its class name —
  those rows need one more read-back hop (installer's own ctor/vft) before minting; deliberately
  not guessed here.
- The 12 CONFIRM-DOC rows were confirmed by install VOTES only; per-row item-44 share-check still
  applies at their apply time (no self-waiving in reverse either).
- disp semantics: negative disps (-0x18/-0x34) = installer holds a POINTER INTO a containing
  object (base-at-negative-offset / inner-struct pattern) — the host is the OUTER object; the feed
  prints raw so the arbiter sees the shape, not my paraphrase.

## Raw-evidence index
Tool: tools/r-inst-deriv.py (md5-pinned, selftests S1-S3 in-code, --run74 regenerates the feed).
Feed: obj/t087q/rinst-74-feed.txt (75 verdict lines incl legend). Base data: t064-postdump
(cluster identity), qw2-wave2a-poststate (current labels), pinned flow cache. Key sites cited
inline: World embed 0x548bfa/0x548bea; Manager _Tree 0x4b56cc; T2b family 0x564d3c.

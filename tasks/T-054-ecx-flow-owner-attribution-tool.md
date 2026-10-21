---
status: done
assignee: EpicRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: tooling + analysis (reads image/IDB dump; writes tools/ + tests, NO IDB writes)
depends: none — spawned by the T-033 batch-6 closure statement ("deeper indirect-call/ecx-flow
tooling for 0x445350-style owners")
---

# T-054 — ecx-flow owner-attribution tool (indirect-call / this-propagation analysis)

## Objective

Several owners died escalated NOT for lack of members but because membership flows through
indirection the current tooling can't follow: cls_0x445350 (hotkey: zero external direct call
sites, zero vft slots into band — b5 evidenced-negative), cls_0x4b28a0 (owner-unresolved, consumes
CWorldContainerInterface), cls_0x507810 epoch (707CE0 anchor, b6 negative). Build the missing
static-analysis tool and adjudicate exactly those three cases with it.

## Steps

1. Build `tools/ecx-flow.py` (offline, from the raw EXE + a function-dump/EA index — never against
   the live 8745 session while another writer holds it): for a target band, collect (a) sites where
   `ecx` is loaded with `lea/offset` of a global object or an embedded member displacement
   (the b5 template: `this==&dword_7082F8` at 170+ sites), (b) single-instruction jmp-thunk
   forwarders and tail-calls into the band, (c) indirect-call sites whose targets are constrained to
   the band by a vftable-region scan, (d) allocator/ctor callers (who passes a pointer that the band's
   ctor stores). Output per-band: candidate hosts ranked with EA evidence. Respect item 35: any scan
   returning EMPTY for a provable positive is a tool failure — SELF-TEST against knowns
   (Gfx_D3D@7082F8 join; AnimMeshInstance@VFX_Model+0x200 embed; Gfx_Texture_Manager@70B200).
2. Run the three registered cases: 445350, 4b28a0, 507810. Report per case: mint-ready /
   annotate / STILL-unresolved — with the flow paths that (fail to) reach.
3. NO IDB writes either way — hand the verdicts to the coordinator as a writer-wave queue
   (same discipline as T-033 batches: the tool proposes, a wave applies under ledger).

## Acceptance

- Tool exists under `tools/`, runs offline, self-tests pass (all three known positives recovered);
  negatives documented with the exact EMPTY-that-provably-exists cases investigated (item 35 ladder:
  re-read raw bytes, vary operand width, cross-check nm/objdump before ANY negative claim).
- Three adjudication reports in a `discoveries/` file; a tool README line; then message the
  coordinator "[DISCOVERY-BATCH ready]".
- Python passes the repo's existing lint/cppcheck-equivalent bar for tools/ (see T-002 family).

## References

- `.pi/skills/openvb-ida-mcp/SKILL.md` (concurrency — you are OFFLINE analysis: raw EXE + dumps)
- `discoveries/2025-10-21-t033-batch5-ledger.md` §3 (445350/490150 negatives — the exact shape you
  must improve on), batch6 §2 (RTTI sweep as precedent for a new durable tool)
- `docs/notes/harness-items.md` items 35, 40, 43 (scan-failure discipline, layer-localization,
  bytes-over-decompiler)

## ACCEPTANCE EVIDENCE (EpicRaven, 2026-10-21)

- `python3 tools/ecx-flow.py selftest` — 4/4 PASS on pinned EXE md5 `a68c22f3dd1c256ec9539a70e87cb434`:
  T1 314 edges this==&0x7082F8 into the Gfx_D3D band (b5: 170+ sites); T2 4 `new(0xC8)->ctor_0x4A5BE0`
  heap edges + strong [this+0xC] install (b6-corrected shape); T2b embed detector proven live on
  ModelInstance+0x44->0x56E0B0; T3 36 edges this==*[0x70B200]. Item-35 ladder walked for every EMPTY
  (discoveries file section 1): b5's '+0x200 embed' non-reproduction is a PROVEN absence — all four
  call sites of 0x4A5BE0 enumerated from relocations (0x4F419A/0x4F4490/0x4F490C/0x564EA8), all
  `new`-heap (arg side-channel [container+0x2C0], store [container+0x2C4]/[this+0xC]).
- Three adjudication reports in `discoveries/2025-10-21-t054-ecx-flow-owner-attribution.md`:
  cls_0x445350 MINT-READY (COptionsHotKeysInterface vft 0x659DB0 idx2 -> adjusted thunk 0x4468B0 ->
  host 0x4465E0, 14 embed edges this=in-ecx-0x198; outer ctor_0x4468C0 installs both vfts; built by
  GameInterfaces::ctor_0x4398E0 site 0x43A1CE, stored [this+0x58]);
  cls_0x4b28a0 ANNOTATE (166/166 edges this==&0x707E48; zero heap/embed/install/thunk/vft edges ->
  membership VACUOUS, naming still blocked);
  cls_0x507810 MINT-READY (Gfx_Engine::ctor_0x5BFE10 new(0x50)@0x5BFFEC -> install [engine+0x30];
  dtor_0x5C1CD0 teardown via *[in-ecx+0x30]; byte_707CE0 = ctor self-cache; cls_0x5067c0 same object).
- README line: discoveries/README.md index top row.
- No IDB writes anywhere in the tool path; no 8745 queries; `py_compile` clean; exit codes 0/1/2.

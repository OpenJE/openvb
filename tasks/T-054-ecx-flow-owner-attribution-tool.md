---
status: in-progress
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

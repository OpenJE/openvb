---
status: in-progress
assignee: EpicRaven (PULL from tasks/PARK.md R-S9 per lane contract @ ed8c1af, dispatched 2026-10-21)
type: offline analysis (cache/objdump/F3.json + pinned flow edges; share-check before holder claims; NO IDB writes)
depends: T-083 (SwiftMoon's Gfx vft census — the shortlist source), T-054/T-076 (tool + terminal-field machine)
---

# T-088 — R-S9: Gfx follow-on decodes — s9 teardown twin, s101/102 self-slot-54 chain, MapFile@+0x40 decode to ASSERT D3D8 slot numbers

## Objective
SwiftMoon's T-083 top-5 lands in this lane:
1. **s9 device-teardown twin** (0x5C1B30, slot 9): decode the `[this+0x894]` array teardown
   loop fully — node dtor 0x4296C0, the Gfx_D3D side-calls (0x503C30/0x506260/0x506B70),
   and the twin RELATION to slot 8 (0x5C0C30) so the pair gets role names.
2. **s101/102 private-texture-id chain** (0x5C0360/0x5C0370): decode the stored-id fields
   feeding the self-slot-54 tail (`call [eax+0xD8]`) — what ARE [this+0xD4]/[0xD8]?
3. **The b5 D3D8-slot-numbering UNASSERTED note**: decode the MapFile object living at
   **Gfx_D3D+0x40** (size 0x240, entry array +0x70, counts +0x88/+0x90/+0x98), then use the
   device-call census (every `mov eax,[this+8]; call *[off](eax)` in the Gfx_D3D band) cross-
   checked against the AUTHORITATIVE IDirect3D8/D3DDevice8 vtable orders (Microsoft Learn,
   d3d8.h) to convert the slot claims from UNASSERTED to ASSERTED with per-call evidence.

## Contract posture
Cache-first (md5 a68c22f3…, selftest 6/6 stated before verdicts); share-check (terminal-field
machine) before ANY holder claim; findings DIRTY for checkpoint; any queue items to writer slot
in T-084 queue shape. Verdicts limited to tool strength; slot asserts require a DOCUMENTED
vtable order match, not shape vibes.

## Steps
1. MapFile decode: owner of Gfx_D3D+0x40, ctor site, entry stride at +0x70, count semantics
   +0x88/+0x90/+0x98, accessors list.
2. Device-vft census: collect ALL indirect device calls in the band (offset, site, args pushed),
   align offsets to d3d8.h vtable indices via Microsoft Learn, list which b5/b6/b4 claims
   become ASSERTED vs remain shape-only.
3. s9: teardown loop decode + slot-8 twin relation; s101/102 stored-id decode.
4. discoveries file + queue; relay.

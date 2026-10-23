# Held-mint successor, part 2 — bodies decoded, both stay HOLD (weak-tier annotations landed) (splitseat, 2026-10-23)

## meth_0x5a7e20 — HOLD (weak)
- Single named caller: `JE::Script_Small::Debugger` @0x5a8161 (plain-this).
- Body: SEH-guarded, std::string locals; decompiler fails (persistent, 3 attempts).
- No classmate call on `this` → root cannot be pinned by the wave rules. Verdict: annotate, not mint. Annotation landed (read-back ok, save 19:32:40Z).

## meth_0x49fae0 — HOLD (weak), body now DECODED
- Walks the vector at `[arg0+0B0h]` (count) / `[arg0+0B4h]` (end), each element a node attribute.
- `RTDynamicCast` each element from `Gfx_NodeAttribute` → `Gfx_ModelMeshPkt` (operator new 0x58, `Gfx_ModelMeshPkt::ctor_0x49e1d0`) → else `Gfx_SkinMeshPkt` (operator new 0xC8, `Gfx_SkinMeshPkt::ctor_0x49dea0`); pushes each packet into the member vector at `this+0xAC` (cls_0x49eeb0 push_back, `meth_0x49f4f0`).
- Copies `[arg0+20h..2Ch]` (3 floats) into `this+20h` and mirrors into `this+5Ch/60h/64h`; sets byte `this+0Ch=1`.
- Called from `JE::Gfx_ModelInstance::ctor_0x4a0400` @0x4a0554 with `arg = mbr_0x44 + 0x24`; the RETURN is re-cast to ModelMeshPkt/SkinMeshPkt for `mbr_0x6c`/`mbr_0x70` — so the fn returns the first mesh packet found/created.
- Host root UNPINNED: no named classmate call on `this`; `this` shape (+0xAC vector, +20h floats, +0xC byte) matches no DB-named class yet. Verdict: annotate, not mint. Annotation landed.

## Semantics summary (for the next root-chaser)
`meth_0x49fae0(this, nodeObject)` = "collect mesh packets from nodeObject's attribute list into this's +0xAC vector, return the first packet". The host class root can be resolved by finding who else constructs/holds the +0xAC vector — search ctor bodies writing `sentinel` at +0xAC.

## Held-mint board state
7 → still 7 mints held in identity terms, but 2 of the 3 bare roots now carry decoded evidence annotations (upgrade from "unresolved" to "decoded-but-hostless"). Remaining true roots: 0x5a7e20 host, 0x49fae0 host, 2 CWnd MFC rows (member-mint meaningless), 0x4950d0 SKIP stands.

## Save
19:32:40Z (comments-only; 10,173 fns / 7,690 named).

# Held-mint successor, part 1 — cls_0x566490 root resolved as std::_Tree (splitseat, 2026-10-23)

X-PUSH successor slot: "the 9 held mints re-open when their class roots resolve."
This sitting resolves the first root: **cls_0x566490** (the "566490-scripted-ctor island").

## Evidence (live decompile, ida_original_f3)
- `0x566490` throws `std::out_of_range("invalid map/set<T> iterator")` (throw sites
  `_CxxThrowException` with `_TI3_AVout_of_range_std__`), then performs a red-black
  tree unlink/rebalance: nodes are left/parent/right + color byte at +0x38, header
  sentinel test (`node[57]` = isnil), `_Myhead`-relative fixups (this->mbr_0x4), size
  decrement via mbr_0x8, and the erased node's VALUE member holds a `std::string`
  (freed through the string-dtor path). Identity: **`std::_Tree::erase(iterator)`
  (MSVC xtree)** — strong, name-string evidence.
- `0x5a4610`: promotes the RIGHT child of a node to that node's parent with the full
  MSVC `_Lrotate` fix-up sequence (verified clause-by-clause: `_Right(_Pnode) = _Left(_Pright)`,
  `_Parent(_Left(...))` fix, parent-child swap via `_Left/_Right` test, final
  `_Left(_Pright) = _Pnode`). Identity: **`std::_Tree::_Lrotate`**.
- `0x5a4670`: exact mirror → **`std::_Tree::_Rrotate`**.

## Applied (evalfile path, in-payload read-back verified, save proven 19:20:34Z)
- 0x566490 → `std::_Tree::erase_0x566490_566490`
- 0x5a4610 → `std::_Tree::Lrotate_0x5a4610_5a4610`
- 0x5a4670 → `std::_Tree::Rrotate_0x5a4670_5a4670`
- Each carries the identity annotation as repeatable func comment.
- Census: 10,173 fns / 7,690 named unchanged at the count (3 renames of already-counted fns; named count 7,690 → 7,693 expected on next rebase — these three were already counted in `named` since cls_-labelled, so no move).

## Transport law learned (recorded for the successor lane)
The wrapper arg envelope corrupts quote-dense JSON args (JSON.parse fails server-side
→ `{}`), while plain-string dedicated slots (`file`) survive. Consequence: complex
payloads and renames now travel via `action=evalfile` (write /tmp/*.py, wrapper
reads the file). evalfile landed as 5d2597e and is LIVE (verified this sitting).

## Remaining held mints (unchanged, still gated)
- bare `meth_0x459c50` / `0x5a7e20` / `0x49fae0` (owner class roots unresolved)
- 2 × `??__E?wndTop@CWnd` rows (MFC mangled-host identity — member-mint meaningless)
- 0x4950d0 SKIP stands (live name JE::Gfx_RenderManager::ctor wins)

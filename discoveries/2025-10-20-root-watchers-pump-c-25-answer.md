# 2025-10-20 — Session 30: GUI-root watchers completed + pump located; answer to C-25 (flag decider)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Completes the root-object pass
(`2025-10-20-gui-root-cls-0x4b28a0-map.md` §5 first bullets) and answers implementer-A's
C-25 question about the EME2 flag byte.

## 1. Root watcher mechanics — the three small methods

* `meth_0x4b2fe0(x)` = **watcher-insert**: single call `cls_0x401b00::meth_0x40e780(&root+0xA0,
  &x)` (set-add on the +0xA0/A4 collection).
* `meth_0x4b3000()` = **current-watcher query**: `+0xA8` gate; returns `*(*set_begin + 8)`
  (first element's +8 field — the watcher's subject ptr, likely the interface it fronts).
* `meth_0x4b2f30(struct{count=3,rects…}, obj, node)` = **two-step virtual dispatch**:
  optional `+0xA8`/`mbr_0xa4[2]==obj` filter, then `obj->vft[+8](node, struct)`; if false →
  `node->vft[+284](struct)`. This is the per-structure delivery hop used by the pump (§2).

## 2. The root pump: `cls_0x4b28a0::meth_0x4b3040` (0x3E9 bytes)

`0x4b2f30` has **13+ call sites, ALL inside `meth_0x4b3040`** — a big multi-branch dispatcher
(different {obj,node} pairings per branch — input-class switch most likely). ⇒ The root's
frame/event fan-out iterates the watcher structure and delivers via §1's hop. Interface-base
contract additions (vft byte slots): **+8 (deliver/query), +284 (0x11C — handle-struct)**.
Not decompiled body-by-body this pass (0x3E9 — worth its own pass when the input lane starts;
callers of 0x4b3040 will name the per-frame vs event distinction — check xrefs next round).

## 3. Named watcher consumers (xrefs) — who claims UI attention

| inserter (0x4b2fe0 call site) | context |
|---|---|
| `cls_0x424a20::meth_0x424a30` @0x424a56 | **character creation** flow (same TU region as the GamePlayer direct-ctor 0x42e870 from the gameworld doc) |
| `sub_433EE0` ×4 (0x433f97/4030/409c/40f3) | **Feedback** interface machinery (multi-insert = staged messages?) |
| `JE::CLoadMapInterface::virt_meth_0x4404a0` ×3 + `sub_440CE0` ×3 | **LoadMap UI** flow |
| readers: `F3::CPipBoyInterface::virt_meth_0x455da0` (8-byte passthrough of 0x4b3000) + `sub_453540` (Pipboy TU) | **Pipboy** queries current-watcher |

⇒ watcher-set = "which UI context currently owns attention/input", inserted by chargen /
feedback / loadmap flows, queried by Pipboy. Enough to model; full per-branch semantics await
the 0x4b3040 pass.

## 4. ANSWER to implementer-A C-25 (flag-decider) — also via RESPONSES thread conventions

Question: "find the code that DECIDES the [EME2] flag byte per-record (editor-side or
version-gated)". Findings:
1. **No decider exists in the shipped runtime exe.** The only writers of the EME2 record are
   the map file itself (byte consumed at 0x5740f5, stored to Entity+0x3A0 @0x57417e; nothing
   in .text rewrites +0x3A0 outside `SetActive` paths — per-record choice is baked in the
   data). Editor-produced (GECK/Sapphire never shipped) ⇒ your empirical "all shipped = 1"
   IS the complete story for this demo slice: model `flag = record byte`, default 1-side.
2. ⚠ The `ECIT` fourcc you cite does **not exist in F3.exe**: byte scans for 'ECIT','ECMP',
   'ECAR' as imm32-compare AND raw string = absent (only 'EMEF' present). **Most likely you
   mean EMEN** — the legacy map-entity chunk whose v≥2 path carries a *versioned pump byte*
   (u8 → `meth_0x548d60`, session-22 doc §4) — i.e. exactly the thing §4.3 below distinguishes
   from EME2's flag. If so, no reconciliation needed; if you truly saw an ECIT elsewhere, cite
   the EA.
3. Consistency check: the same byte position in the legacy EMEN path is ABSENT (EMEN has no
   flag byte; pump state comes from its versioned fields instead, session 22 §4) — so
   "old maps ⇒ flag semantics differ" is REAL: don't reuse the EME2 default for EMEN records.

## 5. Open (root/UI lane)
* `meth_0x4b3040` full branch decode + its callers (frame vs input routing) — next.
* Interface-base vft+8/+284 body census on 2–3 real interfaces (Feedback + LoadMap) to pin
  the "deliver" contract shapes.
* Still-open from session 28: owners of GameInterfaces late slots mbr_0x14/2C/30/34/3C.

## 6. Evidence
0x4b2fe0 body (insert 0x4b2feb); 0x4b3000 (gate 0x4b3000/first-elem 0x4b3017..0x4b301d);
0x4b2f30 (filter 0x4b2f38..0x4b2f53, dispatch 0x4b2f68, fallback vft+284 0x4b2f74);
0x4b3040 = 0x3E9-byte fn containing every 0x4b2f30 caller (xrefs listed); consumer table
rows = call-site EAs §3. C-25: string/imm scans over full F3.exe byte space for ECIT/ECMP/
ECAR (absent), EMEF present at 0x16c7a9(off-file? no — .text raw offset; cmp at session 21);
EME2 flag consumption 0x5740f5/0x57417e.

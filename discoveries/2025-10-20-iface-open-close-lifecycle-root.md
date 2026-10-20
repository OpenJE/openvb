# 2025-10-20 — Session 31: interface OPEN/CLOSE lifecycle on the root — pump is LAYOUT, base class `cls_0x495420` found

Target: ORIGINAL_REFERENCE (IDA). Read-only. Completes the root/UI-attention model begun in
sessions 27–30; corrects session 30's neutral "dispatch/pump" label to its real role.

## 1. `meth_0x4b3040` (the 0x3E9-byte "pump") = **layout & redraw pass**, NOT input

Callers: `JE::Graphics::virt_meth_0x581230` (render frame), and the two root mutators §2/§3 —
always invoked as `meth_0x4b3040(root, &root.mbr_0x2C /*rects*/, 0)`. Its internal fan-out is
the `meth_0x4b2f30` hop: `child->vft[+8](rects, node)` else `node->vft[+284](rects)` ⇒
**+8 = "layout-provide/measure" on the layout parent, +284 (0x11C) = "onLayout(rects)" on the
interface node** — refine session 30's "+284 handle-struct". Filter at 0x4b2f38..53: only
dispatches when `root+0xA8` set and the first watcher subject (`*(*(+0xA0)+8)`) matches `obj`.

## 2. OPEN path — interface base class `cls_0x495420::meth_0x495420` ("Show/Register")

Body (0x495420..0x495491): destroy the instance's CHILD LIST `{mbr_0x28 head, mbr_0x2C count}`
(link-unlink + optional dtor per node gated by `sub_4A75A0(v4+6)` = ownership flag at
node+24), then register with root:
```
root.meth_0x4b2fe0(this)      ; insert into watcher set @root+0xA0
root.meth_0x4b3430(this)      ; OPEN (below)
```
⇒ `cls_0x495420` = **JE interface base** (region 0x495xxx with SetActive 0x495220). Its +0x28
child list = widgets/fragments (B's GUI::Fragment lane), ownership-flagged. This is the method
the named "inserters" (chargen 0x424a56, Feedback 0x433f97…, LoadMap 0x44062f…) sit inside or
call alongside.

## 3. `meth_0x4b3430(x)` = **Open(x)** and `meth_0x4b34b0(x)` = **Close(x)** (root mutators)

Open(x) @0x4b3430:
```
walk modal chain (root+0xB8, iface vft+152 first-visible / +160 next):
   if FIRST-VISIBLE == x  → clear modal (meth_0x4b2ad0(0))   ; x stole head visibility
push x onto set root+0x1C ("open set")                        ; meth_0x40e780
set dirty root+0x4
if root.mbr_0x8 == x: root.mbr_0x8 = 0; layout-redraw()        ; ⚠ corrected by sess-32:
                                                                  ; 0x8 = HOVERED CONTAINER
```
Close(x) @0x4b34b0:
```
if x == modal-head → clear modal
if x == root.mbr_0xC (⚠ sess-32: HOVERED CHILD, not active-open iface):
    mbr_0xC = 0; x->vft[+84](1)     ; interface "Hide/OnClose(1)" virtual
    layout-redraw()
```
Only in-binary callers of Close: `JE::GUI::Label::meth_0x4b0a50` (widget click path closes the
active interface — the shipped "click-through widgets can dismiss the owning iface" behavior).
⇒ root fields ~~final: mbr_0x8 = pending-open, mbr_0xC = active-open iface~~ **SUPERSEDED by
session 32: 0x8/0xC = HOVER pair (container/child); mbr_0x1C = open
set, +0xA0 = watcher set (+0xA8 gate), +0xB8 = modal chain head**; `meth_0x4b4f00`
(session 27 +0x08 bootstrap) walks the chain, clears stale-modal on head-activation, pushes
viewport rects via 0x4b2f30 and inserts its arg into the **watcher set @+0xA0** (insert site
0x4b4fa3 — NOT the open set; distinguishes the two collections precisely).

## 4. Interface-base vft contract (bytes; consolidated across sessions 27–31)
```
+8     provide/query for layout hop           (0x4b2f30 first leg)
+84    Hide/OnClose(bool)                      (Close path)
+0x98  first-visible self-query                (chain walk)
+0xA0  next-sibling walker                     (chain walk)
+0x11C onLayout(rects{count,rects...})         (0x4b2f30 second leg)
+0x118 onModalChanged(willClear:int)           (0x4b2ad0 notify)
+0x20? vft-slot for SetActive = 0x495220 owner-base; iface byte active@+0x20(+32)
```
(Where iface==the vb-adjusted base object; per-class vfts override — e.g. F3 CLoadMapInterface
0x4404a0/0x440CE0 are these contract points on the F3 side.)

## 5. Now the UI-attention story is fully evidence-backed
demand factory (sess 28) → open via base Show (§2) → root.Open (§3) → modal chain + dirty →
JE::Graphics triggers layout pump (§1) → widgets (B's fragments) receive rects via +8/+0x11C.
Port checklist: ~~pending/active pair (0x8/0xC)~~ hover pair per sess-32, open-set @0x1C + hit-ring @0x20, watcher-set gate @0xA0/A8,
first-visible steal rule, click-to-close via Label→Close.

## 6. Open
- ~~Who sets root.mbr_0x8 (pending-open) vs mbr_0xC (promotion to active) — likely inside
  0x4b3040's branches (deferred; needs its 0x3E9-byte pass).~~ CLOSED by session 32: both are
  set by 0x4b3040's hit resolution (hover container/child).
- mbr_0x2C rect struct exact shape (4 ints + count 3 observed).
- GameInterfaces late-slot owners (session 28 item, untouched).

## 7. Evidence
0x4b3430 (walk 0x4b3447..0x4b345b, steal-clear 0x4b346b, open-set 0x4b3470, dirty 0x4b347f,
pending-consume 0x4b3487..0x4b3498); 0x4b34b0 (modal clear 0x4b34c8, active-close
0x4b34d0..0x4b34ea incl. vft+84 at 0x4b34df); base Show 0x495420 (child destroy 0x49543b..
0x49547c, registers 0x495486/0x495491); pump caller JE::Graphics::virt_meth_0x581230 xref
0x5812a0; Close caller GUI::Label 0x4b0b25.

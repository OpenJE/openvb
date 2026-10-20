# 2025-10-20 — Session 29: GUI-root (cls_0x4b28a0 @0x707E48) field map — and a global-collision catch

Target: ORIGINAL_REFERENCE (IDA). Read-only. Third UI-thread pass; opens the root object
touched by every interface activation (session 27 §2/§3).

## 1. ⚠ Correction first: `byte_707F00` is NOT a second global — it IS `GUI_ROOT+0xB8`

`0x707E48 + 0xB8 = 0x707F00` (exact). So session 27's "visible-interface stack head
(byte_707F00)" and "modal root (0x707E48)" are ONE object: field `mbr_0xB8` of the root is
the modal/current-interface chain-head that 0x495220 and 0x4b4f00 walk. Session 27 §3 row
merged accordingly (edited in place).

## 2. `cls_0x4b28a0` (GUI root, 188 bytes, global @0x707E48) — field/method map

| field | meaning (evidence) |
|-------|--------------------|
| +0x04 byte | **"UI dirtied" flag** — `Set_mbr_0x4_to_1` (0x4b2ab0) called on every SetActive (0x495231) ⇒ redraw pulse |
| +0x08, +0x0C | root viewport dims (used in 0x4b4f41..f7c: passed as (w,h) to layout calc 0x4b2f30 with a {3, rects…} struct) |
| +0x2C..+0x38 | four rect/clip ints (assembled into the v13 struct, count=3 at v13[0]) |
| +0xA0/+0xA4 | **RbTree set of watchers/registrations** — `meth_0x4b4f00` inserts its arg into it (0x4b4f95..0x4b4fb9 via sub_433760 = set-insert) |
| +0xB8 | **modal/current interface chain head** (= the mis-called "byte_707F00"); chain walked by vft+152 (first-visible query) and vft+160 (next) |

methods (28 total in DB; decoded subset):
* `meth_0x4b2ab0` set-dirty; `meth_0x4b2ad0(x)`: **set/clear modal** — calls
  `current->vft[+280](x)` before storing (interface "modal changed" notification, slot +280
  = 0x118 on the interface base).
* `meth_0x4b4f00(newModal)`: walk chain from +0xB8 via +152/+160; if first visible ≠ new →
  clear modal (via 0x4b2ad0(0)); then viewport layout push (0x4b2f30) + register arg into
  the +0xA0 set. Called with the Mainmenu base by GameInterfaces virt +0x08 (session 27 §1) —
  so that virt = **root-bootstrap(newModal=Mainmenu)**.

⇒ the "focus" model is: interfaces chain off `root+0xB8` (+152/+160 traversal), one is modal
(notified via its vft+280), dirty-flag pulses per SetActive, watchers set at +0xA0.

## 3. Interface-base virtuals implied (for JE::Interfaces / iface classes)
| iface vft slot | role |
|---|---|
| +0x98 (152 dec? — the walk calls are [vft+152]/[vft+160] BYTES) | "first visible child" query (non-null stops walk) |
| +0xA0 | "next sibling" walker |
| +0x118 (280) | OnModalChanged(bool) notification |
| +0x?? = 0x495220's owner | SetActive(byte) at iface+0x20(+32) |
(vft byte offsets — decompile said +152/+160 as raw call displacements ✓)

## 4. Practical for the port (A/B)
- One root object to model (188 B, ~5 fields used), not a global sprawl.
- Interface activation ordering: SetActive → dirty pulse; head-of-chain special-cased
  (modal reset path) — match it or dialogue-focus behavior diverges.
- The +0xA0 set = "interested parties" registrations — find its ITERATION site to learn what
  watchers do (next cheap check: reads of mbr_0xa4 in this class: methods 0x4b2c10/0x4b2d40
  etc. — unexplored yet).

## 5. Open
- meth_0x4b2f30 (viewport/clip calc) & 0x4b2fe0/0x4b3000/0x4b3040 bodies (small trio).
- +0xA0 set consumers (what watches watchers).
- Who else calls 0x4b2ad0 / 0x4b4f00 (demand paths for CSMenu/dialogue presumably clear/restore
  modal around GameClient::slot43/48 usage).
- The remaining late-slot mystery (GameInterfaces mbr_0x14/2C/30/34/3C) is INDEPENDENT of the
  root mechanism — still open from session 28.

## 6. Evidence
Root ctor? (0x4b28a0 meth = ctor candidate). Set_mbr_0x4 0x4b2ab0; modal-setter 0x4b2ad3..
0x4b2af6 (vft+280 notify 0x4b2af0); bootstrap/insert 0x4b4f08..0x4b4fbb (walk 0x4b4f16..
0x4b4f2e, layout 0x4b4f7c, set-insert 0x4b4fa3). SetActive call site 0x49522f/0x495284.
Global identity proof: 0x707E48+0xB8=0x707F00 arithmetic.

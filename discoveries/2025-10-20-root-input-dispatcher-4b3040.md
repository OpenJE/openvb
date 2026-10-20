# 2025-10-20 — Session 32: root.meth_0x4b3040 = MOUSE/HOVER DISPATCHER — corrects sessions 30/31 label

Target: ORIGINAL_REFERENCE (IDA). Read-only. Full branch decode of `meth_0x4b3040` (0x3E9 B)
— the pass we deferred in sessions 30/31. Result: it is NOT a layout-only pump; it is the
**per-frame input/hover distributor**, and `mbr_0x8/mbr_0xC` are a HOVER pair, not
pending/active-open. ⚠ Those two labels in the sessions 30/31 docs are corrected here (files
annotated in place).

## 1. Structure of `meth_0x4b3040(root, src{4 ints}, extra)` @0x4b3040

```
0. RE-ENTRANCY GUARD: if root.mbr_0x28 → return 0 (set elsewhere during dispatch — this is
   the "inside UI event" flag; find its writer when wiring the port: candidates 4b2f30 legs).
1. VIEWPORT SWAP: save {+0x2C,+0x30,+0x34,+0x38}; copy from src; SCALE:
      +0x2C = src0 / *(+0x68); +0x30 = src1 / *(+0x6C)     ; device→virtual transform
   +0x34 high bits: if (src2 & 0xFFFF0000): +0x34 >>= 16; wide-flag=1   ; packed second
   message word — the `+0x34` int = BUTTON/STATE MASK (bits 1,2,4 = three "buttons")
2. HIT RESOLUTION (priority):
   a) modal chain (+0xB8): walk vft+152/160 → first-visible = hover CONTAINER (v11); no
      child resolution (v12=0) — modal grabs everything
   b) else watcher gate (+0xA8): v11 = first watcher subject (via +0xA4 set head [+8]);
      v12 = v11->meth_0x4952b0(vx, vy, 0)         ; CONTAINER hit-test at virtual coords
      if !v12: v12 = v11->Get(+0x24)->vft[+8]     ; CONTAINER's current-child record fallback
                                                  ; (⚠ units corrected by SESS-33 §2: the getter
                                                  ; returns &container+0x90; field read = +0x94)
   c) else: ITERATE the open collection rooted at +0x20/+0x24 (ring; adjacent to the +0x1C
      set — so the root has BOTH: +0x1C set "opened" and +0x20 ring "hit-testable") —
      first container whose 0x4952b0(x,y) hits wins (v11,v12)
3. HOVER STATE MACHINE (this is the 0x8/0xC pair's real role: 0x8 = hovered CONTAINER,
   0xC = hovered CHILD):
   no-modal: if v12 changed && v12: hop(code=1, v11, v12)      ; ENTER (+2d=1)
             if old 0xC && v12 != old: hop(code=3) via (0x8,0xC) ; LEAVE
             v12==0: deliver 3, clear both
   modal:    v11/v12 must match modal's own hit else deliver 3 + clear   ; modal steals hover
   promote:  0xC = v12 ; 0x8 = v11                                     ; LABEL_39
4. BUTTON EDGE DELIVERY (bits of +0x34 vs saved +0x20-mask... saved old mask = local_20):
   bit0 (mask&1):  rise → code 4 (or 7 if wide); fall → code 5, +6 first if
                   old-record-target +0x10 == current child (paired release+click)
                   store +0x10 = v12 on rise
   bit1 (&2):      rise 8/11, fall 9 (+10 if +0x14 match), store +0x14
   bit2 (&4):      rise 12/15, fall 13 (+14 if +0x18 match), store +0x18
5. VIEWPORT-CHANGE code 2 when +0x2C/+0x30 differ post-scale
6. code 16 when +0x38 nonzero (fourth src word — wheel/scroll int?)
```
hop = `meth_0x4b2f30(msg{code, x, y, mask…, extra}, container, child)`:
`container->vft[+8](child, msg)` else `child->vft[+0x11C](msg)`.

## 2. Corrected delivery-code vocabulary (confidence per row)
```
1 ENTER · 3 LEAVE           (certain from pairing/flow)
2 VIEWPORT-RESIZED          (certain: fires on scale diff)
4/8/12 PRESS-rising bits0-2 · 5/9/13 RELEASE-falling · 6/10/14 CLICK(complete pair)
                              (strong: edge + recorded-target logic)
7/11/15 = wide-flag variants (observed; input-device or alt-mode?)
16 = src-word-3 delivery (scroll? timer? — unresolved)
```
⇒ interface/widget handler contract: **vft+0x11C(msg{code,x,y,…})** on widgets directly, or
via container `vft+8(child,msg)` routing. F3 interfaces' 0x41c910-style methods and the
GUI::Fragment tree (B) sit under this.

## 3. Consequences for the port
- Root needs: re-entrancy flag (+0x28), device→virtual divisors (+0x68/+0x6C), button-mask +
  packed-high-word source, 3-tier hit priority (modal > watcher > open-ring), hover pair
  (0x8/0xC), per-bit press records (0x10/14/18), open set (0x1C) AND hit ring (0x20).
- JE::Graphics calls it per frame (sess 31) — input state arrives via the SAME call's
  src struct, i.e. **poll-driven, not callback-driven**: whoever feeds the graphics frame
  pushes current mouse x/y/mask into that struct (find producer when wiring: likely
  `JE::Input`-side copy before 0x581230).
- Codes 6/10/14 (click-complete) explain widget "activation" (Label close-click session 31:
  a click on a Label → release pair → Close(x)).

## 4. Open
- Writer of +0x28 re-entrancy flag (dispatch-time set?) — grep pass.
- code 16 semantics; wide-flag (param_1a) meaning.
- +0x1C set vs +0x20 ring relationship (who removes from either on hide) — Close(x) didn't
  touch them visibly; likely inside the iface-side +0x11C handler or 0x40e780-adjacent.
- root +0x44..+0x64 band still unmapped (dims 0x68/0x6C now named as divisors).

## 5. Evidence
0x4b3040: guard 0x4b3046; swap/scale 0x4b3066..0x4b30c0; high-word unpack 0x4b30c8..0x4b30d2;
hit tiers 0x4b3101..0x4b31ab; hover machine 0x4b31c4..0x4b3268 (promote 0x4b3265/68);
edge blocks 0x4b326b..0x4b33d3 (codes at 0x4b32d9/0x4b32e3/0x4b32f0/0x4b3308 etc.);
viewport code 2 0x4b33e8..0x4b33fb; code 16 0x4b3400..0x4b3418.

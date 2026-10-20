# 2025-10-20 — Session 33: hit-test body + message struct pinned (and a types warning for the corridor)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Small pass closing session 32 §-items reachable
statically; adds the widget-contract slot and the exact msg struct.

## 1. Hit-test `meth_0x4952b0(container, x, y, z?)` — body (0x4952b0..0x4952ff)

```
if (container+0x80 flag)  → return 0          ; ⚠ "inactive/in-modal" gate — see §3 types
ring = *(container+0x40)                      ; child ring head (nodes: +0 next, +8 child)
if ring empty → 0
for node in ring: r = child->vft[+188](x, y, z?)   ; widget point hit-test, returns hit-id/0
                  first non-zero wins
```
⇒ **widget contract gains slot `vft+188 (0xBC)` = PointHit(x,y,extra)**. Together with the
session-32 contract set (+8/+0x11C/+0x98/+0xA0/+0x118/+84) this is the full observed
iface/widget surface. Hit ids (non-zero `v12` values) flow straight into the hover pair.

## 2. The delivery message struct (from 0x4b3040's v36 assembly — exact)
```
msg[0] = code (1..16 vocab, sess-32 §2)
msg[1] = virtual X  (src.x / root+0x68 divisor)
msg[2] = virtual Y  (src.y / root+0x6C)
msg[3] = button/state mask (root+0x34, low bits 1/2/4; high word unpacked at 0x4b30c8)
msg[4] = src word 3 (the '16'-delivery payload)
msg[5] = extra param passed into 0x4b3040 (0 at all observed call sites; frame-id?)
```
call signatures (session 32 §1 hop, now precise):
```
container->vft[+8](container, child, &msg)     ; container routing (may consume/redirect)
else child->vft[+0x11C](child, &msg)           ; widget event handler
```
`meth_0x495290` = `return container + 0x90` (address-of, not value) — the tier-b fallback in
0x4b3040 read `*(that + 4)` ⇒ field **container+0x94** = "current child record"; its [0] =
vft, deref → `vft+8` = same routing leg as the hop. (Session 32 §1b wording "Get(+0x24)->
vft+8" — units corrected: the 4-based member math lands at +0x90/+0x94, not +0x24.)

## 3. ⚠ Types warning (this corridor is IDA-typing soup — port from EAs + this sheet, not names)
The same offsets are typed `_DWORD*`(+4-units) in some bodies, `char*`(+1) in others, and three
different "interface base" candidates exist (cls_0x495420 48 B; the CWorldContainerInterface
label on generic bodies; JE::Interfaces 4 B). Concretely: "SetActive byte @+32" (sess 27/31,
char units) vs "gate @+32" here (dword units = +0x80) are **different fields** — both readings
appear on differently-typed bodies of near-identical code. Do not merge them; if the reimpl
needs certainty on (a) the +0x20/+0x80 gate pair and (b) the 48-byte base's +0x28 child-list vs
+0x40 ring identity, take one live instance dump (breakpoint at 0x4952b0 hit with container ptr)
— 5 minutes of lockstep, and A owns that.

## 4. Status of session 32's opens
- code-16 payload = msg[4] (src word 3) — semantics still data-side unknown.
- `+0x28` re-entrancy writer: NOT found by root-ref-adjacent write scan (no hits) ⇒ set via a
  register-held root ptr — deferred to the dump in §3.
- wide-flag (msg high-word) meaning — deferred likewise.
- 0x1C-set vs 0x20-ring sync: Close(x) (sess 31) didn't touch them; likely the iface-side
  handler (widget vft+0x11C) or destroy paths — chase when a specific interface is ported.

## 5. Evidence
0x4952b0 (gate 0x4952b1? — `if (*(this+32))` @0x4952b1 in IDA's dword units = +0x80; ring
0x4952c0..0x4952d0; walk+call 0x4952e8); 0x495290 (return this+36·4 @0x495293); msg struct
fields from 0x4b3040 assembly EAs (sess 32 §5) — v36[1]/[2] at 0x4b30ed/0x4b30fb, [3] mask
0x4b30db, [4] 0x4b30fb, [5] 0x4b30e3.

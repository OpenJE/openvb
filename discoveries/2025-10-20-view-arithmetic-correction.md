# 2025-10-20 — View-arithmetic correction to PackState field list; tick fields settled (session 25)

Target: ORIGINAL_REFERENCE. Read-only. Fixes a systematic offset error in my own sessions
15/24 and settles the +0x44/+0x48 question. Lesson logged in conventions: **always confirm
which vft-view `this` is before quoting field offsets from a decompile.**

## 1. PackState `this` = the IWorldServer VIEW (world base + 4), not the base

PackState (0x545930) lives at slot 11 of **0x66856C** — the vftptr stored at World+0x04, and
`GetWorldServerInstance` hands out `global+1` (= base+4). `__thiscall` ecx at the call is
therefore **base+4**, so every field it "reads at +N" is really world-base **+N+4**… with the
corollary that its mode check `cmp [ebp+0x34],3` tests **world base+0x38**, and its meta-int
stream fields are base-relative:
```
PackState reads (as decompiled)   real world-base fields
 +0x44, +0x48                     → +0x48, +0x4C   (tick lastDt + neighbor)
 +0x6C, +0x70, +0x74, +0x7C,      → +0x70, +0x74, +0x78, +0x80,
 +0x80, +0x84                        +0x84, +0x88   (clock/accum/id-base/counter block)
```
⇒ session 15 §1-A's "+0x44…+0x84" list and session 24's "tick and pack disagree" puzzle are
both artifacts of this shift. Under base-corrected arithmetic, PackState's first two ints are
the tick's OWN lastDt(+0x48)/carry neighborhood — i.e. **pack ships the tick clock phase**,
and there is no missing read-path (my "read elsewhere" claim is RETRACTED — the +0x44/48
reads in the disasm (0x544aa1..0x544b0f) are the tick function itself).

## 2. What is now internally consistent

- tick (0x544a90, this=BASE): state=base+0x34 {0 invalid,1..3 kinds,4 skip}; carry base+0x44;
  lastDt base+0x48; skip base+0x58; copy-out base+0x64; frames base+0x68 (+=[3]).
- InitClock (session 23): epoch Year..Second = base+0x14..0x28; SecondsPerRound base+0x40.
- PackState (this=+4 view): ships {+0x48,+0x4C,+0x70,+0x74,+0x78,+0x80,+0x84,+0x88} and
  requires view+0x34 (=base+0x38) == 3.
- UnpackState (0x547e60): its incoming `this` base is STILL unresolved (it opens with a
  vb-adjust call, ecx may be base, view, or outer). Its field reads +0x3C..+0x7C and the
  +0x2C=3 write must be re-paired once the base is known — §7 of session 24 stays OPEN
  pending the same delegated write-breakpoint run.
- id-base/id-counter: setter masks to 29 bits at (view)+0x7C ⇒ base+0x80; getter
  `mov eax,[ecx+0x88]` on ws view ⇒ base+0x8C counter (consistent with session 14 §4.1).

## 3. Rule added to discoveries/README conventions

Before quoting any decompiled field offset: state which vft/view ecx holds (base, +4 server
view, +0xC third view, vb…) and normalize to base. Several earlier tables predate this rule;
where they conflict with §1 above, THIS doc wins; affected rows in sessions 14/15/24 carry
pointer notes.

## 4. Queue

1. Session-24 §7 experiment (reimpl) — will confirm base-identity for UnpackState in one run.
2. Clip-name table (GameAnimationCache) — next static target.
3. Pack/unpack field diff: RETABULATE after §7 lands (current two "field lists" were
   base-mixed; do not port either yet).

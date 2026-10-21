# 2025-10-20 — T-010a answer: the "from-behind" test is NOT a 3π/4 sector (session 40)

Target: ORIGINAL_REFERENCE. Read-only. Research reply to the T-010a dispatch (assignee
HappyWolf/implementation side — this file is the routed-fact they asked the research lane for).
Falsifies the reimpl's substituted rule and hands over the real structure.

## 1. The guessed rule is dead on arrival

`|angle| > 3·π/4` **cannot** be the shipped rule:
- `ds:0x6cb56c` (the first compare constant, used ~10× app-wide) = **0.0f**, not π.
- The three "sector" constants are **360.0, 180.0, −180.0 DEGREES** (raw-read floats at
  0x660820/0x660824/0x660828) — full-circle wrap values.
- There is no 2.356 (3π/4) or 135.0 constant at any of these sites. The guess has no
  numeric source; delete it.

## 2. Actual shipped structure — `GameEffectDamage::Apply` @0x45c240..0x45c2e4 (raw)

```
inputs : three angles (esp+0x38/0x3c/0x40 slots): negated Δeuler components ×0.0174533
         (π/180 → radians) @0x45c251/261/271
helpers: call 0x47272B (out ptr edx=esp+0x70)  ; builds the target-vs-caster transform
         call 0x471357 (3 out ptrs esp+0x64, +0x3c, +0x40 region) ; reduces it to (X,Y)
angle  : fld[esp+0x40]; fld[esp+0x38]; fpatan      ; atan2(Y, X)   @0x45c294-29c
test chain (x87, result = BYTE flag at [esp+0xF]):
   fcom   0.0      @0x45c29e  ; sign test
   fcom  −180.0    @0x45c2ab  ; only on one branch
   fcom   180.0    @0x45c2b8
   fcomp  360.0    @0x45c2c5
   ⇒ flag := 1 on THREE paths (0x45c2b6-je-target, 0x45c2d2, 0x45c2db);
     flag stays 0 otherwise. Provable reading: the chain is a RANGE test
     "is angle ∈ (−180, 180] (mod 360)" implemented via wrap comparisons — i.e.
     signed-angle OUT-OF-RANGE ⇒ flag=1. (The radians-vs-degrees ambiguity of the
     fpatan result is the one open sub-question — see §3.)
consumer: flag feeds the post-@0x45c2e4 block: target vft[+80] gate, then the
     damage-type switch @0x45c425 (effect+0xA8) whose per-case second-roll sites
     0x45c44f/0x45c4a5/... read [esp+0x27] — the byte whose window overlaps the
     flag slot region — before hurt-message ids (canonical-adjacent: sess 22 §2b,
     sess 30 §1).
```

## 3. What this settles vs. what remains

- SETTLED: constant values; the compare-chain SHAPE (0/−180/180/360 in that order,
  byte-flag output); fpatan-based angle derivation; consumer gate order. The 33/34-vs-37/
  38/39 pick is driven by THIS flag + the damage-type switch — not a threshold on the raw
  angle.
- REMAINING (implementation-side or one more research hour): whether 0x471357's outputs are
  already degrees (making the ±180/360 compares meaningful range checks — most likely, given
  the π/180 pre-scale and that comparing radians to 180 would be vacuous) — verifying
  0x471357/0x47272B as Euler-triangle→angle helpers answers it. If degrees: the rule is
  exactly `flag = (angle ≤ −180 || angle > 180)` after atan2 — a **wrap detector**, which in
  gameplay terms is the classic "target's facing vs incoming direction sign flip", NOT a
  "behind arc". A behind-arc model would need a per-attack threshold; none exists.
- The reimpl's anim-pair selection should therefore be re-derived from the flag consumer
  (byte at esp+0xF → which of the hurt-message ids), NOT from any angle threshold.

## 4. For the coordinator
This row's premise ("from-behind sector floats") was itself the guess — the floats are a
wrap/range test. Flagged per standing rule 3 rather than coded around. HappyWolf's next step
is the consumer trace in Apply's tail, which they own; the numbers here are final.

## 5. CONSUMER FOUND (sess 41, Apply tail @0x45c30c–0x45c358 raw)

```
0x45c30c: cmp BYTE [eff+0xAD], 0 ; !=0 → 0x45c35f (other selector)
0x45c326: dl = [esp+0xF] (WRAP FLAG) ; al = (flag!=0) ; eax = al + 0x21(33)
          ⇒ anim = 33 (angle in (−180,180])  or 34 (out of range)
0x45c336: cmp BYTE [eff+0xAC], 0 ; ==0 → flag?0:-1 &3 + 0x25
          ⇒ anim = 37 + (flag ? 0 : {0..3 from -flag&3})  → 37/38/39(/40)
```
So BOTH candidate pairs are selected by: the wrap flag × two EFFECT BYTES +0xAC/+0xAD —
which are, per session 21 §7, the **drain-time AoE markers** (`v11[172]=flag`, `v11[173]=1`
in the attack finisher). ⇒ "33/34 vs 37/38/39" is (angle-wrap) × (last-target / multi-target)
— a positional-arc test composed with target ORDER. HappyWolf's re-derivation: key the
selector off [eff+0xAC]/[eff+0xAD] first, wrap flag second; no threshold anywhere.
 eff+0xAC = "is-last-target-of-action" (drain sets byte172=local flag when action==target's
 owner chain), eff+0xAD = "multi-target/AoE" (drain sets 1 for non-final hits).
Death-thread note (coordinator caveat): this window contains NO state/ChangeState call —
model gate vft+0x50 + engine op 0x548f90 only; does not conflict with the pinned OnDeath 6→7.

## 6. GW+0x40 write census (routed question from coordinator, sess 41)
- slot0 (0x436240) = `mov eax,[ecx+0x3c]; ret` on the +4 ws view ⇒ **GW+0x40** ✓ both sides.
- Whole-world-range write census: **exactly two writers of GW+0x40** — World ctor
  (0x548b23, =6) and InitClock (0x546b5b default 6 / 0x546c8f = atol("Seconds Per Round")).
  NO continuous writer. (Other grep hits were esp-stack locals.)
- Consumer @0x5a95ac semantics: per-DISPATCHED-EVENT loop (0x5a9530..4f iterates a queue;
  per item: `acc(+0x10) += 1000×GW+0x40`) — i.e. one ROUND advance per processed item, not
  per frame. Alternate mode: when owner byte [+0x319] set (0x5a955d/0x5a9567), the event's
  explicit time param is stored (→ +0x29C) and the round clock is NOT consulted.
⇒ Coordinator's "SecondsPerRound" label STANDS; my slot0 claim STANDS; T-011 must treat
+0x10 as a round-based virtual clock (ms domain = N rounds × SecondsPerRound×1000), never
mix it with real-ms action deadlines unless the +0x319 selector says explicit-real-time.

## 5b. CORRECTION to §5 (sess 42, per FastArrow's `2025-10-20-aoe-reaction-flags-ac-ad-answer.md`
##     + coordinator ruling — the §5 READING of the bytes was wrong; selector arithmetic stands)
- WRONG in §5: calling eff+0xAC/+0xAD "session-21 drain AoE markers (last-target/multi-target)"
  and deriving "(wrap-flag × target-order)". They ARE written by the same finisher
  (meth_0x403770 @0x4038D7–F7, decimal 172/173 — my §7 read and FastArrow's agree on the SITE),
  but they are **volley-batch reaction-control flags**, not target-order ones:
  * first record of the pending-hit list: +0xAC = (≥2 volley-marked records), +0xAD = 0
  * every non-first record: +0xAD = 1 → Apply @0x45C30C jumps PAST the whole hit-reaction
    block ⇒ secondary hits in a volley are deliberately silent (animate once per volley).
- Selector pairs therefore: stationary branch @0x45C336 is +0xAC-gated {37,38}/{38,39}
  (clear/set) — NOT `37 + (−flag&3) → {37..40}` (that formula was my mis-framing of the same
  bytes; retracted).
- The WRAP FLAG finding stands unchanged: 4-compare chain vs {0,±180,360}, radians ⇒ the
  ±180/360 compares are unreachable (degree-read retracted as a live path by the
  coordinator's ruling: radians literal, degrees kept as comment only).
- **[§5c]** The 40/41-vs-death-anim boundary question from the T-020 row is **VOID**: shipped
  `QueueAnimEvent_0x4673D0` groups reactions {33,34,37,38,40,41} → `meth_0x464D70` and
  death/terminal {43..52} → `meth_0x464940` behind the `mbr_0x9c` latch (set @0x4674C9, blocks later
  re-queues). There is no 0x28-base death group, so no reaction id can conflate with a death id; id
  39 IS consumable by the queue (case 19/39) but the Apply site never emits it.
- FIDELITY NOTE for the reimpl port: first-record-silent-rest pattern is a behavioural gap,
  observable as multi-sub-attack unarmed hits animating once, not N times.

## 5c. T-020 THIRD-SOURCE RE-READ (PureKnight 01a15cb7-e27e) — verdict: **confirms Implementer-C**, and breaks §5b's "{37,38}/{38,39}"
Independence caveat stated up front: I read the T-020 row **including the author's pointer section**
before my first byte dump (the BLIND GUARD commit 28489e4 arrived after), so this is not a fully
blind third source. To keep it load-bearing anyway: I dumped raw bytes from `$OPENVB_ORIG_EXE` myself,
disassembled with `objdump -M intel` (independent of IDA), and derived the set and the reaction-vs-death
boundary from the consumer window before comparing with any prior wording.

Bytes actually read (0x45C30C..0x45C359, raw file at `.text` offset map, byte-for-byte):
```
38 9e ad 00 00 00   cmp  byte [esi+0ADh], bl
75 4b               jne  0x45c35f                      ; non-first volley records: no reaction anim
8b 4e 48 / 8b 41 04 / 8b 50 04                         ; ecx=[esi+48], eax=[ecx+4], edx=[eax+4]
38 9c 0a a6 03 00 00   cmp byte [edx+ecx+3A6h], bl
74 10               je   0x45c336
8a 54 24 0f         mov  dl, [esp+0Fh]                 ; WRAP FLAG byte
33 c0 / 3a d3 / 0f 95 c0   xor eax,eax; cmp dl,bl; setne al
83 c0 21            add  eax, 21h  ; base 33, mask +1  ⇒ {33,34}
eb 22               jmp  0x45c358
38 9e ac 00 00 00   cmp  byte [esi+0ACh], bl
8a 44 24 0f         mov  al, [esp+0Fh]
74 0c               je   0x45c34e
f6 d8 / 1b c0 / 83 e0 03   neg al; sbb eax,eax; and eax,3   ; 0 or 3 from (wrap!=0)
83 c0 26            add  eax, 26h  ; base 38           ⇒ {38,41}
eb 0a               jmp  0x45c358
f6 d8 / 1b c0 / 83 e0 03   (0x45c34e: same mask)
83 c0 25            add  eax, 25h  ; base 37           ⇒ {37,40}
53 50 … call 0x4673d0 (GameEntity::QueueAnimEvent)
```
- Reachable set from this selector: **{33,34} ∪ {37,38,40,41}**; **39 unreachable** here.
  `and eax,3` after `neg/sbb` is a boolean mask yielding 0-or-3 — confirmed byte-for-byte,
  i.e. Implementer-C's encoding note `f6d8 1bc0 83e003 83c025 / 83c026` is exactly what the file holds.
- §5 original ("37/38/39(/40)") and §5b correction ("+0xAC-gated {37,38}/{38,39} (clear/set)") are BOTH
  wrong as *sets*: +0xAC selects the **base** (37 vs 38); the wrap flag selects **+0 vs +3**. §5b's pair
  reading implies mask=±1 behaviour that these bytes do not have. Note the 33/34 branch legitimately uses
  a +1 mask (`setne`+`add eax,21h`) — the two masks differ by branch; that is shipped behaviour, not a misread.
- Reaction-vs-death boundary, from MY read of `QueueAnimEvent_0x4673d0`: cases **33,34,37,38,40,41 all
  dispatch together to `meth_0x464d70`** (hit-reaction group). Death/terminal group is **43..52**
  (sets the `mbr_0x9c=1` latch that also blocks any later re-queue, then `meth_0x464940`).
  There is **no death base at 0x28(40)** — 40/41 cannot conflate with a death id; 39 is consumed
  (case 19/39 → `meth_0x464b80(1)`) but simply never emitted by this window. The T-020 parenthetical
  premise ("death-anim base of 0x28(40) in the 43..52 set") is self-inconsistent and should die.
- §5b's wrap-flag and volley-silencing ([+0AD]!=0 → jump past block) findings stand; this section only
  corrects the reachable-id set, per Implementer-C's derivation.

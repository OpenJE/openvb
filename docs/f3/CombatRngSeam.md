# The shipped random draw, and every place it is used

T-027. Everything here is read out of the shipped binary (`F3.exe`, IDA original-F3 DB); nothing is
inferred from our port. Cited by EA because the DB's own names are the only stable handle.

## 1. The generator: `sub_48CCC0` (0x48CCC0..0x48CCF6) — not `rand()`

Disassembly, verbatim:

```
48ccc0  mov  al, byte_707C64
48ccc5  test al, al
48ccc7  jnz  short loc_48CCD9          ; already seeded?
48ccc9  push 0 ; Time
48cccb  call __time32                  ; wall clock, SECONDS
48ccd0  push eax
48ccd1  call F3::SetStartupTime        ; 0x48CCA0
48ccd6  add  esp, 8
48ccd9  mov  eax, dword ptr F3__start_time
48ccde  imul eax, 19660Dh              ; * 1664525
48cce4  add  eax, 3C6EF35Fh            ; + 1013904223
48cce9  mov  dword ptr F3__start_time, eax
48ccee  shr  eax, 8
48ccf1  and  eax, 7FFFh                ; -> 0..32767
48ccf6  retn
```

`F3::SetStartupTime` (0x48CCA0) is two stores and a return:

```
start_time = time32;   byte_707C64 = 1;   return time32;
```

So the mechanism is:

| fact | value | evidence |
|---|---|---|
| algorithm | 32-bit LCG, `x = 1664525*x + 1013904223` (the Knuth/MMIX constants) | 0x48ccde / 0x48cce4 |
| state | ONE global dword, `F3::start_time` — the same variable the engine calls the startup time | 0x48ccd9, 0x48cce9 |
| seed | `time32(NULL)` (whole seconds), taken lazily on the **first draw**, not at startup | 0x48cccb, guarded by `byte_707C64` |
| emit | `(state >> 8) & 0x7FFF` → **0..32767**, and the advance happens *before* the emit | 0x48ccee..0x48ccf1 |
| reseed | none. There is no other writer of the state and no reset path | xref set of `SetStartupTime` |

Two consequences that matter for a reimplementation:

1. **The stream is a pure function of one integer.** Given the seed, every dice outcome in the whole
   session is determined. That is why T-026 digests are meaningful at all — and why a test that does
   not pin the seed asserts nothing.
2. **The low 8 bits are discarded** on output, so `draw % 101` (the to-hit roll, §3) is *not* uniform:
   32768 values over 101 buckets leaves a residue of 104, and the discarded bits are exactly the ones a
   uniform mapping would want. The original has this bias. Mirror it; do not "fix" it.

`std::rand()` returns 0..32767 too — which is why the old fallback in our port passed every range
check while producing a completely different sequence.

## 2. The dice wrapper: `sub_48CD00` (0x48CD00..0x48CD2E)

`sub_48CD00(a, b)`: if `a == b` return `a`; swap so `v3 = min`, `v2 = max`; return
`min + sub_48CCC0() % (max - min + 1)`. Our port calls this `F3::CombatDice` (`src/F3/CombatMath.cpp`).
There is a third consumer of the raw generator, `sub_48CD30` (0x48CD30, size 0x65) — **purpose not yet
identified**, and it is the only remaining caller that does not go through the wrapper.

`JE::RandomNumber` (calls `sub_48CD00` at 0x5c389a) is the shipped *public* accessor for a draw. So a
port that exposes one injectable seam is not inventing a hook — the original already has a named entry
point for "give me a number", and everything else is internal.

## 3. The draw surface, enumerated from xrefs to `sub_48CD00` (18 direct sites)

Cross-references are the authority here, not grep of our tree: this is who draws in the shipped game.

| caller | EA(s) of the draw | what it is | ported? |
|---|---|---|---|
| `sub_401280` | 0x401285 | unidentified (small, 0x3e) | no |
| `GameActionAttack::meth_0x402260` | 0x4022f4 | **to-hit roll** `Dice(0,100)` | yes — `src/F3/GameActionAttack.cpp:163` |
| `GameCreature::CalculateUnarmedDamage_409f90` | 0x40a00e, 0x40a0ae | unarmed: raw roll + DT roll | yes — `CalculateUnarmedDamage` |
| `GameWeapon::CalculateDamage_413310` | 0x4133f1, 0x413462 | weapon: raw roll + DT roll | yes — `CalculateDamage` |
| `sub_45C140` | 0x45c442, 45c45c, 45c473, 45c487, 0x45c49d, 0x45c4b3, 0x45c4c9, 0x45c4e6 | **eight** draws in one function — the reaction/presentation chain that ledger entry 5 (`var_B1` stack residue) also lives in | no |
| `GameEffect::virt_meth_0x462710` | 0x462722 | effect roll | no |
| `GameEffect::virt_meth_0x462740` | 0x462768 | effect roll (bigger method, 0x103) | no |
| `sub_467640` | 0x4676d8 | unidentified | no |
| `JE::DialogueWindow::virt_meth_0x595010` | 0x5954fe | **dialogue** draws a number | no |
| `JE::RandomNumber` | 0x5c389a | the public wrapper itself | n/a |

Notes worth carrying:

- "crit chance" and "scatter" are **not separate generators**; they are among these `sub_48CD00` calls.
  Anyone looking for a per-system RNG will not find one — there is one stream, 18 consumers.
- The eight draws in `sub_45C140` are the single biggest consumer block. Because the stream is global and
  sequential, **the reaction selection consumes draw #n**, so any per-frame digest that excludes the
  reaction-anim id while including damage is still coupled: a change in damage-draw *count* shifts the
  reaction outcome. That is the argument for putting the draw **count** (or the state dword) into the
  digest, not just the derived values. (T-026's exclusion decision should read this line.)
- `JE::DialogueWindow` drawing means randomised dialogue branches exist — relevant to whoever ports
  dialogue replay determinism.

## 4. The seam as landed (and what it costs us)

`src/F3/CombatMath.{hpp,cpp}`:

- `int ShippedRand()` — `sub_48CCC0` verbatim, including the lazy `time()` seed and the one-time flag.
- `void SeedShippedRand(unsigned)` — mirrors `F3::SetStartupTime`'s store; **test-only**, registered as a
  deviation in `docs/DEFERRED-NOTES.md`.
- `CombatDice(a, b, rnd)` — `rnd == NULL` now takes from `ShippedRand()` (previously `rand()`, a silent
  fidelity deviation). A non-NULL `rnd` is the per-call injection the tests use.

## 5. The rule

**No test may assert a damage value (or any drawn value) while the draw comes from the live global
stream.** Pin it first — either `SeedShippedRand(k)` or an injected `RandFn` — and assert the exact
number. A range assertion (`>= min && <= max`) on an unseeded stream proves only that modulo works;
that is how two lanes signed off on a test that asserted nothing. The sensitivity control is equally
mandatory: assert that a *different* seed gives a *different* number, otherwise the exact-value check may
be passing on a formula that ignores the draw entirely (`CombatMath.cpp` carries that control as
"assert is sensitive, not vacuous").

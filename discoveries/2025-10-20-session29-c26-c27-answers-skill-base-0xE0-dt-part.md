# Session 29 — answers to implementer asks C-26/C-27 (chat round): base-skill array is +0xE0 (B's C-32 off by 8), DT-row selector = PART (B right, A's dual-role note stands), zero-on-9 asymmetry accepted (A right)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Per chat protocol v2 the discovery agent writes only under `discoveries/` — this is the
official answer entry for implementer-A C-26/C-27 and implementer-B C-32/C-33 threads.

## 1. A's C-26 ask 1 (skill enum values) — B answered (C-32); ONE correction

B's tables are right EXCEPT the base array offset. Raw @0x409824:
`mov eax, [ecx + eax*4 + 0E0h]` — the decompiler's `this->gap8[4i+216]` display =
8 + 216 + 4i → **base-skill array = GameCreature+0xE0, int[13]**, not +0xD8.
(216 is the gap8-relative index; forgetting the +8 member offset gives 0xD8 — that's the trap.
The 0xD8 area is instead the tail of the GCRE 7-stat block per the GCRE docs — indexing skills
at +0xD8 will read stats as skill 0/1.) Session 17 §9 already had 0xE0; now raw-locked.

## 2. C-26 ask 2 / C-27 leftover (called-shot dword_64F858 index → body part)

Table = [10,40,40,40,40,60,120,60] (idx 0..7; idx8 = FLT_MAX sentinel). Semantic mapping to
the canonical 9-part list: 0=90 (unarmed target check uses part==4 for +MELEE_DAMAGE and the
dual-row rule "5,6 → other row") points at the F2 `bodypart_e` order:
0 head(→120? no…) — see caution below. Honest answer: **not statically resolvable** — no
consumer cross-checks part-name↔index; only two anchors exist:
- part 9 = "unset" (guard values), 0..7 index the table and the DR rows (`4·part`);
- the DR row switch treats {5,6} as a distinct anatomy group ⇒ 5/6 are the two most exposed
  limbs *(inferred: legs by F2 lineage)*; and the `part==4 → +MELEE_DAMAGE` unarmed special
  suggests 4 = arm/hand. **VOID — coordinator ruling 2026-10-20: 0x409ffd's `param_1[4]` is the
  MODE DAMAGE TYPE index (same operand B's C-33 used for the in-row DT index 0..5), not a body
  part. No part-anatomy corroboration survives from static RE; T-010c stands as its own task.**
Recommendation: B's proposed mapping from the hurt-message ids is as good as static RE yields;
settle definitively from `items.met`/GCRE per-part DR array lengths (6 rows × parts…) or one
debugger hit-test table dump. Session 6 §5 now records the raw table + these anchors only.

## 3. C-33 (B): DT-row selector = PART — ACCEPTED, and A's model keeps its second consumer

B's read of 0x413310's switch is correct (patched into session 6 §5 with raw EAs; the
default case computing a value, not returning, is noted). One reconciliation for both:
`param_5` doubles as the DR-ROW selector here, while the same value's `param_1[4]` (the
switch's `4·X` index — ammo/weapon DT index 0..5) selects WITHIN the row — so A's
"type-index-inside-row" structure survives; only the row-selector's *name* changes from
style to part. A's §3 correction about the two zero-on-9 sites (weapon unconditional
@0x41333d; unarmed conditional `&& !usable` @0x409fec) is ACCEPTED into session 6 §5 —
the sites genuinely differ; my session 6 wording conflated them (fixed).

## 4. C-27 (A): hit-test assumption — RESOLVED by session 17 (already public)

`meth_0x402260` = chance (weapon/unarmed fork @+0x71) → `RandRange(0,100)` **inclusive, 101
outcomes** @0x4022f4 → `chance >= roll` @0x402393, plus the on-screen
`"Hit/Miss: Chance = %d, Random = %d"` readout @0x40236f (diff-able in lockstep). B's C-33 §1
matches; use that range or boundary rolls diverge (A already noted).

## 5. Doc changes this round

- session 6 §5: DT-row selector → part, raw EAs, default-falls-through, zero-on-9 asymmetry;
- session 17 §4/§9: roll op + range emphasis; skill base +0xE0 raw-locked; B-offset trap note;
- this file = the chat-visible answer record.

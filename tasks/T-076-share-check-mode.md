---
status: in-progress
assignee: EpicRaven (T-076 filed by LoudDragon `01a15f82` in bab1146/69a5792, dispatched on return 2026-10-21)
type: tooling (offline; tools/ecx-flow.py extension; NO IDB writes; README table untouched per coordinator-only law)
depends: T-054 (the tool), harness item 44 (rule-1 source), T-064 [B] falsification (the test-case origin)
---

# T-076 — `share-check` mode: item-44 rule-1 as an executable pre-queue gate

## Objective
Turn "I should have run it before queuing [B]" into infrastructure: a mode that computes the
item-44 rule-1 machine test from the md5-pinned edges and returns a QUEUE-BLOCKING verdict, so no
future wave needs the lesson again.

## Contract
- Input: candidate EA set (`--addrs` / `--cls`).
- Test (per candidate, external edges only, intra-set edges excluded):
  - **view constants** = distinct adjusted-this shapes (`this = base + d` / `this = &G + d`);
  - **holder fields** = distinct field-access paths (`this = *[chain]`);
  - `>=2 distinct view constants` OR `>=2 distinct holder fields` ⇒ **SHARING** (block);
  - no external edges at all ⇒ **NO-EVIDENCE** (block — nothing supports a membership mint);
  - else **SINGLE-OWNER** with the constants/holders/plain-call census printed (pass).
- Exit code: 0 = all candidates pass; 1 = any BLOCK (usable directly in wave scripts).
- Gated: selftest must be green (now 6/6 — T5 bakes the [B] known-SHARING cases and the
  445350/507810 known-PASS cases as regression) before any share-check verdict prints.

## Acceptance
- Mode committed in tools/ecx-flow.py; T5 gate rows reproduce the banked rulings
  (0x434250, 0x434120 SHARING; 445350-band, 0x507810 PASS); offline-only, cache discipline
  unchanged; status relayed to coordinator for the table mirror (new law); discoveries note only
  if a wave-relevant surprise surfaces (none expected — this is formalization, not new evidence).

## Notes
- FYIs acknowledged on receipt: T-067 DXVK wall broken (live work will flow), T-071 STATE-agrees,
  T-050 closed-by-ruling on this lane's statics, README/boards-table coordinator-only law honored.
- T-073 (HappyStorm running the weapons/flow census on the 17 orphans) and T-075 (TrueCastle,
  single-writer census pattern on 0x707E48) noted as tool-succession lines; arbitration pings route
  here — share-check is exactly what they should be running before their queues form.

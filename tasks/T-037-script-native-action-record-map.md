---
status: done
assignee: WildYak
depends: T-030 (seam ported), T-033 (class identity helps but is not required)
---

> **Closed 2026-10-21 (statics half + driver proposal adjudicated by RedXenon at `9f9ab24`).**
> Findings: `docs/f3/ScriptNativeActionDoors.md`. Sizes `0xC8`/`0xC8`/`0x24`/`0x3C` confirmed against
> `F3.json`; argument-derived stores **1 / 3 / 2 / 0** — the ctor is the record. `+0x14` from statics:
> no native and no ctor writes it; `0x550737` is the only writer on this path.
> Driver acceptance is met **as far as driver scope reaches** (bare `attack` now passes `mode = NULL` and
> `nowMs = 0`; the 6-arg form announces its injection). Three things it could not fix are port-side and
> are listed as follow-ups below, not as this row's gaps.

> **Statics half delivered 2026-10-21 → `docs/f3/ScriptNativeActionDoors.md`.** All four natives recovered from the
> read-only IDB: sizes `0xC8`/`0xC8`/`0x24`/`0x3C`; argument-derived stores are **1** (`Attack`: `+0x38` = the
> target's `Entity+0x1DC` ref key), **3** (`JustAttack`: `+0x6C=0`, `+0x6F=0`, then the same `+0x38`), **2**
> (`StartDialogue`: `+0x1C`, `+0x19=0` — plus a pre-enqueue flush of `Entity+0x304` via `0x54c440`), **0**
> (`ReloadWeapon`). Acceptance item 2 remains open: the `attack` verb change is a **proposal in RedXenon's hands**
> (they hold `tools/sim-driver.cpp`), and item 3's live half is NiceHawk's T-034/R4.

# T-037 — Second input door: what the script natives push into the Action record

## Why
`0x550720` is the single gameplay-order funnel and our seam is ported, but the driver reaches it the way *we* chose,
not the way the shipped script natives do. The remaining doors named in T-030's verdict are the **script natives**
(`F3::Attack` `0x459f5d`, `JustAttack` `0x45a025`, `StartDialogue` `0x45a0f0`, `ReloadWeapon` `0x45a3e6`/`0x45a3fb`) and
the **server packet path** (`GameServer::virt_meth_0x46dd70`, 4 sites). Until the native-side parameter map exists, we
cannot say a scripted order is the same order AMX would issue — and AMX is a large unported system.

## What
For each native, from statics (no debugger unless a field is genuinely unreachable):

**Entry-point correction (WildYak, 2026-10-21): the offline class DB cannot answer for these natives.** They are free
functions (`F3::Attack`, `F3::JustAttack`, `F3::StartDialogue`, `F3::ReloadWeapon_45a380`), not class methods, so
`f3_address_lookup 0x459f5d` returns **0 matches** — my original hint told the next reader to start with a tool that
cannot work. Use `docs/f3/EntityActionQueueSeam.md`'s xref list, or the read-only IDB.
1. The action record it allocates (`GameActionAttack` is 0xC8 — confirm size per native) and **which offsets it writes
   before the `call 0x550720`**, with the argument index each value came from.
2. Which fields it leaves **untouched** — those are the ones our driver is currently inventing.
3. The return value and any post-enqueue write (e.g. does it set `+0x14` itself, or does the callee? T-034's R4 is
   testing the same question from the other side — coordinate, do not duplicate).

## Acceptance
- A table: native → record type → offsets written (with arg provenance) → offsets untouched → return convention.
- `tools/sim-driver.cpp` updated so at least the `attack` verb builds its record **the way the native does**, with the
  difference from the current hand-built version stated in the commit. (Talk to RedXenon before editing their file.)
- Anything you cannot determine statically goes on the routed-question list, not into code as a guess.

## Boundary
Native call sites are the *script-visible* contract; the packet path may build the same record with different fields
filled. This row answers "what does AMX's door look like", not "is the sim correct".

## Follow-ups this row created (RedXenon declined them as out of driver scope; they need an apply lane)

1. **`StartScripted(target)`** — a record-builder that touches ONLY the target slot, leaving the actor
   pre-write, the `9 -> 0` calledShotPart preflight and the `nowMs` timer stamps out of it. Today
   `GameActionAttack::Start()` still writes `actor` before the seam overwrites it.
2. **The two gate bytes** `+0x6C` (turn-to-face, read at `0x4012e6`) and `+0x6F` (approach-into-range,
   read at `0x404651`, guarding the `new(0x48) GameActionMoveTo` + `0x550720` at `0x4046a7..0x40470e`).
   **Until they exist, `JustAttack` is not expressible in this port** — the entire difference from
   `Attack` is those two bytes.
3. **Ref-key vs pointer.** Shipped persists `target = *(Entity+0x1DC)` and re-resolves it at every use
   (`0x40120b`, `0x401304`, `0x404409`, and `GameActionInitiateDialogue` does the same conversion in
   `OnCreate 0x404b99` then re-resolves each tick at `0x404c08`). The port stores a live
   `SpawnedEntity*`, so *"target despawned mid-attack"* is reachable in the original and unreachable in
   the sim. Needs either a ref-id field on `SpawnedEntity` or a numbered deviation — do not leave it
   implicit.
4. **`StartDialogue`'s pre-enqueue flush** (`0x54c440` destroys every action already queued on the source,
   walking the same `Entity+0x304` deque the funnel appends to). Any future `dialogue` verb must include
   it or it is modelling a different door.
5. **Server packet door** `GameServer::virt_meth_0x46dd70` (4 sites) — still unexamined; this row's
   boundary.

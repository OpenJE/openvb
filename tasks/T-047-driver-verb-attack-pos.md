---
status: open
assignee: unassigned (port work, tools/sim-driver.cpp; follows T-028 driver)
depends: T-044 (established the door), T-037 (the native door must NOT be used)
---

# T-047 — Driver verb `attack-pos <actor> <x> <y> <z>`: the packet-91 place-attack surface

## Established, do not re-derive (T-044, docs/f3/AttackPositionOverrideDoors.md)
- A location-targeting attack EXISTS in shipped: server packet ID 91 (0x5B) handled at
  `0x46df14` inside `F3::GameServer::virt_meth_0x46dd70` builds `GameActionAttack` from wire fields
  (attackerID, targetID, int→rec+0x78, hasPos byte, optional x/y/z floats), sets record `+0x48`
  (hasPos byte) and `+0x4C/+0x50/+0x54` (X/Y/Z) via `meth_0x401100`, and enqueues through the
  funnel `0x550720`.
- Accept guard: `attacker && (hasPos || target)` at `0x46decb` — **no target entity required**.
- Consumers prefer the override: `OnCreate_0x4011d0` @`0x4011da` (no-target fatal only when both
  absent), `OnServerUpdate_404320` @`0x404410` skips target resolution when `+0x48`, aim helpers
  `0x401315`/`0x402e84`.
- **The script-native door never reaches it**: T-037 stands — `Attack()` sets no `+0x48`. Do not
  wire this verb onto the native path; it models the PACKET surface.

## Task
Add `attack-pos <actor> <x> <y> <z>` to `tools/sim-driver.cpp`: allocate the record like the port's
existing attack verb, set the same fields the packet handler sets (hasPos + XYZ), enqueue through the
port's insert seam, and tick. Digest and `turnOrder=` must show the action on the actor exactly like
an entity-target attack except for the missing target key.

## Acceptance
- sim-driver-verify gains a case: `attack-pos` produces the action, the pump dispatches it, and the
  actor's existing target (if any) is NOT required; a negative case shows `attack` without target
  still refuses while `attack-pos` succeeds.
- run-all-tests + app-boot quoted from the row's run (gate-integrity stamp), mingw and msvc both
  green (driver is a tools/ TU; if any src/F3 header changes, MSVC is mandatory).

## Boundary
The port has no network layer; modelling packet 91 as a driver verb is the accepted representation
(T-028 precedent: the driver reaches seams the shipped code reaches from the wire). The override
COORDINATES are what parity-compares; whether the shipped AI ever SENDS packet 91 is out of scope
and unproven either way.

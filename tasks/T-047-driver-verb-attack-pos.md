---
status: done
assignee: PureKnight `01a15cb7-e27e`
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

## Result (PureKnight 01a15cb7-e27e, 2025-10-21)
Implemented. Port `GameActionAttack` gains the packet-91 group verbatim: `positionOverride`
(shipped +0x48, written by `SetPositionOverride` = meth_0x401100 equivalent) + `overridePos[3]`
(+0x4C/+0x50/+0x54). `Start()` guard now mirrors the shipped accept shape `attacker && (hasPos || target)`
(0x46decb); Update's place-branch walks to the coordinates (same step model as the targeted walk), waits
first-strike due, and WHIFFES with no damage roll — shipped victim-at-coordinate resolution is NOT RE'd,
so the port invents none (cited comment in-place; if a later lane traces the shipped occupant-damage
consumer, that parity question is a NEW row, not a silent upgrade here). Driver verb `attack-pos <actor>
<x> <y> <z>` enqueues through the same seam as `attack` (D-2 block-full refusal, owner-backlink check);
digest distinguishes ONLY by owner/target-token shape (`ak=GameActionAttack`, no target) — no new digest
field, per coordinator rule.
Verify cases 9/10: positive (enqueued+dispatched+walks, bystander untouched) AND the negative pairing
(bare `attack` with missing target still rc=1 while `attack-pos` needs no victim; malformed `attack-pos`
fails grammar). One interim FAIL reproduced the stale-digest race in my own case (old /tmp file graded
while wineserver lagged) → fixed with `rm -f` + non-empty assertion before grading; 8 consecutive green
runs after. Builds: MSVC cl.exe rc0 after one real portability find (MSVC2002 rejects braced init in the
mem-init list — moved to body), mingw green; run-all-tests 10 pass / 0 fail (PathManager 88>=80, GUILoading
150>=116, CombatMath 43>=43, app-boot exit 124 alive-past-6s exe 02:42:09), gate-integrity: table OK 9 rows,
stamp present, payload current. Files: src/F3/GameActionAttack.{hpp,cpp}, tools/sim-driver.cpp,
tools/sim-driver-verify.sh.

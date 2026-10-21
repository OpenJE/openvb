---
status: in-progress
assignee: KeenStorm (dispatched by LoudDragon `01a15f82`, 2026-10-21 — accepted offer of row (ii))
type: research (read-only on ida_original_f3; discoveries/ left DIRTY until coordinator checkpoint)
depends: T-057/T-059 (the composition proof + embed census that made this class central)
---

# T-061 — Decode the CEngineInterface virtual surface (vft 0x69A95C, all six slots)

## Objective
27+ host classes provably embed ONE `CEngineInterface` at computed offsets (T-057 composition,
T-059 cleanup mirror). Its vft 0x69A95C has 6 slots and the interface is the surface every
skin/widget/host path flows through — yet only edges have been met sideways. Complete the surface.

## Known edges at filing (coordinator-verified against the frozen export's vft dump)
- slot 0 = `0x5803c0`, slot 2 = `0x5803e0`, slot 5 = `0x580360` (export-listed; slots 1/3/4 exist by
  length-6 but unnamed in the export's sparse map — raw read all six).
- 0x580360 per GUILoadingRecon §11: writes the global that `Window::SetInterface_0x4af980` reads;
  loads `[esi+0x74]` from the stream at global `0x707DA8` — the SAME stream-global the Fragment
  slot-8 reload lane uses @0x4b1e50.
- Globals `0x707DA8` / `0x707E48` (Fragment slots 8/14 + T-055's 4b28a0 singleton purity) point back
  at this host class.

## Steps
1. F3.json first (owner + members around +0x74 + vft xrefs), then raw bytes for ALL SIX slots —
   the four not-yet-asked ones get at least a one-line classification so the surface is COMPLETE.
2. Per-slot semantics: who dispatches through it (callers via vft read of hosts' embed offsets),
   what each handler reads/writes, the +0x74 member's full writer/reader census.
3. Identify what the interface IS (engine-side session? stream/global broker?) — verdict language,
   not vibes; anything not statically resolvable stays unresolved with named risk.

## Acceptance
- `discoveries/` file (LEFT DIRTY, ping for checkpoint): six-slot table (slot EA / bytes verified /
  callers / reads-writes / one-line verdict + confidence), the +0x74 census, the host-facing meaning
  of the surface, open questions named.
- Read-only honored (no IDB writes; writer slot is CalmRaven's during this window); EA-anchored;
  F3.json names as locators only.

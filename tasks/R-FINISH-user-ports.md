---
status: seated
assignee: KeenStorm (product lane — all edits are src/ + modules/openje; slices sequenced AFTER M-001 slice 2, each independently committable)
type: product completion (finish user-started functions; every fact needed is ALREADY in discoveries/)
---

# R-FINISH — the half-written functions, closed out with today's receipts

Rule: **COMPLETABLE-NOW** items cite the discovery file that supplies the missing fact — the code edit
is product-lane work, but ZERO new research is needed for any of them. **GATED** items name their gate.
**NARRATIVE** items are retired-placeholder language (no work; reword or leave). Per item 57, each
completion also lands the function's EA citation.

## COMPLETABLE-NOW (facts in-hand; suggested order = cheapest payoff first)
1. **src/F3/GameWorld.hpp:62** — `m_worldParamsPad` "placeholder for +0x90 VFX_Parameter (layout pending)":
   layout CLOSED today by R-VEG-E (discoveries/2025-10-21-r-veg-e…: +0x20 type enum/+0x24 default bits/
   +0x28-2C-30 buffer trio; holder = RB-tree). Model the member properly.
2. **JE/VFX_Parameter.cpp:29** — "legacy placeholder fields (pre-reconstruction database record)": same
   R-VEG-E facts; sweep to the closed field roles.
3. **JE/VFX_Parameter.cpp:207** — `"use_managed_file" modeled as GetInt for now`: R-AMX native map (209-n)
   + the .veg doc §1 vocabulary (VFX_TriggeredEffect = type 14) give the real read path.
4. **JE/World.cpp:61-67** — pause-tick `clock()` placeholders "until JE::TickMilliseconds": the tick clock
   IS modeled now (T-071/T-080 pump chain; R2 clock-leap RE-CONFIRMED at 0x65AD8170) — wire the modeled
   source, cite it.
5. **JE/ActionAnim.cpp:44-47** — three commented-out stub calls: callees since RE'd by the queue atlas
   (T-051/T-074/T-085 + T-060's SendActionOn path) — restore the calls against the modeled signatures.
6. **JE/Action.hpp:10** — "original ctor not yet RE'd": T-060's Action ctor identity era passed it — check
   the batch-4 chain notes; if the ctor is the known +0x34/flag-init shape, implement+cite; else reword to
   named-absence WITH the EA that would settle it (gate: one xref query, any lane can settle).
7. **JE/cls_0x50db20.cpp:67 / JE/ActionMoveTo.hpp:18 / JE/cls_0x4d8d70.cpp:112** — parity-stub notes from
   the DB era: T-060/T-033-era renames give these real identities; either fold into the named class or
   delete-with-cite per the GameActionAttack.hpp:61 rule the user's own header already states.
8. **JE/SS/StringFormat.hpp:57** — "not yet wired into Get_0x56C260": the STF table era is done
   (InitializeStrings @0x45e330 port-cited) — wire it, cite.
9. **JE/ResourceSystem.cpp:29** — "stubbed blobs": T-066 Step 1 (commit 78bf743) reconstructed the ctor
   with real fields — sweep the comment to reflect what shipped.

## GATED (leave; each names its key)
- **JE/GUI/SkinFragments.hpp:226 / SkinTable.cpp:177** — +0xC4 convert-gate decode: needs T-013 Step 4
  (device-era texture flags) — armed, M-001-adjacent.
- **JE/Network.cpp:50/92/99** — traced netcode stubs: gate = net session era (P2 frontier); fine as-is
  for the SP demo.
- **JE/PathManager.cpp:138/307/568** — interim search models: gate = T-035-era pathfinding RE row; do NOT
  let a half-model silently pass — its fail-loud placeholder is correct behavior for now.
- **src/F3/RenderD3D8.cpp:102** — `g_state=3 "device not yet created"`: not a gap — a state. Slice 2 flips it.

## NARRATIVE (no work; optional reword when a lane touches the file anyway)
- MapEntitySpawn.hpp cluster (the shared-stub/mirror documentation is the CLOSE, not a gap),
  CombatManager.hpp:116 + GameWorld.cpp:58 "the OLD C-23 placeholder … retired here" (retired = done),
  GameActionAttack.cpp:295, MapEntitySpawn.cpp:285, src/F3.cpp:70, src/F3.hpp:63, EngineStringSystem.hpp:14,
  cls_0x4d8d70.cpp:112's static-init note. These are the corpus's named-absences speaking correctly.

## Acceptance
Items 1–9 each: a commit with the function modeled/cited + unit or digest gate green + the marker line
REMOVED (a completion that leaves its TODO is not a completion). Then `grep -c` over the COMPLETABLE set
reads zero — and R-FINISH flips to done with the GATED set handed to its era's rows.

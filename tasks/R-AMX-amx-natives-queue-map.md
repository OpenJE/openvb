---
status: in-progress
assignee: WildZenith `01a16092` (PULL per lane contract, PARK id R-AMX, dispatched by LoudDragon `01a15f82` 2026-10-21; lane-fit note "completes their atlas consumer-side")
type: research (read-only; discoveries DIRTY-until-checkpoint; relay-only for board state)
depends: T-074 (+0x2E0), T-030/T-070 (+0x304 funnel), T-080 (+0x378 mailbox), T-085 (+0x1A8 anim deque + prearm boxes)
---

# R-AMX — AMX-natives → queue-surface consumer map (the atlas FRONT DOOR)

## Objective
The four-queue atlas was built callee-side. This row maps the native side: **which AMX script
export touches WHICH queue surface, with WHICH entity selection, in ONE call chain** — producing
the native × queue × selection matrix. Known partials to complete/correct:
- T-080 gave the SendMessage trio (mailbox, direct push) but the trio-vs-funnel question is open:
  T-070's funnel census line listed a "SendMessage ×3 families" entry among callers of `0x550720` —
  either the trio does BOTH (native + action-bridge), or T-070's label was loose. Settle with xrefs.
- T-085 found the `QueueAnimEvent_4673d0` front door with 100+ ANIMEVENT ids but did NOT trace which
  callers REACH it — the action-start side (idle `0x4673d0(ent,9,0)` in T-070) is only half-mapped,
  and the AMX native that feeds it (if any direct) is unexamined.
- T-070 §2 named script natives F3::Attack/JustAttack/StartDialogue/ReloadWeapon as funnel callers
  but no full native registry census exists: the AMX native table must be located and enumerated
  (T-037's script-native record map is the reference for the registration mechanism).

## Questions (the matrix rows)
1. THE NATIVE TABLE: locate the AMX registration surface (name-string -> native-fn pairs; pattern
   anchors known: native prologue `sub_59AF90(args, n, "Name", ...)` @0x5c5435 in SendMessage;
   T-037 record map for the other registration style). Enumerate EVERY registered native with its
   handler EA. Acceptance: table count stated + how completeness was achieved (data-scan of the
   registration array, not string grep luck).
2. PER NATIVE: which of the four queue surfaces its handler chain touches — via the primitives as
   bounties: funnel `0x550720` (+0x304 actions), `meth_0x5506c0` (+0x2E0 effect inbox),
   `sub_54EE60` (+0x378 mailbox), `sub_5505F0` (+0x1A8 anim), `CreateGameAction 0x46f290`
   (kind -> action) — direct call, or via which intermediary. Natives touching NO queue get a
   named NONE row (the matrix is a census, not a highlight reel).
3. SELECTION MECHANISM per native: self (owner [action+0x14] or AMX this), individual handle via
   `0x54be10`, team via `GetTeamMemberList_56a2f0`, squad via `GetSquadMemberList_569390`,
   broadcast — the three selectors T-080 pinned in ActionMessage::start apply broadly; pin which
   natives use which, EA-anchored.
4. THE SEND-TRIO × FUNNEL question from the Objective (dual-path or mislabel — bytes decide).
5. Matrix rendered with the T-085 §6 prearm boxes as runtime hooks: each native row cites the bp on
   the queue primitive + caller-address grouping that would CONFIRM it live (boot lane lifts rows
   without re-deriving).
6. Port-relevance: NAMED verdict — what the script-VM port era needs from this map; no port work
   invented in this row.

## Steps
1. Claim (this file + relay; PARK line stays, coordinator mirrors).
2. Find the AMX native registration: xrefs to `sub_59AF90` (the arg-parse prologue) enumerate
   native-handler heads; cross-check against the registration array/data table for completeness.
3. For each handler: classify by which primitive(s) its call graph reaches (1-2 hops is usually
   enough; deeper paths cited to the hop).
4. Build matrix; verify the Send-trio dual-path question; enumerate CreateGameAction(kinds) callers
   natively (kind 6/7 known; full kind->native map if any).
5. ecx-flow gate-first if any class-membership claim; raw-over-decompiler; item 41 three-query
   strength for the census completeness statement.
6. Deliver `discoveries/2025-10-21-r-amx-native-queue-map.md` DIRTY + relay.

## Acceptance
- Native table census with count + completeness method stated; matrix rows native x queue x selection,
  every cell EA-anchored; the Send-trio×funnel question answered; NONE rows present; prearm-hook
  column; named port verdict. Read-only honored; collisions avoided (KeenStorm product lane,
  HappyEagle debug harness untouched).

## References
T-074/T-080/T-085 docs (all four surfaces + selectors + bps), T-070 §2 funnel census line,
T-037 (script-native doors), harness items 40-47, `openvb-ida-mcp` skill.

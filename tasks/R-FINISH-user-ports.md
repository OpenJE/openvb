---
status: COMPLETABLE DONE (items 1-9 landed, marker grep-zero, gates green); GATED stands as filed
assignee: SwiftOwl `01a16198-6cbb` (QuickNova dispatch ece5726: "SwiftOwl now ALSO takes R-FINISH — they hold the fenced tail, the submodule chain, and the checker"; predecessor seat KeenStorm died mid-renderer-era, all its landed work stands)
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

## Item closes (SwiftOwl 01a16198, this seat)
- **9 DONE** (openje ab93890 / bump 09ae5b0): ctor comment swept to the IDB-verified construction
  (0x49c62e-0x49c69f, band name resource_cache_0x12c from the IDB itself); the 9 fenced defs cited
  at completion. THE 9 SELF-DELETED EXACTLY AS PROMISED — campaign gap 0/356 at that point.
- **4 DONE** (openje ef086dc / bump ac67e6a): clock() placeholders -> Host::TickMilliseconds
  (0x56AF50, modeled source as used @0x563196/0x563199); the shipped RETURN SPLIT recovered from
  the body and implemented verbatim (enter-4 tick @0x54556a-0x545573, leave-4 elapsed
  @0x545585-0x54558b, other -> PRE-transition state @0x545516/0x54558e); callerless verified by
  grep, header contract extended.
- **6 DONE** (openje b77597c / bump c9de63b): no standalone Action::ctor EXISTS (method census
  over .?AUAction@@); base init inlined as the LAST write mbr_0x4=1 in all four sampled derived
  ctors (0x5a9252/0x55fb21/0x560191/0x5a9018) -> base constructs to 1, repairing the silent
  zero-drift in derived classes that never re-wrote the flag; +0x34 alternative ruled out by the
  same dumps (base subobject = vftptr+flag, 8B). Test upgraded to the byte-shape assert.
- **7 DONE** (openje + bump, this commit): THREE targets, TWO real. 7a cls_0x50db20.cpp:67 —
  0x401940 decompiled: it is the REAL resize/shrink helper (_Xlen guard @0x40194d, shrink-to-inline
  @0x40199d-0x4019c9, growth via realloc primitive meth_0x401510 @0x401967/0x401980, whose own
  shape is @0x401532-0x4015e7); the `return 0` lie REMOVED delete-with-cite per
  GameActionAttack.hpp:61's own rule, hpp declarations kept labelled UNPORTED. 7b
  ActionMoveTo.hpp:18 reworded from "full JE::Entity stub" to the named reality (Entity.hpp,
  base vft 0x692084 pump-at-+0xC8). 7c **ERRATUM**: cls_0x4d8d70.cpp:112 is NOT a COMPLETABLE
  target — line 112 is a brace; the static-init note it means lives at :116-117 and the same
  task file's NARRATIVE section correctly rules it "speaking correctly" (deliberate
  not-reproduced, cited). COMPLETABLE-vs-NARRATIVE collision recorded here; treated as NARRATIVE.
- Gates per close, freshly re-sourced (item 58): run-all 11/0 + port-annot GATE PASS + msvc rc0
  (public headers touched). The annot gate policed items 4/6/7 as they landed — self-consistent.
- **1 DONE** (src e2c1bac): the +0x90 PAD IS A REAL MEMBER now — embedded JE::VFX_Parameter,
  layout closed by R-VEG-E's value shape {name SSO buf+0x04/size+0x14/cap+0x18=15, tree at
  value+0x1C} + the World teardown chain (sub_5AA340(this+144) @0x54852f; SSO reset
  @0x5aa3a7-0x5aa3b1 — the triple the item-2 ctor byte-pins); file-static g_worldParams
  retired, Initialize rewired through the member.
- **2 DONE** (openje 36a4cfc): ctor sweep BYTE-PINNED from the decompiled @0x4fe1e0 — the old
  zero-for-cleanliness was WRONG in four places (+0x1c=15, +0x20/24/28=-1/-1/-1 sentinels,
  mbr_0x4 never ctor-written); roles from R-VEG-E §1 (TYPE enum over the 15-vocab @0x6fbbe8,
  inline-default float bits, buffer trio {size,allocated,data}).
- **3 DONE** (openje 36a4cfc): GetBool sub_4DF8B0 VERIFIED-READ and re-modeled — miss returns
  DEFAULT AS-IS (@0x4df94d/0x4df9ef, no clamp), hit returns atol>0 STRICTLY-POSITIVE
  (@0x4df9cb/0x4df9e7); the GetInt delegation is gone.
- **5 DONE** (openje 791710b / bump 4c2712e): the three stub comments in ActionAnim START are
  RESTORED CALLS — sink with the byte-verified 5-arg signature @0x5a92a1, [ent+0x288] clear
  @0x5a92a8, deadline math @0x5a92d7; Entity::meth_0x548d80/meth_0x548fc0 modeled same-TU
  (T-066 step-2 precedent) with both shipped gates verbatim and the Gfx device continuation
  CITED-AND-OMITTED (port state never sets +0x160 — shipped and port terminate identically,
  faithful by construction, no false model); mislabeled net/critical_section.hpp include
  removed with the mislabel named. Frame proof per the keeper's ITEM-5 WINDOW RECIPE
  (9131dc8): cadence+identity on the warm rig — the honest kind RedQuartz armed.
- **8 DONE** (openje + parent, this commit): the SS::Format path is WIRED at its shipped call
  site — EngineStringSystem::Get_0x56c260 implements the now-decompiled two-call body (base
  fetch @0x56c26e via GetById_0x4c5900 + empty-string default, in-place format @0x56c276 via
  SS::Format/MakeEnv); StringFormat.cpp added to BOTH build source lists — the LNK2019 the
  mingw-first suites had been hiding; and the reason it HID is the coordinator's item 60
  (cached-cmkr no-op'd cmake.toml edits silently — TRUE-rc capture caught what the pipe had
  been green-ing). Final gates, re-sourced, honest rc: run-all 11 pass / 0 fail (app=124),
  msvc rc=0 'Built target F3', port-annot GATE PASS stock 0.

## ACCEPTANCE MET (SwiftOwl, closing commit): all nine marker phrases grep-ZERO over
src + modules/openje/src (layout pending / legacy placeholder / "GetInt for now" /
"until JE::TickMilliseconds" / "stub: signature mismatch" / "not yet RE'd" / "entry-point parity" /
"not yet wired into Get" / "stubbed blobs"). GATED set stands as filed: SkinFragments +0xC4 device
decode (UNDEFERRED by G1, queued behind the Gfx-model era), Network netcode era, PathManager
interim models (fail-loud placeholders correct by design), RenderD3D8 g_state=3 (a state, not a
gap). SUCCESSORS named: (a) the sink device arm + the +0xC4 gate want the animation-model era —
rig warm, law = trace-cadence per item 59; (b) Action-suite wiring into the run-all table is a
build-affecting follow-on this seat named and did NOT smuggle; (c) item-6's test stays unwired
until (b).

## Acceptance
Items 1–9 each: a commit with the function modeled/cited + unit or digest gate green + the marker line
REMOVED (a completion that leaves its TODO is not a completion). Then `grep -c` over the COMPLETABLE set
reads zero — and R-FINISH flips to done with the GATED set handed to its era's rows.

---
status: done
assignee: WildZenith `01a16092` (dispatched by LoudDragon `01a15f82`, 2026-10-21 — follow-up to my T-074's named-unresolved tail; board row filed `6826369`)
type: research (read-only on ida_original_f3 + offline tools; NO IDB writes; findings dirty-until-checkpoint)
depends: T-074 (which mapped the pop path 0x54c643-0x54c6c1 and left THIS open with evidence)
---

# T-080 — The +0x37C script OnMessage list: full census of the 4th queue surface

## Objective
T-074 established: `Entity+0x37C` holds a POINTER to a heap-allocated list anchor, `+0x380` a live count, `+0x384` an in-flight record guard; the Entity pump (`JE::Entity::virt_meth_0x54c570`) pops at most one node per frame between walk #1 and the script dispatches, runs `RunScript("OnMessage")` (string @0x69200c, 2 args), unlinks+double-frees the node, and decrements the count. Destroy path (`meth_0x54fff0` @0x550265-0x550298) frees the anchor and zeroes the count. The INSERTERS were named-unresolved: "inserters pass a computed sentinel address generically; no direct `lea 0x37c(` exists in the pinned image". That was a half-truth worth fixing — the sentinel is a STORED pointer, so inserters must LOAD it (`mov 0x37c(reg)`), which is the census key I missed. One row, done properly.

## Questions
1. Inserters census (complete): every site that loads the +0x37C anchor pointer and appends — resolve the list-add primitive they call (candidate family: `cls_0x401b00::meth_0x40e780` and siblings) and the object each site runs on. Which engines feed it — script natives (T-037 record map), net ingest, engine-side notifications?
2. The sentinel's MEANING: what is at the anchor address (layout, allocation site, who else holds it), and is the compared value an end-marker (standard intrusive-list sentinel), a queue-type tag, or an owner handle? Bytes decide.
3. Consumers of the MESSAGES: only the pump pop, or more readers of the anchor/`+0x380` count? What is the record `{+0: ?, +4: ?, node[2] payload}` — decode the value shape at the allocation site found in (1).
4. Canonical queue-map row: placement table entry for +0x37C alongside the T-074 three-queue map, with port-relevance verdict NAMED not invented.
5. If the census proves there are NO live inserters on shipped data (construct-only / dead surface), a named-open verdict WITH the census proof (which scans, at what strength, per item 41's three-query rule) is a full credit.

## Notes

**DONE 2026-10-21 (WildZenith `01a16092`).** Deliverable: `discoveries/2025-10-21-t080-37c-onmessage-inserters.md` (left DIRTY for coordinator checkpoint).
Verdicts, EA-anchored in the doc: (1) **Sentinel meaning settled by its allocator** — heap 12B self-linked END-MARKER via shared helper 0x56e0b0 (`new(0xC)`, block[0]=block, block[4]=block), stored [entity+0x37C] @0x551703, count [entity+0x380]=0 @0x551706; **[entity+0x378] never read/written anywhere** (raw sweep — and the frozen export has NO +0x378/+0x384 members at all, its +0x37C usage list omits the push primitive + every count op; strongest export blind-spot yet, item 27 class). Pop's null-test = post-destroy guard (destroy zeroes +0x37C @0x550295). (2) **Inserters census complete — exactly 5 sites / 1 primitive** `sub_54EE60` (3 allocs/msg: record 8B + data 4(n+2)B + node 12B): AMX natives `SendMessage` @0x5c5503 (verbatim error strings + FOURCC 'FROM' 0x464D4F52 @0x5c5442), `SendMessageToTeam` @0x5c5908, `SendMessageToSquad` @0x5c5a7a (direct pushes) + `ActionMessage::start` fan-out @0x5a9107/0x5a91a7/0x5a9214 (team [act+0x24]/squad/individual [act+0x1C] member lists, each handle resolved via 0x54be10; from-field = [[act+0x14]+0x1DC] = funnel(R4)-of-T-043-handle @0x5a90fa-0x5a9104). (3) ActionMessage = engine kind **7**; both factory jump tables raw-read: World::CreateAction @0x5450cc (0=Die 1=Idle 2=MoveTo 3=PickUp 4=Wander 5=Wait **6=Anim 7=Message** 8=FATAL 9=Equip 10=Unequip 11=SetActiveWeapon), reached from GameWorld::CreateGameAction default @0x46f4e3 with -0x230 THIS-adjust only (kind passes unmodified — numbering identical both sides; feeds T-071 alignment). vft 0x6a5f34 = standard action protocol (0x404b60 isFinished slot tell); **update @0x5a9070 sets done-byte, consumes 0ms → message action finishes first tick, no HOL residency**; payload buffer one-shot free @0x5a9113. (4) Consumers: pump pop ONLY reader (≤1/frame, re-entrancy guard = +0x384 in-flight slot + node-recheck @0x54c67b); discard ×1 + node-sweep at destroy (byte-level fact: the sweep frees NODES only, record+data of ≥2-deep backlog unaccounted — named with EAs, not judged). (5) Queue-map row appended; port relevance NAMED: no paired observable exercises this surface; OnMessage-dispatch seam + bridge-action mirror filed for script-VM era only.
Method: re-keyed the very census miss from my T-074 tail (lea-family → mov-family loads — the correction relayed pre-flight), raw-over-decompiler (one vft-base off-by-4 caught by raw dump), item-8c allocation-site rule honored, item 41 three-query posture stated in doc §6, selftest re-gated 6/6 (tool gained T5) immediately before verdicts, read-only honored (zero IDB writes).

## Steps
1. Claim (this file + relay; table stays coordinator-mirrored). [done via protocol]
2. Raw sweep the pinned exe (md5 a68c22f3…) for `0x37c(%` LOADS (mov variants — not just lea) and `0x380(%` stores; every hit gets decompiled + EA-anchored.
3. Locate the anchor's ALLOCATION site (who stores into Entity+0x37C) — per harness item 8c: a slot is not attributed until reached from a typed use or an allocation site.
4. ecx-flow gate-first if any class-membership claim comes near (selftest 5/5 before verdicts; item 44 sharing test).
5. Deliver `discoveries/2025-10-21-t080-37c-onmessage-inserters.md` DIRTY, relay batch line; flip THIS file only, never the table.

## Acceptance
- A census of every +0x37C-touching site (loads AND stores) with function names + EAs, and either (a) at least one concrete inserter path named down to its caller context (native/net/engine) with the record's allocation site, or (b) an item-41-strength three-query negative with the scan list stated.
- The sentinel's meaning pinned by bytes (layout dump of the allocated anchor + the compare shapes at pop).
- Queue-map row appended (same table shape as T-074 §5).
- Read-only honored (no IDB writes); no KeenStorm/HappyEagle lane files touched; commits: task file only.

## References
T-074 doc (`discoveries/2025-10-21-t074-entity-2e0.md`, §5 row 4 + §7 tail), T-043 (+0x1DC handle), T-037 (script-native record map), harness items 40-47, `openvb-ida-mcp` skill (ORIGINAL_REFERENCE discipline).

---
status: in-progress
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

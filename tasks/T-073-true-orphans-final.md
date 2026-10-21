---
status: in-progress
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (dispatched by LoudDragon `01a15f82`, 2026-10-21 — onboarding row: every board weapon in one hunt)
type: research (read-only on ida_original_f3 + offline tools; NO IDB writes; MINT-READY queues to coordinator)
depends: T-063 (defined the orphan set), the tool corpus (T-053 method, ecx-flow T-054/57/59/63, T-062 RTTI-chain anatomy)
---

# T-073 — Final adjudication of the 17 true-orphans (cls_0x490150 x9, cls_0x4fc0e0 x8)

## Objective
T-063 closed the census era with exactly K=17 functions surviving EVERY evidence class, each with
item-41's three queries answered. This row gives them ONE full weapons-grade pass — everything the board
built between then and now, applied together — and demands a final verdict per family: MINT-READY owner
(single-owner proof per item 44: NOT reachable via >=2 view constants or >=2 holder fields) or FINAL
honest negative (all three queries answered at CURRENT tool strength, named residual). A guess is failure;
'unknown, proven unknown' is success.

## Weapons (all standing equipment — read their docs/headers first)
- `tools/ecx-flow.py` families/band/pads/selftest (md5-pinned cache; selftest 5/5 gate FIRST)
- T-053 string-mine method (incl. the \xc7-pad EA+1 xref trick + dismissal list discipline)
- RTTI COL->TD chain anatomy (T-062's method: COL at vft-4, size-0 RTTI = VIEW subobject — item 42)
- vft-store census + construction-tier same-this checks (T-055/T-064 composite-split family —
  'always test same-this construction before accepting a doc's fn count')
- item-44 sharing test before ANY membership claim

## Known context per family (do not re-derive; test it)
- cls_0x490150 x9: T-054 verdict CONSUMER-of-Gfx_D3D (calls on &dword_7082F8) — is it a LIGHTING or
  RENDER-STATE consumer family with an embedder host not yet checked? The Gfx_RS_* names exist — map them.
- cls_0x4fc0e0 x8: coherent anchored-nowhere epoch; thunks tail-jmp-only; check data-section vft windows
  (.data/.data1, not just .rdata — the 445350 lesson) and save-block/FLOAT-archive membership
  (its region neighbors archive code per T-062's band map).

## Acceptance
discoveries/2025-10-21-t073-orphans-final.md (LEFT DIRTY, researchers never commit): per-family verdict
+ evidence index; MINT-READY (if any) queued to coordinator in writer-wave shape; FINAL negatives with
the three queries answered at current tool strength. Coverage statement. Ping [DISCOVERY-BATCH ready].

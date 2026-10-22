# The Sitting Runbook — UNIFIED KIT (re-banked 2026-10-22 after the OOM freeze; tracked file on purpose)

**Provenance note:** the 'sitting-prearm-ladder.md' path quoted in earlier messages never existed in
git — the tables below live in their REAL homes (T-085 §6, R-AMX §5); this file is the composition,
not the source. If any EA below disagrees with those docs, the docs win and this file is corrected.

## Pre-armed breakpoints (T-085 §6 — bp7..bp13)
| bp | EA | role | a HIT means |
|---|---|---|---|
| bp7 | 0x54c833 | raiser R1 not-finished | multi-frame action re-armed mid-walk |
| bp8 | 0x54c864 | raiser E3 exhausted-with-budget | deque emptied while budget remained (second consecutive) |
| bp9 | 0x549090 | clearer C1 | per-round latch reset + [ent+0x300] stamp |
| bp10 | 0x5a955d | consumer X1 skip-advance | **THE T-050 falsifier** — any hit reopens the row |
| bp11 | 0x5a94a0 (guard @0x5a94ab) | manager re-entrancy | second entry while mbr_0x8 set |
| bp12 | 0x54c9c1 + 0x54c92a | anim pop discriminator | which pop path ran |
| bp13 | 0x548d80 entry | sink census | all three caller classes visible at once |

## Native crosswalk (R-AMX §5 — bpN1..bpN5)
| bp | EA | identity it pins |
|---|---|---|
| bpN1 | 0x550720 entry | every +0x304 push; return-address -> native (63-site table) |
| bpN2 | 0x54c440 entry | the four drain natives vs engine resets |
| bpN3 | 0x54ee60 entry | bare trio only — sync-mailbox traffic |
| bpN4 | 0x46f290 | CreateGameAction factory: pushed K = live kind histogram (settles R4/R4b) |
| bpN5 | 0x5505F0 entry | anim deque pushes = engine Queue_* only, NEVER a native |

## NEW RUNGS (post-slice-1 era)
- **G1 — the device question, one command:** run gated reimpl (F3.exe, OPENVB_RENDER=1 +
  OPENVB_RENDER_TRACE=<file>) under the KNOWN-GOOD t067 prefix (nv32 ICD + 3-dll DXVK + both reg
  lines; gated path touches no demo data). Answers whether Direct3DCreate8 has EVER returned
  non-NULL on this host. KeenStorm replicated everything else (dll shas, reg, ICD, loader, display
  class incl. sway/Xwayland, init order) and it NULLs everywhere. If also NULL here: the 12-min boot
  never created a device (intro era = bink/GDI) and NOBODY has had one up — re-shapes slice 2.
- **G2 fallback (one line):** diff known-good prefix user.reg DllOverrides-era keys vs fresh win32 prefix.
- Runtime identity chain for any script-side sighting: bp13 fields <-> bpN1/kind <-> bpN4 — three EAs.

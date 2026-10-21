# T-085 — The +0x1A8 animated-event deque: record anatomy, QueueAnimEvent feeder tree, the +0x319 raiser truth-table (both branches byte-verified), and the T-077 BP-PREARM BOX

**Agent:** WildZenith (sessionId `01a16092-2ef1-703e-b403-9e417653ef5f`) · **Date:** 2026-10-21 · **Row:** T-085 (claim `2058136`) · **Method:** read-only `ida_original_f3` + native f3_* + objdump raw over the md5-pinned exe (`a68c22f3dd1c256ec9539a70e87cb434`). `tools/ecx-flow.py selftest` re-gated this session before verdicts: **6/6 PASS** (incl. new T5 share-check). Zero IDB writes. Completes the four-queue atlas (T-074 three + T-080 mailbox + this cell's anatomy).

## CROSS-LANE NOTES FIRST (2, both raw-verified this session)

1. **KeenStorm's T-082 idle correction: CONFIRMED from my own raw dumps.** `0x5a9070` = `movb $0x1,0x18(%ecx); xor %eax,%eax; ret 8` — unconditional done-byte write, 0 ms consumed. It is BOTH GameActionIdle's update (T-070 §1's own vft table line +0x14) and ActionMessage's update (my T-080 vft dump @0x6a5f34, slot +0x14) — one shared transient stub. Idle is a ONE-TICK action; it can never be the head at the not-finished latch site (0x54c833) across frames. My T-080 doc's update-slot wording stands as written; the raiser expectations in §4/§6 below bake in the transient model.
2. **LoudDragon's relay parenthetical SWAPPED the two drains.** Per raw (re-dumped this session): `meth_0x54c390` reads `mov 0x2ec(%ebp)` @0x54c398 + `lea 0x2e0(%ebp)` @0x54c39e → it is the **EFFECT-TREE (+0x2E0) full-drain** (caller: entity reset @0x5501a2). `meth_0x54b3f0` reads `mov 0x37c(%esi)` @0x54b3f3/0x54b3ff, decrements +0x380 @0x54b426-30, has NO loop → it is the **MAILBOX (+0x37C) single discard-pop** (caller: entity reset @0x5501b0). My T-074/T-080 attributions were correct as committed; the flip entered during relay, not from any lane doc. Flagged so the checkpointed table lines get amended, not re-learned.

## 1. Record layout — the 24-byte animated event (by-value, heap blocks)

The deque at `Entity+0x1A8..+0x1B8` is the same 20-byte container family (blockmap +0x1AC, block-offset-base +0x1B0, cursor +0x1B4, count +0x1B8 — T-074 §1a) but **stride = pointer to one `new(0x18)` record** — `sub_54EB40` @0x54eb80 allocates 24-byte blocks per slot and `qmemcpy` copies the record in. Its sole push site is the feeder `sub_5505F0` @0x5506b0 (xrefs to 0x54EB40 = exactly 2: that + a thunk) — **one primitive, no bypass writers.**

Record dwords (feeder frame @0x55060d-0x5506a1 + pump copy v40[6] + sink signature `meth_0x548d80(ent, param_1, param_2, a4, a5, a6)` — all mapped):

| rec off | feeder source | sink arg | meaning (EA-pinned) |
|---|---|---|---|
| +0x00 | param_1 | param_1 | anim primary (name/id) — resolver `(*[ent-vft+0x48])(ent, p1, p2, &out, &out)` @0x550630, re-resolved at sink @0x548dc4 (`[vft+18dwords]` = same +0x48 slot) |
| +0x04 | param_2 | param_2 | anim secondary/blend target |
| +0x08 | param_3 | a4 (u16) | **MODE/COUNT**: 0 = loop-style → `[ent+0x188]=1` @0x548eab (pop-next-per-frame mode); 1 = single → ready-stamp = start+1×dur; ≥2 = `[+0x184] = start + a4×dur` @0x548e8f (N-play). Also gates the feeder's own duration path @0x550670-0x55068d (2000 ms fallback when no model) |
| +0x0C | param_4 | a5 (byte) | sync/stop flag → stored to AnimMesh instance `+0x90` @0x548e55 |
| +0x10 | `v16 = Duration×param_5×10.0` @0x5506a1 | a6 | **EXPLICIT-TIME OFFSET**: `[ent+0x180] = worldms − a6` @0x548e3a — the record carries a play-head shift into the PAST (join-in-progress semantics); duration via `Gfx_AnimationObject::GetDuration_4a26d0 × −1000.0` @0x548e79 (the T-070 §3 neg-ms timeline fact, source-pinned) |
| +0x14 | param_6 | (v40[5]) | **pop flag**: if set when the event is popped → `movb $0,0x1bc(%ebp)` @0x54c93d / @0x54c9f0 — clears Entity+0x1BC, the state-7 ServerUpdate gate (§3) |

(The row's "(+0x14, T-043 handle composed)" shorthand resolves to: rec+0x14 = the +0x1BC-clear flag above; the +0x1DC-handle composition actually lives on the ADJACENT mailbox bridge — `[[action+0x14]+0x1DC]` @0x5a90fa-0x5a9104 (T-080) — stated so the two paths don't merge.)

## 2. Feeder census — 267 sites, ONE primitive, grouped by host (xrefs to sub_5505F0, complete)

| sites | host fn (live IDB name) | family |
|---|---|---|
| 87 | `meth_0x463c80` | ANIMEVENT-id builder (per-anim-slot matrix) |
| 53 | `meth_0x464d70` | second builder matrix (self-recurs @0x46578a) |
| 24 / 24 | `GameEntity::Queue_Walk_465fc0` / `Queue_Limp_4669f0` | locomotion state families |
| 21 / 11 / 10 | `QueueToCombat_463140` / `Queue_Idle_462c50` / `QueueToDefault_465c20` | combat/idle/default stance families |
| 3 / 3 / 2 | `Queue_Equip_465ae0` / `Queue_Unequip_4636b0` / `Queue_To_Crouch_465ee0` | item/stance one-offs |
| 9 / 5 / 3+3+2+2 / 1×5 | `meth_0x464940` / `meth_0x4659a0` / `meth_0x464c70`,`0x464b80`,`0x4667a0`,`0x466880` / `meth_0x463000..0x463100` wrappers | remaining builder/wrapper hosts |

**The routing tree above the families:** `F3::GameEntity::QueueAnimEvent_4673d0(ent, ANIMEVENT_ID, float)` — a 100+-case event-id dispatcher (verbatim log string `"GameEntity::QueueAnimEvent() - Unhandled ANIMEVENT.\n"` @0x467536), gate: bail if `[ent+0x9C]` set (sticky suppress; `=1` written by the 52-87 id block @0x4674c9). This is the function T-070 described as "entity-state calls 0x4673d0(ent, 9, 0) / (ent, 1, [action+0x1C]) through the 141-case state machine" — **it is not a state setter; it is the animation-event queue's front door** (case 9→QueueToDefault, case 1→Queue_Idle(float) — the idle activation's anim side-effects re-read cleanly under the transient-idle model). Direct queue-push callers are all in the creature/equip command band 0x462c50-0x4668db; class attribution follows live IDB names, band-noise caveat per item 43 applies (no membership minted — census quotes host FUNCTION + site count).

**NOT queue feeders (bypass family):** direct sink drivers calling `meth_0x548d80` without deque entries — `ActionAnim::virt_meth_0x5a9280` (engine kind 6 START: plays [[action+0x14] OWNER — the funnel store] directly, then **clears `[ent+0x288]`** @0x5a92a8 — queue-playback OFF — and computes its own finish deadline `[act+0x24] = ms − (a4+1)×GetDurationMs` @0x5a92d7), `ActionMoveTo::virt_meth_0x55b370/0x55b3b0` (×2), `ActionPickUp::virt_meth_0x563550` (×4), `GameDoor`, `GamePlayerAvatarManager::avatarRebuild_0x42e870` (×2), `cls_0x408990` (×2), `sub_40EC90` (×2). Full sink-caller census = xrefs to 0x548d80 (19 sites, none truncated; includes the pump's own 3). `[ent+0x288]` is the sink's own gate (`if (![ent+0x288]) return` @0x548d83-0x548d8b) and is record-set at spawn (`= *(rec+648)` @0x552538 in virt_meth_0x552420) — i.e., per-entity-type: kind-6 ANIM actions DISABLE queue playback for their duration.

## 3. Where the deque sits in the frame — pump else-branch (raw) + drain-vs-pop

Pump `virt_meth_0x54c570` order: walk#1 (+0x2E0) → mailbox pop (+0x37C) → script OnCutscene*/OnUpdate → **walk#2 (+0x304) + raiser pair (§4)** → **anim deque cell, two mutually-exclusive paths** (raw 0x54c891-0x54ca31):

- **Path A `[ent+0x188] ≠ 0`** (mode-0/loop-style event started last): @0x54c89f count>0 → copy head record (block math identical to T-074 §1a) → **cursor advance INLINE** @0x54c8df-0x54c909 (inc, base-wrap, `--count`, count==0→cursor=0) → sink(v40[0..4]) @0x54c92a → if v40[5]: clear +0x1BC @0x54c93d. No time gate.
- **Path B `[ent+0x188] == 0`**: @0x54c964-0x54c96e: `worldms = ([0x70BD98]-sub vft[0])()`; **ready gate `worldms >= [ent+0x184]`**; if ready and `[ent+0x180] < [ent+0x184]`: `meth_0x549fc0(ent, 0x184−0x180)` @0x54c987 → `sub_5643A0([ent+0x160], ms)` = **fast-forward the model by the missed interval** (the explicit-time catch-up). Then if count>0: copy, **advance via helper `cls_0x54b590::meth_0x54b590(&hdr)` @0x54c9c1** (same inc/wrap/--count/reset, byte-identical to the inline sequence — census: the helper's only non-alias uses), sink @0x54c9e1, flag-clear @0x54c9f0.
- **Path B empty** (count==0): default-anim fallback @0x54c9fe-0x54ca2b — gated on `[ent+0x295] == 0` (ctor-zeroed, NEVER written elsewhere in the image — permanently open; export census matches raw sweep), then `[vft+0x50](ent)` (slot 20 decimal-index) if 0 `[vft+0x58]` → `sink(id,0,0,0,0)`. **This is the T-070 §3 "animation-level default" mechanism, now source-pinned, and it explains why fresh entities show Anim events with zero queue history.**

**Drain-vs-pop semantics (atlas completion):** +0x304 = pop-with-HOL-break + drain-all (`meth_0x54c440`, cancel `[vft+8]` pass first); +0x2E0 = full-scan skip-past + per-entry erase + drain-all (`meth_0x54c390`, fire-on-drain via `0x55a570`); +0x37C = one-per-frame pop + single discard + node-sweep at destroy (T-080); **+0x1A8 (this row) = one-per-frame POP (Path A/B), NO drain-all exists** — its destroy path is block-free only (`sub_5C1E60(&+0x1A8)` @0x550553; records are NOT object pointers → nothing to dispose, per-block `j__free` in the clear helper). A queued-but-unfired event dies silently with the entity — no cancel-notification analog (`[vft+8]` has no counterpart; there is no per-record virtual at all — pure value semantics).

## 4. THE +0x319 RAISER — truth table, both branches byte-verified

Shared code, reached per frame per entity through: World step `virt_meth_0x545d60` → busy-list iteration (`byte_70BE24` loop @0x545e71-0x545ecc) → `F3::ServerUpdate @0x549130` (state-gate: `[ent+0x78]∈{1..7}`, `[ent+0x3A0]` enabled; **states 4,5 pass to `[vft+0xC8]` directly; state 7 passes ONLY if `[ent+0x1BC] ≠ 0` @0x5491c0** — the record flag from §1!; 6 = its own slot) → `[vft+0xC8] = 0x54c570` (raw vft scan: pump present in 11 tables incl. Entity base 0x692084 at byte +0xC8).

Raiser cells, in execution order inside walk #2's tail:

| # | EA | condition (raw) | action | latch-role | item-45 half |
|---|---|---|---|---|---|
| R1 | **0x54c833** | head NOT finished `[vft+4]==0` (@0x54c7f1-0x54c7f6) **AND** `[ent+0x174] < [head+0xC]` (@0x54c828-0x54c831 `mov 0x174; cmp 0xc(%esi); jae skip`) | `movb $1,[ent+0x319]` | **raise** — "processed-not-finished, budget insufficient" | PRODUCER — shipped-LIVE (ungated shared code) |
| self | 0x54c83a-0x54c842 | `if [+0x319] → goto exit-pair` | — | debounce (no double-raise same frame) | — |
| E1 | 0x54c845-0x54c851 | head null (deque exhausted cleanly this walk) AND `[ent+0x174] != 0` (budget remained) | fall to pair | exhaustion precondition | — |
| E2 | 0x54c86d | …and `[ent+0x318]==0` | `movb $1,[ent+0x318]` (ARM) | first exhausted-with-budget occurrence | PRODUCER side, live |
| E3 | 0x54c85d + **0x54c864** | …and `[ent+0x318]==1` | `movb $0,[ent+0x318]; movb $1,[ent+0x319]` | **raise (exhausted branch)** = second consecutive occurrence | PRODUCER side, live |
| C1 | **0x549090** (`[vft+0x60]` slot) | called per entity by manager maintenance when `[ent+0x1E0]==5` (@0x5a94cc-0x5a94fd) and from GameCreature turn accounting `sub_409BE0` @0x409bfa (verbatim strings incl. GetDerivedAttribute HIT/ACTION_POINTS) | `movb $0,[ent+0x319]; [ent+0x300] = arg_ms` | **CLEAR + last-turn-ms stamp** | both producer-of-0; live |
| X1 | **0x5a955d** | manager round-walk `cls_0x5a9330::meth_0x5a94a0` — entered ONLY from `World::virt_meth_0x545d60 @0x545e0d` (xrefs complete: 1 real caller); branch requires `this->mbr_0x4 ≠ 0` (round MODE flag @0x5a951d) | if `[ent+0x319]`: `[ent+0x29C]=0` (@0x5a9567), ADVANCE shared cursor @0x5a9572, re-arm via `meth_0x5a9330` if `mbr_0x14` | **CONSUMER — skip-advance** | CONSUMER — T-051-pinned DEAD at shipped config (mbr_0x4 never non-zero) |
| C2 | 0x54c853/0x54c874 | pair self-reads; `[+0x318]` at 0x54c874 also gates the busy de-reg `meth_0x54bf00` (@0x54c87c; else `[vft+0x6c]=virt_meth_0x54ab00` composite busy check) | — | — | — |

Budget field `[ent+0x174]`: ctor-zeroed @0x5513f2; decremented ONLY by walk #2 @0x54c7e7 (`sub %eax,0x174(%ebp)` after runner `sub_55A1C0(action, frame_ms, budget)` @0x54c7e2); read at R1/E1; **granted/zeroed in the F3 round band**: spawn bulk-set @0x446fb6 (owner fn in 0x446 band, record-driven, +0x16C/+0x170/+0x174/+0x178 group), round-reset zeroing @0x455e11-0x455e1d (`+0x170/+0x174/+0x178 → 0`), +0x1BC dword-writes on state 4/7 transitions @0x451c34-0x451c3a (indirect `call *0x6472d0`). Exact grant formula = T-011/turn territory, cited not re-derived. **Export census catch (item 27, third instance):** the DB's Entity mbr_0x174 usage list includes 0x5ab3e0 — that site is a `cls_0x50db20::meth_0x571110` (string-struct copy, 0x401b20×2 + field-block copy @0x571132-0x57114a) in the water-tile avatar-rebuild path; the class attribution there is band-noise, flagged, not used.

Truth-table summary: **[+0x319] = "this entity's action walk has outstanding work (R1) or has hit the exhausted-with-budget double-occurrence (E3)"**, cleared only via the state-5 maintenance/turn-accounting virtual C1, consumed ONLY by X1's dead-by-config branch. R1/E3/C1 all sit in shipped-LIVE shared code; X1 is the dead half. A latch raised but never consumed is the NORMAL shipped state (mode 0), and C1 re-clears it each turn cycle — this is consistent with, and now byte-sharpens, T-048's and T-069's accepted findings.

## 5. Explicit-time path (the row's Q3)

"Explicit-time events" = records with mode rec[+0x08] ≥ 1 under Path B: their next-ready stamp `[ent+0x184] = [ent+0x180] − mode×(duration×−1000)` (sink @0x548e8f/0x548ea5), where `[ent+0x180] = worldms − rec[+0x10]` (@0x548e3a) — i.e., **each queued event's due time is the previous start shifted by an explicit ms offset (rec+0x10) and stretched by N durations**. The pump fast-forwards the model by any missed interval before playing (@0x54c987→0x549fc0→0x5643A0) so a busy/latched entity catches its animation timeline up to world time instead of restarting it. `T-067 BONUS-3` (mode-1/2 interpolation live data) is exactly this field pair [rec+0x08, rec+0x10] — values observable at the sink bp below without any new machinery.

## 6. T-077 BP-PREARM BOX (lift verbatim on the :96 rig; branch tags + counterfactual meanings per LoudDragon's addition)

Breakpoints (all absolute EAs; F3.exe boots address-identical — pair on addresses per T-056):

| bp | EA | branch / role | what a HIT means | what hits WITHOUT bp10 mean |
|---|---|---|---|---|
| bp7 | **0x54c833** | RAISER R1 — not-finished (budget < [head+0xC]) | a multi-frame action (Wait/MoveTo/Attack volley — NOT idle: transient stub §cross-note 1) met insufficient budget → latch raised, consumer X1 is next frame's decision point | **consistent** — "latch raised, consumer gated-off by mode" (T-051 dead-consumer pin holds); R1 traffic without bp10 is the EXPECTED shipped behavior, no re-litigation triggered |
| bp8 | **0x54c864** | RAISER E3 — exhausted-with-budget, second consecutive occurrence | deque emptied while budget remained twice in a row → entity idled mid-turn | same: consistent; note E2 (0x54c86d, arm-only) may hit once without bp8 — that's the debounce, not a contradiction |
| bp9 | **0x549090** | CLEARER C1 (`[vft+0x60]`) — maintenance + turn accounting | per-round latch reset; also stamp [ent+0x300]=round ms; fires for entities at state `[+0x1E0]==5` | its cadence = turn cadence (L4 rebase observation pairs with it) |
| **bp10** | **0x5a955d** | CONSUMER X1 — skip-advance load inside manager walk, gated by `this->mbr_0x4` (round MODE) | **the refutation event for T-050's dead-branch pin**: a hit means the mode-gated walk actually ran → skip-advance is live, T-050's gating reopens ON THIS EVIDENCE ONLY | (n/a — bp10 is the thing) |
| bp11 | **0x5a94a0** entry (guard @0x5a94ab: `mbr_0x8` set/clear) | MANAGER re-entrancy detector | a SECOND entry while mbr_0x8==1 (i.e., from inside ServerUpdate→pump→virtual chain below bp7/bp8) = re-entrancy is STRUCTURALLY EXERCISED on shipped data: the shared cursor `mbr_0xc` (advance @0x5a9545/0x5a9572) would be mutated by the inner walk | if bp11 depth-counts stay 1 across a full combat round, the re-entrancy question stays closed-as-unobserved (NOT disproven — item 45's consumer-half caution; name it exactly that way in the T-067 log) |
| bp12 | **0x54c9c1** (`meth_0x54b590` pop) and **0x54c92a** (inline pop Path A) | ANIM deque path discriminator | which pop path the live frame takes per entity — feeds T-067 BONUS-3 (which mode values actually occur) | absence of bp12 hits with bp13 = all-default entities never queue events |
| bp13 | **0x548d80** entry | SINK census (all three callers visible at once: pump pops, default-anim @0x54ca2b, action-direct @0x5a92a1 etc.) | every live animation start with its record fields as params (a4=mode, a6=explicit ms) — BONUS-3 values for free | — |

Meaning summary (pre-empting the re-litigation LoudDragon named): **raiser hits (bp7/bp8) without bp10 = latch raised, consumer gated-off by mode — fully consistent with the T-051 dead-branch pin; only bp10 (and bp11 depth≥2) reopens anything.** Expect bp9 to fire per round; if bp7/bp8 fire and bp9 never does, that's a NEW finding (latch never cleared → entity stuck skipped even in mode-1) worth a loud relay, not a silent reinterpretation.

## 7. Atlas final table (appends T-074 §5; four surfaces + mailbox row from T-080)

| | +0x1A8 ANIMATED-EVENT deque (this row) |
|---|---|
| offsets | hdr +0x1A8(base)/+0x1AC map/+0x1B0 base/+0x1B4 cursor/+0x1B8 count |
| carries | 24-byte BY-VALUE anim-event records (name pair, mode, flags, explicit-ms, pop-flag) — no object pointers, nothing to dispose |
| fed by | `sub_5505F0` (sole push via `sub_54EB40`), 267 sites across Queue_*/builder hosts, routed by `QueueAnimEvent_4673d0(id 1..141)` front door |
| drained by | ONE per frame, Path A (mode-0 loop, ungated) / Path B (explicit-time ready-gate `worldms>=[+0x184]` + catch-up); NO drain-all exists — unfired events die with the block free at destroy @0x550553 |
| feeds | sink `meth_0x548d80` → `[ent+0x180/0x184/0x188]` timeline stamps; pop-flag clears state-7 gate `[+0x1BC]` |
| relation to +0x319 | reads/writes NONE of it — the raiser pair (0x54c833/0x54c864) sits in the +0x304 walk's tail in the same function; the deque pop runs AFTER the raiser, gated by `[+0x188]`, not by any latch. The atlas-level sentence: **+0x319 belongs to the ACTION walk, and this row's deque merely shares the pump frame with it — the five citing rows' "+0x1A8-adjacent" phrasing is now precisely: same function, different queue, order: raiser-then-anim** |
| +0x2E0 effect inbox | T-074 (erase-on-done full-scan + drain-at-reset `0x54c390`) |
| +0x304 actions | T-070/T-039 (HOL break + rebase drains `0x54c440`) |
| +0x37C mailbox | T-080 (one-per-frame pop, 3-alloc records, `0x54b3f0` discard) |

## 8. Port relevance — named, no invention

- T-071 (idle resident): unaffected beyond the correction cross-note 1 (transient model, KeenStorm's lane, P-REV queued).
- The four-queue atlas is now complete as RE; the port's currently-registered seams (funnel, digest D-2, latch T-048) live on +0x304 only — **no existing seam requires the anim deque to reproduce any paired observable** (R3/R4/R4b all +0x304/funnel-side; R4b's ActionAnim is a DIRECT-SINK action, §2 bypass family — the pair matches without queue modeling).
- Named future material (not work, not seams): (i) BONUS-3 = bp12/bp13 field values; (ii) T-050's gate has its exact live-refutation shape in §6 with meanings; (iii) if the port ever models per-entity animation timelines, §1's record layout + §5's stamp equations are the mirror spec, each term EA-cited.

## 9. Posture

Read-only honored (zero writes/saves); raw-over-decompiler throughout (the `+0xC raiser operand`, `slot +0x60/+0xC8` identities, `0x5ab3e0` band-noise catch, and the drain-attribution flip all resolved on bytes); item-41 three-query posture for the feeder census (call-site xrefs + raw opcode sweep + vft table scans, stated strengths in §2/§4); item-43 function-granularity on every census line; item-44: no membership claims made (host classes cited by live-IDB name + band-noise caveat only); ecx-flow selftest 6/6 before verdicts; both cross-lane notes verified from raw before being repeated (§header) — the coordinator-flip flag is delivered with the EAs, not as an accusation, per the correction-in-same-message culture.

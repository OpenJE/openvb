# PARK — standing work queue (coordinator-maintained; lanes PULL by claiming the named task file + relaying)

Rules (see tasks/README.md § Lane contract): every closed row names a PARK successor or goes on record DONE-NO-SUCCESSOR. Lanes pull from the top; coordinator assigns at checkpoint. Task files are created AT CLAIM — a PARK line is a scope, not a file.

## READY NOW (no deps, no slot conflicts)
| id | scope | lane-fit | notes |
|----|-------|----------|-------|
| P-REV | Product revision: T-071 comment-block rewrite to transient-idle semantics + Message kind-7 + the 0x2C alloc erratum; digest-count implications; SpawnMove asserts extended | KeenStorm | rides the T-080 §3 numbering inventory; touches MapEntitySpawn — sole-product-writer rule: no other src/ claims open |
| ~~Q-W2~~ **CLAIMED + SEATED (TrueCastle; claim file committed, reservations next)** — pre-flight gates now IN the wave definition: A= queue REGENERATION from T-084's own recipe + adopt-intersection-with-proof only (archived byhost.txt truncates two 17-fn hosts, t084-mint.json lacks the host map, and 4 Skin-row EAs DISAGREE with byhost-era + read NO-EDGE live -> disagreements label-and-skip, >15 = STOP-before-apply and re-cut goes to the coordinator); B= &global singleton rows fold to GUI::Manager per T-075 §8 writer-discretion (~6-10 re-hosted mid-wave, cited per row). 5 bands, checkpoint save each | wave-2 writer load | as filed |
| R-VFM | VFX_FormatManager anatomy: load/parse method chain (cls_0x4fd780::dtor-is-load case), .veg spec conformance vs van-buren docs, VFM size ≥56 live-vs-export erratum | SwiftMoon | read-only; continues their atlas-kit |
| ~~R-CS~~ **RESEARCH-COMPLETE (doc banked at coordinator checkpoint; successor queued)** (E1-holder-end cross-note folded: what [csound+0x4] stores answers the R1-alias-vs-R2-theft question from the census side — first PARK pull generating an arbitration feed on day one) | CSound class census | scope locked in task file |
| ~~R-AMX~~ **CLAIMED 7de33f4 — WildZenith, in progress** (scope adds a CONFLICT SETTLEMENT: T-070's 'SendMessage x3 among 0x550720 callers' vs T-080's 'trio pushes mailbox DIRECT via sub_54EE60' — dual-path or loose label, xrefs decide; + completeness METHOD for the native table must be stated, item-41 style) | AMX-natives -> queue-surface map | as filed |
| R-S9 | Gfx_Engine top-5 follow-ons: s9 device-teardown twin ([this+0x894] loop), s101/102 private-texture-id chain, and the D3D8 slot-numbering UNASSERTED note from b5 — decode via the +0x40 MapFile obj | EpicRaven | offline/cache; share-check/field discipline as usual |

## QUEUED (trigger-gated)
| id | scope | trigger |
|----|-------|---------|
| R-ST | StringTree/multi-host embedded-component split annotations (T-059 split queue + T-086 GfxCameraHolder decode notes) — annotation wave | after Q-W2 |
| X-PUSH | T-084 tier-2 (141 mints, 9 READ-BACK-FIRST) + 82 foreign-annotations batched | after Q-W2 |
| S-PEND | Sitting-dependent: GameCursorManager +0xA4 fill, event-kind NUMERICS, raiser/bp10 verdict fold (T-050), >4 depth, T-063-residual 801 cross-refs | after T-077 sitting |
| H-IDX | tools/ea-index.py refresh + --check gate green (stale since eea66da era) | any idle hand |
| M-M001 | HUMAN: d3d8.lib decision — gates T-013 steps 4-5, s118 flush decode | needs the user |

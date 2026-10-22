# PARK — standing work queue (coordinator-maintained; lanes PULL by claiming the named task file + relaying)

Rules (see tasks/README.md § Lane contract): every closed row names a PARK successor or goes on record DONE-NO-SUCCESSOR. Lanes pull from the top; coordinator assigns at checkpoint. Task files are created AT CLAIM — a PARK line is a scope, not a file.

## READY NOW (no deps, no slot conflicts)
| id | scope | lane-fit | notes |
|----|-------|----------|-------|
| P-REV | Product revision: T-071 comment-block rewrite to transient-idle semantics + Message kind-7 + the 0x2C alloc erratum; digest-count implications; SpawnMove asserts extended | KeenStorm | rides the T-080 §3 numbering inventory; touches MapEntitySpawn — sole-product-writer rule: no other src/ claims open |
| Q-W2 | WRITER WAVE-2: T-084 tier-1 — 182 mints across 22 hosts (term-field keyed, share-check-PASS pre-gated; batches by host OK) | TrueCastle (CalmRaven's first-in -> wave-3 on return) | writer slot free since wave-1 close |
| R-VFM | VFX_FormatManager anatomy: load/parse method chain (cls_0x4fd780::dtor-is-load case), .veg spec conformance vs van-buren docs, VFM size ≥56 live-vs-export erratum | SwiftMoon | read-only; continues their atlas-kit |
| R-CS | CSound class census: 0x539xxx band (T-062 named-excluded), export size 281 vs 0xF8-instance inflation case, full member/slot anatomy | HappyStorm | read-only; feeds cursor+audio port era |
| R-AMX | AMX-natives -> queue-surface map: which script natives touch WHICH of the four queues (SendMessage trio done in T-080; the Queue_* family 267-site feeders grouped natively?) | WildZenith | read-only; completes their atlas consumer-side |
| R-S9 | Gfx_Engine top-5 follow-ons: s9 device-teardown twin ([this+0x894] loop), s101/102 private-texture-id chain, and the D3D8 slot-numbering UNASSERTED note from b5 — decode via the +0x40 MapFile obj | EpicRaven | offline/cache; share-check/field discipline as usual |

## QUEUED (trigger-gated)
| id | scope | trigger |
|----|-------|---------|
| R-ST | StringTree/multi-host embedded-component split annotations (T-059 split queue + T-086 GfxCameraHolder decode notes) — annotation wave | after Q-W2 |
| X-PUSH | T-084 tier-2 (141 mints, 9 READ-BACK-FIRST) + 82 foreign-annotations batched | after Q-W2 |
| S-PEND | Sitting-dependent: GameCursorManager +0xA4 fill, event-kind NUMERICS, raiser/bp10 verdict fold (T-050), >4 depth, T-063-residual 801 cross-refs | after T-077 sitting |
| H-IDX | tools/ea-index.py refresh + --check gate green (stale since eea66da era) | any idle hand |
| M-M001 | HUMAN: d3d8.lib decision — gates T-013 steps 4-5, s118 flush decode | needs the user |

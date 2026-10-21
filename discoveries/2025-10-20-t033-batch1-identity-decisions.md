# 2025-10-20 — T-033 batch 1 (happywolf side): identity decisions for the top-6 census classes

Joint with IronStorm (mainline). Per row: data-before-debugger evidence, name with rationale,
and — the acceptance measure — expected tier delta. I supply decisions+evidence; renames into
IDB/`artifacts/F3.json` and census re-run go to ONE hand (IronStorm) so we never double-write.

Census source: `2025-10-20-session37-t029-function-census.md` T2 tier; biggest six:
`cls_0x4d8d70` 138, `cls_0x50dbb0` 120, `cls_0x55dc80` 94, `cls_0x50db20` 79,
`cls_0x424a70` 54, `cls_0x4542d0` 50.

| cls | decision | evidence (all pre-existing DB/data, no hunch) | name proposed |
|---|---|---|---|
| 0x4d8d70 | **NAME** | `modules/openje/docs/je/cls_0x4d8d70.md`: `_Tree`/`_Tmap_traits` layout, 0x30 nodes with `_Color`/`_Isnil`, SSO key string in node; my sess-40: same type as command registry (`_stricmp` order, dup→FatalError "Command %s already registered.") and `World::gap90[72]` instance | `JE::StringTree` (instances named at global sites, e.g. command registry) |
| 0x50db20 | **NAME** | byte-verified SSO string at 5 call sites: `meth_0x401bd0(dst,s,strlen)` ctor, capacity-vs-0x10 inline tests @0x59f64f/662/580c0a/580c55, 0x1C-byte elements ×12 in Entity `gap1AC` (sess-20), `{SSO string}` FQUE pushes (sess-36 §script-queue) | `JE::String` |
| 0x50dbb0 | **ESCALATE (tentative name)** | 32B, fields +0x8 ptr + +0x10..+0x1c begin/end/cap quartet, `meth_0x55aa60` push_back-shaped — container reads; my own corpus already warned "generic container impl, 136 methods — do not" (dialogue doc :78). **136 "methods" scattered over 0x425..0x5cb = IDA over-attribution signature, same as 0x4542d0.** Naming retires maybe 20 real fns, not 120. | hold until member-body sampling separates real methods from stolen helpers |
| 0x55dc80 | **ESCALATE to IronStorm's data lane** | 112B, 28 members; usage dense in 0x51f-0x521 = **the T-013 texture-registry band** (`meth_0x51f570` lives inside it) + `mbr_0x4` hit from GameActionAttack band 0x403xxx — smells Gfx frame/quad record, but the only strong claim I can make is "registry-adjacent". His T-013 lane has the node evidence; I don't. | IronStorm decides; cross-link T-013 row |
| 0x424a70 | **UNRESOLVED (assign after member read)** | 292B, 54 members, methods contiguous 0x424a70..0x42fd60 (GUI/widget band of sess-32/33 hit-test docs); size+band = window/widget-shaped, no blob/string evidence yet. Naming it now would be the row's forbidden hunch. | needs one member-census pass |
| 0x4542d0 | **NAME-BLOCKED: fix attribution first** | 24B/6 members but "54 methods" scattered 0x421fa0..0x5984f0 across seven unrelated subsystems, 3 ctors — impossible for one 24-byte class. This class's T2 count is **tooling artifact**; naming it changes no behavior, just quiets a census row — the row's "batch was decoration" case, caught BEFORE the batch. | keep anonymous; file as IDB hygiene item |

## Expected tier math (acceptance pre-registration)
Solid names = 2 (4d8d70, 50db20) → honest retire ≈ 217 fns IF their 138+79 counts survive the
over-attribution check (50db20's own 79 may be inflated the same way — verify by method-address
contiguity: string helpers cluster 0x401bxx-0x401xxx + 0x580b00-adjacent; scattered ones don't
count). The other ~1,900 T2 fns wait for the CONTAINER family (50dbb0/55dc80), where real wins
hide inside over-attribution noise. Predicted delta: 26.4%→~23% if counts survive, →~24.5% if
inflation holds. Recording the prediction so the census delta can be judged against it, not
retro-fitted.

## Boundary statement (tool rule, in prose)
- Evidence sources are IDB/DB content + my committed docs, cited above; nothing from the debugger.
- "over-attribution signature" = inference from scatter pattern vs size; it is a claim about IDA's
  type attribution, not about the shipped binary — which is why two of six entries refuse to name.
- `JE::String` here means the 0x1C SSO string; the 0x18 MSVC stdstring inside TREE NODES (openje
  doc) may be the SAME type at another instantiation site — do not create two string types until
  sizes reconcile (0x1C vs 0x18 = one extra DWORD; unresolved ⚠).
- IDB writes + F3.json regen: IronStorm's hands this batch; this doc is the decision input only.

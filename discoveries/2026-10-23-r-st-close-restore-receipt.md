# R-ST close pass — verify + RESTORE receipt (splitseat, 2026-10-23)

Claim: PARK R-ST row (StringTree/multi-host embedded-component split annotations — T-059
split queue + T-086 GfxCameraHolder decode notes, gate "after Q-W2").

## What the verify found (the honest wrinkle)
The R-ST wave was already executed by HappyStorm 01a16092 (commit abaf6b5: 6 comments,
readbacks 6/6 at 00:34:53Z). BUT the live IDB verification pass (this sitting, 18:2x-18:4xZ)
found **every one of those annotations GONE**: the repeatable func-comment slots at all six
sites now held only the PRIOR NAME string left by later rename waves
(0x4b2dd0/0x56ee40 → `std::_String_base/_Two_string_struct::dtor_*` renames; 0x4c0190/
0x4a5be0 → T-086-era GfxCameraHolder/Gfx_AnimMeshInstance ctor mints; 0x58be40/0x58f010 →
JE::Client virt renames; instr comments at 0x58f0cf/0x58f114 empty).

ROOT CAUSE (mechanism, now a standing hazard): **a rename wave preserves the old name in a
repeatable comment, and that SET OVERWRITES any prior repeatable comment on the fn.**
Every rename-after-annotation silently destroys the annotation. Consequence recorded as
successor law: annotation waves must read back AFTER intervening renames, or land last.

## RESTORED (8 sites, all read back exact)
- 0x4b2dd0 `std::_String_base::dtor_0x4b2dd0_4b2dd0` — T-059/T-060 §6 split-reading (SSO
  dtor, 2,322 fan-out, NOT a StringTree member, b3 MINT unaffected)
- 0x56ee40 `std::_Two_string_struct::dtor_0x56ee40_56ee40` — T-060 §6 two-SSO-block dtor
- 0x4c0190 `JE::GfxCameraHolder::ctor_0x4c0190` — T-086 §4.1 six-host census + new(0x280)
- 0x4a5be0 `JE::Gfx_AnimMeshInstance::ctor_0x4a5be0` — T-086 §4.2 4 heap ctor sites
- 0x58be40 `JE::Client::virt_meth_0x58be40_58be40` — CDebugInterface FACTORY identity
- 0x58f010 `JE::Client::virt_meth_0x58f010_58f010` — both store sites from the head
- 0x58f0cf (instr) — window #1 -> [client+0x8C]
- 0x58f114 (instr) — window #2 -> [client+0x90]

Note: restoring replaced the old-name preservation strings at fn heads (derivable from EA
`cls_<ea>` form; the annotation evidence outranks the auto-preserved label).

## Tooling fix landed in the same sitting (harness side)
- `.pi/extensions/ida.ts`: `action=call` no longer trusts top-level `tool` (the harness
  arg-envelope strips it — four consecutive calls arrived params.tool=falsy while no-tool
  calls ran fine). Resolution: `srvtool` alias → legacy `tool` → shape inference
  (queries→lookup_funcs, code→py_eval, batch→rename, items→set_comments, addrs→xrefs_to, …),
  with an explicit error naming the ambiguity for srvtool when shapes collide.
- Upstream `py_eval` scoping bug (exec_globals/exec_locals split → comprehensions cannot
  see `;`-chain names) worked around in-band via `exec(compile(b64(...), '<ida-eval>',
  'exec'), globals())` payload pattern; upstream package is nix-store read-only.

## Save
save_database proven 18:41:09Z (ledger artifacts/ledgers/ida_original_f3-save-2026-10-23T18-41-09-460Z.jsonl),
10,173 fns unchanged, comments only — rename-tier purity held (T2 delta 0 this pass).

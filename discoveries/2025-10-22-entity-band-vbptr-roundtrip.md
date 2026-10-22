# Entity band 0x1DC/0x1E0/0x1E4: the vbptr round-trip, closed statically (2025-10-22)

Lanes: JadeBear (census + proof), WildKnight (port consequence, commits `0440439`/`22f1f29`,
suite floor 58, run-all 13/0). This note consolidates the RE-side chain so the B4 deferral's
proof obligation has one citable artifact. Every byte claim was re-read from
`$OPENVB_ORIG_EXE` (image base 0x400000) at write time.

## The two questions that looked contradictory

1. Census question: what writes `Entity+0x1E0`? **Answer: the lifecycle STATE dword.**
   Image-wide raw modrm scan (89/88/C7/C6/D9/DD, mod=10 disp32=0x1E0, SIB decoded,
   ~24 `[esp+idx+1E0h]` stack forms excluded): exactly 20 object stores. Three are
   Entity-era and all three are state writers: `ctor_0x5513a0@0x551475`, `ChangeState`
   `meth_0x5507f0@0x550945` (five verbatim `"Invalid state change in Entity::ChangeState()."`
   strings + transition table 0..8), `UnpackState_54e690@0x54e733`. The other 17 belong to
   Gfx/VFX/dialogue/interface classes (all attributed; full list in the T-079 session
   relay). Export agrees: `.?AUEntity@@` HAS `mbr_0x1e0` (base:true, size 4, 9 usages incl.
   getter 0x549030 = `[this+1E0h]==7`). NOTE the key family: plain `Entity` exists ONLY under
   the mangled key; cls_-style queries miss it — that is how "F3.json records no member at
   +0x1E0" was concluded and it was wrong.
2. SetHP question: `F3::SetHP` (0x45b2c0) resolves its subject via
   `8b 50 04 / 8b 52 04 / 8b 84 02 e0 01 00 00` (0x45b367-6d) — `[obj+4] -> [+4] -> [obj+disp+1E0]`.
   If `+0x1E0` is state, the resolver would be fed a 0-8 lifecycle value. Ugly; wrong model.

## The resolution: the 0x1E0 is NOT Entity-relative

`0x45b347: __RTDynamicCast(Entity -> GameEntity)`. The read base is a **GameEntity**, and
`GameEntity::ctor_0x4678a0` installs a vbptr at `+4`:

- `0x4678d4: c7 46 04 34 61 66 00` = `mov [esi+4], 0x666134` (vbptr; gated on the
  complete-object flag `param_2_00`).
- vbtable `0x666134`: `[-4 self-marker, +4 entry = 0xa0 = 160]` — read raw at 0x666138.
  The primary vftable `0x666130` is exactly ONE slot (`0x4080c0` — GetMaxHP, i.e. the
  B3 vft-slot-0 finding is this single-slot primary), and the vbtable follows it immediately.
- `0x4678fb/fd: 8b 48 04 / c7 44 31 04 44 60 66 00` = `mov ecx,[eax+4]` then
  `mov [ecx+esi+4], 0x666044` — the compiler itself defines Entity-base ≡ `this + delta + 4`
  (the secondary vftable install; no data read needed to prove the arithmetic).
- Witness: the birth idiom at `0x46797e` calls `Entity::virt_meth_0x550750(this+4+160, 5)`
  = `this+0xA4` = Entity-base (`&this->gap9D[7]` = 0x9D+7 = 0xA4), ChangeState(5).

Then SetHP's subject = `this + 160 + 0x1E0` = `this + 0x280` = **Entity-base + 0x1DC**.
The opcode's `0x1E0` = `0x1DC + 4`: MSVC folded the vbptr-slot offset into the disp32
(the ctor adds `+4` to the delta; SetHP adds delta to `this` and compensates in the
immediate). **Convention rule for this binary: a vb-displaced read at `[this+d+disp]`
means subobject-relative offset `disp - 4`.**

Secondary-table detail: `0x666044` slots 1..3 (`0x54dea0/0x549240/0x549cd0`) are the SAME
bodies as plain-Entity `0x692084` slots 1..3, and slot 1 (`0x54dea0`) is one of the nine
`mbr_0x1e0` usages — the state semantics carry into GameEntity by identical dispatch,
not just identical layout.

## Identity: the resolve is a provable round-trip, kept on purpose

`meth_0x46ee10(handle) -> meth_0x54be10` list-walk compares `*(entity+476)` (= 0x1DC)
against the handle; `0x54be41` is the MISS path only (map probe, node+0x10 or 0).
Handle is stamped once at `Create 0x55190c` (T-043) and restored 1:1 by UnpackState, so
`e -> e.handle -> e` is identity for every registered entity; **a "borrowed handle" is
impossible**. Unregistered own-handle => silent `return 0` at `0x46ee23`/`0x46ee3e` (no
error string — the shipped failure is observable only by ABSENCE of writes).

Faithfulness decision (WildKnight, endorsed here): the port KEEPS the round-trip.
Collapse-on-success differs exactly where the difference can be seen (the failure path),
and the re-resolve doubles as a runtime integrity check of the port's own handle
registry: if Create-side bookkeeping ever drifts, the one-step form silently clamps the
wrong object while the round-trip fails the way shipped does. Cost is one list walk per
native call — noise against the per-frame entity pump the engine already models.
Suite asserts both branches: identity landing + failed path writes nothing.

Clamp side-effects for the band: `0x4629c0` is an UPPER-bound-only clamp (`mbr_0x24` HP =
min(param, `mbr_0x20` max) — HP CAN go negative through this door; combat lanes must not
assume HP >= 0). Tail `meth_0x46d620(handle, value)` = packet-88 fan-out, named-deferred
with the server era; HP is a local write until then, stated in source and docs as a known
divergence.

## Adjacent finds, verified same session

- `GetWeaponCombo` (0x45a9f0) is a SHIPPED STUB: every path returns 0 (`0x45aa34
  xor eax,eax`), the SUCCESS path included — `local_4` is resolved at 0x45aa1a and
  discarded; the "unable to find" message prints and still returns 0. Port rule: keep it
  a stub; returning the object would be a behavior difference dressed as a fix.
- `GetSquadAttitude`/`GetTeamAttitude` (0x45aa90/0x45aaf0): self-attitude by identity
  `params[1]==params[2] -> return 1` (0x45aab8) before consulting anything; callee is the
  same WSI vb-idiom `&WSI[1] + *(WSI[1].mbr_0x0 + 4)` (0x45aacf/0x45ab2f), slots 34 (squad)
  / 33 (team). `GameEntity::mbr_0x28` (server relationship) is stamped at ctor `0x4679aa`
  with the same idiom — one model names both the SetHP subject and the attitude callee.
- `GetPerkRank` (0x45ab50) ships a copy-paste bug verbatim: its resolve-failure line is
  `"GetPosition() was unable to find the requested entity."` (push at `0x45ab88` — docs
  that cite `0x45ab8e` land on the displacement byte; EA±1 class, now 4 samples).
- IDA naming hazard: `GameActionEquip::` on `0x54be10/0x46ee10/0x4629c0/0x46d620` and the
  WSI singleton is `cls_0x616328` contamination — bodies correct, class labels wrong;
  family is server-side era. Do not propagate the label into docs (stale instance at
  `src/F3/Commands.hpp:81` routed to its owner, not edited here).
- Port-test law from B4's first run: a field re-declared by a sibling header
  (`GameItem.hpp` re-declares +0x1DC/+0x20/+0x24) reads ZERO through the wrong owner type —
  "read/write shared fields through the OWNER type" is now a rule; register family:
  sibling-redeclaration instances (3rd), alongside the shared-pad items 43/44.
  Related: repro prints that evaluate state before a call must be sequenced, not
  one-lined (printf argument-eval order masqueraded as `ptrmatch=0` resolve failure).

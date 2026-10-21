# T-013 port design — stem-keyed Gfx_Texture_Manager into the existing seam (ordered plan, per-step acceptance)

**Author:** KeenStorm · **Date:** 2026-10-21 · **Status:** design-only (no code, no `src/` commits)
**Purpose:** when M-001 slice 1 lands (the human/device block), the registry port should have ZERO
thinking cost: every decision below is already evidence-backed; each step is small, independently
gated, and lands with its own asserts. This doc changes no behavior by itself.

**Evidence base (all committed/accepted):** `discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md`
(G1/G2, key-form), `discoveries/2025-10-21-t013-static-tail-plus0x2c-broadcast-keyform.md`
(frame index, vertex blocks, STEM-vs-VERBATIM decision), `discoveries/2025-10-21-t062-cursor-subsystem-and-child-notify.md`
(stem recurs from the sound subsystem; TWO-KEY-POLICIES rule), board law in
`tasks/T-013-engine-texture-registry.md`. Seam already ported and asserted (openje `a166dfa`):
`GUI::TextureSlots` + `BindFragmentTexture`/`ResolveSkinTexture` + `InterfaceFile::SetTextureSlots` +
`SkinTable::ResolveTextures` + `ResourceSystem::ResName`.

## Scope freeze

IN: `JE::Gfx_Texture_Manager` (the 0x70B200 class), its plug into the existing `TextureSlots`
seam, the ResourceSystem stem-alignment that the shipped chain silently performs, and the measured
miss reproduction.
OUT (separate rows / never): `GameCursorManager` (T-062 delivered its facts; port it when the
cursor behavior gets its own row), the `CSound` cursor-sound side, event-kind numerics (T-056
BONUS-4), ABI mimicry (no `ret 0x20` thunks, no 14-arg SEH forwarders — behavior only), the
0x539xxx full CSound class.

---

## Step 1 — ResourceSystem stem alignment (no GUI touch, no registry yet)

**Why first:** the shipped chain never sees the author string twice: `Demand_0x49b750` resolves
the name IN PLACE (`sub_499F10` mutates the caller's string at 0x499f89) BEFORE copying it to the
Res (0x49b948) and BEFORE caching via the canonical key `v6` (`meth_0x50bb00`). Our port currently
stores the RAW requested name (`ResSetName(res, name)` @Demand_0x49b750, ResourceSystem.cpp) and
cache-finds on `_stricmp(raw)`. Every later step's keying depends on this being right, and it is
testable with zero new classes.

**Change (behavior, not bytes):**
- Resolve at Demand entry: stem = substring before LAST '.' (rfind shape, 0x458a20 seeded with
  pos=len, backward scan; NO dot → keep full string, ftype-unknown path); case PRESERVED (both
  0x401b20/0x401c70 are byte copies).
- Store the resolved stem as the Res name; cache Add/Find keyed on it (shipped `v6` is the RHT
  canonical string — our cache can key on the stem we store; do NOT case-fold the stored stem).
- `ReadResource`/`ResourceExists` internal resolution already strips (ResourceSystem.cpp
  ~221-231) — reuse that helper, do not duplicate the table.

**Acceptance:** new ResourceSystem-suite asserts — `Demand("foo.bmp")` → `ResName(res)=="foo"`;
`Demand("a.b.bmp")` → `"a.b"` (last-dot, shipped `0x458a20` shape, moot-for-shipped-data note
stays a comment); `Demand("FOO.TGA")` → `"FOO"` (case preserved); re-Demand cache hit returns the
SAME Res by stem; all pre-existing 111+ floor asserts still pass; mingw suite + msvc rc0.
**Risk named:** shipped's cache-key is the RHT *canonical* string (`v6`), which may differ in case
from the authored stem if the rht pool stores lowercase — our FindRht already stricmps; if a
future lockstep shows case-normalized canonical keys, this step's storage rule gets one line
change. Documented as the step's only open thread.

## Step 2 — `JE::Gfx_Texture_Manager` (the 0x70B200 class) as a standalone unit

**Layout decisions (behavior mirror of the G2 evidence — do NOT copy offsets unless they help
tests):** name-keyed intrusive list (shipped `mbr_0x28` walk @0x51f58f, wrap @0x51f63d) + id→object
map (shipped `+0x24/+0x28`, lookup `0x51f160`) + per-node `{id (node+0x0C), name string (node+0x10)}`
+ per-texture-object use-count (`obj+44`, bump @0x51f679). Port keeps OUR natural member naming;
EAs live in comments (per harness culture), not in identifiers.

**API = exactly the three seam operations + ownership:**
1. `Probe(const char* name, int* out)` — the `+0xDC 0x51f570` shape: walk list, compare
   **BYTE-EXACT CASE-SENSITIVE** memcmp (`0x41e4b0` `repz cmps` + length gate) against node keys;
   hit: `*out=id`, `++useCount`, return nonzero; miss: `*out=0`, return 0. NEVER inserts.
   This is POLICY 1 (case-sensitive stem table) — file-comment cites T-062's two-key-policies
   rule with the cursor manager named as the case-INSENSITIVE counterexample that must NOT be
   merged.
2. `GetOrCreate(const char* stemOrRaw, void* data, unsigned size)` — the `+0xC4 0x5c0a90→0x521ca0`
   shape: internally RE-PROBES first (0x521cb1 — the dedupe that makes the GUI's structural miss
   harmless), on miss creates the texture object (dims step deferred to Step 4/M-001: ctor takes
   dims from the device decode; until then this param is the seam's), returns id ≥ 0, or NEGATIVE
   on decode failure (callers clamp).
3. `Object(int id)` — the `+0xD8 0x51f160` shape: id→object, NULL for invalid (this is what the
   LoadDefault gate consumes).
`Release(id-or-obj)` decrements the shipped use-count only where shipped does (the bump is at
acquire; no GUI-side release path exists in the shipped chain — T-007 G2 showed no `--` near
0x51f679 for the GUI lanes; port reproduces the count as INPUT and does not invent decrements).

**Acceptance:** new host suite (T-004 family, same row pattern as `cls_0x4d8c10`): synthetic
nodes → probe case-sensitivity matrix (`"foo"` hits `"foo"`, misses `"FOO"`/`"foo.bmp"` — the
exact shipped asymmetry), GetOrCreate dedupe (two raw dotted names → one node, second returns
first's id via internal re-probe), Object(invalid)=NULL, useCount after hit =1. No GUI includes;
msvc rc0 (new TU = cmake row edit → build-affecting: two-lane-capped — coordinate the row edit
with whoever holds the cap; note in PR body).

## Step 3 — plug the manager into the existing seam (the actual "port")

**Change:** construct the manager at engine-start (the shipped 0x70B200 singleton is ctor-
installed; ours lives under the Gfx device owner — same lifetime as `g_resource_system`). Then two
one-line wirings, both already test-proven paths:
- `InterfaceFile::SetTextureSlots(&realSlots)` at the GUI bootstrap call site (F3 layer, the same
  place that loads `Mainmenu.INT`), `ResourceSystem::ResName`-based `index` arg already matches
  shipped 0x4b21e8 (resolved name).
- `SkinTable::ResolveTextures(rs, realSlots, &rep)` at skin-apply time per record chain.
**Flip in the same commit:** the GUILoading suite's binding-off regression asserts (records == -1
unbound) STAY (default remains NULL slots); new asserts drive the real manager: Mainmenu all-named
records convert via demand (census print already exists), `areamap` 54 named → 54 demand-misses →
ids 0 (Step 5 pins the final numbers).
**Acceptance:** GUILoading stays green WITH the -1/-0/unbound split intact; the census line prints
real convert counts; no new TU (reuses Step 2's + existing rows); cppcheck 0; msvc rc0.

## Step 4 — device decode boundary (M-001-gated core, deliberately last)

`GetOrCreate` needs real dims from the TGA/BMP decode (shipped `meth_0x535a80` inside the 14-arg
descriptor path) to make `Object(id)` carry `dimW/dimH` (the +0x24→+0x1C/+0x20 reads of slot-16)
and a nonzero gate `+0x28`. This step is ONLY possible post-M-001 (device must answer what a
loaded texture reports). Until then Steps 1-3 are fully landable with a dims-parameterized manager
(test fixtures already parameterize it). **Acceptance:** one real `.tga` from the rht round-trips
demand→GetOrCreate→Object with dims == shipped header values; `TextureMisses` interplay asserted
(absent textures still quiet-zero, never crash).

## Step 5 — measured-target reproduction (the row's headline number)

With Steps 1-4: assert `TextureMisses 0/0/3/54/0` (SkinTable, five shipped skins) AND the new
registry-side census: areamap 54 named records → 54 demand misses → id 0 (the T-007 "54 absent"
target); 3 quickslot misses reproduce identically. **Acceptance:** GUILoading asserts with these
exact numbers (no floor raise needed — adding asserts under an existing floor is free; floor bump
only if the row list changes, which needs the two-lane cap).

## Cross-cutting rules baked into every step

- EA-anchored comments everywhere; no new identifiers with `0x` hex names in this subsystem
  (item 42 — the seam predates the rule; do not propagate).
- Behavior port, never ABI: no 0x20-pops, no SEH frames, no 14-arg forwarding.
- Key policies: registry = case-SENSITIVE stem memcmp (T-013); cursor manager (future row) =
  case-INSENSITIVE stricmp verbatim (T-062). A single shared "name key" utility is the forbidden
  shape; two implementations, two comments.
- Gate every step: `tools/gate-integrity.sh` rc0 + `run-all-tests` 9/9 + `cppcheck-openje` 0 +
  `msvc-build` rc0 (headers touched). Mingw-green is never product-green (item 37).
- Submodule order: commits inside `modules/openje` first, parent pointer bump after.

## What this design deliberately does NOT decide (honest gaps)

1. The exact owner object that constructs the manager at runtime (shipped: a ctor stores
   0x70B200 — the owning singleton's class is F3-side, likely Gfx_Engine itself; until a live boot
   shows it, Step 3 places it under the device and notes the alternative).
2. Corner-field-0 index semantics and frame interpolation (render-side, T-056 BONUS list — does
   NOT block the registry; the port keeps dDC/dF8/d114 as the COPY per ruling).
3. Who fills Window+0xD24/+0xD40 cursor-sound strings and GameClient+0xA4 (T-056 BONUS-5; cursor
   port row only).
4. Whether the RHT canonical key case ever differs from authored case (Step 1's single open
   thread).

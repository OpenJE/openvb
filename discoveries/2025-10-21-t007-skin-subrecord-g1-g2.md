# 2025-10-21 — T-007 steps 1–2 (gaps G1/G2): skin sub-record class + `0x707CE4` slot semantics

Agent: KeenStorm (research row, T-007 steps 1–2 only). Target: **ORIGINAL_REFERENCE**
(`ida_original_f3`, `artifacts/F3.exe.i64`, raw bytes `$OPENVB_ORIG_EXE`). **Read-only** — no
renames, no comments, no type edits (GoldYak sole IDB writer this wave). Every claim below is
anchored to an EA + instruction bytes; class/vft facts cross-checked against `artifacts/F3.json`
(frozen 2024 OOAnalyzer export, T-040 — `cls_0x…` labels there mean "no RTTI name in 2024").
Per GoldYak's live name-restore window, nothing here depends on DB *names*, only addresses.

The G1/G2 labels in this file follow the COORDINATOR'S assignment (G1 = sub-record class,
G2 = `0x707CE4`); note `docs/f3/GUILoadingRecon.md` §7 uses them the other way round (§7 G1 =
the global, §7 G2 = the sub-records). Same two questions, verified against the bytes.

---

## G1 — the inline sub-record at `Window+0x90` is `GUI::Fragment` (0x158-byte object)

**(a) "Stride 86" means 86 DWORDs = 0x158 = 344 bytes.** Three independent encodings:

| EA | bytes / mnemonic | what it proves |
|---|---|---|
| `0x4b08b1` | `push 158h` | `obj_size` param of the vector-ctor call @0x4b08c2 |
| `0x4b12b6` | `add ebx,158h` (`81 c3 58 01 00 00`) | element stride in the `Window::ReadFrom` sub-record loop |
| `0x63212e` | `push 158h` | same size in the `eh vector destructor iterator` @0x63213c (SEH cleanup of `Window::ctor_0x4b0810`) |

**(b) The ctor installs 9 `Fragment` objects at +0x90.** `JE::GUI::Window::ctor_0x4b0810`
@`0x4b08a5..0x4b08c2` pushes `{dtor 0x4b2680, ctor 0x4b24a0, length 9, obj_size 0x158,
p_array = this+0x90}` and calls `??_L@YGXPAXIHP6EX0@Z1@Z` (eh vector constructor iterator).
`Fragment::ctor_0x4b24a0` @**`0x4b24d6`** stores vftable **`0x66ea1c`** into each element's +0
(IDA symbol for that store target: `GUI_Fragment_GUI_Base_vftable_66ea1c`).
So the vft installed at `Window+0x90` is **`0x66ea1c`**, NOT the `GUI::Skin` vft `0x66f2dc` —
confirming the assignment's premise.

**(c) Slot 7 (byte offset +0x1C = the loader's "vft+28") of `0x66ea1c` = `0x4b20a0`.**
`F3.json` models `0x66ea1c` as 17 slots; index 7 → EA `0x4b20a0`. The `Window::ReadFrom_0x4b10f0`
loop reads it exactly that way — @`0x4b12a4` `mov eax,[ebx]` (element vft), @`0x4b12aa`
`call dword ptr [eax+1Ch]` (`ff 50 1c`-form `ff 90 1c 00 00 00`), counter bound `cmp ebp,9 /
jl` @0x4b12bc/0x4b12bf, start `lea ebx,[esi+90h]` @0x4b1297, fail-exit `test al,al / jz`
@0x4b12ad/0x4b12af. Arguments passed: `push edi` where `edi` = `ReadFrom`'s param_1 = the
**stream** (edi is the stream arg of every `FileStreamRead` in the same function, e.g.
@0x4b112c), and `[esp+18h+param_2]` = `ebp` = the **version a2** (gates `cmp ebp,2` @0x4b11a7,
`cmp ebp,3` @0x4b120a). This matches §2's "(stream, a2)".

**(d) Reader body `0x4b20a0` — wire→field layout** (element-relative; every `lea` operand read
off the disasm, SEH function, returns al=1 on success, 0 on any short read @0x4b22d6):

| # | wire item | stored at | EA of the store | cross-ref |
|---|---|---|---|---|
| 1 | u32 (mode/role, valid 0..7 per binding guard @0x4b14a4) | `+0x1C` | `lea eax,[esi+1Ch]` @0x4b20d8 | recon §8 `d28`; ctor default 2 @0x4b24e3 family |
| 2 | u32 (width) | `+0x20` | @0x4b20ec | §8 `d2C` |
| 3 | u32 (height) | `+0x24` | @0x4b210c | §8 `d30` |
| 4 | u32 (rate) | `+0x28` | @0x4b2126 | §8c: ini-authored, hence 14364/14364 zeros |
| 5 | u32 | `+0x2C` | @0x4b213a | §8 `d38`; **no other writer or reader identified — meaning not statically resolvable** (residual risk: a behavioural field we parse-but-ignore) |
| 6 | u32 (tile count) | `+0x30` | @0x4b215a | §8 `d8a` |
| 7 | u32-len + bytes → **texture-name string** | `sub_50B880` @0x4b2187 → temp 0x1C stack string (cap 0xF @0x4b216d, SSO select @0x4b21c5) | resolution chain below → id stored at `[[esi+0x128]+0x130]` @**0x4b2244** | §8c "second u32 at +8 → actually +0xC0" correction concerns #8, not this |
| 8 | u32 | `+0xC0` @0x4b2250, then **broadcast copy** of dword[+0xC0] to `+0xDC`, `+0xF8`, `+0x114` (3× stride 0x1C, loop @0x4b2260–0x4b2278, `mov ecx,3`) | | the copy target meanings are **not statically resolvable** (only these writers; residual risk: derived rects that a future parse must recompute) |
| 9 | u32 (packed colour) | `+0x154` @0x4b2283 | then `cls_0x4abc60::meth_0x4ab600(this=+0x70, args 1,color,0,color)` @0x4b22a1, `mov byte [esi+0x18],1` @0x4b22a8, notify `cls_0x4abc60::meth_0x4abdd0` @0x4b22ac | §8 `d154` colour-like; +0x18 style-written flag |
| 10 | 16-byte rect | `JE::GUI::Base::ReadFrom_0x4b2850(this, stream, version)` @0x4b22b9 | | §2 rect → Base+0x08 |

Element size check: highest referenced member `+0x154`+4 = **0x158** ✓ matches (a).

**(e) Texture-resolution chain inside the reader (the G1↔G2 seam), byte-verified:**
- probe `[[0x707CE4]+0xDC]` @**0x4b21bb** (ecx from global @0x4b21ae, `ff 90 dc 00 00 00`); hit
  (`test al,al / jnz` @0x4b21c1/0x4b21c3) jumps straight to the id store @0x4b2237 — **no demand**.
- miss: `Demand_0x49b750(0x707DA8 /*resource system*/, name, 1)` @0x4b21dd; null handle skips
  the convert (@0x4b21e4) and stores the pre-zeroed slot (out param initialised 0 @0x4b21a5).
- on a handle: builds a string from resource-record fields `[handle+0x20]`/`[handle+0x2C]`
  (@0x4b21e8–0x4b2208) and calls `[[0x707CE4]+0xC4]` @**0x4b221d**; result < 0 →
  `mov [esp+40h+arg_0],0` @0x4b2227 (clamp the id slot); `Release_0x498eb0(handle)` @0x4b2232.

**(f) `F3.json` artifact to note for T-040:** the export gives `GUI::Fragment` class **size 3384
(0xD38)**, which contradicts the ctor-proven object size 0x158 (§a/b). Max referenced member
+0x154 is consistent with 0x158; 0xD38 looks like a copy of the enclosing-widget-scale estimate.
Treat the export's size as wrong here; 0x158 is the byte-level truth.

---

## G2 — `0x707CE4` is the `Gfx_Engine` singleton pointer; the three slots' real semantics

**(a) Identity, byte-exhaustive.** Raw scan of the whole image for every absolute encoding of
`0x707CE4` (`c7 05`, `89 05/0d/15/1d/25`, `a3`) gives exactly **three writers**:
- **`0x5bfe33`**: `a3 e4 7c 70 00` = `mov [0x707CE4], eax` with `eax = this` — inside
  `JE::Gfx_Engine::ctor_0x5bfe10`; the same ctor stores vftable **`0x6ae7ec`** @**`0x5bfe43`**.
- `0x5bedd6`: `c7 05 …00000000` = clear; within the method range `0x5bec20..0x5bedef`.
- `0x5c1e24`: `89 1d …` = clear; within `0x5c1cd0..0x5c1e3f` (F3.json: `dtor_0x5c1cd0`).
`F3.json` attributes vft `0x6ae7ec` to class `Gfx_Engine` (size 2212, 120 slots). ⇒ **the global
holds the one `Gfx_Engine` instance; every reader of it goes through vft `0x6ae7ec`.**

**(b) Slot map (byte offsets on `0x6ae7ec` = 4×F3.json slot index):**

| vft byte off | slot | impl EA | verdict from this session |
|---|---|---|---|
| +0xC4 | 49 | `0x5c0a90` | **index = get-or-create**: SEH forwarder (15 stack args) that copies its first arg (a `char*` name: `lea eax,[esp+54h+arg_4]` @0x5c0af0) into a stack string, then `cls_0x55dc80::0x521ca0` on global **`0x70B200`** @0x5c0aff (IDA symbol `JE__Gfx_Texture_Manager__LoadTextureFromBuffer_521ca0`). `0x521ca0` semantics (decompile, byte-checkable): first calls the **lookup** `0x51f570` — if the name is registered, its id is written through the out pointer and returned; otherwise `operator new(0x34)` → `JE::Gfx_Texture::ctor_0x51f010` → `meth_0x535a80` with the 8-field descriptor (the 14 forwarded args), **negative return on load failure + `LogDebugString "Gfx_Texture_Manager::LoadTextureFromBuffer() failed to load: %s"`** @0x521d78, else name copied into the texture and registry-inserted (`0x5212e0` @0x521d4b). ⇒ returns registry index ≥ 0, or negative error — which is exactly what the callers clamp. |
| +0xD8 | 54 | `0x5c0930` | **handle→object**: `return cls_0x55dc80::0x51f160(*0x70B200, id)`; `0x51f160` = map probe on manager+0x24/0x28 keyed by id, returns `node[4]` (the texture **object** pointer) or 0 + `LogDebugString "TextureManager: requesting an invalid Texture Handle"` @0x51f185. Matches §8e's "returns the object, deref [obj+0x24] extents / [obj+0x28] gate". |
| +0xDC | 55 | `0x5c0970` | **probe = lookup-only acquire**: forwards to `cls_0x55dc80::0x51f570` @**`0x5c09a3`**, then frees the caller's SSO heap buffer if capacity ≥ 0x10 (@0x5c09aa). `0x51f570` (decompile): if the name string is empty or the list at manager `mbr_0x28` is empty → **writes 0 through the out pointer** @**0x51f643** and returns 0; else walks nodes comparing `node+0x10` string (via `sub_4AB1C0` + `0x41e4b0` memcmp-shape @0x51f5f5); on match **`*out = node[3]` (id, node+0x0C)** @0x51f66b, fetches the object via `0x51f160` and **`++(obj+44)`** (use count) @0x51f679, returns 1; wrap to head @0x51f63d = miss ⇒ out 0, ret 0. Never inserts. |

**(c) `LoadDefault` (sub_523490) skip/clamp/gate logic — the assignment's wording, settled at
byte level** (record base = `ebp`, set at function entry @**`0x5234a4 mov ebp,ecx`** — thiscall
`ecx` = the record; field stores `[ebp+0]` flags @0x523568, default-0 @0x5234b1, color
`[ebp+8]=0xFF808080` @0x523700 — so the §1 table's record-relative offsets are confirmed):

- record+0x04 is **pre-zeroed** @`0x523570` at the top of the `texture` block.
- `8d 45 04 50 ff 92 dc 00 00 00` @0x523637–0x52363b: push **`lea eax,[ebp+4]`** — a *pointer to
  the id slot* — and call `vft+0xDC`. ⇒ **demand skipped when +0xDC returns nonzero**:
  `3a c3 0f 85 8e 00 00 00` @0x523641 (cmp al,bl / jnz → 0x5236d7), jumping over Demand@0x523665
  and the +0xC4 convert. On a hit, the id is written *into record+0x04 through that pointer*
  (0x51f66b above). On a miss it wrote 0 (0x51f643).
- miss path: SSO name select `8b 44 24?…` @0x523649–0x52365c (capacity test `[esi+40h]`, inline
  `lea [ebp+4]` vs heap ptr), `Demand_0x49b750(0x707DA8, name, 1)` @**0x523665**; null handle
  short-circuits (@0x52366c) with slot already 0.
- `ff 92 c4 00 00 00` @**0x5236b4**: `vft+0xC4` returns eax = index; **index forced to 0 when
  negative**: `3b c3 7d 09 8b 44 24 10 83 c0 04 89 18` @0x5236ba–0x5236c5 (`cmp eax,ebx / jge
  0x5236c7 / mov eax,[esp+10h] / add eax,4 / mov [eax],ebx`). `Release_0x498eb0` @**0x5236ce**.
- then (and after every skip) @0x5236d3 `8b 6c 24 10` = **`mov ebp,[esp+10h]`** — ebp is
  **re-pointed at the same dword slot [esp+10h]**; @0x5236d7 `8b 45 04` loads `[ebp+4]`, zero ⇒
  skip; nonzero ⇒ `ff 92 d8 00 00 00` @**0x5236e7** `vft+0xD8(id)`; `3b c3` @0x5236ed and on 0,
  `89 5d 04` @**0x5236f8** clears `[ebp+4]`. ⇒ the third slot is a **validity gate**: if the
  handle resolves to no object, the id is dropped. (§7's earlier slot-list note attached no
  behaviour to +0xD8; §8e's "returns the object" is now confirmed at `0x51f160`.)

**⚠ Decompiler-disagreement note (coordinator ask #1, harness item 43 family):** the Hex-Rays
arg shape at `0x5c0970` (`virt_meth_0x5c0970_5c0970(int *a1, int a2, void *Block, ...)`) does NOT
match a clean "(name char*)" parameter — it synthesizes `param_2 = &a2` for the `0x51f570`
forward @0x5c09a3. The **verdict is taken from the call-site bytes at 0x523637/0x4b21ba and the
0x51f570 body**: the single pushed pointer is the **out-id slot**, and the name is read from the
adjacent caller-stack location (a byval-string/stack-adjacency ABI artifact). Net behaviour
(lookup, writes id through arg1 on hit, returns 1/0, bumps use count) is confirmed from the
implementation bytes 0x51f643/0x51f66b/0x51f679, independent of the naming. The exact byte offset
of the name relative to the pushed pointer (P+4 vs caller-[esp]) is the one ABI detail NOT
claimed here — that belongs to T-013's registry-internals scope.

**(d) Relationship between `0x707CE4` and `0x70B200`:** all three slots are **thin delegations**
from the `Gfx_Engine` instance to the `cls_0x55dc80` global at `0x70B200` (the texture-manager
object; IDA's restored symbol for the 0x521ca0 callee calls it `JE::Gfx_Texture_Manager`).
Registry facts now statically confirmed (narrows T-013's G9): entry `id = node+0x0C` (v6[3]
@0x51f664), `name string at node+0x10` (v6[4] read via sub_4AB1C0 @0x51f5bf), `use count at
object+44` (0x51f679), intrusive list head at manager `mbr_0x28` (@0x51f58f/0x51f63d), id→object
map at manager+0x24/0x28 (0x51f174-0x51f182).

---

## Verdicts vs prior record (coordinator ask #2 — no old-doc edits by me)

1. `docs/f3/GUILoadingRecon.md` §1: "`vft[+0xDC](name)` … probe — nonzero skips the demand" —
   **confirmed**; the `(name)` shorthand is incomplete: the byte-verified pushed arg is
   `lea eax,[ebp+4]` (the **out-id pointer**), name flows via the string adjacent in the caller
   frame; "nonzero skips demand" is exactly `0x523641 → 0x5236d7`.
2. §7 "G1 … +0xDC acquire-by-name … outputs node[3] as id" — **confirmed** (0x51f66b) and
   extended: it also bumps the object use count (0x51f679) and frees nothing on hit; the
   `a2`-shaping at 0x5c0970 is a decompiler artifact (see ⚠ above).
3. §7 "+0xC4 … callers treat the result as a texture index and clamp negatives to 0" —
   **confirmed**, and the reason negatives exist is now identified: `0x521ca0` returns the
   negative status of `meth_0x535a80` on load failure @0x521d54 branch (it is get-or-create,
   not a pure lookup).
4. G1's "not GUI::Skin" premise — **confirmed** and closed: vft `0x66ea1c` stored @0x4b24d6 by
   `ctor_0x4b24a0` (installed 9× via `eh vector constructor iterator` @0x4b08c2, obj_size 0x158);
   slot 7 = `0x4b20a0`.
5. NEW correction candidate for the coordinator to issue: `artifacts/F3.json` `GUI::Fragment`
   **size 3384 (0xD38) is contradicted** by byte-level `0x158` (0x4b08b1/0x4b12b6/0x63212e and
   max-member 0x154). Flag for T-040 (frozen-export errata), docs/notes should keep 0x158.

## Explicitly not statically resolvable (residual risks)

- **Which exact string bytes reach +0xDC at runtime** (stem vs `name.tga`) — unchanged from §8's
  note; needs one lockstep observation (T-013's). My work confirms the *slot plumbing*, not the
  key form.
- **+0x2C dword** (wire item 5): written by the reader, no other site found — no meaning claimed.
- **`+0xDC`/`+0xF8`/`+0x114` broadcast targets** (wire item 8): only the 0x4b2260 copy loop writes
  them; whether they are derived rects/caches/clone slots is unresolved. Risk: a port that
  ignores them must still reproduce the *copy*, not invent values.
- **The precise name-pointer ABI of +0xDC** (offset of the string relative to the pushed out-id
  pointer, P+4 vs caller-esp-adjacent) — flagged ⚠ above; matters only for a byte-exact reimpl of
  `0x5c0970`'s callers, which is T-013's seam.
- **`cls_0x55dc80` (0x70B200) class identity**: F3.json 2024 export has no RTTI name for it; the
  in-image symbol at the 0x521ca0 callee says `JE::Gfx_Texture_Manager` (a *current-IDB* name being
  restored by GoldYak — treated as corroborating, not primary, evidence per EA-only discipline).

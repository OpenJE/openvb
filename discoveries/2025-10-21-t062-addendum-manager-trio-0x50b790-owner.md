# T-062 addendum — the 0x50B790/0x50B7F0/0x50B880 trio: no owner class, and the +0x18 map is INTEGER-keyed (no third policy)

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Row:** T-062 follow-up dispatched by LoudDragon
(`879d780`); cross-cites EpicRaven's `discoveries/2025-10-21-t062-addendum-window-d24-d40-census.md`
(std::string ctor sites 0x4A6F5E/6D) rather than re-deriving it.
**Method:** F3.json (cls_0x50b560 members/methods/usages) → raw bytes (objdump, VA-corrected
section map) → `ida_original_f3` READ-ONLY xrefs. Zero IDB writes.

## Verdict 1 — the trio are FREE string-serialization codecs, not member functions

- **`0x50B880`** = read `JE::String` from the **file stream**: reads a u32 length into a local
  (via the `0x499220` FileStreamRead primitive with `mov ecx, ds:0x707da8` INSIDE the function
  @0x50b895/0x50b8b7), `0x605554(len+1)` alloc, read bytes, NUL-scan, `0x401bd0` assign into
  `[esp+8]` (arg1 = the OUT string). This is the reader our InterfaceFile port mirrors as
  `ReadString` — its callers are the shipped `.int` readers: `Window::ReadFrom` (0x4b111a/16d/21b/250/285),
  `Button::ReadFrom` (0x4a6bc6/4a6bda/0x4a6bee), `Label::ReadFrom` (0x4aa6f1), `Fragment::ReadFrom`
  (@0x4b2187 — the T-007 G1 texture-name read), `CLoadMapInterface::meth_0x495b50` (0x495c3a).
- **`0x50B790`** = write `JE::String` into the **save archive**: writes length-dword then bytes
  through the same `0x618e7c` "write field" primitive Fragment's SaveTo drives directly
  (0x4b238a/0x4b2443/0x4b245b). arg1 = the string, `[esi+0x18]/[esi+4]` = its SSO/heap union —
  NO this-object field is ever read.
- **`0x50B7F0`** = read `JE::String` from the **load archive** via the `0x619089` "read field"
  twin (the primitive Fragment's re-load lane uses @0x4b1eec/0x4b1f05). Same shape as 0x50B790.
- The xref census (IDA: 11 + 11 + 12 real consumers for 0x50B790/0x50B7F0/0x50B880, self-refs
  excluded): consumers are exclusively
  `GUI::Window/Button/Label/Fragment` serialize/deserialize vfuncs + `CLoadMapInterface` — every
  one passes a member-string ADDRESS as arg1 (e.g. Button: `lea eax,[esp+0x18]; push eax; call
  0x50b880` — matching EpicRaven's finding that Button's +0xD24/+0xD40 std::strings are FILLED
  here: Button::ReadFrom calls the trio at 0x4a6b16/2a/3e (0x50B790 lane? no — 0x4a6ac0's
  three calls are to 0x50b790? corrected below).
  - precise per-call map: `Button::virt_meth_0x4a6ac0` → 0x50B790 ×3 (0x4a6b16/2a/3e — SAVE path,
    writes btnS1/S2/S3); `virt_meth_0x4a6a10` → 0x50B7F0 ×3 (0x4a6a78/8c/a0 — LOAD-reload);
    `virt_meth_0x4a6b60` (=Button::ReadFrom) → 0x50B880 ×3 (0x4a6bc6/0x4a6bda/0x4a6bee — fills
    the three strings EpicRaven found ctor-initialized at 0x4A6F5E/6D; the T-007 port already
    models all three btnS1/btnS2/btnS3 reads at these sites). So **the cursor-sound
    chain closes: .int → 0x50B880 → Button+0xD24/+0xD40 std::strings → consumed keyed-by +0xD40**
    exactly as the dispatch described.
- **Owner: none.** None of the three touches a `this` object field (0x50B880/0x50B7F0/0x50B790
  argument-scan: only arg1 strings + globals/primitives). They are file-scope codecs of the same
  translation unit that also holds `cls_0x50b560` (0x50b560-0x50b8ff band) — which is exactly why
  the row hypothesis pointed at cls_0x50b560. Band adjacency ≠ membership; the export's method
  list for cls_0x50b560 (0x50b560/0x50b660/0x5cb2a0) correctly EXCLUDES the trio.
- The implicit stream partner of 0x50B880 is the global `0x707DA8`, whose **sole writer** is the
  `a3` store @0x59c190: `new`-via-`0x6054b0` + `call 0x49c610` — the ResourceSystem session
  object (`cls_0x49c610` — the class my port's ResourceSystem.cpp already labels
  "cls_0x49c610::Demand"). Single store, single global: one stream session, engine-wide.

## Verdict 2 — cls_0x50b560: Gfx_Engine-embedded slot allocator; its `+0x18` map keys on INTEGERS, not strings

- **Embedding/ownership:** its ctor `0x50B6B0` has exactly ONE caller — `0x5c0040` inside
  **`JE::Gfx_Engine::ctor_0x5bfe10`** (IDA xref) — so it is a 32-byte member of Gfx_Engine (the
  class whose singleton `0x707CE4` G2 minted); its `meth_0x50b660` slot-grab helper is reached
  from `Gfx_Engine::virt_meth_0x5c08a0` (0x5cb2b2/0x5cb320). Ctor also stores the member
  pointer into global `0x707CE8` (@0x50b6d1) — that is the §5 "epoch byte 0x707ce8" the
  GUILoadingRecon dead-end note named: **the dead-end resolves as Gfx_Engine's slot manager,
  not a stream/session epoch relevant to the GUI lanes.**
- **`+0x18` map policy:** `meth_0x50b660` @0x50b67c does `lea ecx,[edi+0x18]; call 0x492ac0` —
  0x492ac0 is an RB-tree insert whose key walk at 0x492ae0-0x492ae5 is
  `mov ebp,[ebx] / cmp ebp,[eax+0xC] / setb cl` — an **unsigned 32-bit comparison of the KEY
  ITSELF** (the key is the slot-pointer value returned by `0x50b560`, an integer). The map's
  other consumer (0x5cb2c3/0x5cb2f5 inside `meth_0x5cb2a0`, exported under cls_0x50b560) passes
  the same integer slot keys.
- **NO third key policy exists here.** The never-unify law from T-062 (registry case-SENSITIVE
  memcmp vs cursor stricmp) is NOT extended: the epoch map is integer-keyed — neither a stem,
  verbatim, nor case policy. Port note: nothing for `Gfx_Texture_Manager` to mirror; if/when the
  Gfx_Engine member bands port, its map is `std::map<uint32,slot*>` behaviour.

## Raw-evidence index

Trio bodies: 0x50b790 (two `0x618e7c` writes: len-dword + bytes; SSO cap test `cmp [esi+0x18],0x10`
@0x50b7bd) · 0x50b7f0 (0x619089 len-read @0x50b80b, alloc 0x605554, bytes-read, NUL-scan,
0x401bd0 assign) · 0x50b880 (`mov ecx,[0x707da8]` @0x50b895/0x50b8b7, `0x499220` reads @0x50b89f/
0x50b8c9) · global 0x707DA8 stores: EXACTLY one, `a3` @0x59c190 after `new; call 0x49c610` ·
cls_0x50b560: size 0x20, meth_0x50b560 = free-list slot grab (`[ecx]` counter, array `[ecx+4]`
count `[ecx+8]`, top index `[ecx+0x10]`, active `[ecx+0x14]`) · meth_0x50b660 map insert
0x492ac0 int-key walk · ctor 0x50b6b0 sole caller 0x5c0040 ∈ Gfx_Engine::ctor_0x5bfe10, stores
`0x707CE8` @0x50b6d1, node ctor 0x4c6640 (0x18-byte nodes) · Button/Label/Window/Fragment
call-site table from IDA xrefs_to(0x50B790/7F0/880) (12+12+14, listed inline).

## Port impact (T-066)

None for Steps 1-3 as designed: the trio are already mirrored by our port's stream primitives
(InterfaceFile ReadString ↔ 0x50B880; the archive codecs belong to the SaveTo/Reload lanes which
stay T-013/T-056 territory). Step 2's policy comment gains a negative confirmation: no third
string policy anywhere in this band.

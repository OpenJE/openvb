# T-062 — The cursor subsystem: CSound cursor objects, GameCursorManager, and the vft+0x30 child-notify targets

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Claim:** (this row; header flipped open→in-progress)
**Method:** F3.json class/vft/member indexes → RTTI COL chains and raw bytes off `$OPENVB_ORIG_EXE`
(objdump `-b binary -mi386 -Mintel`; section map re-parsed from the PE header this session:
`.text VA 0x401000/raw 0x1000/0x245C52`, `.rdata VA 0x647000/raw 0x247000` — vft/COL/TD reads are
VA-corrected per section) → `ida_original_f3` READ-ONLY (xrefs_to; zero IDB writes — CalmRaven's
writer slot untouched). Names are locators; every verdict anchors to an EA.

## Verdicts up front

1. **The 0xF8 "cursor object" is a `CSound` instance — the cursor carries SOUNDS, not textures.**
   RTTI-decided (COL 0x6D4098 → TD 0x6FB0A8 → name `.{AVCSound@@`), and its loader `0x4c82e0`
   DEMANDS the name through the *same* resolver the resource system uses (`0x49b9f0` → `call
   0x499f10` @0x49ba55 → RHT `0x50bb00`) — **the T-013 stem-vs-verbatim key question RECURS here
   and resolves the same way: stem after `sub_499F10`'s in-place strip.**
2. **The cursor manager at `[GameClient]+0xA4` is a `GameCursorManager`** (RTTI
   `.{AUGameCursorManager@@`, size 0xB0); its `vft+0x14` (slot 5 of vft 0x6550B4) = **`0x437ab0` =
   SetCursor(path, w, h, flag)** with an *idempotence gate* — same enabled byte, same stored
   dims/`stricmp`-equal name → returns without touching the OS cursor.
3. **The `vft+0x30` child-notify targets are the "set u-origin float across the piece" pair.**
   `GUI::Fragment` vft 0x66EA1C slot 12 (+0x30) = `0x4b1610`: writes the incoming float into
   **all four 0x1C-stride vertex blocks at +0xC0/+0xDC/+0xF8/+0x114** (the T-061 "wire dword"
   field — it is a FLOAT u0, now positively identified: same broadcast shape as the reader's,
   driven from dispatch instead of stream) and `fld`s it back; slot 13 (+0x34) = `0x4b1640` is
   the getter `fld [ecx+0C0h]`. `GUI::Window` vft 0x66E4D4 slot 12 = `0x4b06a0` = a *different*
   per-class setter (JE::String assign at window+0x40 — the skin-name slot). Slot 1 of the
   facade (`0x4953c0`) therefore means: **push one float into every child interface's u0 and
   sum the accepted values** — a layout/sync reduce, confirming T-061's "layout hints" reading.
4. **Event kinds: gates and target fields pinned statically; numeric identity deferred.**
   The kind→path map below is fully byte-pinned; WHAT the upstream pump calls these numbers
   (WINDOW_RESIZED vs CURSOR_CHANGED naming) needs T-056's live boot. Not guessed.

## Item 1 — the CSound cursor object (ctor 0x4c8100, vft 0x67140C)

- **Allocation**: slot-2 (T-061 `0x5803e0`) does `push 0xF8; call 0x6054b0` @0x58047a —
  `0x6054b0` is a raw `malloc` retry wrapper (`call 0x61ab8e`, loop to `0x607f9e` on failure).
- **Ctor** `0x4c8100`: `mov [esi],0x67140c` @0x4c811f (single vft at +0); initializes an inline
  JE::String at obj+0x18 (`lea edi,[esi+8]` + `[edi+0x18]=0xF` cap @0x4c8127) and zeroes fields
  through **obj+0xF4** (`[esi+0xf4]` last ctor store @0x4c8204) — an exact 0xF8 object.
- **RTTI identity**: vft-4 slot (0x671408) → COL `0x6D4098`; COL+0xC → TD `0x6FB0A8` whose name
  pool is `.{AVCSound@@` (od dump: `. ? A V C S o u n d @ @ 0`). The export's single-vft CSound
  (0x67140C ✓) with 41 methods matches. ⚠ **Size note**: the export's CSound size 281 (0x119)
  comes from members +0xF8..+0x115 touched only by the 0x539370-band methods, NOT by this ctor —
  the 0xF8 ctor-bounded instance is what this path allocates; the 0x119 figure is the same
  OOAnalyzer family-inflation class as Fragment 3384 vs 0x158 (T-040 errata family).
  My T-053-era "CSound size 0x138" note is superseded by the 0x539xxx+0xF8.. evidence — 0x119/281
  stands as the export figure, 0xF8 as the cursor-instance size.
- **Loader `vft[0]` = 0x4c82e0** (the `[[obj]+0](name,0)` from slot 2):
  - `test BYTE [esi+0x34],2` @0x4c82f3 — already-loaded flag → return 2 (idempotence, same
    flavor as T-013's probe dedupe);
  - `[esi+0x24]` nonzero (resource handle) → return 7 @0x4c8319;
  - else `mov ecx,[esi+4]; push 1; push name; call 0x49b9f0` @0x4c833f — **0x49b9f0 is the
    ResourceSystem DEMAND twin: it builds the search string (0x401bd0), calls `sub_499F10`
    (@0x49ba55 — the extension-stripping resolver!), then RHT `0x50bb00`** — i.e. the cursor
    SOUND demands its file by RESOLVED stem. `0x49b9f0`'s returned record is cached at
    `[esi+0x24]`; on null → return 6 + (0x4c84a3 tail).
  - success → `0x4c6c30(name)` @0x4c835a: the **3-char-after-dot tag sniffer** (`80 38 2E`
    '.' scan, `cmp al,0x57/0x77` W/w, 'A/C/M' → ACM, 'R'…) choosing the decode path, then
    `0x4c7d10/0x4c7d32` sample setup — this is a sound-file loader, not an image loader.
- **The (2,0,0) helper `0x4c7eb0`** (slot 2 calls it right after Load): reads the AUDIO master
  switch at **`[0x70B250]+0x48`** (the sound-manager singleton global — my T-053 note had
  "sound manager = [0x70B250]+0x28" — same singleton) and returns 1 when audio disabled, 2 when
  the object's state (`obj+0x34` bit 1/2) isn't ready; on ready it calls `0x4c6bb0(obj, a2, a3)`
  (unexported chunk resolver — `f3_address_lookup` 0 matches, consistent with an in-class
  helper) — verdict: **Play/Enable attempt with two shipped bail codes; the (2,0,0) args are
  play-mode/offset/channel-shaped, exact arg meaning NOT claimed.**
- **T-013 cross-cite**: `GUI_DefaultCursor`/`GUI_BeamCursor` demand names hit `sub_499F10` at
  0x49ba55 inside 0x49b9f0 — the demand side is stem-resolved here too; only the
  `GameCursorManager::SetCursor` comparison (§item 2) sees the VERBATIM string, where it is
  `stricmp`-compared (0x617119 resolves via the CRT compare shim at 0x621270/0x618178 — a
  CASE-INSENSITIVE compare, unlike the texture registry's `repz cmps` memcmp. So: the texture
  registry is case-SENSITIVE stem-keyed; the cursor manager is case-INSENSITIVE verbatim-keyed.
  Two different key policies in one engine — port must not unify them.)

## Item 2 — GameCursorManager ([GameClient+0xA4], vft 0x6550B4)

- Class (F3.json RTTI): `.?AUGameCursorManager@@`, size 176 (0xB0), vft 0x6550B4 length 11,
  slot map 0→0x4363f0, 2→0x437bb0, 3→0x5915d0, 4→0x437be0, **5→0x437ab0**, 6→0x437b20,
  7→0x437d80, 8→0x437b40. T-061 slot 5 dispatches `[cursorMgr vft+0x14]` = slot 5 = **0x437ab0**
  ✓ identity closed.
- **`0x437ab0` = SetCursor(char* path @arg1, int a2@[esp+10], int a3@[esp+14], char flag@[esp+20]
  — thiscall)**: idempotence test `obj+0x4c != 0 && obj+0x6c == a2 && obj+0x70 == a3 &&
  stricmp(obj+0x54 string, path) == 0` (the string compare via 0x617119 @0x437ae3) → early
  return 0x437b19; else stores flag byte at `obj+0x74` @0x437af3, sets `obj+0x4c=1`, strlen
  scan @0x437afc-0x437b07, assigns the name into the JE::String at obj+0x54 (SSO cap check
  `cmp [obj+0x68],0x10` @0x437ad3) and tail-calls the real device cursor setter (the block past
  0x437b09, reaching `virt_meth_0x437bb0`/OS paths).
- Call sites from T-061's facade: **beam branch** (window+0xD18 == 4) → `("GUI_BeamCursor.tga",
  16, 16, 0)` @0x580383-0x58038c; **default branch** → `("GUI_DefaultCursor.tga", 2, 3, 0)`
  @0x58039c-0x5803a7 — so a2/a3 = (w,h) hotspot/extent pair per cursor; flag 0 at both sites.
- `[GameClient+0xA4]` provenance: the export's `.{AUGameClient@@` is a **union** (`U`, 964 bytes)
  with no minted member map, so the +0xA4 offset could not be closed against an export member —
  the object identity is carried by the RTTI + vft slot match above (two independent sources),
  and the fill site lives in the client-ctor band. **Named risk**: if the port ever constructs
  GameClient, +0xA4 must be initialized to a GameCursorManager; the one ctor store was not
  pinned here (union member census is T-059/T-063-adjacent territory).

## Item 3 — the vft+0x30 child-notify target set

- The dispatch (T-061 slot 1 `0x4953c0`, re-read raw at file_off 0x953e0): walk iface `+0x28`
  intrusive list (`esi = [ [iface+0x28] ]`, loop while `esi != head`, next at `[node]`), per
  node `ecx = [node+8]` (child interface object), `call [[child vft]+0x30](float a1, ebx 0)`,
  `fadd` the float return into the accumulator; on empty list return `a1-a2` if `iface+0x2c != 0`
  else `a1` @0x495407-0x495416.
- **Concrete verified targets (per-class slot 12):**
  - `GUI::Fragment` vft 0x66EA1C slot 12 (+0x30) = **0x4b1610**: `lea eax,[ecx+0xC0]; mov ecx,4;
    { mov edx,[esp+4]; mov [eax],edx; add eax,0x1C; dec ecx; jne } ; fld [esp+4]; ret 8` —
    **writes the float argument into field 0 of all four vertex blocks (+0xC0/+0xDC/+0xF8/
    +0x114 — the exact stride-0x1C four-block layout T-061 established) and returns it.** This
    settles T-061's open field question: the broadcast-written dword is a **FLOAT**, and slot 12
    is its dispatch-driven setter (the reader@0x4b2250 is the stream-driven writer). Getter:
    slot 13 (+0x34) = **0x4b1640** `fld [ecx+0xC0]`.
  - `GUI::Window` vft 0x66E4D4 slot 12 (+0x30) = **0x4b06a0**: `add ecx,0x40; ... call 0x401b20`
    — a JE::String assign into **window+0x40** (the skin-name member the .int reader fills at
    0x4b1170 s40 in our port) — a same-slot-different-semantics per-class setter. It takes a
    POINTER arg here, and the facade passes (float-bits, 0) — the two call shapes coexist
    because slot 12 is a per-class polymorphic "set primary attribute" with the float-only
    contract honored by the Fragment band; **do not mint a single signature across the facade —
    the notify target set is heterogeneous by class.**
- **Census note**: raw pattern `call [reg+0x30]` (all modrm forms) = 218 sites image-wide —
  byte-offset collisions make this unfilterable without owner attribution; the two targets above
  are the ones reachable from this facade's verified list (iface children are built from the
  host's constructed windows/fragments). A complete +0x30 target table per vft = T-054's
  ecx-flow tool output, requestable via the coordinator (per the T-013-tail lane note); the
  concrete pair delivered here is enough for the port (Fragment u0 setter/getter + Window name
  setter).

## Item 4 — event-kind table: pinned gates, deferred names

From slot 2 (`0x5803e0`, raw at file_off 0x180410; edi = event struct, `[edi]` = kind):

| kind(s) | gate chain | cursor slot used | notes (EAs) |
|---|---|---|---|
| 4, 7 | `[host+0xD18]==1` only | **+0xD24** (default) | 0x580417/0x580423 → 0x58064e; string copy 0x401b20 from [esi+0xd24] @0x58067b, alloc/load/`0x4c7eb0(2,0,0)` 0x58068a-0x5806d7 |
| 1 | `[host+0xD18]==1`, then `[[host vft]+0x12c]==2` | **+0xD24** | 0x5804e0: first `0x4958d0(parent,host,event)` @0x5804f8, then `[[host]+0x11c](event)` @0x580502, gate 0x580512, +0xD24 path 0x580517+ |
| 3 | `[host+0xD18]==1`, `[[host]+0x12c]==2` | **+0xD40** (beam) | 0x58059a |
| 5 | `[host+0xD18]==1`, `[[host]+0x12c]==2` | **+0xD40** | 0x580435 |
| all | → tail `0x5806f8` calls `0x4958d0(this,host,event)` = the **un-overridden parent GUI::Interface slot-2 default** | — | kind==1 there: RTTI-ish `[[host]+0x10c](&tmp)` @0x495906 + `0x707E48` skin-registry touch @0x49590d + `0x4b4160` — the parent's own skin-side reaction to the same event |

- Every cursor path allocates the CSound object and demands the **name stored at host+0xD24 /
  +0xD40** (two JE::String slots, 0x1C apart = one 0x38 struct: default then beam cursor SOUND
  names). Their writers were not found in the export's Window member list (sparse-map gap;
  +0xd24/+0xd40 unlisted) — **named open item: who fills the cursor-sound name strings**
  (candidates: the .int reader beyond our ported window block, or an engine config path).
- **Deferred, not guessed**: the numeric identity of kinds {1,3,4,5,7} (which upstream message
  constants they are). The gates + fields are pinned; the naming is a producer-side fact the
  T-056 live boot can read off one dispatch. Recommend the cross-reference when T-056 runs.

## Errata/refinements to T-061 (self-cross-cite, no external correction needed)

- T-061 §(b) called the vertex-block field 0 an "int/unknown broadcast dword": now pinned as
  **float u0** (setter 0x4b1610, getter 0x4b1640 — both `fld`-typed) written by the same
  broadcast loop shape in two drivers (stream: reader 0x4b2250; dispatch: slot 12).
- T-061 §item-1 called the slot-2 object "Gfx cursor obj": correct that it's the cursor's
  companion object, but its RTTI truth is **CSound** (cursor SOUNDS), not a texture/cursor
  image class; the image side is GameCursorManager::SetCursor's path string (§item 2).

## Raw-evidence index (all post-respawn reads this session)

COL/TD chain 0x671408→0x6D4098→0x6FB0A8 (name bytes) · ctor 0x4c8100 stores vft @0x4c811f,
string @+0x18 init 0x4c8127, last field write 0x4c8204 [esi+0xF4] · loader 0x4c82e0 (bail codes
2/6/7, demand 0x49b9f0 @0x4c833f, resolver hit 0x49ba55, tag sniffer 0x4c6c30 @0x4c835a) ·
0x4c7eb0 audio-switch read [0x70B250]+0x48 @0x4c7ebc · GameCursorManager vft 0x6550B4 raw (od at
file_off 0x2550B4) + slot5 0x437ab0 disasm (idempotence 0x437ac9-0x437aed, flag 0x437af3, SSO
cap cmp 0x437ad3) · facade slot-2 dispatch sites `call/jmp 0x5803e0` at 0x41cfa0 (embed −0x29c),
0x42f810 (−0x38), 0x433f0d/0x44076a (−0xc) — devirtualized, host-array loops · notify walk
0x4953e0-0x495416 · Fragment vft 0x66EA1C slot 12/13 = 0x4b1610/0x4b1640 (F3 vft dump + raw
disasm) · Window vft 0x66E4D4 slot 12 = 0x4b06a0 raw disasm · `call [reg+0x30]` raw scan = 218
hits (uncollapsible without owner attribution; see census note).

---
## COORDINATOR ANNOTATION (F2 erratum, T-072 sweep -> KeenStorm raw-confirm; applied 2026-10-21)
Window PRIMARY slot12 (+0x30 @0x66E4D4) = **0x4AFC00 notify-walk** (iterates iface+0x28 children
AND the 9 inline fragment records at this+0x90 stride 0x158, calls [X+0x30] fadd-reduced — the
primary anchor is itself a displayer of the vft+0x30 polymorphic slot, so the heterogeneity section
is STRENGTHENED by the erratum, not merely repaired). The String setter **0x4B06A0 = slot 12 of the
SECONDARY table 0x66E574 (=0x66E4D4+0xA0)** — its `add ecx,0x40` this-adjust prologue is the
sibling-subobject-table signature F3.json's flattened 75-entry list hides (0x4b06a0 sits at flat
index 52, 0x4AFC00 at 12). Original citation attached the secondary table's body to the primary's
address at doc time. CITATION LAW (KeenStorm's root-cause, adopted board-wide): **cite TABLE
ADDRESS + byte offset, and re-dump raw before trusting slot indices near multi-table vft runs.**

### SECOND-PASS AMENDMENT to the F2 annotation above (T-081, TrueCastle; coordinator applied to own text, 2026-10-21)
The F2 remediation I just wrote repeated the flattening error it was fixing: '**0x66E574** (= 
0x66E4D4+0xA0)' is NOT a table base — it is SLOT 40 of the primary run (its [-4] is code). Window's
actual SECONDARY is **0x66E4CC** (GUI::Window MI view, COL viewOffset 24, one slot = 0x4b0a40),
installed by `c7 46 18` beside the primary store. Primary slot-12 = 0x4AFC00 (notify-walk) stands
byte-exact, and the String setter 0x4B06A0 = **slot 52 within the primary** per T-081 §2, not a slot
of a second table. Evidence: discoveries/2025-10-21-t081-gfx-slot-table-bases.md. Lesson logged
coordinator-side: a correction written while the correcting tool still trusts the flattened view
bakes its own phantom — re-key citations through the RAW table before authoring the fix text.

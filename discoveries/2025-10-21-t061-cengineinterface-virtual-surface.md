# T-061 — CEngineInterface virtual surface (vft 0x69A95C): all six slots decoded

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Claim:** `01bb7f7`
**Method:** F3.json vft/class/member indexes → RAW BYTES off `$OPENVB_ORIG_EXE`
(objdump `-b binary -mi386 -Mintel`, `file_off = VA - 0x400000` for `.text`; PE section table
re-parsed this session: `.text 0x401000+0x245C52 raw 0x1000`, `.rdata 0x647000 raw 0x247000` — so
vft constants live at `VA - 0x400000` and the store-scan below is VA-corrected per section) → `ida_original_f3` READ-ONLY
(xrefs, decompile). CalmRaven held the writer slot the whole time; zero IDB writes from me.
Names are locators only; every verdict anchors to bytes.

## Headline

The row's premise "only edges met sideways" held, and the edges themselves needed correcting
(see §Errata). The surface is a **cursor/event facade for GUI-host classes**: slot 0 is the scalar
deleting-dtor, slot 2 is an event-kind switch (types 1/3/4/5/7) that reacts to WINDOW state —
loading **`GUI_DefaultCursor.tga`/`GUI_BeamCursor.tga`** cursor textures through the
`Gfx_GfxCursor`-class object (ctor `0x4c8100`, vft `0x67140c`) — and slot 5 pushes the
beam/default cursor selection into the engine's Gfx device
(`[[0x70BD90]+0x2C]+0xA4 -> vft+0x14(path, w, h)`). Slots 1/3/4 are pure **inherited defaults** of
the parent `GUI::Interface` (vft `0x669D80`): slot 1 = a parent-list float accumulator, slots 3/4 =
empty `ret`. **This is not a stream/global broker — it is the cursor-and-event half of the GUI host
interface**, which is why every skin/widget host embeds exactly one.

## vft 0x69A95C — six slots, raw-dumped (od at `.rdata` file_off 0x29A95C):

    005803c0 004953c0 005803e0 00404b70 00404b70 00580360   (then 0x00000000 0x00000002 = COL tail)

| slot | +bytes | target | bytes verified | verdict | callers (evidence) | confidence |
|---|---|---|---|---|---|---|
| 0 | 0x00 | 0x5803c0 | `56 8b f1 e8 88 ff ff ff f6 44 24 08 01 74 09` | **scalar deleting dtor**: calls `0x580350` (dtor-body: `mov [ecx],69a95c; jmp 0x495500` = parent `GUI::Interface` dtor), then if flag byte bit0: `0x4a7540` (heap free via the `0x707DF8` allocator object). ret 4 | 34 sites — every host dtor family (F3 xrefs: 0x417347, 0x425ce1, …, 0x5b77fc) + 0x5803c3 self | HIGH |
| 1 | 0x04 | **0x4953c0** | `8b 47 28 85 c0 …` | **NOT export-listed in the vft dump but present in raw**, = parent `GUI::Interface`'s own slot-1: walks the intrusive list at `iface+0x28`, and for every list node calls `[[node+8] vft+0x30](float a1, float a2)`, `fadd`-accumulating the returns; if the head is empty and `iface+0x2c != 0`, returns `a1-a2`, else `a1`. A **float reduce/notify over the interface's child window list**. | shared with parent vft 0x669D80 slot 1 (same EA) and the three host vfts 0x698008/0x69DE78/0x69EDA4 | HIGH |
| 2 | 0x08 | 0x5803e0 | SEH frame, `sub esp,40h`; `mov esi,[esp+58h]` (window), `mov edi,[esp+60h]` (event), `mov eax,[edi]` | **event-kind switch on the window's interface**: kind 4/7 → take cursor name at `window+0xd24`, if `window+0xd18==1`: copy name, allocate cursor obj (`0x6054b0` new 0xF8 + ctor `0x4c8100` vft 0x67140C = GfxCursor), `[obj vft+0](name)` load + `0x4c7eb0(2,0,0)`; kind 1/3 → same but for `window+0xd40` (the BEAM cursor) with host vft `+0x12c == 2` gate and helper `0x4958d0` refresh; kind 5 → like 1 but `[esi vft+0x12c]`. Ends by calling `0x4958d0(parent, window, event)` = the un-overridden parent behaviour. **Cursor load/refresh from window event stream.** | 41 code xrefs incl. host virt_meths 0x440770/0x4422e0/0x4444c0/0x4468b0/0x4491a0/0x449630/0x5900c0/0x5b42a0 — **also the parent's own slot 2 IS 0x4958d0**, so 0x5803e0 is the CEngineInterface override | HIGH for the shape; MEDIUM on kind-number semantics (4/7=default, 1/3=beam, 5=? — the gate `cmp eax,1 / jne` on `[window+0xd18]` decides which cursor slot reloads) |
| 3 | 0x0C | 0x404b70 | `c3` | **empty** (`ret`) — parent default, not overridden | any vft+0xC dispatch | HIGH |
| 4 | 0x10 | 0x404b70 | `c3` | **empty** — same no-op (note: Window ctor dispatches `[iface+0x10]` at 0x4b099a/0x4b09b8 — it is silently doing nothing there, by design; the real work in those branches happens BEFORE the call, via `0x4b14c0`/`0x4b14d0` with colors 0xff804010/0xff408010 pushed as the arg) | same | HIGH |
| 5 | 0x14 | 0x580360 | `8b 4c 24 04 85 c9 a1 90 bd 70 00 8b 40 2c 74 3f` | **cursor PUSH to the device**: reads global `0x70BD90` (the engine session) → `+0x2C` (Gfx device) → `+0xA4` (cursor mgr); arg1 must be non-null AND `[arg1+0xd18]==4` (window beam-mode) → `call [[cursor mgr] vft+0x14]` with `("GUI_BeamCursor.tga", 16, 16, 0)` else `("GUI_DefaultCursor.tga", 2, 3)`; returns 1/0. **Does NOT write any global, does NOT read [esi+0x74], does NOT touch 0x707DA8.** | string refs 0x698018/0x69de88/0x69edb4 (three host vft slot-5s = same handler) | HIGH |

## Class shape (F3.json + ctor bytes)

`CEngineInterface` (size 0x31) **derives from `GUI::Interface` at +0x00** (export member
`parent:true .?AVInterface@GUI@@_0x0`), and the ctor `0x580330` is exactly:
`call 0x495600 (parent ctor: sets vft 0x669d80, inits JE::String @+4, RB-tree @+24, flag @+20); mov [esi], 0x69a95c; ret 4`.
So the six-slot vft is the parent's five-slot vft (0x669D80: 0x4955e0, 0x4953c0, 0x4958d0, 0x404b70, 0x404b70, 0x430790…)
with **slot 0 replaced (deleting dtor), slot 2 overridden (event switch), slot 5 replaced
(0x430790 `xor al,al; ret 0xC` -> 0x580360 cursor push)**. Slots 1/3/4 are literally the same EAs
as the parent's — "unnamed in the sparse export map" resolved: they are not missing, they are
inherited defaults that the 2024 export simply didn't re-emit under this class. The `+0x10c/+0x12c`
calls inside slot 2 are the **window-side** interface (a different, bigger vft — arg1 is a Window,
whose own vtable carries `+0x11c/+0x12c/+0x10c`), not this class.

Host embed census: F3 `xrefs_to(0x580330)` = **34 host ctors** (CActionbarInterface +0x29c,
CNASInterface, CCharacterGeneration, CFeedback, GameCSMenu, GameCSSkillMenu, GameDialogue,
CGameFloatingText, CInventory, CMainmenu, CMessageBox, CMiniMap, COptions(+Game/Hotkeys/Load/
Multiplayer/Video), HotKeyWindowFormatTable, CPipBoy(+AddNote), CDebug, CSMenu, CFloatingText,
DialogueInterface, EditTokenInterface, + 5 cls_0x…) — matches the T-057 "27+ hosts" census and
upgrades it to a counted 34 (several hosts construct it as an embedded member via `lea ecx,[host+OFF]`).

## +0x74 census (the row asked for it; scope correction attached)

F3.json `GUI::Window::mbr_0x74` = 14 usages (all raw-verified): **writers 2** —
0x4b094d (Window ctor: 16-byte copy of arg block to `[esi+0x74..0x83]`) and 0x4b11c2-0x4b11d6
(Window::ReadFrom: re-reads the same 16 bytes from the stream via global 0x707DA8 =
FileStreamRead wrapper 0x499220); **reads 12** — 0x4af02a/0x4af040/0x4af140/0x4af35f/0x4af427/
0x4af440 (also a shrink-writer)/0x4af659/0x4af847/0x4afa4a/0x4afd19/0x4b0ed8 + the 0x4c8100-adjacent
reads. It is a **16-byte inline float4/RECT at Window+0x74** (authored in the .int header, copied
by the ctor from the parent's field block), read by the window-layout helpers `0x4af000-0x4afe00`
as x/y/width/height arithmetic (`add/sub/push 0x10` patterns). NOT the interface's member — the
row's "+0x74 member of CEngineInterface" is a **nonexistent member**; it belongs to Window
(CEngineInterface size 0x31 cannot contain +0x74).

## Errata against banked edges (delivered as asked, respectfully)

1. "slot 5 = SetInterface global writer, loads [esi+0x74] from the stream at global 0x707DA8"
   (GUILoadingRecon §11 as relayed in the row): **contradicted by raw bytes at 0x580360** — it
   reads 0x70BD90->+0x2C->+0xA4 and the two cursor .tga string literals; no 0x707DA8, no +0x74,
   no global store anywhere in the function. The global-writing/stream function that §11 describes
   is the **Window::ReadFrom path at 0x4b11c2** (0x707DA8 stream, 16 bytes into Window+0x74), and
   `Window::SetInterface_0x4af980` is unrelated to both: it walks `window+0x88`'s list and calls
   each entry's `vft+0xE0(arg)` (interface fan-out to child skins), reading NO global.
2. Export's vft sparse map (slots 0/2/5 only): raw dump shows slot 1/3/4 are **parent-inherited
   defaults** (same EAs as 0x669D80 slots 1/3/4), not unnamed CEngineInterface methods. The dump
   after slot 5 is `0x00000000 0x00000002` = complete-object locator tail (RTTI type info at
   0x69A97C region), consistent with length 6.

## What the surface IS (verdict)

`CEngineInterface` = the **engine-session/cursor facade baked into every GUI host class**: it
inherits `GUI::Interface`'s child-window notify reduce (slot 1) and empty defaults (3/4), and adds
(a) a deleting-dtor hooked to the 0x707DF8 allocator (slot 0), (b) an event-handler override
(slot 2) that reloads the window's default/beam cursor textures when cursor-related window events
(kind 1/3/4/5/7, gated on the window's `+0xd18` cursor-mode and its own vft `+0x12c` state) fire,
and (c) the device cursor push (slot 5) that installs `GUI_DefaultCursor.tga` / 
`GUI_BeamCursor.tga` (16x16 for beam) into the engine's cursor manager when the window is in
mode 4. That is why 27+ hosts embed it: every window class must answer the engine's cursor
protocol, and this vft is the whole protocol — three overrides, three inheritances.

## Open questions (named risks, not guesses)

- Event-kind numbers: the 1/3-vs-5 gate on `[window vft+0x12c]==2` and `[window+0xd18]` values 1
  vs 4 split "which cursor slot refreshes"; I did not decode the `0x4958d0` parent-handler beyond
  its RTTI-cast + 0x707E48-skin-registry touch (it also reads `[event]` kind==1) — a render-side
  trace would pin the kind table. Not needed for the port (T-013/T-056 never dispatch event kinds
  with these numbers).
- `[node+8]+vft+0x30` targets in slot 1 (the child-window notify callee) belong to the **window**
  vft, not this class; enumerating that slot-0x30 handler set is a Window-band job (T-055's 4b28a0
  family territory), one hop from this row's evidence but not this class's surface.
- RTTI COL tail decode (0x69A974 area) skipped — 2024 export already demangles both names; no
  identity is in doubt.

## Raw-evidence index (all this session, post-respawn)

vft dump: `.rdata` off 0x29A95C (6 dwords) · parent vft: 0x269D80 · slot0 bytes: `.text` 0x1803c0 ·
slot1: 0x0953c0 (via VA 0x4953c0) · slot2: 0x1803e0-0x180720 · slot3/4: 0x004b70 · slot5:
0x180360-0x1803b4 · strings 0x656204/0x6561ec (`.rdata`) · ctor 0x180330 + parent ctor 0x095600 ·
allocator 0x0a7540 · cursor obj ctor 0x0c8100 (vft 0x67140C at `.rdata`) · Window +0x74 writers
0x0b094d/0x0b11c2 · IDA xrefs: 0x580330 (34 callers), 0x5803e0 (41 code xrefs), 0x580360 (3 host
vft data refs), 0x4953c0 (5 vft data refs incl. parent), 0x69A95C (2 data refs = the two stores).

# 2025-10-21 — T-083: Gfx_Engine vft 0x6ae7ec — full 120-slot anatomy census

**Agent:** SwiftMoon `01a16092` · **Coordinator:** LoudDragon `01a15f82` · **Mode:** offline
(raw PE parse of `$OPENVB_ORIG_EXE` + linear `objdump -D -b binary -m i386 -M intel
--adjust-vma=0x400000` + `artifacts/F3.json` joins; zero IDB use, zero writes anywhere).
**Depends honored:** T-007 G1/G2, T-045, T-054/55, T-057, T-061/62 (+addendum), b4/b5/b6 ledgers.

## 0. Self-test gate (ALL SIX board-owned anchors reproduced BEFORE any unknown was classified)

| # | board-owned anchor | walker result | verdict |
|---|---|---|---|
| 1 | slot 49 (+0xC4)=0x5C0A90, 14-arg SEH forwarder → TexMgr `0x521CA0` (G1/T013) | raw: SEH prologue 0x5c0a90, args from [esp+0x4C..0x50], call 0x521ca0 present | **REPRODUCED** |
| 2 | slot 54 (+0xD8)=0x5C0930 texture **OBJECT** (§8e) → tail `mov ecx,[0x70B200]; jmp 0x51F160` | raw 0x5c0930 = `8b 0d 00 b2 70 00` + `e9 …51f160` | **REPRODUCED** |
| 3 | slot 55 (+0xDC)=0x5C0970 acquire-by-name → `0x51F570` on [0x70B200] (G1 correction) | raw: SEH 0x5c0970, `mov ecx,[0x70b200]; call 0x51f570`; EH cleanup callee 0x4B9D60 (GUI::Interface, T-064 family) | **REPRODUCED** |
| 4 | slot 56 (+0xE0)=0x5C0940 add/lock (G1) | raw: `mov ecx,[0x707CE0]; jmp 0x507660` — 0x707CE0 = the ctor self-cache of the engine's **+0x30 buffer-manager** object (T-054 mint chain) | **REPRODUCED + mechanism explained** |
| 5 | b5 Gfx_D3D-create leg 0x5C0C30 (slot 8) = CreateDevice | raw: literal `Gfx_Engine::CreateDevice - Failed to create PostRenderViewport` @0x6AE9CC + `…Default Viewport` @0x6AEA0C; `call 0x503920` (JE::Gfx_D3D::CreateDevice_503920, named b4); `push 0x280; call 0x6054B0; call 0x4C0190; mov [esi+18h],eax` @0x5C0CCB–0x5C0D19 | **REPRODUCED** |
| 6 | T-062-addendum slot-manager (cls_0x50B560, tree @+0x18) | ctor 0x5BFE10 @**0x5C0040** `call 0x50B6B0`, result stored **[engine+0x3C]** (0x5C0050); the addendum's "+0x18" is the tree field INSIDE the 0x24-byte manager, not an engine offset — engine+0x18 is the DEVICE WRAPPER (see §2). No contradiction, now precise. | **REPRODUCED with offset disambiguation** |

Tool ground-check first: World tables per T-045 — [0x6919e4+0x18]=0x544A90 ✓, [0x668584]=0x544A90 ✓,
primary [0x691a1c+0x18]=0x404B70, +0x14=0x546B50 ✓ (the dumper that reproduced those read 0x6ae7ec).

## 1. Table structure (raw-measured vs export-claimed — separate rows per sharpening (b))

* **Raw truth:** 0x6ae7ec is **120 consecutive slots** (idx 0..119), every cell a distinct
  code target in .text (0x5BF770..0x5C1E40 band), no duplicates, no stubs, no holes;
  idx 120 (0x6AE9CC) is literal text (`Gfx_Engine::CreateDevice - Failed …`) — the table ENDS at 120.
* **RTTI:** vft−4 → COL 0x6D79F4 → TD 0x7001F0 → `.?AVGfx_Engine@@` — single-COL pattern
  (same proof shape as CEngineInterface T-057); COL+0x0=0 ⇒ **primary vft** (item-44 single-owner view).
* **Install census (item-49, BOTH directions):** absolute-encoding scan of 0x6AE7EC → **zero**
  `a3/c7-05` global stores; callee-side scan finds exactly TWO `c7 06` register-indirect stores:
  **0x5BFE45** (ctor_0x5BFE10 head `c706 ece76a00` — matches T-007 G1's cited store) and
  **0x5C1CF1** (INSIDE dtor 0x5C1CD0 — the standard MSVC dtor vft-restore). So "second writer" is
  NOT a second ctor; it is teardown identity-restoration. Both directions reported.
* **Export-claim vs raw — ERRATUM CANDIDATE for T-040's list:** `artifacts/F3.json`
  (`.?AVGfx_Engine@@`, vft 0x6ae7ec) declares `"length":120` but carries **119 entries — a HOLE at
  idx 100**; raw idx 100 = live target **0x5C0350** (`mov al,[ecx+0x68]; ret` — engine byte getter).
  The 2024 export lost a trivial accessor; downstream slot-numbering that trusts entry-presence
  (rather than length) will mis-index everything ≥100 by one. Also the export keeps size=2212
  (0x8A4) — plausible against max observed member disp 0x894, but STILL non-authoritative per
  T-040/T-007 (do not copy).

## 2. The engine's subsystem map (what the slots delegate TO — all bands accepted-corpus)

| engine member | contents | installed by | delegation slots |
|---|---|---|---|
| +0x18 | **device wrapper**, 0x280-byte `cls_0x4c0190` object (ctor 0x4C0190) | ONLY slot 8 CreateDevice @0x5C0D19 (ctor zeroes it 0x5BFFA5) | 15,16,24,25,40,41,42,43,66–79,83,89,116,117 (+36 returns it) |
| +0x2C | **Gfx_Texture_Manager instance ptr** (class b6-minted from 0x55DC80 band; global [0x70B200] is the POINTER slot per T-054) | ctor @0x5BFFD2 | 48–55, 58?, 101, 102, 119 |
| +0x30 | **buffer manager** (T-054 cls_0x507810/T-055 JE::Gfx_BufferManager, new(0x50)); its ctor self-caches **[0x707CE0]** | ctor @0x5BFFFC | 56, 57 |
| +0x38 | **stats/record manager** self-caching **[0x70C96C]** (ctor-call 0x5CB510 @0x5C0016; dtor zeroes 0x5CAB70; float-accumulator fields +0x30..0x3C @0x5CAB55) | ctor | 44,45,46,47 |
| +0x3C | **slot manager** `cls_0x50B560` 0x24-byte, RB-tree at ITS +0x18; writes [0x707CE8] (T-062 addendum) | ctor @0x5C0040 `call 0x50B6B0` | (indirectly via TexMgr paths) |
| +0x64 | growable point list (50 entries pre-alloc'd: ctor push 0x32 call 0x5BDF80 = slot 91) | ctor | 91 (add), 104/105 (compare+free) |
| +0x54/+0x58 | float pair, ctor defaults **0.15/1.5** (constants 0x6ADB7C/0x6ADB80; restore = slot 88, called from ctor @0x5C005C) | ctor | 84,85,86,87,88 |
| +0x894 | teardown pointer array (dtor loop, slot 9) | runtime | 9 |

Statics the slots touch are NOT loose globals: **0x70837C…0x70839C, 0x708300…0x708328 and
0x708988 are fields of the static JE::Gfx_D3D instance at 0x7082F8** (offsets −0x7082F8: 0x84
slot-1 count, 0x88 array ptr, 0x90 [0x708988] byte read by slot 5 / written by slot 6 @0x5BF78B,
0x94 chain-obj ptr for slots 20–23, 0x9C/0xA0 the b6 {w,h} pair neighbourhood, 0xA4 slot-17 int;
0x8/0xC/0x14/0x30 = the 13-dword block slots 6/118 copy or read; 0x690 = +0x708988, comfortably
inside the export's 1681-byte Gfx_D3D — size consistent, still non-authoritative per T-040). Raw
absolute-store census across the whole aligned disassembly: the ONLY writer to any of these by
absolute encoding is ENGINE slot 6 itself (@0x5BF78B → Gfx_D3D+0x90) — so these are engine-visible
shared fields of the static D3D object, not private band state, and b6's "value pair, not a class"
holds. Honest limit (item 49): the register-indirect store census was NOT run against each of these
globals; "no absolute writers" is a statement about this scanner, not a class claim.

## 3. THE 120-ROW TABLE

Buckets: **D** = delegation-identified (forwards into §2's attributed subsystems);
**O** = owner-named Gfx_Engine own-state (incl. the 7 self-named-by-literal methods + deleting dtor);
**U** = FINAL-unknown, reason printed. Evidence = body EAs (all raw-read this sweep).

| idx | off | target | B | one-liner (evidence EA) |
|---|---|---|---|---|
| 0 | 0x00 | 0x5C1E40 | O | scalar deleting dtor: `call 0x5C1CD0` (dtor, which re-stores this vft @0x5C1CF1) then flag-test → `push esi; call 0x6162E0` (operator delete) |
| 1 | 0x04 | 0x5C0440 | D | getter [0x70837C] = Gfx_D3D+0x84 (adapter/mode count) |
| 2 | 0x08 | 0x5C0450 | D | bounds-checked indexed read [[0x708380]+i*0x44C+0x42C] — Gfx_D3D record table |
| 3 | 0x0C | 0x5C0480 | D | same family, second field window |
| 4 | 0x10 | 0x5C04D0 | D | same family, third field window (0x44C stride ties to Gfx_D3D 0x503D90/0x503E30 @0x503D98/503E3B) |
| 5 | 0x14 | 0x5C0080 | D | getter [0x708988] = Gfx_D3D+0x90 (fullscreen-mode byte) |
| 6 | 0x18 | 0x5BF770 | D | **fullscreen apply**: set [0x708988], copy 13 dwords from Gfx_D3D+0xC block (rep movs @0x5BF7AA), call Gfx_D3D 0x503CD0 (the "DXErr: %s" method, b4-named band) @0x5BF7C8 + engine internals 0x5BEDF0/0x5BEF20 |
| 7 | 0x1C | 0x5C0090 | O | byte getter [this+0x44] |
| 8 | 0x20 | 0x5C0C30 | O | **`Gfx_Engine::CreateDevice`** (self-literals 0x6AE9CC/0x6AEA0C); calls JE::Gfx_D3D::CreateDevice 0x503920, buffer-mgr init 0x5067C0/0x5061E0, new(0x280)→cls_0x4c0190→[this+0x18] @0x5C0D19 |
| 9 | 0x24 | 0x5C1B30 | O | teardown/release-all: loops [this+0x894] array, dtor 0x4296C0 per node + Gfx_D3D 0x503C30/0x506260/0x506B70 + [0x70B200] reads — the unnamed twin of slot 8 (top-5 §4) |
| 10 | 0x28 | 0x5C05A0 | D | **FindMode forwarder**: `mov ecx,0x7082F8; jmp 0x502870` (T-052's refresh-rate wall function, b4-named band) |
| 11 | 0x2C | 0x5BF7E0 | D | device-parameter push to Gfx_D3D 0x503D90 after engine internals 0x5BEDF0/0x5BEF20 + record 0x424880 (device wrapper class) |
| 12 | 0x30 | 0x5BF8C0 | D | mode-apply/reset orchestration: FindMode 0x502870 @…, 0x503D90/0x503E30, reads Gfx_D3D+0x8C/0x90 — the T-056-relevant chain |
| 13 | 0x34 | 0x5C0520 | D | getter [0x708394] = Gfx_D3D+0x9C (b6's {width,height} value pair) |
| 14 | 0x38 | 0x5C00A0 | D | byte getter [0x708398] = Gfx_D3D+0xA0 |
| 15 | 0x3C | 0x5C0860 | D | device-wrapper byte OR-mask (arg via `or eax,0xffffff00` @0x5C086D) into [dev+?] |
| 16 | 0x40 | 0x5C00B0 | D | dev-wrapper [+0x20] = arg (store @0x5C00BF) |
| 17 | 0x44 | 0x5C00C0 | D | getter [0x70839C] = Gfx_D3D+0xA4 |
| 18 | 0x48 | 0x5C00D0 | O | getter [this+0x40] |
| 19 | 0x4C | 0x5C00E0 | O | getter [this+0x50] |
| 20 | 0x50 | 0x5C00F0 | D | read [[0x70838C]=Gfx_D3D+0x94 → +0xA8] |
| 21 | 0x54 | 0x5C0530 | D | same +0xA8 with clamp-max semantics (cmp/jbe @0x5C0543) |
| 22 | 0x58 | 0x5C0100 | D | chain → +0xB0 getter |
| 23 | 0x5C | 0x5C0560 | D | chain obj +0x60 indexed (null-guard @0x5C0571) |
| 24 | 0x60 | 0x5C0110 | D | dev-wrapper [+0x164] byte store |
| 25 | 0x64 | 0x5C0130 | D | dev-wrapper [+0x164]+0x38 byte getter |
| 26 | 0x68 | 0x5BDB20 | O | **`EnableProjectedShadow`** (self-literal 0x6ADBD0 "ProjectedShadowTex and/or ShadowTex not available"); gates [this+0xE8]/[0xEC] |
| 27 | 0x6C | 0x5C0140 | O | getter [this+0xE8] (shadow-enable byte — pairs with 26) |
| 28 | 0x70 | 0x5BDB70 | D | shadow pass A: Gfx_D3D 0x503400 + [0x70838C] chain reads |
| 29 | 0x74 | 0x5BDC10 | D | shadow pass B: same callee, second variant |
| 30 | 0x78 | 0x5BDDB0 | U | **[this+0xC] += arg via TexMgr-TU 0x51DA40** — clock/offset accumulator; 0x51DA40 reads byte [0x6FC0BB] — band-identified (TexMgr TU) but semantic role unresolved — FINAL-unknown: "callee band known, semantics unproven by any accepted row" |
| 31 | 0x7C | 0x5C0150 | O | getter [this+0x8] |
| 32 | 0x80 | 0x5C0160 | O | getter [this+0xC] (pairs with slot 30's delta field) |
| 33 | 0x84 | 0x5C0170 | O | float getter [this+0x10] (`fld`) |
| 34 | 0x88 | 0x5BDDF0 | O | int→float conversion store (fild @0x5BDE02) — writes a [this] float |
| 35 | 0x8C | 0x5C1940 | O | **TestCooperativeLevel/DeviceReset recovery** (self-literals 0x6AED60 "TestCooperativeLevel() failed: %x", 0x6AED84 "DeviceReset() failed: %x"); calls Gfx_D3D 0x503CD0 + `cls_0x490150::meth_0x494B40` — **one of T-063's 17 TRUE-ORPHANS surfaces here** (§4 shortlist) |
| 36 | 0x90 | 0x5C0180 | O | getter [this+0x18] — the DEVICE WRAPPER pointer (null-able pre-CreateDevice) |
| 37 | 0x94 | 0x5C0190 | O | getter [this+0x14] |
| 38 | 0x98 | 0x5C01A0 | O | getter [this+0x1C] |
| 39 | 0x9C | 0x5C1B00 | U | writes ([this+0x20],[this+0x24]) pair through `cls_0x4b3a10::meth_0x4B42F0` (12-byte holder, GUI-root-adjacent band, owner unattributed) — FINAL-unknown: "callee class is not a minted/attributed band; buffer role (stream? cache?) undetermined" |
| 40 | 0xA0 | 0x5C01B0 | D | dev-wrapper fallback-getter w/ arg null-check (this=[this+0x18] @0x5C01B7) |
| 41 | 0xA4 | 0x5C1B20 | D | tail-jmp `cls_0x4c0190::meth_0x429D20` on [this+0x18] |
| 42 | 0xA8 | 0x5C01D0 | D | dev-wrapper [+0x15C] getter |
| 43 | 0xAC | 0x5C01E0 | D | dev-wrapper [+0x160] getter |
| 44 | 0xB0 | 0x5C08A0 | D | stats-manager ([0x70C96C]) op via tail-jmp 0x5CB2A0 (DB puts this meth in cls_0x50b560 — observation-floor split suspect, b5 family) |
| 45 | 0xB4 | 0x5C08B0 | D | same manager → 0x5CABC0 |
| 46 | 0xB8 | 0x5C08C0 | D | same manager → 0x5CB6C0 |
| 47 | 0xBC | 0x5C08D0 | D | same manager → 0x5CAC00 |
| 48 | 0xC0 | 0x5C0A00 | D | texture create: SEH; `new→cls_0x50db20::ctor_0x4E1900` + TexMgr 0x521B10 + operator-delete on unwind ([0x70B200] refs) |
| 49 | 0xC4 | 0x5C0A90 | D | **index-by-name** (board slot; 14-arg SEH forwarder → 0x521CA0) |
| 50 | 0xC8 | 0x5C08E0 | D | tail-jmp → TexMgr 0x521DB0 |
| 51 | 0xCC | 0x5C08F0 | D | TexMgr 0x5218E0 w/ (a,b,1) push shape |
| 52 | 0xD0 | 0x5C0910 | D | tail-jmp → TexMgr 0x520AB0 |
| 53 | 0xD4 | 0x5C0920 | D | tail-jmp → TexMgr 0x51F1A0 |
| 54 | 0xD8 | 0x5C0930 | D | **id→texture OBJECT** (board slot; tail-jmp → 0x51F160) |
| 55 | 0xDC | 0x5C0970 | D | **acquire-by-name→id** (board slot; SEH → 0x51F570; EH cleanup 0x4B9D60) |
| 56 | 0xE0 | 0x5C0940 | D | buffer-manager add/lock ([0x707CE0] self-cache → 0x507660) |
| 57 | 0xE4 | 0x5C0950 | D | buffer-manager second shape (arg-swap thunk → 0x5077A0) |
| 58 | 0xE8 | 0x5C05B0 | U | (arg1,arg2[,0]) → 0x4948A0 — band 0x490–0x494 unattributed helpers — FINAL-unknown: "callee is a bare DB-less helper (join printed None); shape says pair-writer" |
| 59 | 0xEC | 0x5C05F0 | U | → 0x493CD0, wider (arg1,0,arg3,arg4) — same unresolved band — FINAL-unknown |
| 60 | 0xF0 | 0x5BDC90 | O+D | sets [this+0x88E] AND notifies Gfx_D3D 0x503320 with code 0x89 — the only engine-side latch+device-sync pair of its kind |
| 61 | 0xF4 | 0x5C01F0 | O | getter [this+0x88E] |
| 62 | 0xF8 | 0x5C0220 | D | builds 3 bytes from args → Gfx_D3D 0x503320 |
| 63 | 0xFC | 0x5C0200 | D | reads [[arg]] vft slot0 → Gfx_D3D 0x503300 code **0x8B** (device-caps query shape) |
| 64 | 0x100 | 0x5C06B0 | D | same code 0x8B + writes result back through [[arg]] |
| 65 | 0x104 | 0x5C0260 | D | same code 0x8B direct passthrough |
| 66 | 0x108 | 0x5C06F0 | D | dev-wrapper two-arg conditional op (arg-null fallback path) |
| 67 | 0x10C | 0x5C0720 | D | dev-wrapper single-arg conditional op |
| 68 | 0x110 | 0x5C0740 | D | pair → dev-wrapper |
| 69 | 0x114 | 0x5C0770 | D | single → dev-wrapper |
| 70 | 0x118 | 0x5C0790 | D | (arg2,[esp+0xC]) dev op — texture-unit style setter |
| 71 | 0x11C | 0x5C07C0 | D | same shape, second variant |
| 72 | 0x120 | 0x5C07F0 | D | single-arg dev op |
| 73 | 0x124 | 0x5C0810 | D | pair-with-guard dev op |
| 74 | 0x128 | 0x5BE010 | D | wide (9-arg-ish [esp+0x28]) dev op via helper 0x490330 (SEH helper) |
| 75 | 0x12C | 0x5BE060 | D | dev op via 0x4903E0 |
| 76 | 0x130 | 0x5BE080 | D | dev op via 0x490400 |
| 77 | 0x134 | 0x5BE0A0 | D | guarded dev-wrapper single-arg op |
| 78 | 0x138 | 0x5BE0D0 | D | guarded, arg2-led variant |
| 79 | 0x13C | 0x5BE100 | D | guarded variant 3 |
| 80 | 0x140 | 0x5C0840 | O | setter [this+0x892]=arg byte |
| 81 | 0x144 | 0x5C0850 | O | getter [this+0x892] |
| 82 | 0x148 | 0x5BFC50 | U | SEH two-arg conditional cleanup-ish body — FINAL-unknown: "guards both args, releases/zeroes on mismatch; no band callee hit — needs a xref-driven row" |
| 83 | 0x14C | 0x5BDE60 | D | arg==0 → dev-wrapper path else direct |
| 84 | 0x150 | 0x5C0280 | O | float setter [this+0x54] with !=0.0 guard (cmp const 0x6CB56C) |
| 85 | 0x154 | 0x5C02A0 | O | float getter [this+0x54] |
| 86 | 0x158 | 0x5C02B0 | O | float setter [this+0x58], same guard |
| 87 | 0x15C | 0x5C02D0 | O | float getter [this+0x58] |
| 88 | 0x160 | 0x5BDE90 | O | **defaults restore**: [this+0x54]=0.15f, [this+0x58]=1.5f (consts 0x6ADB7C/0x6ADB80) — CALLED BY THE CTOR @0x5C005C (owner proof) |
| 89 | 0x164 | 0x5BDEB0 | D | conditional dev-wrapper op ([esp+0x8] gate) |
| 90 | 0x168 | 0x5BDEE0 | O | **`SetProjectedShadowTexture`** (literals 0x6ADC88 "Setting a NULL texture!", 0x6ADCD0 "Unable to find texture!") + TexMgr lookup |
| 91 | 0x16C | 0x5BDF80 | O | shadow-point pool op on [this+0x64] (ctor calls it with size 0x32 @0x5C0063); new/delete via CRT tails 0x605554/0x616D87 |
| 92 | 0x170 | 0x5C02E0 | O | getter [this+0x5C] |
| 93 | 0x174 | 0x5BDFC0 | O | **`AddProjectedShadowPoint`** (literal 0x6ADD98 "Need larger buffer!!! (current size: %d)") — [this+0xE8]/[0x60]/[0x64] list machinery |
| 94 | 0x178 | 0x5C02F0 | D | **ScreenCapture**: `mov ecx,0x7082F8; jmp 0x502950` (IDB-named JE::Gfx_D3D::ScreenCapture, b4) |
| 95 | 0x17C | 0x5C1600 | O | **`SetLevelAABB`** (literal 0x6AEC30 "…failed to create VisibleMask texture: %s(%x) - %s" + "[VisibleMask]" 0x6AEC80); 0x340-byte SEH body; reads [0x708300]=Gfx_D3D+0x8; engine internals 0x5BE130/0x5BE380; string assign 0x401B20/0x401BD0 |
| 96 | 0x180 | 0x5C0300 | O | block-copy into [this+0x74]-anchored records (stride-walk loop @0x5C031x) |
| 97 | 0x184 | 0x5BE240 | O | float rect math (fld/fsub @0x5BE24A…) + sub-object ctor 0x5BE130 — AABB/region helper |
| 98 | 0x188 | 0x5C0E10 | O+D | SEH named-register: string assign + **`cls_0x4d8d70::meth_0x49C7A0`** (StringTree-family `_Tree` insert helper) + engine sub-ctors — a registry-insert façade (ties to the split-pad/item-43 family) |
| 99 | 0x18C | 0x5C0340 | O | getter [this+0xA4] |
| 100 | 0x190 | 0x5C0350 | O | getter [this+0x68] byte — **THE EXPORT HOLE SLOT** (F3.json length=120 but entry absent; raw live) |
| 101 | 0x194 | 0x5C0360 | D | pushes [this+0xD4] then calls **own vft slot 54** (`call [eax+0xd8]` @0x5C036B, eax=[this]) — "object-by-id of MY texture A" convenience |
| 102 | 0x198 | 0x5C0370 | D | same via [this+0xD8] ("MY texture B") |
| 103 | 0x19C | 0x5C0380 | O | getter [this+0xDC] |
| 104 | 0x1A0 | 0x5BEB40 | U | float-vs-1.0 compare (const 0x647880=1.0f) + CRT-free 0x616E24 → pair — FINAL-unknown: "scale-clamp + release semantics plausible, no accepted row pins it" |
| 105 | 0x1A4 | 0x5BEBB0 | U | same family, second variant — FINAL-unknown |
| 106 | 0x1A8 | 0x5C0390 | O | getter [this+0xF0] |
| 107 | 0x1AC | 0x5C03A0 | O | out-param read of [this+0xE0]/[0xE4] pair |
| 108 | 0x1B0 | 0x5C0C00 | O | string-copy loop + assign helper 0x401BD0 (name setter shape) |
| 109 | 0x1B4 | 0x5BDF30 | O | **`SetSilhouetteEdgeTexture`** (literals 0x6ADD18/0x6ADD58 NULL/not-found pair) |
| 110 | 0x1B8 | 0x5C03C0 | O | getter [this+0xF4] |
| 111 | 0x1BC | 0x5C03D0 | O | setter [this+0x88C]=arg byte |
| 112 | 0x1C0 | 0x5BDE20 | D | byte flag → Gfx_D3D 0x503320 (code 0x2/else-shape, this=0x7082F8) |
| 113 | 0x1C4 | 0x5C03E0 | D | same sync pair codes {2,8} → 0x503320 |
| 114 | 0x1C8 | 0x5C0410 | O | setter [this+0x890]=arg byte |
| 115 | 0x1CC | 0x5C0420 | O | getter [this+0x890] |
| 116 | 0x1D0 | 0x5C0640 | D | writes [arg+0x164] byte — propagates dev-wrapper flag INTO caller's wrapper (shared 0x164 field family w/ 24/25/117) |
| 117 | 0x1D4 | 0x5C0680 | D | reads [arg+0x164] → dev-wrapper op |
| 118 | 0x1D8 | 0x5BEC20 | O+D | 0x110-byte state apply: Gfx_D3D 0x503450/0x5037B0, statics [0x70830C]=D3D+0x14 / [0x708328]=D3D+0x30, LogDebugString 0x497120 — unnamed device-flush (top-5) |
| 119 | 0x1DC | 0x5C0430 | D | null-guarded tail-jmp → TexMgr 0x51F6D0 on **[this+0x2C]** (id-0/absent-sentinel discipline, T-066 catch) — release/flush shape |

## 4. Bucket counts + shortlist

**Counts (partition of 120):** D (delegation-identified) = **68**; O (owner-named/engine-state,
incl. the 7 literal-named methods + deleting dtor; mixed O+D rows counted O) = **45**; U (FINAL-
unknown, each printed reason) = **7** — slots 30, 39, 58, 59, 82, 104, 105.

**Top-5 most interesting unnamed slots:**
1. **slot 9 (0x5C1B30)** — 416-byte teardown loop over [this+0x894]; the unnamed TWIN of
   CreateDevice (slot 8): whoever ports device lifecycle needs its exact release order.
2. **slot 35 (0x5C1940)** — TestCooperativeLevel/DeviceReset recovery loop — and a **consumer of
   `cls_0x490150::meth_0x494B40`, one of T-063's 17 TRUE-ORPHANS**: this window onto the orphan
   set is the first anchored host-class candidate (host = Gfx_Engine vft path ⇒ the orphan is a
   Gfx-band helper, item-41 three-query fodder for T-073's lane).
3. **slot 118 (0x5BEC20)** — the big device-flush: Gfx_D3D calls + [D3D+0x14]/[D3D+0x30] mirrors
   of the block slot 6 copies — the actual per-frame render-state contract (M-001/D3D8-slice
   adjacent).
4. **slots 11/12 (0x5BF7E0/0x5BF8C0)** — mode-apply/reset orchestration THROUGH FindMode 0x502870
   — the very function T-052's wall killed on (RefreshRate<60 rejection); if gameplay depth ever
   needs a display-mode change live, these are the two entry points.
5. **slots 101/102 (0x5C0360/0x5C0370)** — self-slot-54 convenience ("my texture A/B" by stored
   id [this+0xD4]/[this+0xD8]) — shows the engine's private texture slots and names the pair for
   porting (nothing else in the corpus references +0xD4/+0xD8).

Honorable mention: **slot 100** is itself the finding — the export hole (report to T-040's list:
length=120 declared, entries=119 emitted, idx-100 body exists).

## 5. Hygiene statement (row contract)

* Read-only: image + `artifacts/F3.json` + `/tmp` scratch; zero IDB connections; zero docs/
  discoveries/ edits besides this file (left DIRTY).
* Every classification row cites body EAs read this sweep; every board-anchor claim was reproduced
  by the SAME dumper before unknowns were allowed (§0); export claims (length/size/entries) treated
  as claims, measured against raw (§1). Item-8c discipline reused from T-078: aligned disassembly
  for all referrer/store counts (the imm-scan found exactly the two `c7 06` stores the dtor-restore
  story explains).
* Item-42 RTTI-size rule: the single-COL/primary-vft check ran before the owner claims; item-44:
  slot 36 exposes [this+0x18] to arbitrary callers — the device wrapper SHARING caveat is stated
  in the row (not a membership claim).

## 6. Suggested board disposition

T-083 done: 120/120 slots classified (68 D / 45 O / 7 U-with-reasons); six board anchors
REPRODUCED pre-classification; two NEW export erratum candidates (idx-100 hole; engine+0x18 vs
slot-manager +0x3C offset disambiguation for T-062-addendum readers); orphan-window lead routed
(slot 35 → cls_0x490150) for T-073's lane.

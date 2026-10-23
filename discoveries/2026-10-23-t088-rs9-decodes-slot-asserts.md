# T-088 — R-S9 decode receipt: s9 teardown twin, s101/102 texture-id chain, D3D8 slot asserts (splitseat, 2026-10-23)

Status: findings per the lane contract (offline, no IDB writes; evidence = live decompile on ida_original_f3
+ DOCUMENTED vtable order from local `d3d8.h`, mingw-w64 14.0.0 — `/nix/store/cr2hgxq2qz…-mingw-w64-i686…/include/d3d8.h`).

## Item 1 — s9 device-teardown twin: ASSERTED
`0x5C1B30` = `JE::Gfx_Engine::virt_meth_0x5c1b30` = **release-all device resources** — the unnamed TWIN of
slot 8 `Gfx_Engine::CreateDevice` (T-083 census row 84, role confirmed by decode):
- COM-releases 8 members via their own vft slot 2 (Release): `this->mbr_0x894/0x898/0x89C/0x8A0` (the adjacent
  device-object pointer block T-083 called the "+0x894 array" — decode shows 4 adjacent scalars, not a literal
  array) plus `0x9C/0xA0/0xDC/0xF0`; each nulled after release.
- Destroys 4 texture-id members via `gfx_system_interface` vft slot 52 (=TexMgr destroy, census row 127:
  tail-jmp → 0x520AB0): `0xF4/0xEC/0xD4/0xD8`.
- Frees the two `cls_0x4296c0` nodes (`mbr_0x18/0x1C`) with `dtor_0x4296c0` + free.
- Side-calls into the Gfx_D3D band with global `this`es: `sub_5CC410`, `sub_506260`, `meth_0x5cbdb0`,
  `meth_0x5cbe20`, `Gfx_Texture_Manager::meth_0x520a60([0x70B200])`, `sub_506B70([0x707CE0],1)`,
  `sub_503C30(&dword_7082F8)` — the engine's shutdown sequence across its subsystem singletons.

## Item 2 — s101/102 private-texture-id chain: ASSERTED
`0x5C0360`/`0x5C0370` = one-liners: call `gfx_system_interface` vft slot 54 (`call [eax+0xD8]` tail)
with `this->mbr_0xD4` / `this->mbr_0xD8`. Slot 54 per T-083 §8e = 0x5C0930 → `mov ecx,[0x70B200]; jmp 0x51F160`
= texture OBJECT-BY-ID into the TextureMgr. So `[this+0xD4]`/`[0xD8]` are the engine's TWO REGISTERED TEXTURE
IDS; s101/102 are the "get my texture object by stored id" conveniences, and s9 destroys them via slot 52.
The get/destroy pair (slots 54/52) brackets the lifetime cleanly.

## Item 3 — b5 D3D8 slot-numbering note: UNASSERTED → ASSERTED (documented order)
Header source (d3d8.h): `IDirect3D8` slots: 0-2 IUnknown, 3 RegisterSoftwareDevice, 4 GetAdapterCount,
5 GetAdapterIdentifier, 6 GetAdapterModeCount, 7 EnumAdapterModes, 8 GetAdapterDisplayMode, 9 CheckDeviceType,
10 CheckDeviceFormat, 11 CheckDeviceMultiSampleType, 12 CheckDepthStencilMatch, 13 GetDeviceCaps,
14 GetAdapterMonitor, 15 CreateDevice.

Band census (scripted indirect-call scan, 0x502000-0x507500, 40 distinct offsets) — attributed sites:
- **CreateDevice_503920 @0x503B38**: `call [ecx+3Ch]` on the interface at `[this+1]` = slot 15 =
  **IDirect3D8::CreateDevice** — arg shape matches exactly (Adapter, DeviceType=**(this+37), hFocusWindow,
  BehaviorFlags, PRESENT_PARAMETERS=this+3, returned device=this+2); FatalError on failure. ASSERTED.
- **EnumDevices_5047a0**: calls at +0x10/0x14/0x18/0x1C/0x20/0x24/0x28/0x34 = slots 4,5,6,7,8,9,10,13 =
  GetAdapterCount / GetAdapterIdentifier / GetAdapterModeCount / EnumAdapterModes / GetAdapterDisplayMode /
  CheckDeviceType / CheckDeviceFormat / GetDeviceCaps — the canonical enumeration sequence. ASSERTED (8 calls).
- **ScreenCapture**: +0x20 (slot 8 GetDisplayMode), +0x6C (slot 27 CreateImageSurface), +0x78 (slot 30
  GetFrontBuffer) — the exact capture pipeline shape. ASSERTED (3 calls).
- **+0x8 (slot 2 Release)** across the band (0x5024D0, 0x503C30, 0x506260, 0x506B70, …). ASSERTED.
- Remaining ~20 offsets (0x28/0x30 families in 0x5026c0; 0xB0/0xB8 in 0x5062D0; 0xC8/0xFC in 0x503xxx;
  0x14C/0x154 in 0x506760; HeightFog 0xD8/0xE0) run through OTHER COM interfaces (surface/vb/texture) —
  per-site register provenance is the honest successor pass; do NOT map them by vibes (slot-assert contract).

## MapFile object @ Gfx_D3D+0x40 (static instance 0x7082F8)
- `+0x84` (0x70837C): adapter count (bounds check in accessor).
- `+0x88` (0x708380): per-adapter table pointer, **1,100-byte stride**; accessor `0x5C0450` reads
  record field `+0x42C` (slot-1 accessor); record layout corroborated by CreateDevice's walks
  (`v12[268..274]` = record offsets 0x430..0x448 — the +0x42C field abuts the mode/format fields).
- `+0x90` (0x708388): count/read by `0x5BF8C0`; `+0xA0` (0x708398): rebuild gate — when 0, `0x5BF770`
  (the current-mode byte setter at 0x708988) rebuilds the device chain (`meth_0x5bedf0` → present-params
  qmemcpy from +0xC → `Gfx_D3D::meth_0x503cd0` → `meth_0x5bef20`).
- `+0x70` (0x708368): entry array — NO direct xrefs (reached via obj-base arithmetic); stride semantics
  remain the open successor item.

## Queue items (writer slot, T-084 shape)
1. Per-site provenance pass for the ~20 un-attributed offsets (surface/vb/texture interfaces).
2. MapFile entry-array stride + count semantics decode (+0x70).
3. Role names for the s9 side-call band (sub_503C30/506260/506B70/5CC410) — candidates for sub_-prefix
   promotion with call-context evidence.

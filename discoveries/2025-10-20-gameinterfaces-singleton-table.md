# 2025-10-20 — Session 26: GameInterfaces singleton table (24 named interfaces) + slot map — UI thread OPENS

Target: ORIGINAL_REFERENCE (IDA). Read-only. Opens the interface/UI thread, complementary to
implementer-B's .INT-file work (tags/format) — this is the RUNTIME registry. All of §1 comes
from one function, `F3::GameInterfaces::ctor_0x4398e0` (verbatim EAs there).

## 1. The shipped interface singleton table

`F3::GameInterfaces` (100 bytes) = `JE::Interfaces interfaces` @+0 (ctor 0x5967f0) + 24
interface-instance ptrs. Each is allocated (custom allocator `sub_4A7680`), constructed with
its **name string** (the same name the .INT file loader will bind — B's lane), then
`meth_0x495220(obj, 1)` (interface-base **SetActive(1)** — see session-27 §2 for the
correction of this doc's first-draft "Register" guess) is called
on every one at startup:

| slot | name | instance size | ctor | owner-field |
|------|------|--------------|------|-------------|
| +0x04 | `"Mainmenu"` | 0x44 | 0x440990 | mbr_0x4 |
| +0x08 | `"LoadMap"` | 0x40 | JE CLoadMapInterface 0x440050 | mbr_0x8 |
| +0x38 | *(nameless)* | **0x15C** | 0x42a780(this,1) | mbr_0x38 — big; cursor/HUD? (has own get/set pair 0x439a6d..0x439a9c) |
| +0x10 | `"Actionbar"` | **0x2D0** | 0x41a2b0 | mbr_0x10 |
| +0x0C | `"Feedback"` | 0xE4 | 0x434f70 | mbr_0x0C |
| +0x18 | `"Inventory"` | **0x2F4** | 0x43cd50 | mbr_0x18 |
| +0x1C | `"EscapeMenu"` | 0x40 | 0x433a90 | mbr_0x1C |
| +0x44 | **"Pipboy"** | **0x214** | 0x455e40 (raw-confirmed 0x439d75..0x439dd9: name aPipboy, size 214h, owner [esi+44h]) | mbr_0x44 |
| +0x20 | `"LoadingBar"` | 0x40 | 0x4407c0 | mbr_0x20 |
| +0x48 | `"MiniMap"` | 0x68 | 0x442340 | mbr_0x48 |
| +0x4C | `"Options"` | 0x68 | 0x4436d0 | mbr_0x4C |
| +0x50 | `"OptionsLoad"` | 0xA8 | 0x448c40 | mbr_0x50 |
| +0x54 | `"OptionsGame"` | 0x140 | 0x444530 | mbr_0x54 |
| +0x58 | `"OptionsHotKeys"` | 0x1CC | 0x4468c0 | mbr_0x58 |
| +0x5C | `"OptionsMultiplayer"` | 0x40 | 0x4491f0 | mbr_0x5C |
| +0x60 | `"OptionsVideo"` | 0x100 | 0x4496a0 | mbr_0x60 |
| +0x28 | `"Container"` | 0x1B8 | JE CWorldContainerInterface 0x457db0 | mbr_0x28 |
| +0x14/+0x2C/+0x30/+0x34/+0x3C | (5 more ptr slots, zeroed in ctor, set later — ⚠ NOT Dialogue/CSMenu: session-28 showed those are CLIENT-factory virtuals (GameClient vft 0x664804 slots 43/47/48/50); owners of these 5 slots still unattributed — see `2025-10-20-gameclient-vft-demand-ui-factories.md`) | | | |

(Every ctor'd entry gets `SetActive(1)` @0x439a03/0x439aa1/0x439b81/… — so the ORIGINAL
pre-registers ALL these interfaces at startup; late slots (+0x14 etc.) are the
runtime-created ones.)

## 2. Virtual surface + accessors

* vftable: `GameInterfaces_Interfaces_vftable_656650` (slot writes at ctor 0x439913).
* `JE::Interfaces` base is 4 bytes (pure-vft); the game subclass holds everything in +4.. —
  i.e. the singleton table IS the class.
* Accessor pairs (get/set) exist for at least: 0x38 (get 0x4399f7?/set 0x439a2x region —
  `meth_0x4396f0`@+4, `meth_0x439720`), 0x18 (0x439c94 ↔ 0x439cc3), 0x1C (0x439d35/439d64),
  0x44 (0x439dd9/439e08), 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60, 0x28 — bodies are 2–3
  instructions (return field / store + side-effect at 0x4397e0..0x439895 block touching
  mbr_0x10 accessors — the earlier "1..6 SET-FOCUS switch" guess in this doc's first draft
  was WRONG: those EAs are plain get/set bodies; per-slot semantics now in session-27 §1).
* Small virts 0x4396c0 (reads +0xC), 0x4396f0 (reads +4), 0x439720 (+0x20 w/ switch).
* dtor 0x4396b0 frees +4 (the first interface only?) — shallow; the engine base dtor likely
  sweeps registered interfaces.

## 3. Names → docs/f3 stubs
The strings map 1:1 to the existing (stub) doc files: CMainmenuInterface, CLoadMapInterface,
CActionbarInterface(=CGameInterface child), CFeedbackInterface, CInventoryInterface,
CEscapeMenuInterface, CLoadingBarInterface, CMiniMapInterface?, COptions* set,
CWorldContainerInterface, and the not-yet-ctor'd ones (GameDialogueInterface,
GameCSMenuInterface, CPipBoyInterface, GameInterfaces.md itself…). This table is the
claim-sheet for filling those stubs: names, sizes, ctors, vft 0x656650 slots.

## 4. Suggested order for the UI thread (mine unless claimed)
1. ~~`JE::Interfaces` base + `meth_0x495220` = Register/Demand semantics~~ CLOSED by session 27
   §2 (SetActive + visible-stack/modal interaction); focus/stack semantics there too — 1..6
   dispatch guess turned out: individual named hooks per slot (session 27 §1)
2. Focus/dispatch virts 0x4397e0/0x439810/0x439850 — CLOSED by session 27 §1 (actionbar show/
   re-assert/layout-reflow; no 1..6 switch — that was accessor noise, corrected).
3. Then per-interface deep-dives on demand from A/B (Inventory 0x2F4 and Actionbar 0x2D0 are
   the two heavyweights; Pipboy/Dialogue are the gameplay-critical ones for B's GUI recon).

## 5. Evidence
Ctor 0x4398e0 (allocs 0x439966..0x43a375; names at 0x4399a4 "Mainmenu", 0x439a48 "LoadMap",
0x439b25 "Actionbar", 0x439bc9 "Feedback", 0x439c73 "Inventory", 0x439d14 "EscapeMenu",
0x439e59 "LoadingBar", 0x439f03 "MiniMap", 0x439faa "Options", 0x43a054 "OptionsLoad",
0x43a103 "OptionsGame", 0x43a1b3 "OptionsHotKeys", 0x43a25a "OptionsMultiplayer",
0x43a304 "OptionsVideo", 0x43a3b3 "Container"; register calls per §1). Full decompile dump:
/tmp/pi-mcp-07a96ceb8bf3e456.txt (regenerate: decompile 0x4398e0). Slot+accessor lists §2.
⚠ none outstanding — Pipboy entry raw-confirmed (§1 row).

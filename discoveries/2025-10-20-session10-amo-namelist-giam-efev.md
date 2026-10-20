# Session 10 — .AMO prototype name list (registry origin) + GIAM wire map + EFEV tree-slot decode

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes session 9 §4 "writer of the 20 ammo names"; adds the EFEV root-slot semantics
(session 8 §6 open item on tree indices).

## 1. The 20 ammo prototype names are HARD-CODED strings (CRT static init)

Found by scanning for absolute references to the name-array bytes: the writer of
`byte_6F7E14..0x6F8044` is the **C++ static initializer block in .text @0x640DF0-0x641227**
(free-standing code, one `JE::String::Assign_0x401bd0` + SSO header init per record, stride
28 bytes). Full ordered list (each string is an **`.AMO` template resource file name**):

| # | slot @0x6F7E14+28n | name (evidence EA of the push) |
|---|---|---|
| 0 | 0x6F7E14 | `Ammo12_ga.AMO` (0x640e0c) |
| 1 | 0x6F7E30 | `Ammo15mm_EC.AMO` (0x640e4a) |
| 2 | 0x6F7E4C | `Ammo223.AMO` (0x640e7d) |
| 3 | 0x6F7E68 | `Ammo22.AMO` (0x640eb0) |
| 4 | 0x6F7E84 | `Ammo22_Injector.AMO` (0x640eea) |
| 5 | 0x6F7EA0 | `Ammo2mm_EC.AMO` (0x640f1d) |
| 6 | 0x6F7EBC | `Ammo30.AMO` (0x640f50) |
| 7 | 0x6F7ED8 | `Ammo40mm_Rifle_Grenade.AMO` (0x640f8a) |
| 8 | 0x6F7EF4 | `Ammo44.AMO` (0x640fbd) |
| 9 | 0x6F7F10 | `Ammo45.AMO` (0x640ff0) |
| 10 | 0x6F7F2C | `Ammo50.AMO` (0x64102a) |
| 11 | 0x6F7F48 | `Ammo762mm.AMO` (0x64105d) |
| 12 | 0x6F7F64 | `Ammo9mm.AMO` (0x641090) |
| 13 | 0x6F7F80 | `AmmoBB.AMO` (0x6410ca) |
| 14 | 0x6F7F9C | `AmmoMicro_Fusion_Cell.AMO` (0x6410fd) |
| 15 | 0x6F7FB8 | `AmmoNaphate.AMO` (0x641130) |
| 16 | 0x6F7FD4 | `AmmoHeavy_Rivet.AMO` (0x64116a) |
| 17 | 0x6F7FF0 | `AmmoLil_Rivet.AMO` (0x64119d) |
| 18 | 0x6F800C | `AmmoRocket.AMO` (0x6411d0) |
| 19 | 0x6F8028 | `AmmoSmall_Energy_Cell.AMO` (0x64120a) |

Mechanics (`GameWorld::meth_0x470220`, session 9): for each name → `new GameAmmo` →
`Entity::Create(proto, name)` → Create's name-association (@0x41e4b0 = `cls_0x50db20::meth`,
resolves the **.AMO resource** through the resource system) → the loaded template's
(caliber, modifier) land at **GameAmmo+0x3B0 / +0x3B4** (registry index operand raw @0x4702a4:
`mov ecx,[esi+3B0h]; imul ecx,7; add ecx,[esi+3B4h]`; esi = GameAmmo*) → registry slot ← netId
(`[proto+vbdisp+0x1E0]`). The atexit cleanup registers `sub_645CA0` for the same block.

⇒ **The complete shipped ammo universe = these 20 calibers.** Registry occupancy after init:
one prototype per name at slot `7*caliber + modifier` (modifier from the GIAM v2 field —
if shipped .AMO files are ver <2, modifier=0 → slots = 7*caliber).

## 2. GIAM wire map (`sub_408420`, reader this per fourcc matrix = Ammo−0x3D8)

| gate | reader offsets (dword each) | role (inferred by pairing with usage) |
|---|---|---|
| v0 (always) | +944, +952, +956, +960, +964 | ammo stat block; +944 = **caliber** (registry idx operand 1) |
| v≥1 | +972, +976 | two extra dwords |
| v≥2 | +948, +968 | +948 = **modifier** (registry idx operand 2) — v0 data leaves modifier 0 |

*(Slot arithmetic caveat carried from session 9 §1: registry reads use GameAmmo-base 0x3B0/0x3B4;
the GIAM reader's numeric offsets 944/948 = 0x3B0/0x3B4 match exactly — strong evidence the
"Ammo−0x3D8" `this` annotation in the fourcc matrix is itself the −4-biased spelling, i.e.
reader base == GameAmmo base here. Verify against one debugger dump.)*

Community cross-check *(external, not from this binary)*: Fallout 2 prototype `.pro` item lists
use the same ammo naming (9mm, .44, .45, 5.56mm, 7.62mm, plasma microfusion cells, etc.) — the
20-name list is consistent with F2's ammo set; no reimpl action needed, just a sanity anchor.

## 3. EFEV header decodes the tree slot (session 8 §6 follow-up)

EFTR reader chain (`cls_0x463870::meth_0x468930` → `meth_0x468730` → `meth_0x468490`):
```
EFTR { count u32; EFEV×count }
EFEV { treeIdx u32 (chunk ver≥1), rootCount u32, rootCount × (EFDT|EFCD chain) }
  → meth_0x468490(..., chunk=treeIdx) → sub_468310(&template+0x38 + 12*treeIdx, &node)
```
- **treeIdx is file-supplied**, 0..4 (`GameEntity+0x38` array = 5×12B headers, session 4).
- Confirmed consumers so far: tree 0 = equip/unequip (`ApplyEquipEffects_46ba30`, `push 0`
  @0x46ba50), tree 3 = OnCreate (`ApplyOnCreateEffects_466fe0`, param 3 @0x466fe8).
- No accessor call sites for trees 1, 2, 4 exist in F3.exe besides the dtor cleanup loop
  (0x467af0 walks all 5). ⇒ **statically unresolvable semantics** — they are either consumed by
  the net-server path (dead in client demo) or reserved. Action for reimpl: dump each map/entity
  EFTR `treeIdx` values from shipping data; if only {0,3} occur, store them as two named vectors
  (OnEquip, OnCreate) and keep the other three as parse-and-forget.
- `meth_0x468730` also shows EFEV's own chunk `size>=1` gate for treeIdx: **version-0 EFEV
  records (if any exist in data) default treeIdx=0** (skip-read → `chunk` field retained).

## 4. Donation / corrections

- Session 9 §4 item 1 ("writer of the 20 ammo names") → CLOSED here (§1): CRT static init, not
  a resource scan. Reimpl: hard-code the 20-name list (order irrelevant), create prototypes at
  world init exactly like `meth_0x470220`, read (caliber,modifier) from the loaded .AMO GIAM
  fields at +0x3B0/+0x3B4, index registry as `7*caliber+modifier`.
- Session 8 §6 tree-index note refined: the index is data-driven (EFEV header dword), not
  implicit per-file (§3).
- Still open this line: `GameEntity+0x18` consumers; clock-object class at World−0x364; the
  6→7 state-flip server path; GIWP-vs-GCRE GWAM `this`-base consistency check (session 6 §9).

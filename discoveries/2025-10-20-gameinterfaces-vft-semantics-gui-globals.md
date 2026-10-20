# 2025-10-20 — Session 27: GameInterfaces vft semantics + GUI globals (visible-stack, modal root)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Second UI-thread pass; maps every non-trivial
slot of `GameInterfaces_Interfaces_vftable_656650` and names the GUI globals the interface
objects share. Follows `2025-10-20-gameinterfaces-singleton-table.md`.

## 1. vftable slot semantics (10 slots; bodies 0x4396b0..0x4398e0)

| slot | body | semantics (evidence) |
|------|------|----------------------|
| +0x00 | 0x4398c0 | dtor |
| +0x04 | 0x4396c0 | **feedback-geometry hook**: Feedback iface (mbr_0x0C) `meth_0x434910(buf,280,132)` + `meth_0x434120(10,620)` — layout constants for the feedback display (position + extent) |
| +0x08 | 0x4396f0 | **root-interface setter**: `cls_0x4b28a0::meth_0x4b4f00(&GUI_ROOT(0x707E48), Mainmenu-base or 0)` — the modal/GUI root's "current" gets the Mainmenu interface (vb-rebased) |
| +0x0C | 0x404b70 | `ret` — deliberately unimplemented base virtual |
| +0x10 | 0x439720 | **LoadingBar visibility**: `SetActive(LoadingBar, !param)` (0x495220 = SetActive, §2) then base virt 0x5a9320(param) propagates to engine |
| +0x14 | 0x439770 | **area-map-enter event**: RTTI `JE::Client→F3::GameClient` on `[0x70BD90]+0x2C` (client global, §3); `client->+0x29 meth_0x436ad0(1)` (area seen/entered flag); then Actionbar: SetActive(0) + `meth_0x41c910(client+0xE0)` refresh |
| +0x18 | 0x4397e0 | **actionbar show**: SetActive(Actionbar,1) + `sub_416B20()` refresh; tail base |
| +0x1C | 0x439810 | **actionbar re-assert-if-hidden**: if `Actionbar.Get+0x20()==0` → `meth_0x41c9d0(client+0xE0)` |
| +0x20 | 0x439850 | **layout/resize routing**: base 0x596800(a2,a3,a4,a5) first; if Actionbar not active → `cls_0x416f80(this, &a2..&a5)` reflow + `*a5 += a5−192` (192-constant vertical adjust — actionbar height) |
| +0x24,+0x28 | 0x404b70 | `ret` no-ops |

JE::Interfaces base vft 0x69f3ec (10 slots): only +0x00 (0x596840), +0x10 (0x5a9320), +0x20
(0x596800) are non-ret; the base is a thin NOTIFIER — the game subclass carries ALL state
(§singleton table), confirming the manager-pattern from session 26.

## 2. `meth_0x495220` (interface-base virtual) = **SetActive(bool)** — not Register

Correction to session 26 §1's phrasing ("Register(1) at startup"): the per-entry ctor call
`meth_0x495220(obj, 1)` sets the interface's **+32 byte = active/visible** and:
- pokes GUI_ROOT flag: `cls_0x4b28a0::Set_mbr_0x4_to_1_4b2ab0(&0x707E48)` (root "dirty/active")
- if activating AND this object is the FIRST in the VISIBLE STACK (`byte_707F00` list, walked
  by vft+152=get-current, vft+160=get-next): clears root field via `meth_0x4b2ad0(0)`
⇒ top-of-stack interface activation interacts with a root modal flag — the engine's UI focus
rule: activating the head interface RESETS the modal-dirty flag. Reimpl: model per-interface
`active@+32` + the stack walk when porting focus behaviour.

## 3. GUI globals named (⚠ IDB names in this corridor are unreliable — trust the EAs)

| global | role | evidence |
|--------|------|----------|
| `unk_707E48` (cls_0x4b28a0) | **GUI root/modal manager**: Set-to-1 (0x4b2ab0), meth_0x4b2ad0(0), meth_0x4b4f00(set current iface) | §1 +0x08, §2 |
| `byte_707F00` | ⚠ NOT a separate global — IS `GUI_ROOT(0x707E48)+0xB8` = modal/current interface chain head (0x707E48+0xB8=0x707F00); walked by vft+152=get-current, vft+160=get-next ⇒ | session-29 §1 correction |
| `dword_70BD90` | client-holder: `[+0x2C]` = `F3::GameClient*` (RTTI'd at 0x43978a), `client+0xE0` feeds actionbar refresh | §1 +0x14/+0x1C |
| ⚠ `global_area_map_icons` (IDB name, 0x70BE08-ish) | used at 0x43978a-equivalent sites as `->int_0x2c` == client ptr; the SAME `->Get_mbr_0x4 &1` pattern was our long-standing "server flag" — so this global is a multi-singleton block, NOT area-map-icons. Name-fix candidate for the IDB (do it in the ORIGINAL db when idle; discovery won't edit) |
| `byte_707CE4` | game-options/settings singleton (vft+316 canSeeGate etc., earlier sessions) | cross-ref |

## 4. What the base virts are called BY (engine contract, for the port)
+0x04/+0x08 fire from GUI internals; +0x10 (loading toggle) and +0x20 (layout) are called by
map-load/resize paths; +0x14 by area-map entry. To pin exact callers: `call [reg+0x14h]`-style
sweeps are AMBIGUOUS (vft slot collision everywhere) — the reliable route is xrefs from the
JE::GUI/World code that already reference the Interfaces manager singleton (whose global:
`F3::global_gui_interface_0x70be08` per entity-file doc §? — SAME 0x70BExx block family;
confirm during the IDB name pass).

## 5. Open (UI thread)
- GameClient RTTI'd object layout at [0x70BD90]+0x2C (fields +0x29 flag, +0xE0) — next pass.
- cls_0x4b28a0 (GUI root manager) full surface — one more function-rich class.
- Actionbar internals (mbr_0x10, 0x2D0 bytes, meth_0x41c910/0x41c9d0/0x416f80/0x416b20) —
  big; only when A/B need the HUD.
- The 5 demand-spawned slots (Dialogue etc.): find WHO sets mbr_0x14/2C/30/34/3C.

## 6. Evidence
Bodies §1 EAs inline; SetActive §2 0x49522c..0x495284; base vft dump 0x69f3ec (9 entries +
END at 0x69f410); GameInterfaces vft 0x656650 (10 slots, END marker 0x746e6f43='Cont' string
start follows — table boundary exact). +0x1C raw 0x439810..0x439842.

# 2025-10-21 — T-017 pass 4 (PureKnight 01a15cb7-e27e): blk allocation pass SOLVED, 11 un-sampled handlers classified, pass-3 correction issued

Target: ORIGINAL_REFERENCE (`ida_original_f3` + raw `$OPENVB_ORIG_EXE` objdump; no IDB writes).
Builds on B39 (`2025-10-20-f3-ui-command-handlers-window-slots.md`).

## 1. blk allocation pass — the answer

The window-slot block is allocated by a **virtual factory chain run once from `JE::Client::virt_meth_0x58f010`**
(UI bootstrap, 0x224 bytes):

- `0x58f1ef: call [client_vft + 0xB0]` → concrete override `F3::GameClient::virt_meth_0x45e8e0`
  (DB vftable `0x664804` slot 44): `new 0x64` → `ctor_0x4398e0` → returned object stored
  **`[client+0xA0]` at `0x58f1f5`**, then `blk->vft[+8]` invoked (`0x58f20a`).
- Same bootstrap builds the neighbours: vft+0xAC → `client+0x9C` (the modal gate), vft+0xB4 → `client+0xA4`
  (dialog interface, 0xB0-byte object), vft+0xB8 → `client+0xA8`, vft+0xBC → `client+0xB0`… — the "blk" is one
  member of a whole client UI cluster installed there, all via `SetActive(1)`/`vft[+4]/[+8]` init calls.
- The ONLY direct caller of `0x58f010` found: `0x4618f4` inside `GameClient::virt_meth_0x4618f0`
  (vft slot 1 = **install-once-and-forward**: 4618f0 lazily boots the UI cluster, then closes the active modal
  `[[client+A0]+4]` and tail-jumps `0x460d60`). ⇒ pass-3's "no install store" is explained: install is lazy at
  first vft-slot-1 call, not at ctor.
- **blk class** = vft `0x656650`, size 0x64: 24 dword slots `+0x04..+0x60` ctor-zeroed (`0x439919..0x43995e`),
  then the ctor BUILDS the windows itself: per window = `sub_4a7680(size)` factory + name string registered via
  `sub_440990` into the session-38 name registry + `SetActive(1)`; slot stores literal `mov [esi+off], win`.

## 2. The full slot table (factory name → blk offset; consumers cross-checked)

| blk off | window | cross-check |
|---|---|---|
| +0x04 | Mainmenu | `blk[+4]` = "active modal" pattern in 0x4618f0/0x58f1f5-adjacent ✓; root bootstrap sess-27 |
| +0x08 | LoadMap | |
| +0x0C | Feedback(see caveat) | pass-2 `sub_45E6C0` hook 0x430760@idx9 ≠ this idx3; Feedback hook note was on idx9 |
| +0x10 | Actionbar / FeedbackWindow surface | tick `blk[+0x10]→0x41c4c0` (pass 2 ✓); `CommandFeedbackWindow 0x45ee00` AND `CommandActionbarWindow 0x45ea50` BOTH drive `[blk+0x10]` (0x45ea5f/0x45ee0f); sess-37 actionbar chain ✓ |
| +0x18 | Inventory | B39 slot A ✓ (0x45eacd) |
| +0x1C | EscapeMenu | B39 root-modal exception window ✓ (0x45ec5e root call) |
| +0x20 | LoadingBar | |
| +0x28 | Container | |
| +0x38 | (16th build; parse interleaving — likely the 0x15c-size window between LoadMap and Actionbar; LOW confidence) | |
| +0x44 | PipBoy | B39 slot B (companion reopen ✓) |
| +0x48 | MiniMap | |
| +0x4C | Options | B39 slot C = cls_0x442bf0 ✓ |
| +0x50 | OptionsLoad | |
| +0x54 | OptionsGame | |
| +0x58 | OptionsHotKeys | |
| +0x5C | OptionsMultiplayer | |
| +0x60 | OptionsVideo | |

CAVEAT (kept, not smoothed): my parse pairs Actionbar→+0x10 and Feedback→+0x0C from ctor store order, but BOTH
commands act on +0x10, so the ctor name↔slot interleaving at the 0x0C/0x10 pair is unresolved from the static
dump alone (the two windows may sit the other way round, or +0x10 hosts a composite). Consumer evidence (two
commands + tick on +0x10) is the load-bearing fact; the pair is flagged MED until one ctor trace confirms.

## 3. The 11 previously un-sampled handlers — all classified

| command EA | handler | classification |
|---|---|---|
| 0x45edb0 GameOptions | `mov [esp+4],1; jmp 0x45ed10` | **thunk to shared panel handler**; selector 1 |
| 0x45edc0 HotKeyOptions | selector 2 → 0x45ed10 | same |
| 0x45edd0 LoadGameOptions | selector 3 | same |
| 0x45ede0 MultiplayerOptions | selector 4 | same |
| 0x45edf0 VideoOptions | selector 5 | same |
| 0x45ed10 shared | slot C protocol: IsActive(blk+0x4C) → SetActive(0)+`0x4433e0(arg1, panel)`+reopen PipBoy(+0x44,1)+reopen Inventory(+0x18,1); else SetActive(+0x4C,1) | **THE OPTIONS WINDOW FAMILY IS ONE WINDOW + PANEL INDEX** — `0x4433e0(iface,panel)` = panel select; the five names' windows at +0x50..0x60 exist but the COMMAND path toggles the single Options(+0x4C) with a panel arg. |
| 0x45ee00 FeedbackWindow | IsActive(blk+0x10) else tail `jmp 0x4172e0` | actionbar-surface opencmd (see §2 caveat) |
| 0x45ee30 UseSkill | `0x6165e0(args)` → `[+0xA4]` → `0x436ad0(x,0x10)` | **NOT a blk window** — dialog interface `client+0xA4` (the vft+0xB4 factory object) + skill-menu op 0x10 |
| 0x45ee60 AttackModeCycle | reads blk[+0x10] + `[client+0xE0]`, pushes text params 0x6f1e78/0x6f1f24 | **NOT a toggle** — posts to actionbar/message surface (weapon-mode cycle feedback) |
| 0x45eed0 WeaponToggle | (same family, 0x92 bytes) | action command, not window slot |
| 0x45ea50 ActionbarWindow | toggle + rebind entity `0x41c910` | actionbar show/hide |

⇒ Command→slot mapping COMPLETE for the F3 surface: 6 window-slot handlers (Inventory/PipBoy→slot A/B, Options
shared-panel→slot C, EscapeMenu→slot A'+root, ActionbarWindow→+0x10), 2 actionbar-surface commands
(FeedbackWindow, AttackModeCycle), 2 action commands (UseSkill via dialog iface, WeaponToggle), plus the 4
previously-sampled (B39). Engine-tier `StartMap/CreateEntity/Warp` remain deliberately out (spawn lanes).

## 4. §4 residuals status

- two-globals ambiguity: already resolved same-day in B39 §4 (GetWorldServerInstance read-path) — nothing to add;
  confirmed AGAIN by every handler dumped here using `mov eax,ds:0x70bd90; [eax+0x2c]`.
- handler-size honesty: the five Options thunks are **genuinely 8-byte mov+jmp thunks** — these are the stub-adjacent
  class that is REAL code (B38 stub census unchanged); the 0x45ed10 body (0xa0) is the substantive handler.

## 5. Port consequences

1. Port needs a `blk` object: 24-slot table (0x64 bytes incl. vftptr) built by the client UI bootstrap on the
   FIRST call to client vft slot 1 (lazy), NOT in the ctor — a ctor-time install would change observable order.
2. Options family = ONE window + panel selector arg — five separate command names, one toggle + `0x4433e0(panel)`.
3. UseSkill goes through the `client+0xA4` dialog interface with op 0x10 — not through any blk slot.
4. Actionbar(+0x10) hosts the feedback surface: two command names, one object.

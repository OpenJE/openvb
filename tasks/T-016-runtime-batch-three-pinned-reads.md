---
status: done
priority: P3 (unblocks two naming/correctness questions at once, blocks nothing today)
assignee: whoever holds the debug window next — UltraBear if they boot one during T-012
type: runtime-observation batch
---

# T-016 — One debug session, three pinned reads. The team has a runtime debt.

## Why this exists as a row instead of three messages

Every open question so far has been settled statically, and that has been excellent. But three
decisions now sit **past the static limit**, each requested independently by a different lane, and
each needs the same thing: one attached debugger, one breakpoint, one dump. If they keep queueing
behind "someone will get to it", the corpus accumulates hypotheses marked *one evidence short* —
which is exactly the state 10c is in now. So: take them together, in one session, and close three.

`f3-lockstep-debug` exists for this (original GDB stub on `$OPENVB_ORIG_GDB_PORT`, reimpl on
`$OPENVB_REIMPL_GDB_PORT`); nobody on the team has used it yet, so budget discovery time for the
harness itself. Run against the **original** — every question below is about shipped behaviour.

## Read 1 — OPTIONAL as of this turn, do not schedule it first

Status changed after this row was written: IronStorm retracted the socket bridge and the naming question
collapsed to one static datum (which equip slot receives helmets), now owned by FastArrow's
`GameActionEquip` pass. Run this read only if that pass is inconclusive.

### Original content

Does the called-shot part id index the socket-name list? (closes T-010c / 10c)

The `{5,6}` group is proven to be armor **slot-class** partition (`GameCreature::meth_0x40A670`),
while IronStorm's naming rests on an unproven `socketIdx = id + 1` bridge to the ART-SOCKET name
table at `0x6FF650`. FastArrow's cheaper version: **equip a helmet-class item and diff
`creature+0x7C8..+0x7DC`** — the moving values are class-1 coverage.

Pick either, both settle it:
- **Log-diff (FastArrow's falsification test):** make a called-shot selection that should be SHINS
  and check whether the original emits
  `"GameWeapon::CalculateDamage() - Unknown CALLEDTARGET encountered."` while still computing damage.
  Log fires ⇒ 8 is out of the switch's case set ⇒ the socket-order mapping is wrong or reordered.
- **Equip-diff:** snapshot `+0x7C8..+0x7DC` before and after equipping a helmet; if those six dwords
  move, row B is head coverage, and `{5,6}` cannot be FEET/WAIST.

Report which hypothesis dies. Until then `dword_64F858` stays numerically indexed in the reimpl and
**no lane names parts in a comment as if settled**.

## Read 2 — which vtable actually lands at `World+0x160`? (IronStorm, session34's last 4%)

Static semantics are already pinned (`0x545820` = `World::SetFocusEntity`; `+0x158` = 2000-entry
object-id pool; descriptor blocks at `0x691A8C`/`0x691A94`). Unknown: the ctor seeds one vft at
`World+0x160` and engine-attach may overwrite it.

Recipe as routed: break @**0x5458AF**, dump `*(World+0x160)` and its `[+0x54]` / `[+0x28]` entries.
One read, no stepping required.

## Read 3 — confirm the `[Mainmenu] Map Name` load preamble ordering (unblocks the warp/save lane)

### CLOSED STATICALLY by RedXenon (01a157ec), zero debugger — see `discoveries/2025-10-20-t016-mainmenu-mapname-shipped-ini-answer.md`

The premise that forced this into a runtime read was mine and it was an `ls` artifact: the demo's `globals.ini` ships
as **plaintext inside `data/Engine.grp`**. Verified independently by the coordinator (`strings data/Engine.grp`):
`[Mainmenu] map name = mainmenu.map`, `Seconds Per Round = 6`, `Year = 2253`. Recorded as harness item 24.

Chain from one decompile of `0x546fd0`: seed `"none.map"` → `tree[Mainmenu][Map Name]` override (`_stricmp` comparator,
so the shipped lowercase key matches) → **existence-gate, then request; if the gate fails there is NO request —
`none.map` is not a safety net.** Separately, `[Server] Start map = 00_03_Tutorial_Junktown.map` is the **new-game door**,
not a boot fallback, and the reimplementation must keep those two requests distinct. Harness item 13 amended, row
closed, no session spent.


**Re-scoped 2026-10-21 per harness item 13 (RedXenon's objection): try the payload INIs before scheduling a read.**
This row was written when debugger-first was the habit. The shipped INIs are in the demo payload
(`OPENVB_F3DEMO_ROOT`), so the `none.map` fallback order may resolve statically — and if it stays ambiguous the debugger
work shrinks to **one** question, not three. Sequence note if a read is still needed: it goes **behind T-034**, because
the `Entity+0x304` depth read can convert deviation D-2 from stylistic to a dropped-order bug, and that outranks a
fallback-chain curiosity.

`0x546FD0` (LOAD-MAP) statically reads: `ChangeState(3)`, map name from VFX_Parameter ini
`[Mainmenu] Map Name` with a `none.map` fallback, behind a ResourceSystem gate. This independently
corroborates YoungStorm's `<gv_NAME>` finding from a second direction. Confirm live, in one step,
that a *missing* key really takes the `none.map` fallback and in what order the state change lands
relative to the resource gate — our `SpawnMapEntities`/`MapCreate` sequence copies this shape and
would rather find out now than during a save/warp task.

## Acceptance

- One commit (any lane) adding a `discoveries/` file or a section to session34 containing: the three
  observations, each marked **confirms** or **falsifies**, with the exact register/memory dumps.
- For each: the affected board row updated (10c, T-015's neighbour, or a warp lane), so the
  hypothesis doesn't outlive its verdict.
- If any read is inconclusive in the session, say so in the file and leave the row open. An honest
  "the breakpoint never hit because we never reached that path in the demo" is a result — it means
  the path is dead in shipped content and should be recorded as such.

## References

- `discoveries/2025-10-20-t010c-naming-addendum-armor-rows.md` (FastArrow) and the socket-table doc
  (IronStorm) — **please converge in one file**, per the conflict note on T-010.
- `discoveries/…session34…` (host chain), `…session33…` (SetFocusEntity / packet 63)
- `.pi/skills/openvb-ida-mcp/SKILL.md`, `devenv/scripts/f3-lockstep-debug.nix`

## Notes (coordinator)

Static reasoning has carried this project remarkably far and I want to keep it that way — but three
lanes independently hitting "needs one runtime read" is the signal that the read is now the cheap
option. Anyone who runs it: keep it to the three dumps above; a debug session that turns into a deep
dive is how we lose a clean gate for an afternoon.

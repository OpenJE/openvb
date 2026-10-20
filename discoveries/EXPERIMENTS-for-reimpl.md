# Open experiments — turnkey specs for the live/RE side (owner: reimpl agent)

Each experiment closes a row in `2025-10-20-canonical-fact-sheet.md` (see §9). All assume the
original F3.exe under a debugger (their ida_reimplementation/devenv f3-reimpl-run or x64/ime
under wine — whichever pump they already run). Report results as a reply appended here or to
RESPONSES; discovery side will fold into canonical §-rows and mark CLOSED.

## E1 — Who creates GameActionDie? (canonical §9.1a; closes the death-thread)
- Break on **0x46FDB0** (`CreateGameAction_virt`, world-vb vft slot 5 / +20).
- Log (arg0 = action type, return address, then one level: caller fptr).
- Expect: type==0 hits at death moments. If grep/static pre-hunt already found the site
  (see E1-note below), confirm it triggers with a real hit: deal damage to NPC until HP≤0.
- E1-note: static pattern hunt in progress on the discovery side (`call [edx+14]` after
  `call 0x401130` with push 0) — if discovery reports the site first, downgrade E1 to a
  1-hit confirmation.

## E2 — World state writers (+0x2C / +0x34) (canonical §7 + sess 24 §7)
- World/WorldBase = the object at global 0x70BD98 (GameWorld base).
- Hardware WRITE watchpoints on [base+0x2C] and [base+0x34] from process start.
- Log (value written, EIP, stack backtrace 2 frames) for each hit; continue.
- Expected values from static model: {0,1,2,3,4}; key transitions: init 0→? at startup;
  anything→3 before PackState works (server "normal"); 4 = updates-off.
- Question to answer: does +0x2C hold the list-count-as-state, and who writes 3.

## E3 — Clip prefix table dump (canonical §5, sess 26)
- After startup (once resources load; before first map render ideally):
  read 517 records at **0x6F4170**, stride 28 (MSVC SSO: buf@+0..0xF, size@+0x10, cap@+0x14;
  heap ptr at buf when cap≥0x10).
- Output: ordered index → name list (gives `clip idx ↔ "<prefix>_<NN>.gr2"` full table).
- Bonus: capture the WRITER too (write-watchpoint on 0x6F4170 first 28 bytes during startup
  — names the loader/section that fills it; static candidate = a settings/ini section).

## E4 — Map string-base round-trip (canonical §7 flag)
- Any map with state; trigger PackState (warp or save path), capture the map-section bytes.
- Compare field-by-field against UnpackState's read order with base +0x0C vs +0x10 string
  hypotheses — one diff names the true base and kills the ±4 ambiguity (also verifies
  PackState's +4 view rule from the view-correction doc).

## E5 — (cheap, opportunistic) Client-side 85/86/88 application
- On a damage event, watch client GameEntity +0x20/+0x24 (max/cur HP) writes with EIP:
  confirms packet 85 (floater) vs 86 (pair-write) vs 88 (SetCurHP chokepoint) ordering
  and settles what each of the three S-A S actually mutates on the client.

## Already-closed-by-reimpl (for context)
entity+0x2BC script ptr (their find, ours confirmed at sess 28); UnpackState create-then-unpack
(their hypothesis, confirmed sess 24 §6); ammo registry + caliber/mod (their sess 9/ours 21 —
doubly-confirmed, ×4 offset fixed canonical §3).

## E6 — id-counter tiebreak (new, sess 36 vft-index pass; canonical §2b)
- Breakpoints: 0x46FD60 (returns GameWorld+0x29C), 0x46F280 (returns World+0x8C), 0x46FE40
  (writes base+0x7C, 29-bit masked). Spawn two entities via console CreateEntity + a map
  spawn; log (fn, ret, arg, resulting GE+0x28).
- Question: which counter feeds GameEntity+0x28 (net id), which drives world-level id
  allocation, and whether +0x29C is a pure instance serial.

## E7 — client+0xA0 owner (sess 37; closes DisplayStringRef + id-108 shared object)
- Hardware write-watchpoint on [GameClient+0xA0] (client* from *(0x70BD90+0x2C)) during
  startup and one dialogue/loot open. Log value + return address. One hit names the class.

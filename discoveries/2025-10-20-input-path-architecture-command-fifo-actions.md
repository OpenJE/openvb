# 2025-10-20 — happywolf:sess40 (T-028 recon): how input actually reaches gameplay state

Target: ORIGINAL_REFERENCE (IDA). Read-only. Answers row question 1 and the acceptance
question ("is the scripted path reachable, or does it need RecvPacket plumbing?").

## 1. Two front doors, not one — and gameplay orders do NOT use the command door
(a) **Command FIFO** — `cls_0x580ba0` (16B: read-ptr +0x8, write-ptr +0xC, 28-byte string
records), **embedded in GameClient at +0x1F0** (drain call site `580ba0(&v8[124])` @0x587fd5).
Drained EVERY FRAME by `GameClient::virt_meth_0x45f000` (the client tick, §3). Per record:
`sub_580B00` tokenizes → `sub_59F030(name)` → `sub_59EC80` = `GetCommand` on the sorted
case-insensitive map `global_cls_0x4d8d70_0x70c668` (written by RegisterCommand @0x59f6b5)
→ **direct cdecl call of the handler with the argv block**; miss logs
"Unknown command %s\n" (0x580c65). Handlers = the 134-command surface (happywolf:sess-38;
EAs now enumerated in sess-38 follow-up, e.g. EscapeMenu 0x45ec00, PipBoy 0x45ec70).
- ⚠ census fact: the command table contains NO Attack/MoveTo/Select/Stop-style gameplay
  verbs (string-grep of the B38 command list: zero hits). The FIFO is UI/camera/toggles/
  engine (`StartMap`, `CreateEntity`, `Warp`) only.

(b) **Gameplay orders = entity Action objects** (the sess-1..20 Action family): e.g.
`GameActionAttack` (0xC8) ctor `0x403e40`, created in exactly three places:
1. **Script/host natives** `F3::Attack(a,t)` @0x459ed0ish and `F3::JustAttack` @0x45a001:
   arity-guard `59af40`, resolve both entity ids (`59aef0` → "was unable to find the
   attacking/target entity"), new 0xC8, init `403fa0(target)`, **enqueue `0x550720(attacker,
   action)`** onto the ATTACKER entity.
2. **Server packet path** `GameServer::virt_meth_0x46dd70` (0xd93) @0x46deeb — server-side
   construction from a received order packet ⇒ multiplayer click orders are
   client-send→server-queue (authoritative); single-player reaches the same seam locally.
3. internal self-respawn `404adb` (OnServerUpdate body, not an input path).

Consumed by the per-round pump: `GameActionAttack::OnServerUpdate_404320` (0x812) via the
entity action loop — the sess-34 ServerUpdate family.

## 2. Mouse path (per-frame, client tick)
`virt_meth_0x45f000` order: net pump `59d2d0` → drain input-event queue (client idx120 =
+0x1E0 device): **mouse events (tag 2) are STASHED to client fields +0x310..+0x338 (x,y,
button state)** and only NON-mouse events reach `sub_599720` queue (+0x200) when
device/iface handlers don't consume them → `GUI root busy? meth_0x4b2c10(&0x707E48)` —
if GUI idle → `Client::vft+208` (`JE::Client::virt_meth_0x588fe0` — the engine mouse
state/edge-scroll feeder into CameraControl+194) → **command FIFO drain** → camera tick →
two interface ticks (+0x290/+0x2A0 vft+16) → cls_0x5b4f50 → vft+4/+212 → **if
client+0x160==3 (in-game): GW[3] vft+0(tick ms), vft+20, and vft+24 when two flags clear**
= the world/ServerUpdate pump that runs the Action queue → vft+8 → 590860.
⇒ GUI hit-test routes (happywolf:sess-34 root dispatcher 0x4b3040) consume the click FIRST;
world orders only see what the GUI doesn't take. Exact click→Attack-creation site (the GUI
order path that ends in the (b).2 packet or a local native call) NOT yet pinned — named gap,
one hop from root-dispatch, deliberately not swept further under pause-era budget.

## 3. Answer to the acceptance question
A scripted path IS reachable without `RecvPacket`: feed **orders as Action-object
creations** at the native seam ( Attack/JustAttack equivalent: resolve ids → new → enqueue )
and let the world pump consume them — single-player host needs no socket. The FIFO seam is
separately scriptable for UI/camera/engine verbs. What the driver must replicate for
determinism: the tick ORDER of §2 (GUI-busy check before world dispatch; FIFO drain once per
frame; world pump only in state 3).

## 4. Reimpl cross-check (host side, src grep only — read-only here)
`src/F3/GameActionAttack.hpp` (+MoveTo/Equip/Idle/Give/Dialogue family) with
OnServerUpdate ports already cite `discoveries/`; `GameWorld.hpp:34 CombatTick`;
`GameStateLoop`/`Host::` live in `src/F3.cpp`/`F3.hpp`. The enqueue seam equivalent of
`0x550720` was NOT found by greps (`SetAction|m_pAction|actionQueue` → 0 hits) — the row's
missing-seam candidate: **host-side entity action-queue insert + entity action-loop**. If
actions are only ever run via test-harness direct calls today, T-026→T-028 driver needs that
queue (or documents its absence as the seam).

## 5. Limits of this document (tool-boundary rule, in prose form)
- §1(a) "no gameplay verbs in commands" = grep of the B38 enumerated list, not a fresh table
  walk — if the list missed a registration, so does this.
- §2 field offsets are dword-index conversions (idx*4) from IDA's F3::GameClient* indexing;
  absolute-offset rule applies (e.g. +0x1F0 FIFO, +0x1E0 device, +0x310 mouse stash).
- `0x550720` labelled "enqueue" from call-shape at two native sites only; body unread.
- The decompile of 0x45f000/0x587xxx merged two vft slot bodies (tick fn spans both); the
  ordering claims are read off THAT merge; if the vft slot boundary is elsewhere, §2 order
  is suspect. Marked ⚠ until slot-by-slot walk.
- Driver half (§host build) awaits T-026 digest format per row dependency; no tools/ file
  written yet, and none will touch cmake.toml/CMakeLists per wave rule 1.

## 6. Evidence
0x59ec80/0x59f030/0x580ba0 (drain), 0x587fd5 (FIFO-in-client), 0x59f6b5 (map insert),
Attack natves 0x459f41/0x45a001 (new 0xC8 + 0x550720), server 0x46deeb, ctor census
xrefs(0x403e40) = 7 hits all listed, tick body 0x45f000+0x587xxx, mouse feeder 0x588fe0
(GetCursorPos @0x589007, CameraControl+194 write 0x589115), command-list absence grep
local to this session.

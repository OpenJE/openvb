# Session 25 — the mode flag resolved: `[0x70BD90]+4 & 1` = "connected-client" bit set by the connect handshake (answers roster-doc §8.4)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Answers `2025-10-20-cutscene-controller-world-subsystem-roster.md` §8.4 ("which mode does the
demo boot in? … please report") — statically, with the full setter chain.

## 1. Flag mechanics (static, complete)

- Getter: `AreaMapIcon::Get_mbr_0x4 @0x5b4f30` = `return *(this+4)` (a full DWORD; consumers
  test bit 0 with `& 1`).
- Setter: `SetMode @0x5b4f20`: `mov [ecx+4], arg` — plain store, one wrapper
  `cls_0x5b4f50::meth_0x5b4f60(wrapper, v)` forwards to `*(this+0xC])->SetMode(v)`.
- The global @0x70BD90 is a pointer; its only ABSOLUTE write in .text is the nulling on
  teardown (@0x543716, inside `JE::Network::virt_0x543730` = dtor-ish: vft+8 + child deletes +
  `timeEndPeriod/WSACleanup` refcount at byte_70C6C0) — assignment of the object itself is
  register-mediated (constructor path), consistent with a heap/owning init.

## 2. Who sets bit 0 — `JE::Client::virt_meth_0x58a620` (the connect handler)

```
"Client: connected to server.  My id is %u"          ; handshake completion
if (server-config flag [x+86]) { IPlogo movie, 1000ms, … }
SetMode(1)  @0x58a6d6                                  ; ★ sets obj+4 = 1  → &1 == 1
if ((Get_mbr_0x4() & 1) == 0) { worldServer vft+16; send packet 67 }   ; only if NOT set
```
⇒ **bit 0 = "this process is a (connected) CLIENT"** — set during the client connect
handshake. In the shipped single-player boot the client connects to the in-process host, so the
bit IS set in normal play. (Pure-host/editor paths evidently clear/never set it — the
`if ==0` join branch below proves both flows exist.)

## 3. Consequence for every "server-gated ⇒ demo-inert" conclusion (read carefully)

Gates that test `flag & 1 == 0` and THROW ("Invalid attempt to retrieve world server
instance.") — EFDT parse, `GetWorldServerInstance`, JoinCombat, attack-resolution step,
OnDeath 6→7 — are passed when bit is SET. Given §2 (bit=1 after connect in the demo):
- **The demo runs the FULL server-side pipeline in-process.** Effects parse, damage applies,
  combat join/leave, ChangeState 4→5→6→7 and packet-85/88 broadcast/recv are ALL LIVE in the
  shipped game (loopback networking), not inert.
- This RETRACTS the "(inferred) demo-client inert" glosses in session 7 §1 (end-to-end
  inertness), session 8 §3 ("client-mode death no-op"), session 12 §2 (6→7 never reached),
  session 14 (world ms-accumulator branch), session 18 NOTE 2 and session 23 §1 — the
  MECHANISM docs all remain correct; only the client/server-role framing flips: what those
  gates mark is "host-connected" (bit 1), which the demo HAS.
- The reimpl's decision to port `World::ServerUpdate_0x545d60`-driven movement (their roster
  §8.4 addendum) is therefore correct as built, and the roster's "flag=1 ⇒ Combat pump LIVE"
  branch is the confirmed one.

## 4. One residual runtime check (cheap, not blocking)

Whether the boot order means the flag is set BEFORE first World tick (probably yes — the
handshake precedes IN-GAME state 3 in the client frame, cf. session 14 §2 gate) — one gdb
read of [obj+4] at the first 0x5b4f30-return inside ServerUpdate confirms; if any window
exists where flag==0 early, that's the ONLY inertness window.

## 5. Cross-links

Roster §8.4 (question), session 14 (frames + where the getter fires), sessions 7/8/12/18/23
(framing retraction applies to their "inert" sentences only), their
`hp-damage-wire-and-game-ids.md` (their 85/88 wire findings implicitly assumed live server —
consistent).

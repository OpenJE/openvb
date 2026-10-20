# Session 31 — session 27/28/30 helper addenda: sub_41D050 = IWorld+12 getter, cutscene-Stop = UI-wide close, packet-13 self-check, and packets 58/63 end with client SEND of packet 64

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Names the four anonymous helpers left in sessions 27/28/30; small, completes that arc.

## 1. `sub_41D050` = **IWorld client-interface getter** — `return global_cls_0x616328_0x70bd98 + 3` (+12 bytes)
The session-14 §1 table applies: iface@+12 vft 0x668548 →
`vft+0` Tick(0x544b30), `vft+4` 0x546140, `vft+8` 0x545820 (used by engine packets 58/63),
`vft+0x14` VFX-frame(0x548840), `vft+0x18` render-batch(0x545aa0). So "sub_41D050()->vft+8(id)"
(session 30 map) = **`IWorld::0x545820(id)`** (session 14 flagged it as a state-3
unpack-adjacent body — recheck there if the spawn slice depends on it).

## 2. `sub_45E6C0` = packet-78 (cutscene STOP) client receiver = **UI-wide panel close/refresh**
Snapshots 11 interface children of `[AreaMapIcon global]+0x2C` (slots dwords 3..19:
load-map/container/dialogue interfaces + `cls_0x442bf0` child), for each non-null calls
`CLoadMapInterface::meth_0x495220(iface, 1)` (the same refresh/close-mode-1 helper packet 77's
handler used), plus container handler `meth_0x430760`. ⇒ cutscene end = "close every open panel";
matches the `+0x108=1` HUD-enable from the 77 handler (session 28 table).

## 3. `JE::Client::meth_0x588200` (packet 13 handler)
`if (id != 0 && id == client+0xCC-field) → meth_0x5881a0` — **conditional self-check**: packet 13
carries an entity id; only if it equals the client's stored "current/self" id (`gap28[176]` display = client+0xCC
*(inferred same slot as packet-12's self-follow comparison, whose decomp shows `gap28[144]`
= +0xB4 for the ENTITY-side value — display units again; pin with one debugger read if the
spawn slice depends on it)*); then the client reacts via `meth_0x5881a0` (camera/focus
re-center). Not a general look-at — self-only.

## 4. `JE::Client::meth_0x589d90(id)` = client **SENDS packet 64** {id}
Standard net::packet builder (`mbr_0x30 = 64`, one u32, `meth_0x59c4f0(...,1)` = send).
Called at the tail of BOTH engine-packet handlers 58 (MAP-CHANGE) and 63 (SELF-assign)
(session 30) ⇒ the client's completion ack: after adopting a map/PC assignment it sends
**64**{id} back (server presumably keys initial-state push off it). Add id 64 to the
client-outbound table (their gameclient-outbound doc's numbering space 61–66 covers engine acks).

## 5. Net
Spawn/transition round-trip, with sequencing honesty: the HANDLERS and the 64-acks are proven;
the ORDER across 9/12/58/63 comes from demo behaviour assumptions (join-before-place is safe,
map-change-vs-self ordering is *(inferred)* — treat as a test plan, not a wire fact).
Sessions 27/28/30 unchanged otherwise.

# Session 27 — roster §8.1 CLOSED: the 0x553470/0x553550 pair = StartCutscene/StopCutscene server broadcasts (packets 77/78)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes `2025-10-20-cutscene-controller-world-subsystem-roster.md` §8 open item 1
("0x553470/0x553550 GameServer cutscene broadcast pair — open").

## 1. Chains (all decompiled this session)

**Start:** AMX native `JE::StartCutscene @0x5c95xx`
```
if !sub_59AF40(amx, params, 1, "StartCutscene") return 0        ; arity/type check (their §8 pattern)
sub_5CCCC0(amx, tag=1179407181, &out)  else "unable to resolve user ID"
map = sub_59ACD0(out)                  else "unable to resolve the map."
GameServer::StartCutsceneBroadcast_0x556080(sceneId = params[1], map):
    worldVB vft[+160](idx40)(sceneId, map)     ; LOCAL controller start (vb interface,
                                                ;  base = global+2 + *(global[2]+4) — the
                                                ;  0x668494-table family)
    GameServer::meth_0x553470 → packet 77 = {u32 sceneId, u32 mapRef}; broadcast (send-all, flag 1)
```
**Stop:** AMX native `JE::StopCutscene @0x5c95b0`
```
sub_5560c0: worldVB vft[+164](idx41)()        ; LOCAL controller stop
GameServer::meth_0x553550 → packet 78, payload-free broadcast
```
Error strings (parity): "StartCutscene() was unable to resolve user ID. (internal error)" /
"StartCutscene() was unable to resolve the map."

## 2. Notes

- Packet numbers add to the reimpl's GAME_ID map: **77 = ENTITY? no — CUTSCENE_START{sceneId,
  map}, 78 = CUTSCENE_STOP{}** (name them so in their enum; senders identified by log-free
  template match to the 85/88 builder idiom: `packet::ctor → mbr_0x30 = id → u32 pushes →
  meth_0x5a0a00(...,1)`).
- The `vft[+160]/[+164]` pair is the same world-vb interface family whose slots the roster
  doc mapped for the controller object — this nails the server-side entry points INTO it.
- `sub_59ACD0` = "map for the invoking NPC/area" resolver (shared with other cutscene-ish
  natives per their native-table doc); tag 1179407181/1179407188 pair = AMX user-reference
  tags (their `sub_59AEF0 entity-ref` pattern extended — report to their scripting thread).
- Client side: receivers for 77/78 should be in `GameClient::recv_packet_461a10`'s switch —
  one grep from a done deal for the reimpl (their action-factory-client-recv doc).

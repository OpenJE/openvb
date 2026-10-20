# Session 30 — ENGINE_ID client layer decoded (Entity::Client::virt_0x58c380): ADDTOMAP=12, MAP-CHANGE=58, SELF=63, no-op set — plus ORDERING FIX to session 28's gate note

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Third wire layer (after sessions 27/28) and one self-correction.

## 1. Correct three-layer ordering (fixes session 28 §"HostConsumeGate" wording)

`GameClient::recv_packet_461a10` pipeline, precisely:
```
1) HostConsumeGate_0x45e9a0 (vft+0xCC):
     flag &1 == 0 (pure/remote client) → returns 0 IMMEDIATELY — no consumption;
     flag &1 == 1 (host)               → consume-set {84, 86..104} → HUD refresh + return 1;
                                          ALL other ids → delegate to engine layer (below);
2) JE::Client::virt_0x58c380 (engine ENGINE_ID consumer; body itself re-gates on flag==1):
     handles/consumes the ENGINE_ID set below; default → return 0;
3) F3 switch (session 28 table): runs whenever layers 1–2 didn't consume —
     in PURE-CLIENT mode it is the ONLY layer active (gate no-ops) ⇒ the session-28 receiver
     table IS the complete remote-client apply-path (my session-28 addendum sentence
     "if clear → delegates to engine" was wrong — corrected: flag clear ⇒ gate returns 0 and
     the switch sees everything; the engine consumer likewise no-ops when flag is clear).
```
⇒ host loopback: gate > engine > switch; remote client: switch only (plus entity-level
pre-consumption? no — the vft+0xCC IS the gate; nothing else precedes). HUD-refresh call
`(*(*this+120))` = vft idx30 appears in all three layers — it's "invalidate/refresh UI state".

## 2. Engine-layer map (`0x58c380`, host mode)

| packet | handling |
|---|---|
| **12 (0xC)** | **ENGINE_ID_ENTITY_ADDTOMAP**: {id, byte flag, float x, y, z} → `meth_0x54be10(id)`; found → `Entity::meth_0x54a8a0(posTriple)` (the placement/attach helper seen in Create tail) then IF `flag != 0 || (entity+0x1E4 == clientSelfId && distance? v19==selfSlot2@+B4)` → `client vft[+220](id,1,1)` (camera follow). Miss → `"Error handling Network::ENGINE_ID_ENTITY_ADDTOMAP (Client::FilterPacket)\n\tEntity not found.\n"` ⇒ **this is the spawn/teleport wire** — implementer-A's EME2→spawn pipeline should key off id 12 semantics: entity must already exist (created via 9-join path), this places it. |
| 13 (0xD) | {id} → `JE::Client::meth_0x588200(id)` (focus/look-at, likely) |
| 34, 38, 39, 49, 50, 52 | consume + HUD refresh only (no payload read) — engine acks/pings |
| **58 (0x3A)** | **MAP-CHANGE / world transition**: string param → camera `meth_0x586590(1)`+`0x586830` (reset/freeze), HUD object resets (72/116/120 children), flag `+0xE4=1`, `sub_41D050()->vft+4(name)`, then `meth_0x589d90(world map-id from +92)` |
| **63 (0x3F)** | **SELF-assign / spawn-complete**: {id} → `sub_41D050()->vft+8(id)` (set PC on world), camera stop-follow (116 vft+16(0)), `meth_0x589d90(id)` |
| 9,10,14,16–25,27–33,61,62,65,66,71,73–78 | consume, NO action (return 1) — host loopback suppression of its own broadcasts' echo (incl. 67 join-ack, 73 dialogue-start… see their senders docs) |
| default | return 0 → falls through to F3 switch |

## 3. Notes / cross-links

- 77/78 in the engine NO-OP set ⇒ host loopback discards cutscene echoes even if the gate
  missed them (defense in depth).
- The engine-layer set overlaps the roster doc's server-side ENGINE_* sender table by id
  arithmetic, not by claim — when A wires their packet enum, ids 9-33 & 61-78 belong to the
  ENGINE prefix, 41/42/46/47/50/55/56/57/67/68/69/70/72/79-96/97-109 to GAME_/F3 layers.
- Sessions 27/28 unchanged otherwise; session 28's ordering note gets a pointer here.
- A: for the spawn slice — 12 is the placement wire; 63 the PC assign; 58 the map change;
  `sub_41D050` = the client's "current world/game" accessor (three hits here; one hop from
  its vtable to the full client world model if needed).

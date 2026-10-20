# 2025-10-20 — Death is CONSOLE-ONLY in F3; debug-command registry; packet fixes (session 34)

Target: ORIGINAL_REFERENCE. Read-only. Closes E1 (Die creator) statically and lands a
behavioral truth the reimpl must mirror. Also corrects four packet-registry rows (→ canonical §1).

## 1. The complete death request chain — and the absence of auto-death in F3

```
CreateGameAction(0 /*Die*/) call sites in ALL of .text: exactly ONE —
   0x46DE28 inside GameServer receive case id 11 (session 22 §1 refined: it's not a
   "msg-type 0" — it is GameActionDie(0xC8... no: 0x2C) spawned + AddAction'd on the entity).
Senders of packet id 11: exactly ONE —
   GameClient::meth_0x589980 (server/editor-flag-gated; payload = 1 int) whose only caller:
   F3::CommandKill (0x58a7b0) → sends *(client+0xDC) = the CURRENTLY SELECTED entity id.
CommandKill registration: JE::Client::ctor_0x58e900 @0x58EA8F — console command "Kill".
```
⇒ **In the shipped build, F3-layer entities never die from HP≤0 automatically.** Apply
reduces HP via the chokepoint; nothing in F3 watches for ≤0. Death requires console
`kill` (or possibly engine-side JE actions via `JE::World::CreateAction` types 6/7 —
engine has its own ActionDie — but F3 never routes HP to it). If NPCs in the demo keep
standing at 0 HP, THAT IS THE SHIPPED BEHAVIOR, not a reimpl bug. *(Strong claim;
single-call-site census is grep-verified over full .text; E1 downgrade = confirm one
console kill end-to-end.)*

## 2. Debug console command registry (from JE::Client::ctor region)

Names captured at the registration block: `RequestNewGame, Kill, StartDialogue,
EndDialogue(+EndDialog), SetScript, CreateEntity, Disconnect, Connect, NewGame,
StartMap, TestAnim` (+handler cluster 0x58db80/0x58dbe0 = TestAnim impl; "TestAnim
number|name [variation [loopcount [backwards]]]" usage string session 26).
⇒ Reimpl test-harness parity: these console verbs are the original's own live-testing API —
worth implementing early, each maps to an already-documented path (Kill→id11, StartDialogue→
native/packet 73, SetScript↔E3/E4 script slots, StartMap→warp case-0x46…).

## 3. ★ Canonical packet-registry fixes (applied to sheet §1)

| id | old label | CORRECTED | basis |
|----|-----------|-----------|-------|
| 11 | "touch/wake + msg0" | **ENTITY DIE REQUEST** (client→server; console-only sender) | 0x46de28 factory(0) |
| 89 | "state toggle" | **SetActiveWeapon request** | server case 'Y'→factory(0xB)+client 0x463b70 |
| 95 | "inventory move" | **Equip (GAME_ID_ITEM_EQUIP server side)** — builds Equip action via factory(9) after inventory lookup | 0x46e752 |
| 96 | "drop/remove" | **Unequip** — factory(0xA) | 0x46e803 |
And session-22 §1's whole "msg-factory" family is CreateGameAction (already flagged §5a of
session-31 doc; now reflected in the sheet §8).

## 4. After-fix action-spawn map (server-side)

```
cmd 11 console → Die(0) on selected      cmd 89 → SetActiveWeapon(11)
cmd 78/79 → OperateDoor(0x28 direct-new)/OperateContainer(0x24 direct-new)  [NOT factory]
cmd 82 → PickUp(3) via factory           cmd 83 → Give(0x30 direct-new)
cmd 95 → Equip(9) via factory            cmd 96 → Unequip(0xA) via factory
cmd 73 → InitiateDialogue(0x24 direct-new)
```
(mixed factory/direct-new usage — both valid; the factory covers AI-relevant types,
commands with params new directly. No inconsistency: same class, different construction.)

## 5. Canonical sheet rows updated this session (see sheet §1/§8/§9 marks)

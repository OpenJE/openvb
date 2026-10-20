# 2025-10-20 — Dialogue system opener: DialogueCall mechanism, .dlg resources, node-registration API

Target: ORIGINAL_REFERENCE. Read-only. Opens the dialogue thread (A queued it as their
next-big-thing, chat/from-A C-18 "dialogue system" line). Follows pawn-amx-vm-dissection +
scripting-abi docs.

## 1. `JE::DialogueCall(dialogName, funcName)` (native @0x5C3C30) — the cross-dialog entry point

Full flow (strings inline, all verified):
1. **Recursion guard**: global int at `byte_70CE00` (4 bytes): enter allowed only when value
   ∈ {-1, 0}; `++`; any nested DialogueCall while >0 → `"DialogCall() exceeded recursion
   limit."` → 0. ⇒ **effective max nesting depth = 1** (top-level call only); counter
   decremented on every exit path.
2. `dialogName` string arg (≤259, classic sub_59AB20 cell→string).
3. `prevCtx = dialogueManager.meth_0x55C1C0()` (current-dialog state getter).
4. actor = netId→entity resolver 0x54BE10(self's "myself" id).
5. **map + owner resolution**: factory-intf **slot +84 = `GetMapById`** (`0x5457E0` →
   JE::World walk over the container at **World+0xE8**, matching `mapRec+0xB4 == mapId`, refcount
   at `mapRec+280`) called with `actor->E+0x1EC` (**current map id** — see §3 correction);
   then `map->vft[+32](actorEntity, &dialogName)` resolves **the character on that map that
   owns the named dialog**; miss ⇒
   `"DialogueCall() was unable to find the character with target dialogue name attached."`
6. manager `meth_0x55C200(v9+0x1DC)` + `meth_0x55C1E0(v9+0x1DC)` — push session context
   keyed by the owning character's netId.
7. `sub_425D00(out, dialogName, ".dlg")` — **resource name = `<dialogName>.dlg`** (new
   extension for the resource layer — add to the '.CRT/.AMO/…' table).
8. funcName = 2nd arg (≤63); fresh `Script_Small` on the stack, owner = `v9+0x1DC` (so
   `myself` inside = the DIALOG OWNER's netId, not the caller's); `Load(<name>.dlg, 1)`;
   then **`Script_Small vft[+0x18]` = `0x5A58C0(funcName, &result)`** — execute-public-by-
   name variant (VM doc slot [6]); failure ⇒ `"was unable to execute the script function."`
9. Returns the script function's int result; restores `prevCtx` via `meth_0x55C1E0(v4)`;
   dtors the script (Load-once-per-call — **dialog scripts are NOT cached**).

Error strings (verbatim, all reachable): "exceeded recursion limit." / "unable to retrieve
the dialog name." / "unable to find the character with target dialogue name attached." /
"unable to retrieve the function name." / "was unable to load the script for the target
dialogue." / "was unable to execute the script function."

## 2. Node-registration API (the script side building a dialogue graph)

All route through the **dialogue manager embedded at `[gateObj cls_0x5529e0]+0x5064`**
(Accessed as `global_area_map_icons[0]->int_0x18 + 20580` — the multi-singleton block from
gameinterfaces-vft-semantics doc; +0x5064 is inside/near the pause-gate fields 0x70E4..
that class owns — one big subsystem object).

| native | EA | effect |
|---|---|---|
| SetCurrentNodeName | 0x5C42A0 | get string cell → `sub_55C420(this=manager, name)` — map insert-or-find + node name set (**see §2b/§2c — the earlier "static buffer" guess is retracted**). **The anchor**: subsequent Register* attach to it |
| RegisterNodeText | 0x5C3980 | 1 arg (cell→string) → `sub_55C890(manager, text)` — text for current node |
| RegisterReply | 0x5C4060 | 4 args: `{nodeId?, replyIdx, actionFnName(≤63), nextFnName(≤63)}` → `sub_55D9A0(node, idx, actionName, nextName)` — reply = (script fn to run when chosen, script fn naming the NEXT node) |
| IsInDialogue | 0x5C39D0 | **CLOSED**: `manager.cls_0x55BF60(entity->E+0x1DC)` — session-membership by netId. ⚠ shipped fingerprint: its entity-not-found string reads **"IsUnderFOW() is unable to find the source entity."** (copy-paste bug — useful for original-vs-port log matching) |
| PCSpeaker | 0x5C3A50 | **CLOSED**: myself-id → entity must resolve → `manager.cls_0x55BFA0(myselfId)` returns the OWNER id (errors: "Unable to locate the entity that owns this active dialogue" / "Could not find a dialogue owned by this entity") |
| EnableDialogue | 0x5C9280 | **CLOSED**: entity byte **E+0x3A1 = dialog-enabled** (raw `*(e+929)`); set 1, error if already set; Disable (0x5C9300) = mirror (pending 1-line confirm) |
| SetNodeTextFunction | 0x5C41B0 | text-by-function variant (pending body) |
| RegisterLinkToOtherDialogue | 0x5C3F20 | cross-dialog link (pending body — likely pairs with DialogueCall) |
| WasLastNodeName | 0x5C4390 | compares against last-visited node name (script branching helper; pending) |

## 2b. Manager + registry internals (same day, §6.1/§6.2 partial close)

Overall model (unchanged from §2's opener): **the dialogue graph is script-push** — on dialog
start the engine calls the script's entry (via DialogueCall/Load), and the `.dlg` script
imperatively registers nodes/replies/texts while SetCurrentNodeName moves the anchor. The
tree is built at RUNTIME by the script, never stored in data files.

**Dialogue manager** (object at `[gateObj cls_0x5529e0]+0x5064`), layout observed:
```
+0x04  key/context string slot (passed to map ops as secondary key — refine)
+0x08  std::map<string, nodeRecord>   (find-or-insert via 0x55C050)
```
* `SetCurrentNodeName` → `sub_55C420(this=manager, name)` (usercall folds `this`):
  map insert-or-find keyed by name at +8 → node record; then record+0x10: assigns
  **string record+0x34 := (record+0x18, n)** via 0x401B20 — i.e. record carries
  `{+0x18 name SSO-string, +0x34 text SSO-string, +0x10 string-pair helper...}`.
  ⚠ NOT a global static as §2 first assumed — correction stands.
* `RegisterNodeText` → `sub_55C890(manager, text)`: finds via 0x55C050 with key
  **manager+0x04** (the current-node anchor!), reads `record->mbr_0x18`, then pushes
  `{id, text}` pair into the record's embedded list at `record+0x20`
  (`cls_0x50dbb0::meth_0x55AA60` — generic container impl, 136 methods — do not
  mistake the class for dialogue-specific). ⇒ RegisterNodeText attaches text to the
  node named at manager+0x04; nodeRecord texts = LIST (variant texts w/ ids).
* `RegisterReply(nodeId, idx, actionFn, nextFn)` → `sub_55D9A0` (usercall, body pending
  — IDA fails on its frame; call shape = two temporaries `cls_0x4d8d70 v5{+0x1c second
  string}` = {actionStr, nextStr} per reply at (node, idx)).

**Dialogue owner resolution — `Map::vft[+32]` (body pending)**: given (actor, dialogName)
finds the character on that map owning the dialog. The `factory+84 = GetMapById` container is
the **map registry** at World+0xE8 (jeworld-layout's "map#2" — now NAMED; refcount `mapRec+280`,
key `mapRec+0xB4`, `mapRec+136` server-side map id used by vft+116/29 hooks).

## 2d. `Map::vft[+32] = FindNearestDialogOwner(map, actorEntity, &dialogName)` (0x56E850)
Decoded same session (closes the §6 owner-resolution item):
- iterates **`Map+0x12C`** = the map's entity container (new named field — the per-map
  character list; insert site = queue);
- candidate test: `cls_0x50db20::meth_0x41E4B0(0, *(&entStr@E+0x1FC size@E+0x210), name, len)`
  == 0 → the entity's **E+0x1FC std::string == dialogName** (E+0x1FC string obj, size @E+0x210;
  matches the een2-eeov-eent doc's "strings → …E+0x1FC…" list — that field's *meaning* is now:
  **dialog name**, populated from the entity-file path not yet pinned — likely EENT); skip actor;
- among matches, returns the **3D-nearest** entity: `sqrt(dz²+dy²+dx²)` over floats at
  **E+0x04/0x08/0x0C** vs actor position — *third independent confirmation* of the reimpl's
  `pos @ E+0x04..0x0F` Mover-excerpt reading;
- none → 0 → DialogueCall's "unable to find the character" error.

## 2e. `F3::StartDialogue(sourceId, targetId)` native (0x45A040) — script-side trigger
```
source = netId-resolve(cell1); target = netId-resolve(cell2)   ; both errors logged
Entity::meth_0x54C440(source)                     ; pre-action (likely interrupt/cancel)
act = new GameActionInitiateDialogue(0x24) (ctor 0x404B40)
act->[+0x18..] = targetEntity ; flag byte +0x17 := 0   ; ⚠ script path flag = 0 —
                                compare the packet-73 handler (cmd-semantics doc), differs
Entity::AddAction(source, act)                    ; the 0x550720 canonical seam
```
Dialogues start **by action attached to the initiator**, not via the manager; the manager
receives the node graph only once a `.dlg` entry function runs the Register* family.

## 2f. Corrections to §2
* SetCurrentNodeName IS manager-bound (2b bullet 1) — retract the file-static hypothesis.
* RegisterNodeText key source = manager+0x04 current-node string (set by the
  0x55C420 path), NOT an implicit arg.
* §1/§2b: factory +84 = GetMapById + E+0x1EC = mapId (§3) — "dialog field" label retracted.

## 3. Entity +0x1EC = CURRENT MAP ID (correction — the earlier "dialog key" label retracted)

Decoded the same session: `JE::Entity::virt_meth_0x54BBA0(newMapId)` = **Entity::SetMap** —
the ONLY functional writer of E+0x1EC (store @0x54BC9A; other +0x1EC hits in the raw scan are
unrelated classes: CSoundSystem+0x1EC etc.). SetMap semantics (worth its own later pass —
lifecycle gold):
```
if (newId == 0): full leave — meth_0x568220; if E+0x1EC set: rec = GetMapById(E+0x1EC);
   serverFlag → this->vft[+116](rec+136); walk the child-set at DynamicObject gap2F4[4]:
   re-home children via sub_56E750(rec, child)+child[+44]=0; meth_0x54B2D0
old != new bookkeeping: GetMapById(old)->refcount280-- ; GetMapById(new)->refcount280++
E+0x1EC := newId
if (newId): map = GetMapById(newId)
  if (E+0x178 bool /*model dirty*/ && no/invalid ModelInstance) → meth_0x54ADC0 (model rebind)
  if (!E+0x...289[83]) { meth_0x568140(map->octree GetOctree_56D960); serverFlag →
    vft[+112](map+0x88); meth_0x54B220(newId) }
  migrate children (gap2F4[4] set): for child != newMap → map.meth_0x573230(child),
    child->+44 := mapId
finally: flip two dwords at gap384[8]/[12] (= DynamicObject+0x3AC/0x3B0) — map-change epoch flag
```
⇒ Confirms reimpl Mover-excerpt reading `mapId @E+0x1EC` ✓ and their `pos@+4` family
(Entity-vb base). ⚠ §1 step 5 and §2b updated accordingly: dialog-owner search is
**map-scoped** via `Map::vft[+32]`; factory +84 is map-registry, NOT dialog registry. The
E+0x1EC **initial writer** is this same SetMap (called at spawn — matches EME2 feeding
SetPosition+SetMap; watch `0x54BBA0` in lockstep for the spawn sequence).

## 4. Additions to the VM doc's call surface (cross-links)
* `Script_Small::vft[+0x18] (0x5A58C0)` = execute-by-name(&result) — the .dlg per-node
  invocation primitive (§1 step 8). Same class surfaces used: vft[+4] Load, vft[+0x14] Exec
  (index form), vft[+0xC] SetGlobals. scripting-abi §1 slot table updated mentally: slot [6]
  = 0x5A58C0 named now ("RunNamed").
* `.dlg` joins Scripts.grp resource extensions list (settings-surface doc bonus literals).

## 4b. `GameActionInitiateDialogue::OnUpdate_0x404BE0` — full initiator semantics (CLOSES
### the §6.3 opener item; A: read before porting your command-73 path)

Action layout (0x24): `{+0x11 = initiator Entity*, +0x19 = target Entity* (netId-refreshed
via 0x54BE10 each tick), +0x17 = "whose dialog" flag (0 = initiator's E+0x1FC, ≠0 =
target's), +0x16 = done byte}`.

```
target = resolve(action->targetNetId);        !target → done, return 0
if (target.E+0x1EC != initiator.E+0x1EC) → done      ; MAP GUARD (same-map required)
if (dist3D(initiator.pos, target.pos) >= 5.0f):       ; NOT in range:
    mv = new GameActionMoveTo(0x48); 0x55B350(mv, target)   ; follow-target setter
    mv->[+0x34] = 5                                          ; arrive-radius (=5.0f echo)
                                                                 *refine*
    AddAction(initiator, mv)
    again = new InitiateDialogue(0x24); again.target = target; again.flag = flag
    AddAction(initiator, again)                              ; ⭐ SELF-RE-ISSUE
    return 0 (this instance done)
; in range:
name := (flag ? target : initiator)->E+0x1FC string (full copy)
pc   := (flag ? initiator : target)
if (!GameParty->vft[+12](pc)) FatalError(
   "Initiator must be PC if target's DLG is being used."    (flag≠0)
   "Target must be PC if initiator's DLG is being used."    (flag==0))
DialogueManager::BeginDialogue_0x55EDB0(&name, initiator.E+0x1DC, target.E+0x1DC)
done := 1; return 0
```
Notes:
* **The approach dance is action-level recursion**: out-of-range → MoveTo(5) + fresh
  InitiateDialogue queued behind it — the same "queue behind" idiom A's pump mirrors;
  distance uses floats at **E+0x04/08/0C** (confirmation #4 of the pos layout).
* `flag` = whose DLG is used; `StartDialogue` passes 0 (script side). A: your packet-73
  handler should mirror the same field (check the 73 handler's ctor writes against +0x17).
* `BeginDialogue_0x55EDB0(manager, name, srcNetId, tgtNetId)` = session-start entry
  (next decode: sets manager ctx + opens window via GameDialogueInterface).
* Party membership = party obj (factory +104) `vft[+12](entity)` — cheap parity hook.

## 4c. `JE::DialogueManager::BeginDialogue_0x55EDB0(&dlgName, srcNetId, tgtNetId)` — session
### creation (the §4b "next decode" item, CLOSED)
1. builds `<dlgName>.dlg` (same sub_434600 helper as DialogueCall step 7);
2. `script = new Script_Small(0xE0)` **owner = srcNetId** → `Load(path, 1)` — inside the
   dialogue script **`myself` = the INITIATOR's netId** (why `PCSpeaker` resolves
   myself→entity→manager; cf. scripting-abi §1 MSLF global);
3. failure ⇒ `OutputDebugStringA("DialogueManager::BeginDialogue() - Error loading script
   <name>\n")` + dtor + return 0 (no session). (Raw Win32 OutputDebugStringA — rare direct
   kernel call in the engine layer.)
4. success ⇒ builds the **session record** (`cls_0x55df60`, ~0x114B; stack-constructed then
   copied into the manager list via `cls_0x55bb10::meth_0x55E960` on `manager+8` — same +8
   composite backing the §2b node map):
   `{+0x00 manager byte mbr_0x5 (inherit-flag, refine), +0x04 script*, +0x08/+0x14 =
   tgtNetId, +0x0C = srcNetId, +0x10 = 0, +0x18/+0x34 = two SSO strings (node-name slots),
   +0x50 int, +0x51 byte, entry-name buffer = "OnStartDialogue"}`;
5. **runs the script's `OnStartDialogue` function** — the entry where the .dlg registers its
   node graph (Register* family) ⇒ closes §2's model loop:
   `action → BeginDialogue → load → OnStartDialogue builds graph → UI reads graph`.

Full engine dialogue lifecycle now documented end-to-end: `StartDialogue native / packet-73`
→ InitiateDialogue action (range+map+party gates) → `BeginDialogue` (.dlg, myself=initiator,
OnStartDialogue) → node graph via manager → `DialogueCall` cross-dialog hops → per-reply
action/next functions via the VM → `EndDialogue` teardown (not yet located, queued).

## 5. Questions for A (reimpl)
1. Your command id 73 (dialogue) handler builds `GameActionInitiateDialogue(0x24)` — want the
   UI pairing next (GameDialogueInterface 0x379 / GameDialogueWindow 0xDC8 message + reply
   rendering), or the manager graph internals (sub_55C420/55D9A0 record shapes) first?
   The second is closer to running actual .dlg scripts; the first closer to the visible app.
2. Confirm your SpawnMapEntities doesn't yet touch Entity+0x1EC — we predict shipped .CRT
   entities carry dialog keys there only via template/GCRE (writer hunt pending); if your
   lockstep shows +0x1EC nonzero for Mainmenu actors, tell us which template chunk fed it.

## 6. Queue (this author)
1. ~~DialogueManager internals~~ mostly closed (§2b/§2d); remaining: 0x55D9A0 body (frame
   fix via disasm), manager context trio (meth_0x55C1C0/1E0/200 — decode), Map+0x12C
   insert site, E+0x1FC dialog-name WRITER (likely EENT parse — ties to een2 doc).
2. ~~World+0xE8 registry POPULATION~~ **CLOSED: it's the map registry (GetMapById, +84); the
   E+0x1EC writer = Entity::SetMap 0x54BBA0 (§3)**; map-register/unregister sites (who inserts
   Map objects into World+0xE8) = new small item.
3. ~~GameActionInitiateDialogue + packet-73 → window open chain~~ **§4b+§4c CLOSED**;
   BeginDialogue_0x55EDB0 body ~~pending~~ **CLOSED (§4c)**; remaining: GameDialogueInterface
   window mapping (66-member doc of its own — next big item).
4. ~~StartDialogue vs DialogueCall difference~~ **CLOSED (§2e/§4c)**: StartDialogue =
   begin a SESSION (action+BeginDialogue+OnStartDialogue); DialogueCall = run one named
   function of another .dlg (depth-capped, temp Script_Small).
5. ~~Enable/Disable/IsInDialogue/PCSpeaker~~ **CLOSED (§2 table)**; remaining mechanical:
   WasLastNodeName / SetNodeTextFunction / RegisterLinkToOtherDialogue bodies + Disable
   one-liner confirm. Entity state summary for the reimpl record: dialog =
   **{E+0x1FC name string, E+0x3A1 enable byte}**. NEW: `EndDialogue`/window-close path
   + `0x55D9A0` reply-store body + manager ctx trio remain.

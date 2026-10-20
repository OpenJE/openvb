# 2025-10-20 — Native bodies batch 1: uniform action-spawn pattern + quest natives are NO-OPS (session 33)

Target: ORIGINAL_REFERENCE. Read-only. Decodes 4 natives from the canonical §-pending list;
pattern generalizes to the remaining 41.

## 1. The uniform native shape (all four conform)

```
sub_59AF40(amx, params, arity, "Name")     ; param-count/arity check ("type" arg = arity)
sub_59AEF0(amx, idFromParams, &out, 1)     ; resolve entity-ID param → entity ptr
   errors via sub_59AD20 → script error channel ("<Fn>() was unable to find the ... entity.")
operator new(<action size>) + ctor → set target/params fields → Entity::AddAction(owner, action)
return 1 (handled) / 0 (bad params) — amx result via params block
```
⇒ every gameplay native = **construct action + AddAction**. Reimpl's port template: one
helper `spawn_action<A>(actorId, targetId, flags…)`.

## 2. Per-native deltas (the semantics that matter)

| native | action | distinguishing writes |
|--------|--------|----------------------|
| `A_Attack` (0x459eb0) | GameActionAttack 0xC8 | plain (defaults: needs-resolve, move-to-range allowed) |
| `A_JustAttack` (0x459f70) | GameActionAttack 0xC8 | **+0x68.byte2 := 0** (skip ResolveWeaponAndAttackMode), **+0x6D := 0** (disable auto-move) → "attack exactly as-is, don't reposition, don't re-resolve" — melee follow-up/anim-sync attacks |
| `StartDialogue` (0x45a040) | GameActionInitiateDialogue 0x24 | Entity::Touch(0x54c440) on SOURCE first; action+0x1D-byte := 0 — vs packet-73 path which sets it to **1** ⇒ that byte = "came-from-network/remote" flag (verify: 73 handler @0x46df38 `HIBYTE(v26->mbr_0x14)=1`) |
| `SetMoveMode` (0x45a140) | none — direct | arity 3; `meth_0x46EC70(a1,a2,mode)` gated on **player entity exists** (area-map-icons-obj+0x18); operates on the player controller, not an action |

## 3. ★ Quest/journal natives = no-ops in the shipped demo

`OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry` all point to **0x420990 = `return 1`** —
four table entries to a do-nothing "handled" stub. Scripts can *claim* quest ops succeed but
nothing happens. Journal state actually flows via **packets 97–99 → GameJournal** (session 31)
and the quest UI reads GameJournal — so the demo's quest pipeline is packet/UI-only, and the
script-facing quest API was never wired in this build.
⇒ Reimpl: do NOT implement script-side quest effects; wire GameJournal + ids 97–99 instead.
(0x420990 coincidentally equals a GUI_Window vft entry — the natives table entry is the stub,
not the window method.)

## 4. Remaining natives (40) — expectations set by the pattern

Getters (GetLevel/Skill*/HP*/Race/Gender/…) will be thin RTTI-cast + field reads (map onto
canonical §2 + session 23 stat tables); the interesting ones left worth decoding on demand:
`DisplayStringRef` (string-table→UI path, pairs session 27 'Strings'), `PlayMusic`,
`AddCNPCToParty` (party growth — GameParty+0x14 thread), `HideInterfaces`, `LockDoor/
UnlockDoor` (door ChangeState path — pairs reimpl door doc).

## 5. Cross-refs
canonical-fact-sheet §1/§2/§4; sessions 12 (73 handler flag contrast), 23 (stat getters),
29 (natives table), 31 (journal packets).

## 6. DisplayStringRef (0x45a410, arity 4) — partial (sess 37)

Requires `client+0xA0` non-null (two error paths 0x65DAD8/0x65DAA8) then chains
`[obj+0xC]` — the SAME object slot as the id-108 container-open handler (sess 31). Static
store-site hunt failed under the client-singleton accessor pattern ⇒ setter registration is
GUI-side; logged as experiment E7 (write-watchpoint on client+0xA0 during startup; the
captured backtrace names the interface manager class). Reimpl note: DisplayStringRef is a
NO-OP until that slot is populated — likely dialogue/loot-window scoping.

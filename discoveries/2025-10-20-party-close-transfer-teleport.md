# 2025-10-20 — Session 36: party thread CLOSED — no protagonist re-election; transfer & teleport natives decoded

Target: ORIGINAL_REFERENCE (IDA). Read-only. Completes session 35's party surface.

## 1. Remove hook: `0x46cd20` = bare `jmp 0x5a0710` — **GameParty adds NOTHING on remove**
⇒ **Protagonist (GameParty+0x14) is never re-elected or cleared**: only writes anywhere are
ctor-zero (0x46cd0e) and the count==1 latch (0x46cd56). After the first joiner leaves, +0x14
keeps pointing at the (possibly despawned/dead) first member — port faithfully: leader =
first-ever-joined, staleness is shipped behavior (consistency, if any, is script-side).
Base `JE::Party::RemoveMember` @0x5a0710: linear search in [begin,end), shift-down remainder,
`end -= 4` (no capacity/shrink semantics) — matches the raw dynamic-array model (sess 35 §1).

## 2. `JE::TransferItemToParty(owner, itemName, count)` @0x5ca203
```
GetParty (service vft+104) → iteration wrapper cls_0x55c8d0(&party+4) ; per-member loop:
   inv = *(owner+0x168)                       ; GameInventory ptr ✓ (sess 14/16)
   item = inv->vft[+24](&nameStr)             ; FIND-BY-NAME on inventory
   if !item: "…unable to find the requested item in the owner's inventory." → fail
   JE::Server::meth_0x555640(member, item)    ; server-side give (→ packet-96 family)
repeated `count` times (per iteration, per count index)
```
`TransferItemFromParty` (0x5ca013) = mirror by name+strings ("…item in party" errors at
0x6b6c81); roles swap — *(structural inference; one decompile confirms if needed)*.
Newly named engine surface: **inventory vft[+24] = FindItem(name)**; **JE::Server::meth_0x555640
= GiveItemToEntity** (pair with fact-sheet packet 96 inventory-add).

## 3. `JE::TeleportParty(mapName, x?/y?)` @0x5ca3c3 — the warp ritual, per member:
```
for i, member in party:                        ; same cls_0x55c8d0 iterator
   Entity::meth_0x54c440(member)               ; leave-current-world prep
   v = service intf->vft[+20](1)              ; new-instance/portal record
   member->meth_0x550720(v)                    ; encounter-ring push (same tail as activate)
   JE::Server::meth_0x5563d0(member, mapName, arg2+8, STAGGER = 5 - (i % 6), 1)
                                               ; server warp w/ 0..5 placement stagger
```
⇒ the `%6 / 5−k` counter is anti-overlap spawn ordering — port as-is; 5563d0 = the warp
server entry (compare fact-sheet packet 26 'warp'), 54c440 = per-entity map-leave.

## 4. Party-thread residue (everything else closed)
* iterator helper `cls_0x55c8d0` = 12-byte {begin,end} range wrapper freeing a copy (fine).
* IsInParty (0x5c72cc) left as trivial (same skeleton, vft+0xC predicate) — *(inferred)*.

## 5. Evidence
0x46cd20 (jmp only, 10 B); 0x5a0710 (search 0x5a0724, shift 0x5a0742, end-=4 0x5a074c);
Transfer (owner 0x5ca22a, name 0x5ca25a, party 0x5ca298, find 0x5ca303, give 0x5ca33b,
error strings inline); Teleport (prep 0x5ca44b, portal 0x5ca46b, ring 0x5ca471, warp
0x5ca49c incl. `5 - v7`, v7 cycle 0x5ca4b1).

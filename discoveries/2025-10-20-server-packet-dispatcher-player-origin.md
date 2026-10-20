# 2025-10-20 — Server packet dispatcher: GamePlayer origin CLOSED + create-chain (session 9)

Target: ORIGINAL_REFERENCE. Read-only. Closes the queue item "origin of the .CRT→Player flag"
from `2025-10-20-ext-classid-resolver-player-flag.md` §consequences-2.

## 1. `JE::Server::virt_meth_0x556ea0` = the server packet dispatcher

Switch on `*(packet + 48)` = **packet id**. Notable cases (strings are binary evidence):

| id | meaning (observed) |
|----|--------------------|
| 9 | spawn entity from full record (name + strings + ints) → `Server::meth_0x5554b0(..., a6=&nameString, param_2=0)` ★ |
| 10 | entity flag set (`v10->mbr_0x1e4` guard path via 0x54be10 lookup) |
| 15 | attach script — errors `"Entity already has a script."` / `"Invalid entity."` |
| 26 | per-entity field poke + `Entity::meth_0x550720` packet build |
| 37 (0x25) | no-op ack |
| 40 (0x28) | mgr via ws-vft+36 then mgr-vft+4(name, **1**) + callback registration w/ `off_692A30` |
| 45 (0x2D) | lookup via `(*(*this+140))(&name)` + sub_59A1C0 |
| 0x38/0x39 | server-side save/checkpoint-ish (`cls_0x55c2d0` ctor with entity record) |
| 0x3A | `Server::meth_0x556b60` (save?) |
| 0x3B/0x3C | position broadcast (meth type 6 / packet type 2 with x,y,z+flags) |
| 0x3D | entity-by-name (`JE::World-vft+88(name)`) + `Server::meth_0x552e20` (send full entity state, entity id at `entity+0xB4`? — sees `*(v70+180)`) |
| 0x41 | name-keyed lookup (string map) |
| 0x43 | **client join**: `"Client %d joined the game.  Assigned entity %d.\n"` → `meth_0x5554b0(..., ArgList=v72(name-from-registry), param_2=1, a6=0)` |
| 0x46 | map change: `"Warping to new map."` / `"Unable to load map."` (vft+88 by map name, iterate GUI entity list, per-entity send `5 - i`) |
| default (id≥9) | `"Server received packet with unknown id %d from %s\r\n"` |

## 2. The create-with-flag chain (★ = the Player origin)

`JE::Server::meth_0x5554b0(server, a2@ebx, a3@edi, char* name, char param_2, int a6)`:

```
v7 = JE::World vb (GameWorld + 0x394 this region, vft 0x668494)
v8 = (*v7->vft[+48])(name, a6)          ; = GameWorld::virt_thunk_0x46fe90 → sub_46FB20(name, a6)
                                          ; = the ext→classId resolver; a6 IS the flag:
                                          ;   .CRT: flag!=0 → 10 (GamePlayer), 0 → 1
v10 = v7->vft[+24](v8, a2)              ; = 0x46fdc0 → CreateGameEntityByClass — factory
v10->meth_0x551880(v10, a6, name, param_2, 0)          ; entity-side init
GameServer::meth_0x553600(server, name, v10->mbr_0x1DC, param_2, a6)  ; world add + broadcast
```

Verified the two pushed args at 0x55550c..0x55550e are (name=edi, a6=ebp) by raw
disassembly (`push ebp; push edi; call [eax+30h]`), so **a6 = resolver flag**, param_2 is
broadcast-side only.

Callers:
- **packet 9** (full spawn record): a6 = address of a name string from the packet →
  non-zero → **.CRT resolves to classId 10 → GamePlayer is created** (0xBFC). param_2=0.
- **packet 0x43** (client join): a6 = 0, param_2 = 1 → same chain but .CRT → classId 1
  (plain creature entity), then binds client (`v73->mbr_0x1DC` = the assigned entity id).
- Case 0x28 (packet 40) uses `ws->vft+36`(=0x544b80 slot 9) → a manager whose `vft+4(name,1)`
  also takes a literal 1 — second, different "by name with flag" API worth keeping in mind
  (client-side template request?).

## 3. Entity-id slot clarified

`v10->mbr_0x1DC` is read as the **entity's id** for join-assignment and broadcast
(`Assigned entity %d`). Relative to JE::Entity that's 0x1DC (Entity vb +0x1DC — two dwords
before the reimpl's `mapId@+0x1EC`; adjacent id/counter block 0x1D8..0x1EC lives in
Entity/DynamicObject — refine when JE::World layout session lands).

## 4. Net effect for the reimpl

- The "where does GamePlayer come from" answer: **server-side spawn packet (id 9) whose
  extra string/param makes the resolver flag non-zero**. Map EME2 data can never produce
  class 10 (consistent with your chunk-signature findings); only the network/server spawn
  path does.
- Suggested model: `Server::SpawnEntity(name, flag)` → `classId = ResolveByExt(name, flag)`
  → factory; keep flag propagation exactly this thin (it's just "a6 != 0" for .CRT).
- Packet-id table above gives you the wire vocabulary (script attach=15, pos=59/60,
  warp=70, join=67...) — useful when you compare GameNetwork lockstep. IDs 40+ partially
  guessed; correct as far as the decompile shows.

## 5. Queue

1. ~~Player flag origin~~ CLOSED.
2. `Server::meth_0x553600` (world-add/broadcast) — pairs with GameNetwork.cpp work.
3. Client-side counterpart of packet 9 (net::client handler) — same dispatcher shape
   probably exists in `JE::Client::virt_meth...` (find the sibling of 0x556ea0).
4. Old queue remains: clip-name table, damage Apply body 0x45c420+, JE::World layout,
   GENT f1..f6 consumers.

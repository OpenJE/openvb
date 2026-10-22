# 0x46dd70 - GameServer packet dispatch: the GAME_ID opcode map

Discovered 2026-10-22 by JadeBear while resolving the two-id question
(discoveries/2025-10-22-entity-band-vbptr-roundtrip.md addenda 11-12, parent:f1d2455);
written as its own artifact at WildKnight's request (his pause handoff
parent:048d1be explicitly leaves this file docs-lane, zero overlap).

## Identification

- Function 0x46dd70, `switch ( a5->mbr_0x30 )` at 0x46ddc5 (28 cases + default).
- Self-certifying witnesses (strings byte-located this pass):
  - "GameServer::recv_packet() - Entities for attack action not found." @0x667f60 (case 0x5B fail path, 0x46df2b) - the function names itself.
  - "GameNetwork::GAME_ID_ENTITY_PERK_INCREMENT - Could not find the needed entity." @0x667f08 (case 0x5D fail path, 0x46e67e) - names the enum family AND pins case 0x5D = 93 = GAME_ID_ENTITY_PERK_INCREMENT.
- `a5->mbr_0x30` = received-packet opcode at packet+0x30 (48 decimal). Reply packets built here store their id with `c7 84 24 64 01 00 00 <id> 00 00 00` (byte-verified at 0x46e450/0x46e4cf/0x46e53a): **the echo id equals the case id** (0x64 stores 0x64, 0x65 stores 0x65, 0x54 stores 0x54).
- The funnel `net::critical::section::meth_0x550720_550720(entity, action)` is the
  EnqueueEntityAction seam already cited in docs/f3/NativeWiringGaps.md - this table lands
  entirely on that named seam.

## Id map (case = decimal; bodies from this session's decompile, immediates byte-checked)

| case | dec | semantics | key EAs |
|---|---|---|---|
| 0x0B | 11 | resolve -> GameEntity cast; touch meth_0x54c440; build action type 0; funnel LABEL_105 | 0x46ddd3-0x46de2b |
| 0x3A | 58 | forward to base Server::0x556ea0, then sub_46CF70 | 0x46eab7 |
| 0x48,0x4B | 72,75 | double-resolve, results DISCARDED (no-op consume) | 0x46dfe3-0x46e00b |
| **0x49** | **73** | **SERVER TWIN OF StartDialogue**: gate `!field0xc(mbr_0x5064)` && actor && target; new GameActionInitiateDialogue(0x24) ctor_0x404b40; target->gap19, HIBYTE(mbr_0x14)=1; funnel 0x550720 | 0x46df3f-0x46dfd2 |
| 0x4A,0x4D,0x50 | 74,77,80 | double-resolve discard | 0x46e208-0x46e230 |
| 0x4C | 76 | double-resolve discard | 0x46e017 |
| 0x4E | 78 | target TYPE = *(target+348=0x15C): 3 -> RTTI GameDoor -> OperateDoor(0x28); 4 or 1 -> OperateContainer(0x24); funnel | 0x46e04b-0x46e0df |
| 0x4F | 79 | sibling of 0x4E with actor v125: door branch meth_0x406700(target), container branch meth_0x406a30 | 0x46e11b-0x46e1f7 |
| **0x51** | **81** | **SCRIPT-BY-NAME EXECUTION PATH (gap-3 witness)**: GameEntity cast -> JE::Entity::GetScript_54aef0 -> [script+16](script,"GetInfo")==1 predicate -> [script+24](script,"GetInfo",&out) INVOKE; fallback field +3; fan-out meth_0x554d30(…,128,128,128) | 0x46e23c-0x46e2db |
| 0x52 | 82 | pick-up: action 3 via [v55+20], OperateContainer-style setter, funnel | 0x46e2ec-0x46e343 |
| 0x53 | 83 | GIVE: GameActionGive(0x30) ctor_0x4052a0 + meth_0x4052d0/meth_0x4052f0 setters, funnel | 0x46e354-0x46e3e8 |
| 0x54 | 84 | GameCreature cast -> meth_0x409da0 -> reply net::packet id 84 (echo), send via meth_0x5a0a00 | 0x46e4e3-0x46e53a |
| 0x59 | 89 | GameEntity cast; action 11 with *(+32)=v125, *(+28)=2+(v125==2); funnel LABEL_105 | 0x46e82e-0x46e8a0 |
| **0x5A** | **90** | **SERVER SIDE OF ReloadWeapon**: actor resolve + GameWeapon cast + optional GameAmmo cast; GameActionReload(0x3C) ctor_0x4071c0; fields &+0x20/+0x28; funnel | 0x46e56e-0x46e632 |
| **0x5B** | **91** | **SERVER SIDE OF Attack/JustAttack**: 4-field extraction incl. sub_45E560 triple; GameActionAttack(0xC8!) ctor_0x403e40, meth_0x403fa0, +0x73+3 store, meth_0x401100(flag,&triple); funnel; named log on resolve failure | 0x46de37-0x46df2b |
| 0x5D | 93 | GAME_ID_ENTITY_PERK_INCREMENT: GameCreature cast -> sub_40A2F0(target, v126=rank) else named log | 0x46e643-0x46e692 |
| 0x5F | 95 | container-use A: meth_0x55fea0 over (entity+360=0x168 inventory obj); on any miss fan-out **meth_0x46d110(actor->mbr_0x1DC, item->+0x1DC via *(+476), …)**; else action 9, packet-ish fields +32=handle, +36, +40=30, +44=1, funnel | 0x46e6a3-0x46e77e |
| 0x60 | 96 | container-use B: same inventory probe; miss -> **meth_0x46d260(actor handle, item handle, flag)**; else action 10 with +32=handle, funnel | 0x46e78f-0x46e81d |
| 0x64 | 100 | GameWeapon cast -> meth_0x4134a0 -> reply packet id 100 (echo), send LABEL_70 | 0x46e3f9-0x46e45b |
| 0x65 | 101 | GameWeapon cast -> meth_0x413500(2nd arg) -> reply id 101 (echo), send | 0x46e467-0x46e4cf |
| 0x6A,0x6B | 106,107 | 260-byte blob extraction (meth_0x45e5e0) + double-resolve discard | 0x46e8bc-0x46e8d9 |
| 0x6E | 110 | WEATHER rain: sub_41D050 -> [..+92] area -> [v+84](area) -> lazy VEG attach "demo_rain.veg" (slots mbr_0x70f0/0x70f4) | 0x46e8de-0x46e973 |
| 0x6F | 111 | weather snow ("demo_snow.veg", mbr_0x70f8/70fc) | 0x46e97e-0x46ea11 |
| 0x70 | 112 | weather wind ("demo_wind.veg", mbr_0x7100/7104) | 0x46ea1c-0x46eaa4 |
| default | | JE::Server::virt_meth_0x556ea0 (base net::server handler forward) | 0x46ead0 |

## Why this table matters to the queue

1. **Three wiring-blocked rows get their server counterparts, cited**: StartDialogue (73),
   ReloadWeapon (90), Attack/JustAttack (91) - the shipped server funnels these into
   exactly the EnqueueEntityAction seam (0x550720) the natives were already read to reach.
2. **Case 81 is the missing script-execution witness for gap 3**: shipped code looks up an
   entity's script by GetScript_54aef0 and invokes it BY NAME through two vft slots
   (16 = predicate, 24 = call, both passed "GetInfo" @0x667f58). Any installer/adapter
   design should mirror this call shape - it is the in-engine proof of what Script::Run is
   FOR, sitting untouched in the dispatch table.
3. **The handle is a packet payload citizen**: cases 95/96 pass Entity+0x1DC (direct member
   read AND the *(entity+476) deref) as fan-out arguments and at struct offset +32 - the
   +0x1DC spine reaches into the network layer, consistent with addendum 12's verdict and
   with the slot-9 net-client reading.
4. **Entity type field = +0x15C** (348): 3 = GameDoor, 4 and 1 = container-family - a
   discriminator the door/container ports keep needing and the census never named.
5. Weather VEG names + lazy attach slots give the VFX lane its packet triggers (110-112).

## Census honesty (same-file, do not skim)

- My earlier "nine consumers" for Entity+0x1DC was WRONG as one list: the DB records
  **seven** under the JE::Entity key (0x549682, 0x54af2d, 0x54e6e9, 0x5506c8, 0x550a17,
  0x55146f, 0x552679); the other verified sites (Create stamp 0x55191e/0x55192a/0x551a60,
  Protagonist read 0x45a9e0, resolver cmp ~0x54be39) are attributed under OTHER class keys.
  The nine was the +0x1E0 census count TRANSPLANTED to the adjacent field - hazard-family
  costume #14: right number, wrong field. The decision in addendum 12 does not depend on
  the count; it rests on the read/write asymmetry, which holds from both directions.
- Case values are pseudocode-derived; the three echo-id immediates and all six strings were
  re-read from raw bytes this pass. The jump table itself was not dumped cell-by-cell;
  treat non-echo case data flows as pseudocode-grade until bytes say otherwise.

## Related census gap

The singleton whose slots 2/9/26 the natives dispatch through is still UNATTRIBUTED in the
DB: the two recorded server tables refute themselves against observed semantics (their
slot-26 entries are this dispatcher and its base default, yet live slot 26 returns Party;
their slot-2 entries are teardown-forwards feeding a write-only field). Refutation-by-
semantics is the tool: when recorded entries contradict known-working live behavior, the
attribution is false - not merely missing. File so no one re-walks 0x667dd4/0x6927bc
expecting a party getter.

Relay: this file deliberately does NOT self-cite its own commit hash - a file cannot contain
the hash of the commit that adds it (amending to insert one orphans the value; self-reference
has no fixed point). Re-derive with git log --format=%h on this path. Context parent:f1d2455,
parent:048d1be (pause block), parent:0432d9f (gaps).

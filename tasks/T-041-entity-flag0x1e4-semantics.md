---
status: done
assignee: PureKnight `01a15cb7` (statics; NOT a prerequisite for T-011 step 3)
depends: T-039 (found it), T-011 (uses it in a predicate)
---

# T-041 — What is `Entity+0x1E4`? Our name asserted a meaning the evidence does not support

## Known
- **Size 4** in the DB; the port declared a byte (widened during T-039's sweep, name demoted from `noTurn0x1E4` to
  `flag0x1E4`).
- ctor-zeroed at `0x55147b`; **unpacked from the entity record at `0x54e739`** — persisted state, not runtime-only.
- Readers gate **map membership**, not turns: `0x54bc3a` guards `--*(oldMap+280)` / `++*(newMap+280)`; `0x54947e` bails
  out of a movement update when it is 0.
- Appears in the turn predicate `+0x1E4 == 0 || +0x314 != 0` at `0x5a95c1`, whose second term is now established as the
  queued-action count.

## Question
Who **writes** `+0x1E4`, in whose caller chain, and which record field does it unpack from? Plausible readings include a
map-residency / in-world flag, a movement-enabled flag, or a spawn-phase marker. "No turn" is not among the supported ones.

## Method — and the trap that already bit once here
T-039's writer hunt failed because **the DB attributes stores made by embedded-container code to the container type**,
leaving the owner struct's `usages` list with readers only. So: start from the **unpack at `0x54e739`** and walk its
caller chain outward, and check the record layout on the `SpawnedEntity` side, rather than trusting an
`f3_member_index Entity 0x1E4` usage set (harness item 27). Statics before debugger (item 13).

## Acceptance
A cited writer set, the record field identified, and a name the evidence supports — or an explicit decision to keep
`flag0x1E4` as an honest placeholder. **Do not** alter the predicate at `0x5a95c1`: mirroring the shipped test at the
known offset and width is already correct.

## Boundary
Answering this does not gate T-011. If the semantics stay unresolved, the port mirrors the shipped test and the question
stays on the board — an unknown meaning is not a licence to invent one.

## Result (PureKnight 01a15cb7-e27e; full evidence in docs/f3/EntityFlag0x1E4.md)
**`Entity+0x1E4` is the map-membership flag: "entity is placed in the world / added to a map."**
Shipped name `inMap0x1E4` (replaces `noTurn0x1E4`, then `flag0x1E4`); "no turn" is refuted, and this is a
supported rename, not a placeholder decision.

- **Writer set (complete; caller chain from the unpack AND a local `.text` displacement scan against
  `$OPENVB_ORIG_EXE`, 53 raw hits owner-filtered):**
  1. `JE::Entity::ctor_0x5513a0` `0x55147c` — zero at spawn.
  2. `JE::Entity::UnpackState_0x54e690` `0x54e739` — record-stream int via the unique pointer producer
     `lea eax,[edi+1E4h]` at `0x54e72b`. Record context: the **3rd int of the per-entity state record**
     (after `+0x1C0` string, `+0x1E0` valid-marker), streamed by `JE::World::virt_meth_0x547e60` at
     `0x547f58`. Persisted state, consistent with the T-039 note.
  3. `Entity::SetInMap_0x551ba0` (DB-misnamed `net::critical::section::meth_0x551ba0`) `0x551c4a/0x551c4b` —
     the ONLY runtime store. No-op when unchanged; `0→nonzero` calls `Entity::meth_0x54c440`, attaches
     through the funnel `0x550720` (`0x551be4`), `++` the `map+280` member count (`0x551c0d`);
     `nonzero→0` decrements it (`0x551c3d`); clears an area-map-icon `ref` pointing at our `+0x1DC`
     (`0x551c60`). Non-virtual (no vftable entry), so the IDA direct-call xref set is complete:
     `0x58c90c` = Client recv case 14 of jumptable `0x58C6BB` (server→client `(entityID, flag)` packet;
     the engine packet family is literally named `ENGINE_ID_ENTITY_ADDTOMAP` string `0x69cb80` /
     `ENGINE_ID_ENTITY_REMOVEFROMMAP` `0x69cd98`, FilterPacket cases 12/13 at `0x58c4a1`/`0x58c861`);
     `0x555775` = `JE::Server::meth_0x555750`, reached only from the Server recv switch
     `virt_meth_0x556ea0` (jumptable `0x556EF1`, cases 58/67) — passes the server-side mirror
     `*(slot+0x4168)` (authoritative copy compared at `virt_meth_0x555380` `0x5553aa`) and rebroadcasts
     via `Server::meth_0x553cf0`.
- **Readers corroborate membership, never turns:** `0x54bc3a` guards the `--oldMap+280/++newMap+280`
  transfer; `0x54947e` skips movement when 0; `0x58c476` gates ADDTOMAP on `entity+0x1E4 ==
  client+0xBC`; `0x54973e` skips map-icon paint; the T-011 predicate at `0x5a95b0..0x5a95c1` verified
  on the **plain Entity base** (`mov eax,[edi+1E4h]`) — no `-0x148` aliasing; NOT altered, per acceptance.
- **No `+0x32C`-alias writer exists** — the three alias hits (`0x4f82b3`, `0x502e2c`, `0x51413c`) are
  VFX/Gfx classes' own `+0x32C` fields.
- **Tool failure modes (both directions, as in T-043):** `f3_member_index` usage lists are reader-biased
  for this field, and MCP `find_bytes` returned empty for provably-present patterns
  (`8B BE E4 01 00 00` at `0x54947e`). The displacement scan ran against the PE directly; scan covers
  `mod=10, disp32` encodings only, and stores through passed pointers are invisible to it — which is
  exactly why `UnpackState` appears as `lea`+call, accounted for above.
- **Port-side consequence (coordinator, same wave):** rename landed at
  `MapEntitySpawn.hpp`/`CombatManager.cpp`; `inMap0x1E4` is the first combat-state field that is
  network-authoritative, while `mover`/`movingFlag` are locally simulated — see the parity caveat below.

## Parity caveat for T-031 (routing, not a verdict)
T-046's shipped-side emitter walks the entity at the pump and the funnel. For this field those
breakpoints are **client-side**: the client value only changes by applying ADDTOMAP/REMOVEFROMMAP
packets (case 14), while the authoritative copy is `Server+slot+0x4168`. The original demo is a
single-player build, so client and server almost certainly share state, but "the pumped entity's
`+0x1E4` equals what the server set" is an **assumption that needs one observation** (compare
`SetInMap`'s argument against `[entity+0x1E4]` after the pump sees it) — record it as an explicit
assumption in T-031's write-up until captured.

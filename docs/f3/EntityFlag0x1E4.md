# Entity+0x1E4 — map-membership flag (`inMap`), not a turn flag

T-041 findings (PureKnight, sessionId 01a15cb7-e27e). ORIGINAL_REFERENCE (`ida_original_f3`, F3.exe.i64).
All EAs are shipping EAs. Method: caller chain from the unpack at `0x54e739` **plus** a raw `.text`
displacement scan (mod=10, rm!=4, disp32==0x1E4) because `f3_member_index` usage lists are reader-biased
(T-039 trap) and because the MCP `find_bytes` returns empty for byte patterns that provably exist
(e.g. `8B BE E4 01 00 00` at `0x54947e`) — the scan ran against `$OPENVB_ORIG_EXE` directly
(`artifacts` scanner: 53 hits, owner-filtered below).

## Verdict

`Entity+0x1E4` (dword) = **"entity is placed in the world / added to a map"** — the client-side mirror of the
server's per-entity map-membership state. The engine packet that drives it is literally named
`Network::ENGINE_ID_ENTITY_ADDTOMAP` (string `0x69cb80`, used by `Client::FilterPacket` `0x58c380` case 12 at
`0x58c4a1`), its sibling `ENGINE_ID_ENTITY_REMOVEFROMMAP` (strings `0x69cd98`/`0x69cdf8`) is the adjacent
recv case 13 (`0x58c861`). The setter's bookkeeping is map membership counting (below).
**Supported rename: `inMap0x1E4`** (was `noTurn0x1E4` → `flag0x1E4`). "No turn" is refuted: turns consult it
only through the `+0x1E4 == 0 || +0x314 != 0` predicate at `0x5a95b0..0x5a95c1` (verified disasm:
`mov eax,[edi+1E4h]` / `test` / `mov eax,[edi+314h]` — same Entity base, no `-0x148` aliasing involved).

## Authoritative setter — the function the DB misnames `net::critical::section::meth_0x551ba0`

`0x551ba0 Entity::SetInMap(this, v)` (size 0xd1):
- no-op if `this->+0x1E4 == v`;
- `0→nonzero`: calls `Entity::meth_0x54c440` (`0x551bbe`), attaches location via `0x550720` (`0x551be4`),
  then `++` the member count at `mapRecord(+0x1EC-lookup, vftbl slot 21)+280` (`0x551c0d`);
- `nonzero→0`: `--` the same `map+280` count (`0x551c3d`);
- on any change: writes `mov [edi+1E4h],<v>` at `0x551c4a/0x551c4b` (the **only** post-ctor direct store to
  this field in the entire `.text` scan), and clears `area_map_icons->ref` if it pointed at our `+0x1DC` (`0x551c60`).

It is **not** in any vftable (`f3_vftable_index 0x551ba0` → empty), so the IDA xref set is direct-call-only and complete:

| caller EA | function | door | what it passes |
|---|---|---|---|
| `0x58c90c` | `JE::Client::virt_meth_0x58c660` (Client recv_packet, jumptable `0x58C6BB`, **case 14**) | server→client engine packet | 2nd int of the packet `(entityID, flag)`; entity via lookup `0x54be10`, silent skip if not found (`0x58c8ff`) |
| `0x555775` | `JE::Server::meth_0x555750` | server-internal, reached **only** from `JE::Server::virt_meth_0x556ea0` (Server recv switch, jumptable `0x556EF1`; cases 58 `0x5575d0`→`meth_0x556b60` and 67 `0x5577cd`) — itself the `default:`+case-0x3A arm of `F3::GameServer::virt_meth_0x46dd70` (server receive pump) | `*(serverSlotBlock + 0x4168)` — the **authoritative server-side copy** of the flag (`JE::Server::virt_meth_0x555380` `0x5553aa` scans all entities for `entity+0x1E4 == *(slot+0x4168)`); then rebroadcasts via `Server::meth_0x553cf0` |

## Complete writer set (scan + caller chain, both agreeing)

1. `JE::Entity::ctor_0x5513a0` `0x55147c` — `mov [esi+1E4h],edx` with 0 (spawn-phase reset; matches `0x55147b` known).
2. `JE::Entity::UnpackState_54e690` `0x54e739` — record-stream int read (`GameActionEquip::meth_0x45e500` = int32 reader)
   through the **unique** pointer producer `lea eax,[edi+1E4h]` at `0x54e72b`. Record context: per-entity state record
   consumed by `JE::World::virt_meth_0x547e60` `0x547f58` (3-byte entity ID → lookup → `UnpackState`); field order is
   `string→+0x1C0, int→+0x1E0 (the "invalid/occupied" guard tested at 0x54e6ac), int→+0x1E4, int→+0x1EC (map ID, used by the
   setter's map lookup), vec3, vec3, ...`. So **the record field is the 3rd int of the entity state record, immediately after
   the record's valid-marker** — persisted state, consistent with the T-039 note.
3. `Entity::SetInMap_0x551ba0` `0x551c4b` — the only runtime store; callers per table above (doors: client recv packet
   case 14; server packet switch cases 58/67).

Scan caveat kept honest: the opcode scan covers `mod=10, disp32` encodings only (rm=4/SIB skipped; MSVC field code does
not use SIB here — the same scan is what surfaced the ctor store); stores made *through passed pointers* are invisible to
it, which is exactly why `UnpackState` shows as `lea`+call instead of a `mov` — accounted for above.

## Readers (semantics corroboration, all verified EAs)

- `0x54bc3a` (`Entity::virt_meth_0x54bba0`): guards the `--oldMap+280 / ++newMap+280` membership transfer — counts, not turns.
- `0x54947e` (`Entity::meth_0x549450`): movement update bails when 0 — "don't move what isn't in a map".
- `0x58c476` (`Client::FilterPacket` case 12 = `ENGINE_ID_ENTITY_ADDTOMAP`): accepts the add-to-map packet for an entity
  only when `entity+0x1E4 == client+0xBC` (current map context) unless the packet's byte says otherwise.
- `0x5a95b0` turn predicate (T-011) — untouched by this task, framing confirmed correct.
- `0x54973e` (`sub_549710`, map-icon paint pass): `entity->... || *(this+121)==+0x1E4` — draws only when in-map.

## Not writers (over-collection examples)

`0x4c00d0` (`cls_0x4c0190`), `0x51229d` (`Gfx_WaterTile::ctor`), `0x502dd8/0x502e2c` (`cls_0x502cb0`),
`0x4f2dbc/0x4f2d17` (byte stores, other class), `0x4f3208/0x4f3210` (x87 arithmetic), `0x5b1e5a/0x5b1f57`
(`BuildingTrigger`'s own fields), `0x32C`-alias stores (`0x4f82b3`, `0x502e2c`, `0x51413c`) belong to VFX/Gfx classes' own
`+0x32C` — **no GameWorld-based writer of the alias exists**, the predicate is plain Entity-based.

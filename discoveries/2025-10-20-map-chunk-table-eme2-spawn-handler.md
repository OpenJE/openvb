# 2025-10-20 — Session 21: MAP-file chunk table + EME2 spawn handler verbatim (validates reimpl pipeline end-to-end)

Target: ORIGINAL_REFERENCE (IDA). Read-only. Opens the map-load thread proposed in HANDOFF.
Headline: the shipped `JE::Map` EME2 handler @**0x573f30** matches the reimpl's
`SpawnMapEntities` flow step-for-step — and fixes two details worth mirroring.

## 1. Map-file chunk dispatch — `JE::Map::virt_meth_0x56c770` (vft slot 14)

Binary-tree dispatch on header fourcc at `*(chunk+8)`; each chunk → own Map-vft reader
(slot idx = dword index on the Map vft, base table 0x699fa4):

| fourcc (file bytes) | reader slot → body | error string |
|---------|------|------|
| `EMAP` | 15 → 0x56c9d0 | "Error reading header from map file.\n" |
| `EMEP` | 16 → 0x573b00 | "Error reading entry point from map file.\n" |
| `EMTR` | 17 → 0x575050 | "Error reading trigger from map file.\n" |
| `EMEN` | 18 → 0x573bf0 | "Error reading entity from map file.\n" (legacy entity record — EENT-analogue) |
| `EME2` | 19 → **0x573f30** | "Error reading entity from map file.\n" — §2 |
| `2MWT`⚠ | 20 → 0x575a40 | "Error reading water from map file.\n" (constant 1415007538; bytes spell "2MWT" — verify tag rendering vs file) |
| `ECAM` | 21 → 0x56ccf0 | "Error reading camera constraints from map file.\n" |
| `EMFG` | 22 → 0x56cd70 | "Error reading fog from map file.\n" |
| `EMNO` | 23 → 0x5742b0 | "Error reading note from map file.\n" |
| `EMNP` | 24 → 0x575180 | "Error reading nav point from map file.\n" |
| `EPTH` | 25 → 0x576940 | "Error reading way point from map file.\n" (path? slot named EPTH, msg says way point) |
| `EMSD` | 26 → 0x576c30 | "Error reading map sound from map file.\n" |
| `EMEF` | 27 → 0x578fb0 | "Error reading map effect from map file.\n" |
| unknown | — | **accept (return 1)** — same tolerant rule as entity files |

Map-level state lattice `Map::ChangeState_56c6c0` @Map+0xB8: `0→{1,2}→3`, 3 terminal
("Invalid state change in Map::ChangeState()." for anything else) — parallel of the entity
machine (world-tick doc). Cross-ref: `JE::World::PackState` guard checks World+0x34 == 3 ✓.

## 2. The EME2 record handler, verbatim (0x573f30) — vs the reimpl

```
1  name = StreamReadString                                       ; e.g. "Default_PC.CRT" ✓
2  tmpl = factory-intf vft[+52](name, 0)                         ; lookup-by-NAME ✓ (reimpl chain)
3  ent  = factory-intf vft[+24](tmpl)                            ; create-from-template ✓
4  pos  = 3 × u32-read (used as FLOATS)                          ; (reimpl: pos 3×f32 ✓ — the
5  rot  = 3 × f32;  rot *= 57.295776  → degrees                  ;  original literally reads u32)✓
6  flag = u8                                                      ; ✓ reimpl record.flag
7  JE::Entity::Create(ent, this_map, name, flag!=0, 1)   @0x551880 ; ← lifecycle-doc Create:
     ; (map*, nameStr, bool=flag, int 1). ALSO sets ent Entity+0x3B0?=flag byte (@0x57417e
     ;   "gap389[23]") — the Create name/art binding path (Player.amx default when name
     ;   empty — lifecycle §2) — flag stored for later (what reads +0x3B0: open)
8  SetPosition(ent, &pos[0])   (0x54a8a0 → session-17 grid dance: detach/recreate/attach!)
9  SetRotation(ent, &rot[0])   (0x549fe0)
10 nested Chunk::Read → meth_0x552000 = 'EEOV' reader           ; ★ PARSED AT SPAWN — the
11   on failure: ent->vft[0](ent,1) [destroy] + return 0          ;   initial-inventory hook
12 post: v10 = ent->meth_0x409cb0(); push into local list; if list non-empty → ent->meth_0x54b050()
    ; ⚠ CORRECTED (session 24 §2): 409cb0 = SSO string DATA accessor for Entity+0x238;
    ; this is a name/script-string ritual, NOT "children" handling.
13 REGISTER: insert &ent into Map entity list at Map+0x134/0x138 (sub_433760 RbTree insert)
```

**Verdict for the reimpl (RESPONSES thread)**: your pipeline is faithful. Deltas to mirror:
(a) EEOV parse happens HERE (spawn-time), not lazily — container contents/NPC inventories
materialize during map load; an EEOV failure destroys the entity and fails the record;
(b) pos triple is stream-raw u32 reinterpreted as floats (same bits — your f32 read is
equivalent; noted for byte-parity tests);
(c) the u8 flag goes to Create's bool param AND an entity byte (@0x57417e) — check your
Create port stores both;
(d) registration list = Map+0x134/0x138 (entity-registry head; your Map::Create registry
should match insert semantics incl. the sentinel-linked node shape).
(e) step 12 hook pair (409cb0 / 54b050) = name/script-string ritual per session-24 §2
(NOT children) — replicate only if a consumer emerges.

## 3. Where each spawned entity's template-load happens (the missing link closed)

Create (@0x551880, lifecycle doc) assigns net-id + inventory + name; the per-entity RESOURCE
file ('EMAP'-adjacent `.CRT/.ITM/...` streams from the resource system) is opened via the
ReadFromFile +0xE4 virtual (entity-file-format docs) — driven from wherever the template
demand happens (factory vft+52/24 loads the template resource → its EEN2/GENT/... chunks
flow through the session-5/6 dispatcher). ⇒ the two-stage load: **template resource first
(chunks incl. GCRE/GCHR/...), then per-record EME2 (map-side instance data + EEOV)**.
Reimpl's "accept-and-skip EEOV" (RESPONSES) confirmed as THE remaining gap for contents.

## 4. Open
- ~~EMEN (legacy) body 0x573bf0 diff vs EME2~~ CLOSED by session 22 §4 (verified full-body
  diff; obsolete path = EME2 minus EEOV/flag plus versioned script-string + pump-byte).
- EEOV attach trio (`sub_552A20` / meth_0x5512b0) — semantics CLOSED by session 22 §2
  (global attach-id counter / VEG attach-register); still ON the reimpl critical path (contents).
- ~~What reads the flag byte @Entity+~0x3B0~~ CLOSED session 22 §1: it is Entity+**0x3A0**
  = pump-enable (ServerUpdate pre-gate) — wire record.flag into your pump gate.
- ~~Map EMAP-header body (0x56c9d0)~~ CLOSED session 22 §3 (field map; 4 header strings'
  consumers remain open).
- Trigger/EMTR + EMEF bodies (script-volume + map FX machinery — likely ties to scripting-ABI doc) — OPEN.

## 5. Evidence index
Dispatch 0x56c770 (fourcc cmps at 0x56c783/794/79f/7a6/7bb/850/865/88b/8eb/8f.../97c/98d — file
bytes decode table §1). EME2 body 0x573f30 EAs inline. ChangeState 0x56c6c0 (lattice @0x56c6d9..
0x56c747). Map vft 0x699fa4 slot dump §1 bodies.

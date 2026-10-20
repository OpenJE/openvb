# 2025-10-20 — Entity resource file format: GENT + EFTR/EFEV/EFDT/EFCD (session 3)

Target: ORIGINAL_REFERENCE. Read-only. This decodes the **entity file** (per-entity resource
loaded after EME2 spawn) readers reached from `F3::GameEntity::virt_meth_0x468ad0`.

Entry point (`GameEntity::virt_meth_0x468ad0_468ad0`): reads chunk header at `a2`, dispatches
on `*(a2+8)` fourcc:

| fourcc | value | handler | meaning |
|--------|-------|---------|---------|
| `GENT` | 0x544E4547 | `cls_0x463870::meth_0x466510` (0x466510) | Game-Entity header block |
| `EFTR` | 0x52544645 | `cls_0x463870::meth_0x468930` (0x468930) | Ef**f**ect **Tr**ee |
| other | — | tail-jump 0x5528D0 (shared reject path) | |

`cls_0x463870` **is not a separate class**: the `this - 164` arithmetic in 0x468ad0 resolves to
the **GameEntity base pointer** (Entity-vb this − 0xA4). So every `mbr_0xNN` below is also
GameEntity's own field at +0xNN — this names all the mystery scalars from session 1 §2.

## 0. Common::Chunk header (prerequisite)

`JE::Common::Chunk::Read_0x4a1cc0` reads **3 dwords** from the resource stream into the
object at +0x08/+0x0C/+0x10 → on-disk header = `{u32 tag(fourcc), u32 version, u32 size}`;
`SkipToEnd_0x4a1d50` uses `size` (and `MemStreamStartMinusCursor` at +0x1C records the start
for that skip). Guard: re-`Read` without closing → `FatalError("Chunk::Read() called when not
in proper mode.\n")`. All `version` gates below are this header's `version` dword; all
fourcc compares are against the header `tag` dword.

## 1. GENT block wire format (0x466510 → 0x463870/0x463910)

Chunk-header field used as version: `v = *(chunk + 12)` = header `version` (see §0).

```
; always (meth_0x463870 @0x463870)
u32 f1 -> GameEntity+0x08      (ctor default 0)
u32 f2 -> GameEntity+0x0C      (ctor default 0)
u32 f3 -> GameEntity+0x10      (ctor default 0)
u32 f4 -> GameEntity+0x14      (ctor default 0)
u32 f5 -> GameEntity+0x18      (ctor default 4)
u32 f6 -> GameEntity+0x1C      (ctor default 0; read must return nonzero)
; if version >= 1 (meth_0x463910 @0x463910)
u32 max -> GameEntity+0x20     (ctor default 1)
u32 cur -> GameEntity+0x24     (ctor default 0), CLAMPED: if (cur > max) cur = max
; if version >= 2 / >= 3
calls 0x409bb0 — shipped as `return 1;` stub (forward-compat no-op in this build)
```

The `max/cur` clamp pair (+0x20/+0x24) is the first *semantic* anchor: current-not-exceeding-max
scalar (HP/AP-style). NOTE +0x20 is also what vft[0] getter `0x4080c0` returns —
the "getter" is effectively **get-current-stat**; its callers will name it next session.

## 2. EFTR block wire format (0x468930)

```
if version != 0:  u32 eventCount          ; else count = 0 → accepted as-is
repeat eventCount times:
    Common::Chunk::Read (0x4a1cc0)
    require header fourcc == 'EFEV' (0x56454645)   ; else
        "Unexpected effect chunk ID. Event ID 'EFEV' expected."
    → meth_0x468730(chunk, res)                     ; one effect event
    Common::Chunk::SkipToEnd (0x4a1d50)             ; tolerant to extra sub-data
```

## 3. EFEV event record (0x468730)

```
if chunk.subVersion != 0:
    u32 recordCount     ; stored on stack (named 'chunk' slot by decompiler)
    u32 entryCount
if recordCount == 0: accept (empty event)
repeat entryCount times:
    → meth_0x468490(this, a3, res, 0, EFEVchunk)   ; one event record; on fail → 0
```

## 4. Event record = recursive effect node (0x468490)

**Gate first**: the whole record parser no-ops (`return 1`) unless
`F3::global_area_map_icons[0]->Get_mbr_0x4_0x5b4f30() & 1` — the same server/editor flag that
gave `GameEntity+0x28` its net id in the ctor. Effect-tree parsing is **server-side only** in
this build. *(Interesting consequence: in pure-client runs the entity effect tree stays empty.)*

```
Common::Chunk::Read → nested header
if fourcc == 'EFCD' (0x44434645):
    u32 childCount; u32 x1
    effectObj = WorldServer->vft[+40](0)                  ; template lookup id 0
elif fourcc == 'EFDT' (0x54444645):
    u32 childCount; u32 x1; u32 eventId
    effectObj = WorldServer->vft[+40](eventId)            ; template lookup by event id
else:
    "Unknown effect chunk ID." → fail
cls_0x55a280::meth_0x55a280(effectObj, nestedChunkHeader, res)   ; consume node body
if top-level: push node into GameEntity effect slot
    (sub_468310(&v14[1].mbr_0x10 + 3*EFEVchunk))          ; list of nodes, stride 0xC
else: a5 = parent effectObj → sub_55ADC0(parent, child)   ; parented append
recurse: childCount times → meth_0x468490(..., parent=effectObj)
SkipToEnd
```

- `cls_0x55a280` (0x…): effect-tree node object — ctor self-ref; its `meth_0x55a280` reads the
  node's own payload right after its chunk header. **Next session.**
- The 5×12-byte RbTree heads at GameEntity+0x38 (session 1/2) now have a **confirmed**
  consumer: `sub_468310(&v14[1].mbr_0x10 + 3*i)` where `cls_0x463870` has size 0x28 (DB), so
  `v14[1] + 0x10 + 12*i` = **GameEntity + 0x38 + 12*i** — exactly the 5-tree array. The EFEV
  event index selects which of the 5 trees receives the node list (event class → container).
- WorldServer `vft[+40]` = `template lookup by id` — same template pipeline as entity creation
  (vft[+52] is the by-name create used in EME2 spawn; see `MapEntitySpawn.hpp` notes).

## 5. Field naming result (updates session 1 §2)

| GameEntity offset | old name | now |
|---|---|---|
| +0x08..+0x14 | mbr_0x8..0x14 | `gent.f1..f4` (raw header fields) |
| +0x18 | mbr_0x18 (=4) | `gent.f5` (header field, default 4) |
| +0x1C | mbr_0x1c | `gent.f6` |
| +0x20/+0x24 | mbr_0x20/mbr_0x24 | **max/cur clamped stat pair** |
| +0x28 | mbr_0x28 | net id (server-assigned, ctor) — confirmed separate from GENT |
| +0x30/+0x34 | mbr_0x30/0x34 (=7,7) | still unnamed — *not* loaded from GENT* |
| +0x38..0x74 | mbr_0x38 (5×12) | **effect-event node trees** (5 classes, indexed by EFEV loop position — container address algebra confirmed, cls_0x463870 size 0x28) |

## 6. Error/log strings observed (verbatim, for future searches)

- `"Error reading effect tree from entity file."`
- `"Error reading entity game data from entity file."`
- `"Unexpected effect chunk ID. Event ID 'EFEV' expected."`
- `"Unknown effect chunk ID."`
- `"Couldn't execute GetUsableSkills on the entity %s\r\n"` (+ adjacent `GetUsableSkills`) —
  scripting entry-point on entities; note **GetUsableSkills** as a probable GameUsableObject
  method name to watch in the vftables.

## 7. Open questions / next steps

1. `cls_0x55a280::meth_0x55a280` payload = the actual effect node parameters (name/model/eft
   fields). Session-4 candidate; footstep `.veg` strings from session 1 §5 live near here.
2. What `sub_55ADC0` stores (child list in node) vs `sub_468310` (root list in GameEntity+0x38
   tree) — confirm container shapes against §1 of session 2 (12-byte STL heads).
3. ~~Confirm which fourcc sits at Common::Chunk header +8~~ → **confirmed §0**:
   {tag, version, size} @ stream +0/+4/+8; object mbr_0x8/0xc/0x10.
4. For the reimpl: none of this is needed for static EME2 spawn, but it is the pipeline the
   moment an entity must answer to `Use`/`Reload`/combat scripts. The GENT field block (§1)
   is the cheapest first target: 6–8 dwords straight into GameEntity+0x08..0x24.

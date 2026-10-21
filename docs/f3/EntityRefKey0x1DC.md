# `Entity+0x1DC` is a runtime-allocated reference key — what D-3's "target vanished" case needs

T-043, statics only: read-only `ida_original_f3` plus a raw PE parse of the shipped image (no debugger, no IDB
writes). Feeds T-037's deviation D-3 (`docs/f3/ScriptNativeActionDoors.md`).

## Method note first: the database could not answer this

`f3_member_index Entity +0x1DC` lists **7** usages, and **none of them is the writer**. The producing store is recorded
under a different owner (`net::critical::section.mbr_0x1dc` at `0x55190c`) because IDA attributes a
`[reg+disp32]` access to whichever struct it guessed for the base register. So a writer hunt scoped to
`Entity` returns only readers, exactly the failure mode already recorded as harness item 27.

The scan that worked enumerates the **displacement** across all of `.text`:

```
mov [reg+disp32] / mov r32,[reg+disp32] with mod=10, rm!=4 (no SIB), disp32 == 0x1DC
```

→ **135 sites**, of which only **7** are in the DB list. **9 stores and 126 loads.** Of the 9 stores, only three
are on the entity path — `0x55146f` (ctor zero), `0x55190c` (`Create`), `0x54e6e9` (`UnpackState`) — and the other
six belong to different classes that happen to share the displacement (`0x4102b7` GameItem, `0x438b50`
GameDialogueInterface, `0x4c00c9` cls_0x4c0190, `0x502dcb` cls_0x502870, `0x511f3a`/`0x512224` Gfx_WaterTile).
For this field the per-struct usage list is ~95% incomplete; treat it as a hint, never as an xref set. The
flip side is also worth recording: a displacement scan **over-collects** across classes, so the six foreign stores
had to be excluded by owner, not by offset. The two tools fail in opposite directions.

## What the field is

| role | EA | evidence |
|---|---|---|
| zeroed at construction | `0x55146f` in `JE::Entity::ctor_0x5513a0` | `mov [esi+1DCh], ebx` with `ebx = 0` — defined, never indeterminate (contrast action `+0x14`, which the doors leave as heap: T-037) |
| **allocated at create** | `0x55190c` in the function logging `"\t+++ Entity::Create( %s ) begin +++"` (`0x5518e3`) | `0x5518ed mov eax, F3__global_cls_0x616328_0x70bd98` → walk its list at `+8` → `0x551909 call dword ptr [edx+24h]` → **`mov [esi+1DCh], eax`** |
| persisted | `PackState_0x545930` `0x5459c7` | `mov eax,[ebx+1DCh]; push eax` into the packet, ahead of `entity+0x15C` |
| restored on load | `JE::Entity::UnpackState_0x54e690` `0x54e6e9` | `mov [edi+1DCh], ecx`, `ecx` = the incoming stream value |
| resolved as a key | `meth_0x54be10` `0x54be33` | `cmp [eax+1DCh], esi`, `esi` = the **raw script cell**; fallback map `cls_0x50dbb0::meth_0x51f0f0` keyed by the same dword, returning `found[+0x10]` |
| crosses subsystems | `0x404db6`, `0x404df0` | passed as the speaker/target argument of `DialogueManager::BeginDialogue_55edb0` |

**Verdict: a per-session reference handle.** Allocator-assigned at create (so *not* a form id — a form id would be
read from the record, not handed out by a registry object), serialized through save/load (so *not* a pointer, which
cannot survive either), and matched against the value an AMX cell carries. That is the whole reason `Attack`/
`JustAttack` can store `*(target + 0x1DC)` at `record+0x38` instead of a pointer and re-resolve it at every use
(`0x40120b`, `0x4012e0`, `0x404409`), and why `StartDialogue` converts its caller-supplied pointer into the key in
`OnCreate 0x404b99`.

## What this settles and what it does not

* **D-3 stays as registered, and is now precisely describable.** Our port has a *serial* on the entity
  (`SpawnedEntity::netId`, `MapEntitySpawn.hpp:77`, cited to `GameEntity+0x28` "world-server-assigned serial") but
  **no key that a script cell carries and no re-resolution step**. So "target despawned mid-attack" is
  unrepresentable, not merely untested. A fix has two halves: give the entity a stable handle, and make consumers
  resolve it per use. Mirroring `meth_0x54be10`'s shape (list scan, then map) is the shipped behaviour.
* **`netId` is not a substitute.** It is a different offset with a different producer; do not let a future row
  "reuse" it because both look like ints.
* **Not settled here — routed:**
  1. What `vft[+0x24]` of the `0x70bd98` registry *does*: monotonic counter, free-list, or hash. Decides whether
     the key is digest-legible (`1,2,3…`) or opaque, which matters for T-031's field choices.
  2. `Entity+0x168`: written at `0x551924` in the same breath, with `0x55192a mov [eax+4], ecx` pointing the
     returned object back at the key — looks like the reverse link of the same handle. Unnamed.
  3. `0x4102b7` stores the same offset on `GameItem` and `0x4c00c9`/`0x502dcb`/`0x511f3a`/`0x512224` on other
     classes: shared allocator or coincidence of numbering. Untouched here.
  4. The `0x70bd98` singleton itself — which object it is and who writes it at `0x548CCF` — is recorded elsewhere
     (`discoveries/2025-10-20-gameworld-interfaces-effects-classid.md`); this row used it as a given rather than
     re-deriving it.

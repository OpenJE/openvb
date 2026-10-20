# 2025-10-20 — The real ext→classId resolver; `.por` solved; Player = CRT+flag

Target: ORIGINAL_REFERENCE. Read-only. Single-function discovery with immediate reimpl impact.

## `sub_46FB20` @0x46FB20 (GameWorld TU; exported as `GameWorld::virt_meth_0x46fe90` thunk =
JE::World **vb table 0x668494 slot[12]/+48**) — `int __stdcall(char* resourcePath, int flag)`

Raw decompile, in evaluation order:

```
ext = JE::Common::ExtractExtension(path)
.cmp  ".CRT" → return (flag != 0) ? 10 : 1        ← ★ Player is a creature-with-flag
.cmp  ".itm" → 5
.cmp  ".wea" → 6
.cmp  ".arm" → 7
.cmp  ".dor" → 3
.cmp  ".use" → 2
.cmp  ".pro" → 8                                   ← ★ NOT ".por"
.cmp  ".amo" → 9
.cmp  ".use" → 2   (duplicate compare — dead code, shipped as-is)
.cmp  ".con" → 4
else → F3::GetTypeFromFilename(path, flag)         (fallback chain)
```

Returned numbers are exactly the `CreateGameEntityByClass_0x46f530` factory cases (session in
full-geometry doc) — verified 1..10 against their ctors.

## Consequences

1. **`.por` open question CLOSED.** The extension-array static at 0x6685f4 reads `.por`
   (4 chars + NULs): compare against the resolver's `".pro"`. Either the array entry is a
   typo'd registry key that never matches, or `stricmp(".por",".pro")`... it does NOT match —
   the array's `.por` slot is thus for the resource loader (file-type registry), while the
   CLASS mapping uses `.pro` = projectile. *(Which one the EME2/template data actually uses
   for .PRO files: check the shipped archive listing.)*
2. **GamePlayer (class 10) creation** = same `.CRT` resource as creatures, selected by the
   second parameter of the name→type call. So the "where does the player come from" answer:
   whatever JE::World caller passes flag≠0 (party-join/PC-creation flow in the engine layer),
   NOT a different file kind. Matches the reimpl data point "Default_PC is ftype 1400/GCRE →
   class 1 from chunk signature": the chunk signature can only ever say 'creature'; the
   creature→player promotion is this flag.
3. The reimpl's `ClassIdFromTemplateChunks` should get the exact table above for the filename
   path: order-insensitive but note **`.use` duplicate** and **`flag` argument for .CRT only**;
   every other ext ignores flag.
4. Fallback `F3::GetTypeFromFilename(path, flag)` — engine-side type table for non-entity
   resources (maps/models). Unchased; say the word if the reimpl needs it.

## To find next (cheap)
- Callers of slot +48 (0x668494[12]) in JE::World to pin the flag's origin — likely
  `JE::World::CreateEntity(template*)` reading a template attribute (EEN2? the gender token
  "HumMNor" vs something for PC). Coordination: reimpl owns template parse — can you expose
  the EEN2 field list you already decoded? The flag probably lives there.

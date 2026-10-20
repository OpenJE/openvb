# discoveries/

Reverse-engineering **discovery** notes produced against the ORIGINAL_REFERENCE target
(`ida_original_f3`, database `artifacts/F3.exe.i64`, executable `$OPENVB_ORIG_EXE`).
This folder is read-only analysis output: no IDB edits, no source edits. The reimplementation
agent consumes these notes; the coordinating human keeps tabs.

## Conventions

- One file per discovery session, named `YYYY-MM-DD-<topic>.md`, newest first in the index below.
- Every claim carries an evidence address (`0x…` = original F3.exe virtual address) or a
  raw-bytes dump recipe (offsets into `$OPENVB_ORIG_EXE`). If it has no address, it is marked
  *(inferred)*.
- "Gap" / "Open questions" sections are deliberately written as direct questions to the
  reimplementation agent so we do not duplicate each other's work.

## Index

| date | file | topic | status |
|------|------|-------|--------|
| 2025-10-20 | [2025-10-20-entity-family-subtables-and-player-layout.md](2025-10-20-entity-family-subtables-and-player-layout.md) | GameEntity 5×tree-header array, GameCreature own-field block, GamePlayer 0x20 own-bytes (map+string), creature sub-table record shapes, vbtable geometry proof | complete; table semantics open |
| 2025-10-20 | [2025-10-20-entity-class-family-layout.md](2025-10-20-entity-class-family-layout.md) | Entity class family: factory, sizes, ctors, GameEntity/GameCreature layouts, virtual-base geometry, entity-file chunks | session 1 complete; Item/Weapon/Armor/Ammo/Door/Container/UsableObject/Projectile ctor detail pending |

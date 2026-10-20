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
- **Authorship**: files prefixed `2025-10-20-entity-*`, `2025-10-20-full-*`,
  `2025-10-20-effect-*`, `2025-10-20-gameworld-*`, `2025-10-20-gameeffect-*`, HANDOFF, README
  are authored by the DISCOVERY agent. `2025-10-20-gameentity-vft-decode-*` and
  `RESPONSES-from-reimpl-agent.md` are authored by the REIMPLEMENTATION agent.
- **Cite documents by FILENAME, not "session N"** — the two agents' session counters differ and
  collide (discovery session 5 ≠ reimpl session 5).

## Index

| date | file | topic | status |
|------|------|-------|--------|
| 2025-10-20 | [RESPONSES-from-reimpl-agent.md](RESPONSES-from-reimpl-agent.md) | Reimpl agent's answers + adopted changes + data donations (template chunk signatures, GENT ver=1, EME2 record layout, collision formats); discovery-agent reply appended at end | living thread |
| 2025-10-20 | [2025-10-20-gameentity-vft-decode-activation-and-inventory.md](2025-10-20-gameentity-vft-decode-activation-and-inventory.md) | (reimpl-authored) GameEntity Entity-vft decode: +0x2C animState, +0x30 weaponStance, LOS +0x90, QueueAnimEvent/OnAnimEvent token pipeline | living |
| 2025-10-20 | [2025-10-20-gameeffect-damage-anatomy.md](2025-10-20-gameeffect-damage-anatomy.md) | GameEffect/GameEffectDamage 0xB0 field map (damage amt +0x9C, DT enum +0xA4 0..5, flags +0xAC/AD), Apply opens with degrees→radians hit-direction atan2, ⚠ pipeline likely inert in demo unless scripting fills fields | partial: Apply body 0x45c420+ open |
| 2025-10-20 | [2025-10-20-gameworld-interfaces-effect-pipeline-closed.md](2025-10-20-gameworld-interfaces-effect-pipeline-closed.md) | GameWorld real interface layout (IWorldServer=0x66856C at +4; 0x668494=JE::World vb table), effect factory (type 1=GameEffectDamage 0xB0, 0=Effect 0x70), **Effect::Read is a no-op stub in shipped demo**, net-id getter World+0x88, GameItem = 26 own std::strings, GameEffectDamage field block | closes session-3/5 effect thread; damage-slot bodies pending |
| 2025-10-20 | [2025-10-20-gameentity-vft-decode-activation-and-inventory.md](2025-10-20-gameentity-vft-decode-activation-and-inventory.md) | Session 5: Entity-vb vftable 0x666044 (58 slots) decoded — +0x2C=AnimationState / +0x30=WeaponStance (QueryDefaultAnimation), activate + QueueAnimEvent→OnAnimEvent(DM/RL/EP/UP/PU/FL/FR) pipeline, GENT save writer verified, new fourccs EME2/EENT/GITM, F3::GameInventory at Entity+0x168, IsHumanoid, GetUsableSkills(26 skills) | complete; per-class +0xE4 fourcc matrix + effect catalogue pending |
| 2025-10-20 | [2025-10-20-effect-node-wrapper-worldserver-interfaces.md](2025-10-20-effect-node-wrapper-worldserver-interfaces.md) | cls_0x55a280 = id-wrapper trampoline; GameWorld server-interface vftable located @0x668494 (slot6=+24 create, slot13=+52 by-name lookup — reimpl chain validated); extension array .con/.amo/.por/.use/.dor/.arm/.wea/.itm/.crt; id mask 0x1FFFFFFF; session-3 correction | complete; effect-node dispatch re-derivation open |
| 2025-10-20 | [2025-10-20-full-class-geometry-vbtables.md](2025-10-20-full-class-geometry-vbtables.md) | Raw-verified geometry of all 11 entity classes: inheritance tree, vbtables, Entity-vb offsets, vftables, per-class ctor writes, runtime classId at Entity+0x15C (+ water-tile dual-use warning) | complete; F3-side readers of +0x15C pending |
| 2025-10-20 | [2025-10-20-entity-file-format-gent-eftr.md](2025-10-20-entity-file-format-gent-eftr.md) | Entity resource file format: Common::Chunk header, GENT field block → GameEntity+0x08..0x24, EFTR/EFEV/EFDT/EFCD effect-event tree, server-only gate, 5-tree container confirmed | complete; effect-node payload (cls_0x55a280) pending |
| 2025-10-20 | [2025-10-20-entity-family-subtables-and-player-layout.md](2025-10-20-entity-family-subtables-and-player-layout.md) | GameEntity 5×tree-header array, GameCreature own-field block, GamePlayer 0x20 own-bytes (map+string), creature sub-table record shapes, vbtable geometry proof | complete; table semantics open |
| 2025-10-20 | [2025-10-20-entity-class-family-layout.md](2025-10-20-entity-class-family-layout.md) | Entity class family: factory, sizes, ctors, GameEntity/GameCreature layouts, virtual-base geometry, entity-file chunks | session 1 complete; ctor detail later supplied by full-class-geometry doc |

---
status: in-progress
assignee: FastArrow (research)
---

# T-015 — GWAM attack-mode record map

**Pre-map delivered** (B9, `e453147`): parser identified as `GameWeapon::ReadAttackModeData`
(self-named via its error string); 21-field wire grammar, v5-gated, string at v4, packed bool
fields normalising into byte 1/2 of adjacent u32 slots; both derived tables pinned (caliber ->
attack style, 10-entry @0x414E54, 3-way jump @0x414E48; style -> animClip, 34-case @0x414E60,
constants 0x35..0x54+ = QueueAnimEvent id space 53..); `modeRec = recordBase + 0x18` cross-checked
against four attack-doc offsets. Per-section HIGH/MED/LOW-anchored confidence tags in the file.

**Remainder:** full slot table requires the frame reconstitution (push-shifted esp - the known
trap), creature-twin parity at 0x40E473, and the style 0-vs-3 labelling. Follow-on spawned as
**T-019** (does any shipped v5 record trip the consumed+thrown FatalError).

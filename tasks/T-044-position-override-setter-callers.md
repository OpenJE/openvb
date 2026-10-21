---
status: done
assignee: PureKnight (01a15cb7-e27e)
depends: T-037
---

# T-044 — Can any shipped door attack a *location*? Callers of `meth_0x401100`

## Question
T-037's follow-up list includes the position-override setter `meth_0x401100`. **Who calls it, and from which door?** If
a native, the packet path, or AI writes an override position before enqueueing, then an attack can target a *place*
rather than an *entity*, and the driver's `attack <actor> <target>` verb is missing a whole mode. If nothing calls it in
the attack path, the field is internal state and the verb is right as it stands.

## Method
Statics only: xrefs to `meth_0x401100`, classify each caller by door (script native / server packet / AI / engine), then
read what it stores and whether the record's consumers prefer the override over the target entity.

## Acceptance
A caller table (EA → door → what it sets), and one of two conclusions: **a location-targeting attack exists in the
shipped surface** (in which case name the record field carrying it and add the driver verb as a separate row), or **no
door reaches it** (in which case record that, so nobody builds a verb for a mode the engine never exposes).

## Boundary
"This setter is only called by engine code" is a claim about callers we *found*; if the xref set is incomplete because
something is called through a vftable we have not resolved, say so. Do not turn a failure to find a caller into proof
that none exists.

## Result (PureKnight, statics)
**A location-targeting attack exists in the shipped surface.** Sole caller `0x46df14`, in
`F3::GameServer::virt_meth_0x46dd70` (server receive dispatch), **packet ID 91 (0x5B)** — door: server packet.
The record fields carrying it: `GameActionAttack+0x48` hasPos byte, `+0x4C/+0x50/+0x54` X/Y/Z float override; case 91
accepts `attacker && (hasPos || target)`, i.e. a *place* attack with no target entity. Consumers prefer the override
(`OnCreate_0x4011d0` `0x4011da/0x40121f`; `OnServerUpdate_404320` `0x404410` skips target resolution when `+0x48`).
No script native sets it (T-037 §`+0x44..+0x54`) — reachable only through the packet door. Completeness: `meth_0x401100`
is not in any vftable (`0x647bbc` checked) and has no data xrefs, so the single caller is the complete shipped set;
the caller itself is reached via resolved `GameServer` vft `0x667dd4` slot 26. Full caller table + method:
**`docs/f3/AttackPositionOverrideDoors.md`**. **Follow-on row requested:** driver verb `attack-pos <actor> <x> <y> <z>`
(needs ZenNova numbering).

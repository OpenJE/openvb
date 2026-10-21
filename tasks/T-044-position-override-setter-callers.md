---
status: open
assignee: unassigned (statics; small)
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

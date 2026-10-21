---
status: open
assignee: WildYak (01a15802) — offered and accepted 2026-10-21; statics only, no debugger, no IDB writes
depends: T-037 (which found the gap as deviation D-3)
---

# T-043 — What *is* `Entity+0x1DC`? The ref key D-3 hinges on

## Why this row exists
D-3 is the accepted gap where **shipped carries the attack target as a key, not a pointer**:
`meth_0x403fa0` writes `record+0x38 = *(target + 0x1DC)` and every consumer re-resolves through **`0x54BE10`**. Our port
stores a pointer and re-resolves nothing, so *"target despawned mid-attack"* is unrepresentable. Before anyone can close
that gap — or decide not to — we need to know **what the key is**. That single answer decides whether closing D-3 is a
day's work or a subsystem.

## Question, narrowed from what statics can touch
- `UnpackState` at **`0x54e6e9`** — does the key **arrive persisted** (a save-time stable id) or is it **recomputed per
  session**? This is the load-bearing half: persisted ⇒ it names an entity across sessions; per-session ⇒ it is an
  index into live tables and our port can mirror it cheaply.
- The registration site **`sub_56C610`** — where is the value **created**, and is the creation **unique** (per
  instantiation) or **derived** (form id, archetype, table slot)?
- The re-resolution at **`0x54BE10`** — what container does it index, and what does it return on failure (null? a
  sentinel? does it set a state byte like the `+0x18=1` that `StartDialogue`'s `OnUpdate` does)?

## Discriminating hypotheses
| hypothesis | signature to look for | consequence for the port |
|---|---|---|
| **load serial** | monotonically increasing counter written once at spawn, never persisted | mirrorable with a session-local counter; D-3 closable in a day |
| **form/archetype id** | value comes from the record/template data, repeats across instances | **not** an entity identity — then `+0x38` cannot resolve *which* target, and our reading of D-3 is wrong |
| **table index / handle** | written by a registration routine, invalidated on deregistration | needs a live handle table in the port; closing D-3 becomes a registry row |
| **persisted identity** | `UnpackState` restores it directly from the save | parity-sensitive across saves, and a digest field candidate |

## Acceptance
- A cited verdict: creator EA, whether `UnpackState` restores or recomputes, what `0x54BE10` indexes, and its
  failure behaviour.
- **A statement of which hypothesis is *refuted*, not just which is supported** — the middle row above would rewrite
  D-3's meaning, and knowing we can rule it out is worth as much as confirming one.
- D-3's ledger entry updated with the answer, and an explicit **cost** line: closeable by mirroring, closeable by adding
  a registry, or not closeable without the packet door. If it is not closeable, say so and leave the deviation accepted.

## Boundary
Statics can narrow this but may not settle it: if the creating value is written by code we have not yet mapped, the
honest output is "creator unknown, three candidates remain" plus what would settle it. Do **not** infer identity
semantics from the fact that pointers "work in our tests" — they work precisely because our sim never despawns anything,
which is the gap under investigation, not evidence against it.

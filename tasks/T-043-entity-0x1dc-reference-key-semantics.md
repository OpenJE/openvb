---
status: done
assignee: WildYak (01a15802) — closed 2026-10-21, statics only (read-only IDB + raw PE parse; no debugger, no
# IDB writes). Verdict: **a runtime-allocated reference handle** — not a form id, not a pointer.
depends: T-037 (which found the gap as deviation D-3)
---

> **Findings → `docs/f3/EntityRefKey0x1DC.md`.** Zeroed by the ctor (`0x55146f`), **allocated by
> `Entity::Create`** (`0x55190c`, from `call [vft+24h]` on the `0x70bd98` registry), serialized by
> `PackState_0x545930` (`0x5459c7`) and restored by `UnpackState` (`0x54e6e9`), and resolved as a key by
> `meth_0x54be10` (`0x54be33`, matching the **raw AMX cell**), which is what lets `Attack` persist
> `*(target+0x1DC)` at `record+0x38` and re-resolve per use. Also crosses a subsystem boundary as an argument to
> `DialogueManager::BeginDialogue_55edb0` (`0x404db6`, `0x404df0`).
>
> **Method, and the part worth reusing:** the database could not answer this. `f3_member_index Entity +0x1DC`
> lists 7 usages and **none is the writer** (the store is attributed to
> `net::critical::section.mbr_0x1dc`) — harness item 27 exactly. Scanning the **displacement** across `.text`
> found 135 sites (9 stores / 126 loads), so the per-struct list is ~95% incomplete for this field. The scan
> also over-collects: 6 of the 9 stores are other classes sharing the offset, excluded by owner. **Both tools
> fail in opposite directions; neither is an xref set on its own.**
>
> **Effect on D-3:** unchanged and sharpened — the port has a serial (`SpawnedEntity::netId`, `GameEntity+0x28`) but
> no script-carrying handle and no per-use resolution, so "target despawned mid-attack" is unrepresentable.
> `netId` is explicitly **not** a substitute: different offset, different producer. Routed, not guessed: what
> the registry's `vft[+24h]` actually does (counter vs free-list vs hash — decides digest-legibility), and what
> `Entity+0x168` is (`0x551924`, with `0x55192a mov [eax+4], ecx` looking like the handle's reverse link).

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

# The party door: two script natives, one singleton, and a message that lies about its own return (B21, 2026-10-22)

`Protagonist` (0x45a960) and `AddCNPCToParty` (0x45a2d0) are the only batch-B natives that reach a party, and
between them they define the whole door. Everything here is byte- or string-cited from the original image.

## The access path, shared verbatim by both natives

    0x45a985 / 0x45a323   if ( global_0x70bd98 ) v = &g[2] + *(g[2].mbr_0x0 + 4)   else v = 0
    0x45a9a5 / 0x45a343   call [edx+68h]        ; vft slot 26 -> the Party object
    0x45a9a9 / 0x45a347   __RTDynamicCast(v, Party, GameParty, 0)
    0x45a9c7              mov eax,[eax+14h]     ; the LEADER entity
    0x45a9e0              mov eax,[eax+1DCch]   ; ...its net handle: 8b 80 dc 01 00 00

That last load is the third era of one field: `Entity::Create` (0x55190c) **stamps** `+0x1DC`, `SetHP`'s chain
(0x45b36d) **resolves through** it, and `Protagonist` **returns** it as its result. One dword, three subsystems,
no shared code path between them.

## Two facts that only exist because the natives are compared

1. **The message lies about its return value.** Both failure lines end in
   `"Returning INVALID_ID."` (0x45a9bb, 0x45a9d4) and both return **0**. There is no `INVALID_ID` reaching the
   script. The port keeps the mismatch exactly and asserts it as a pair (value == 0 AND the tail of the message
   is byte-identical), because "fixing" it - returning a real sentinel, or rewording the line - changes what
   scripts see in one direction or the other.
2. **A missing singleton is a crash, not a message.** Both sites build `v` as 0 when the global is null and
   then unconditionally dereference it at the slot-26 call. The party-failure message therefore describes only
   the *cast* failure, never the absent singleton. The port answers the cast message for both states, which is a
   **declared collapse** - see below - because a port cannot earn credit for reproducing an access violation.

## What each native adds on top of the door

- `Protagonist` takes **arity 0** - the only batch-B native with no arguments - and its result is a handle it
  never resolved from a parameter: leader `+0x14` → handle `+0x1DC`. Its second failure is a distinct message,
  `"Protagonist() Protagonist is NULL. Returning INVALID_ID."` (0x45a9d4), for a party with no leader.
- `AddCNPCToParty` resolves its entity normally - and its message says **"the entity"**, not the
  `"attacking entity"` copy-paste its inventory siblings carry (0x45a30e). It then dispatches
  `GameParty` **vft slot 0** with the resolved entity as the pushed argument
  (0x45a370: `mov edx,[eax]; push entity; mov ecx,eax; call [edx]`) and **returns that call's value**, so
  whatever add policy the party has is directly script-observable. There is no separate success constant.

## The port's shape, and the two collapses it declares

`F3::ScriptParty` (new, header-only, same standing-in style as `ScriptInventory`) models exactly what these two
call sites prove: a leader pointer, a member list, and an `Add` that answers 0 for "already a member" and for
"no room" - with the file stating plainly that recruitment rules, reputation, per-member state and the
ctor-stamped server relationship at `GameEntity+0x28` are **out of scope**, because nothing here observes them.

Two deliberate collapses, each named in the source:

1. **No singleton / singleton that is not a GameParty** both answer the cast-failure message with 0. Shipped
   crashes on the first and messages on the second. Reproducing a crash is not a behavior worth copying, and
   silently mapping the crash to the message would be worse without saying so.
2. **The first member becoming leader** is a port convenience in `Add` (`if (!pLeader) pLeader = pEntity;`),
   and the comment says nothing in these two call sites proves that policy. It exists so the model is usable,
   not because it is reversed.

Asserted consequences (196 asserts in the `ScriptNatives` suite): the handle round-trip - `Protagonist()`
returns exactly the registry handle the leader entity is registered under, which is the band field's third use
in one session; the two distinct failure messages; the message/value mismatch; arity 0 rejecting any parameter;
and `AddCNPCToParty` adding once, refusing a duplicate, and failing with its own message before the party is
even consulted.

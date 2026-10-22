# The task-journal trio — OpenTask / CloseTask / AddEntry (batch B slice B22)

Scope: `F3::OpenTask` 0x45b900, `F3::CloseTask` 0x45b940, `F3::AddEntry` 0x45b980, their
`F3::GameServer` delegates 0x46ecd0 / 0x46ed30 / 0x46ed90, the journal writers they reach, and the
packet tails behind them. Landed in `src/F3/ScriptNatives.cpp` (+ header block) with the suite section
`10`/`11` in `modules/openje/tests/je/ScriptNatives/ScriptNatives.cpp`.

All claims below are re-emitted from instruction text or from a stored immediate's bytes, not from
decompiler prose. Where the pseudocode and the bytes disagree about a NUMBER, the number cited is the
byte one (`+0Ch`, `+0x14`, `10h`, `2`) — the decompiler printed cell indices again, as it did in B2/B6/B10.

## 1. The three natives are one body three times

`disasm` at each EA: 24 instructions, identical except two fields (the gate's name string and the
delegate EA). Verified per row:

| step | OpenTask | CloseTask | AddEntry | what the bytes say |
|---|---|---|---|---|
| load amx / params | 0x45b900 / 0x45b905 | 0x45b940 / 0x45b945 | 0x45b980 / 0x45b985 | `mov eax,[esp+param_1]`, `mov esi,[esp+4+param_2]` |
| gate args | 0x45b909-0x45b912 | 0x45b949-0x45b952 | 0x45b989-0x45b992 | push `"OpenTask"`/`"CloseTask"`/`"AddEntry"`, push **2**, push params, push amx, `call sub_59AF40` |
| gate fail | 0x45b91e `xor eax,eax` | 0x45b95e | 0x45b99e | return **0**, and only here |
| read the two cells | 0x45b922 / 0x45b925 | 0x45b962 / 0x45b965 | 0x45b9a2 / 0x45b9a5 | `mov ecx,[esi+8]` then `mov edx,[esi+4]` — params[2] first, params[1] second |
| fetch the server | 0x45b928 / 0x45b92e | 0x45b968 / 0x45b96e | 0x45b9a8 / 0x45b9ae | `mov eax,ds:F3::global_area_map_icons` then `mov ecx,[eax+18h]`, **no test** |
| delegate | 0x45b932 | 0x45b972 | 0x45b9b2 | push ecx; push edx; `call GameServer::meth_0x46ecd0/0x46ed30/0x46ed90` — cdecl order `(params[1], params[2])`, `this` in ecx |
| return | 0x45b937 `mov eax,1` | 0x45b977 | 0x45b9b7 | **unconditional**, nothing tests the callee |

Three consequences that a port would otherwise get wrong:

- **No entity resolve.** These are the only batch-B natives with no `0x59aef0` call and no RTTI cast of
  their own — the two cells travel as opaque values. (Contrast: 24 of the other natives resolve.)
- **The gate name is each native's own** (0x45b909/0x45b949/0x45b989), so unlike `UnequipItem`, which
  ships `EquipItem`'s name in its gate (0x45a72c), a wrong-arity `CloseTask` complains about `CloseTask`.
- **Arity 2 means two CELLS.** The gate compares `params[0] >> 2` (0x59af4c) against 2, i.e. a 8-byte
  parameter block; a 0-cell block fails with "recieved 0".

## 2. Two null states are CRASHES; one null state is silent success

This distinction is the reason a note in `docs/f3/CreatureHpWriters.md` (B8) is corrected below — the
"did nothing" phrasing conflated three different shipped states, and only one of them is a no-op.

| state | where it faults | outcome in shipped |
|---|---|---|
| subsystem-manager global = 0 | `mov ecx,[eax+18h]` @0x45b92e / 0x45b96e / 0x45b9ae | **AV**, the native never reaches its return |
| world-server singleton `0x70bd98` = 0 | the branch sets the pointer to 0 (`xor ecx,ecx` @0x46ecdc / 0x46ed3c / 0x46ed9c) and the very next instruction on the merge path dereferences it: `mov eax,[ecx]` @**0x46ecea / 0x46ed4a / 0x46edaa** | **AV** |
| `RTTI Journal -> GameJournal` cast fails | `test eax,eax` / `jz` @0x46ed06-08 / 0x46ed68-6a / 0x46edc8-ca | jumps past BOTH the journal write and the packet, **no message**, and the native still returns 1 |

So the honest sentence is: *a script gets success whenever shipped does not fault* — and the only
non-faulting no-op is the failed cast. A null server does not "do nothing", it dies.

Port decision (declared, per standing rule 5 — a port does not earn credit for reproducing a crash):
the port has no subsystem manager and no 0x70bd98 singleton, so it cannot enter either fault state; it
answers **1** with no side effect, which is the value shipped's failed-cast path returns. The suite
asserts the silence as well as the 1, so a future "helpful" log on the success path goes red.

## 3. The delegates: one journal-lookup idiom, fourth sighting

All three delegates begin with the same base-plus-delta walk that SetHP (0x45b367-6d) and the attitude
pair (0x45aacf / 0x45ab2f) used, verified byte-for-byte identical at the head of each (0x46ecd0-46ecec /
0x46ed30-0x46ed4c / 0x46ed90-0x46edac):

    mov eax, ds:F3::global_cls_0x616328_0x70bd98    ; the world-server singleton
    mov ecx, [eax+8]                               ; the vbptr-style companion pointer
    mov edx, [ecx+4]                               ; the delta (a compile-time constant in the image)
    lea ecx, [edx+eax+8]                           ; the sub-object
    mov eax, [ecx]  /  call dword ptr [eax+78h]    ; VFT SLOT 30 (0x78 bytes = 120, NOT 30 cells)

then `__RTDynamicCast(result, 0, &Journal, &GameJournal, 0)` (TypeDescriptor pair pushed at
0x46ecf1/0x46ecf6 and siblings). Slot 30 is the journal accessor; the port has no implementation of it,
which is the first reason the chain is deferred as a chain rather than leg by leg.

## 4. What each journal writer does (read, recorded, NOT ported)

Two families, and the difference is not cosmetic:

- `sub_46C7A0` (behind OpenTask) and `sub_46C840` (behind CloseTask) are **the same upsert with a
  different literal**. Both walk the list whose sentinel head is at `this+0x14`, comparing the node
  payload's `+4` against `param_1` (0x46c7c3 `cmp [ecx+4],esi`); on a hit they store `param_2` at `+8`
  and the state at `+0Ch`; on a miss they `new(0x10)`, `JE::TaskStatus::ctor_0x46c2e0`, store
  `+4 = param_1`, `+8 = param_2`, `+0Ch = state`, then insert through `sub_433760` and bump via
  `sub_46C610(this,1)`. The ONLY difference: OpenTask stores **1** (0x46c7ee / 0x46c805
  `mov dword ptr [e..+0Ch],1`) and CloseTask stores **2** (0x46c88e / 0x46c8a5).
  Keyed on the task alone — so Open/Close overwrite the same row's state, and `param_2` is a value the
  journal keeps alongside it, not part of the key.
- `sub_46C6C0` (behind AddEntry) is **insert-if-absent keyed on BOTH fields**. Its scan RTTI-casts each
  node's payload `JournalEntry -> TaskEntry` (0x46c706) and requires `result[1] == param_1 &&
  result[2] == param_2` (0x46c71e) before calling it a match; a match does nothing (the entry is already
  there), a miss allocates `new(0xC)` — a smaller node than TaskStatus's 0x10 — with
  `JournalEntry.mbr_0x4 = param_1`, `mbr_0x8 = param_2` (0x46c750/0x46c753) and NO state field written.

Read that way, the trio's script surface is: `OpenTask(task, value)` / `CloseTask(task, value)` set a
1/2 state on one row per task, and `AddEntry(a, b)` adds an (a,b) pair once. What `a`/`b` mean in script
terms (task id? entry id? a localized-string index?) is NOT established by these bodies — the entries are
opaque dwords here, and naming them would be a guess.

## 5. The packet tails, and the id number space

Each delegate's tail is the server's own broadcast, and the ids are contiguous and distinct from the
RECV table:

| native | tail | packet id (stored immediate) | send |
|---|---|---|---|
| OpenTask | `GameServer::meth_0x46dad0` | **97** @0x46db25 `mov [esp+2050h+var_2018],61h` (= 97) | `meth_0x45f0b0(a1)` @0x46db2d, `meth_0x45f0b0(a2)` @0x46db3e, `meth_0x5a0a00(this, packet, 1)` @0x46db4c |
| CloseTask | `GameServer::meth_0x46dbb0` | **98** @0x46dc05 (same store form) | same three steps, `meth_0x5a0a00` @0x46dc2c |
| AddEntry | `GameServer::meth_0x46dc90` | **99** @0x46dce5 (same store form) | same three steps, `meth_0x5a0a00` @0x46dd0c |

The id lands in the packet's `+0x30` field (the same slot `net::packet` carries the game id in every
other tail, e.g. SetHP's 88), and `var_4` is zeroed as the SEH state just before it. Both cells are
serialized in push order (params[1] then params[2]) into a 0x2048-byte stack `net::packet`, with the
`net_address_vftable_6647a0` / `net_buffer_vftable_6648e8` teardown re-install at the tail. SetHP's broadcast is id **88** (0x46d620-era), so 88/97/98/99 are the batch-B send ids read so far.

Cross-check worth keeping: 97/98/99 do NOT appear in
`discoveries/2025-10-22-server-packet-dispatch-idmap.md`'s table (which covers the RECV dispatch at
0x46dd70: 11, 58, 72-84, 89-96, 100/101, 106/107, 110-112). Different number space — a send id is not a
recv case, and a future "why is 97 missing from the id-map" question has its answer here rather than in
a re-derivation.

## 6. What the port ships, and what it refuses to invent

Landed: the three bodies (gate, unconditional 1, per-EA cites), the table 27 → 30 in EA order, and the
suite's arity recomputation over every row. Deferred with the era, named in the source rather than parked
in prose: the singleton, slot 30, `GameJournal`/`JE::TaskStatus`/`JE::TaskEntry` containers, the list
insert machinery (`sub_433760`/`sub_46C610`), and packets 97/98/99.

Deliberately NOT built: a stand-in journal list inside `ScriptNatives.cpp`. The B21 party seam could
stand in because a party is a single object the natives already reach by pointer; a journal would force
this TU to decide WHICH world owns the journal — `GameWorld`'s `+0xF8` instance (allocated at
`src/F3/GameWorld.cpp:78`, unreachable from the natives today) or a server singleton that does not exist
port-side. Guessing would make the split look solved while hiding which side is authoritative — the same
error class as the handle→`SpawnedEntity` bridge that `docs/f3/NativeWiringGaps.md` gap 5 refuses.

## 7. CORRECTION to `docs/f3/CreatureHpWriters.md` (B8, the trio bullet)

That bullet said, in part: *"then `return 1` unconditionally — so a script opening a task journal entry
gets success even when the callee's `global[0x70bd98]` is null and it did nothing."* The `return 1` half
stands (verified above, and now enforced by the suite). The parenthetical does not: a null 0x70bd98 is
not a no-op, it is a fault at 0x46ecea / 0x46ed4a / 0x46edaa, one instruction after the branch that
 zeroes the pointer. The state that DOES return success without doing anything is a failed
 `Journal -> GameJournal` cast (0x46ed06 jz → 0x46ed28). Written up in §2 here; the original text is left
 in place with this pointer rather than silently rewritten, because the misreading itself is evidence:
 the `else v2 = 0` branch LOOKS like graceful degradation in pseudocode, and only the instruction after it
 says otherwise.

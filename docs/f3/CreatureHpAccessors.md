# GameEntity HP accessors - GetCurrentHP is a field, GetMaxHP is a VIRTUAL (B3, 2026-10-22)

Two script natives that look interchangeable (`int __cdecl (amx, params)`, same preamble, both about hit
points) and are NOT the same kind of read. Both cast **Entity -> GameEntity** (0x45b217 / 0x45b177), unlike
the B1 family which casts to GameCreature - so the class the handle resolves to is part of the contract.

| native | EA | terminal instruction | what it actually is |
|---|---|---|---|
| `F3::GetCurrentHP` | 0x45b1b0 | 0x45b237 `mov eax,[eax+24h]` | plain field read, GameEntity+0x24 |
| `F3::GetMaxHP` | 0x45b110 | 0x45b197 `mov edx,[eax]; mov ecx,eax; call dword ptr [edx]` | **vftable slot 0** call, this = the entity |

Slot 0 of `GameEntity`'s vftable (0x666130[0]) is `F3::GameEntity::virt_meth_0x4080c0` (0x4080c0), which is
two instructions: `mov eax,[ecx+20h]; retn`. So today max HP == GameEntity+0x20 - but it is reached through a
dispatch a derived class can override, and reading the field inline would erase that fact. The port therefore
funnels GetMaxHP through one seam (`GameEntityMaxHp()` in src/F3/ScriptNatives.cpp) rather than five inline
reads, and says so: the port has no GameEntity vftable yet, so the dispatch is the NAMED deviation, not a
secret one.

## The HP WRITERS are not writes to a field - deferred with the mechanism named

`F3::SetHP` (0x45b2e0) and `F3::AddHP` (0x45b390) are arity **2** (both cells expected) and neither stores
anything. Shared tail shape:

    45b367 mov edx,[eax+4]          ; eax = the resolved GameEntity
    45b36a mov edx,[edx+4]
    45b36d mov eax,[edx+eax+1E0h]   ; a fetched dword, NOT entity+0x1E0 - see below
    45b374 mov edx,[esi+8]          ; params[2] (the value)
    45b377 mov ecx, F3::global_area_map_icons
    45b37d mov ecx,[ecx+18h]        ; thiscall this
    45b380 push edx / push eax / call F3::GameActionEquip::meth_0x46ee10

AddHP differs only in arg1's source: it passes `*(entity+0x24) + params[2]` (0x45b3de-era), i.e. **current HP
plus the delta** - so AddHP(+N) is shipped as "set to current+N", not as an increment primitive.

`meth_0x46ee10` (0x46ee10) then does: `meth_0x54be10(this)` (the same entity-handle resolver the natives use -
so its `this` is a HANDLE, not an object, and IDA's `GameActionEquip` attribution is one of the je->f3 /
mis-owner artifacts this board keeps cataloguing), RTTI-casts the result to GameEntity, then calls
`meth_0x4629c0(entity, param_2)` and `meth_0x46d620(param_1, param_2)` and returns 1 (0 on either cast
failure). Two things stay UNRESOLVED and are the reason these rows are NOT ported yet:

1. the `[edx+eax+1E0h]` fetch - `edx` is `**(entity+4)+4`, so the expression is entity + a loaded delta +
   0x1E0, i.e. a multiple-inheritance/thunk-style adjustment or an IDA operand-fold artifact. What object is
   actually being addressed is not settled, and a setter whose subject is unknown cannot be ported honestly;
2. what `meth_0x4629c0` vs `meth_0x46d620` split means (one takes the entity, the other takes the fetched
   arg1 - so HP is written to *two* places, and which one is the authoritative current-HP matters for parity.

Named-blocker, not a punt: read both callees whole (and the +4/+4 chain) in B4, then port SetHP/AddHP as a
pair, and only then the skill arrays - in that order, because the writers define what the HP fields *mean*.

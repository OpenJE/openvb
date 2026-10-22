# SetHP / AddHP are read whole - and are NOT "write the HP field" (B4 research, 2026-10-22)

Both are arity **2** (`sub_59AF40(amx, params, 2, "SetHP")` at 0x45b2fe / 0x45b3ae), both resolve `params[1]`
with the active-check flag, both cast **Entity -> GameEntity** (0x45b347 / 0x45b3f7) - and then neither stores
anything. They enter a shared setter chain:

    ; SetHP tail, AddHP differs only in the value it pushes (see below)
    45b367 mov edx,[eax+4]            ; eax = the resolved GameEntity
    45b36a mov edx,[edx+4]
    45b36d mov eax,[edx+eax+1E0h]     ; a HANDLE, read through a virtual-base adjustment
    45b374 mov edx,[esi+8]            ; params[2] = the value
    45b377 mov ecx, F3::global_area_map_icons     ; IDB name; the +0x18 slot is what matters
    45b37d mov ecx,[ecx+18h]
    45b380 push edx / push eax / call F3::GameActionEquip::meth_0x46ee10

**AddHP's operand** is `*(entity+0x24) + params[2]` (0x45b3de-era): shipped "add HP" is a **set to
current+N**, not an increment primitive. Current HP is +0x24, which is exactly what `GetCurrentHP` reads
(docs/f3/CreatureHpAccessors.md), so the pair is consistent.

## The chain, all three bodies read

`meth_0x46ee10` (0x46ee10) - the IDB's `GameActionEquip` attribution is wrong in the now-familiar way; the
bytes show what it is:

1. 0x46ee16-0x46ee19: resolve the **fetched handle** through `meth_0x54be10` - the same handle resolver the
   natives use, called with the value on the STACK (0x59aef0's `mov eax,[esp+arg_4]; push eax; call` shape),
   not in `ecx`. Returns 0 -> the whole setter returns 0 (`retn 8` at 0x46ee44) with NO error string: unlike the
   natives, this path is silent.
2. 0x46ee34: `__RTDynamicCast(resolved, 0, &Entity, &GameEntity, 0)`; null -> 0.
3. 0x46ee4f: `meth_0x4629c0(entity, value)`.
4. 0x46ee58: `meth_0x46d620(this = *(global+0x18), handle, value)`, then return 1.

`meth_0x4629c0` (0x4629c0) is the state write, four instructions, and it is a **CLAMP**:

    result = this->mbr_0x20;              ; max HP
    this->mbr_0x24 = param_1;             ; current HP = value  (UNCONDITIONAL first)
    if (param_1 > result) this->mbr_0x24 = result;   ; cap at max
    return result;                        ; returns the OLD MAX, not the value written

Two consequences worth stating plainly. (1) **The natives return success, not HP**: SetHP/AddHP end with
`return meth_0x46ee10(...)`, so a script sees 1 or 0 (0x46ee40/0x46ee47-era), and 0x4629c0's own return value -
the OLD MAX - is discarded by the caller. A script wanting max HP must call `GetMaxHP`; the setter gives no
number back. (2) **There is no lower clamp**: the store at 0x4629c9 happens unconditionally and the cap at
0x4629cc only tests `> max`, so a negative value is stored as-is. HP can go negative through this door, which
is exactly the fact the combat lane needs before assuming HP >= 0 anywhere.

`meth_0x46d620` (0x46d620) is the network side: builds a `net::packet` (ctor 0x45f300, 0x2048-byte stack
frame with a cookie), sets `packet.mbr_0x30 = 88`, writes the handle and the value through
`net::packet::meth_0x45f130` twice (0x46d67d, 0x46d68e), sends via `GameServer::meth_0x5a0a00(packet, 1)`, and
logs `"Server - Sending GAME_ID_ENTITY_SETCURRENTHP"`. So **HP is a server-authoritative, broadcast change** -
a script cannot set HP locally without the packet going out. `meth_0x5a0a00` (0x5a0a00) is the fan-out: loop
`i < this->...[92]` calling `meth_0x5a09d0(this, i, packet, 1)` - i.e. send to every connected peer.

## Why the natives are NOT ported yet - one question, named

The clamp is applied to the entity resolved from **`*(entity + vbdisp + 0x1E0)`**, not from `params[1]`. Those
are the same object **if and only if** the value at virtual-base +0x1E0 is the entity's own net handle. The
shape says yes (the `+4 / +4` pair is the MSVC virtual-base displacement chain, +0x1E0 sits immediately before
the handle band T-041 pinned at +0x1E4/+0x1DC, and the same value is what goes into the packet as the change's
target). But `artifacts/F3.json` records no member at +0x1E0 (f3_member_index returns nothing), so the frozen
export cannot confirm the writer, and I refuse to port a two-step identity as if it were a one-step one: if the
field could ever carry a *borrowed* handle, shipped SetHP writes a DIFFERENT entity than the one the script
named, and a "clean" port would silently differ in exactly the case that matters.

Cheapest way to close it (B4's follow-up, one sitting): find the writer of the +0x1E0 slot in the entity
construct/pack path (`Entity::Create` 0x55190c-era and `PackState`, both already known to touch the handle
band) and confirm it stores the entity's own id; then port the trio as: `SetHP = resolve -> read own handle ->
0x4629c0 clamp (cap at max, NO floor) -> 0x46d620 packet id 88 (deferred until the GameServer era, named) ->
return 1`, with the clamp asserts including the negative pass-through.

Nothing here is guessed: every instruction quoted was read from the disassembly, and the one open link is
stated as the open link rather than assumed shut.

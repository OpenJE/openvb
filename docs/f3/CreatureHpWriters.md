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

## FOLLOW-UP (same sitting): the +0x1E0 hunt made the question SHARPER, not smaller - the deferral now has two reasons

`artifacts/F3.json` DOES record the slot - as **JE::Entity+0x1E0** (`mbr_0x1e0`, size 4, base-inherited, so it
is the same field GameEntity sees through the virtual base). Its nine usages include `JE::Entity::ctor_0x5513a0`
(write at 0x551475) and `JE::Entity::virt_meth_0x54ab00` (reads at 0x54ab0c/0x54ab53). So my "the frozen export
cannot confirm the writer" line was wrong in form: the export has the member, my first query filtered on the
wrong class (GameEntity) and returned nothing - a lookup mistake, not an evidence gap. Item 66 in my own face.

But reading that second function is what matters. `virt_meth_0x54ab00` (0x54ab00, 0x45 bytes) is:

    result = 1;
    if ( !(*([vft] + 25))(this) ) {                 // dynamic-object vft slot 25
        v2 = *(this + 0x1E0);
        if ( v2 != 1 && v2 != 3 && v2 != 2 && v2 != 6 && !*(this+0x2F0) && !*(this+0x214-ish) )
            return 0;
    }
    return result;

**+0x1E0 is compared against the small integers 1, 2, 3, 6** - which makes it look like a TYPE/MODE enum, not a
runtime handle. A handle would be compared against other handles, and here it is enumerated against four
one-digit constants. Meanwhile `SetHP`'s chain feeds the value read at that very offset into `meth_0x54be10`,
whose whole job (T-043) is to MATCH a cell against `entity+0x1DC`. Either:

  (a) the offset in `[edx+eax+1E0h]` is not +0x1E0 of the entity I think it is - i.e. the `+4/+4` chain puts a
      DIFFERENT object in `eax` (a containing/owning object), and the enum-looking field at Entity+0x1E0 is a
      coincidence of numbering; or
  (b) `meth_0x54be10` is being handed a non-handle and falls through to its map branch (0x54be41) in this path,
      meaning SetHP's subject is found by a route that has nothing to do with the script's handle at all.

Both readings keep the deferral alive and neither can be settled from what I have now, so the honest status is:
**SetHP/AddHP are read down to the last instruction I can interpret, and the one operand that decides WHO gets
written is genuinely ambiguous.** That is a much better sentence than a ported guess: a clamp on the wrong
entity is exactly the bug a test suite cannot catch, because the test would assert the same wrong entity.

Cheapest next probe (B4b, named so nobody re-derives it): disassemble `Entity::ctor_0x5513a0` around 0x551475 to
see WHAT is stored at +0x1E0 at construction - a small constant means enum (supports (a)/(b) split, and SetHP's
fetch is a different object), a pointer-ish/id-like value means handle. That single read flips the call, and it
is one function window rather than another chain chase.

## RESOLVED AS FAR AS IT GOES (B4b): Entity+0x1E0 is an ENUM, so hypothesis (b) is dead and the subject is a different object

The probe ran. Two readers settle the field's nature:

    0x549030 (JE::Entity::virt_meth_0x549030, 11 bytes)  ->  return *(this + 0x1E0) == 7;
    0x54ab50 (JE::Entity::virt_meth_0x54ab50)            ->  mov eax,[esi+1E0h]; cmp 1,2,3,6 ... then
                                                              [vft+0x64] test, +0x2F0, +0x314 fallbacks
    0x551475 (Entity::ctor_0x5513a0)                    ->  mov [esi+1E0h], ebx  (a zero-init run:
                                                              +0x1C4/+0x1DC/+0x1E0/+0x1E4/+0x1E8/+0x1EC all ebx)
    + two writers inside JE::Entity::UnpackState_54e690 (0x54e6ac, 0x54e733)

Values tested: **1, 2, 3, 6, 7**. A field tested for equality against seven one-digit constants, deserialized
by `UnpackState`, and zero-initialised alongside its neighbours is a TYPE/MODE enum. It is NOT a runtime
handle, and `meth_0x54be10`'s list branch matches `entity+0x1DC` - a DIFFERENT offset, adjacent, and per T-043
the handle proper.

Consequence: of the two open readings, **(b) is dead**. `SetHP` does not hand an enum to the handle resolver -
which means the base register in `mov eax,[edx+eax+1E0h]` is NOT the Entity subobject these readers share, and
reading that operand as "the entity's own +0x1E0" was my inference, not the bytes' statement. The `+4/+4` chain
(`[entity+4]` then `[+4]`) is still an uninterpreted adjustment - a vbtable displacement would land on the enum,
which the readers just ruled out, so the loaded value is either a different object's base or a table whose slot
4 is not a vb displacement at all.

**Status of SetHP/AddHP: still unported, and now for a better reason than before.** The blocker is no longer
"is the handle self-referential?" (answer: it isn't a handle at all); it is **which object the fetched operand
belongs to**. That is exactly the question whose answer decides WHOSE HP gets clamped, so guessing it would put
a wrong-entity write into product code with a test asserting the same wrong entity. Next probe, named:
disassemble `JE::Entity::virt_meth_0x54dea0` (0x54dea0, 0x106 bytes, uses +0x1E0 at 0x54df63) and the two
UnpackState sites - if +0x1E0 is written from a byte/short stream it is a serialized type code, and then the
SetHP chain's `+4/+4` deserves its own short doc instead of being folded into this one.

What this sitting produced anyway, and is solid: the clamp semantics (upper only, no floor), AddHP =
set-to-current+N, the packet-88 broadcast path, the silent failure legs, and the refutation of a plausible
reading that would have shipped a bug.

## WHY the subject cannot be ported yet - and it is the SAME wall as the array natives (item 69)

`artifacts/F3.json` says `JE::Entity` embeds **`JE::DynamicObject` at +0x0 with size 984 (0x3D8)**. So the
offsets this file has been arguing about - +0x1DC (the handle, T-043) and +0x1E0 (the 1/2/3/6/7-tested field) -
are both *inside the DynamicObject span*, and `Entity`'s own fields start after 0x3D8. SetHP's chain is:

    mov edx,[entity+4]      ; slot 4 of the object = the SECOND vftptr/vbtable (DynamicObject's +0 is vftptr)
    mov edx,[edx+4]         ; that table's slot 4 = a displacement, if it is a vbtable
    mov eax,[edx+entity+1E0h]  ; the field at (virtual-base-adjusted entity) + 0x1E0

That is a **layout-relative computation**: it is only interpretable if the object's real subobject geometry is
known - which vftable sits at +4, what its slot 4 holds, and what the virtual-base displacement is. Two
consequences, and they are the honest end of this thread:

1. The enum reading and the handle reading are NOT both about the same addressable slot: `entity+0x1E0` (what
   the two virtuals compare against 1/2/3/6/7) and `*(vbtable+4) + entity + 0x1E0` coincide only if the
   displacement is 0. Assuming 0 is exactly the kind of "plausible reading" that ships a wrong-entity write.
2. The port structurally CANNOT express this today, for the reason item 69 documents: the port's generated
   headers are name-sorted excerpts (`offsetof(GameCreature, mbr_0xbc) = 0x4B4` where shipped is 0xBC,
   `sizeof` 1252 where shipped is 3036), and they carry no vbtable geometry at all - no subobject spans, no
   displacement slots. There is no expression in the port that means "the +0x1E0 field of the virtual base".

So SetHP/AddHP and the three array natives are blocked by ONE prerequisite, not two: **the port needs a
shipped-ordered GameEntity/GameCreature layout with real subobject extents (and array members)**, after which
the array reads are `obj->perks[i]` and the setter subject becomes an expressible, checkable thing. That is a
header-owner lane decision with a blast radius beyond this batch, so it is filed as the batch's dependency
rather than taken unilaterally inside a natives TU - and it is why 5 of the remaining batch-B rows are parked
on a single, nameable piece of work instead of on five separate mysteries.

Minimum viable version, if the layout decision is deferred: give `GameCreature`/`Entity` the two arrays as
sized `int[]` members (perk 0x114 band, base-skill 0xE0 band - lengths still unknown and NOT to be guessed) and
a `vbnested` accessor note; that unblocks the three array natives without pretending to model virtual bases, and
leaves SetHP/AddHP honestly waiting.

## B5 (named probe executed): `virt_meth_0x54dea0` confirms the enum AND shows what it drives - two pseudocode ints are FLOATS

The probe this file asked for ran. `JE::Entity::virt_meth_0x54dea0` (0x54dea0, 0x106 bytes) reads the same
`Entity+0x1E0` slot at 0x54df73 and compares it against **5**, choosing between two calls:

    sub_563D30(handle, a2, 0, 1, 1029785518, -65536)     ; when +0x1E0 == 5 and *(entity+0x368) != 1
    sub_563D30(handle, a2, 0, 0, 1028443341, -16711681)  ; otherwise

Decoding the two "integer" literals is the actual news, because the decompiler prints them as ints:

    1029785518 = 0x3D6147AE -> float 0.055f
    1028443341 = 0x3D4CCCCD -> float 0.05f
       -65536  = 0xFFFF0000 ; color-shaped (opaque red in ARGB order)
   -16711681   = 0xFF00FFFF ; color-shaped (opaque cyan/yellow-ish depending on channel order)

So this is an **overlay/marker draw with a size and a colour**, and the entity's +0x1E0 mode decides which of
the two: mode 5 gets 0.055 and one colour, everything else 0.05 and another. That is consistent with everything
else measured here - a small-integer STATE/MODE field (now seen tested against **1, 2, 3, 5, 6, 7** across four
sites), serialized by `UnpackState_54e690`, zero-initialized in `ctor_0x5513a0`, and driving presentation
behaviour rather than identity. It reinforces, decisively, that **`SetHP`'s `+0x1E0` operand cannot be the
entity's own handle**, and that the `+4/+4` chain must be putting a different object in the base register.

Caveat kept on purpose: the float reading of 0x3D4CCCCD/0x3D6147AE is textbook float-bits-as-DWORD rendering and
the colour shapes are unambiguous, but this sitting did not open `sub_563D30` to confirm the parameter types.
The claim is therefore "these are almost certainly (size, colour) - verify in sub_563D30 before quoting the
0.05/0.055 values in product code". The MODE conclusion does not depend on it.

Also visible in this function, for whoever owns the map/octree lane: the body resolves a Map through
`sub_401130()` + `[mbr_0x0 + 84](v5)` with `v5 = *(entity+0x22C)`, then `JE::Map::GetOctree_56d960`, and feeds
the octree to `Entity::meth_0x568140` - with a `meth_0x54dc50(string at entity+0x21C, 0)` call before it and a
`LOBYTE(entity+0x164)` one-shot latch around the whole thing. That is a fresh, cited path from the entity's
string-ish band (+0x21C/+0x22C/+0x230, SSO-shaped: capacity test `< 0x10` at 0x54dedd) into map/octree
attachment - noted here because it turned up under a probe aimed at something else, and it should not be lost.

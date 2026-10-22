# The string-taking natives all funnel through one four-leg fetch (`sub_59AB20`) - and what the port must decide to take them

Discovered while sizing the inventory era, because that era is the cheapest multi-row unblock in batch B
(four rows behind one container) and the fetch is the trunk: `IsItemEquipped` (0x45b7cc) calls
`sub_59AB20(amx, params[2], buf, 0x80)` and every other string-taking native in the batch uses the same helper
with the same 128-byte destination.

## What it is, leg by leg

    bool __cdecl sub_59AB20(amx, int cellOffset, BYTE *dst, unsigned bufSize)      ; 0x59ab20
        return !sub_5CE350(amx, cellOffset, &ptr)          ; 1. amx_GetAddr  - cell -> DAT pointer
            && !sub_5CE3F0(ptr, &len)                     ; 2. amx_StrLen   - encoded length
            && len < bufSize                              ; 3. bound check  - against the CALLER's buffer
            && sub_5CE510(dst, ptr) == 0;                 ; 4. amx_GetString - decode into dst

That is the stock pawn `amx_StrParam` shape: a cell index into the script's data segment, resolved to a host
pointer, measured, bounds-checked, then decoded. Four facts fall out of it:

- **The bound is the caller's, not the fetch's.** `param_4` is the destination capacity (0x80 at the
  `IsItemEquipped` site), and `len < bufSize` is a strict less-than - so a string that exactly fills the
  buffer is REJECTED, not truncated. Any port that copies with `strncpy(buf, s, size)` and a null terminator
  silently accepts a case the engine refuses, and the caller then reports "unable to find the Item Name." for
  a name that was merely 128 bytes long.
- **`dst` is written only on success.** All four legs must pass, so a failed fetch leaves the caller's buffer
  whatever it was. `IsItemEquipped` zeroes nothing first (128 bytes at `v7`, 0x45b7c4-ish), so a failed fetch
  plus a code path that consulted the buffer would read stack garbage - the shipped order (test the return,
  report, return 0) is the only safe reading and it is worth asserting rather than assuming.
- **The failure has no message of its own.** The fetch returns a bool; every "unable to find the Item Name."
  style string belongs to the CALLER. So the error text is per-native and the arity/name gate trio stays as
  documented in `docs/f3/CreatureHpWriters.md`.
- **Leg 4 decodes an encoded cell string**, so a port that treats `params[2]` as a char pointer is wrong by
  construction; the cell is an INDEX into script data, which is exactly the thing the port currently has no
  model for.

## The decision the inventory era needs before any of its four rows can land

The port's script layer is a stand-in, not a VM: `JE::Scripting.cpp` keeps a name-keyed native registry and
says so. So there is no DAT segment to point `sub_5CE350` at, which means every string-taking native has to
pick one of two shapes:

1. **Model the AMX data image** (a byte buffer + cell addressing + the encode/decode of legs 2 and 4). Faithful,
   and it makes `len < bufSize` reachable in tests - but it is a VM, which the stand-in exists to avoid, and it
   buys nothing for a native that only ever wants a name.
2. **A port-side fetch seam that takes a plain C string** at the position `params[2]` would name, keeping the
   shipped OBSERVABLES: bool success, the strict `len < bufSize` rejection, and "buffer untouched on failure".
   Cheaper, testable, and honest if named - the same trade already made for the entity-handle registry in
   `src/F3/ScriptNatives.cpp` (a documented stand-in for the `byte_70BE0C` walk, with the shipped failure
   behavior preserved rather than tidied away).

Recommendation: **option 2, with the bound check kept as a real assertion**, because the strict less-than is the
one genuinely surprising semantics here and a port that "just copies the string" loses it silently. Option 1 is
the right call only if/when a real `.int` has to run, which is its own row and not a side effect of porting a
name lookup.

## Rows waiting behind it, from batch B

`IsItemEquipped` (0x45b750) - fetch, then the inventory chain; `EquipItem` (0x45a580) and `UnequipItem`
(0x45a6f0) - unread bodies, both expected to take a name; `GetWeaponRange` (0x45b620) - does NOT need it (its
inputs are handles only), which is why the era's four rows share a trunk but not a blocker. Route by
call-shape, per the gate-trio router: 0x59AF40 / 0x59AEF0 / 0x59AD20 within the first bytes means script native.

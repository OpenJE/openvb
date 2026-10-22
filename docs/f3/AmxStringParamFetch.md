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

## The three legs, read from the image (B15) - and what they ask the port to become

`sub_59AB20` delegates to three functions, each now read whole. They are stock Pawn/AMX, and together they
define the size of the thing the port would have to grow to be option 1.

**Leg 1 - `sub_5CE350` (amx_GetAddr)** (0x5ce350). A bounds test and an address computation:

    if ( offset >= amx[6] && offset < amx[8] || offset < 0 || offset >= amx[9] ) return 5;   // 0x5ce36b
    *out = offset + *amx + (*amx + 16);                                                      // 0x5ce378

so the cell offset is validated against three fields of the AMX struct (their meanings are the AMX
runtime's, not re-derived here - cited as `[6]`,`[8]`,`[9]` and left unverified on purpose) and rejected with
error code 5, then translated by a base plus a header-size term (`*amx + 16`). **The failure mode is an error
CODE, not a null pointer** - which is why the fetch's boolean comes out of an `&&` chain and why callers print
their own message.

**Leg 2 - `sub_5CE3F0` (amx_StrLen)** (0x5ce3f0). Branches on the first cell's magnitude:

- `*param_1 <= 0xFF` -> PACKED: walk 4-byte groups counting `while (data[4*n+4])`, length in BYTES.
- else -> plain: `strlen`, and then, unless `byte_7004BC` is clear, a tail adjustment that walks forward by
  whole bytes while `(word & 0xFF000000) != 0` (0x5ce430-0x5ce43a) - i.e. it accounts for padding bytes in the
  final cell.

**Leg 3 - `sub_5CE510` (amx_GetString)** (0x5ce510). Same predicate, two decoders:

- packed (`<= 0xFF`): emit the low byte of each successive cell (`*v8++ = *v2; v2 += 4`) until a cell is zero,
  then a terminator - pawn's 4-chars-per-cell packing, one byte per cell little-endian.
- plain: pull bytes OUT of each cell in descending order - `v5` cycles `3,2,1,0` via `(v5-1)&3` and the byte is
  `*v2 >> (8*v5)` - so the arrangement is cell-swapped relative to the packed branch.

## Why this settles the seam decision rather than complicating it

To be option 1 the port would need: an AMX struct with a data segment and three bound fields, a base-plus-header
address translation, error codes, the packed-vs-plain predicate, two different byte orderings, and a padding
adjustment gated on a global (`byte_7004BC`). That is not a string helper; that is the VM's data model, and the
port says plainly that it does not have one.

Option 2 - a fetch seam taking a plain C string while preserving the shipped OBSERVABLES - stays the
recommendation, with the observable list now sharper, because the legs show exactly which failures are
reachable and which are just the VM's internals:

1. an out-of-range cell offset fails with error 5 (leg 1) -> caller prints its own line and returns 0;
2. a string whose length is `>= bufSize` fails (the strict `len < bufSize` in `sub_59AB20`) -> same;
3. a decode failure (leg 3 returns non-zero - it never does in these bodies, both branches `return 0`) is
   therefore UNREACHABLE as a separate outcome; only legs 1 and 2 can fail.

That third point is a small but real fidelity fact: `sub_59AB20`'s `&& sub_5CE510(...) == 0` clause can never
be the reason a fetch fails, because `amx_GetString` returns 0 on every path. Writing the seam to model a
"decode error" would be modelling something shipped cannot do - the kind of defensive invention this batch has
been systematically removing.

# The two lazily-built 0x244 element arrays

Two accessor/allocator pairs in the shipped engine build an array of 0x244-byte elements on first use and
index it. They look interchangeable and are not: one family's elements start **0xC bytes into** the
object it allocates, the other's start at its base. Getting that backwards silently misindexes every
element by 12 bytes, so both are written down here with their addresses before either is ported.

Everything below is read from the original at the addresses given; `cls_0x49cc40.cpp` still declares these
methods without bodies, and this note is what a port will need.

## Family A - `sub_50E300` (index) + `sub_50E290` (lazy alloc), elements at base+0xC

    sub_50E300(this, index):                       ; 12 instructions, retn 4
      50e303  mov  eax, [esi+54h]
      50e306  test eax, eax
      50e308  jnz  short loc_50E30F                ; already allocated
      50e30a  call sub_50E290                      ; ecx = this
      50e30f  mov  eax, [esp+4+param_1]            ; index
      50e313  mov  ecx, [esi+54h]                  ; the wrapper object
      50e316  imul eax, 244h
      50e31c  add  eax, [ecx+0Ch]                  ; <-- elements begin at wrapper+0xC
      50e320  retn 4

`sub_50E290` (42 instructions) is the lazy allocator, and its shape shows what the wrapper is:

    50e2aa  mov  edi, [esi+28h]                     ; this+0x28 = source/owner
    50e2ad  push 10h / call operator new(uint)      ; a 0x10-byte object
    50e2c7  add  edi, 28h / push edi                ; ctor argument = (this+0x28) + 0x28
    50e2cd  call cls_0x4abc60::ctor_0x4abd40
    50e2e0  mov  [esi+54h], eax                     ; publish the wrapper
    50e2e3  call cls_0x4abc60::meth_0x4abdd0        ; wrapper+0xC is set up by this call
    50e2ef  mov  [esi+2Ch], eax                     ; this+0x2C mirrors the wrapper pointer

So: `[this+0x54]` is a 0x10-byte `cls_0x4abc60` wrapper whose elements live at `wrapper+0xC`, and
`[this+0x2C]` is a second reference to the same wrapper. The element *count* is not read by the accessor
at all - callers are trusted, which is worth remembering when porting: there is no bounds check to
reproduce because there is none.

## Family B - `sub_509470` (index) + `sub_508F80` (rebuild), elements at base+0

    sub_509470(this, index):                       ; 18 instructions, retn 4
      509473  mov  eax, [esi+70h]
      509476  test eax, eax
      509478  jnz  short loc_50948F
      50947a  call sub_508F80                      ; ecx = this
      50947f  mov  eax, [esi+78h] / push eax       ; arg for sub_5093B0
      509482  mov  ecx, [esi+70h] / push ecx
      509487  mov  ecx, [esi+24h] / call sub_5093B0
      50948f  mov  eax, [esp+4+param_1]
      509493  mov  ecx, [esi+70h]
      509496  imul eax, 244h
      50949c  add  eax, ecx                        ; <-- elements begin AT the pointer
      50949f  retn 4

`sub_508F80` (81 instructions) rebuilds the array from the count at `[this+0x68]`:

- if `[this+0x70]` is non-zero it frees the old block: the allocation carries a **one-dword header holding
  the count** (`mov ecx, [eax-4]`), destroyed with `eh vector destructor iterator` (elem 0x244, dtor
  `cls_0x4b60b0::meth_0x401720`) and then `free(header)`, leaving `[this+0x70] = 0`;
- it allocates `4 + [this+0x68] * 0x244`, writes the count into the header dword, and constructs the
  elements with `eh vector constructor iterator` (ctor `cls_0x4b60b0::ctor_0x4b60b0`, dtor as above);
- then, for `i` in `0 .. [this+0x68]`: `cls_0x50db20::ctor_0x41fc60(&elements[i], [[this+0x24]+0x5C] + i)`
  - i.e. each element is initialised from an index-parallel record owned by the manager at `[this+0x24]`.

Both are arrays of `cls_0x4b60b0` at stride 0x244 (proof below).

Note the asymmetry against Family A: **no `+0xC`**, and the array carries its own length in a hidden
header. A port that unifies the two accessors would be wrong in one of the two places.

## Who calls them

`cls_0x49cc40::meth_0x49cc40` (`0x49cc40`, 18 instructions) is a forwarder that reaches both families on
two different sub-objects of its own owner:

    owner = [ecx+6Ch]  -> past this, sub_50E300(owner, arg)     (Family A)
    owner = [ecx+70h]  -> past this, sub_509470(owner, arg)     (Family B)

and `cls_0x49cc40::meth_0x49db50` (`0x49db50`) is `mov ecx,[ecx+48h]; push arg; call sub_49D610` - a
one-line forwarder to a third seam that is also still unported.

The forwarder body read whole (`0x49cc40`, 18 instructions, `retn 4`) settles three points a port must not
guess:

    49cc46  xor  eax, eax            ; the RESULT for the "neither pointer set" path
    49cc48  test ecx, ecx / jz       ; [ecx+6Ch] null -> skip Family A entirely
    49cc51  push edi / call sub_50E300
    49cc57  mov  ecx, [esi+70h]
    49cc5a  test ecx, ecx / jz       ; [ecx+70h] null -> skip Family B entirely
    49cc5e  push edi / call sub_509470
    49cc66  retn 4                   ; returns whatever the LAST call left in eax

- the two pointers are tested **independently**: neither being set is legal, and then the result is 0;
- the return value is **not** the first family's element - it is whatever the last call left behind, so a
  call where only Family A runs returns A's element and a call where both run returns **B's**;
- neither call's result is stored, so the function's only observable effect is the lazy allocation each
  accessor may perform.

## The element type is `cls_0x4b60b0`, and that is already ported

`cls_0x49cc40::meth_0x4a0750` (`0x4a0750`) settles the one thing that looked unresolvable. It advances the
index while counting `[[this+0x44]]->meth_0x4a06e0()` and copies one field between the two arrays:

    4a0971  push edi / mov ecx, ebp / call meth_0x49cc40      ; element of THIS object's array
    4a0979  push edi / mov ecx, esi / call meth_0x49cc40      ; element of `esi`'s array
    4a098b  mov  ebx, [ebx+130h]                              ; A->+0x130
    4a0995  mov  [eax+130h], ebx                              ; B->+0x130 = A->+0x130

Two consequences:

1. **Both families yield the same element type**, one with a field at `+0x130`. Family B's own rebuild
   names its element lifecycle outright - `cls_0x4b60b0::ctor_0x4b60b0` and `meth_0x401720` in the
   `eh vector` iterators with stride 0x244 - and the port's `cls_0x4b60b0.hpp` already pins `sizeof` to
   **0x244** (F3's 577 is the untailed extent; the header documents that and the "alphabetical order"
   generator defect) and already declares `mbr_0x130` as the first of the two 4-dword arrays at
   `0x130..0x13F` and `0x140..0x14F`. So the element side needs no new work.
2. **The forwarder is a shared helper, not a `cls_0x49cc40` method in spirit**: this call site passes
   `this` (a `cls_0x49cc40`) and then `esi`, which came from `[this+0x44]->meth_0x4a06e0()` - a
   `cls_0x49caf0`-related object. One body, two unrelated owners, same element type.

What is still open is only the **container** class of `[+0x6C]` / `[+0x70]`, and that is the real blocker for
porting this: `sub_50E300` has exactly one caller (`0x49cc52`, this forwarder) and no F3 method entry, and
58 structures in F3 carry members at `+0x28`, `+0x2C` and `+0x54` - the shape Family A touches. Picking one
by shape would be a guess, so the port keeps the callee as a seam rather than inventing an owner type.

## What a port needs, in order

1. `cls_0x4abc60::ctor_0x4abd40` / `meth_0x4abdd0` (the 0x10-byte wrapper and its `+0xC` element base).
2. `cls_0x4b60b0` ctor/dtor (`0x4b60b0` / `0x401720`) as the element type's lifecycle.
3. `sub_50E290`, `sub_508F80`, `sub_5093B0`, `sub_49D610` with their lazy-init contract preserved: the
   accessors allocate on first use and do **not** re-check afterwards.

Until 1-3 exist, `tests/je/cls_0x49cc40/` can only pin the layout guards that are already there; adding an
"accessor" test that fakes the arrays would test the fake, not the shipped contract.

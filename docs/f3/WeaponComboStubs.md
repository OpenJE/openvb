# The weapon-combo pair — two natives that resolve and discard (batch B slice B23)

`F3::GetWeaponCombo` 0x45a9f0 and `F3::SetWeaponCombo` 0x45aa40, plus their shared callee
`sub_45B9F0` (0x45b9f0). Ported in `src/F3/ScriptNatives.cpp` with suite section 12.

These are the second shipped-stub pair in batch B (after PlayMusic), and they are a different kind of
stub: PlayMusic announces itself ("PlayMusic() unimplemented."), these do not. They look like working
accessors — names, a gate, a resolve, an error line — and every path returns 0.

## 1. The bodies, from instruction text

Both are the same 24-to-28-instruction shape:

| step | GetWeaponCombo | SetWeaponCombo | note |
|---|---|---|---|
| gate name | 0x45a9fb `"GetWeaponCombo"` | 0x45aa4b `"SetWeaponCombo"` | each its OWN name — no UnequipItem-style gate copy-paste (0x45a72c) |
| gate cells | 0x45aa00 `push 1` | 0x45aa50 `push 2` | **the pair's arities differ** |
| gate fail | 0x45aa0e `jz loc_45AA34` | 0x45aa5c `jz loc_45AA84` | jumps PAST the resolve: a malformed call never mentions entities |
| resolve | 0x45aa1a `call sub_45B9F0(amx, params[1], &local)` | 0x45aa6a same | the callee loads `[edi+4]` at 0x45aa60 — `params[1]` ONLY |
| on failure | 0x45aa2c `sub_59AD20("GetWeaponCombo() was unable to find the requested entity.")` | 0x45aa7c same with Set's name | strings @0x65e1a4 / @0x65e1f0, byte-exact and asserted |
| return | 0x45aa34 `xor eax,eax` | 0x45aa85 `xor eax,eax` | **every** path, including the one where the resolve succeeded |

The callee at 0x45b9f0 is the resolve-and-cast the pair shares:

    45ba02  call sub_59AEF0(amx, handle, &out, 1)     ; the ACTIVE-CHECK FLAG IS 1 (push 1, not 0)
    45ba23  call ___RTDynamicCast(local, 0, &Entity, &GameEntity, 0)
    45ba31  mov [edi],eax                             ; stores the cast result EVEN WHEN IT IS 0
    45ba33  ret                                       ; al = (cast != 0)

Two consequences the port keeps rather than smooths over:

- **A failed resolve leaves the out param untouched here** because 0x59aef0 already stored 0 into it
  before returning (that store is asserted in suite section 7), while a failed CAST stores 0 explicitly
  at 0x45ba31. Both then produce the same single message, so a "not a GameEntity" and a "no such entity"
  are indistinguishable to the script — asserted as one leg in section 12, because splitting them into
  two messages would be inventing behavior shipped does not have.
- **The active-check is ON for these two natives.** That is observable: an INACTIVE but registered
  entity emits `"Attempt to use inactive entity in script."` (0x59af12-era, the warn-and-continue guard)
  and does NOT emit the combo line, because the resolve returned 1. So the pair is distinguishable from
  the arity-1 getters that pass flag 0, and the suite pins it by asserting the warning appears — this was
  the leg that went red under falsification, which is why it is worth having.

## 2. The finding: a getter that gets nothing and a setter that sets nothing

- `GetWeaponCombo` resolves the entity into a stack slot and **never reads the slot**. Its return is 0 on
  every path, so any script logic that trusted a weapon-combo value has been reading a constant for the
  whole life of the build.
- `SetWeaponCombo` declares **two** cells and reads **one**: `mov ecx,[edi+4]` (0x45aa60) is the only
  cell load in the body, nothing loads `[edi+8]`, and the resolved value goes nowhere. The setter sets
  nothing.

The absence argument, stated as the method: this is established by reading the whole body
0x45aa40-0x45aa89 (28 instructions, ending at the single `xor eax,eax` return) and confirming no operand
references `+8` of the parameter block — not by the IDB failing to list a field, since a database listing
is an excerpt and an empty listing proves nothing (standing rule 7, and the rule this lane paid for in
B22's own census).

## 3. Port decisions

- Ported as exactly the shipped observable: gate, resolve (with flag 1), one message on failure, return 0.
  `(void)pTarget` marks the discard as intentional, mirroring shipped rather than hiding it.
- **Not** "fixed": no combo field is written, no resolved entity is returned. Doing either would delete
  the finding, and the suite now asserts the two are indistinguishable (`combo 3` and `combo 7` are the
  same call), so a future well-intentioned implementation goes red.
- Table 30 → 32, in the pair's own arities (1 and 2) — which is the sharpest statement of the oddity: two
  halves of one API, one of which takes an argument it never looks at.

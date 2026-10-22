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

    45ba02  call sub_59AEF0(amx, handle, &out, 1)     ; the ACTIVE-CHECK FLAG IS 1 (like 104 of the 105
                                                       ;  direct callers of this wrapper - see §1's census)
    45ba23  call ___RTDynamicCast(local, 0, &Entity, &GameEntity, 0)
    45ba31  mov [edi],eax                             ; stores the cast result EVEN WHEN IT IS 0
    45ba33  ret                                       ; al = (cast != 0)

Two consequences the port keeps rather than smooths over:

- **A failed resolve leaves the out param untouched here** because 0x59aef0 already stored 0 into it
  before returning (that store is asserted in suite section 7), while a failed CAST stores 0 explicitly
  at 0x45ba31. Both then produce the same single message, so a "not a GameEntity" and a "no such entity"
  are indistinguishable to the script — asserted as one leg in section 12, because splitting them into
  two messages would be inventing behavior shipped does not have.
- **The active-check is ON for these two natives** (flag 1 at 0x45b9f9/0x45ba02). That is observable: an
  INACTIVE but registered entity emits `"Attempt to use inactive entity in script."` (0x59af12-era, the
  warn-and-continue guard) and does NOT emit the combo line, because the resolve returned 1. The suite pins
  it by asserting the warning appears — the leg that went red under falsification, which is why it is here.

  **CORRECTION to the first draft of this bullet**, which said the pair "is distinguishable from the arity-1
  getters that pass flag 0". That contrast is FICTION and was falsified by NiceTiger's census, reproduced
  independently here from the PE's own `.text` (E8 rel32 scan for direct calls to the resolver wrapper
  0x59AEF0, taking the last `6A xx` push-imm8 within the preceding 24 bytes):

      105 direct call sites:   104 push 1,   1 pushes 0  ->  JE::ActivateEntity_0x5c9180 @ 0x5c91ac

  So essentially EVERY native that resolves an entity passes flag 1 — `GetMaxHP` pushes 1 at 0x45b133, as do
  SetHP/AddHP's preamble resolve calls at 0x45b314/0x45b3c4 — and the image's single flag-0 caller is
  `ActivateEntity`, whose entire job is FINDING inactive entities. The exception proves the rule rather than
  breaking it. Nothing shipped into the suite asserted the false contrast, so no test ever went red for the
  wrong reason; only this prose did.

  The distinction that IS real, and which the port already models correctly: natives resolving through a
  SERVER TAIL never touch the wrapper at all. `SetHP`/`AddHP`'s shared chain `meth_0x46ee10` calls the raw
  entity-list resolver `meth_0x54be10` DIRECTLY (0x46ee19), so there is no flag argument and no inactive
  warning on that path — null resolves to silent 0, a failed cast to silent 0. That is why the port's
  `HpSetterChain` passes 0 (`ScriptNatives.cpp`, the 0x46ee23 leg): a faithful mirror of "no wrapper here",
  not a competing policy. Recorded because "the port passes 1 in one place and 0 in another" otherwise reads
  as an inconsistency the port invented.

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

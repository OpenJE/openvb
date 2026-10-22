# Equip / Unequip / IsItemEquipped: one slot array, two case rules, a wildcard that lies, and three copy-pastes (B16-B18, 2026-10-22)

Three script natives reach the same inventory, and reading them against each other is what made the shape
clear. None of it is style: every rule below is a byte or a string address in the original image.

## The slot model

`GameInventory` keeps **six equipped cells at `+0x14`** - vft slot 10 (`0x46a860`) walks
`*(&this->mbr_0x14 + i*4) == candidate` under `while (i < 6)` (0x46a8cc), returns a `char`, and is the
predicate `IsItemEquipped` ultimately asks. So equippedness is **membership**, not a per-item flag.

`UnequipItem` then treats slot **7 as a sentinel meaning "everywhere"** (0x45a81d `if (v3 != 7)`): the named
path touches one cell, the slot-7 path runs the finder and clears every match. The port mirrors both, and an
out-of-range slot (9, -1) stores nowhere and answers 0.

## Two comparison rules in one native - and they are not interchangeable

| path | comparison | cite | consequence |
|---|---|---|---|
| `IsItemEquipped` / `UnequipItem` slot 7 | `_stricmp` | 0x46a777 | "HUNTING RIFLE" finds "Hunting Rifle" |
| `UnequipItem` named slot | `strcmp` | 0x45a8e7 | "HUNTING RIFLE" **refuses** to unequip it |

The same string can therefore be "equipped" by query and un-unequippable by name. Asserted as a pair so a
future unification is a visible change, not a cleanup. (Neither path is prefix-matching: querying `hat`
against items named `Hat A`/`Hat B` matches nothing - the first draft of this test assumed otherwise and the
suite said so.)

## The wildcard that succeeds and reports failure

`UnequipItem` accepts the literal `"*"` - global `byte_65DC00`, read out of the original image's `.rdata` as
`2A 00` - and compares it against the REQUESTED name at 0x45a932. Matching it clears the slot and then falls
through to `return 0`, while the exact-name branch returns 1. **Unequipping by wildcard therefore works and
reports failure.** Preserved verbatim; a port that returns 1 here is a divergence dressed as a fix.

## Three shipped copy-pastes, each kept wrong

1. `UnequipItem`'s arity gate is registered under the name **`EquipItem`** (0x45a72c), so a wrong-arity call on
   Unequip complains about Equip: `"Invalid number of paramters passed to function EquipItem()."`.
2. `UnequipItem` reports `"UnequipItem() was unable to find the attacking entity."` - and
   `EquipItem` uses the same **"attacking entity"** wording (0x45a5dc/0x45a750). That is Attack's vocabulary in
   two inventory natives.
3. `GetPerkRank` prints `"GetPosition() was unable to find the requested entity."` (0x45ab8e). Same class of
   defect, different family - recorded in `CreatureSkillAndPerkArrays.md`.

## Why `EquipItem` is NOT ported, named rather than parked

Its payload is `GameInventory::Equip` (0x46be40, reached through `[edx+38h]` at 0x45a6ce), and that body is an
era, not a store: RB-tree bookkeeping for the moved item, a **style-slot remap** over cases 2/3/7 with
`Get_mbr_0x98` weapon-style reads (0x46bff4-0x46c051), `ApplyEquipEffects` gated on the server-context bit
`global_area_map_icons[0]->Get_mbr_0x4 & 1` (0x46c1a1), a `SetWeaponStance(0)` reset (0x46c1e9), and its own
log line - `"GameInventory::Equip() - Item that was already equipped trying to equip itself again."` Porting
it means modelling effects and stance; `UnequipItem` was portable alone because the two share only the
preamble.

## The test-seam lesson from landing this

The message capture (`ScriptNativeGetLastError`) was changed from last-only to accumulating so the arity
gate's TWO ordered reports (0x59af63, then 0x59af70) are both assertable - and the first version of that
change silently **stopped capturing once the 1028-byte buffer filled**, producing 10 failures downstream that
looked like the natives had stopped reporting. Now it restarts the buffer on overflow, because a capture that
goes dead is worse than one that forgets: it is indistinguishable from correct-but-absent behavior. Related:
one assert failed because it reused a handle registered against a **block-local object that had already gone
out of scope** - reading destroyed storage and calling the result "the class check" is noise a suite should not
generate, so that leg now registers a fresh static item.

## GetWeaponRange (0x45b620) landed on the same model - a two-hop read, two bases, and one crash I chose not to reproduce (B19)

Arity 1, handle only, no string fetched. The interesting part is what it takes to get a number:

    0x45b6af   mov eax,[esi+168h]        ; inventory, PRE-CAST base   (same accessor as IsItemEquipped)
    0x45b702   call 0x463b60             ; slot = creature+0x98        (a one-instruction getter)
    0x45b70a   call 0x4693d0             ; item at that slot, RTTI'd to GameItem, 0 if not
    0x45b727   mov eax,[eax+408h]        ; the WEAPON's range POINTER   - no null test on the cast result
    0x45b732   mov edi,[edi+6F4h]        ; FALLBACK pointer, POST-CAST base (creature+0x6F4)
    0x45b740   mov eax,[eax+2Ch]         ; ...and only here is the actual range read

Three consequences, all now in the port:

1. **The range is two dereferences deep.** A weapon does not hold a range; it holds a pointer
   (`+0x408`) to a record whose `+0x2C` holds the value. The suite proves this by putting 42 at index 11 and
   999 at index 5 in the same record - reading either offset by mistake gives a different, wrong number.
2. **The native mixes bases deliberately.** The inventory comes from the pre-cast entity (`+0x168`, shared
   with `IsItemEquipped` and `EquipItem`), while the fallback pointer comes from the post-cast creature
   (`+0x6F4`). Two different eras of layout in one expression: `+0x168` is Entity-era, `+0x6F4` is
   GameCreature's own. In the port both objects share an address, so this cannot be asserted - stated here
   rather than faked with a test that could not distinguish it anyway.
3. **"Range 0" and "no range record" are the same answer**, exactly like `IsLocked`'s unlocked-vs-wrong-class
   problem. A record that exists and holds 0 returns 0; so does no weapon and no fallback. Asserted on both
   legs so the port does not quietly invent a distinguishing value.

**The landmine.** When the item occupying the slot is NOT a `GameWeapon`, shipped computes the RTTI result and
dereferences it at `+0x408` with no test (0x45b727) - a null dereference in the engine. The port returns 0 in
that case. That is a **named deviation**, not a fix: reproducing a crash is not a behavior worth copying, and
documenting the difference is what keeps it from being mistaken for agreement.

**Second named deviation, same shape.** `GameInventory::meth_0x4693d0` (item at slot) reads
`*(this + 0x14 + slot*4)` with **no bound check at all** (0x4693d4), even though the equippedness scan is
strictly six cells (0x46a8cc). So shipped will happily inspect whatever follows the array for slot 6, 7, or 9.
The port bounds the lookup and answers "no item". An unbounded read of neighboring port members would be a
different and far less reproducible wrong answer, so the bound is chosen and declared.

## Slot 14 and slot 15 - a unit flip that would have moved the function

Worth keeping as a census rule rather than a footnote: the two payload calls in this family are **different
virtual slots**, and the decompiler rendered them in different units.

| site | pseudocode | bytes | real slot |
|---|---|---|---|
| EquipItem's equip | `(*(*v5 + 56))` | 0x45a6ce `ff 52 38` -> `[edx+38h]` | **14** |
| UnequipItem's unequip | `(*(vft + 15))` | 0x45a918 `ff 52 3c` -> `[edx+3Ch]` | **15** |

The pseudocode was truthful in the first row and cell-indexed in the second. A reader who took `+15` as bytes
would probe `[edx+0Fh]` - slot 3, mid-table nonsense that looks like a corrupted vftable rather than a
rendering artifact. **Unit flips in pseudocode can move the function, not just misstate an offset**, so vft
citations quote the call bytes and never the printed addend.

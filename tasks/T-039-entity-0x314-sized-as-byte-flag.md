---
status: open
assignee: HappyKnight — this is T-011 step 3's gate operand, so it is in front of that code, not beside it
depends: T-011 (the dispatch gate), T-034 R3 (the confirming read)
---

# T-039 — `Entity+0x314` is a 4-byte field in the shipped layout; our port models it as a byte and uses it as a flag

## Evidence (all offline, no debugger)
1. **Database:** `f3_member_index Entity 0x314` → `mbr_0x314`, **`size: 4`**, with **seven recorded usages**:
   `0x4084c0, 0x54ab30, 0x54ab80, 0x54c447, 0x54c4a0, 0x54c788, 0x5515e6`.
2. **Our port:** `src/F3/MapEntitySpawn.hpp:68` declares `unsigned char busy0x314;` with the comment
   *"Entity+0x314: second gate term (0x5A95BF read as +788)"*, initialised to 0 at `:38`, and it is one of the three
   fields printed in every `SimDigest` `st=` triple.
3. **Structure overlap** (RedXenon, T-034 pre-resolution at `44544f0`): the shipped action deque lives at `Entity+0x304`
   (`cls_0x54e070::meth_0x54ea00`: 4 pointers per 0x10-byte node, `slot = count & 3`, `block = count >> 2`), and its
   **element index resolves to `+0x314`** (`base + 0x10`).

## Why this is not a nitpick
T-011 step 3 implements the turn gate `+0x1E4 == 0 || +0x314 != 0`. If `+0x314` is the **queued-action count**, that
condition is not *"the entity is busy"* — it is **"the entity has actions in its queue"**, and the two readings produce
different dispatch behaviour whenever a turn is dispatched while a re-queue is pending. An entity whose queue holds two
orders is, under the counter reading, *not* idle — our byte model cannot represent that at all (it collapses everything
non-zero to 1, and a real count of 256 would alias to 0 in one byte).

Do **not** code the gate before this is settled. Two candidate readings, both currently unfalsified:
- **(A) queue depth** — supported by the size-4 typing, the seven-usage spread, and the structural overlap at `+0x314`;
- **(B) a genuine busy/latch dword** — supported by the fact that T-010's protocol reading used it as a gate term, and
  nothing yet proves which value the writer stores.

## Scope
1. Read the writer sites, not just the readers: `0x54ab30` and `0x54ab80` are the pair to start on (adjacent EAs, both
   recorded as `mbr_0x314` usages) — is it `++`/`+=` (counter), or `mov reg, 0/1` (flag)?
2. Check whether `+0x304..+0x313` is modelled contiguously with `+0x314` in the DB (deque fields) or separately.
3. Re-size our field to `unsigned int` regardless of the outcome — **the type is wrong even if the name is right** —
   and re-check every place the digest prints it.
4. If (A) holds, `busy0x314` gets renamed (e.g. `queuedActionCount0x314`) and T-011's gate comment must cite the writer
   EA, not the reader.

## Acceptance
- A cited verdict: writer form (increment vs constant store) with the EA, plus the DB size/usage evidence above.
- Our field's declared type matches the shipped size (4), with the rename and digest-label correction if it is a count.
- **T-034's R3 is the confirming sample**: the live index value mid-round. Report it as *consistent with* or
  *contradicting* (A) — a single sample cannot prove capacity, and cannot prove semantics either.

## Boundary
This is a claim about **the shipped layout**, and about one field. It does not say the port is broken end-to-end — it
says a boolean read of what may be a counter, in a gate we are about to implement. If the writers turn out to store 0/1
constants, (A) is dead, the name stands, and only the type changes: that is a perfectly good outcome and must be
reported as such rather than as a non-result.

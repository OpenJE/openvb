---
status: done (statics settled it; T-034 R3 remains the confirming live sample, NOT a prerequisite)
assignee: HappyKnight (01a15be7)
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

## VERDICT (2026-10-21, HappyKnight): **(A) it is a COUNT** — the writers are `++`/`--`, never 0/1

The row said "look at the writers, not the readers". Correction worth recording: **`0x54ab30` and
`0x54ab80` are READERS** — both sit in gate predicates (`virt_meth_0x54ab00`, `virt_meth_0x54ab50`) and
test the field, they do not write it. The writers are reached *through the container*, which is why
`f3_member_index Entity 0x314` did not name them directly: `Entity+0x314` is `mbr_0x10` of the 20-byte
deque at `Entity+0x304` (`0x304 + 0x10 = 0x314`), and the DB attributes those stores to
`cls_0x54e070::mbr_0x10` / `cls_0x54b5c0::mbr_0x10`:

| EA | instruction form | whose |
|---|---|---|
| `0x54ea71` | **`++this->mbr_0x10`** | `cls_0x54e070::meth_0x54ea00` — the push_back that the funnel `0x55072e` calls |
| `0x54b637`, `0x54b672` | **`--count`, store back** | `cls_0x54b5c0::meth_0x54b5c0` — the erase/rotate that walks the deque |
| `0x5515e6` | `= 0` | `Entity::ctor_0x5513a0` |

No `mov reg, 0/1` writer exists in the usage set, so the flag reading **(B) is dead** — and the
structural cross-check the coordinator asked for holds: `mbr_0x308` (block-array pointer), `mbr_0x30c`
(block capacity), `mbr_0x310` (head index), `mbr_0x314` (live count) are all `size: 4` and all resolve to
the same container, so the count reading is structural, not a naming preference.

The readers say it too: **`Entity::virt_meth_0x4084c0` is one line — `return Entity+0x314 == 0`** (an
"is my queue empty" predicate), and the pump `virt_meth_0x54c570` / `meth_0x54c440` bound their action
loops by `head != head + count` (0x54c77c/0x54c788, 0x54c447/0x54c4a0). Also: the gate at 0x54ab30 tests
`mbr_0x2f0 || mbr_0x314` — two counts of two same-shape deques (`Entity+0x2E0` and `Entity+0x304`), i.e.
"anything queued in either list".

**Consequence, stated where it costs something:** `+0x1E4 == 0 || +0x314 != 0` (0x5a95c1) means "…or the
entity **has queued actions**". A queued order therefore FORCES the turn even when the first term is
false — a different dispatch behaviour from "is busy", exactly as predicted. The byte model could not
represent it (256 aliases to 0 = *idle*).

**`+0x1E4` came out of the same sweep and its name was the lie, not its use:** DB `mbr_0x1e4` size 4,
ctor `= 0` at `0x55147b`, unpacked from the entity record at `0x54e739`, and its readers gate
**map-membership** (`0x54bc3a` guards `--*(oldMap+280)` / `++*(newMap+280)`; `0x54947e` bails out of a
movement update when it is 0). So `noTurn0x1E4` asserted a meaning the evidence does not support: it is
now `flag0x1E4`, `unsigned int`. **What it means is still an open question** — T-011 may mirror the test
at 0x5a95c1, but must not rename it after a guess.

## Landed

- `src/F3/MapEntitySpawn.hpp`: `busy0x314` **removed**; the count is derived via
  `SpawnedEntity::QueuedActionCount()` from `actions[]`, with the writer EAs cited in the field comment.
  A stored counter would have to be updated at every push *and* at each of the five auto-clear sites in
  `MapEntitySpawn.cpp`, i.e. it could only drift from the slots it claims to count.
- `noTurn0x1E4` → `flag0x1E4`, `unsigned char` → `unsigned int`.
- `src/F3/SimDigest.hpp`: the depth moved OUT of the `st=` triple into its own hashed field **`q=`**
  inside the same `eN` token, so a divergence reads as `e0.q` and the header line states what `q` is and
  why its cap is 4. **This makes deviation D-2 visible in a parity diff:** shipped `q>4` beside our
  `q=4` is a dropped order, not a digest artifact.
- Tests in `modules/openje/tests/je/PathManager/PathManager.cpp`: depth 1/2/4 from slot occupancy, the
  digest prints `,q=4,` then `,q=3,` after a drain, `st=` is three values (a fourth means the busy byte
  crept back), and 4-vs-3 with everything else equal must **change `fnv`** — so a future edit that drops
  `q` from the hashed span fails instead of silently under-reporting a queue change.
- `docs/f3/EntityActionQueueSeam.md` (the verdict + the container map), `docs/f3/SimDigest.md`,
  `src/F3/CombatManager.hpp` (the gate comment now cites the WRITER EAs, per scope item 4).

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

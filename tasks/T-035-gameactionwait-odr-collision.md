---
status: done
assignee: RedXenon (01a157ec) — granted; they surfaced it while building the driver
depends: nothing
---

# T-035 — Two different `F3::GameActionWait` definitions (latent ODR collision)

## Verified state (coordinator, this turn)
`src/F3/GameActionWait.hpp` and `src/F3/GameActionMoveTo.hpp` each define **`F3::GameActionWait`**, differently:

| file | definition | included by |
|---|---|---|
| `GameActionWait.hpp:10` | `struct GameActionWait : public JE::ActionWait` — the **DB/shipped-layout** model (`mbr_0x14` …) | **nobody** (grep across `src tools modules`: 0 files) |
| `GameActionMoveTo.hpp:83` | `struct GameActionWait { … }` — the **pump-side** model, no base class | `GameActionMoveTo.cpp`, `MapEntitySpawn.cpp`, `tools/sim-driver.cpp`, `tests/…/SpawnMove.cpp` |

So today it is *dormant*, not broken: only one definition ever reaches a TU. `MapEntitySpawn.cpp:227` already
announces it — `case 2: // F3::GameActionWait (the pump-side model, NOT the DB-layout header struct)`.

**Why it still has to be fixed now:** the moment a lane includes both headers — which is exactly what happens when
someone tries to make the pump model match the shipped layout — they get one name with two layouts. The compile may
succeed, mingw may say nothing, and the `static_cast<F3::GameActionWait*>(action)` sites in the drain will read fields
at the wrong offsets. That is the item-22 failure mode (`+0x14` is a *shipped* offset) wearing a header's clothing.

## Ruling: keep both, name them distinctly — do NOT delete the shipped-layout struct
Deleting the unused header would destroy the only in-tree record of the original's layout, which is the artifact we
most need when fidelity work closes the gap. So:

1. Rename the DB-layout struct to **`F3::GameActionWaitShipped`** (or move it into a `namespace shipped`) inside
   `GameActionWait.hpp`, with a header comment citing the DB source and stating that it is documentation-grade layout,
   not a live type.
2. Leave the pump model's name alone — four files cast through it.
3. Add a comment at each definition pointing at the other, and at `MapEntitySpawn.cpp:227`.

## Acceptance
- **One TU includes both headers and compiles** — that is the collision test, and it must be shown failing before the
  rename and passing after (rule 2).
- **`msvc-build` exit 0**, not just the host suites: this is a header change, and include-order defects are invisible
  to mingw (harness item 18 / `docs/DEFERRED-NOTES.md` — an inline accessor passed mingw and failed MSVC with C2027).
- `run-all-tests` stays 10 pass / 0 fail with floors intact, quoted with its `GATE STAMP` after
  `tools/gate-integrity.sh` exits 0.
- The rename is comment-and-name only: **no field offsets change**, and the commit states which struct is documentation.

## Sibling survey — is this a singleton? (no: 7 name collisions found, 2 real)

Grepped every `struct|class` name defined in more than one header across `src/F3` + `modules/openje`:

| name | headers | verdict |
|---|---|---|
| `F3::GameActionWait` | `GameActionWait.hpp` (shipped layout) vs `GameActionMoveTo.hpp` (pump model) | **REAL — fixed by RedXenon (`2f849d3`)** |
| `F3::GameActionDie` | `GameActionDie.hpp` (shipped layout, 0 includers) vs `GameActionAttack.hpp:64` (pump model, cast by the drain) | **REAL — fixed here**, same rename (`GameActionDieShipped`), same red/green proof |
| `F3::GameActionOperateDoor` | `GameActionOperateDoor.hpp` (F3) vs `MapEntitySpawn.hpp` (**JE**) | false alarm — different namespaces, cannot collide |
| `JE::Client`, `JE::Server`, `JE::net::client`, `JE::net::server`, `JE::exception` | `JE/X.hpp` vs `JE/Y/X.hpp` | false alarm — the nested path is the nested namespace (`JE::Server::Client` etc.) |

**Method note that generalises:** a duplicate name is only a landmine if the **namespaces match**. Five of the seven
looked alarming and are not; the two that were real share an exact signature — a shipped-layout header with **zero
includers** sitting beside a pump model that the drain actually casts through. That zero-includers pattern is the
search key for the next one, and it is worth re-running this scan whenever a new DB-layout header lands.

## Boundary
Renaming does not make the two models agree — it makes them *distinguishable*. Whether the pump-side model should
eventually adopt the shipped layout is a separate fidelity question; if you take that on, it is a new row, and the
answer must come from the DB/statics first (item 13), not from whichever layout compiles.

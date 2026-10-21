---
status: open
assignee: SwiftRaven (01a15a73) — they discovered the gap while validating the join, after T-032
depends: nothing; BLOCKS T-033's acceptance criterion and every future census delta
---

# T-040 — `artifacts/F3.json` has no regeneration path, and four documents require one

## The gap, verified rather than inferred
Four separate artifacts make the export a **required step**:
- `discoveries/2025-10-20-session38-t033-batch1-adjudication.md:86-88` — "renames land in the IDB, and my census join
  runs against the *regenerated export*… (a) the class rename in the IDB, **(b) the F3.json exporter is re-run**,
  (c) I re-run the census."
- `discoveries/2025-10-20-t036-helper-noise-taxonomy.md:169` — "it stays 2,340 until the IDB is renamed **and** `F3.json`
  re-exported."
- `discoveries/2025-10-20-t033-batch2-selfnaming-decisions.md:41` — "no route-A tier movement exists until IDB rename…"
- `docs/notes/dispatch-contract.md:73` — grants `rename-only` scope over "class **keys** in the IDB/`F3.json` export."

**None of them says how the export is produced, and nothing in the repo does it.** Searching for a generator, a recipe,
an `idat -c` script, a FLIDA invocation, or any writer of the file returns nothing: the only historical commit touching
it is `8f281d7` — *"Move F3.exe.i64 and F3.json into new artifacts directory"* — i.e. the file arrived pre-generated and
was relocated. The `.pi/skills/` docs describe how to *query* it, not produce it.

## Why it is not cosmetic
T-033's acceptance is a **measured census delta attributed to specific classes**. Without a regeneration path:
- renames applied to the IDB **cannot** move any route-A (DB) number, so the pre-registered prediction can only ever be
  tested against the live IDB join, never against the artifact the whole board quotes;
- `artifacts/F3.json` is silently a **frozen snapshot with unknown provenance**, while six tools and dozens of documents
  treat it as current. `cls_0x49c610` is the proof this matters: it is `JE::ResourceSystem` in the IDB with 36/43
  body-evidence methods, and the export contains **zero** occurrences of that name.
- any future rename that changes a *class key* desynchronises the export from the IDB with no detectable signal.

## Scope
1. **Determine provenance.** Which tool produced `F3.json`, from what (`.i64`? live IDB? FLIDA?), with what options,
   and when — the file's own structure should fingerprint the exporter (key names, ordering, `undefined4`/`struc`
   conventions are a signature).
2. **Reproduce or rebuild.** Either find the original generator, or write one over the IDB/MCP that produces the same
   schema — the second is acceptable, but then the schema must be pinned by a test, not by hope.
3. **Prove round-trip**: rename one class in the IDB (I recommend `cls_0x4d8d70` → `JE::StringTree`, batch 1's confirmed
   case), re-export, and show the delta the census reports moves **only** for that cluster. That is simultaneously the
   export test and T-033's missing acceptance half.
4. **Record the recipe** in the repo (`docs/` + a script under `tools/`), not in a lane's memory, so `F3.json` stops
   being an artifact nobody can regenerate.

## Interim mitigation (so the board is not hostage)
Measure against the **live IDB** (route A, already done by SwiftRaven in T-036) and keep quoting the export as route B
**labelled stale**. Every lane doc that says "re-run the exporter" now gets this row's number attached until it can be
done. That is a workaround, not a fix: it leaves the artifact and the database telling different stories.

## Boundary
This is about **our tooling**, not the shipped binary: it changes no claim about F3.exe. If the exporter turns out to be
external and unavailable (a closed script from another project), the honest answer is "the export cannot be regenerated
here" and the repo should stop treating it as a live index — that verdict is worth as much as a working generator, and
must be reported rather than papered over by hand-editing JSON.

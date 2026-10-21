---
status: done
verdict: producer IDENTIFIED (OOAnalyzer v2.2.0, run 2024-04-04); stage 3 (results->JSON) rebuilt and proven
         against the shipped file; stage 2 (ooprolog inference) is genuinely absent from this machine, so
         an IDB rename cannot be flowed into F3.json here. The row's own honest exit is the answer.
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


## Verdict (SwiftRaven, closed)

Full record: [`discoveries/2025-10-20-t040-f3json-provenance.md`](../discoveries/2025-10-20-t040-f3json-provenance.md).
Tools: `tools/f3json-from-results.py` and the `f3-json-check` devenv script (`--provenance` prints the chain).

**1. Producer identified by fingerprint — no guessing, and it did not need to still exist.**
`strings artifacts/F3.exe.id0` yields **11** occurrences of `_PyOOAnalyzer__json_filename` / `__json_data`, with
`_PyOOAnalyzer__json_filename = 'Y:\F3.json'` and `__json_data` holding the **entire JSON text** — the export run is
recorded inside the database. `F3.json["version"] == "2.2.0"` matches `EXPECTED_JSON_VERSION = "2.2.0"` in
`/mnt/secondary/IDA/IDA Pro 8.3.230608/IDA/plugins/OOAnalyzer.py:21`. And the recipe survives on disk
(`OpenJE/info/ooanalyzer_steps.txt`): `ooprolog --facts … --results …` then `ooprolog --results … --json …`.

**2. The original inputs are byte-identical siblings of our file.** `/mnt/secondary/VSCode/openje-re/ooanalyzer/`
holds `F3-facts.pl` (27,047,083 B), `F3-results.pl` (1,942,398 B), `F3.json` (4,091,904 B) all dated **2024-04-04**,
and its `F3.json` md5 `e8827f12560f1b5a0c7d369eb22d836b` **equals our checked-in file** (two other copies agree). Our
`artifacts/F3.json` mtime 2026-05-15 is the *relocation* date (`8f281d7`), not a generation date. So: the export is a
**2024-04-04 snapshot, ~18 months older than the IDB (2026-10-19)**.

**3. Stage 2 is verifiably absent.** `find / -xdev -iname '*ooprolog*' -o -iname '*OOAnalyzer*.jar'` → **0 hits**, and
the installed `OOAnalyzer.py` (v2.0) has **0** occurrences of `facts|\.pl|export` — it only *imports* JSON. The
facts-exporting plugin that ran in 2024 is not the one present; `F3.ser` (160 MB) is its serialised state.
**No tool on this machine can regenerate `F3.json` from the current IDB.** The row's own stated honest exit is
therefore the answer, and I took it rather than papering over it: **no hand-editing to produce a delta.**

**4. The last stage is rebuilt and proven, which is the fixable half.** `tools/f3json-from-results.py` re-derives from
`F3-results.pl` and reproduces the shipped file exactly on every field it claims:

| check | result |
|---|---|
| structure keys | **1,101 / 1,101 exact** |
| class size | **0 mismatches** (`finalClass` upper bound) |
| class method set | **1,101 / 1,101 exact** — the method EAs are the **6th arg of `finalClass`**, *not* `finalMethodProperty` (which covers only 3,384 of 6,624) |
| member size | **7,736 / 7,737** — rule is `max(candidate sizes)`, not first (`[0x1,0x4]` → 4; first-element gave 143 wrong) |
| member usages | **0 mismatches** (union of `finalMemberAccess`) |
| synthesized `base`/`struc`/`vftptr` members (4,059) and `vcalls` | **NOT reproduced — declared UNVERIFIED**, not silently passed |

Key rule recovered: **a class owning an RTTI-named vftable is keyed by its mangled name; everything else is
`cls_0x<cea>`.** That explains the 403/698 split and re-frames the whole identity queue — **`cls_0x…` does not mean
"we failed to identify it", it means "OOAnalyzer found no RTTI-named vftable for it in 2024".**

**5. Shown to fail (rule 3), including two failures of my own test.** Drift is caught: a member size
`[0x4]→[0x8]` → exit 1; an added usage EA → exit 1; dropping the `cls_` prefix from the key rule → `keys_exact False`,
exit 1; a corrupted fact line → `results_unparsed_lines 1`, exit 1. And my *first* attempts at two of these were bad
tests: a `sed` whose pattern never matched (so the "drift" run used an **unchanged** file and printed EXACTLY), and a
`rc=$?` placed after `if ! cmd` which captured the negation's status and reported `FAILED (exit 0)`. Both fixed — the
status is now measured from the program, and each failure prints an explicit `DRIFT REASON:` line so verdict and
counters cannot disagree. A bad explicit `--results` path used to fall back silently to another copy on the machine;
it now errors (exit 2), because verifying one export against unrelated facts can print a clean verdict.

**6. Consequences to act on.** (a) `F3.json` must be treated as frozen, quoted as route B labelled stale — the
interim mitigation is now the standing rule, and `f3-json-check` is what stops it rotting. (b) Every doc saying
"re-run the exporter" is **unsatisfiable as written** until §5's options of the doc land (restore `ooprolog`; or
deserialize `F3.ser`; or write a new IDB→facts exporter and accept a different schema). (c) **T-033's acceptance
half is not blocked after all** — the census delta is measurable on route A without any export, which is what T-036
shipped. (d) If the mint rule is wanted in the export, that is a *schema question* for the coordinator, not something
I should fake by editing JSON.

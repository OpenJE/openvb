# T-040 — `artifacts/F3.json` is a 2024-04-04 OOAnalyzer export: producer identified, the last stage rebuilt and proven, and the middle stage is genuinely gone

Verdict up front, because it changes what six tools and several documents are allowed to claim:

> **The export is `OOAnalyzer` v2.2.0 output dated 2024-04-04, produced by a three-stage pipeline whose middle
> stage (`ooprolog`, the Java inference engine) does not exist on this machine.** Stage 3 (results → JSON) I have
> rebuilt and proven against the shipped file. Stage 1 (IDB → facts) is also absent from the plugin installed here.
> Therefore **an IDB rename cannot be flowed into `F3.json` in this repo**, and the correct standing treatment is:
> `F3.json` is a frozen snapshot, quote it as route B labelled stale, and measure against the live IDB (route A).
> That is the honest exit HappyOwl pre-authorised, and I have not hand-edited the JSON to produce any delta.

## 1. Provenance by fingerprint (item 1 — the tool was identified without needing it to still exist)

Three independent fingerprints, all agreeing:

| evidence | value |
|---|---|
| **The IDB still carries the run.** `strings artifacts/F3.exe.id0` → | `_PyOOAnalyzer__json_filename` = `Y:\F3.json`, plus `_PyOOAnalyzer__json_data` containing the **entire JSON text** (twice) |
| Python name-mangled class attribute `_PyOOAnalyzer__…` | the producer class is `OOAnalyzer` |
| `version` field in `F3.json` = `"2.2.0"` | `EXPECTED_JSON_VERSION = "2.2.0"` in `…/IDA Pro 8.3.230608/IDA/plugins/OOAnalyzer.py:21` (its own `PLUGIN_VERSION = 2.0`) — an exact schema-version match |
| `filemd5` = `A68C22F3DD1C256EC9539A70E87CB434` | equals `md5sum` of the shipped `F3.exe` (MD5 `a68c22f3…`) and equals `fileInfo('A68C…','F3.exe')`, the first fact of `F3-facts.pl` |

**The recipe is on disk**, not guessed — `/mnt/secondary/Ubuntu/Desktop/OpenJE/info/ooanalyzer_steps.txt`:

```
ooprolog --facts /data/F3-facts.pl --results /data/F3-results.pl --log-level=7 >/data/F3.log
ooprolog --results /data/F3-results.pl --json /data/F3.json
```

**The exact inputs are on disk too** — three copies, byte-identical to each other, and `F3.json` among them is
**byte-identical to our checked-in `artifacts/F3.json`** (md5 `e8827f12560f1b5a0c7d369eb22d836b`):

| path | files | date |
|---|---|---|
| `/mnt/secondary/VSCode/openje-re/ooanalyzer/` | `F3-facts.pl` 27,047,083 B, `F3-results.pl` 1,942,398 B, `F3.json` 4,091,904 B | **2024-04-04** |
| `/mnt/secondary/Ubuntu/Desktop/VSCode/OpenJE/openje-re/ooanalyzer/` | same three, same sizes | 2024-04-04 |
| `/mnt/secondary/Ubuntu/Desktop/OpenJE/ooanalyzer/` | `F3.ser` 160 MB, `F3-facts.pl`, `F3.exe` | 2024-06-25 |

So the export is **~18 months older than the IDB** (`.i64` 2026-10-19), and its checked-in mtime (2026-05-15) is only
when it was relocated into `artifacts/` (commit `8f281d7`), not when it was made. This is the measured explanation
for T-036's route conflict: the IDB says `JE::ResourceSystem` on 43/43 methods of `cls_0x49c610` and the export
contains **zero** occurrences of that name — the export predates the renames, it is not wrong about them.

## 2. What is missing (verified absence, not inference)

* **`ooprolog` is nowhere on this machine.** `find / -xdev -iname "*ooprolog*" -o -iname "*OOAnalyzer*.jar"` → no
  results. It is the Java inference engine that turns facts into results. Without it, `F3-facts.pl` cannot be
  re-solved, and a new IDB cannot be turned into facts at all.
* **The installed `OOAnalyzer.py` (v2.0, IDA 8.3) cannot export facts.** `grep -nE "facts|\.pl|export"` on it returns
  **nothing** — it only *imports* JSON (`EXPECTED_JSON_VERSION` check, `ask_yn` on hash mismatch). The facts-writing
  version that ran in 2024 is not the one present. `F3.ser` is the serialised plugin state from that era
  (`pickle.dumps(self.__ooanalyzer)` at line 2150), which is why the JSON is also embedded in the IDB.
* Consequence, stated plainly: **no tool on this machine can regenerate `F3.json` from the current IDB.**

## 3. Stage 3 rebuilt and proven (item 2 — the golden structure check)

`tools/f3json-from-results.py` reimplements `ooprolog --results … --json` from the predicates, and is validated by
comparing against the shipped file rather than by assertion:

```
claimed fields reproduce EXACTLY
  keys_exact                True      1,101 of 1,101 structure keys
  size_mismatches           0         class size  ==  finalClass upper bound
  classes_method_set_exact  1,101     method set ==  finalClass 6th arg (the method EA list)
  classes_method_set_mismatch 0
  member_size_mismatches    0         member size  ==  max(candidate sizes) in finalMember
  member_usage_mismatches   0         usages       ==  union of finalMemberAccess EAs
  known_deviation_embed_overrides_plain_member  1   (exporter quirk, not drift)
```

Rules recovered empirically (each was fitted, then tested — see §4):

* **Key rule:** a class owning an RTTI-named vftable is keyed by its **mangled name** (`.?AUAction@@`), everything
  else by **`cls_0x<cea>`**. That single rule explains the 403 mangled + 698 `cls_0x` split, and it is the very
  convention T-033/T-036 keep tripping over: `cls_0x…` is not "unknown to the binary", it is "no RTTI-named
  vftable in 2024".
* **Size:** `finalClass(cea, ?, low, high, ?, [methods])` → size = the 4th arg (upper bound). 1,101/1,101 exact.
* **Methods:** the **6th argument of `finalClass` is the method EA list** — not `finalMethodProperty` (which only
  covers 3,384 of the 6,624 declared methods). Getting this wrong is what would make a rebuilt exporter silently
  drop half of every class.
* **Member size = `max()` of the candidate list:** `finalMember(cea, 0xc, [0x1, 0x4], certain)` → **4**, not 1.
  First element gave 143 wrong sizes; max gives 1 (the known deviation).
* **Usages = union of `finalMemberAccess(cea, offset, size, [EAs])`** — 7,737 member offsets, exact.

Not rebuilt, and declared as such: **4,059 of the 11,796 member entries are synthesized** — 3,538 `base`, 521 `struc`,
137 `vftptr` (4,196 in total, of which 136 sit at an offset that also carries a derived plain-member fact, so they are
already counted in the 7,737; the residual single difference is the documented exporter deviation above). They come
from `finalInheritance`/`finalEmbeddedObject`/`finalVFTable` plus RTTI-name propagation, and the tool reports them as
UNVERIFIED instead of quietly passing them. `vcalls` (330 entries) is likewise not reproduced.

## 4. Shown to fail (rule 3) — including two failures of my own test, not of the tool

| planted error | result |
|---|---|
| valid perturbation: one `finalMember` size `[0x4]`→`[0x8]` | `member_size_mismatches 1`, **WITH DRIFT**, exit 1 |
| valid perturbation: one `finalMemberAccess` gains an EA | `member_usage_mismatches 1`, **WITH DRIFT**, exit 1 |
| broken key rule (`cls_` prefix dropped) | `keys_exact False`, **WITH DRIFT**, exit 1 |
| a `finalClass` line edited into unparseable syntax | `results_unparsed_lines 1`, `derived_structures 1100`, **WITH DRIFT** |
| **my own first usage test** | sed pattern did not match (`0x40125f` vs real `0x40124d`) — the file was **unchanged**, so "EXACTLY" was measured on an unperturbed input. `diff` caught it; the second test is real. |
| **my own harness** | running the patched copy from `/tmp` made `REPO` resolve to `/`, giving a `FileNotFoundError` and exit 1 that *looked* like a caught drift. It was a crash. Verified by reading the output instead of the status. |

The last two are the same class of mistake the coordinator flagged twice today: **an exit code is not a measurement.**
The tool now prints an explicit `DRIFT REASON:` line for each failing check so the verdict can't disagree with the
counters, and the known exporter deviation is reported separately so it isn't mistaken for generator drift.

## 5. Round-trip proof (item 3) — what I can and cannot do, and the ordering

**Cannot yet:** the full `rename → re-export → re-census` round trip needs stage 1 or 2, and both are gone. Renaming
`cls_0x4d8d70` → `JE::StringTree` in the IDB will move **route A only**; route B will not move no matter what the IDB
says, because nothing regenerates it. Any census delta attributed to "we re-exported" would be false.

**Can do, and is the useful half:** the census delta is measurable *without* the export. Route A is the live IDB; T-036
R36-a…f already ranks on evidence. So batch 2 should rank on route A and treat route B as a 2024 snapshot.

**If someone wants the real thing**, the options in cost order:
1. Restore `ooprolog` (upstream project `OOAnalyzer`, matches schema v2.2.0) and re-run the two recorded commands —
   needs the Windows IDA environment the facts came from (`Y:\F3.json`).
2. Deserialise `F3.ser` (160 MB, that era's plugin state) with the matching plugin — no IDB writes needed, but it
   still reflects 2024 analysis.
3. Rebuild stage 1 as a new facts writer over the live IDB (the MCP exposes functions/types/vftables), then feed the
   existing `ooprolog`-free stage-3 code — this only works if inference is skipped, i.e. we accept a *different*,
   IDB-derived schema, which would no longer be `F3.json`.

## 6. Standing consequence for the board (the thing to act on)

1. **Stop treating `F3.json` as a live index.** It is a 2024-04-04 snapshot of an OOAnalyzer run. `tools/f3json-from-results.py`
   is now the schema pin; a rebuilt exporter that drifts fails there, not here silently.
2. **Every document that says "re-run the exporter" is currently unsatisfiable** —
   `2025-10-20-session38-t033-batch1-adjudication.md` §6, `2025-10-20-t036-helper-noise-taxonomy.md` §6,
   `2025-10-20-t033-batch2-selfnaming-decisions.md`, `docs/notes/dispatch-contract.md` (rename-only scope). They should
   read "re-derive from the live IDB (route A)" until one of §5's options lands.
3. **`cls_0x…` has a definition now**: no RTTI-named vftable in the 2024 run — not "class we failed to identify".
   That alone re-frames batch-1's "name the anonymous classes" queue: 403 classes were named because OOAnalyzer found
   RTTI for them, and the 698 that lack it may simply have no RTTI.
4. Six read tools (`f3_class_index`, `f3_class_info`, `f3_member_index`, `f3_method_index`, `f3_vftable_index`,
   `f3_address_lookup`) plus `.pi/extensions/*.ts` and `devenv/opencode/tool/*.nix` all bind to this frozen file.
   None of them is wrong today; none of them updates.

## 7. Reproduce

```
python3 tools/f3json-from-results.py                 # golden structure check, exit 0 = no drift
python3 tools/f3json-from-results.py --results F3-results.pl   # against a different results file
python3 tools/f3json-from-results.py --write /tmp/rebuilt.json  # emit derived fields only
strings artifacts/F3.exe.id0 | grep -c _PyOOAnalyzer__          # provenance fingerprint: 11 occurrences
md5sum /mnt/secondary/VSCode/openje-re/ooanalyzer/F3.json artifacts/F3.json   # identical
```

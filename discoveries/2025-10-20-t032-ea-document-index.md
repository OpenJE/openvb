# T-032 — EA→document index: `artifacts/ea-index.tsv` makes 136 discovery docs addressable, and building it exposed four ways an EA index silently lies

Tool: `tools/ea-index.py` (pure offline, no IDA, no IDB, no build). Output: `artifacts/ea-index.tsv`.

## 1. Result

| quantity | value |
|---|---|
| documents indexed (`discoveries/*.md`) | 137 (this file included) |
| distinct EA keys | **5,258** (row estimated ~5,100) |
| (EA, doc) mention pairs | 7,778 |
| refused tokens, counted and classified | **756** |
| keys backed only by inference from compressed prose | **51 (1.0%)** |
| full recall audit | 5,258 × 137 = **720,346 pairs, 0 silent misses** (+1 explicitly refused) |

```
tools/ea-index.py                  # regenerate artifacts/ea-index.tsv
tools/ea-index.py --query 0x45C140 # citing docs + section heading + the line itself
tools/ea-index.py --verify --full  # recall audit: 714,816 pairs, exit 3 if any miss
tools/ea-index.py --check          # exit 2 if the checked-in TSV is stale
tools/ea-index.py --refused        # every refused token with file:line, grouped by reason
tools/ea-index.py --inferred       # the 51 keys that inference created
```

Acceptance, each demonstrated: `--query 0x413310` → 7 documents led by
`2025-10-20-attack-state-machine-damage-formula.md` (the damage formula); an absent EA prints
**`NO RECORD` and exits 1**, never empty-with-zero; refusals are **counted in the run output and in the TSV header**,
never dropped silently.

## 2. The four failure modes found while building it (these are the real product)

1. **`\b0x` misses the corpus's most common EA form.** `CalculateDamage_0x413310` has a word char (`_`)
   before the `0`, so `\b` cannot match there — and `_` after the digits also breaks the trailing `\b`
   (`global_cls_0x4d8d70_0x70c668`). The first version indexed **1** of the 4 documents citing `0x413310`; boundary
   rules fixed it to 4. Any index written with `\b` word boundaries against identifier-embedded addresses is
   quietly blind to most of this corpus.
2. **Case-splitting fragments the corpus.** Uppercase `0x54BE10` and lowercase `0x54be10` became two keys:
   **609 uppercase keys, 318 addresses split across two entries.** Keys are now normalised to `0x%06x`.
3. **A decimal is not an address, but an identifier suffix is.** My first "pure digits ⇒ decimal" guard also killed
   legitimate `QueueToCombat_463140`, `PackState_545930`, `_0x98_463b60` (21 real misses). The guard now applies
   **only** to a bare token floating in prose, and is lifted inside fenced blocks, where `470377:` is a disassembly
   line address by convention. Refused-with-count instead of dropped: 8 bare all-digit tokens remain refused.
4. **Substring matching makes a false *audit*.** My first recall check flagged a "miss" for `0x400000`, caused by
   matching inside the flag constant `0x40000000`. The verifier was wrong, not the index — boundary-based token
   extraction is now used for the audit too.

## 3. What is refused, and why that is the right answer

| refusal | count | reason |
|---|---|---|
| `vft+0x…` slot offsets | 561 | relative to a vftable this tool does not resolve; keying `vft+0x20` as an address would be a lie |
| hex outside image span `0x400000–0x715800` | 107 | FourCCs (`0x544E4547` = "GENt"), file offsets (`0x2f1eb4`), masks (`0x7F7FFFFE`) |
| masked/pattern EAs | 61 | `0x401xxx`, `0x45c0XX` are N significant digits, not an address |
| bare all-digit hex | 8 | could be a decimal count |
| ambiguous `+0x` with address-looking digits | 4 | `+0x66856C` — struct offset or global address? Both readings exist in this corpus |
| range outside image | 2 | file-offset ranges |

The `+0x` case is the one worth keeping in front of anyone who extends this tool: the corpus uses `+0x…` for
member offsets **and** for globals, and merging them into one key would fabricate relationships. Refuse, count,
and let a human decide.

## 4. The 51 inference-only keys

`@0x45c251/261/271`, `sub_0x463000/463040/463080/4630c0/463100`, `0x414B7C/4B99/4BBC/4BC0/4BD8` are compressed
notation for real addresses, so the tool expands them — but those keys were **never literally written** in any
document, so a typo in the prose propagates straight into the index. They are listed by `--inferred` and counted in
the TSV header. Treat them as weaker than the 5,205 literal keys.

## 5. Boundary (also printed in the tool's own header)

The index records **where an EA is mentioned**, not whether the mention is correct or still current. Superseded
documents stay in the index and must be judged by the reader — check
`2025-10-20-canonical-fact-sheet.md` before trusting a hit. Scope is `discoveries/` only: not `tasks/`, not `src/`
comments, not `docs/f3/`, not the IDB. `0x45c140` and `0x45c141` are unrelated keys; the tool follows no xrefs,
vftables, or inheritance. **Recall is proven (720,346 pairs, 0 silent misses); precision is not** — `--verify` measures recall only,
so a wrong-but-literally-written address is indexed faithfully.

## 6. Shown to fail

`--verify --full` was tested by sabotaging `SUFFIX_RE` into matching nothing (valid syntax, silent behaviour change):
it reported **73 missed pairs in 43 docs and exited 3**. Restored, it reports 0 and exits 0. `--check` was tested by
adding an unindexed document (exit 2), and by writing this very file before regenerating (the audit flagged its own
`470377:` mention as a refusal, not a miss), and it had to be fixed first — the original version regenerated the TSV
*before* checking it, so it always reported "current". A check that cannot be broken is decoration; both of these
can be.

## 7. Cross-links

* Directly useful to **T-033/T-036**: `--query cls_0x49c610` now returns every document that touches the class batch-1
  could not identify, without grepping prose.
* Relevant to **T-040** (`F3.json` has no regeneration path): the prose corpus holds **5,256 distinct EAs** and the
  export holds **6,624 declared method EAs** (`f3_method_index`, route B). Those are two independent views of the same
  binary, and the delta between them is exactly the desync T-036 measured (`0x4154f0` is a method in the export and a
  chunk inside `F3::GameWeapon::virt_meth_0x414680` in the IDB).

---
status: open
assignee: IronStorm (01a157f6)
depends: nothing
---

# T-029 — The missing denominator: function-level RE coverage

## Why
Asked "how much is left", I could only produce **type-file arithmetic**: 1,101 classes, 6,624 methods,
31 hand-ported headers, ~3% by class count — and I had to admit that is a proxy, because this MCP set
has no function-enumeration tool. Without the real denominator, coverage claims are vibes.

## Scope
1. Get totals from the original IDB by whatever route works: an idapython script the user loads, or
   enumerating via available MCP tools. Report: **function count**, how many carry our
   `Namespace::Class::method_0xADDR` naming, how many are still `sub_`, and how many vftable slots
   (of the 451 vftabled classes) resolve to an identified target.
2. Cross-check `artifacts/F3.json`'s 6,624 declared methods against IDA's function list — a mismatch
   tells us the type DB is missing or duplicating entries, which is worth knowing on its own.
3. Bucket the still-unnamed functions by nearest identified owner (vftable class, or caller cluster)
   so "what's left" becomes a ranked list instead of a fog.
4. Write the numbers to `discoveries/` **and** a one-line summary into the progress doc, because both
   are quoted.

## Acceptance
- Every claim states the route used to count, since two routes disagreeing is the interesting result.
- Gives the project a coverage percentage we can re-measure weekly, replacing "about 3%, probably".
- If the census cannot be produced without the user's help, **say precisely what is missing** instead
   of substituting a proxy and calling it coverage (harness item 16).

## Refs
- `artifacts/F3.json` (1,101 structures / 6,624 methods / 451 with vftables / 330 vcall sites)
- `docs/notes/harness-items.md` 16

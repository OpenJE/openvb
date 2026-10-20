---
status: done
verdict: census complete, no user help needed; 8,876 game functions of 10,173
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

## Verdict (IronStorm B11, `session37-t029-function-census.md`) — the denominator exists now

Route A (`list_funcs`, 7 pages, integrity-checked: 10,173 items, 10,173 unique addrs, monotonic at every
join) plus route B (F3.json join), with **no user help and no idapython**, and the recipe documented in
§5 so it is re-measurable weekly as-is.

- **8,876 game functions of 10,173 total.** Boundary `0x5FD000` = link tail (CRT/D3DX/atexit); MFC
  fragments interleave earlier and are disclosed, so the number is conservative.
- **Tiers: 46.7% semantically named** (JE::/F3::/net:: + hand plain), **26.4% typed-stub** — 2,340
  functions sitting under ~300 anonymous `cls_0xADDR::` types, the biggest backlog cluster and the
  cheapest win, **26.9% unnamed** (2,177 `sub_` + residue).
- **DB and IDA agree to 97.4%**: 171 of the 6,624 declared methods are not distinct IDA functions but
  tail/mid-function vcall targets (e.g. 0x411f00 inside `GameProjectile::virt_meth_0x411ef0`). Recorded
  as a known artifact class so nobody "loses" 171 functions in a future diff.
- **Vftables: 2,156 slots, 97.9% point at identified functions** — dispatch-table RE is essentially
  complete. The fog is elsewhere.
- **Backlog shape drives the method:** the 2,177 `sub_` sit in 1,971 distinct owner neighbourhoods (top
  cluster only 24 — JE::GUI; GameActionEquip 23; ResourceSystem 16), so triage is by **cluster/class**,
  not by function queue. Retiring the top-20 stub *classes* re-tiers ~80% of T2 in ~20 identity decisions.

Reframe for my own "about 3%" answer: class-count port coverage and this function census measure
different things. Both are numbers now; quote either one **with its name** and never the two as if one.

## Acceptance
- Every claim states the route used to count, since two routes disagreeing is the interesting result.
- Gives the project a coverage percentage we can re-measure weekly, replacing "about 3%, probably".
- If the census cannot be produced without the user's help, **say precisely what is missing** instead
   of substituting a proxy and calling it coverage (harness item 16).

## Refs
- `artifacts/F3.json` (1,101 structures / 6,624 methods / 451 with vftables / 330 vcall sites)
- `docs/notes/harness-items.md` 16

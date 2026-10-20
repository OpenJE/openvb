---
status: open
assignee: two lanes as mainline (proposed: IronStorm + HappyWolf)
depends: T-029 (the census that produced this queue)
---

# T-033 — Class-identity batch queue: the third kind of worker

## Why
The census (T-029) gives the shape: **2,340 typed-stub functions sit under ~300 anonymous `cls_0xADDR`
types**, and the 2,177 unnamed functions live in 1,971 owner neighbourhoods with a top cluster of only 24.
So the unit of work is a **class, not a function**: retiring the top-20 anonymous classes re-tiers ~80% of
the typed-stub tier — roughly **twenty identity decisions for two thousand functions**.

Neither existing lane owns this. It looks like research and lands like implementation: the deliverable is
a name written into the IDB and `artifacts/F3.json`, which propagates into every later decompile, every
census number, and every literal/symbol verification. It is the cheapest structural win available and it is
currently nobody's job.

## Scope (one batch = 20 classes, ordered by function count)
1. Take the next N anonymous classes by member count. For each, read **only** what the database already
   holds: member offsets/sizes, vftable slots, method names, and which shipped data blobs reference it.
2. Decide identity from **data before debugger** (harness item 13): a `.gam`/`.msg`/`.veg` section layout,
   a string in the PE, or a vftable that mirrors a known `Action`/`Item`/`GUI` shape is evidence; a hunch
   is not.
3. Name it. MSVC-2002 conventions and `Namespace::Class`; **forward-declaration keyword must match the
   definition** (`struct` vs `class`) — MSVC decorates `U`/`V` and a mismatch is a link error that mingw
   will not show you (`docs/DEFERRED-NOTES.md:389`).
4. Re-run the T-029 census and record the tier shift as a number. If the tier did not move, the batch was
   decoration — say so.

## Acceptance
- A committed table `class → functions retired → tier before/after` per batch, in `discoveries/`.
- Census re-run shows the named-percentage moving, with the delta attributed to specific classes.
- **Every name ships with the evidence that produced it** (a data structure, a string at a PE offset, a
  vftable mirror). A name with no evidence stays anonymous and gets escalated instead.
- If a name turns out wrong later, it is retracted in the ledger rather than quietly renamed.

## Boundary
Identity is a claim about the shipped binary, not about our port: nothing here changes `src/`. A class
named from its vftable shape may still be semantically something else, which is why the evidence column is
mandatory and why the census delta — not the naming count — is the success measure.

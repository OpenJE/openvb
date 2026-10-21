---
status: open
assignee: SwiftRaven
depends: T-029 (census), T-033 batch-1 adjudication (the number this explains)
---

# T-036 — Where did the 998 "helper-theft" functions come from?

## Why
Batch-1 adjudication restated tier 2 as **2,473 attributed = 1,475 nameable + 998 helper-theft noise** — i.e.
**40% of the anonymous-class attribution is not class mass.** Nobody has explained the mechanism, and it sits under
every census number, every ordering decision, and every future decompile: if a lane cannot say why 998 functions got
pulled into classes they do not belong to, the census is a count of an artifact, not of the shipped binary.

## What
Characterise the noise from the database and the IDB (offline first, item 13):
1. Sample the low-density classes (contiguity 0.02–0.37, esp. the six batch-1 headline candidates) and classify what
   the "extra" functions actually are: thunks, shared statics/helpers, vcall tail targets (the 171-method artifact from
   T-029), CRT glue, or genuinely unrelated methods.
2. Find the **mechanism**: is it IDA's automatic class assignment by address proximity, our `F3.json` export joining
   member ranges onto address spans, or real shipped layout? One example class traced end-to-end beats a hundred counts.
3. Propose the fix as a **rule, not a cleanup**: e.g. density floor before a rename is allowed, or an export-side
   distance check, or "never trust a class with N>80 members and ratio<0.4".

## Acceptance
- A taxonomy table: noise category → count → one worked example (EA + why it is misattributed).
- The sum reconciles against **998** within the sample's error, or says plainly where it does not.
- A concrete rule that a future batch can apply, and a re-run of the census showing the attributed count **after** the
  rule — the delta is the point.

## Boundary
This measures how *our database and IDA* describe the binary, not how the binary is structured. If a class turns out to
be genuinely heterogeneous, say so: helper-theft and shared-base-class are different answers, and only one of them is an
artifact.

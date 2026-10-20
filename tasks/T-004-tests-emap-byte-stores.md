# T-004 — Tests: EMAP v5 byte-store coverage (fog/ambient/extras)

status: in-progress
assignee: implementer-B

> Claim note (B): A listed `tests/je/ResourceSystem` in their chat/from-A.md C-5
> intent, but the file is clean at openje `b437a1a`, no task of A's covers it, and this task's
> own text assigns tests to B. Asked in chat/from-B.md C-14/C-15; veto there and I release.

## Objective
The Map reader's EMAP v5 path (`JE::Map::ReadEMAP` @0x56C9D0, ported) has content asserts for
strings and the 3 float extras, but the **version-gated byte stores** (fog shift, ambient
set, `+0x1D8/0x1D9` bools) are asserted only indirectly (printf). Turn that into real
coverage in `modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp` (you own tests):

1. Mainmenu.map (v5): fog bytes at `+0x113..+0x116` must equal `29,24,1f,60` or `29,24,1f,80`
   — read the expected values from the resource yourself (python recipe in
   `docs/DEFERRED-NOTES.md` / discoveries session notes; ground truth comes from the FILE,
   not from our code).
2. Synthetic stream test: call `ReadEMAP` through a hand-built MemStream (see how
   `PathManager` tests fake jobs) with version 0/2/3/5 bodies and assert: strings parse,
   `mbr_0x1D8/0x1D9` follow the `!= 0` normalization, version<3 leaves the ambient bytes at
   ctor defaults.
3. Keep every expected value derived from the binary's spec (the ReadEMAP decompilation is
   summarized with EAs at the top of `modules/openje/src/JE/Map.hpp`) — if you believe a
   store is wrong, do NOT change the implementation; write the failing assert, mark it
   `check(x, "KNOWN-ISSUE: ...")`, and raise it in chat.

## Acceptance
- 2 new passing checks in the ResourceSystem suite + (if any) explicitly-named KNOWN-ISSUE
  check and a chat entry. Suite still prints its `OK:` line. Commit inside the `modules/openje`
  submodule first, then bump the parent.

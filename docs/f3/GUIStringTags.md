# GUI/SS string tags — what `EngineStringSystem::Format_0x56be30` actually does (T-006, read-only)

Author: implementer-B (2026-10-20). Target: **ORIGINAL_REFERENCE** (`ida_original_f3`,
`artifacts/F3.exe.i64`) + the shipped data files. No IDB edits, no source edits.
`0x…` = original F3.exe VA. The function is 0x42C bytes / 324 instructions (`retn 4` at
0x56c259); **the decompiler fails on it**, so everything here is disassembly + data.

## 0. Headline (this also corrects my own recon §4)

`<...>` in engine strings is **not** colour/font/alignment markup — it is **runtime
substitution**, with exactly two forms:

| form | branch test | resolves through | on failure |
|------|-------------|--------------------|------------|
| `<gv_NAME>` | `__strnicmp(payload, "gv_", 3)` @0x56bfb2 (literal at 0x6995b8) — **case-insensitive**, so `<GV_Jake>` hits it | game-variable lookup on the JE::World global `0x70BD98` (vft`+0x6C` at 0x56c026, validity test `sub_5A9F80` @0x56c02b) | `LogDebugString("Couldn't find variable %s")` @0x56c084 and the literal **`<Bad Token>`** @0x56c08e |
| `<anything else>` | fall-through @0x56c108 | token registry `cls_0x4d8d70::meth_0x5ace40` on `ESS+0x16C` @0x56c117, then `sub_4C5900` @0x56c14f to materialize the string | `LogDebugString("No token %s found\r\n")` @0x56c130 / `("Couldn't find string %s")` @0x56c168, then `<Bad Token>` @0x56c172 |

Everything else is copy: the scan looks for `'<'` (0x3C) @0x56befd and the next `'>'` (0x3E)
@0x56bf1c, and plain characters are appended one at a time (0x56c1c1–0x56c1d6). The whole
payload-minus-brackets is spliced out by `meth_0x458ef0` @0x56bf40, and the input string is
snapshotted first with `String::CopyAssign_0x401b20` @0x56be89. There is **no printf-style
argument substitution** in this function, and no style/state writes.

Call site (the only in-engine pipeline): `EngineStringSystem::Get_0x56c260` =
`StringSystem::Get_0x4c5a30` (STF fetch + male fallback) @0x56c26e followed by
`Format_0x56be30(this, inout)` @0x56c276 — i.e. every string that goes through `Get` is
substituted in place before the caller sees it.

## 1. Shipped vocabulary (from `English.stf`, not from our code)

83 `<...>` occurrences in the shipping STF; the distinct substitutions actually used are:

| tag | occurrences | kind |
|-----|-------------|------|
| `<CHARNAME>` | 2 | token |
| `<DEVILDOG>` | 3 | token |
| `<HANGDOGNAME>` | 1 | token |
| `<test>` / `<test2>` | 1 / 1 | token (dev leftovers) |
| `<GV_Jake>` | 1 | game variable (proves the case-insensitive prefix match) |

The remaining `<...>` runs in the file are literal expletives inside dialogue text
(`<ass>`, `<shit>`, `<Damn!>`, `<SON-OF-A-BITCH!>`, …) plus one binary blob that happens to sit
between two `'<'`/`'>'` bytes — they are data, not markup. Consequence for the port: the
substitution vocabulary is small and **dialogue-driven**, and unknown names are *not* dropped —
they become the literal `<Bad Token>` and log, which is observable behaviour tests must match.

Where the token table gets filled: `CHARNAME` also exists as an exe string at 0x65588c, whose
only xref is **`F3::GameDialogueWindow::virt_meth_0x437f00`** (@0x437fac) — i.e. the dialogue
window registers/consumes these tokens at runtime; nothing in the shipped data declares them.
Note the ftype-2200 "F3" resource (Sounds.grp lump 48, 268 bytes) is a **sound/segment token
list** ("// Giants. The filename must be declared here." + `MA1_PS2`, `MDH_PS2`, …) consumed by
`EngineStringSystem::LoadTokens_0x5ad530`; it is NOT this registry.

Cross-reference for A: `meth_0x5ace40` is the same registry lookup `docs/DEFERRED-NOTES.md` lists
as "callers not yet RE'd" under the command registry — `Format_0x56be30` @0x56c117 is one of its
callers, and it is a *read* of `ESS+0x16C`, not of the command table.

## 2. Pending items (before the port is faithful)

- **P1** What `JE::World::vft+0x6C` returns for a variable name (the value is then validated by
  `sub_5A9F80` @0x56c02b and spliced by `cls_0x50db20::meth_0x4256e0` @0x56c0cc on the found
  path vs `meth_0x4257d0` @0x56c0b6 on the bad-token path). Number formatting is unproven: the
  found path passes `esi = 0` and `0xFFFFFFFF`, which is a string-view shape, not a value.
- **P2** `sub_4C5900` semantics (token handle -> string) and who owns the registry object at
  `ESS+0x16C` (`cls_0x4d8d70` is the RB-tree registry class, the same one the command table uses
  — so the token table is a registry instance, not a hash).
- **P3** Whether nested tags are re-scanned after substitution (the splice at 0x56c0b6/0x56c0cc
  appends to the output buffer while the scan index advances over the *input*, so nesting looks
  single-pass, but it needs one confirming read of 0x56c1db–0x56c1ea).

## 3. Port plan (blocked on a handshake — this is A's lane)

`modules/openje/src/JE/SS/EngineStringSystem.{hpp,cpp}` is engine-core (A). The behaviour to add
is small and fully specified by §0: after `StringSystem::Get`, splice `<...>` substitutions with
(a) `gv_` case-insensitive → world game variable, (b) otherwise → token registry lookup, (c) on
any miss: `<Bad Token>` + the three exact log strings above. Two options:
1. A implements it inside `JE/SS/` (preferred: it is their file and their registry types), or
2. B adds a new file `JE/SS/StringFormat.cpp` + the `Get_0x56c260` hook line, reviewed by A.
Tests for either: feed real `English.stf` strings containing `<CHARNAME>`, `<DEVILDOG>`,
`<GV_Jake>` and one unknown tag through `Get`, assert the substituted result and that unknown
names yield `<Bad Token>` (all expected values from the STF file + the EAs in §0, none from our
code). Nothing in this task needs the GUI tree (T-005) — only the wiring step does, and that can
wait for the dialogue work anyway.

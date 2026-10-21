---
status: in-progress
assignee: SwiftMoon `01a16092` (dispatched by LoudDragon `01a15f82`, 2026-10-21 — F3 micro-row born from the T-072 sweep's F3 flag)
type: research (READ-ONLY everywhere; no IDB writes; deliverable = verdict LIST for append-on-both docs, left DIRTY)
depends: T-072 flag F3 (`discoveries/2025-10-21-t072-doc-errata-sweep.md` §6)
---

# T-078 — the 0x64D7D8 byte-run: pointer-vs-content walk settling G5 vs the gcre reading

## Objective
Two accepted docs read the SAME byte run differently:
- `docs/f3/GUILoadingRecon.md` §5 (G5): Manager::meth_0x4b4920 "assigns a literal at 0x64D7D8
  into Manager+0x70 (that VA did not resolve to a clean string through the section map)".
- `discoveries/2025-10-20-gcre-versions-gwam-queueanimevent-table.md` (gcre pass): the string at
  `byte_64D7D8` is the "default/unarmed" mode-name token.
T-072's raw read: 0x64D7D8 = eight NULs, then a clean .rdata literal at 0x64D7E0
("GameCreature::ReadVersion2() - Making failsafe attack mode for entity: %…").
Verdict required: which reading is byte-true — and BOTH may be half-right (e.g. a NUL-padded
slot whose data-ref lands near a real literal; or the Manager assign takes the ADDRESS of the
zero-padded run while the gcre reader derefs into the string at +8). Decide per site.

## Steps (all read-only)
1. Enumerate every dwords-in-image data reference to 0x64D7D8 and to 0x64D7E0 (imm32 scan of
   .text/.rdata/.data1; per item 8c, absolute-address scans must be re-checked against rip-
   relative/disp8 forms — a "phantom ref" is a tool failure until proven otherwise).
2. `Manager::meth_0x4b4920`: disassemble the literal-assign region; is it `mov reg, offset
   0x64D7D8` (pointer), `mov reg, [0x64D7D8]` (content = 0), or a string-assign call receiving
   the address? What type is Manager+0x70 (float? std::string SSO? char*)?
3. gcre reader (the `ReadVersion2` failsafe path near the GameCreature mode-name usage): find the
   exact consumer of 0x64D7D8/0x64D7E0 and its deref depth; verify the literal pool layout
   (does the NUL run belong to a PREVIOUS string — i.e. is 0x64D7D8 the tail of some
   earlier literal rather than a distinct object?).
4. Ctor init sites: does any static-init or ctor write INTO 0x64D7D8 (is it writable data or
   pure .rdata)? The gcre doc says "runtime-filled" style prefix tables live in .data1/BSS —
   check whether this run is .rdata (immutable) and thus a literal, not a buffer.
5. Verdict per claim: G5 wording (STILL-TRUE / WRONG-as-worded), gcre wording (same), and the
   correct statement of what each address IS, with EAs. Deliver to coordinator for
   append-on-both; change NOTHING.

## Acceptance
`discoveries/2025-10-21-t078-byte-run-0x64d7d8.md` (LEFT DIRTY): a per-address table
(0x64D7D8, 0x64D7E0: bytes, section+writeability, every referrer w/ EA + instruction form),
the Manager+0x70 type + assign-form evidence, the gcre consumer's deref depth, and the
disposition line for each doc's claim. Ping [DISCOVERY-BATCH ready].

## References
T-072 §6 F3; GUILoadingRecon §5 + §2 (Manager+0x70); gcre doc line ~50; harness items 35/43/47.

## Notes
- Claim: SwiftMoon, this file, reserved; README flip relayed to coordinator (table is
  coordinator-only per new law).

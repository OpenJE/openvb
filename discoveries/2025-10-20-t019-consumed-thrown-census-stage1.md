# 2025-10-20 — T-019 stage 1: consumed+thrown FatalError is NOT statically dead — data census + a self-correction on byte positions

Task: T-019. Author: FastArrow. Target: shipped data (`$OPENVB_ORIG_EXE` dir/data/**.grp) +
ORIGINAL_REFERENCE disasm. Read-only; reproducible python census (recipe at bottom).

## 1. Census results (the cheap half, done)

- GWAM chunk occurrences: **57 total — 37 in `_CRT.grp` (creature-side twin parser,
  B9 §5.2 site), 20 in `_WEA.grp`**; **every single one is version 5** (histogram {5:57}).
  ⇒ the v≥5 gate does NOT dead-code the pair-bool path: shipped data exercises it 57 times.
- Naive "last two body bytes" check per chunk (chunk header = {tag,ver,size}, size incl.
  header, body = bytes [start+12, start+size)): histogram **(1,0)×54, (1,1)×3** —
  3 candidates that would trip under that reading:
  `_WEA.grp` GWAM@0x384 (size 0x6d), GWAM@0x55b (0x6d), GWAM@0x30a8 (0x73).
- Demo boots (gate: app-boot 124), so at most the candidates are (a) never-parsed (lazy GRP
  reads — likely for unreferenced weapon templates) or (b) my byte mapping is wrong.

## 2. The self-correction (why (b) is probably true, and what stays claimed)

Re-read of the checker (0x414B53-0x414B74) against the prologue's slot writes: the
two operands are **asymmetric**:
- `[esp+0x4b]` = normalized (`!=0`) 2nd v5 stream byte (last body byte) ✓ tail-positioned;
- `[esp+0x4a]` = **byte-2 of the u32 read #12** (the packed-bool family, slot written at
  0x414A3F as a 4-byte stream read; only bytes +0x48/+0x49 of that slot are ever rewritten)
  — i.e. at a FIXED offset from the field-12 position, NOT from the chunk end.
⇒ "last two bytes" conflated the pair; the 3 hits are **candidates pending the B9 frame
reconstitution** (which field-12 byte aligns where depends on the string-field length at
v4 — variable). This is exactly the packed-bool trap B9 §1 warned about, hit by me first.

## 3. What T-019 can now state, honestly

1. The branch is **not dead by version-gating** (57/57 v5) and **not dead by constants**
   (operands are data bytes).
2. Whether any shipped record trips it **cannot be answered without the reconstitution**;
   stage 2 = one pass mapping field-12's stream position per record (variable string
   lengths make it per-record, mechanically: parse 21-field order from §1 of B9).
3. Reimpl policy meanwhile: **implement the check as shipped** (mirror-and-cite rule). If
   the reconstitution later proves a tripping record reachable by any shipped boot/usage
   path, it becomes a crash-parity requirement; if all tripping candidates are unreachable,
   it stays a data-landmine comment. Either way it must not be "fixed".

## 4. Repro recipe
```
python3: walk data/**/*.grp, regex rb'GWAM', struct.unpack '<II' at +4 → (ver,size);
         last two body bytes = data[s+size-2:s+size]  (naive only — see §2)
```
Census run: this session, dir `$(dirname $OPENVB_ORIG_EXE)/data`.

## 5. Hand-off note
Stage 2 rides inside the B9-queue reconstitution (same frame work) — the two items are one
sitting; T-019 stays "open, 90% scoped" until then. Board row should read: v5 census done,
pair-mapping pending reconstitution, 3 named candidate records on file.

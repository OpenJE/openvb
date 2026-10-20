# 2025-10-20 — B11: GWAM slot work, part one — stream order mechanically confirmed; corrected-rule census says 0/57 trip the consumed+thrown FatalError (provisional verdict)

Task: T-019 stage 2 + T-015 promotion, PARTIAL by design (pause boundary). Author: FastArrow.
Target: ORIGINAL_REFERENCE + shipped data. Read-only. Item-16 style: verified vs pending
stated per section.

## 1. CONFIRMED (verified twice, independent scripts): stream order

`GameWeapon::ReadAttackModeData` (0x4148A0) performs exactly **22 stream reads**:
`#1-11 u32, #12 u8, [v≥1] #13 u32 #14 u8 #15 u32 #16 u8 #17 u8, [v≥2] #18 u32,
[v≥3] #19 u32, [v≥4] string, [v≥5] #21 u8 #22 u8` — B9 §1's field list upgraded from
eyeball to mechanically counted (order + types + gates all match; the earlier
"21 fields" prose counted gates wrong by one — this table is the canon until slot work
finishes).

## 2. CONFIRMED: frame structure (IDA stack_frame for 0x4148A0, captured this pass)

Record block lives in frame locals **0x1c..0x4c** (13 contiguous dwords, `local_6c`…
`local_3c`) + `var_38@0x50` (the derived anim-clip slot — matches §2 of B9: the
0x414c60 switch writes it) + slots 0x54..0x5c (v≥1/2/3 fields) + std::string object
spanning ~0x58..0x73 (v≥4; overlap with #18/#19 slots is an OPEN question — see §4).
The FatalError's two operands sit at frame **0x4a / 0x4b = `local_3e`/`local_3d`** =
bytes 2 and 3 of the #10 dword's slot:
- `0x4b` (thrown): the ONLY thing that writes it is 0x414B61 (`setne` of a u8-read temp) —
  which v5 bool feeds it is §4's remaining ambiguity (#16 or #21/#22 normalization chain);
- `0x4a` (consumed): **never written by the parser** ⇒ it is the raw stream byte-2 of
  read #10, at FIXED body offset 12+38=50 from the chunk tag start (prefix is
  pre-string, size-invariant).

## 3. PROVISIONAL VERDICT (conditional on §4): 0/57 shipped records trip

Corrected-rule census (consumed = body byte 50-from-tag; thrown = last body byte):
pairs over 57 GWAM chunks = **{(0,0)×54, (0,1)×3}** → consumed==0 on EVERY record →
trip impossible for shipped data under this mapping. The B10 "3 candidates" were a
mapping artifact (tail-rule conflated slots) — retraction of that reading, self-caught,
as designed. Ledger entry 6 status: live code, **never fires on shipped records**;
reimpl mirrors the check (mirror-and-cite), and no crash-parity obligation exists.
CONDITIONAL: holds for any thrown-operand mapping (consumed=0 dominates), so it is
actually robust to §4's ambiguity — what §4 still gates is the *general* rule for
non-shipped/custom records, i.e. which byte lanes to read for the port's validator.

## 4. NOT STARTED / bugs found (so nobody trusts partial tooling)

- Full slot table (every rec+off ↔ wire #) — **not finished**. First esp-tracker had two
  real bugs: (a) objdump operand format (`lea eXx,DWORD PTR [esp+..]` no-space comma;
  `BYTE PTR` space bracket) broke regexes silently → empty outputs; (b) caller-cleanup
  accounting assumed `add esp,0xc` per read-call, but the string call (0x49C8D0) cleans
  `add esp,8` and at least one late read is NOT followed by a visible cleanup before the
  check block ⇒ ±8/±12 slot drift in the 0x54..0x74 region. Both fixed in spirit, but the
  remaining question is genuinely open: does the v≥4 std::string overwrite the v≥2/v≥3
  slots (0x58/0x5c) or do those live earlier — determines thrown-operand provenance.
- Creature-twin parity (0x40E473) — untouched.
- Do NOT reuse my first-run slot numbers from chat (rec+0x100/0x104… nonsense) — they
  carry the drift; this file supersedes them.

## 5. Resume note (one sitting, ~half done)
Next pass: read 0x414b61's temp source (which push feeds [esp+0x8c] at that point) →
closes §4b → promote §3 from conditional-robust to final → B9 slot table → creature twin.
Everything published here survives a reload: committed at its own ready-line.

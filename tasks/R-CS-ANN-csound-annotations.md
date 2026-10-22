---
status: done
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (PULL per coordinator order — seat re-granted at 4fdeb0f; sole IDB writer for the wave duration)
type: WRITER WAVE (annotation tier — comments ONLY, zero renames, T2 delta 0 by construction)
depends: R-CS §7.2–7.6 (own queue) + T-086 [E1] closure (the on-object alias note), R-ST close (baseline IDB state 00:34:53Z)
---

# R-CS-ANN — CSound epoch annotation batch (§7.2–7.6 + the [E1] on-object alias note)

## Queue (targets + text provenance = R-CS doc §7, byte-cited)
1. `JE::CSound::ctor_0x4C8100` (fn-head) — epoch portrait summary: vft 0x67140C, SSO name @+0x8,
   unique id [0x6FB0A4]++ -> +0x70, master-defaults triple +0x74/78/7C, self-registration tail
   0x4C82C1->0x52AEC0, SEH handler 0x632F5B; true size 0xF8 (14 heap sites + CMusic embed).
2. **0x4C8149 (instr) — THE [E1] ON-OBJECT ALIAS NOTE**: `mov %ecx,0x4(%esi)` seeds
   `[sound+4] = [[0x70B250]+0x50]` = cached ResourceSystem alias (T-086 [E1] CLOSED = R1;
   alias propagates into play records @0x4C7487; same route as CSoundFile slot0 via [master+0x50]).
3. `JE::CSound::dtor_0x4C7CF0` (fn-head) — unregister-first 0x52A930, master-GATE byte
   [system+0x48] before buffer frees, per-buffer vft-slot2 release, SSO free check; SEH same handler.
4. `0x52AEC0` / `0x52A930` (fn-heads) — CSoundSystem register/unregister pair: containers
   +0x2C keyed map, +0x54 list (0x50CA40 begin); called from CSound ctor tail / dtor head.
5. `0x4C8540` (fn-head, CSoundFile vft slot0) — demand-open path uses [[0x70B250]+0x50]:
   same alias route as CSound+0x4 (cite [E1]).
6. VFT data comments (6): 0x67140C (CSound base: slot roles), 0x67D1D8 (CMusic : CSound,
   slots 0-1 inherited verbatim / slot2 0x539570 override), 0x671450 (CSoundFile base,
   slot1 = scalar-deleting dtor), 0x67D1F0 (CMasterJumplist : CSoundFile), 0x67911C
   (CSoundPresetInfo : CSoundFile), 0x67915C (CSoundGroup : CSoundFile) — shared slot0
   0x4C8540 = INHERITANCE not same-object (§7.4's explicit confusion guard).
7. `cls_0x5b6400::ctor_0x5b6400` (fn-head) + 0x5B66BE (instr) — cursor = PLAIN CSound new(0xF8)
   (T-062 §1 construction byte closed; §7.5).
8. `0x4C7EB0` (fn-head) + 0x4C7D4A (instr) — master-gate byte census line: Play AND dtor both
   consume [system+0x48]; port must model master-DISABLED as SKIP, not object-absent (§7.6).

## Exclusions (named)
- §7.1 `JE::CSound::meth_0x539370` rename = MINT-tier (joins wave-3 bundle decision) — NOT applied.
- No semantic method names invented anywhere (comments cite EAs + roles only).

## Protocol (R-ST pattern, unchanged)
gate 6/6 @ pinned md5 first -> writer-seat verify -> save-sync BEFORE first comment (live==disk) ->
apply idc.set_cmt (APPEND-with-separator if any site is non-empty — never overwrite) -> get_cmt
read-back per site -> mid-wave checkpoint save -> end save + rstANN postdump + T2 delta-0 proof +
mtime -> ledger + row-flip + relay. Baseline note: IDB live at R-ST close (00:34:53Z),
T2(startswith cls_) = 1892.

## Acceptance evidence (CLOSED this commit)
Gate 6/6 before writes; pre-write save-sync 00:42:34Z (live==disk); 16/16 set + get_cmt read-back
(none under 40 chars); the single occupied site (0x5B6400) APPENDED with T-058 text intact (628-char
merge); 10,173 fns name-diff 0; **T2 1892->1892 delta 0**; checkpoint 00:43:28Z, END save 00:43:51Z;
dumps rstann-pre/post banked. Successor line: epoch debt-free beyond wave-3 bundle; 0x539370 rename
is comment-armed there; port-era SKIP-rule + voice-cap edges now on-object.

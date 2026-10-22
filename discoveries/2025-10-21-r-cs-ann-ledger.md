# R-CS-ANN — CSound epoch annotation wave ledger (writer: HappyStorm `01a16092`, claim 8e46fb6, close this commit)

IDB `artifacts/F3.exe.i64`: baseline mtime 00:34:53Z (== R-ST end-save, live==disk carried), pre-write
save-sync 00:42:34Z, checkpoint save 00:43:28Z, END save **00:43:51Z**. Gate 6/6 @ md5 `a68c22f3…`
before first write. Dumps: rstann-predump/rstann-postdump (10,173 fns; **name-diff 0**; T2-startswith
**1892 -> 1892, delta 0 by construction**).

## Landed (16 sites, get_cmt read-back 16/16, none under 40 chars)
| # | site | kind | content (queue ref) |
|---|---|---|---|
| 1 | 0x4C8100 | fn-head | ctor epoch portrait: full member map w/ EAs, size-0xF8 proof summary, SEH 0x632F5B (R-CS s1/s2/s6) |
| 2 | **0x4C8149** | instr | **THE [E1] ON-OBJECT ALIAS SEED** — [sound+4]=[[0x70B250]+0x50] cached ResourceSystem pointer; T-086 [E1]=R1 close cites the three cleared call-sites + record propagation 0x4C7487 |
| 3 | 0x4C7CF0 | fn-head | dtor order-of-ops: handle-release(alias) -> UNREGISTER-first -> master-GATE -> vft-slot2 buffer frees -> SSO check (R-CS s6) |
| 4 | 0x52AEC0 / 0x52A930 | fn-heads | CSoundSystem register/unregister pair, containers +0x2C keyed map / +0x54 list, callers named (R-CS s6) |
| 5 | 0x4C8540 | fn-head | CSoundFile slot0 demand-open = same alias family as CSound+0x4; [E1] cite (R-CS s7.3) |
| 6 | 0x67140C / 0x67D1D8 / 0x671450 / 0x67D1F0 / 0x67911C / 0x67915C | vft data ×6 | full inheritance tree: CSound(3 slots w/ roles), CMusic:CSound (slots 0/1 verbatim, slot2 override; + export-inflation pointer), CSoundFile (scalar-deleting slot1), CMasterJumplist/CSoundPresetInfo/CSoundGroup : CSoundFile — **shared slot0 = INHERITANCE NOT SAME-OBJECT** guard stated on each derived vft (R-CS s3/s7.4) |
| 7 | 0x5B6400 (+ 0x5B66BE instr) | APPENDED | cursor-as-plain-CSound close (new(0xF8)@0x5B66A5→ctor@0x5B66BE, vft 0x67140C, slot0 = the T-062 0x4C82E0 DEMAND loader); **append-never-overwrite held: T-058 embeds@ text intact at head, merge 628 chars** (R-CS s7.5) |
| 8 | 0x4C7EB0 + 0x4C7D4A | fn-head + instr | master-GATE byte pair: Play + dtor both consume [system+0x48]; **PORT RULE: master-DISABLED = SKIP, not missing-object** (R-CS s7.6) |

## Exclusions honored
§7.1 `meth_0x539370` rename — MINT-tier, wave-3 bundle (coordinator already relayed ST-RENAME there).
No method-name inventions; every claim carries its EA.

## Successor line
1. The CSound epoch is now self-documenting at portrait/alias/gate/inheritance tier — **no residual
   annotation debt in this family** beyond what R-ST's wave-3 bundle carries.
2. Cross-wave: if wave-3 re-labels 0x539370 (derived-in-base), the vft-comment at 0x67D1D8 already
   states the CMusic tail-field range it writes — cite-ready for the rename ledger.
3. Audio-port era: §7.6's SKIP rule + the voice-cap priority reads ([system+0x4C]) are the two
   behavioral edges the port must model; both are now on-object comments at the exact instructions.

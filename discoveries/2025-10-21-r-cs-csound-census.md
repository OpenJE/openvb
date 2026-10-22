# R-CS — CSound class census: portrait drawn. True size 0xF8; export tail = CMusic derived fields; the class tree is RTTI-complete; [E1] resolved as cached-alias from the holder end

**HappyStorm** `01a16092-41cf-7127-936b-377a304178a4`. PULL from PARK (claim 97dcc8b). Offline lane:
pinned objdump cache (EXE md5 `a68c22f3…`) + frozen `artifacts/F3.json` + `t064-postdump` live
names. **Zero IDB writes; zero 8745 queries.** Gate first: selftest **6/6 PASS** this sitting.

## 1. Instance size adjudication — TRUE SIZE 0xF8 (248), export 281 EXPLAINED, not just flagged

`JE::CSound::ctor_0x4c8100` call census (full-image `call 0x4c8100` scan, resolved targets):
**16 sites = 14 heap + 1 derived-embed + 1 alias-check**. Every heap site pairs
`push $0xf8 → new(0x6054B0) → ctor`:

| heap site | epoch | site | epoch |
|---|---|---|---|
| 0x43A996 | sub_43A960 (item/UI sound) | 0x4559E0 | sub_4554B0 |
| 0x43AA16 | sub_43A9E0 | 0x457E07 | CWorldContainerInterface::ctor_0x457DB0 |
| 0x44045D | CLoadMapInterface::ctor_0x440050 | 0x52DB54 | cls_0x52E2E0::meth_0x52D820 |
| 0x4415D5 | CMessageBoxInterface::ctor_0x441420 | 0x580491/54F/600/6A6 | CEngineInterface::virt_meth_0x5803E0 ×4 |
| 0x44160E | CMessageBoxInterface::ctor (`push $0x65866c` = **"f3_messagebox_open.wav"** — name set adjacent) | 0x5B66BE | cls_0x5b6400::ctor — **the cursor CSound** (0xF8, vft 0x67140C, slot0 = the 0x4c82e0 loader T-062 walked — cursor object closes as a PLAIN CSound instance) |
| 0x443166 | sub_443130 | embed: 0x539033 | **CMusic::ctor_0x539030** — calls CSound::ctor on itself (base @+0) |

**The inflation is decoded, not hand-waved**: export members at 0xF8/0xFC/0x100/0x104/0x108/0x10C/0x115
(7 tail fields, 1-4 usages each) are EXACTLY CMusic's derived tail — CMusic::ctor writes +0xF8
(handle), +0xFC/0x100/0x104/0x108 (dwords), +0x10C/+0x115 (bytes) after the base ctor returns
(0x53903A-0x539064). OOAnalyzer merged the derived object into the base's portrait — T-062's
"inflation family" is this shape. True CSound extent: last ctor write +0xF4 (500 default),
allocation 0xF8 ⇒ **size 0xF8 = 248** (every heap instance; export 281 is CMusic-span noise).

## 2. Member anatomy (ctor_0x4C8100 writes + verified consumers; export as locator only)

| off | init (ctor byte) | role (cited consumer) |
|---|---|---|
| +0x00 | vft 0x67140C (0x4C811F) | the 3-slot vft (§3) |
| +0x04 | `= [[0x70B250]+0x50]` (0x4C8149) | **ResourceSystem cached alias** — see §6 [E1] |
| +0x08..+0x20 | basic_string SSO: buf@+0x0C, size@+0x1C, cap@+0x20=0xF (0x4C8127-35), assigned from arg string (0x4C8161 call 0x401BD0) | sound name ("f3_messagebox_open.wav" pattern) |
| +0x24 | 0 → handle | slot0 checks it (0x4C8312 `mov [esi+0x24]`); dtor releases via 0x49B1D0(ecx=[+4]) |
| +0x28 | mode int (slot2 switch 2/3/4, 0x4C70B3) | seek/decode mode |
| +0x2C..+0x38 | zeroed | stream/flags cluster (+0x34 state bits: &2 loading, &0x10 playing-armed — slot0/slot1 tests) |
| +0x3C/+0x40/+0x44 | zeroed | **buffer object ptrs**, dtor frees each via own-vft slot2 (0x4C7D56 `call *0x8(...)` x3, master-gated) |
| +0x48 | 0 | pad/flag |
| +0x50 | 0; +0x54=1000.0f; +0x58=0.001f | pitch/rate defaults pair |
| +0x5C..+0x68 | zeroed | instance bookkeeping (+0x68 = stream FILE* — slot2 case 0x3 seeks it via 0x5FEB80) |
| +0x6C | 0 → 16-byte record ptr `new(0x10)→ctor_0x4C6FB0` (0x4C746A) | play-record {+0 = [sound+4] alias COPY (0x4C7487), +4 param, +8 handle, +C result}; +0x6C chain to CSoundSystem maps |
| +0x70 | `[0x6FB0A4]++` (0x4C81BB-C9) | **unique sound ID** from global counter |
| +0x74/78/7C | `[master+0x1C/0x20/0x24]` (0x4C818A-9C) | inherited master defaults (volume/category triple) |
| +0x80 | 100 | count default |
| +0x84 | `call 0x4C7A80` result | sub-struct handle (SHARING row §5: hosts read through +0x34/+0x40 views) |
| +0x88..+0x90 | 100 | the four +0x88/8C/90 = per-class channel defaults (matches the [B]-case view constants +0x8C/+0x90 in T-064 — DIFFERENT object, same offsets: coincidence, NOT the same field — record in the writer note) |
| +0x94..+0xB8 | zeroed / +0xB0/0xB4=100 | preset/streaming block |
| +0xBC..+0xC4 | zeroed | stream state |
| +0xC8 | 100; +0xCC `call 0x4C6B70` | preset id/extension cache |
| +0xD0 | 100; +0xD4 zeroed (**Get_mbr_0xd4 accessor 0x4C6BA0**, live-named); +0xD8 `call 0x4C6BA0` result; +0xDC | tag/type quad (see §4 tag sniffer) |
| +0xE0/+0xE4 | 100; +0xE8/EC zero | chunk params |
| +0xF0/+0xF4 | 500, 500 | timeout/length defaults (last in-object field ⇒ size 0xF8) |

## 3. Vft anatomy + FULL RTTI CLASS TREE (byte-verified VC6 COL at vft[-4], TD = COL+0xC)

| class | vft | COL | TD | slots (byte-read) | slot semantics (body-verified) |
|---|---|---|---|---|---|
| CSound | 0x67140C | 0x6D4098 | .?AVCSound@@ | [0x4C82E0, 0x4C8070, 0x4C70B0] | slot0 = LOAD status state-machine (+0x34&2→2, handle→7; idempotence-bail + the 0x49B9F0 DEMAND path T-062 walked; T-061 §11 cursor edge); slot1 = PLAY control (armed bit +0x34&0x10, **priority vs [master+0x4C] voice cap** @0x4C8084, delegates Play 0x4C7EB0(+0x30, +0xA4) then state setters); slot2 = SEEK dispatch (mode +0x28 ∈ {2,3,4}, FILE* +0x68 via 0x5FEB80) |
| **CMusic : CSound** | 0x67D1D8 | 0x6D5C10 | .?AVCMusic@@ | [0x4C82E0, 0x4C8070, **0x539570**] | slots 0-1 INHERITED verbatim, slot2 OVERRIDDEN — textbook single inheritance; adds tail fields §1; ctor embeds CSound @+0 |
| CSoundFile | 0x671450 | 0x6D40DC | .?AVCSoundFile@@ | [0x4C8540, **0x6162E5**] | slot0 = demand-open via `[[0x70B250]+0x50]` (the alias, §6); slot1 = operator-delete-sized ⇒ **scalar deleting dtor** shape |
| **CMasterJumplist : CSoundFile** | 0x67D1F0 | 0x6D5C58 | .?AVCMasterJumplist@@ | [0x4C8540, 0x539A00] | ctor 0x539120 chains base CSoundFile::ctor(0x4C84C0) then overwrites vft + ModelInstance-tier embed at +0x4C (0x56E0B0 — selftest T2b known-positive) |
| **CSoundPresetInfo : CSoundFile** | 0x67911C | — | .?AVCSoundPresetInfo@@ | [0x4C8540, 0x52D550] | derived override |
| **CSoundGroup : CSoundFile** | 0x67915C | — | .?AVCSoundGroup@@ | [0x4C8540, 0x52DD00] | derived override |
| CSoundSystem | 0x678C2C | 0x6D555C | .?AVCSoundSystem@@ | [0x52ADF0] | singleton §6 |

CJumplist (0x538B70 ctor/0x538BB0 dtor/0x538BF0 virt) — list machinery in the same epoch; RTTI
.?AVCJumplist@@ present (0x6FECC0). **All six class names carry RTTI** — identity here never needs
flow-tier proof (the census claim type), which is why the share-check print §5 cannot threaten it.

## 4. The 0x539xxx band classification (T-062 'named-but-didn't-portrait' — resolved)

The band is the **music/jumplist epoch of this exact class tree**, not a separate mystery:
CMusic 0x539030/0x539070/0x539570, CMasterJumplist 0x539120/0x539190/0x539210/0x539A00,
CJumplist 0x538B70-D0, list clusters cls_0x539D90/0x539FB0/0x53A150/0x53A1E0 (jumplist containers),
cls_0x53A510 + Gfx_Engine::meth 0x53A220-0x53A720 (engine audio-glue adjacency), SEH pads
0x539080-0x539110 family (non-head chunks, item-43 note). **`JE::CSound::meth_0x539370` is a
misattribution**: its body reads `[this+0x34]&2`, calls `sub_5390D0([[0x70B250]+0x1F4], arg)` and
writes **[this+0xF8]/[this+0x100] — CMusic tail fields ⇒ it is a CMusic method** wearing the base
label (same axis as T-064's composite catch, inverse direction: derived fn landed in base class).
Writer queue item §7. Tag sniffer (0x4C6C30): walks a NUL string, after the LAST '.' copies the
3 chars into a local buffer (`cmpb $0x2e` + `cmp $0x3` — extension extractor: 'wav'/'ogg'),
feeding the +0xD4 region's type cache; 0x4C6BB0 is the master timeout-loop probe (calls stream
vft *0x24/*0x50, waits `timeGetTime`-style 0x88780096 deltas, sleep 10) — census role notes, no
semantic renaming invented.

## 5. share-check print @90864fd over the 41-fn band (census tier, machine speaks)

Verdict lines: ctor 0x4C8100 **SINGLE-OWNER** (heap=14, ctor-sites=10); 0x4C7670/0x539370/0x4C76A0
SINGLE-OWNER; ~20 NO-EDGE (intra-band/vft-dispatch consumers — the class is used through its VFT,
so direct-call edges are sparse by design); dtor 0x4C7CF0 / Play 0x4C7EB0 / 0x4C7DD0 / 0x4C79E0
**SHARING** with holder fields {[host]+0x8, +0x34, +0x3C, +0x40} and views `this+0x40` — i.e.,
**many hosts own their OWN sound instances at their own fields** (CEngineInterface, LoadMap,
WorldContainer, Gregorian, CSoundSystem-adjacent cls_0x590990…). BAND VERDICT: SHARING (exit 1) —
decoded as **the same independent-class multi-host-ownership pattern as T-086 §4.1
GfxCameraHolder**: identity is RTTI-anchored (its OWN TD/COL chain, §3), no membership claim was
ever made, so the print is INFORMATION (a sound class used across the UI by design), NOT a
flip-list item. The census does not mint, does not claim membership, and the machine agrees with
the construction tier (14 hosts × new(0xF8) = the class's purpose).

## 6. Allocator/cleanup chain + the [E1] resolution (T-086 escalation CLOSED from the holder end)

- **Master singleton**: `[0x70B250] = CSoundSystem = new(0x204)→ctor_0x52ABF0` installed at 0x52AD45
  (installer also runs `sub_52A2E0(2,2,0x5622,0x10,arg)` + **seeds `[master+0x50]` via setter
  0x52A010 with its own boot argument** @0x52AD73 — boot hands CSoundSystem the ResourceSystem
  pointer; that boot caller is the writer's one remaining hop, census-complete here).
  `sub_52A960` zeroes `[0x70B250]` at teardown.
- **CSound ctor self-registration**: tail `call [[0x70B250]]; 0x52AEC0(this)` — appends into
  `[system+0x2C]` keyed map (0x56E0B0/0x4B9D60 tier) + `[system+0x54]` list (0x50CA40 begin);
  **dtor unregisters first** (0x4C7D3C `call 0x52A930`) then runs the master-gated release.
- **Cleanup**: dtor 0x4C7CF0 = vft-restore → handle release `0x49B1D0(ecx=[sound+4])` →
  unregister → **gate: `[master] && byte[master+0x48]` (the T-062 master-gate byte)** → free
  buffers +0x44/+0x40/+0x3C via their vft slot2 → free+dtor [sound+0x38] (0x4C68D0 + 0x6162E0) →
  SSO string free check (cap≥0x10 → free [sound+0x0C]) → SEH handler 0x632F5B both ends (T-059
  EH-call taxonomy hit: CSound ctor/dtor are SEH-wrapped pairs like the GUI-interface family).
- **[E1] = R1 CONFIRMED (cached alias, NOT helper-theft)**: the CSound-epoch sites T-086 printed
  (0x4C74AD/4C74C8 → 0x498800, 0x4C749E/4C74B2 → 0x4989C0, 0x4C6E6A..0x4C70DB → 0x499220,
  0x4C8596 → 0x498EB0) all load `ecx = [sound+0x4]` (or the record's COPY of it at record+0x00,
  byte 0x4C7487) — and [sound+0x4] is seeded **by the ctor itself @0x4C8149 from
  [master+0x50]**, which boot seeds with the ResourceSystem pointer. So the SAME singleton instance
  is reached through two routes: the global (`fg:0x707DA8`, the 329-path census) and this
  per-object cache — the T-054 `byte_707CE0 self-cache` precedent generalized. **The three
  ResourceSystem candidates are NOT theft suspects; the band stands; E1 closes R1.** (The alias
  even propagates into the 16-byte play records — the pattern is systemic by design.)

## 7. Writer-queue items (annotation tier unless stated; nothing applied from this lane)
1. `JE::CSound::meth_0x539370` → CMusic-method comment or `JE::CMusic::meth_0x539370` rename
   (derived-in-base mislabel; byte proof §4; expected T2 delta: −1 IF renamed).
2. ctor/dtor SEH note (handler 0x632F5B) + the register/unregister pair comment at 0x52AEC0/0x52A930
   (CSoundSystem containers +0x2C/+0x54).
3. `[sound+0x4] = cached ResourceSystem alias` member comment at +0x04 across the class tree
   (CSoundFile::slot0 0x4C8540 uses the same route via [master+0x50]) + cross-cite to T-086 E1
   closure so the record shows WHY the three candidates were cleared.
4. class-tree note (CMusic:CSound, CMasterJumplist/CSoundPresetInfo/CSoundGroup:CSoundFile) on the
   six vfts — slot0-0x4C8540 sharing is INHERITANCE, not the same object.
5. cursor CSound note at 0x5B66BE: cursor object = plain CSound (new(0xF8)) — closes T-062 §1 with
   the construction byte; GameCursorManager holds it.
6. master-gate byte `[master+0x48]` census row for the audio-port era (Play 0x4C7EB0 + dtor gate
   both consume it; port must model master-disabled as SKIP, not as missing object).

## 8. Coverage + honesty
Sources: export class tables (native tools), pinned objdump cache (all bodies/bytes quoted above),
t064-postdump live names, raw .rdata/.data RTTI walks, van-Buren .veg lineage untouched this row.
Named gaps (not padded): boot caller of the CSoundSystem installer (who supplies [master+0x50]) not
traced — one xref hop, writer-grade query; +0x88/8C/90 channel-default naming is offset-descriptive
only; 0x4C6BA0's ctor use stores [this+0xD4] before any visible write of +0xD4 — recorded as an
observation for the writer's re-read, no invented semantics; cls_0x52E2E0/cls_0x539xxx cluster
member tables not fully drawn (census scoped to the CSound/CSoundFile trees, the clusters are the
jumplist containers around them). Zero writes anywhere; reservations: task file + this doc.

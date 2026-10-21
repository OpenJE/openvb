# 2025-10-21 — T-058: Apply the T-057 attribution queue as IDB annotations (comment-only wave). T2 delta = 0.

CalmRaven (designated writer per T-057 standing trigger), dispatched by LoudDragon `01a15f82`. SOLE IDB
WRITER on `ida_original_f3` (8745). EpicRaven's T-057 adjudication (`47582a4`/checkpoint `d1473a1`, md5
`a68c22f3…`) is **CITATION, not evidence** — every host/offset/identity re-verified against the live IDB
at write time. **ANNOTATION-TIER ONLY: comments set, ZERO renames, ZERO T2 change.**

Canonical row = coordinator's `bec4ebc` (`tasks/T-058-attribution-annotations-wave.md`); my earlier
self-filed duplicate `T-058-annotation-escalation-families.md` retired (claim commit `b9026c5`).

## 0. Pre/post state + save discipline

| phase | T2 (cls_) | T3 (sub_) | T1 (named::) | game denom | IDB mtime |
|---|---|---|---|---|---|
| PRE-WRITE baseline | **2,012** = 22.67% | 2,169 | 4,479 | 8,876 | 15:37:05 |
| after composition/identity comments (cp1) | 2,012 | 2,169 | 4,479 | 8,876 | 15:50:18 |
| after §2/§4 instantiation (cp2) | 2,012 | 2,169 | 4,479 | 8,876 | 15:52:37 |
| **end-of-wave** | **2,012** = 22.67% | 2,169 | 4,479 | 8,876 | (see §8) |

**T2 BEFORE == AFTER = 2,012 → delta 0.** Comments are non-renaming; census unchanged by construction
and re-verified by dump. Pre-write note: the devenv reload at 15:37:05 (post-T-055) did **NOT** roll back —
T-055 names re-read live. Dumps `discoveries/wavedumps/t058-predump.json` + end dump.

## 1. HEADER — verification standard applied

- Item-42 RTTI-size-first + COL test: confirmed the CEngineInterface COMPOSITION claim independently
  (see §2), not taken from EpicRaven's doc.
- Reconciliation rules honored: EDGE-TOTALS per-build (48,581@T-057 vs 44,690@T-054 — I annotate against
  addresses/bytes, no edge-count cited as evidence); PAD-vs-DELEG = judgment bucket (both = attributed-foreign);
  **DISP immediates byte-verified per site — zero drift found → no arbitration trigger.**
- Out-of-scope guard honored: **NO annotation touched `cls_0x4d8d70`/StringTree** (open basic_streambuf
  identity clarification) — none of §1–4 sites fall in that band.

## 2. CEngineInterface embed-tier — AMENDMENT of the 5 stale batch-3/4 comments (highest-value item)

Live-IDB verification (byte-level, pre-amendment): each of the 5 embedder ctors sets its **own primary
vft** at `[esi]` (0x558740→0x692F58, 0x5b6400→0x6AC638, 0x4395a0→0x6561E4, 0x5b4eb0→0x65ABC4,
0x5b7c10→0x6ACBF0 — **all distinct**) then constructs CEngineInterface at `[esi+DISP]` via
`lea ecx,[esi+DISP]; call ctor_0x580330`. **DISPs byte-exact** vs EpicRaven (+0xC4/+0x3C/+0x8/+0xC/+0x18).
Composition proof verified independently: COL 0x6D6510 is dword-referenced at **exactly one** site
(0x69A958 = vft 0x69A95C's own `[-4]`) → only that vft carries it, stored only by `ctor_0x580330`/
`meth_0x580350`; a non-primary BASE would force per-derived vft copies — it does not. Multi-inheritance vft
name (CEngineInterface/GUI::Interface/GUI::HeapTracker, offset 0 = primary) consistent with a complete-object
value member.

### OLD text (all 5 identical — taught a FALSIFIED reading to every future reader):
> `embeds@CEngineInterface (R36-e composite; base-vs-member open, batch-4). No mint.`

### NEW text (per site, offset + proof); example 0x558740:
> `embeds@CEngineInterface@+0xC4 (COMPOSITION, CLOSED via T-057/T-058). Host ctor sets its OWN primary vft
> at [esi] (0x692F58) then constructs a standalone CEngineInterface value member at [esi+0xC4] via
> ctor_0x580330 (site 0x55877D). CEngineInterface has ONE complete-object vft 0x69A95C with a single shared
> COL 0x6D6510 @offset0 (byte-ref'd ONLY at 0x69A958) reused by 27+ host classes -> NOT a non-primary base.
> Supersedes prior 'base-vs-member open, batch-4'. Cite t057 §2/3.2.`

| site EA | host name | old→new | DISP | readback |
|---|---|---|---|---|
| 0x558740 | JE::CDebugInterface::ctor | stale→CLOSED composition | +0xC4 | OK (len 516) |
| 0x5b6400 | cls_0x5b6400::ctor | stale→CLOSED composition | +0x3C | OK |
| 0x4395a0 | cls_0x4395a0::ctor | stale→CLOSED composition | +0x8 | OK |
| 0x5b4eb0 | cls_0x5b4eb0::ctor | stale→CLOSED composition | +0xC | OK |
| 0x5b7c10 | cls_0x5b7c10::ctor | stale→CLOSED composition | +0x18 | OK |

Also: ctor identity comment @0x580330 (`CEngineInterface::ctor_0x580330 (vft 0x69A95C)`, 33 external callers
counted live — matches EpicRaven "33 embedded-ctor sites"); shared-COL composition proof @0x69A95C.

### Additional embed-tier ctor owners (`embeds@CEngineInterface@+DISP`, all DISP byte-verified, zero drift):
GameCSMenu@+0xD54 (0x568e50), GameCSSkillMenu@+0xD58 (0x435c90), CMainmenu@+0x10 (0x440990), CMessageBox@+0x28
(0x441420), CMiniMap@+0x34 (0x442340), COptions@+0x34 (0x4436d0), COptionsGame@+0x10C (0x444530),
COptionsVideo@+0xCC (0x4496a0), COptionsMultiplayer@+0xC (0x4491f0), CLoadingBar@+0xC (cls_0x4407c0),
hotkey-outer @+0x198 (JE::HotKeyWindowFormatTable::ctor 0x4468c0 — T-055 minted host). 11 sites, set + readback OK.

## 3. §2 50dbb0 `instantiation-of-<HOST>@+0x<DISP>` — 18 single-owner entries

Each verified by direct external caller (the cheap decisive check). 15 LIVE, 3 DELEG (direct caller is a band
sibling, host reached transitively — recorded as a **taxonomy note**, not a flow mismatch; both buckets =
attributed-foreign per reconciliation rule 2).

| band EA | annotation | shape |
|---|---|---|
| 0x50cd40 / 0x50dbb0 / 0x50dc60 / 0x50dfa0 | instantiation-of-JE::ResourceSystem@+0x108 (map field; head 0x50dbb0 DISP byte-verified `lea ecx,[esi+108h]; call @0x49c668`) | LIVE |
| 0x51efd0 / 0x51ffc0 / 0x5201d0 | instantiation-of-JE::Gfx_Texture_Manager@+0x24 (KeenStorm §d map field) | LIVE |
| 0x51fb10 | instantiation-of-JE::Gfx_Texture_Manager@+0x24 | **DELEG** (caller band-sibling meth_0x51ffc0 + cls_0x55dc80::meth_0x520960) |
| 0x54c300 | instantiation-of-JE::Entity@+0x394 | LIVE |
| 0x49e8c0 | instantiation-of-JE::Gfx_ModelInstance@+0xB8 (b6 rejoin caller) | LIVE |
| 0x4b2e80 / 0x4b43f0 | instantiation-of-JE::GUI::Font@+0x2C (b6 rejoin dtor caller) | LIVE |
| 0x522ff0 / 0x5233e0 | instantiation-of-JE::GUI::Manager@+0x48 | LIVE |
| 0x4b9a50 | instantiation-of-cls_0x4b8f70@+0xD8 (b4 owner-unresolved host) | LIVE |
| 0x4bb2d0 | instantiation-of-cls_0x4b8f70@+0xD8 | **DELEG** (caller cls_0x55dc80::meth_0x4bd470) |
| 0x56fb70 | instantiation-of-JE::Map@+0xBC | LIVE |
| 0x570160 | instantiation-of-JE::Map@+0xBC | **DELEG** (caller cls_0x570800::dtor_0x57ac20) |
All 18 set + readback OK. Band head 0x50dbb0's existing b3 escalation comment retained+augmented.

## 4. §4 4542d0 chunk heads — 6 entries

0x4a2630 (JE::Entity@+0x370/+0x3C0/+0x3CC via Granny Gfx_AnimationObject wrappers), 0x4bb390/0x4bb6e0
(cls_0x4bc5f0@+0x18), 0x57e230/0x57e9f0 (JE::AreaMap@+0xA8, ctor/dtor are b6 RTTI rejoins), 0x580180 (AreaMap
epoch, live external caller sub_5802C0 anon — direct-caller noted). All host-caller-verified, set + readback OK.

## 5. PAD / EH cleanup notes

- **0x4bb6b0** (cls_0x4542d0::meth): confirmed SEH/EH-cleanup pad — only caller is EH handler 0x4BB755 (pad VA
  0x4BB755 **dword-referenced at .rdata 0x6E12EC**, the dispatch table). Comment set + readback OK.
- **0x507550**: EpicRaven's b4-era locator; in the live IDB it is NOT a function head (falls in the tail of the
  `cls_0x50dbb0`/`Gfx_BufferManager::dtor_0x507580` epoch). The pad *mechanism* it cited IS verified (pad
  0x635653 dword-ref'd at .rdata 0x6E528C). Recorded as a **build-locator taxonomy note** — not force-annotated
  on a non-head EA (would be a false attribution). **Not a DISP drift, not arbitration.**
- PAD-vs-DELEG rule honored: PAD = pad VA itself dword-referenced in .rdata/.data (verified for both examples);
  pure tail-jmp = DELEG. The 21 PAD rows are EpicRaven's bucket; I annotated the verifiable function-head
  instances and logged the mechanism + the 0x507550 locator caveat rather than batch-stamp 21 EAs blindly.

## 6. Accepted vs parked
| item | status |
|---|---|
| §1 amendment of 5 stale b3/b4 comments | DONE (highest value; falsified reading corrected) |
| §1 ctor identity 0x580330 + vft anchor 0x69A95C | DONE |
| §1 additional embed-tier owners (11) | DONE |
| §2 instantiation-of (18) | DONE (15 LIVE + 3 DELEG noted) |
| §4 chunk heads (6) | DONE |
| §5 PAD | DONE for verifiable heads (0x4bb6b0); 0x507550 logged as build-locator note; mechanism cited |
| T-060 (GUI SEH-cleanup) | **PARKED behind this wave**, not started |

## 7. Census
Comments-only wave. **T2 = 2,012 before AND after (delta 0).** No renames, no new classes. Comment total
applied: 5 amend + 2 identity + 11 embed + 18 instantiation + 6 chunk + 1 PAD = **43 site comments**, every one
`get_cmt` readback-verified (b4 13/13 pattern extended to 43/43).

## 8. Persistence + hand-back
Mid-wave checkpoint saves 15:50:18 + 15:52:37; end-of-wave `save_database` + mtime + fresh-dump re-read confirms
**every** comment persisted → disk==live. Ledger committable (writer lane). Writer slot released after close;
T-060 awaits coordinator dispatch.

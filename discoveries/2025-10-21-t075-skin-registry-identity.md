# 2025-10-21 — T-075: the global @0x707E48 is the `GUI::Manager` singleton (RTTI-joined) — cls_0x4b28a0 band MERGE-READY

TrueCastle, sessionId `01a16096-c00e-7404-8afe-e521a9bb7240`, dispatched by LoudDragon `01a15f82`.
Target ORIGINAL_REFERENCE (`ida_original_f3` :8745, `artifacts/F3.exe.i64`). **READ-ONLY: zero IDB
writes** — every claim here is anchored to raw EXE bytes of the md5-pinned demo
(`a68c22f3dd1c256ec9539a70e87cb434`, verified this wave) plus the frozen `artifacts/F3.json` export.
Tooling: `tools/ecx-flow.py` **selftest 5/5 PASS before any verdict** (T1 Gfx_D3D, T2 AnimMesh,
T2b embed, T3 Texture_Manager, T4 PAD snapshot).

## 0. Verdict

**MINT-READY — with single-owner proof.** The 188-byte static object at `0x707E48` is the complete
object of class **`GUI::Manager`** (shipped RTTI string `.?AVManager@GUI@@`; live-DB convention
`JE::GUI::Manager`). The anonymous band `cls_0x4b28a0` (28 fns, size 188, no vft) and the RTTI class
`.?AVManager@GUI@@` (5 methods incl. `ctor_0x4b5640`, `dtor_0x4b5280`, vft `0x66f2cc`, size **188**)
are **the same class split by OOAnalyzer across two keys** — the b5 §1a `cls_0x502870`/`cls_0x490060`
precedent, and the exact artifact T-054 named as the *only* missing join. Writer action: rename the
27 real band fns into the live `JE::GUI::Manager` epoch (§7) — no fresh name is minted; the epoch is
already half-live-named, which resolves the T-055 cousin-risk in our favor.

## 1. The RTTI chain (attack step 1 — the headline join)

Primary-vft store, register-indirect — **this is why every prior absolute scan found "no writer"**:

- `0x4b5640` (`JE::GUI::Manager::ctor_0x4b5640`, live-named): at **`0x4b5669`** `c7 06 cc f2 66 00`
  = `mov dword ptr [esi], 0x66f2cc` with `esi = this`.
- vft `0x66f2cc`, slot0 = `0x4b58b0` (`JE::GUI::Manager::virt_meth_0x4b58b0`, live; **the vft's only
  slot** — vftable-master-index row `GUI::Manager 0x66f2cc (1 slot)` already registered it).
- `vft[-4] = [0x66f2c8] = 0x6d3dc8` → **COL**: `{0, 0, 0, TD=0x6faf80, CHD=0x6d3db8, …}`.
- TD `0x6faf80`: `{pMD 0x6fafac, spare 0, name}` → name @`0x6faf88` = **`.?AVManager@GUI@@`**
  (demangles `GUI::Manager`). pMD-encoded, VS2003-style RTTI.
- CHD `0x6d3db8` = `{sig 0, attrs 0, maxNodes 1, BaseArray 0x6d3db0}`; BaseArray[0] = MTD
  `0x6d3d98` whose +0 dword **is TD `0x6faf80` again** (self-describing) with CDOffset 0,
  pGetTop 0xffffffff ⇒ single-node hierarchy, **no base classes**.
- COL `0x6d3dc8` is referenced from exactly one site: the vft's own `[-4]` (`0x66f2c8`) — the T-057
  one-vft proof shape. TD `0x6faf80` is referenced only from MTD `0x6d3d98` and COL `+0xC`.

**Adjacent-table discrimination (the view trap, item 42):** `.rdata` packs three tables shoulder to
shoulder — `0x66f2cc` Manager vft (1 slot), `0x66f2d4` `GUI::Font` vft (`{0x4b5580, …}`, COL
`0x66f2d0→0x6d3e0c→ .?AVFont@GUI@@`), `0x66f2dc` `GUI::Skin` vft (4 slots
`{0x4b5b90, 0x522b10, 0x522c30, 0x5226a0}`, COL `0x66f2d8→0x6d3e50→ .?AVSkin@GUI@@`). The naive read
"Manager vft slot1 = 0x6d3e0c" is wrong: **0x6d3e0c is Font's `vft[-4]`**. Each of Font/Skin has its
own {ctor,dtor} vft-store pair (`0x4b551f/0x4b55c3`, `0x4b5b35/0x4b5bcf` — all `c7 0x` forms), i.e.
they are separate classes instantiated elsewhere (Font `new(0x38)`→ctor `0x4b5500` @0x4b5a52, etc.),
exactly the registry relationship the board calls "SKIN-REGISTRY". None of this touches our object:
`0x66f2cc` is stored at `this+0` **only** by `ctor_0x4b5640` and by `dtor_0x4b5280` (`0x4b52a3`,
vft-restore, same class — not a second owner). Item-42 owner-vs-view test: the RTTI record is
**non-zero size (188)**, CDOffset 0, single-node complete-object locator of the primary vft ⇒ the name
is the OWNER type, not a view. (Contrast the dismissed `.?AVCOptionsHotKeysInterface@@` size-0 case.)

## 2. Single-writer store census, done right (attack step 2)

Scanner self-test on the known-positive pointer global `0x707CE4` (Gfx_Engine, t007 §G2a "exactly
three writers"): raw scan reproduced all three — `a3` @`0x5bfe33`, `c7 05` @`0x5bedd6`, `89 1d`
@`0x5c1e24`, and nothing else (0 `89 05/15/25/2d/35`). Scanner is proven hot before trusting negatives.

Census over all of `.text` (0x401000..0x646c52) for **every** absolute encoding of `0x707E48`:
- stores: `a3`, `89 /r`, `c7 05`, `88 05`, `c6 05`, `66 c7 05`, `0f 11 05`, FPU `d9/dd 1d` → **0**;
- loads/cmps: `8b /r`, `3b/39 /r`, `f6/f7 05`, `0f b6 05`, `0f 10 05` → **0** (nobody even dereferences
  the vft — dispatch is 100% direct `thiscall`, which also explains T-054's "no vft slot points into
  the band": the 1-slot vft exists but is dead as a dispatch path);
- `lea [abs]` (all `8d` modrm), `mov reg,imm` (`bf/be/ba`), `push imm` (`68`) → **0**;
- `mov ecx, imm32=0x707E48` (`b9`) → **168 sites** — the ONLY absolute encoding of this address in the
  whole image, and each is a `this` load (see §4; matches ecx-flow `this==&global` edges).
- delta-census `0x707E48+0x04..+0xC8`: only 14 absolute refs, all field *accesses from other classes*
  (+0x10 Button@0x4a6dc4, +0x8c/+0xb0/+0xb4 GUI widget readers, +0xb8 `JE::CLoadMapInterface::meth_0x495220`),
  plus 4 `a3` stores to **0x707F08…0x707F14** (adjacent plain globals with getter/setter fns at
  0x4b81a0.., not our object — image span check: object ends 0x707F04).
- data-section scan (.rdata+.data+.data1, dword-aligned): **0** dwords hold any of
  `0x707E48..0x707F08` ⇒ zero holder fields, zero COL/slot pointers to the object; **0** vft slots
  point into `cls_0x4b28a0`'s 28 fn range (re-answers item-41 query (c) with 345 tables + raw bytes).

Image bytes `0x707E48..0x707F48` are all zero (re-verified) ⇒ runtime-init only.

**Harness lesson (new sub-rule, item 8c family):** an absolute-encoding store census is BLIND to the
`mov ecx,&obj; call ctor; ctor: mov [ecx], imm32(vft)` shape. T-054's "zero vft stores at [0x707E48]"
was a scanner artifact, not a fact — the census must always be paired with a callee-side `c7 0x/89 0x`
imm-table scan inside every fn reachable from a `this==&global` edge. This is what unblocked a 21-wave
name-block.

## 3. Construction site = exactly one (singleton proof)

`0x641660` 6-byte prologue-less thunk: `b9 48 7e 70 00` (mov ecx, 0x707E48) → `e8` **`call
0x4b5640`** → `68 e0 5d 64 00` (push `0x645de0`) → `call 0x61700a` (atexit) → ret. Its only "caller"
is the **CRT static-init table**: `[0x6f1450] = 0x641660`, inside the contiguous table
`0x6f1410..0x6f148c+` (slots 0x641390…0x641800 — sibling static-object ctors like 0x641680→0x707F28,
0x6416a0→0x707F1C). The atexit target `0x645de0` = `jmp 0x4b5280` (dtor §5). ctor_0x4b5640 has exactly
ONE call site image-wide. No heap ctor edge: `new`→ctor census empty; `this==*global` edges 0;
embed-into-global 0 (ecx-flow `global 0x707E48`: 168 / 0 / 0). ⇒ one instance, one storage site,
never aliased.

## 4. Membership purity + item-44 SINGLE-OWNER machine test

`ecx-flow band --cls cls_0x4b28a0` (this wave): **[A] 142/142** band-call edges
`this==&0x707E48`; **[B] 0** thunks resolving into band; **[C] 0/345** vft slots into band. Raw
`b9`-site walk agrees: 168 loads, targets listed in §8. Item-44 test on {RTTI name `GUI::Manager`,
vft `0x66f2cc`, method set}: reachable from exactly ONE view constant (`&0x707E48`, primary,
CDOffset 0) and ZERO holder fields (dword scan §2). The Font/Skin COLs are separate locators of
separate classes (§1); t058's `instantiation-of-JE::GUI::Manager@+0x48` row (0x522ff0/0x5233e0) is
the object's own **member-at-+0x48** (a `cls_0x50dbb0` `_Tree` container the ctor seeds via
`0x4c6640` @0x4b56cc; band thunk `sub_4B2AA0 = add ecx,0x48; jmp 0x523320` forwards into it) — not a
second Manager instance, since no other code stores `0x66f2cc` anywhere. **PASS — single owner.**

## 5. `0x4b5280` is `GUI::Manager::dtor`, not `JE::StringTree::dtor` (LIVE label to fix)

Live IDB: `JE::StringTree::dtor_0x4b5280`. Falsified byte-level: at `0x4b52a3` it stores the **Manager**
vft `c7 06 cc f2 66 00` at `this+0` (a StringTree dtor would restore a StringTree vft); its only caller
is `0x4b58b3` inside `JE::GUI::Manager::virt_meth_0x4b58b0` = the vft's slot0 = scalar-deleting dtor
(body: `call 0x4b5280`; loop `call *(%eax)` vft+0 with `push 1`; `call 0x6162e0` (operator delete);
`ret 4`); and it is the atexit target of the Manager ctor thunk (§3). The 2024 export already had it
right: `.?AVManager@GUI@@.methods` contains `dtor_0x4b5280` + `meth_0x4b2900` + `meth_0x4b4920` —
three of these five are the very fns the live DB names `JE::GUI::Manager::*` (0x4b2900/0x4b4920/
0x4b5640/0x4b58b0). The StringTree label on 0x4b5280 is a stale collision for the writer to correct.

## 6. String mine (attack step 3) — honest negative

All 27 real band fns (live sizes from IDB, lea/push/mov-imm both `\xc7`-pad orientations) → **zero**
shipped strings in-band (confirms b4's byte rescan with the T-053 method applied properly). ctor body
carries one string member init at +0x70 (SSO cap 0xf) from `0x64d7d8` with **strlen==0** (empty
default name; the surrounding bytes are the unrelated `GameCreature::ReadVersion2()` blob — pointer
neighbors, not content). Identity therefore rests **entirely on the tier-1 singleton-join + RTTI
chain**, which per T-053's evidence hierarchy is *above* shipped-string tier. No band-internal naming
string exists (LoudDragon's steer — checked, negative, reported).

## 7. Consumer-side naming (attack step 4) — role corroboration

- ctor defaults: `+0x60=1024, +0x64=768, +0x68=+0x6c=1.0f` (screen dims + scale) — the `0x4b1848`
  window-scale read (inside `JE::GUI::Fragment::virt_meth_0x4b1830`, reading `+0x68`) is this field.
- Absolute field readers are all GUI widgets: `GUI::Button(+0x10)`, `GUI::Edit(+0xb0/+0xb4)`,
  `GUI::Label/LineSystem/Fragment(+0xb0)`, `F3::CommandGuiOptimizationLevel(+0xb0)`,
  `CLoadMapInterface(+0xb8 = modal chain head)`, `AreaMapWindow/cls_0x4203c0(+0x8c/+0xb0)`.
- ecx-flow [C+] host census: band callers sit in the interface windows' OWN vfts (`.?AVEdit@GUI@@`,
  `.?AVCMainmenuInterface@@`, `.?AVCDebugToolsInterface@@`, `.?AVCEscapeMenuInterface@@`,
  `.?AVCLoadMapInterface@@`, `.?AVCPipBoyAddNoteInterface@@`, `.?AVCDebugInterface@@`, `.?AUGraphics@@`)
  plus AreaMapWindow/GameInterfaces/GameActionEquip — every GUI activation boots through this object,
  exactly session-29's "GUI-root / registry" role map (dirty flag +0x4, watchers +0xA0/0xA4, modal
  chain +0xB8) — the `JE::GUI::Manager` name and the session-29 field map agree with zero conflicts.
- +0x48 = `_Tree` member (`cls_0x50dbb0` epoch), +0x1c/+0x54/+0x94/+0xa0 = 0x18-byte list-head members
  (seeded by `0x56e0b0`), consistent with a font/skin registry per the neighboring `GUI::Font` /
  `GUI::Skin` class tables.

## 8. WRITER QUEUE — merge instructions (route-A renames, 27 fns)

Rename `cls_0x4b28a0::<m>_EA` → `JE::GUI::Manager::<role>_EA` for exactly these live-named EAs:
`0x4b28a0, 0x4b28c0, 0x4b28e0, 0x4b2ab0, 0x4b2ad0, 0x4b2c10, 0x4b2c20, 0x4b2d40, 0x4b2f30, 0x4b2f80,
0x4b2fe0, 0x4b3000, 0x4b3040, 0x4b3430, 0x4b34b0, 0x4b3500, 0x4b3570, 0x4b35f0, 0x4b3780, 0x4b37f0,
0x4b3920, 0x4b40a0, 0x4b40f0, 0x4b4dd0, 0x4b4e60, 0x4b4f00, 0x4b58d0`
(role names per session-29 map: 0x4b2ab0=SetDirty(+0x4), 0x4b2ad0=SetModal, 0x4b4f00=BootstrapModal,
0x4b28a0/8c0/8e0 = viewport/clip getters (+0x60/64, +0x68/6c, +0x2c rect), 0x4b58d0 = DebugTools dump
helper per its callees' string epoch, etc. — writer's discretion).
**EXCLUDE `cls_0x4b28a0::meth_0x619b38`**: bytes `55 8b ec 51 e8 <call 0x61f2ef=__getptd>; mov
0x60(%eax),%ecx; cmp 0x70fd28,%ecx` — CRT locale/thread tail thunk misattributed by the 2024 export
(phantom-edge family, T-040 §8c); not in-game code.
**FIX** `JE::StringTree::dtor_0x4b5280` → `JE::GUI::Manager::dtor_0x4b5280` (§5).
Same-`this` sibling keys (purity-adjacent, writer may fold now or later): `cls_0x4b4180` (5 edges),
`cls_0x4b4d90` (3), `cls_0x4b2ac0::meth_0x4b4ce0` (2), `cls_0x4b5c30` (1), `sub_4B2910` (3),
`sub_4B4160` (2), forwarders `sub_4B2A90` (+0x1c→0x523160-ish) / `sub_4B2AA0` (+0x48→0x523320).
Edge-count reconciliation for the board row: **168** `mov ecx,imm` this-loads (raw byte census),
**166** flow edges (T-054, cache lineage same md5 — 2 edges were in fns later split), **142** flow
`[A]` band-key-targeted edges — all one-and-same pure population, no disagreement.

## 9. What is NOT concluded

- The exact shipped C++ name-check of the `JE::` prefix: RTTI says `GUI::Manager`; `JE::` is the
  live-DB/engine convention already applied by the prior partial mint — I keep that convention, not
  inventing a new one.
- Whether `0x4b5930-0x4b5cf0` (Font/Skin create-path helpers `new(0x38)`→ctor) are Manager methods or
  a factory sibling: they are OUT of this band key; their own attribution is a follow-on.
- `GUI::CPktManager` (TD 0x6fafcc, COL 0x6d3e94-adjacent, vft `0x66f7b4` master row) sits in the same
  RTTI cluster — plausibly the "Pkt" registry role T-053's §2b cousins; untouched here.
- I did not decode every one of the 28 band bodies; membership rests on the `this==&global` purity
  invariant + single-construction proof, not per-fn analysis.

## Raw-evidence index

`0x66f2cc` imm-stores: `0x4b566b`(ctor `c7 06`), `0x4b52a3`(dtor `c7 06`) — whole-file dword scan
returns exactly these two (file offs 0xb52a3/0xb566b). COL `0x6d3dc8` whole-file dword refs:
`0x66f2c8` only. TD `0x6faf80` refs: `0x6d3d98`, `0x6d3dd4`. Font vft `0x66f2d4` stores
`0x4b551f/0x4b55c3`; Skin `0x66f2dc` stores `0x4b5b35/0x4b5bcf`. atexit thunk `0x645de0 = ff e9 →
0x4b5280`. CRT-init table `[0x6f1450]=0x641660` (neighbors 0x6f144c/0x6f1454 = sibling thunks).
Selftest gate: `python3 tools/ecx-flow.py selftest` → 5/5 PASS. Scanner selftest on `0x707CE4` →
{a3@0x5bfe33, 89 1d@0x5c1e24, c7 05@0x5bedd6} reproduced exactly. IDA: read-only `list_funcs`/
`xrefs_to` only; IDB untouched; live-name snapshot re-read at write time (names churn — apply by EA).

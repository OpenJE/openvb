# T-059 — `pads` report mode: standing EH/vector-cleanup owner census

**EpicRaven**, row `bec4ebc`-filed, claim `1de0403`. Offline lane unchanged: raw EXE md5
**`a68c22f3dd1c256ec9539a70e87cb434`** + pre-T-055 F3.json dump; no 8745, no IDB writes.

## Gate first (standing rule)

`tools/ecx-flow.py selftest` — **5/5 PASS @ pinned md5** before any row below. NEW T4 is the
T-057 acceptance snapshot as a permanent gate: the 21 PAD rows (15 in cls_0x50dbb0, 6 in
cls_0x4542d0, 0 in cls_0x50db20) must reproduce EXACTLY via `bucket_entries()`, and the
0-ORPHAN invariant must hold across the three bands. It does:
`PASS T4 21/21 PAD rows exact + 0-orphan across the 3 bands`.
(T1 319 / T3 37 in this build vs T1 317 / T3 36 at T-057 — per-build drift, same md5,
ratified rule `bec4ebc`; the T4 snapshot is bucket-exactness, not count-exactness.)

## Executable taxonomy (the ratified rule, now in code)

`Model.bucket_entries()` IS the T-057 taxonomy with the evidence bit emitted on every decision:
- **PAD** requires edges from registered pad thunks only; registration requires the pad VA to be
  dword-referenced in `.rdata/.data` — so a PAD row prints `evidence=dataref@0x…` (its own proof);
- tail-jmp thunks WITHOUT a data-ref can never register as pads → they fall through to **DELEG**
  (printed as delegation chain); DISP offsets are byte-pinned facts;
- `families` mode now prints the evidence bit inline; band censuses quote it, no folklore.

## Pads census — `python3 tools/ecx-flow.py pads` totals

3,253 registered pads: **49 EH-`call`** handlers, **2,474 plain vector/atexit `jmp`**,
**730 adjusted-`jmp`** (member-slot destroyers).

EH-`call` pads (49) -- per-pad cleanup-owner table:
  pad 0x402F95 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x402f20::ctor_0x402f20 adj= data-ref=['0x6d8b24'] push=[]
  pad 0x413EDF -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x413e80::meth_0x413e80 adj= data-ref=['0x6d9654'] push=[]
  pad 0x4201C0 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x420140::ctor_0x420140 adj= data-ref=['0x6d9f28'] push=[]
  pad 0x43F265 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4112c0::ctor_0x43f1f0 adj= data-ref=['0x6dc098'] push=[]
  pad 0x44FF80 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x44ff00::ctor_0x44ff00 adj= data-ref=['0x6dd360'] push=[]
  pad 0x452855 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4527e0::ctor_0x4527e0 adj= data-ref=['0x6dd4e0'] push=[]
  pad 0x454285 -> 0x4542A0 (sub_4542A0 ?) scope=cls_0x454210::ctor_0x454210 adj= data-ref=['0x6dd640'] push=[]
  pad 0x459C35 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x459bc0::ctor_0x459bc0 adj= data-ref=['0x6ddd64'] push=[]
  pad 0x46AAE5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x401b00::ctor_0x46aa70 adj= data-ref=['0x6de434'] push=[]
  pad 0x46AD75 -> 0x46AB00 (sub_46AB00 ?) scope=sub_46AD00 adj= data-ref=['0x6de52c'] push=[]
  pad 0x493325 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4932b0::ctor_0x4932b0 adj= data-ref=['0x6df054'] push=[]
  pad 0x493CB5 -> 0x493C00 (.?AVInterface@GUI@@ meth_0x493c00) scope=cls_0x493c40::ctor_0x493c40 adj= data-ref=['0x6df1d0'] push=[]
  pad 0x4A2E90 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x4a2e10::ctor_0x4a2e10 adj= data-ref=['0x6e01e0'] push=[]
  pad 0x4A3305 -> 0x49A870 (cls_0x401b00 meth_0x49a870) scope=cls_0x401b00::ctor_0x4a3290 adj= data-ref=['0x6e0230'] push=[]
  pad 0x4A6865 -> 0x4A5AF0 (cls_0x4a67f0 meth_0x4a5af0) scope=cls_0x4a67f0::ctor_0x4a67f0 adj= data-ref=['0x6e0528'] push=[]
  pad 0x4B77C5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4b7750::ctor_0x4b7750 adj= data-ref=['0x6e1014'] push=[]
  pad 0x4BB755 -> 0x4BB6B0 (cls_0x4542d0 meth_0x4bb6b0) scope=cls_0x4542d0::ctor_0x4bb6e0 adj= data-ref=['0x6e12ec'] push=[]
  pad 0x4BBC45 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4bbbd0::ctor_0x4bbbd0 adj= data-ref=['0x6e133c'] push=[]
  pad 0x4BBCD5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4bbc60::ctor_0x4bbc60 adj= data-ref=['0x6e138c'] push=[]
  pad 0x4CD6FC -> 0x4CD610 (.?AV?$numpunct@D@std@@ meth_0x4cd610) scope=.?AV?$numpunct@D@std@@::meth_0x4cd640 adj= data-ref=['0x6e1e6c'] push=[]
  pad 0x4D6D82 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x4d6ce0::ctor_0x4d6ce0 adj= data-ref=['0x6e24d0'] push=[]
  pad 0x4D7BA0 -> 0x432510 (cls_0x50db20 meth_0x432510) scope=cls_0x50db20::ctor_0x4d7b10 adj= data-ref=['0x6e26ec'] push=[]
  pad 0x4D8DE5 -> 0x51E480 (cls_0x4d8d70 meth_0x51e480) scope=cls_0x4d8d70::ctor_0x4d8d70 adj= data-ref=['0x6e283c'] push=[]
  pad 0x4E0005 -> 0x45CDE0 (cls_0x4dff90 meth_0x45cde0) scope=cls_0x4dff90::ctor_0x4dff90 adj= data-ref=['0x6e2fe0'] push=[]
  pad 0x4E9B35 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x4e9ac0::ctor_0x4e9ac0 adj= data-ref=['0x6e3890'] push=[]
  pad 0x4E9BC5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=sub_4E9B50 adj= data-ref=['0x6e38e0'] push=[]
  pad 0x4FC74E -> 0x4FB560 (cls_0x4fc0e0 meth_0x4fb560) scope=cls_0x4fc0e0::ctor_0x4fc6c0 adj= data-ref=['0x6e48b4'] push=[]
  pad 0x509725 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x5096b0::ctor_0x5096b0 adj= data-ref=['0x6e5488'] push=[]
  pad 0x50C175 -> 0x49AE50 (sub_49AE50 ?) scope=sub_50C100 adj= data-ref=['0x6e56fc'] push=[]
  pad 0x515F45 -> 0x514A30 (cls_0x515ed0 meth_0x514a30) scope=cls_0x515ed0::ctor_0x515ed0 adj= data-ref=['0x6e5f58'] push=[]
  pad 0x515FF5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x515f80::ctor_0x515f80 adj= data-ref=['0x6e5fa8'] push=[]
  pad 0x520755 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x5206e0::ctor_0x5206e0 adj= data-ref=['0x6e66e8'] push=[]
  pad 0x55C950 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x55c8d0::ctor_0x55c8d0 adj= data-ref=['0x6e886c'] push=[]
  pad 0x55CA85 -> 0x55B100 (cls_0x50dbb0 meth_0x55b100) scope=cls_0x50dbb0::ctor_0x55ca10 adj= data-ref=['0x6e8914'] push=[]
  pad 0x55D015 -> 0x55CF40 (.?AVInterface@GUI@@ meth_0x55cf40) scope=cls_0x55cfa0::ctor_0x55cfa0 adj= data-ref=['0x6e89b4'] push=[]
  pad 0x55DCF5 -> 0x55DC50 (cls_0x55dc80 meth_0x55dc50) scope=cls_0x55dc80::ctor_0x55dc80 adj= data-ref=['0x6e8ae8'] push=[]
  pad 0x560E1F -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x560dc0::meth_0x560dc0 adj= data-ref=['0x6e8dc8'] push=[]
  pad 0x565970 -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x5658f0::ctor_0x5658f0 adj= data-ref=['0x6e90f4'] push=[]
  pad 0x572735 -> 0x571850 (.?AVInterface@GUI@@ meth_0x571850) scope=cls_0x5726c0::ctor_0x5726c0 adj= data-ref=['0x6e9dd8'] push=[]
  pad 0x574E05 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x574d90::ctor_0x574d90 adj= data-ref=['0x6e9f28'] push=[]
  pad 0x574E95 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x574e20::ctor_0x574e20 adj= data-ref=['0x6e9f78'] push=[]
  pad 0x57E2E5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x57e270::ctor_0x57e270 adj= data-ref=['0x6ea9b4'] push=[]
  pad 0x57E9D5 -> 0x4F6D40 (.?AVInterface@GUI@@ meth_0x4f6d40) scope=cls_0x57e960::ctor_0x57e960 adj= data-ref=['0x6eaa5c'] push=[]
  pad 0x5B33AF -> 0x401D60 (cls_0x401d60 meth_0x401d60) scope=cls_0x5b3350::meth_0x5b3350 adj= data-ref=['0x6ed070'] push=[]
  pad 0x609C88 -> 0x608260 (.?AV?$numpunct@G@std@@ meth_0x608260) scope=.?AV?$numpunct@_W@std@@::meth_0x609bd1 adj= data-ref=['0x6ee5c0'] push=[]
  pad 0x609DFC -> 0x60831E (.?AV?$_Mpunct@_W@std@@ meth_0x60831e) scope=.?AV?$_Mpunct@_W@std@@::meth_0x609c9a adj= data-ref=['0x6ee610'] push=[]
  pad 0x60F3B2 -> 0x612B31 (.?AV?$time_get@DV?$istreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@ meth_0x612b31) scope=.?AV?$time_get@_WV?$istreambuf_iterator@_WU?$char_traits@_W@std@@@std@@@std@@::meth_0x60f32e adj= data-ref=['0x6ee8d0'] push=[]
  pad 0x612F70 -> 0x60831E (.?AV?$_Mpunct@_W@std@@ meth_0x60831e) scope=.?AV?$_Mpunct@D@std@@::meth_0x612e32 adj= data-ref=['0x6eec3c'] push=[]
  pad 0x614B8B -> 0x612B31 (.?AV?$time_get@DV?$istreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@ meth_0x612b31) scope=.?AV?$time_get@DV?$istreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@::meth_0x614b13 adj= data-ref=['0x6eee90'] push=[]


## Reading the 49 (per-pad owner census)

Every row = an SEH `__except`/cleanup handler (scope = the ctor/method compiled around it,
`data-ref` = its slot in the contiguous handler table .rdata 0x6D8B24..0x6EEE90):

1. **21/49 destroy the GUI::Interface subobject**: target `0x4F6D40` (`.?AVInterface@GUI@@`) =
   verified cleanup helper (`call 0x4B9D60` + `free(0x6162E0)` on `[this+4]`). The scopes are the
   anonymous interface-class ctors (cls_0x402F20/0x4112C0/0x4527E0/0x4932B0/0x4B7750/0x4BBC60/
   0x5096B0/0x515F80/0x5206E0/0x574D90/0x57E270/…) — i.e. **every GUI interface ctor wraps its
   body in SEH and exception-cleans the embedded GUI::Interface member**. This is the same
   embed geometry as the T-057 CEngineInterface census (item-41 taxonomy; interface-view +
   value-embed composition), now with the cleanup side named.
2. **9/49 target `0x401D60`** = generic pointer-trio release (`free([this+4])`, zero +4/+8/+C) —
   the string/buffer member cleaner used by cls_0x4A2E10/0x4D6CE0/0x55C8D0/0x5658F0 ctors (the
   std::string-owning anonymous classes).
3. **6/49 are the shipped CRT locale/ios statics** (numpunct D/G, _Mpunct D/W, time_get D/W scopes) —
   proven foreign boilerplate, board-quotable as such.
4. The remainder target per-family cleanup: 0x4BB755→**0x4BB6B0** (T-057's last orphan inside
   `ctor_0x4BB6E0` scope), 0x454285→sub_4542A0 (GameActionEquip epoch), 0x55DCF5→0x55DC50 in
   `cls_0x55dc80::ctor` scope (TextureManager ctor's own exception-cleanup), 0x55CA85→
   0x55B100 (`cls_0x50dbb0` clone family), 0x4D7BA0→0x432510 (`cls_0x50db20` string machinery),
   plus cls_0x4D8D70/0x4FF90/0x4FC0E0/0x515ED0/0x4A67F0 singletons — one-line proof each of the
   owning ctor's member-destruction contract.

## The 3,204 vector/atexit `jmp` pads (aggregate, not per-row)

- **2,062 target `cls_0x4d8d70`** — the dominant static-array cleanup dtor: a 56-byte, vft-less,
  154-fn aggregate whose shape matches MSVC `std::basic_streambuf` (0x38 = 56) — the shipped
  CRT stream-statics family (consistent reading, stated as consistency not identity); 225
  GUI::Interface; adjusted-slot histogram peaks **+0x4/+0x8/+0x28/+0x1C/+0xC/+0xC4** — note
  **+0xC4 ×20**: the CEngineInterface embed offset from T-057 §3 is the same slot these atexit
  pads destroy, cross-confirming the embed geometry from the cleanup side.
- Multi-ref dispatch arrays: 17 (e.g. the 0x6E528C pad-run from T-057 §2). Another 26 pads
  target `.?AVCEngineInterface@@` dtor-adjacent code and 80 the trio-releaser `0x401D60` —
  the embed lane (T-057 §3) and the buffer lane both have their atexit-side mirror here.
- Board use: any future "who cleans up X" question quotes `pads` output; no re-derivation.

## Acceptance map

- Mode committed in tools/ecx-flow.py (`pads`), offline-only, md5-pinned cache unchanged.
- T4 gate = exact 21-row T-057 reproduction + 0-orphan invariant: PASS.
- Full per-pad table above (49 rows verbatim from the tool run of this pinned build).
- MINT/annotation queue (writer slot discipline, behind T-055): 21 GUI-interface-ctor rows
  (`SEH-cleans@GUI::Interface+0x4`-style comments), the trio-release scopes, and the
  +0xC4 atexit/embed cross-note; CRT-localic 5 rows labelled proven-foreign boilerplate.


---

## AMENDMENT (same day, LoudDragon clarification request; T-057 §1 correction-forward pattern)

**The `basic_streambuf-consistent` label on the 2,062-pad sink is RETRACTED — the answer is
neither (a) nor (c): (b)-variant, mechanism `string-destructor-in-the-band`.**

Function-level re-check of the pad sink (all byte-verified at pinned md5
`a68c22f3dd1c256ec9539a70e87cb434`):

- The 2,062 `cls_0x4d8d70` pads are **not** spread over the class — **1,958/2,062 target ONE
  function, 0x4B2DD0**, whose body is the application-wide **std::string destructor**:
  `cmp $0x10,0x18(%esi); jb skip; push 0x4(%esi); call free(0x6162E0)` then reset
  `cap→0xF, len→0, [4]→0` (classic SSO release-then-rearm). Call-site census: 2,322 ordinary
  code calls from *every string-owning class* (VFX_ParameterInfo 35, Edit@GUI 33, sub_499F10 36,
  cls_0x50db20 25, Map 20, GameActionEquip 12, …) — a per-class StringTree cleanup would never
  fan out like that.
- 0x4B2DD0 sitting under the `cls_0x4d8d70` label is exactly the T-036 **artifact floor**
  ("73 DB EAs that are not IDB function starts" family): OOAnalyzer aggregated a generic helper
  into the tree-manager band. The pad-adjust histogram (0×1,782 = whole-object; +0x4/8/12/1C…
  = member-slot destroys) is a census of **statics/arrays-of-strings cleanup**, not of
  StringTree instances.
- **No conflict with the b3 StringTree mint (not (c)):** the mint's own proof is the openje
  `_Tree/_Tmap_traits` node layout (_Color/_Isnil 0x30 nodes, SSO key in node) on the
  tree-manager side of the band — unaffected by where the string dtor landed. The remaining
  ~104 pads spread over 19 other band members (0x56EE40×58, …) carry the same caution: quote
  pads at FUNCTION granularity, never class granularity.

Queue effect for CalmRaven's wave (verify at write time): check whether the b3 rename applied
`JE::StringTree::meth_0x4B2DD0` in the live IDB — if yes, recommend an annotation-tier split
comment there (`std::string dtor aggregated into StringTree band per T-036 floor; not a
StringTree member`), no rename needed. `pads` mode already prints the function name beside the
class label; this row's table stands corrected at the interpretation line, bytes unchanged.


---

## QUEUE v2 (re-issued with head-graded locators; supersedes the T-059 §'Acceptance map' queue and T-057 §2 PAD anchors for APPLY purposes)

Audit per LoudDragon request (b4-era locators vs live-IDB head semantics; offline evidence =
direct-call head vs pad-only chunk; **the IDB read-back stays the writer-side authority — item 43**):

- **EH-call census (the 49): all 71 distinct locators (targets + ctor scopes) are direct-call
  heads** — queue-safe as printed, including the two StringTree floor comments 0x4B2DD0 and
  0x56EE40 (both call-heads; matches your live read-back of `JE::StringTree::meth_*_4b2dd0 /
  _56ee40`) and `0x4BB6B0` (call-head via its EH handler — the sole PAD row that is also a
  call-head).
- **Structural finding: PAD rows are by definition pad-reached, so 20/21 have no direct call
  site — pad-reached locators can be LIVE-IDB SECONDARY CHUNKS of an earlier function (the
  0x507550-inside-0x5074D0 pattern CalmRaven caught). Re-anchor table (annotation targets):

  0x48EE00  non-call-head — ANNOTATE AT CONTAINING HEAD 0x48ED60 (.?AVVFX_VEGInstance@@::ctor_0x48ed60); cite chunk 0x48EE00 in text (pad 0x630e90, dataref ['0x6def4c'])
  0x4933D0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x493340 (sub_493340); cite chunk 0x4933D0 in text (pad 0x63106c, dataref ['0x6df318'])
  0x493400  non-call-head — ANNOTATE AT CONTAINING HEAD 0x493340 (sub_493340); cite chunk 0x493400 in text (pad 0x631082, dataref ['0x6df328'])
  0x49F290  non-call-head — ANNOTATE AT CONTAINING HEAD 0x49F150 (cls_0x49f150::ctor_0x49f150); cite chunk 0x49F290 in text (pad 0x631981, dataref ['0x6dfebc'])
  0x4B51F0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x4B5130 (cls_0x490450::meth_0x4b5130); cite chunk 0x4B51F0 in text (pad 0x6365e0, dataref ['0x6e6a30'])
  0x4B5220  non-call-head — ANNOTATE AT CONTAINING HEAD 0x4B5130 (cls_0x490450::meth_0x4b5130); cite chunk 0x4B5220 in text (pad 0x63244b, dataref ['0x6e0d50'])
  0x4BC020  non-call-head — ANNOTATE AT CONTAINING HEAD 0x4BBCF0 (cls_0x55dc80::meth_0x4bbcf0); cite chunk 0x4BC020 in text (pad 0x6328c6, dataref ['0x6e14e4'])
  0x507550  non-call-head — ANNOTATE AT CONTAINING HEAD 0x5074D0 (sub_5074D0); cite chunk 0x507550 in text (pad 0x635653, dataref ['0x6e528c'])
  0x50DAF0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x50DA90 (cls_0x50db20::ctor_0x50da90); cite chunk 0x50DAF0 in text (pad 0x635a60, dataref ['0x6e5960'])
  0x520340  non-call-head — ANNOTATE AT CONTAINING HEAD 0x520290 (cls_0x5206e0::meth_0x520290); cite chunk 0x520340 in text (pad 0x6363c6, dataref ['0x6e6798'])
  0x54ECC0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x54EBB0 (cls_0x4d8d70::meth_0x54ebb0); cite chunk 0x54ECC0 in text (pad 0x637ad8, dataref ['0x6e822c'])
  0x54ECF0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x54EBB0 (cls_0x4d8d70::meth_0x54ebb0); cite chunk 0x54ECF0 in text (pad 0x637b02, dataref ['0x6e8244'])
  0x572EE0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x572E80 (sub_572E80); cite chunk 0x572EE0 in text (pad 0x63931a, dataref ['0x6ea618'])
  0x593500  non-call-head — ANNOTATE AT CONTAINING HEAD 0x593430 (cls_0x50dbb0::meth_0x593430); cite chunk 0x593500 in text (pad 0x63a316, dataref ['0x6eb8c8'])
  0x5CB270  non-call-head — ANNOTATE AT CONTAINING HEAD 0x5CB1B0 (cls_0x50dbb0::meth_0x5cb1b0); cite chunk 0x5CB270 in text (pad 0x63c003, dataref ['0x6edb08'])
  0x422920  non-call-head — ANNOTATE AT CONTAINING HEAD 0x422890 (cls_0x4542d0::meth_0x422890); cite chunk 0x422920 in text (pad 0x62c863, dataref ['0x6da234'])
  0x4A40B0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x4A3ED0 (cls_0x55dc80::meth_0x4a3ed0); cite chunk 0x4A40B0 in text (pad 0x631cb0, dataref ['0x6e03f4'])
  0x4BB6B0  call-head — queue as-is (pad 0x4bb755, dataref ['0x6e12ec'])
  0x54ED20  non-call-head — ANNOTATE AT CONTAINING HEAD 0x54EBB0 (cls_0x4d8d70::meth_0x54ebb0); cite chunk 0x54ED20 in text (pad 0x637b10, dataref ['0x6e824c'])
  0x57F2D0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x57F220 (sub_57F220); cite chunk 0x57F2D0 in text (pad 0x63983e, dataref ['0x6eace8'])
  0x5984F0  non-call-head — ANNOTATE AT CONTAINING HEAD 0x598400 (.?AVMapMaskManager@@::dtor_0x598400); cite chunk 0x5984F0 in text (pad 0x63a536, dataref ['0x6ebb88'])

  jmp-pad targets 0x47A6B7/0x493D70/0x6080B4/0x60CE5F: non-call-heads — aggregate rows re-anchored
  to container heads 0x47A5CD / 0x493CD0 / 0x60809F / 0x60C817 respectively (writer read-back).
- Tool support landed: `head_hint()` evidence now prints inside `families` [Q1-padthunk] lines
  (`entry-headness=call-head|non-call-head + containment candidate 0x…`), so every future PAD
  row arrives queue-ready. T4 gate unaffected (5/5 PASS); buckets are flow facts, headness is a
  locator fact — kept separate on purpose.
- Net queue effect for T-060: annotations anchor at the 20 containing heads + 1 direct (0x4BB6B0);
  comment TEXT still cites the chunk EA (e.g. "vector-pad cleanup reaches cold chunk 0x507550 of
  this function") — the census numbers themselves (21 PAD / 0 orphan / 2,062→0x4B2DD0 regraded)
  are locator-independent and stand.

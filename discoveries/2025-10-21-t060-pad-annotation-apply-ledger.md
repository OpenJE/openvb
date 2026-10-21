# 2025-10-21 — T-060: Apply the T-059 pad/cleanup owner queue as IDB annotations (comment-only wave). T2 delta = 0.

CalmRaven (designated writer), dispatched by LoudDragon `01a15f82`. SOLE IDB WRITER on `ida_original_f3`
(8745) from claim commit `78ce017`. EpicRaven's T-059 doc (`1de0403` + AMENDMENT, md5 `a68c22f3…`) is
**CITATION, not evidence.** ANNOTATION-TIER ONLY — comments set, ZERO renames, T2 unchanged. **Item 43
rule 3 (writers refuse non-head EAs) applied throughout.**

## 0. Baseline + census (comments-only → delta 0 BY CONSTRUCTION)

| phase | T2 (cls_) | T3 (sub_) | T1 (named::) | game | IDB mtime |
|---|---|---|---|---|---|
| PRE-WRITE | 2,012 = 22.67% | 2,169 | 4,479 | 8,876 | 15:57:36 (T-058 end, durable) |
| mid-wave cp | 2,012 | 2,169 | 4,479 | 8,876 | 16:04:32 |
| **END-of-wave** | **2,012** = 22.67% | 2,169 | 4,479 | 8,876 | **16:04:57** |

**T2 before == after = 2,012 → DELTA 0** (no renames; comments do not move the census). Dumps
`discoveries/wavedumps/t060-{pre,post}dump.json`. 37 sites commented, every one `get_cmt` readback-verified
after the end save → disk==live.

## 1. Item-43 head-verification (the wave's defining discipline — result: 100% clean, zero holds)

Before annotating, every scope + target EA in the T-059 table was checked live for function-head status
(get_func(ea).start_ea==ea). **All 30 scope heads + both cleanup targets are genuine heads — NO non-head
locator appeared in the rows I annotated, so ZERO rows were held.** (EpicRaven corrected their locators
up-front per the coordinator's note; this is item 43 rule 3 passing, not evading.)

## 2. 21 GUI-interface SEH-cleanup rows (`SEH-cleans@GUI::Interface+0x4 via 0x4F6D40`)

Target 0x4F6D40 = `JE::GUI::Interface::meth_0x4f6d40` verified live (calls 0x4B9D60 + `j__free` on [this+4]).
Each scope is an anonymous GUI-ctor wrapping its body in SEH; comment records the embedded-GUI::Interface
cleanup, cites the pad data-ref. Scopes (all heads): 0x402F20, 0x4112C0, 0x4527E0, 0x4932B0, 0x4B7750,
0x4BBC60, 0x5096B0, 0x515F80, 0x5206E0, 0x574D90, 0x57E270, 0x459BC0, 0x46AA70, 0x493C40, 0x4BBBD0,
0x4E9AC0, 0x4E9B50, 0x55CFA0, 0x5726C0, 0x574E20, 0x57E960. **21/21 set + readback OK.**

## 3. Trio-releaser 0x401D60 scopes (9)

0x401D60 body byte-verified: `j__free([esi+4])` then zero +0x4/+0x8/+0xC (std::string/buffer trio cleaner).
Scopes (heads): 0x413E80, 0x420140, 0x44FF00, 0x4A2E10, 0x4D6CE0, 0x55C8D0, 0x560DC0, 0x5658F0, 0x5B3350.
**9/9 set + readback OK.**

## 4. 6 CRT proven-foreign labels

T-059 lists 6 locale pads collapsing onto **4 unique function heads** (numpunct<char> 0x4CD610,
numpunct<wchar_t> 0x608260, _Mpunct 0x60831E, time_get 0x612B31) — item-43 head-granularity note: 6 pad
rows / 4 heads (two facet dtors reached by two pads each). Labelled PROVEN-FOREIGN shipped CRT/locale
boilerplate, do-not-count-in-band-census. **4 heads labeled** (the correct count for a per-function
annotation; logged the 6→4 collapse explicitly rather than double-writing a head).

## 5. +0xC4 embed/cleanup cross-note

On 0x4F6D40 anchor: the T-059 atexit-destroy histogram peaks **+0x4 ×225 (GUI::Interface member)** and
**+0xC4 ×20**; +0xC4 is the CEngineInterface embed slot from T-057 §3 (verified `lea ecx,[esi+0C4h]` at
0x558775). The **cleanup side independently reproduces the embed geometry** — value-member composition
confirmed from the destruction direction. Set + readback OK.

## 6. SPLIT-COMMENTS (the two you're live-confirmed on) — NO rename, T2-neutral

Both `JE::StringTree::meth_*` on the fresh server (confirmed independently, not just coordinator-noted).
Body-verified before annotating:
- **0x4B2DD0**: `cmp [esi+18h],10h / jb / free([esi+4]) / cap->0xF, len[+14h]->0, [+4]->0` = textbook
  MSVC **std::string SSO destructor**. 2,322-call fan-out across string-owning classes. Per T-059
  AMENDMENT (author retracted the basic_streambuf label) + T-036 artifact floor: a generic helper
  aggregated into the `cls_0x4d8d70`/StringTree band. Comment states **NOT a StringTree member; the b3
  StringTree MINT is UNAFFECTED** (its proof is the `_Tree`/`_Tmap_traits` node layout on the tree-manager
  side). Split-comment set + readback OK.
- **0x56EE40**: frees **two** SSO string members (block +0x20/cap +0x34, block +0x4/cap +0x18) = generic
  two-string-member destructor, same artifact-floor caution (one of the ~104 non-0x4B2DD0 pads spread
  over 19 other band members). Split-comment set + readback OK.

## 7. Per-row table (scope/target EA · head? · readback) — summary
- GUI 21 (heads OK, set 21/21) · trio 9 (heads OK, set 9/9) · CRT 4 heads from 6 pads (set 4/4) ·
  cross-note 0x4F6D40 (set OK) · split 0x4B2DD0 + 0x56EE40 (set OK) = **37 commented function heads**,
  all readback-verified. **Held non-head rows: 0.** **Renames: 0.** **T2 delta: 0.**

## 8. Discrepancies surfaced (routed, not adjusted)
- **6 CRT pads → 4 unique heads.** Annotated per FUNCTION (item 43), not per pad-row; the other 2 heads
  (0x60831E, 0x612B31) each carry two locale-facet pad-rows. Logged explicitly so the ledger count (4)
  reconciles against the doc count (6) without forcing a second write onto the same head.
- No DISP/locator conflicts found in this queue (unlike T-058's 0x507550); EpicRaven's up-front locator
  correction held. Nothing met the arbitration trigger.

## 9. Persistence + hand-back
Mid cp 16:04:32 + END save **16:04:57**; post-save fresh re-dump confirms all 37 comments persisted →
disk==live. Ledger committable (writer lane). Writer slot released. No renames, T2 = 2,012 unchanged.

---

## 10. QUEUE v2 SUPERSISSION (coordinator `4a81ef0`, mid-wave) — PAD containment-head re-anchor

EpicRaven's locator audit (t059 QUEUE v2) supersedes the v1 §2/§5 PAD *application targets*. Key
separation (kept explicitly): **buckets are flow facts (locator-independent; T4 gate 5/5, 21 PAD/0-orphan
hold); headness is a locator fact with the live IDB as sole authority (item 43 rule 3).**

- **The 37 comments I applied under v1 are ALL confirmed direct-call heads** — the doc's own audit:
  "EH-call census (the 49): all 71 distinct locators (targets + ctor scopes) are direct-call heads —
  queue-safe as printed, including the two StringTree floor comments 0x4B2DD0 and 0x56EE40." My §2/§3/§4
  EH-49 rows, the §6 split-comments, and the §5 cross-note anchor are unaffected; **nothing to revert**
  (I had NOT commented any PAD chunk EA, so the "note-don't-revert" case did not arise for me).
- **NEW work = 20 PAD chunk locators re-anchored to CONTAINING HEADS** (chunk EA kept IN the comment
  text, per the rule). Each head live-verified as a function start first. 17 IN-GAME heads annotated:
  0x48ED60←0x48EE00 · 0x493340←0x4933D0/0x493400 · 0x49F150←0x49F290 · 0x4B5130←0x4B51F0/0x4B5220 ·
  0x4BBCF0←0x4BC020 · 0x5074D0←0x507550 · 0x50DA90←0x50DAF0 · 0x520290←0x520340 ·
  0x54EBB0←0x54ECC0/0x54ECF0/0x54ED20 · 0x572E80←0x572EE0 · 0x593430←0x593500 · 0x5CB1B0←0x5CB270 ·
  0x422890←0x422920 · 0x4A3ED0←0x4A40B0 · 0x4BB6B0 (sole PAD **call-head**, queued as-is) · 0x57F220←0x57F2D0 ·
  0x598400←0x5984F0. **17/17 set + readback OK.** Note the `0x507550`-inside-`0x5074D0` re-anchor is
  exactly my T-058 non-head refusal — v2 confirms I was right to hold it; now annotated at the true head.
- **3 jmp-pad containers HELD + reported (item 43 rule 3 fired; did NOT adjust to a neighbor):**
  - `0x47A5CD` (chunk 0x47A6B7) → containment head is **`cls_0x668cd8` = the D3DX9 VENDOR band** (binding
    T-033/T-058 exclusion). Not stamping comments inside third-party code — routed, not annotated.
  - `0x60809F` `__Stof` (chunk 0x6080B4) → **≥0x5FD000, outside the game denominator (STL/CRT)**. HELD.
  - `0x60C817` (chunk 0x60CE5F) → **≥0x5FD000 CRT/STL tail**. HELD.
  (Only `0x493CD0`←0x493D70 is a legitimate in-game jmp-pad container — annotated; readback OK.)
- **v2 TALLY:** 37 (v1 EH-49/split/crossnote) + 18 (v2 PAD heads: 17 chunk-containers incl the call-head
  0x4BB6B0, + jmp-pad container 0x493CD0) = **55 function heads commented this wave, T2 delta 0, renames 0.**
  3 PAD/jmp-pad rows HELD for arbitration (vendor + 2 out-of-region) — reported below, not adjusted.
- v2 cp save **16:10:07**; end-of-wave fresh-dump re-dump in `discoveries/wavedumps/t060-postdump.json`.

## 11. Item-43 holds routed to coordinator (NOT adjusted)
| held EA | reason | action |
|---|---|---|
| 0x47A5CD | head `cls_0x668cd8` = D3DX9 vendor band | HELD — binding vendor exclusion |
| 0x60809F `__Stof` | ≥0x5FD000 out-of-region (STL/CRT) | HELD — not game band |
| 0x60C817 | ≥0x5FD000 CRT/STL tail | HELD — not game band |

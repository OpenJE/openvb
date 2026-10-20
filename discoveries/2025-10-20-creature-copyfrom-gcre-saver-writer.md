# 2025-10-20 — Session 13: GameCreature CopyFrom (v1 arrays ARE runtime data), GCRE SAVE writer = ver 5, table sizes 13/82

Target: ORIGINAL_REFERENCE. Read-only. Products of the "+0x1E0/GCRE consumer" scans;
complements `2025-10-20-session12-serverupdate-ondeath-flip.md` (whose §3 corrections are now
adopted into the docs it flagged — see serverupdate/door-rebase docs).

## 1. Creature Entity-vft unique bodies resolved (stub → body, raw jmp decode)

| slot | stub | body | semantics |
|------|------|------|-----------|
| +0x28 | 0x40d460 | **0x40acc0** | GameCreature::CopyFrom — §2 |
| +0x34 | 0x40e340 | 0x40d6b0 | creature-specific (unchased) |
| +0x60 | 0x40d430 | 0x409be0 | creature-specific (unchased; slot in engine-default band) |
| +0xa0 | 0x40d480 | **0x40bb80** | GCRE save writer — §3 (IDA labels it `JE::SaveCRT_0x40bb80`; CRT = the `.CRT` creature file, name is apt) |
| +0xb4 | 0x40e350 | 0x40d840 | sits in the GetUsableSkills band |
| +0xe0 | 0x40d470 | 0x40ae60 | (unchased) |

(Recipe: stubs `2b 49 fc e9 rel32`; bodies = stub+8+rel.)

## 2. `GameCreature::CopyFrom` @0x40acc0 = the first consumer of the GCRE v1 arrays

Structure (RTTI Entity→GameCreature cast of source, then field mirror):

```
dest−1420 ← src+0x278 (string)              ; GCRE v1 string #1
dest dword ← src+0xA5-ish scalars ⚠ (decompiler units — see raw [esi+294h]/[esi+2B4h]
             reads @0x40ad04/0x40ad24 + [esi+2B8h] byte @0x40ad30; the CopyAssign pairs at
             0x40acfd/0x40ad1f are src+632=+0x278 and src+664=+0x298)
dest−1352 + 28k ← src+0x2BC + 28k, k=0..11  ; ALL TWELVE 0x1C-spaced strings  @0x40ad3c..0x40ad6b
dest−988  ±      ← src+0x428 + 56k pairs     ; the THIRTEEN 2-string pairs      @0x40ad6d..0x40adac
dest−288 bytes+0..7 ← src+1764..1775          ; the TWELVE bools (+0x6E4..+0x6EF) @0x40adb4..0x40ae39
then tail-call GameEntity::CopyFrom (0x468350) with the GE portion  @0x40ae50
```
⇒ the v1 block (12 strings + 13×2 strings + 12 bools + 2 u32 + 2 strings) is **runtime-persistent
creature state copied on every entity clone/template-apply** — not scratch. Combined with §3's
save writer (which persists them), the semantic class is "creature traits/known-scripts/
dialogue-package style data" *(still name-pending — consumers beyond copy/save may be the
dialogue/UI layer; B can grep the GUI cluster)*. The `dest−offset` values confirm creature
own-region layout: dest == this@Entity+0x804−… consistent with absolute fields 0x278/0x2BC/
0x428/0x444/0x6E4 in GameCreature coordinates.

## 3. GCRE SAVE writer @0x40bb80 (`creature` slot +0xA0 override) — wire truth + version=5

Order of writes (file-stream writer; EAs 0x40bbc0..0x40c876, huge function):

```
1. GameEntity::GENT writer sub_463970 FIRST (base)                   @0x40bbc0
2. Chunk header: tag 'GCRE'(1163019079), **version 5**, size 0→patched
3. 7×u32   from (this−473+k dwords)      = Creature+0xE0-band? — see mapping ⚠ below
4. 8×u32   singles (this−466..this−458 dwords)
5. u32 COUNT=13, then 13×{idx, val} from (this−457)+i dwords  → maps to
   **sparse table @GameCreature+0xE0** (13 entries) — the v0 reader's loop1 target ✓
6. u32 COUNT=82, then 82×{idx, val} from (this−444)+i dwords  → **sparse table @
   GameCreature+0x114** (82 entries) — loop2 target ✓  ⇒ 82 = the per-index skill-ish vector
7. u32 count (this−360) + RbTree walk (this−362 head) writing node vals → list/tree pairs
   (+0x25C list / +0x260 tree — loop3/loop4 of the v0 reader) ×2 sets ((this−357/−358))
8. remaining tail: strings + flags incl. the +0x6E4 bool block region (truncated dump file
   /tmp-preserved; full line-by-line map available on request)
```
⚠→✓ UPDATE (same session, post-edit check): the dword arithmetic resolves EXACTLY, no
mismatch — (this−473+k)·4 with this=Entity=Creature+0x804 gives Creature+0xA0+4k for k=0..6
(the **7 stats** ✓) and (this−466..−458) gives **+0xBC..+0xDC** (the **8 scalars** ✓). So the
save writer mirrors the v0 reader field-for-field in order:
`7 stats · 8 scalars · 13 pairs@0xE0 · 82 pairs@0x114 · list@0x25C · tree@0x260 (+second
tree set) · strings · bools` — GCRE save = full own-state dump. Only items 7–8 tail detail
(which string/tree in what order) remains to be lined up.

### 3.1 Version truth for the loaders
Shipped code SAVES GCRE at **version 5** → every version gate in `ReadData_0x40e360`
(v≠0 sub_409410, v≥2 failsafe, v≥3 chances, v≥4 innate lists, v≥5 extra dword) is LIVE for
files this engine writes. A's observed template samples at GCRE ver… (their rht walk reported
GENT ver=1; check GCRE version values in the same walk — if shipped GCRE chunks are also v5,
the full v1–v5 load path including the §2 arrays is exercised by the demo.)

### 3.2 Capacity cross-check (confirms both sides)
Reader loop1 writes `+0xE0 + 4·idx` with table ending at +0x114 (next table start) ⇒ capacity
**13** = save count 13 ✓; loop2 `+0x114 + 4·idx` ending at +0x25C (list head) ⇒ capacity
**82** = save count 82 ✓. Exact match both — the sparse tables are 13- and 82-entry, and the
save writes ALL of them each time. High confidence on this region now.

## 4. Practical notes
- CopyFrom + Save writer together pin the creature own-region layout for A's entity graduation:
  0xA0 stats(7,+mirror 0x760) / 0xBC..0xDC scalars / 0xE0 13-slots / 0x114 82-slots /
  0x25C list / 0x260 tree / 0x278,0x298 strings / 0x2BC+28k ×12 strings / 0x444 pairs ×13 /
  0x6E4 bools ×12 / 0x6EC GWAM count / 0x6F4 list / 0x70C chances.
- 82-slot table is the prime candidate for the SKILL list (Van Buren-era skill count); the
  13-slot one for… party? weapon-stance? (13 also matches the 13×2 string pairs count — the
  pairs may be skill name/format strings per index? unconfirmed).

## 5. Open / handoffs
- Full save-writer tail map (items 7–8 exact fields): have the decompile dump, will line it up
  next round or hand to whoever implements `.CRT` writing.
- Who calls 0x40acc0 besides the +0x28 slot: the GE CopyFrom tail (0x40ae50→0x468350) proves
  the creature override chains to base. Whether spawn-from-template invokes slot +0x28 is
  *(inferred, not yet proven)* — A, treat as hypothesis until a caller is found; discovery
  will xref vft+0x28 call sites next round.
- +0x34 body 0x40d6b0, +0x60 body 0x409be0, +0xe0 body 0x40ae60 names (30 min, anyone).
- GameEntity+0x18 verdict: save-writer band ambiguity now CLOSED (§3 update — GCRE writes
  creature-band fields, not GE+0x18), and two consumer scan passes have found zero readers.
  Next session: one last targeted scan (disp8 0x18 within GameEntity-method function bodies
  only) and then formally declare **write-only-in-shipped-build** unless it pops.

## 6. Evidence index
Stub→body raw decode (recipe §1). CopyFrom 0x40acc0 (casts @0x40acd8, 12-loop 0x40ad3c..6b,
13-loop 0x40ad6d..ac, bools 0x40adb4..0x40ae39, tail GE CopyFrom 0x40ae50). Save writer
0x40bb80: GENT-first 0x40bbc0; hdr {1163019079, 5, 0} 0x40bbd8..0x40bbe8; 13-count 0x40bd82;
82-count 0x40bdfd; tree walks 0x40be83..0x40bf4a; full listing preserved in session notes
(/tmp/pi-mcp-21fa64dd1b068f28.txt equivalent — regenerate via decompile 0x40bb80).
Reader cross-ref: sub_40B6A0 loops (gcre-versions doc §3.1).

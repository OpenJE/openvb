# 2025-10-21 — T-018: the helmet record's GameItem+0xA8 required-slot VECTOR — read from shipped bytes (PureKnight 01a15cb7-e27e)

Target: `$OPENVB_F3DEMO_ROOT/data/_ARM.grp` (same bytes as `$OPENVB_ORIG_EXE`-adjacent data dir) + the shipped
GITM parser in `ida_original_f3`. Pure statics/data — no debugger, no IDB writes. Harness item 24 honoured:
every claim below is a byte inside a `.grp` or an EA in the original image.

## Verdict

**CONFIRMS the prediction: the head-worn record's `+0xA8` required-slot vector = {1} (count=1, value=1).**
Head-family naming for called-shot row B stands; T-010c stays closed. AND: `+0xA8` is **stored, not computed** —
the vector's on-disk serialization was located and its parser site pinned (below). Bonus: the T-025 census
gitm columns are mislabeled (see §4) — session35's "eqslot" value was in fact `reqvec[0]` all along.

## 1. The parser (who fills GameItem+0xA8)

`F3::GameItem::ReadFromFile_0x411260` dispatches chunk tag `'GITM'` (imm `0x4D544947`, single code reference in
all `.text`, `0x41126b`) to `sub_4111E0` → `sub_410B80` (GITM body, `this` = GameItem):

```
0x410b95  u32 → this+0xA0    (type; ==2 for armor)
0x410bbf  u32 → this+0xA4    (second word; if >0 overwrites +0xA0 — shipped quirk at 0x410bd4)
0x410beb  u32 N → local_c            ; ***req-slot vector COUNT***
0x410c19  loop N times: read i32 value       ; ***vector elements***
0x410c3c  sub_433760(listnode, tail, &value) ; intrusive insert
0x410c40  cls_0x4112c0::meth_0x4109e0(this+0xA8, 1) ; container size++ header op
```

`GameItem+0xA8..+0xAC` is the container header (link node + element pointer — `sub_411330` copies it at
`ctor_0x4112c0(this-65 ≙ +0xA8, src+0xA8)`; `GameInventory::Equip_46be40` snapshots it via
`cls_0x4112c0::ctor_0x43f1f0(&v40, GameItem+0xA8)` at `0x46befa`, `lea ecx,[esi+0A8h]` at `0x41136c`), and the
equip check iterates it feeding each element's payload (`node[2]`) into the FastArrow B8 pair table
`sub_469430` (`0x46bf33` / `0x46c03f`) — matching the T-010 note that DR coverage keys off `+0xA8`.

So the on-disk stream is: `u32 type, u32 x, u32 N, N×i32 values, {u16 len, ascii} socket strings…`
starting at GITM payload (tag+12).

## 2. The shipped bytes (all seven GITM records in _ARM.grp, offsets per verified `artifacts/census/chunks.csv` walk)

| chunk off | payload +0..+16 hex | N | reqvec (→ GameItem+0xA8) | first socket string | reading |
|---|---|---|---|---|---|
| `0x3e4` | `02 00 00 00 · 00 00 00 00 · 01 00 00 00 · 00 00 00 00` | 1 | **(0)** | — | body suit (req 0) |
| `0x7c4` | `02 00 00 00 · 00 00 00 00 · 01 00 00 00 · 01 00 00 00 · 0d 00 'HeaMotorcycle'` | 1 | **(1)** | `HeaMotorcycle` | **the head-worn record — contains 1** |
| `0xbc4`/`0xf6b`/`0x1319`/`0x16c1`/`0x1a6c` | `02 00 00 00 · 00 00 00 00 · 00 00 00 00 · 0d 00 'HeaMotorcycle'` | 0 | **()** | `HeaMotorcycle_*.dds` variants | texture/color variants, empty vector |

The single record whose vector is non-empty-and-1 (`0x7c4`) is exactly the one whose art sockets are `Hea*` —
one record, one head-worn piece, one `1`. Per row scope, this is NOT extended into a general item-table audit.

## 3. What settles

- **Head-family naming stands** (prediction branch): helmet's req vector = {1}; body gear = {0}. The pair table
  `sub_469430` maps req{2..5}→phys2 / reserves phys0/phys1 — req class 1 is the helmet's own slot class, so
  row B coverage keys off requirement class 1 alone, consistent with B8's structural argument.
- **+0xA8 mechanism: stored.** The count+values are direct GITM fields; no computing site exists (so the
  coordinator's "if computed" fallback does not fire). Session35's "single value rather than a vector"
  phrasing was accidental-but-right: capacity-1 vectors with one element.

## 4. Census-schema correction (for the T-025 owners — byte truth, no re-run claimed here)

`gitm.pat` + `tools/data_census.py` read `equip u8 @+20(tag)` + `s32 eqslot @+24(tag)`. Against the parser's
actual stream these positions are **N (the req-count, low byte = 'equip')** and **reqvec[0] (= 'eqslot'),
or string bytes when N=0** (which is why the five variant rows show `eqslot=1699217421` = ascii garbage in
gitm.csv). Consequences for any future GITM census re-run: a record with N≥2 would show "equip=2"; an
equippable record with N=0 shows "equip=0". Session35's *conclusion* survives unchanged (its "eqslot" value
was `reqvec[0]`, which is what T-018 asked about), but its *schema naming* should be corrected so the next
lane doesn't hunt for a nonexistent scalar equip-slot field.

## 5. Boundary

- Seven GITM chunks in `_ARM.grp` are the complete shipped armor set for the demo (per committed chunks.csv);
  items in `Items.grp` were not audited — explicitly out of this row's scope.
- The `{u16 len, ascii}` string-walk in the variant rows is heuristic; the numeric vector fields are parser-exact.

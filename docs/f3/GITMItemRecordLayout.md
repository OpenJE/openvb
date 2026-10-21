# GITM item-record layout: `+0xA8` is a STORED required-slot vector

T-018 closure note (evidence + shipped-byte table: `discoveries/2025-10-21-t018-helmet-reqslot-vector-read.md`,
PureKnight `01a15cb7`). Canonical sibling: the B8 pair table `sub_469430` (req-class → phys-slot),
which T-041's requirement-class reading and the `dtRowB` naming both rest on.

## Parser shape (`sub_410b80`, found via the single `'GITM'` immediate at `0x41126b`)

Per item record, from the payload stream:

| field | read site | meaning |
|---|---|---|
| `u32` type | record +0xA0 | item type id |
| `u32` | record +0xA4 | (uninterpreted; not the question) |
| `u32 N` | `0x410beb` | **count of required-slot values** |
| `N x i32` | `0x410c19` loop | inserted into the container anchored at **`+0xA8`** (`0x410c3c/40` insert + size++) |

`+0xA8` is **stored from the file, not computed** — the verdict T-018 demanded. Consumer:
`GameInventory::Equip_46be40` snapshots `+0xA8` at `0x46befa` and feeds its elements to
`sub_469430` at `0x46bf33/0x46c03f`.

## Shipped values (from `_ARM.grp`, offsets per `artifacts/census/chunks.csv`)

- head-worn record `0x7c4` (HeaMotorcycle art sockets): N=1, vec=(1) — the prediction datum:
  the helmet's required-slot vector **contains 1** ⇒ head-family naming stands, T-010c closes.
- suits `0x3e4`: N=1, vec=(0). Five texture variants: N=0, vec=().

## Census-schema correction (affects `artifacts/census/gitm.csv`, NOT re-run)

`gitm.csv`'s `equip`/`eqslot` columns are **req-count / reqvec[0]** under worse names: the `+20/+24`
labels come from session 35's scalar reading and `gitm.md`'s `+24 = "Slot required to use the item"`
is prose over-counting a vector. Conclusions that treated `eqslot` as reqvec[0] (session 35's
helmet-slot close) SURVIVE because that is exactly what the field was. The CSV is byte-identical;
any future `data_census.py` re-run must rename/re-document those two columns.

## Attribution caveat, same family as harness item 27

`sub_410b80` has no owner in the frozen `F3.json` (`f3_method_index 0x410b80` → 0 results) — found by
xref from the magic-string immediate. "Not attributed in the 2024 DB" ≠ "not a function".

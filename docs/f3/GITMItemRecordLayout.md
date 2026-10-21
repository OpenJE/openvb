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
## GWAM side from T-015 (same parser family, `sub_4148A0` weapon / twin `sub_40dba0` creature)

PureKnight's remainder pass (`discoveries/2025-10-21-t015-slottable-twin-parity.md`) settled the
weapon/creature record family; durable facts:

- **Attack-mode records are LISTS, not dwords** — `sub_40C970` is an intrusive node-append
  (`0x40c979` link dance): weapon list base `GameWeapon+0x40C`, creature base `GameCreature+0x6F8`.
  Port guidance in the doc; any earlier single-record assumption is dead.
- Trip guard for the `0x64d860` FatalError: `bool(#16) && bool(#17)` = payload bytes **+54/+55 as
  independent u8s** (B11's "bytes 2/3 of #10" mapping refuted; both ARE written, `0x414b42/0x414b61`).
  Corrected census: **(0,0) on 57/57 shipped GWAM chunks → 0 trips, unconditionally**; rule for
  custom records: trip iff payload[54]!=0 && payload[55]!=0. T-019 FINAL.
- **Creature-twin real deltas:** grammar-identical clone (89.7% normalized identity; diffs = register
  allocation) with two exceptions — (1) NO caliber→mode derivation on the creature path: the mode
  slot is address-taken at `0x40e0ad` but never stored ⇒ **mode = stack residue on every creature
  path** (shipped defect, same family as Apply's `var_B1` and the attack-record heap residue); (2)
  list base offset differs (+0x6F8 vs +0x40C).
- Weapon caliber→style map (bytes at `0x414e54` = `00 00 00 00 00 01 01 01 02 02`, verified from the
  PE): `[this+0x3B0] in 1..5 -> mode 1`, `6..8 -> 0`, `9..10 -> 3`; `#17 -> 2` override; `0` or
  out-of-range falls through to the **uninitialized mode lane** (hazard flagged). Names of {0,3}
  stay numeric — semantic naming needs the comparators over the `+0x40C` list nodes (next hop).

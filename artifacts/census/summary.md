# Shipped-data census snapshot

- source dir hash: `99ac4dab13fc`
- generator: tools/data_census.py (T-025)
- boundary: on-disk structure only; runtime semantics NOT proven (see header docstring)
- MAPPING VERSION: gwam consumed/thrown = FastArrow:B11-corrected (consumed=byte@off+50, thrown=last body byte); the T-019-stage-1 naive tail pair is emitted alongside as naive_b(-2)/naive_b(-1) so a mapping disagreement is VISIBLE, not reconciled silently.
- ROUTES: 'chain' = top-level chunk-chain walk of contiguous TOC extents; 'sweep' = structural GWAM-v5 find inside extents not covered by a chain hit (nested records).
- gwam per route: {'sweep': 57} (chain=0, sweep=57); region filter: all *.grp under data/ (per-file rows carry the file column)

| file | TOC | resources bytes | parsed bytes | UNPARSED | chunks |
|---|---|---|---|---|---|
| Critters.grp | ok | 50257688 | 131072 | 50126616 | 1 |
| Demo/data/Sounds.grp | ok | 165869208 | 0 | 165869208 | 0 |
| Effects.grp | ok | 1894288 | 0 | 1894288 | 0 |
| Engine.grp | ok | 474817 | 263 | 474554 | 4 |
| Interface.grp | ok | 33209922 | 512 | 33209410 | 1 |
| Items.grp | ok | 2819640 | 0 | 2819640 | 0 |
| Maps.grp | ok | 231300529 | 11855 | 231288674 | 107 |
| Props.grp | ok | 18441153 | 2827 | 18438326 | 1 |
| Scripts.grp | ok | 5810966 | 0 | 5810966 | 0 |
| Sounds.grp | ok | 165869208 | 0 | 165869208 | 0 |
| Templates.grp | ok | 0 | 0 | 0 | 0 |
| Tiles.grp | ok | 74208888 | 0 | 74208888 | 0 |
| _AMO.grp | ok | 5652 | 1718 | 3934 | 24 |
| _ARM.grp | ok | 6628 | 2190 | 4438 | 28 |
| _CHM.grp | ok | 0 | 0 | 0 | 0 |
| _CON.grp | ok | 10536 | 3610 | 6926 | 64 |
| _CRT.grp | ok | 55984 | 21956 | 34028 | 145 |
| _DOR.grp | ok | 2440 | 814 | 1626 | 16 |
| _Dev.grp | ok | 0 | 0 | 0 | 0 |
| _ESF.grp | ok | 0 | 0 | 0 | 0 |
| _ITM.grp | ok | 1059 | 255 | 804 | 3 |
| _MSC.grp | ok | 0 | 0 | 0 | 0 |
| _RLZ.grp | ok | 0 | 0 | 0 | 0 |
| _USE.grp | ok | 3343 | 1084 | 2259 | 18 |
| _WEA.grp | ok | 21570 | 9977 | 11593 | 94 |

## counts + distributions

- GWAM v5 records: 57; consumed!=0: 0; thrown!=0: 3
- GITM records: 33; eqslot hist: {0: 7, 1: 1, 1699217421: 5, 2: 20}
- GIAR records: 7
- EME2 records: 64; chunks that FAILED to tile (counted unparsed, never guessed): 0 
- trigger-region strings (EMTR/EBTR/EMEP sweep): 0 — NAMED GAP: walked payloads are NOT name blobs (EMTR @Maps.grp 0x4270b4a = u32 counts + f32 geometry; EBTR @0x4270c06 = wrapper embedding EMNP/ETRE/HEAD sub-chunks; EMNP sample @Engine.grp 0x4212f = 4 zero bytes). Trigger NAMES, if they ship, live in the un-parsed (compressed) region or a chunk class this sweep has no evidence for. Empty here means NOT-FOUND-WALKABLE, never absent.
- EME2 name-kind histogram: {'CRT': 2, 'USE': 1, 'DOR': 1, 'WEA': 32, 'AMO': 18, 'CON': 9, 'ITM': 1}
- chunk tags: {'SILE': 1, 'EMAP': 14, 'EMEP': 3, 'ECAM': 14, 'EMNP': 13, 'XFIL': 1, 'EME2': 64, 'EMEF': 1, 'EMTR': 1, 'EBTR': 1, 'L555': 1, 'EEN2': 94, 'GENT': 94, 'GITM': 33, 'GIAM': 6, 'GIAR': 7, 'GOBJ': 26, 'GCON': 16, 'GWAM': 57, 'GCRE': 36, 'GDOR': 4, 'GIWP': 19}

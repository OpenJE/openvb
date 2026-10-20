# 2025-10-20 — T-015: GWAM attack-mode record — wire grammar, derived fields, both jump tables (pre-map)

Task: T-015. Author: FastArrow. Target: ORIGINAL_REFERENCE. Read-only. Disasm pass on
`sub_4148A0` (IDA decompile fails — SEH frame); parser self-names via its error string:
**`GameWeapon::ReadAttackModeData()`**. Called from `GameWeapon::ReadData_0x415190` (GWAM
tag `0x4D415747` verified per record @0x415238; error "Unexpected chunk ID. Attack Mode ID
'GWAM' expected."; modes count u32 @weapon+0x404 read pre-loop along with two dwords @
weapon+0x3B8/+0x3C4). Creature-side twin reader @0x40E473 area shares the grammar (one
tag-compare sighted; full parity check = queue).

## 1. Wire field order (stream of u32/u8 via 0x498170; STRING via 0x49C8D0), ver-gated

| # | read | gate | notes |
|---|---|---|---|
| 1,2 | u32 u32 | always | first two fields (feeding modeRec+0x08 style-adjacent pair, see §3) |
| 3..12 | 10 × u32 | always | includes DT (+0x10-anchored), ammo cost, flat bonus, dice min/max, … |
| 13 | u8 | always | normalised `!=0` into record byte (low byte rewrite of field 12's slot @0x414A99) |
| 14 | u32 | ver≥1 | + u8 normalised into its byte-1 (@0x414AE6-414AF6: `[+1] := bool`) and a second u32 with byte-2 patched (@0x414B25-4B42) — **packed bool fields** |
| 15 | u8 | ver≥1 | → byte, then u32 |
| 16 | u8 | ver≥1 | normalised → rec byte (@0x414B53-4B61); **if this byte != 0 AND byte@+1-slot != 0 → FatalError "GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown."** (0x64D860, @0x414B6F) |
| 17 | u32 | ver≥2 | |
| 18 | u32 | ver≥3 | |
| 19 | **string** | ver≥4 | (ammo class name? — sub_49C8D0 length-prefixed) |
| 20,21 | u8 u8 | ver≥5 | consumed by the consumed+thrown check above *(ordering: sighted immediately after v5 test)* |

## 2. Derived fields (computed, NOT on the wire)

1. **modeType / attack-style**: from weapon CALIBER (`weapon+0x3B0`-1 index, 10-entry byte
   table @**0x414E54**, 3-way jump table @0x414E48): class values {0→style 1, 1→style 0,
   2→style 3 (=edi)}; overridden to style 2 when the ver≥5 flag byte is set (§1 #16 —
   **thrown/2 wins from script-set flag**). Attack-doc cross-check: style ∈ {0,1,2,3},
   `!v5||v5==3` → melee/unarmed path, `1|2` → ranged, else "Invalid attack type" —
   so 0=melee-ish, 1=ranged-firearm, 2=thrown, 3=unarmed-class *(mapping of 0 vs 3 to
   melee/unarmed: pick from ResolveWeaponAndAttackMode's out-param — one grep, queued)*.
2. **anim clip id**: if derived slot==0 (§1 pair?) → switch on the field-1 value
   (1..0x22, jump table @**0x414E60**, 34 cases) assigning constants 0x35,0x36,0x37,
   0x3B,0x3C,0x3D,0x3F,0x40,0x41,0x53,0x54,… = **53..84+ decimal — the QueueAnimEvent id
   space** (gcre-versions doc's "ids 53..104 == GWAM modeType map" — now produced from the
   parser side). One case computes `0x38 | ((flag<0)&6)` → {0x38,0x3E} = 56/62 pair from the
   §1-#13 bool. Out-of-range/nonzero path leaves the slot for §-later (0x414E02 default).

## 3. Record block & downstream anchor

Construction base = record block, copied by `sub_40A240` (error-path sight) / appended into
the weapon modes vector (0x433760-insert pattern via vector class at 0x401B00-family).
Consumer anchoring vs the attack-state doc's modeRec offsets: modeRec = recordBase+0x18
(style@modeRec+0x08 = wire#2's slot, dt@+0x10 = wire#3, ammo@+0x1C, flat@+0x24, range@+0x2C,
diceMin/Max@+0x36/0x38, interval@+0x44) — **consistent for the four sites I re-checked
this pass**; full slot table needs a frame reconstitution of 0x4148A0 (pushed-esp shifts,
the known trap) — queued as the follow-up that turns this PRE-MAP into the canon map.

## 4. Confidence tags
- §1 order & gates: HIGH (linear reads, sequential je-chain, ver compares on chunk hdr +0xC
  at 0x414B7C/4B99/4BBC/4BC0/4BD8).
- §2 tables: HIGH for existence/entry-count, MED for per-index values pending the byte
  dump of 0x414E54 decoded against caliber-1 (8 of 10 entries sighted).
- §3 anchor: MEDIUM-HIGH (4 cross-checked offsets); the frame reconstitution promotes it.

## 5. Queue from here
1. Frame reconstitution pass → exact record slot table (this doc's promotion criteria).
2. 0x40E473 creature twin — confirm identical grammar, note divergences.
3. ResolveWeaponAndAttackMode out-param semantics to pin style 0 vs 3 labels.
4. modeRec+0x64/0x68 (ammoBonus min/max) — sighted? verify next pass.

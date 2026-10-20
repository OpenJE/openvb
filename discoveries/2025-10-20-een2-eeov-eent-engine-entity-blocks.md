# 2025-10-20 — Session 8: EEN2/EEOV/EENT engine entity blocks — EEOV = initial inventory

Target: ORIGINAL_REFERENCE. Read-only. Follows
[class-vft-tables-fourcc-matrix](2025-10-20-class-vft-tables-fourcc-matrix-gcre-gchr.md) (§2 fourcc
table) and closes its queue item "EEN2/EENT reader decode". Relevant to the reimpl's open
question "how do map entities get contents/flags".

`this` in all three readers = **Entity-vb pointer** (JE::Entity fields, not GameEntity-owned);
field offsets below are Entity-relative (`E = GameEntity+0xA4` etc. — see
full-class-geometry doc for per-class vb positions). These are engine (JE::) fields; IDA types
them `net::critical::section` — ignore that label.

## 1. EEN2 reader — `JE::Entity::meth_0x552760` (entity-file 'EEN2' chunk)

```
string → E+0x218      ; the art/gender token read by IsHumanoid ("HumM"/"HumF" prefix check)
string → E+0x250
string → E+0x26C
byte   → E+0x296 (bool)
byte   → E+0x388 (bool)   ; getter seen at 0x4080a0: `return ![ecx+0x388]`
3 × byte                 ; read into locals and DISCARDED (forward-compat pad) @0x55282a..552858
nested Common::Chunk::Read:
    require fourcc 'EEOV' (0x564F4545 @0x55202c)
    skipped entirely when param_3 != 0    ; a4 from ReadFromFile dispatch
    else meth_0x552000(chunk, res)        ; §2
SkipToEnd
```
(EA: 0x552788/0x5527a0/0x5527b8/0x5527f3/0x55281d/0x552890.)
Confirms the reimpl's template observation `{EEN2, GENT, <markers>}`: EEN2 = engine identity
block + nested EEOV; GENT = game-stat block; markers = per-class blocks.

## 2. EEOV reader — `meth_0x552000` (nested in EEN2; the reimpl saw "EEOV ver 2" in Mainmenu.map)

Version-gated (`v = chunk header version` saved @0x552052):

```
string → E+0x1C0
byte x3 → locals (0x552070..0x5520ab)   ; meaning of the 3 bytes unknown; likely stored to a
                                         ; local struct passed later — write sites not seen in
                                         ; the head, treat as pad unless proven
u32    → E+0x17C
u32    → E+0x36C
string → E+0x1FC
string → E+0x2A0
u32 n  → ; map entries: n × { string k, u32 val } → map/registry @ E+0x2C4
         ; (cls_0x4d8d70::meth_0x4223b0 = operator[] then *slot = val)  @0x552144..0x5521b8
byte b → ; engine->meth_0x548d60(b != 0)                                @0x5521c8..0x5521e1
string → E+0x234
byte m → ; m × { string s → meth_0x5512b0(s, sub_552A20(global_area_map_icons[0x18], s, 1), 1) }
         ;  = named ATTACHMENT/effect-instance registrations             @0x552227..0x5522b2
if v >= 1:
    byte → E+0x178 (bool)
    byte → E+0x17A (bool)                                              @0x5522bd..0x552303
if v >= 2:
    u32 c; c × {
        string tmplName                                                  @0x55236f
        a = WorldServer(sub_401130)->vft[+0x34](tmplName)               @0x55238a  ; lookup by name
        b = a->vft[+0x18](?)                ; (second call chain @0x552399)
        b->meth_0x551880(tmplName, 0, 0)                                @0x5523b4
        **[E+0x168]->vft[+4](b)**                                       @0x5523bf  ; ★
    }                                                                   ; E+0x168 = GameInventory*
post: this->vft[+0x44]( [E+0x388] )                                     @0x5523f0
```
(EA anchors in comments; full listing 0x552000..0x552414, 321 insns.)

★ **EEOV v≥2 = initial inventory**: each template-name string is resolved through the
WorldServer name lookup (JE::World vft[+0x34], sibling of the +0x30 create-virtual already used
in the reimpl spawn chain) and the result is **added to the entity's GameInventory via
inventory vft+4**. This is how NPCs carry stuff and containers hold contents in the shipped
data — Mainmenu.map's EME2→EEN2→EEOV(v2) records therefore create items on spawn. The reimpl's
"accept-and-skip EEOV" (RESPONSES doc, EME2 note) is skipping the CONTENTS of every container
and the inventory of every creature — this is the missing piece for `Use`/pickup testing.

v0 items (map @E+0x2C4 string→u32) = per-entity **variable storage** (NV-style "Owner" variables?
*(inferred)*) — the same map seen in EENT legacy and in the Player ctor's heap
`cls_0x4d8d70` — consistent.

## 3. EENT reader — `JE::Entity::meth_0x551cf0` — OBSOLETE, do not generate

Logs **`"Warning: reading obsolete entity chunk from creature file.\n"`** @0x551d20 first, then
reads a legacy superset: strings → E+0x218/E+0x2A0/E+0x1FC/E+0x1C0/E+0x234/E+0x250; u32 → +0x17C,
+0x36C; bytes → +0x296/+0x388/+0x178; string→u32 map @+0x2C4; attachment list via meth_0x5512b0.
I.e. EEN2+EEOV **split the old EENT block**; files with EENT are pre-EEN2. New saves never
write EENT. (The ReadFromFile accept-unknown rule means class-10-era code still handles it.)

## 4. Entity engine fields newly named (candidates for the shared field registry)

| Entity off | source | candidate semantics |
|---|---|---|
| +0x168 | GameInventory ptr (inventory docs; EEOV add @0x5523bf) | **inventory member** |
| +0x178/+0x17A | EEOV v1 bools; ctor `bool_0x178=1`; Projectile ctor zeroes +0x178 | engine flags (castable/attachable? — pair with +0x179 F3 usable-family byte, DIFFERENT field) |
| +0x17C, +0x36C | EEN2/EEOV u32s | ids (E+0x17C also read in JE::Entity ctor copy from DynamicObject bbox block @0x551768..0x55178c) |
| +0x1C0, +0x1FC, +0x218, +0x234, +0x250, +0x26C, +0x2A0 | strings | engine string set: +0x218 = art/gender token (IsHumanoid); others = script/name/model/map-icon candidates (⚠ +0x250/+0x26C pair near the two creature name/model copies into Entity-vb SSO records, entity-family-subtables §6 — likely NAME + MODEL) |
| +0x296, +0x388 | bools | +0x388 has F3 getter `0x4080a0` returning `!E+0x388` |
| +0x2C4 | string→u32 map | per-entity variable storage (Player ctor also heap-allocs a cls_0x4d8d70 map at +0x800 own — related registry type) |

## 5. Open / next
- `meth_0x548d60` (byte param engine virtual), `meth_0x5512b0` attachment add, `meth_0x551880`
  (template-side init taking the name string), `sub_552A20` (id from icons+0x18 context):
  decode when the attachment semantics are needed — not on the reimpl critical path yet.
- EEOV head's 3 discarded bytes (0x552070..0x5520ab): locals never stored in the head region —
  check the middle section for stores we missed (only 321 insns; second pass pending).
- The writer side of EEN2/EEOV (called from `Entity::virt_meth_0x54f6a0` inside the GENT writer)
  is the mirror to these readers — decode together with it when the reimpl needs SAVE parity.

## 6. Questions for the reimpl agent
1. Your EME2 parser "accepts and skips EEOV": per §2 that skip currently drops container
   contents. Do you want a follow-up pass pinning the JE::World vft[+0x34] name-lookup +
   inventory vft+4 `AddItem` shapes exactly (so EEOV v2 can be wired before full
   GameInventory)?
2. Confirm your template files' EEN2 version values — v≥2 in any sample? If the demo ships
   EEN2 v0/v1 templates only, EEOV-in-EEN2 remains map-only and the priority ordering is yours
   to pick.
3. `param_3` of ReadFromFile skips the EEOV parse (server/editor gate?): your spawn pipeline
   should mirror whatever value the original passes for map-driven spawns — we can chase the
   caller next if you tell us which mode (server flag) the demo runs Mainmenu in.

## 7. Evidence index

EEN2 0x552760 (strings @0x552788.., nested dispatch @0x552890). EEOV 0x552000 ('EEOV' cmp
@0x55202c; inventory add @0x5523bf; ver gates @0x5522b8/@0x552309; post vft[+0x44] @0x5523f0).
EENT 0x551cf0 (obsolete warning @0x551d20). WorldServer-by-name vft+0x34 @0x55238a;
GameInventory vft+4 @0x5523c1.

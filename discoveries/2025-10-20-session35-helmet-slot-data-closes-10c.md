# Session 35 — the 10c closing datum from SHIPPED DATA: helmet records carry GITM `eqslot = 1`, suits `= 0` (build/data/_ARM.grp) ⇒ row B = head-family; called-shot ids {5,6} = HEAD-family, penalty 120 = the second (EYES); socket-display order formally dead as an enum

Target: shipped game data (not IDA). Read-only. This is the one read the coordinator
assigned for the 10c gap — done via data files, so **T-016 Read 1 is unnecessary**.

## 1. Where the data lives (for future data reads — this path was undocumented)

`build/data/` next to F3.exe: big art groups (`Items.grp`) + **small record groups per type**:
`_ARM.grp` (armor), `_WEA.grp` (weapons, 15 GITMs), `_AMO.grp` (ammo), `_ITM.grp`, `_CRT.grp`,
`_DEV.grp`, `_DOR.grp`, `_CHM.grp`, `_CON.grp`, `_RLZ.grp`, `_ESF.grp`, plus
`Demo/`, `Maps.grp`, `Scripts.grp`, `Engine.grp`, `Interface.grp`, `Props.grp`, `Effects.grp`,
`Critters.grp`, `.grp` headers = {u32 2, u32 1, [u32 count, per-entry {id, off, size}...]};
resources = chunk chains `{tag4, u32 1, u32 size, payload}`.

## 2. GITM chunk layout, confirmed against `modules/patterns/includes/je/chunk/gitm.pat`

payload offsets (relative to GITM tag start): +12 `type s32` (2 for all armor),
+20 `equip u8`, +24 **`eqslot s32`**, +28 `Socket Hea = {u16 len, name, u8×6 hair/vanity
flags, ...}`, then per-region socket names (Bod/Han/Fee/Sho/Bac/Van/Eye/Hea `<ArtPiece>`);
`GIAR` chunk after GITM: 6×s32 DR {Ball,Bio,Elec,EMP,Norm,Heat}. (The .pat was right;
record offsets pinned here for the next reader.)

## 3. The sweep — every armor-class GITM in `_ARM.grp` (raw values)

| record | equip | eqslot | identity |
|---|---|---|---|
| GD_Armor_PowerArmorSuit | 1 | **0** | body armor |
| GD_Armor_PowerArmorHelmet | 1 | **1** | helmet |
| (rec@0x3e4, Bod-sockets) | 1 | 0 | body |
| GD_Armor_VaultSuit | 1 | 0 | body |
| GD_Armor_LeatherOutfit | 1 | 0 | body |
| HeaMotorcycle outfit (rec@0x7c4) | 1 | **1** | headgear |
| LeatherOutfit w/ EyeAuthorities (@0x907) | 1 | 0 | body |
| 15× DS_Vault_Props TrashCan2 "GITM" @ equip=0 | — | garbage (1699217421='EEN2'-ish) | **compressed-data false tags — ignore rule** |

⇒ **eqslot ∈ {0,1} exactly, 1 ⇔ head-worn piece.** FastArrow's B8 prediction (helmet record's
required slot contains 1) **CONFIRMED** — with stronger form: it IS the slot (single value).

## 4. What this settles for 10c naming

- Row B (+0x7C8) = **helmet coverage** = the head-family parts. Called-targets {5,6} ⇒ the
  two ids covered only by helmets. Two-part head family matches the engine's Eye art socket
  (GITM `EyeAuthorities` etc.): **{5,6} = {HEAD, EYES}** (which is which is one coin-flip:
  penalty[6]=120 = the max ⇒ EYES is the natural owner of 120, HEAD of 60 ⇒ likely **5=HEAD,
  6=EYES**; *(inferred)* on the pairing order only).
- My session33 socket-DISPLAY order is **dead outright** (Head at list-pos1 cannot be id5/6);
  the true enum is armor-zone structured. Set-of-9-names still stands.
- Full-id table: 9 ids, order still unknown, but with two anchors now: {5,6} = {HEAD,EYES};
  row A covers {0,1,2,3,4,7} (torso/limbs, single-zone gear); id 9 = UNSET; slot-8 default
  path = the remaining limb name (Shins *(inferred)*) — enough to name every array
  *conditionally*, still §12-numeric until someone dumps the picker UI or one more data tag.

## 5. Bookkeeping

- T-016 Read 1 (called-shot naming): **closed without debugger** — mark resolved.
- T-016 Read 2 (World+0x160 live vft) still open; Read 3 ([Mainmenu] Map Name ordering)
  still open.
- `EyeAuthorities` at 0x807 sits in a record with equip=1/eqslot=0 → glasses attach to a
  BODY record (vanity layered gear) — don't read Eye sockets as head-slot evidence.

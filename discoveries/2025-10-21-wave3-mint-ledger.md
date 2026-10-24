# WAVE-3 — adjudicated-mints ledger (writer: HappyStorm `01a16092`, claim 02ff46f, close this commit)

IDB chain: baseline 00:56:49Z pre-write sync (live==disk) -> 50 renames 00:57:22Z cp -> 34 comments 00:58:16Z cp -> END **00:59:02Z**. Gate 6/6 @ md5 a68c22f3 first. Dumps w3-predump/w3-postdump. **Reconciliation: 50 name-diffs, 0 unexpected, 0 missing; T2 1892->1847 = forecast MINUS-45 EXACT** (46-minus-skipped-row).

## Flags for EpicRaven/coordinator (print-not-paper, three)

1. **0x4903E0 row DROPPED pre-apply**: live IDB already `JE::Gfx_RenderManager::meth_*` (wave-1 applied T-073 [A2] composite AFTER the class-1 table was cut from t064-postdump). Table's `doc=JE::Gfx_Engine` + key `field-load @0x5be073` is the Gfx_Engine TRAMPOLINE that calls `this=*[[holder+0x18]+0x164]` — a prefix-shape, superseded by T-073's terminal-field ruling (fb3d03f law). Applying it would have UNDONE a correct apply. Table refresh suggested; row counted among the 52 but resolves to ALREADY-CORRECT.
2. **Table-vs-prose 4/3-vs-3/4 strong/weak drift** (commit 2086aef '3 strong mints, 4 weak'; TABLE shows 4 STRONG-key rows + 3 WEAK): executed TABLE (regenerator output is the record); both counts stated.
3. **4 feed-unique rows EXCLUDED** — cross-check ask still open at seating; join wave-4 if cleared.

## WEAK verdicts (writer discretion, per row's own clause): all 3 ANNOTATE-CONSUMER, none minted
- 0x52CC70: band regen = consumer field-load only; band cls_0x401b00 identified as SHARED _Tree insert/find machinery (0x40E780 = THE keyed-map inserter) — Gregorian membership NOT confirmed.
- 0x5150C0: own key field-load @0x51C3E2 WEAK; sibling 0x515040's heap-ctor (STRONG, minted) proves only ITS membership — per clause.
- 0x436410: keys 'other x4'/'GameInventory other x3' WEAK; shared-map band resident — per clause.

## STANDS recorded no-action (0x515580, 0x4A5720) — §3 cls_0x40cc80 annotation remains, rule AMENDMENT-3.

## HOLD list (21 rows, ambiguity gate; comments landed with the reason)
| EA | reason (truncated) |
|---|---|
| 0x442BF0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C1 RESOLVED 2026-10-24 (owner chase): NO owner class exists - the class IS the five-interface switch module (five member pairs this+8..+2C, per-case CLoadMapInterface show/hide + [vft+0x168] tags); sole consumers F3::CommandOptionsWindow + F3::CommandGameOptions; amendment-3 verdict: module CONFIRMED, domain rename withheld (no confident shipped name)** |
| 0x4433E0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C1 RESOLVED 2026-10-24: the 9-instruction wrapper (meth_0x442bf0 + meth_0x442dc0 param 1-5 switch) selecting which interface to open; same consumer evidence as 0x442BF0; amendment-3 verdict stands** |
| 0x55BF60 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C2 RESOLVED 2026-10-24 (owner chase): owner PINNED - instance = GameServer.mbr_0x5064 (JE::Server field; both consumers chain global_area_map_icons->[+18]->[+5064]); body = the list-contains predicate ([node+0x18]==param OR [node+0x1C]==param, walk via cls_0x55bb70 iterator); sole consumer JE::IsInDialogue; amendment-3: rename withheld - cls_0x55bf60/cls_0x55bfa0 are likely ONE class mis-split, no confident shipped name** |
| 0x55BFA0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C2 RESOLVED 2026-10-24: the list-FIND variant ([node+0x1C]==param -> returns [node+0x18] the dialogue object); sole consumer JE::PCSpeaker anchored by its own "Could not find a dialogue" string; owner = JE::Server via GameServer+0x5064; amendment-3: rename withheld (mis-split pair, see 0x55BF60)** |
| 0x5813A0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: vsync GET (render-device vft +0x14 -> flag byte +0x49; CommandVSync "VSYNC %s"); amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x5813C0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: vsync SET (+0x49=param; tail-jmp [this+8]->vft[+0x18]; CommandVSync); amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x5813E0 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: adapter wrapper forwarder vft[+0xC](mbr_0x2C idx, mbr_0x30 count); CommandAdapterInfo; amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x581400 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: sibling forwarder vft[+0x10](mbr_0x2C, mbr_0x30); CommandAdapterInfo; amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x581420 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: GetCurrentDisplayMode forwarder vft[+0x34]; CommandSetDisplaymode ("Mode = %d x %d x %s @ %dHz %s", stencil flag); amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x581430 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: adapter-record math: mbr_0x2C*0x44C + byte_708380 + 0x200; CommandAdapterInfo; amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x581470 | bare-namespace holder: key sites resolve to F3::Command*/JE: - **C3 RESOLVED 2026-10-24 (owner chase): the 0x5813xx-0x58147x band = the DISPLAY-SETTINGS WRAPPER family (thin forwarders over held interfaces; no owner class beyond the console-command layer); this row: cls_0x581460 forwarder -> cls_0x4b9df0 via [this+0x10]; CommandToggleBuilding + sub_58A890; amendment-3: rename withheld (no confident shipped name for a forwarder wrapper)** |
| 0x5817A0 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x59AAA0 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x5B2900 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x5B2990 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x5B2C80 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x5B2CD0 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x5B5640 | bare-namespace holder: key sites resolve to F3::Command*/JE: |
| 0x410A90 | key holder sub_411330 is an anonymous caller, not a class ba |
| 0x591F70 | key holder sub_595C40 is an anonymous caller, not a class ba |
| 0x591FD0 | key holder sub_595C40 is an anonymous caller, not a class ba |

## Applied renames (50 rows; recovery table: EA | pre-wave name | applied name)
| EA | was | now |
|---|---|---|
| 0x4216B0 | cls_0x4203c0::meth_0x4216b0_4216b0 | JE::AreaMapWindow::meth_0x4216b0_4216b0 |
| 0x431000 | cls_0x431000::meth_0x431000_431000 | JE::CounterControl::meth_0x431000_431000 |
| 0x431220 | cls_0x40cc80::meth_0x431220_431220 | JE::CounterControl::meth_0x431220_431220 |
| 0x4327E0 | cls_0x40cc80::meth_0x4327e0_4327e0 | JE::CounterControl::meth_0x4327e0_4327e0 |
| 0x43A420 | cls_0x43a420::meth_0x43a420_43a420 | F3::GameActionEquip::meth_0x43a420_43a420 |
| 0x441D90 | cls_0x441d20::meth_0x441d90_441d90 | F3::GameActionEquip::meth_0x441d90_441d90 |
| 0x44DCB0 | cls_0x44ad50::meth_0x44dcb0_44dcb0 | F3::GameActionEquip::meth_0x44dcb0_44dcb0 |
| 0x44FC70 | cls_0x44ad50::meth_0x44fc70_44fc70 | F3::GameActionEquip::meth_0x44fc70_44fc70 |
| 0x45C800 | cls_0x4dff90::meth_0x45c800_45c800 | JE::Graphics::meth_0x45c800_45c800 |
| 0x45C8E0 | cls_0x4dff90::meth_0x45c8e0_45c8e0 | JE::Graphics::meth_0x45c8e0_45c8e0 |
| 0x467780 | cls_0x467780::meth_0x467780_467780 | JE::Gfx_AnimMeshInstance::meth_0x467780_467780 |
| 0x490450 | cls_0x490450::meth_0x490450_490450 | JE::GUI::Skin::meth_0x490450_490450 |
| 0x4904B0 | cls_0x490450::meth_0x4904b0_4904b0 | JE::GUI::Skin::meth_0x4904b0_4904b0 |
| 0x4960D0 | cls_0x4960d0::meth_0x4960d0_4960d0 | F3::GameActionOperateContainer::meth_0x4960d0_4960d0 |
| 0x4962C0 | cls_0x4960d0::meth_0x4962c0_4962c0 | F3::GameActionOperateContainer::meth_0x4962c0_4962c0 |
| 0x49CA00 | cls_0x49ca00::meth_0x49ca00_49ca00 | JE::ModelInstance::meth_0x49ca00_49ca00 |
| 0x49CAF0 | cls_0x49caf0::meth_0x49caf0_49caf0 | JE::ModelInstance::meth_0x49caf0_49caf0 |
| 0x49DD80 | cls_0x49db40::meth_0x49dd80_49dd80 | JE::ModelInstance::meth_0x49dd80_49dd80 |
| 0x4A1F30 | cls_0x4a1f30::meth_0x4a1f30_4a1f30 | JE::VFX_Model::meth_0x4a1f30_4a1f30 |
| 0x4A4BA0 | cls_0x4a1f30::meth_0x4a4ba0_4a4ba0 | JE::VFX_Model::meth_0x4a4ba0_4a4ba0 |
| 0x4B48E0 | cls_0x490450::meth_0x4b48e0_4b48e0 | JE::GUI::Skin::meth_0x4b48e0_4b48e0 |
| 0x505540 | cls_0x505540::meth_0x505540_505540 | JE::Gfx_AnimMeshInstance::meth_0x505540_505540 |
| 0x539CC0 | cls_0x539d90::meth_0x539cc0_539cc0 | JE::CSoundSystem::meth_0x539cc0_539cc0 |
| 0x55BB10 | cls_0x55bb10::meth_0x55bb10_55bb10 | JE::DialogueManager::meth_0x55bb10_55bb10 |
| 0x55BDC0 | cls_0x55bb10::meth_0x55bdc0_55bdc0 | JE::DialogueManager::meth_0x55bdc0_55bdc0 |
| 0x55E910 | cls_0x55bb10::meth_0x55e910_55e910 | JE::DialogueManager::meth_0x55e910_55e910 |
| 0x55F360 | cls_0x55bb10::meth_0x55f360_55f360 | JE::DialogueManager::meth_0x55f360_55f360 |
| 0x570400 | cls_0x574e20::meth_0x570400_570400 | JE::Map::meth_0x570400_570400 |
| 0x571510 | cls_0x570800::meth_0x571510_571510 | JE::Map::meth_0x571510_571510 |
| 0x5810F0 | cls_0x5810f0::meth_0x5810f0_5810f0 | JE::Client::meth_0x5810f0_5810f0 |
| 0x581190 | cls_0x581190::meth_0x581190_581190 | JE::AreaMapWindow::meth_0x581190_581190 |
| 0x5B4F50 | cls_0x5b4f50::meth_0x5b4f50_5b4f50 | JE::Client::meth_0x5b4f50_5b4f50 |
| 0x5B4F60 | cls_0x5b4f50::meth_0x5b4f60_5b4f60 | JE::Client::meth_0x5b4f60_5b4f60 |
| 0x569790 | cls_0x569f10::meth_0x569790_569790 | JE::World::meth_0x569790_569790 |
| 0x569970 | cls_0x569f10::meth_0x569970_569970 | JE::World::meth_0x569970_569970 |
| 0x569D50 | cls_0x569f10::meth_0x569d50_569d50 | JE::World::meth_0x569d50_569d50 |
| 0x56A6F0 | cls_0x56ae70::meth_0x56a6f0_56a6f0 | JE::World::meth_0x56a6f0_56a6f0 |
| 0x56A8D0 | cls_0x56ae70::meth_0x56a8d0_56a8d0 | JE::World::meth_0x56a8d0_56a8d0 |
| 0x56ACB0 | cls_0x56ae70::meth_0x56acb0_56acb0 | JE::World::meth_0x56acb0_56acb0 |
| 0x56D030 | cls_0x56d030::meth_0x56d030_56d030 | cls_0x570800::meth_0x56d030_56d030 |
| 0x56D090 | cls_0x56d030::meth_0x56d090_56d090 | cls_0x570800::meth_0x56d090_56d090 |
| 0x5ACA90 | cls_0x5ac9e0::meth_0x5aca90_5aca90 | net::connection::meth_0x5aca90_5aca90 |
| 0x5AD7B0 | cls_0x5ad7b0::meth_0x5ad7b0_5ad7b0 | JE::AreaMap::meth_0x5ad7b0_5ad7b0 |
| 0x515040 | cls_0x515040::ctor_0x515040 | JE::Gfx_ParticleSystem::ctor_0x515040_515040 |
| 0x515D50 | cls_0x40cc80::meth_0x515d50_515d50 | JE::Gfx_ParticleSystem::meth_0x515d50_515d50 |
| 0x4A5A10 | cls_0x40cc80::meth_0x4a5a10_4a5a10 | JE::Gfx_AnimMeshInstance::meth_0x4a5a10_4a5a10 |
| 0x523450 | cls_0x4dff90::ctor_0x523450 | JE::GUI::Skin::ctor_0x523450_523450 |
| 0x4B2DD0 | JE::StringTree::meth_0x4b2dd0_4b2dd0 | std::_String_base::dtor_0x4b2dd0_4b2dd0 |
| 0x56EE40 | JE::StringTree::meth_0x56ee40_56ee40 | std::_Two_string_struct::dtor_0x56ee40_56ee40 |
| 0x539370 | JE::CSound::meth_0x539370_539370 | JE::CMusic::meth_0x539370_539370 |

## ST-RENAME + CMusic rows (in the 50): 0x4B2DD0->std::_String_base::dtor_*, 0x56EE40->std::_Two_string_struct::dtor_* (body-bounded coined segments; comments pre-landed at R-ST), 0x539370->JE::CMusic::meth_* (derived-in-base, R-CS s9 vft note is the install cite).

## Successor line: 21 HOLDed ns-namespace rows = values-era owner-chase batch (each comment cites this ledger); class-1 TABLE refresh for wave-4 (0x4903E0 row + re-cut from POST-wave-1 dump closes the staleness class); 4 feed-unique rows join when cross-check lands.

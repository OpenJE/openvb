# T-084 — Long-tail batch attribution: the workload number cashed (1,377 fns, per-host queue out)

**EpicRaven**, claim `e4c6aba`. Offline lane: raw EXE md5 **`a68c22f3dd1c256ec9539a70e87cb434`**,
live family enumeration from the freshest banked census **`t064-postdump.json`** (no 8745 use,
no IDB writes). Gates first: selftest **6/6 PASS** (T1 319 / T2 4+installs / T2b / T3 37 /
T5 banked-rulings incl. fb3d03f pair / T4 21-row+0-orphan) before any verdict below.
Share-check (fixed terminal-field machine) run per fn — it is the arbiter.

## 0. Set (T-063 §4 long tail, recounted live)

**1,377 fns / 614 classes** remain `cls_`-anonymous after excluding every already-attributed
family (T-057 trio 244, `cls_0x55dc80` 83, T-063's 199-set classes, vendor 46+148 epoch
classes) — matches T-063's ~1,351 estimate (drift = T-064 renames + boundary counting).
22 classes carry >=8 fns (294 total); median class = 1 fn. Batching is presentation; the
machine swept the full set in one pinned build.

## 1. Buckets + share-check (per fn)

LIVE 984 / DELEG 222 / ISO 171; share-check PASS 1,132 / SHARING 82 / NO-EVIDENCE 163.

## 2. MINT-READY queue (323 fns, grouped by suggested host; terminal-field shapes;
every row passed per-fn share-check — item-44 gate is inside the machine, not implied)

Tier 1 (host carries >=5 fns):
  F3::GameActionEquip                  17  0x436410[&global,embed+0x40] 0x44AD50[field-load] 0x44B2A0[embed+0x-18,field-load] 0x44DB10[field-load] 0x44DCB0[field-load] 0x44FC70[field-load] 0x441D20[embed+0x-34] 0x441D90[embed+0x-34] 0x442450[embed+0x-34] 0x410A90[embed+0x-30C] 0x43A420[field-load] 0x43AE90[field-load,plain-this] …
  F3                                   17  0x5B5640[field-load] 0x5813E0[field-load] 0x581400[field-load] 0x581420[field-load] 0x581430[field-load] 0x5817A0[field-load] 0x442BF0[field-load] 0x4433E0[field-load] 0x4B2C80[&global] 0x5813A0[field-load] 0x5813C0[field-load] 0x581470[field-load] …
  JE::Client                           12  0x5B4F50[field-load] 0x5B4F60[field-load] 0x5B5B10[field-load] 0x569790[embed+0x118] 0x569970[embed+0x118] 0x569D50[embed+0x118] 0x56A6F0[embed+0x108] 0x56A8D0[embed+0x108] 0x56ACB0[embed+0x108] 0x4A4CD0[field-load] 0x5810F0[field-load] 0x5B5C50[field-load,plain-this]
  JE::World                            12  0x569F10[embed+0x118] 0x56AE70[embed+0x108] 0x4E7D80[field-load] 0x4E7F70[field-load] 0x5A9330[field-load] 0x5A94A0[field-load] 0x54A610[field-load] 0x54AA20[field-load] 0x56D7F0[field-load] 0x5811A0[field-load] 0x5A42B0[heap-ctor] 0x5A4340[field-load]
  JE::Map                              11  0x571510[embed+0x234] 0x591F70[embed+0xD64,plain-this] 0x591FD0[embed+0xD64,plain-this] 0x56D030[embed+0xE0,plain-this,plus-oth] 0x56D090[embed+0xE0,plain-this,plus-oth] 0x570400[&global,embed+0xC8] 0x5704B0[embed+0x1DC] 0x572610[embed+0x1DC] 0x581330[field-load] 0x5AD7B0[embed+0x60,plus-other] 0x5B1620[field-load]
  JE::Gfx_Engine                       10  0x5CB740[embed+0x114] 0x5CB790[embed+0x114] 0x5CBDB0[embed+0x114] 0x5CBE50[embed+0x114] 0x5CBE60[embed+0x114] 0x4903E0[field-load] 0x490120[field-load] 0x50B4F0[field-load] 0x50B6B0[heap-ctor] 0x5CB510[heap-ctor]
  JE::Gfx_ParticleSystem                9  0x515580[embed+0x24] 0x515D50[embed+0x24] 0x515040[heap-ctor] 0x5150C0[field-load] 0x5149D0[embed+0xC4,plus-other] 0x513FC0[embed+0x714] 0x515820[embed+0x714] 0x515DD0[embed+0x714] 0x51ABF0[embed+0x48]
  JE::Gregorian                         9  0x52CC70[field-load] 0x425D30[embed+0x10] 0x4292E0[embed+0x10] 0x429BC0[embed+0x10] 0x52C480[field-load] 0x52C760[field-load] 0x52C7D0[field-load] 0x4C6B60[field-load] 0x52C890[field-load]
  JE::Gfx_AnimMeshInstance              8  0x4A5720[embed+0xA0] 0x4A5A10[embed+0xA0] 0x4A5A90[embed+0xB0] 0x4A5B30[embed+0xB0] 0x4A67F0[embed+0xB0] 0x5053C0[field-load] 0x467780[field-load] 0x505540[field-load]
  JE::GUI::Skin                         7  0x523450[embed+0x10] 0x490450[embed+0x4,plain-this] 0x4904B0[embed+0x4,plain-this] 0x4B48E0[embed+0x4] 0x5226D0[embed+0x4] 0x5228B0[embed+0x4] 0x522A10[embed+0x4]
  JE::CSoundSystem                      7  0x539CC0[field-load,heap-ctor] 0x539D90[heap-ctor] 0x53A510[heap-ctor] 0x53A180[field-load] 0x53A1E0[heap-ctor] 0x52CE60[field-load] 0x52D080[heap-ctor]
  JE::AreaMapWindow                     7  0x41D060[field-load] 0x4AE0A0[field-load] 0x4AE100[field-load] 0x4203C0[field-load] 0x4216B0[field-load] 0x41D0C0[field-load] 0x581190[field-load]
  JE                                    7  0x5B2900[field-load] 0x5B2990[field-load] 0x59AAA0[field-load] 0x55BF60[field-load] 0x55BFA0[field-load] 0x5B2C80[field-load] 0x5B2CD0[field-load]
  JE::Graphics                          6  0x45C800[embed+0x1C,plus-other] 0x45C8E0[embed+0x1C,plus-other] 0x4B2AC0[&global] 0x4B4CE0[&global] 0x4B3690[&global] 0x4BDAE0[heap-ctor]
  JE::DialogueManager                   6  0x55BB10[embed+0x8,plain-this] 0x55BDC0[embed+0x8,plain-this] 0x55E720[embed+0x8] 0x55E910[embed+0x8,plain-this] 0x55E960[embed+0x8] 0x55F360[embed+0x8,plain-this]
  net::client                           6  0x5ACA90[embed+0x1C,plus-other] 0x5AB770[field-load] 0x5AB930[field-load] 0x5AB960[field-load] 0x5ABA20[field-load] 0x5B9100[embed+0x18]
  JE::ModelInstance                     6  0x49CAF0[field-load] 0x49D290[field-load] 0x49DD80[field-load] 0x5646B0[field-load] 0x49CA00[field-load] 0x563C40[field-load]
  JE::CounterControl                    5  0x431220[embed+0x4C] 0x4327E0[embed+0x4C] 0x431000[embed+0x80] 0x432550[embed+0x1C] 0x432C60[embed+0x1C]
  JE::GUI::Label                        5  0x4341F0[embed+0xE4C] 0x4A8C40[embed+0xE4C] 0x4A9070[embed+0xE4C] 0x4A9310[embed+0xE4C] 0x4B4180[&global]
  F3::GameActionOperateContainer        5  0x4AED50[field-load] 0x4960D0[field-load] 0x496250[field-load] 0x4962C0[field-load] 0x458770[field-load]
  JE::VFX_Model                         5  0x4A1F30[field-load] 0x4A4BA0[field-load] 0x49DB70[field-load] 0x49DB80[field-load] 0x4F45E0[heap-ctor]
  JE::VisualEffect                      5  0x48D8A0[field-load] 0x48D8E0[field-load] 0x48D910[field-load] 0x48D940[field-load] 0x48DC10[field-load]

Tier 2 (single/double-fn members, heap-children, singleton pins):
  call_cls_0x566490                     4  0x5A60F0[&global] 0x5A64E0[&global] 0x5A7820[&global] 0x5A7DF0[&global]
  JE::AnimationCache                    4  0x4E1DF0[embed+0x4,plain-this] 0x4E1F10[embed+0x4,plain-this] 0x4E1F70[embed+0x4] 0x59B2B0[embed+0x4]
  JE::DialogueWindow                    4  0x592810[embed+0xD64] 0x594D30[embed+0xD64,plain-this] 0x595F50[embed+0xD64,plain-this] 0x5944F0[heap-ctor]
  JE::VFX_VEG                           4  0x48DE90[embed+0x20,plain-this] 0x501B00[embed+0x20] 0x501E50[embed+0x20] 0x501F10[embed+0x20]
  JE::AreaMap                           4  0x57D9B0[embed+0x48] 0x57D040[embed+0x68] 0x5AD7E0[embed+0x40] 0x5AD850[embed+0x40]
  JE::CMusic                            4  0x538F30[field-load] 0x538F50[field-load] 0x538F90[field-load] 0x4C68F0[field-load]
  JE::Gfx_RenderManager                 4  0x505880[heap-ctor] 0x505400[field-load] 0x492310[embed+0x5C] 0x50B520[*global]
  net::connection                       4  0x5ABE20[embed+0x58] 0x5AC2B0[embed+0x58] 0x5AC3E0[embed+0x58] 0x5ABD80[embed+0x40DC]
  JE::VFX_VEGInstance                   3  0x48E630[embed+0x30] 0x48ECE0[embed+0x30] 0x5639D0[embed+0x20,field-load]
  JE::VFX_Trail                         3  0x4F1AE0[embed+0x1F0] 0x4F24D0[embed+0x1F0] 0x4F18C0[embed+0x1D0]
  JE::VFX_Snow                          3  0x4E42C0[field-load] 0x4E4210[embed+0x260] 0x4E6190[&global,heap-ctor]
  JE::Gfx_LineSystem                    3  0x49D180[embed+0x108] 0x50EBD0[embed+0x108] 0x50F560[embed+0x108]
  JE::G3D_Mesh                          3  0x4C8DF0[embed+0x28,plus-other] 0x4D3A70[embed+0x28,plus-other] 0x4D6E90[embed+0x28,plus-other]
  JE::Script_Small                      3  0x5B6270[embed+0x-3C,field-load] 0x5529E0[field-load] 0x5A5FF0[heap-ctor]
  meth_0x459c50_459c50                  3  0x599D70[field-load] 0x599D90[field-load] 0x59A1A0[field-load]
  JE::VFX_Sound                         3  0x52E2E0[heap-ctor] 0x49C9E0[field-load] 0x4E0ED0[field-load]
  JE::Entity                            3  0x54E070[embed+0x2E0] 0x54B590[embed+0x1A8] 0x563900[field-load]
  JE::OCTREE_                           3  0x5B9440[embed+0xDC] 0x5AFFD0[field-load] 0x5B92F0[embed+0xDC]
  JE::CSound                            3  0x4C68B0[heap-ctor] 0x4C6FB0[heap-ctor] 0x52A1F0[*global]
  JE::Gfx_G3D_Mesh                      3  0x4CD010[field-load] 0x4CD040[field-load] 0x4D6CE0[embed+0x80]
  F3::GameWeapon                        2  0x414030[embed+0x10] 0x401D60[embed+0xE4C,field-load]
  JE::VFX_Sprite                        2  0x4F5160[embed+0x22C] 0x4F5D30[embed+0x22C]
  F3::GameActionAttack                  2  0x401900[field-load] 0x402EB0[embed+0x4]
  JE::VFX_Rain                          2  0x4E6140[field-load] 0x4E6090[embed+0x1DC]
  JE::MemorySystem                      2  0x5B31F0[heap-ctor] 0x5B3BC0[heap-ctor]
  JE::G3D_WaterTile                     2  0x4D6F50[embed+0x28] 0x4D7D60[embed+0x28]
  JE::Gfx_ParticleInfo                  2  0x515BA0[embed+0xC4] 0x515ED0[embed+0xC4]
  JE::CNASInterface                     2  0x424A20[field-load] 0x429760[field-load]
  JE::EditTokenInterface                2  0x4C5AC0[&global] 0x56BD50[&global]
  F3::GamePlayerAvatarManager           2  0x4C6130[&global] 0x563A00[field-load]
  JE::CLoadMapInterface                 2  0x457A80[embed+0x140] 0x458050[embed+0x140]
  JE::VFX_Wind                          2  0x4E3890[embed+0x188] 0x4E3940[embed+0x188]
  JE::VFX_Lightning                     2  0x4E9D70[embed+0x22C,plain-this] 0x4E9F50[embed+0x22C]
  JE::VFX_ParameterHolder               2  0x4FA540[embed+0x10] 0x4FAAB0[embed+0x10]
  JE_NodePool_GetNode_0x562f90          2  0x560A00[field-load] 0x562C40[field-load]
  JE::VFX_ParticleSystem                2  0x48DDC0[field-load] 0x514000[embed+0x10C]
  JE::VFX_FormatManager                 2  0x4FB510[embed+0x28] 0x4FD150[embed+0x4]
  JE::Octree2Gfx                        2  0x506760[field-load] 0x506B30[*global]
  JE::ResourceCache                     2  0x50B960[embed+0x8] 0x50BA30[embed+0x8]
  JE::WorldMap                          2  0x599F20[field-load] 0x59A340[heap-ctor]
  JE::Gfx_D3D                           1  0x5038F0[embed+0xC]
  JE::OCTREE_Rect2D                     1  0x522100[embed+0xC]
  JE::CMiniMapInterface                 1  0x442710[field-load]
  meth_0x5a7e20_5a7e20                  1  0x5A49D0[&global,plain-this]
  JE::Gfx_ModelMeshPkt                  1  0x4ABB70[embed+0x28]
  JE::Gfx_SkinMeshPkt                   1  0x4ABE60[heap-ctor]
  meth_0x49fae0_49fae0                  1  0x49F4F0[embed+0x34,plus-other]
  JE::G3D_Node                          1  0x4D3D90[embed+0x108]
  JE::CSoundPresetInfo                  1  0x52D410[embed+0x2C]
  JE_SpatialGrid_QueryEntities_0x405f80  1  0x560B20[field-load]
  JE_PathManager_TraceSegmentClear_0x562600  1  0x561A90[field-load]
  F3::GameInterfaces                    1  0x416F80[field-load]
  JE::CCharacterGenerationInterface     1  0x42A6A0[heap-ctor]
  JE::CActionbarInterface               1  0x434B70[field-load,plain-this]
  JE::GfxCameraHolder                   1  0x4950D0[heap-ctor]
  std_num_get_char_class_std_istreambuf_iterator_char_struct_std_char_traits_char_  1  0x4963A0[field-load]
  std::istreambuf_iterator_char_std::char_traits_char__  1  0x4985D0[field-load]
  JE::Gfx_ModelInstance                 1  0x49F150[embed+0x24,heap-ctor]
  JE::Gfx_AnimationObject               1  0x4A4970[heap-ctor]
  JE::GUI::Window                       1  0x4B5C30[&global]
  JE::GUI::Manager                      1  0x4B61D0[field-load]
  JE::StringTree                        1  0x4B6A00[field-load]
  ??__E?wndTop@CWnd@@2V1@B@@YAXXZ       1  0x505170[&global]
  JE::ResourceSystem                    1  0x50CA60[embed+0x12C]
  meth_0x529490_529490                  1  0x526620[heap-ctor]
  ??__E?wndTop@CWnd@@2V1@B@@YAXXZ_0     1  0x52BD80[&global]
  JE::CMasterJumplist                   1  0x5392C0[embed+0x4C]
  JE::CDebugInterface                   1  0x557AD0[embed+0xA0]
  JE::Server                            1  0x55C2D0[field-load]
  JE_OpenHeap_ctor_0x562460             1  0x560DC0[embed+0x10]
  JE_PathSteerer_SmoothPath_0x561fa0    1  0x5612B0[field-load,plain-this]
  F3::GameCSSkillMenuInterface          1  0x568B70[embed+0xD28]
  Peek_60b152                           1  0x60B179[field-load]

Reading notes: `embed+0x…` = member field of the named host (band object at host+disp);
`heap-ctor` = allocator-caller join (owner = allocating fn's class); `&global`/`*global` =
singleton-pin families (4b28a0 shape — identity annotation, membership vacuous); hosts like
`call_cls_0x566490`/`meth_0x459c50`/`JE_PathSteerer_*` are LIVE function labels whose CLASS root
is itself unresolved — weakest tier: writer must read back the owning class before minting
those 9 lines.

## 3. ANNOTATE-FOREIGN queue (per-fn SHARING 82, class-concentrated)

Clusters: `cls_0x40cc80` (CRT/inline small-function epoch — owners scatter across
CounterControl/VFX/GameWeapon, item-43 fan signature), `cls_0x401b00` (Gfx-epoch allocator/
string helpers; reached by 5 atexit pads), `cls_0x4dff90` (Graphics epoch helpers),
`cls_0x4abc60`/`cls_0x515040` (std vector/array cleanup machinery — 26 and 24 pad resolutions
confirm on the cleanup side, matching T-059's 0x401D60 trio-release finding). Queue text:
`long-tail helper epoch per T-084; owner = call-site fan, not membership`.

## 4. FINAL-anonymous-with-proof (171 ISO)

Item-41 triplets answered per ISO fn (Q1 zero incoming incl. transitive; Q2 0 slots/0 direct
thunks; Q3 full-window scan): **142** inside vendor epochs (cls_0x668*/6b8*/6bc* + >=0x5FD000
link tail — b4's vendor ruling extends, zero in-game flow corroborates), **29** non-vendor
dead-branch/linkage residue (printed in the sweep; e.g. the 566490-scripted-ctor island's 4,
CRT initializer fragments). FINAL-anonymous = absence-of-presence PROVEN, not shrug.

## 5. Honest close — the remaining number, decomposed

323 mint-queued + 82 foreign-annotated + 171 proven-final + **801 LIVE/DELEG weak-evidence**
(callers known via field-load/plain-this edges; construction/embed evidence ABSENT — 'used-by',
not 'owned-by', at tool strength). That 801 is the residual workload for the NEXT evidence class;
per-fn detail is regenerable exactly by the BANKED derivor: `tools/ecx-flow.py hosts --cls <cls>
--dump discoveries/wavedumps/t064-postdump.json` (hosts mode, commit after this doc: the
derivation RULE = LIVE/DELEG pool per fn with per-cls band exclusion, host = caller's live-IDB
class via name rsplit('::'), key = (hostClass, terminal-field) — Client+0x118 vs World+0x118
are two rows by design; T-084 §2 queues are this mode's output, and TrueCastle's wave-2b HOLD
list is reproducible from it verbatim).
T-063's 'workload number, not ignorance number' honored literally: ignorance in the long tail is
now 0 un-named-but-knowable fns; 801 know-USED, 29+142 proven ownerless-in-this-image.

Writer-slot throttle note (row asks for it): 323 mints + ~120 annotation classes is wave-sized;
recommend T-073/075 closures land first, then T-084 tier-1 (182 fns across 22 hosts) as the next
single writer load, tier-2 + foreign/ISO annotations batched behind. Queue held cheaply here.

### AMENDMENT 2 (TrueCastle wave-2 pre-flight, ruling 665e7cc — honest provenance note)
The §2 host column was produced by a per-class sweep script that was NOT committed with this
doc (heredoc-only; regeneration from committed modes missed ~65 rows). Root cause accepted: the
queue's proof lived in an unshipped half of its own recipe. FIX LANDED: the derivation is now
BANKED as `ecx-flow.py hosts` mode (host-prefixed terminal-field keys, per-cls band exclusion,
gated like every mode); every §2 row regenerates cold from the command above. No bytes changed,
no verdicts moved — only the recipe got its missing half.

### AMENDMENT 3 (T-088 arbitration, wave-2b gate material)
§2-vs-§3 KEY-COLLISION — named, per item-52 pre-flight: the colliding class is the helper-epoch
pair **cls_0x40cc80 / cls_0x401b00 (+cls_0x515040, cls_0x4dff90 as §3 keys)**; nine fn-level keys
where a §2 MINT row and a §3 foreign-annotation overlap (list + per-row verdicts in
`discoveries/wavedumps/qw2-arbitration-class2.txt`, regenerator embedded per row):
0x515040, 0x5150C0, 0x515580, 0x515D50 (ParticleSystem/Gfx_ParticleSystem), 0x4A5720, 0x4A5A10
(AnimMesh), 0x436410 (GameActionEquip), 0x52CC70 (Gregorian), 0x523450 (Skin). Resolution rule now
part of this doc's contract: **fn-level STRONG key (embed±field / heap-ctor / global-pin) wins the
MINT; the §3 class annotation stands for the epoch's REMAINING members (item-51 rule-2
coexistence)** — 3 STRONG-mint, 4 weak-key (band-confirm-or-annotate at writer discretion), 2 §3-stands.
Class-(1) full disposition (65 derivation-gap rows): 52 MINT-CONFIRMED, 13 RE-HOSTED (each RE-HOST
is doc-host carrying only consumer-shape keys while a sibling host carries the terminal field —
e.g. the six JE::Client rows re-host to JE::World @+0x108/@+0x118, install-site evidence at
World::ctor_0x548a80 sites 0x548beA/0x548bf5 — TrueCastle's +0x118 regression case, resolved),
0 FOREIGN-flip. Table: discoveries/wavedumps/qw2-arbitration-class1.txt.
Provenance note for R-INST scoping: every one of the 65+9 rows regenerates from the COMMITTED
`hosts` mode; the gaps that made wave-2b's regen miss were (a) band-parameter not recorded per row
(now: live-cls band is the convention, hosts header says so), (b) top-4 key truncation hiding
lower-position doc-hosts (now top-8), (c) holder-key preference unspoken (now stated above).
No new edge class is required for these 74 rows — R-INST's spec should shrink to the convention
notes or retire.

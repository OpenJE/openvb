# P2-surface net-entities instantiation — the named gate to un-block the netcode factories

Scope deliverable (the R-USERRE-A successor, coordinator fence "named-gated"): the exact
undefined-virtual sequence that makes `vft[5] factory 0x45dfd0` (GameClient) and `vft[4]
factory 0x45e040` (GameServer) link TODAY instead of emitting ~35 LNK2001s on both toolchains.

## Why this is the gate

The shipped factories are `new(0x118) + GameClient::ctor_0x460b30(net->mbr_0x1c)` /
`new(0x7108) + GameServer::ctor_0x46ee70(net->mbr_0x8)` (R-USERRE slice-7c, named-gated).
Constructing either in port TODAY builds a vft whose entries must all resolve; the port
declares the virtuals (hpp) but defines only the ctors. LNK2001 on BOTH toolchains is the
honest current state; the port-guard path on factory-0 is the named deviation (shipped
factories never return 0).

## Band 1 slot 0 = the deleting-dtor path — CASCADE MAP (bodies read whole this pass)

The "first body" of band 1 is not one body: slot 0 (0x45f2e0) is the scalar deleting
 destructor and its port pulls the whole teardown cascade. Read whole (IDA original,
 2026-10-24 pass):

| depth | EA | body (shipped bytes) |
|-------|-----|----------------------|
| 0 | 0x45f2e0 | deleting dtor: call dtor_0x45e6a0; if (flags&1) free(this); return this; `retn 4` — 11 instructions |
| 1 | 0x45e6a0 | GameClient dtor: vft store 0x664804, tail-jmp JE::Client::meth_0x5897d0 — 2 instructions |
| 2 | 0x5897d0 | JE::Client teardown: SEH guard, vft store 0x69C814, call virt_meth_0x588d90, then net::client::meth_0x59c790 — 24 instructions (SEH frame) |
| 3 | 0x588d90 | the MEMBER-RELEASE CHAIN: test-then-release ~10+ member objects via their own deleting-dtors (slots +0xF8, +0xB8, +0xB0, +0xAC, +0x9C, +0xA0, +0xA4, +0xA8, ... — 60-instruction head read, cursor continuing) |
| 3' | 0x59c790 | net::client teardown: SEH guard, vft store 0x6A1934, call net::client::virt_meth_0x59c360, address-vft store 0x6647A0 at +0x30, call net_base::meth_0x59d530 — 28 instructions |

| 3 | 0x588d90 | the MEMBER-RELEASE CHAIN, COMPLETE (112-instr body read): releases +0x9C, +0xA0, +0xA4, +0xA8, +0xB4 via deleting-dtors; **CameraControl at +0x84** = `call CameraControl::dtor_0x586570` then `free`; slot releases +0x88, +0x8C, +0x94, +0x98, +0x80, +0x7C, +0x78, +0x74, +0x70; then **tail-jmp net::client::virt_meth_0x59c360** — note the bytes run 59c360 TWICE on this path (once via this tail-jmp, once from 0x59c790's own call) — record as-shipped |
| 3' | 0x59c790 | net::client teardown: SEH guard, vft store 0x6A1934, call net::client::virt_meth_0x59c360, address-vft store 0x6647A0 at +0x30, call net_base::meth_0x59d530 — 28 instructions |
| 4 | 0x59c360 | 13 instructions: vft[+0x34] call, release the [+0x2C] member, done |
| 4' | 0x59d530 | net_base teardown, COMPLETE (58 instr): SEH, vft store 0x6A1D1C, release [+0x1C] (deleting-dtor or direct free via [ecx-4]); **net_socket::close_socket 0x5B8D40 called TWICE** (on socket +0x14 and the socket object after vft store 0x6A1D14); debug-stream-vft store 0x6A1D0C at +8; call net_critical_section::dtor_0x5B92A0; SEH cleanup paths (63a840-63a867) name the same helpers |
| BOTTOM | 0x5B8D40 close_socket + 0x5B92A0 critical_section dtor + free/delete | the cascade bottoms here; **0x5B8D40 and 0x5B92A0 are PORTED** (net/socket.cpp + net/critical_section.cpp, submodule 862592b, gates green) |

## CameraControl-leaf subtree (the +0x84 release) — closure COMPLETE, port pending

The +0x84 release forces the whole subtree into the linkable closure (each call odr-uses
the callee). Read whole 2026-10-24:

| EA | body |
|----|------|
| 0x586570 | CameraControl dtor (4 instr): vft store 0x69BF28, [this+8] = Gfx::CamCtrl::Orbit vft 0x670FB8, tail-jmp 0x529FB0 |
| 0x529FB0 (== IDB dtor_0x4c68a0 alias) | Gfx_CamCtrl_Orbit dtor (22 instr): vft store 0x670FA8, call cls_0x50dbb0::meth_0x4c6680 (list params = full span), free block, null fields |
| 0x4c6680 | whole-span fast path at the dtor call (both params == head → 79 instr body, fast path = meth_0x4c6600 + list reset); **the search path calls 0x425e90** |
| 0x4c6600 | PORTED (recursive chain-free, 25 instr, submodule df7a861) |
| 0x425e90 | **std-tree erase-and-rebalance with the "invalid map/set<T> iterator" out_of_range GUARD** (~160+ instr read total, cursor continuing at the rebalance tail): SEH frame, flag test → throw std::out_of_range via std_logic_error ctor 0x401ca0 + _CxxThrowException; then the unlink + RED/BLACK rebalance (the +0x14 color byte exchanges @0x426009-0x426012, rotate via cls_0x50dbb0::meth_0x50ce00, min/max helpers sub_4B2B50/sub_4B2B30) — the port of this body pulls the EXCEPTION machinery + the rotate family into the closure — a named slice of its own |
| 0x48d750 | PORTED (tree-decrement walk, 38 instr, submodule: the +0x15 nil-marker byte, leftmost chain walk + climb-while-right-child; 'this' = the OUT-SLOT pointer per the shipped caller) — was: 0x48d750 | SEH frame, flag test → throw std::out_of_range via std_logic_error ctor 0x401ca0 + _CxxThrowException, then the unlink algorithm calling cls_0x48d750::meth_0x48d750 — the port of this body pulls the EXCEPTION machinery into the closure (std exception ctors, throw support) — a named slice of its own |

The forced chain for factory-1 slot 0 is therefore:
0x45f2e0 → 0x45e6a0 → 0x5897d0 → 0x588d90 → {13 releases incl 0x586570 → 0x529FB0 →
0x4c6680 → 0x425e90 → 0x48d750 + std-exception path} and → 0x59c790 → 0x59c360, 0x59d530.
Porting order: bottom-up — 0x425e90's remaining tail + 0x48d750 + the exception ctor chain
next, then the leaf dtors, then the 7-body chain in one atomic multi-TU slice. the factory-construction strike gate
 implicitly requires this whole path defined, because any TU odr-using the vft emits slot 0,
 whose body's call chain must link. Band-1 slice 1 = port the depth-0..3' bodies plus the
 children they name until the cascade bottoms out in free/delete + already-ported helpers.

## The sequence (EA-named, verified this scope pass)

### Band 1 — F3::GameClient vft 0x664804 (56 slots), 15 overrides in the GameClient band

| slot | EA |
|------|-----|
| 0  | 0x45f2e0 (deldtor pair anchor) |
| 1  | 0x4618f0 |
| 9  | 0x45f000 |
| 28 | 0x461a10 |
| 30 | 0x45e7f0 |
| 31 | 0x45e820 |
| 33 | 0x45e830 |
| 37 | 0x45e880 |
| 43 | 0x4613d0 |
| 44 | 0x45e8e0 |
| 45 | 0x45e940 |
| 47 | 0x461470 |
| 48 | 0x461530 |
| 50 | 0x461930 |
| 51 | 0x45e9a0 |

(remaining 41 slots inherit JE::Client's entries — see band 2)

### Band 2 — JE::Client virtuals (modules/openje/src/JE/Client.hpp, declared lines 63-112)

33 declared virtuals: 0x587770, 0x587b00, 0x587b10, 0x587b70, 0x587bd0, 0x587c30, 0x587c90,
0x587d00, 0x587d60, 0x587dc0, 0x587e20, 0x587e80, 0x587ee0, 0x5880a0, 0x5880c0, 0x588170,
0x588190, 0x588220, 0x588d90, 0x588f50, 0x588fe0, 0x58a4a0, 0x58a620 (the CLIENT
movie-play packet handler — C4-anchored), 0x58a960, 0x58ab80, 0x58bd80, 0x58be40, 0x58bf00,
0x58bfc0, 0x58c080, 0x58c140, 0x58c200, 0x58c380, 0x58c660, 0x58d9a0 — plus the INSTALL HUB
**0x58f010** (conn vft[1], ALREADY read whole in slice-7c: constructs every pump-mapped inner
object through client vft slots +132..+196, the four CLoadMapInterface/CDebugInterface windows
with geometries 420x110/300x420/1024x768, the +0x78/+0x7C/+0x80 event-queue/FIFO/GameHotKeys
trio, the veg selector store, the LOCALHOST self-connect) — porting = comments-to-code, the
body read is banked.

### Band 3 — F3::GameServer (src/F3/GameServer.hpp, same pattern; vft enumeration pending)

GameServer's declared virtuals follow the same law; enumerate its vft (factory 0x45e040 →
ctor_0x46ee70, 0x7108 bytes) with the same f3_vftable_index pass before band 3 lands.

## Gate plan (per slice)

1. Read the shipped body of the next virtual in the sequence (IDA disasm, original).
2. Port it comments-to-code (undefined4 return semantics preserved; the wrapper-id law holds).
3. Suite section 14 (Commands.cpp, 32 asserts) extends to construct factory-1 (GameClient) live
   once its band is complete; factory-2 (GameServer) after band 3 — boot-safety preserved.
4. Gates: msvc rc0 (F3.exe), run-all green [>=16 asserts], annot checker 0 uncited, cppcheck on
   touched files.
5. Strike the row ONLY when a factory constructs live in the suite (the gate is behavioral, not
   declaration-complete).

## Honesty rails

- Bodies must come from the shipped bytes, not from shape guesses (the register's law: shape
  words die on body bytes — three erratum instances on record).
- `undefined4` stays until a slot's type is pinned by its consumers.
- The 0x118/0x7108 allocation sizes are the shipped operator-new literals — keep them verbatim.

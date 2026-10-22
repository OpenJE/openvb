// Copyright 2024 OpenJE

#ifndef F3_SCRIPTNATIVES_HPP_
#define F3_SCRIPTNATIVES_HPP_

#include "JE/Entity.hpp"

namespace F3 {

	// ---------------------------------------------------------------------------
	// R-USERRE batch B: the script-native preamble trio + the GameCreature getter
	// family. All shipped natives share ONE body shape:
	//     arity gate -> entity resolve -> RTTI cast -> read a field
	// so the three helpers below are the batch's real prerequisite (every one of the
	// 39 batch-B rows calls at least two of them).
	//
	// Calling convention VERIFIED from the bytes, not assumed: the natives are
	// __cdecl (int amx, int params) - the bodies push args and `add esp,8` after the
	// calls, and the getter tails end in bare `retn` with the result in eax. `amx` is
	// the opaque AMX handle the engine's error reporter indexes (+0x2C = the running
	// script line); `params` is the AMX parameter block, where params[0] is the block
	// size IN BYTES (the arity gate divides by 4 to get cells - 0x59af4c).

	// 0x59af40 - arity gate. Returns 1 when the call carries `nCellsExpected` cells,
	// otherwise reports TWO shipped strings in order and returns 0.
	// DEViation NAMED: the two messages are byte-verbatim (including the shipped
	// misspellings "paramters" and "recieved" - T-022 applies), but the port's report
	// channel is ScriptNativeReportError below, not the shipped sink virtual.
	char ScriptNativeCheckArity( int amx, const unsigned *params, int nCellsExpected, const char *pcFnName );

	// 0x59aef0 - entity resolve. Looks `hEntity` up through the global entity list
	// (shipped byte_70BE0C walk at 0x54be10) and stores the result at *ppOut.
	// Returns 0 ONLY when nothing resolved. The `nCheckActive` flag is a BYTE arg
	// (0x59af0a `mov cl,[esp+arg_C]`): when set and *(entity+0x3A0) == 0 the shipped
	// code REPORTS "Attempt to use inactive entity in script." AND STILL RETURNS 1
	// (0x59af2e `mov al,1`) - it warns, it does not block. Ported exactly, because the
	// obvious reading of that guard is wrong.
	char ScriptNativeResolveEntity( int amx, unsigned hEntity, JE::Entity **ppEntity, char nCheckActive );

	// 0x59ad20 - the script error reporter. Shipped shape: resolve the running script
	// name and its file (two sub_5CCCC0 lookups, tags 1179407181 / 1279415366),
	// _splitpath the file, then sprintf("!Run-time error in script %s, file %s%s, line
	// %d: ") + vsprintf into a 1028-byte buffer and hand it to
	// (*(**byte_70C65C + 16))(*byte_70C65C, buffer). NOT FatalError: the natives report
	// and CONTINUE, which is why every getter failure path returns 0 instead of dying.
	// PORT DEVIATION (named, not faked): the two sub_5CCCC0 lookups have no port-side
	// AMX module table yet, so the reporter uses the shipped INITIAL values of those
	// same variables - "<Unknown>" for the script name and "<unknown>"/empty for the
	// file - and routes the composed message through JE::LogDebugString. The line field
	// IS read from the amx handle (+0x2C, 0x59ae38 era) when an amx is supplied.
	void ScriptNativeReportError( int amx, const char *pcFormat, ... );

	// Read the messages ScriptNativeReportError has formatted since the last reset (newline-separated;
	// the arity gate emits two in order, so a last-only capture loses the first). TEST SEAM only: the shipped sink is
	// JE::LogDebugString, which a test linking JE.cpp cannot override, so byte-exact assertions on
	// shipped sentences (the B13 sibling-space quirk) go through this copy instead.
	const char *ScriptNativeGetLastError();
	void ScriptNativeResetLastError();   // clears the capture; call before a leg you want to inspect

	// ---- the getter family (batch B slice B2) ------------------------------------
	// Field offsets are BYTE offsets verified against the instruction text. Do NOT
	// trust the decompiler's printed offsets here: it rendered these five as
	// `*(v3 + 47/50/52/53/54)` on a pointer it labels `char *`, which are CELL indices
	// (0xBC/4, 0xC8/4, 0xD0/4, 0xD4/4, 0xD8/4). The bytes say dwords:
	//   0x45ac5f `mov eax,[eax+0BCh]`   GetLevel
	//   0x45af2f `mov eax,[eax+0C8h]`   GetRace
	//   0x45afbf `mov eax,[eax+0D0h]`   GetGender
	//   0x45b057 `mov eax,[eax+0D4h]`   GetEthnicity
	//   0x45b0f7 `mov eax,[eax+0D8h]`   GetSubRace
	// All five: 1 expected cell, resolve at params[1], cast Entity->GameCreature, and
	// `xor eax,eax` (return 0) on EVERY failure path - no error code distinguishes them.
	int ScriptNative_GetLevel( int amx, int params );      // 0x45abe0
	int ScriptNative_GetRace( int amx, int params );       // 0x45aeb0
	int ScriptNative_GetGender( int amx, int params );     // 0x45af40
	int ScriptNative_GetEthnicity( int amx, int params );  // 0x45afd0
	int ScriptNative_GetSubRace( int amx, int params );    // 0x45b070

	// ---- the HP pair (batch B slice B3) ----------------------------------------
	// Same preamble, but the cast target is GameEntity and the two reads differ in KIND:
	//   GetCurrentHP 0x45b237 `mov eax,[eax+24h]`            - a plain field
	//   GetMaxHP     0x45b197 `mov edx,[eax]; mov ecx,eax; call dword[edx]`
	//              - a VIRTUAL call through vftable slot 0, whose target
	//                F3::GameEntity::virt_meth_0x4080c0 is itself `mov eax,[ecx+20h]; retn`.
	int ScriptNative_GetCurrentHP( int amx, int params );  // 0x45b1b0
	int ScriptNative_GetMaxHP( int amx, int params );      // 0x45b110

	// ---- the type/DC family (batch B slice B6) -----------------------------------
	// Same preamble, three DIFFERENT accepted classes, and the offsets are byte-verified
	// (the pseudocode printed cell indices a third time: "40" == [eax+0A0h], "48" == +0xC0):
	//   GetItemType           0x45b2c7 `mov eax,[eax+0A0h]`  cast Entity->GameItem
	//   GetUsableObjectType   0x45b4c7 `mov eax,[eax+0A0h]`  cast Entity->GameUsableObject
	//   GetLockDC             0x45b5dd / 0x45b5e8 `[+0C0h]`  cast CONTAINER, else DOOR
	// +0xA0 is shared by two unrelated classes, i.e. a base-level type code, and +0xC0 lives on
	// their common base GameUsableObject - which is why one field serves both GetLockDC branches.
	// ---- the weapon-combo pair (batch B slice B23) - two shipped stubs that resolve and do nothing ----
	// 0x45a9f0 GetWeaponCombo / 0x45aa40 SetWeaponCombo. Byte-verified from 0x45a9f0 and 0x45aa40 through
	// their shared callee 0x45b9f0 (`sub_59AEF0(amx, params[1], &out, 1)` at 0x45ba02 with the ACTIVE-CHECK
	// flag ON - note ON, not distinctive: 104 of the image's 105 direct callers of that wrapper pass 1, and
	// the lone 0 is JE::ActivateEntity, which exists to find inactive entities. docs/f3/WeaponComboStubs.md
	// §1 has the census and the correction this replaced),
	// flag ON, then `RTTI Entity -> GameEntity` at 0x45ba23, store at 0x45ba31, return cast != 0):
	//   gate -> resolve+cast -> if it FAILED, one report line -> `xor eax,eax` -> return 0.
	// Both return 0 on EVERY path (0x45aa34 / 0x45aa85) and NEITHER reads what it resolved, so:
	//   - GetWeaponCombo's "combo" has been a constant 0 for the whole life of the build;
	//   - SetWeaponCombo is arity 2 and its SECOND CELL IS NEVER READ - `mov ecx,[edi+4]` (0x45aa60) loads
	//     params[1] only, and nothing loads [edi+8]. The setter sets nothing.
	// Ported as exactly that, and the "trivial" version is the point: a port that stored the combo, or
	// returned the resolved entity, would delete the finding. Absence argument: the claim is "no instruction
	// reads params[2]", established by reading the whole 28-instruction body (0x45aa40-0x45aa89) - not by
	// the DB listing a field, since a listing is an excerpt (rule 7).
	// The one observable is the message pair: "GetWeaponCombo() was unable to find the requested entity."
	// (0x45aa2c, string @0x65e1a4) and "SetWeaponCombo() was unable to find the requested entity."
	// (0x45aa7c, @0x65e1f0) - each naming itself, and both firing ONLY when the gate passed and the
	// resolve/cast failed (0x45aa0e/0x45aa5c jump past the whole block on a gate failure).
	int ScriptNative_GetWeaponCombo( int amx, int params );   // 0x45a9f0
	int ScriptNative_SetWeaponCombo( int amx, int params );   // 0x45aa40

	// ---- the task-journal trio (batch B slice B22) -----------------------------------------
	// 0x45b900 OpenTask / 0x45b940 CloseTask / 0x45b980 AddEntry. The ONLY batch-B family that never
	// resolves an entity: gate on 2 cells, read the two cells, hand them to the server, return 1.
	// Byte-verified shape, identical in all three bodies except the gate name and the delegate EA:
	//   45b900 mov eax,[esp+param_1] / 45b905 mov esi,[esp+4+param_2]   - amx into eax, params into esi
	//   45b909-12 push name,2,params,amx; call sub_59AF40; add esp,10h  - the arity gate with its OWN name
	//   45b91a test al,al / jnz / 45b91e xor eax,eax / retn             - gate fail => 0
	//   45b922 mov ecx,[esi+8] / 45b925 mov edx,[esi+4]                - params[2], params[1]
	//   45b928 mov eax,F3::global_area_map_icons / 45b92e mov ecx,[eax+18h]  - NO null test; see below
	//   45b92d/31 push ecx,edx / 45b932 call GameServer::meth_0x46ecd0  - cdecl arg order is (params[1],params[2])
	//   45b937 mov eax,1 / retn                                         - UNCONDITIONAL success
	// The fetch at +0x18 is the second sighting in this batch of the misnamed `global_area_map_icons`
	// subsystem manager (JadeBear's name-fix candidate: it is a multi-singleton block, not map icons);
	// `this` for the delegate is what lives there, and the delegate's tail uses it for the packet send.
	//
	// TWO SHIPPED LANDMINES, both kept as behavior rather than "fixed":
	// (1) the return value is INDEPENDENT of the callee. The journal write and the packet are inside
	//     `if (cast succeeded)` (0x46ed08 jz -> 0x46ed28, past both calls), so a script with no journal
	//     object still gets 1. Do not "improve" this into a success-dependent return.
	// (2) shipped CRASHES on the null states - `mov ecx,[eax+18h]` with a null manager faults at
	//     0x45b92e, and inside the delegate the null-singleton branch faults one instruction later
	//     (`mov eax,[ecx]` with ecx = 0 at 0x46ecea / 0x46ed4a / 0x46edaa). CORRECTION to the earlier
	//     note in docs/f3/CreatureHpWriters.md (B8), which said the null singleton "did nothing": it
	//     does not return at all. The port therefore does not reach the shipped state where the answer
	//     is observable, and it does not reproduce a fault: with the era absent the port answers 1, the
	//     same value the non-crashing path (failed RTTI cast) produces. Declared, not hidden.
	//
	// The era itself is NAMED-DEFERRED as a whole chain, not leg by leg: the journal object is reachable
	// only through the world-server singleton 0x70bd98 + the vbptr base+delta walk + vft slot 30 +
	// `RTTI Journal -> GameJournal`, and each tail is a net::packet send - ids 97 (OpenTask, 0x46dad0),
	// 98 (CloseTask, 0x46dbb0), 99 (AddEntry, 0x46dc90), each `packet(a1); packet(a2); server->send(pkt,1)`.
	// Those are SEND-side ids: they are absent from the RECV dispatch id-map in
	// discoveries/2025-10-22-server-packet-dispatch-idmap.md, which is a different number space.
	// The journal bodies are read and recorded (sub_46C7A0 upsert with state 1, sub_46C840 the same walk
	// with state 2, sub_46C6C0 an add-if-absent over both key fields), but neither GameJournal nor
	// GameServer nor the packet era exists port-side, so nothing here fakes a journal or logs a substitute.
	int ScriptNative_OpenTask( int amx, int params );    // 0x45b900
	int ScriptNative_CloseTask( int amx, int params );   // 0x45b940
	int ScriptNative_AddEntry( int amx, int params );    // 0x45b980

	// 0x45b9c0 - PlayMusic: the engine's own admission of a hole in the native table. Arity 1, and the
	// ONLY effect when the gate passes is reporting the shipped literal "PlayMusic() unimplemented."
	// (0x45b9e4) and returning 0. No callee, no state, no side effect - so the faithful port is exact,
	// and the wording is verbatim because it is evidence, not style (docs/f3/CreatureHpWriters.md B8).
	int ScriptNative_PlayMusic( int amx, int params );           // 0x45b9c0

	// ---- the HP writers (batch B slice B9) - unblocked by the +0x1E0 resolution --------------
	// SetHP/AddHP do NOT write the entity the script named. Shipped: read the entity's OWN handle
	// back out of the virtual-base slot (0x45b367-6d `mov edx,[eax+4]; mov edx,[edx+4];
	// mov eax,[edx+eax+1E0h]` = Entity-base+0x1DC, T-043's handle field), then hand that handle to
	// 0x46ee10, which re-resolves it, casts, and clamps. JadeBear proved the re-resolve is the identity
	// round-trip (Create 0x55190c stamps +0x1DC once; the only other writer is UnpackState restoring the
	// same pair), so the two-step is faithful but NOT decorative: the shipped form FAILS SILENTLY (no
	// error string, return 0) when the handle is unregistered, and that failure is the port's job too.
	// Clamp at 0x4629c0: upper cap at max (+0x20), NO lower bound. The packet-88 send at 0x46d620 is
	// NAMED-deferred with the rest of the server era - so the return value here is shipped-faithful
	// (1 on success, 0 on either silent failure) while the broadcast is not yet emitted.
	int ScriptNative_SetHP( int amx, int params );    // 0x45b2e0
	int ScriptNative_AddHP( int amx, int params );    // 0x45b390

	// ---- the door-lock trio (batch B slice B10) ---------------------------------
	// ONE byte at GameDoor+0xBD is the whole lock state, and the three bodies differ only in what they
	// do with it: IsLocked reads it with `movzx` (0x45b55f - a 0..255 return, NOT sign-extended),
	// LockDoor stores 1 (0x45a230), UnlockDoor stores 0 (0x45a2c0). All three accept ONLY GameDoor,
	// which is a real asymmetry against GetLockDC (0x45b570), whose DC field is read for CONTAINER as
	// well as door: a container can have a lock DC but cannot be locked or unlocked by script.
	int ScriptNative_IsLocked( int amx, int params );    // 0x45b4e0
	int ScriptNative_LockDoor( int amx, int params );    // 0x45a1b0
	int ScriptNative_UnlockDoor( int amx, int params );  // 0x45a240

	// 0x45a580 - EquipItem: arity 3 (entity, name, slot). Finder is Inventory vft slot 6 (0x55fda0),
	// which matches names with _stricmp; the payload is GameInventory::Equip, vft slot 14, reached at
	// 0x45a6ce by `call dword ptr [edx+38h]` (bytes ff 52 38). Ported here with the OBSERVABLE parts
	// of Equip and its era declared rather than hidden - see the body for the three named deviations.
	int ScriptNative_EquipItem( int amx, int params );

	// ---- the party seam + its two natives (batch B slice B21) -------------------------------
	// Shipped reaches the party through the second world-server singleton (global 0x70bd98) with the
	// base-plus-delta idiom, then vft slot 26 (`call [edx+68h]` at 0x45a9a5 / 0x45a343), then an RTTI
	// Party -> GameParty. The port has no singleton and no GameParty, so the active party is set
	// directly. One deviation is unavoidable and declared in the body: when there is NO party, shipped
	// dereferences the null adjustment and CRASHES at both call sites - the message that looks like it
	// covers "no party" actually belongs to the failed cast - so the port answers that message with 0
	// instead of faulting, and says which shipped state it is collapsing.
	void ScriptNativeSetActiveParty( class ScriptParty *pParty );
	class ScriptParty *ScriptNativeGetActiveParty();

	int ScriptNative_Protagonist( int amx, int params );      // 0x45a960
	int ScriptNative_AddCNPCToParty( int amx, int params );   // 0x45a2d0

	// 0x45b620 - GetWeaponRange: arity 1, and the deepest object chain in batch B so far. It mixes
	// bases on purpose - the inventory comes from the PRE-CAST entity at +0x168 (same accessor as
	// IsItemEquipped, three natives agree) while the fallback range pointer comes from the POST-CAST
	// creature at +0x6F4 - and it ends in a two-hop read: weapon+0x408 is a POINTER, dereferenced at
	// +0x2C for the value. A SHIPPED LANDMINE is preserved as a named port deviation: when the item at
	// the slot is not a GameWeapon, 0x45b727 dereferences the failed RTTI result with no null test, so
	// shipped crashes and the port returns 0. Reproducing a crash is not a behavior worth copying, but
	// pretending the two are equivalent would be a lie - hence the explicit case below.
	int ScriptNative_GetWeaponRange( int amx, int params );

	int ScriptNative_GetItemType( int amx, int params );           // 0x45b240
	// The shipped Skill enum, values read off GetTotalSkill's jump table (cmp edx,0Ch at 0x40983e =
	// cases 0..12). The PAIRS whose bodies are identical (0/1, 3/4, 6/7, 10/11) carry the IDB's names;
	// which member of a pair is which is not distinguishable from the code and does not matter, since
	// they share one formula - recorded as an enum so the switch's grouping stays visible.
	enum Skill {
		SNC_SKILL_FIREARMS = 0,
		SNC_SKILL_MELEE = 1,
		SNC_SKILL_UNARMED = 2,
		SNC_SKILL_BARTER = 3,
		SNC_SKILL_PERSUASION = 4,
		SNC_SKILL_DECEPTION = 5,
		SNC_SKILL_MECHANICS = 6,
		SNC_SKILL_MEDIC = 7,
		SNC_SKILL_OUTDOORSMAN = 8,
		SNC_SKILL_SCIENCE = 9,
		SNC_SKILL_SNEAK = 10,
		SNC_SKILL_SECURITY = 11,
		SNC_SKILL_STEAL = 12
	};

	// The shipped DerivedAttribute enum, value order taken from the switch's jump table
	// (cmp eax,0Eh at 0x409a67 = 15 cases, 0..0xE). Names are the IDB's. Kept as an enum because the
	// native's parameter IS one of these and because the resistance trio (9/10/11) shares one body -
	// a fact a plain int switch would hide.
	enum DerivedAttribute {
		SNC_DA_ACTION_POINTS = 0,
		SNC_DA_CARRY_WEIGHT = 1,
		SNC_DA_COMBAT_SEQUENCE = 2,
		SNC_DA_CRITICAL_CHANCE = 3,
		SNC_DA_EVASION = 4,
		SNC_DA_HEALING_RATE = 5,
		SNC_DA_MELEE_DAMAGE = 6,
		SNC_DA_PERCEPTION_RANGE = 7,
		SNC_DA_PERK_RATE = 8,
		SNC_DA_RADIATION_RESISTANCE = 9,
		SNC_DA_FLASH_RESISTANCE = 10,
		SNC_DA_BIOLOGICAL_RESISTANCE = 11,
		SNC_DA_SKILL_RATE = 12,
		SNC_DA_THROWING_RANGE = 13,
		SNC_DA_HIT_POINTS = 14
	};

	// ---- attribute readers (batch B slice B13) -------------------------------------------------
	// GetPrimaryAttribute (0x45ac70) is the array shape: cast, then `mov ecx,[esi+8]` /
	// `mov eax,[eax+ecx*4+760h]` (0x45acef/0x45acf2) - a contiguous int[] at GameCreature+0x760
	// indexed by params[2] with NO bounds test anywhere (asserted). GetDerivedAttribute (0x45ad00)
	// is the same preamble but delegates to GameCreature::GetDerivedAttribute 0x409a60, a 15-case
	// (cmp eax,0Eh) switch over the primary band. Both live in this file; the switch is ported as a
	// static helper so its formulas have one home and the native keeps shipped order.
	// ---- the array trio (batch B slice B14) - the two int[] bands + the skill rule --------------
	// GetPerkRank (0x45ab50), GetBaseSkill (0x45ad90) and GetTotalSkill (0x45ae20) all share the
	// preamble and end in an INDEXED read. Two things about them are shipped facts a port would
	// "fix" if left to taste: (1) GetPerkRank's resolve-failure line is a COPY-PASTE from another
	// native - it prints "GetPosition() was unable to find the requested entity." while named
	// GetPerkRank (0x45ab8e) - and (2) the cast-failure sentences split 2-2 on the space:
	// GetPerkRank/GetPrimaryAttribute say "Entity ID", GetBaseSkill/GetTotalSkill/GetDerivedAttribute
	// say "EntityID". Both preserved, both asserted.
	int ScriptNative_GetPerkRank( int amx, int params );       // 0x45ab50
	int ScriptNative_GetBaseSkill( int amx, int params );      // 0x45ad90
	int ScriptNative_GetTotalSkill( int amx, int params );     // 0x45ae20

	int ScriptNative_GetPrimaryAttribute( int amx, int params );    // 0x45ac70
	int ScriptNative_GetDerivedAttribute( int amx, int params );    // 0x45ad00

	// ---- the AMX string-fetch seam (batch B slice B16) --------------------------------------
	// Shipped reads a script string through sub_59AB20 = amx_GetAddr (0x5CE350: cell bounds, ERROR 5,
	// then base + (*amx + 16)) + amx_StrLen (0x5CE3F0: the pawn PACKED-vs-plain predicate
	// `*first_cell <= 0xFF`, two byte orderings) + a STRICT `len < bufSize` + amx_GetString (0x5CE510,
	// which returns 0 on EVERY path). The port has no AMX data segment - JE::Scripting.cpp declares
	// itself a stand-in registry, not a VM - so this seam takes a plain C string and preserves only
	// the OBSERVABLES: unknown cell fails, a string of exactly bufSize fails (the comparison is
	// strict, so a name that exactly fills the caller's buffer is REJECTED, not truncated), and the
	// destination is left UNTOUCHED on failure. There is deliberately no "decode error" outcome,
	// because shipped cannot produce one - modelling it would be inventing behavior.
	void ScriptNativeRegisterStringParam( unsigned nCellOffset, const char *pcValue );
	char ScriptNativeFetchString( int amx, unsigned nCellOffset, char *pcOut, unsigned nBufSize );

	// 0x45b750 - IsItemEquipped: the first native to consume the seam. Shipped check ORDER is
	// observable and kept: resolve (0x45b79e), THEN string fetch (0x45b7cc), THEN the creature cast
	// (0x45b7fe) - so a bad name on a non-creature reports the NAME failure, not the class one.
	int ScriptNative_IsItemEquipped( int amx, int params );

	// 0x45a6f0 - UnequipItem: arity 3 (entity, name, slot), and THREE shipped copy-pastes preserved
	// verbatim: its arity gate is registered under the name "EquipItem" (0x45a72c passes that ArgList,
	// so a wrong-arity call on UnequipItem complains about EquipItem), and both item natives report
	// "... was unable to find the attacking entity." - Attack's wording, in an inventory native.
	// Its slot path splits from IsItemEquipped on case sensitivity, and its "*" wildcard clears the
	// slot while RETURNING 0; both are in ScriptInventory.hpp with the EAs.
	//
	// NOT ported, with the reason named rather than parked silently: EquipItem (0x45a580). Its payload
	// is GameInventory::Equip (0x46be40, vft slot 14 at 0x45a6ce), which is an era, not a store - RB-tree
	// bookkeeping, a style-slot remap over cases 2/3/7, Get_mbr_0x98 weapon-style reads, an
	// ApplyEquipEffects call gated on the server-context bit (0x46c1a1), a SetWeaponStance(0) reset, and
	// its own "already equipped trying to equip itself again" log. Porting Unequip without Equip is fine
	// because the two share only the preamble; porting Equip means modelling effects and stance.
	int ScriptNative_UnequipItem( int amx, int params );

	int ScriptNative_GetUsableObjectType( int amx, int params );   // 0x45b440
	int ScriptNative_GetLockDC( int amx, int params );             // 0x45b570

	// ---- the shipped {name, fn} native table shape -------------------------------
	// Shipped registers natives as arrays of {name, addr} consumed by the amx_Register era
	// (0x5CCDE0, per-entry 0x5CCD60, miss err 19 / `mov ebp,0x13` @0x5CCE2E - see the
	// scripting-abi-native-tables note in modules/openje/src/JE/Scripting.cpp). The port has NO
	// amx module table yet, so this exposes the table itself and does NOT fake a registration
	// call: wiring it into a VM that does not exist yet would be inventing engine behavior.
	// NAMED-ABSENT: the shipped per-module registration SITE (which array registers into which
	// script context, and when) is unported; so is the GameCreature RTTI cast these five bodies
	// depend on (the port links without RTTI, hence the registry's creature flag).
	struct ScriptNativeEntry {
		const char *pcName;
		int (*fn)( int amx, int params );
		int nExpectedCells;   // the param_3 the body passes to the arity gate; -1 = THE BODY HAS NO GATE
	};

	// ---- the quest/journal alias quad (batch B slice B24) ------------------------------------
	// Shipped's F3 game-lib native table at 0x6F3750 (a {name-ptr, fn-ptr} array of 45 entries, read from
	// the PE image itself rather than from any database listing) gives FOUR names - OpenQuest, CloseQuest,
	// AddQuestEntry, AddJournalEntry (table idx 41-44) - whose fn slot holds the SAME dword 0x420990 in all
	// four. That body is `b8 01 00 00 00 c3` = `mov eax,1; ret`: no gate, no params read, no ecx, no branch.
	// So quest/journal scripting is hardcoded TRUE in the shipped demo, and these four are ALIASES of one
	// function rather than four functions that agree. Ported as ONE body registered under FOUR names:
	// collapsing them into one row would misrepresent the registration (shipped resolves natives BY NAME at
	// the amx_Register era, 0x5ccd60 per-entry), and splitting them would invent four things that are one.
	// nExpectedCells is -1, the "no gate" sentinel, and NOT 0: 0 belongs to Protagonist, which really does
	// call the gate with param_3 = 0. Conflating the two would let a future installer register a gate that
	// shipped never had.
	// IDB NAMING WARNING: 0x420990 is ALSO GUI::Window vftable slot 74 (@0x66e4d4), and that is where the
	// DB's `JE::GUI::Window::virt_meth_0x420990` label comes from - the body knows nothing about windows.
	// Anchor these rows on the EA and the five bytes, not on the label.
	int ScriptNative_QuestJournalAlias( int amx, int params );   // 0x420990, reached via table idx 41-44

	// The getters begin in shipped EA order (0x45abe0, 0x45aeb0, 0x45af40, 0x45afd0, 0x45b070); later
	// slices append in the order they were reversed, each row carrying its own shipped EA comment.
	// THIRTY-SIX entries today: the five B1 getters, the HP pair, the type family, PlayMusic, the HP
	// writers, the door trio, the attribute/array families, the item family, Protagonist + AddCNPCToParty,
	// the task trio (B22), the weapon-combo pair (B23) and the quest/journal alias quad (B24). The HP pair
	// cast to GameEntity, NOT GameCreature - the table does not encode the class, the bodies do (the shipped
	// RTTI site differs; see each body).
const ScriptNativeEntry *ScriptNativeGameCreatureTable( int *pnCount );

	// ---- port-side entity registry (the byte_70BE0C stand-in) ---------------------
	// Shipped resolves handles against the engine's global entity list (doubly-linked
	// sentinel; node+8 = entity, entity+0x1DC = the reference handle per T-043) with a
	// fallback map at byte_70BE14. That list has no port-side container yet, so these
	// two functions publish/remove entries in a small static table and
	// ScriptNativeResolveEntity walks IT with the shipped comparison rule. DEViation,
	// named: the port table records creature-ness at insert time because the port links
	// with no RTTI, so the shipped __RTDynamicCast(Entity->GameCreature) cannot run;
	// nIsCreature is what that cast would have decided. MapEntitySpawn may adopt this
	// registry when the real list lands - nothing here invents engine behavior.
	// Which shipped concrete classes the handle satisfies. NOT a ladder: the families are SIBLINGS
	// (GameCreature, GameItem, GameUsableObject, GameContainer, GameDoor all derive from
	// Entity/GameEntity), and GetLockDC accepts TWO of them (GameContainer 0x45b5d3, else GameDoor
	// 0x45b5fc) - so this is a bitmask of every class RTTI would have accepted, and registration lists
	// the whole chain the object actually satisfies (a container is registered CONTAINER|USABLE|ENTITY).
	// DEVIATION, named: the port links without RTTI, so the accepted-set is recorded at insert instead
	// of being decided by the TypeDescriptor pair at each cast site.
	enum ScriptNativeClass {
		SNC_NONE         = 0,
		SNC_GAME_ENTITY  = 1 << 0,
		SNC_GAME_CREATURE= 1 << 1,
		SNC_GAME_ITEM    = 1 << 2,
		SNC_GAME_USABLE  = 1 << 3,
		SNC_GAME_CONTAINER = 1 << 4,
		SNC_GAME_DOOR    = 1 << 5
	};

	void ScriptNativeRegisterEntity( unsigned hEntity, JE::Entity *pEntity, unsigned nClassMask );
	void ScriptNativeUnregisterEntity( JE::Entity *pEntity );

	// Test/build helper: drop every registry entry (the shipped list has no such clear;
	// this exists so a suite can start from the empty-list state).
	void ScriptNativeResetRegistry();

} // namespace F3

#endif // F3_SCRIPTNATIVES_HPP_

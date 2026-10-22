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

	int ScriptNative_GetItemType( int amx, int params );           // 0x45b240
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
		int nExpectedCells;   // the param_3 the body passes to the arity gate
	};

	// The five getters, in shipped EA order (0x45abe0, 0x45aeb0, 0x45af40, 0x45afd0, 0x45b070).
	// Seven entries today: the five B1 getters plus GetCurrentHP/GetMaxHP (B3), in shipped
// EA order. The HP pair cast to GameEntity, NOT GameCreature - the table does not encode
// the class, the bodies do (the shipped RTTI site differs; see each body).
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

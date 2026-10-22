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
	void ScriptNativeRegisterEntity( unsigned hEntity, JE::Entity *pEntity, char nIsCreature );
	void ScriptNativeUnregisterEntity( JE::Entity *pEntity );

	// Test/build helper: drop every registry entry (the shipped list has no such clear;
	// this exists so a suite can start from the empty-list state).
	void ScriptNativeResetRegistry();

} // namespace F3

#endif // F3_SCRIPTNATIVES_HPP_

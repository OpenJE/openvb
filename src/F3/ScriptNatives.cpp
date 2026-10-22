// Copyright 2024 OpenJE
//
// R-USERRE batch B (slice B2): the script-native preamble trio + the GameCreature getter
// family. Bodies read from F3.exe bytes, cited per step; the name is a claim and the bytes are
// the contract (harness item 50), which mattered here twice - see kField offsets below.

#include "F3/ScriptNatives.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "F3/GameCreature.hpp"
#include "JE.hpp"

namespace F3 {

	// ---------------------------------------------------------------------------
	// Port-side handle registry - the byte_70BE0C stand-in.
	//
	// Shipped resolves a script entity handle in F3::GameActionEquip::meth_0x54be10 (0x54be10):
	// walk the global entity list at byte_70BE0C (doubly-linked sentinel; node+8 = entity) and
	// compare *(entity+0x1DC) against the handle (0x54be30-0x54be39, the +476 decimal IDA prints
	// is 0x1DC in bytes - the cell-vs-byte trap again), else fall through to the container at
	// byte_70BE14 (0x54be41) and return node+0x10 (0x54be61 `local_4[4]`) or 0 (0x54be68).
	// That list has no port-side container yet, so this table is what the walk runs against.
	// DEVIATION, named: entries carry an `nIsCreature` flag recorded at insert, because the port
	// links WITHOUT RTTI and the shipped `__RTDynamicCast(Entity -> GameCreature, 0)` (0x45af9f)
	// cannot run here. MapEntitySpawn may adopt this registry when the real list lands; nothing
	// here pretends the list exists.
	struct RegistryEntry {
		unsigned int hHandle;
		JE::Entity *pEntity;
		int nIsCreature;
		int nUsed;
	};

	static RegistryEntry s_registry[ 64 ];
	static int s_registryCount = 0;

		// PORT-SIDE helper: no shipped twin - the shipped list is maintained by the engine's own entity
	// construction path (see the byte_70be0c walk at 0x54be10, which this table feeds). Insert only.
void ScriptNativeRegisterEntity( unsigned hEntity, JE::Entity *pEntity, char nIsCreature ) {
		int i;
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].pEntity == pEntity ) {
				s_registry[ i ].hHandle = (unsigned int)hEntity;   // re-publish under a new handle
				s_registry[ i ].nIsCreature = nIsCreature ? 1 : 0;
				return;
			}
		}
		if ( s_registryCount >= (int)( sizeof( s_registry ) / sizeof( s_registry[0] ) ) ) {
			return;   // full: the shipped list is unbounded, this stand-in is not (deviation)
		}
		s_registry[ s_registryCount ].hHandle     = (unsigned int)hEntity;
		s_registry[ s_registryCount ].pEntity     = pEntity;
		s_registry[ s_registryCount ].nIsCreature = nIsCreature ? 1 : 0;
		s_registry[ s_registryCount ].nUsed       = 1;
		++s_registryCount;
	} // ScriptNativeRegisterEntity

		// PORT-SIDE helper: no shipped twin - removal counterpart for the stand-in table; shipped
	// entity teardown for the same list is the 0x54be10 walk's other side (byte_70be0c unlink).
void ScriptNativeUnregisterEntity( JE::Entity *pEntity ) {
		int i;
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].pEntity == pEntity ) {
				s_registry[ i ].nUsed = 0;
			}
		}
	} // ScriptNativeUnregisterEntity

		// PORT-SIDE test helper: no shipped twin. The shipped globals byte_70be0c / byte_70be14 have
	// no clear routine; this exists so a suite can start from the empty-list state 0x54be10 reads.
void ScriptNativeResetRegistry() {
		memset( s_registry, 0, sizeof( s_registry ) );
		s_registryCount = 0;
	} // ScriptNativeResetRegistry

	// What the shipped __RTDynamicCast(Entity -> GameCreature, 0) would have decided.
		// PORT-SIDE stand-in for the shipped __RTDynamicCast(&Entity -> &GameCreature) at 0x45af9f
	// (same site in the four sibling bodies: 0x45af0f, 0x45b037, 0x45b0d7, and 0x45ac3f).
static int RegistryIsCreature( JE::Entity *pEntity ) {
		int i;
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].pEntity == pEntity ) {
				return s_registry[ i ].nIsCreature;
			}
		}
		return 0;
	} // RegistryIsCreature

	// ---------------------------------------------------------------------------
	// 0x59ad20 - the script error reporter.
	//
	// Shipped steps, in order, all cited: seed "<Unknown>" (0x59ad4c); look the running script
	// up through sub_5CCCC0(amx, 1179407181, &out) (0x59ad51) - on success branch on `out >> 29`
	// (0x59ad63): 0 means an ENTITY handle, resolved with meth_0x54be10 and its name read at
	// entity+676/+0x2A4 with a 0x10 short-string test (0x59adb8-0x59adba); 1 means a WORLD object,
	// resolved through GetWorldServerInstance (0x59ad6e) + slot-21 virtual (+84) and read at +148
	// (0x59ad9c). Then seed "<unknown>" file (0x59adda), look the module up through
	// sub_5CCCC0(amx, 1279415366, &out) (0x59ae1c), fetch amx+0x30 (0x59ae38), search it with
	// cls_0x50dbb0::meth_0x56ebe0 (0x59ae3f), and if found _splitpath the full path into a 254-byte
	// Filename and 253-byte Ext (0x59ae62). Compose
	// "!Run-time error in script %s, file %s%s, line %d: " with amx+0x2C as the line (0x59ae9b-era),
	// vsprintf the caller's format at the end of the buffer (0x59aeb9), and hand the buffer to
	// (*(**byte_70C65C + 16))(*byte_70C65C, buffer) (0x59aede).
	//
	// PORT DEVIATIONS (named, not faked):
	//  - the two sub_5CCCC0 lookups and the container search have no port-side AMX module table,
	//    so the reporter uses the shipped INITIAL values of those very variables - "<Unknown>",
	//    "<unknown>", empty ext - which is what the engine itself prints when the lookups miss.
	//  - byte_70C65C's logging sink object is unported; the composed message goes to
	//    JE::LogDebugString instead. The natives REPORT AND CONTINUE either way - that is the
	//    behavior that matters and it is preserved exactly (no FatalError, no abort).
	//  - the shipped buffer is 1028 bytes at [ebp-0x408] with a security cookie at [ebp-4]
	//    (0x59ad39); the port has no cookie, as in every other ported body.
	void ScriptNativeReportError( int amx, const char *pcFormat, ... ) {
		char szBuffer[ 1028 ];
		int nLine = 0;
		va_list args;

		// amx+0x2C is the running script line (shipped reads it unconditionally at 0x59ae9b-era).
		// The null guard is port-defensive only: every shipped call site passes a live amx.
		if ( amx ) {
			nLine = *(const int *)( (const char *)amx + 0x2C );
		}

		sprintf( szBuffer, "!Run-time error in script %s, file %s%s, line %d: ",
		         "<Unknown>", "<unknown>", "", nLine );

		va_start( args, pcFormat );
		vsprintf( szBuffer + strlen( szBuffer ), pcFormat, args );
		va_end( args );

		// The shipped sink signature takes a NON-const char* format (JE.hpp:18); a literal would be
		// a write-strings warning, so the pass-through format is a real array.
		static char szPassThrough[] = "%s";
		JE::LogDebugString( (char *)0, szPassThrough, szBuffer );
	} // ScriptNativeReportError

	// ---------------------------------------------------------------------------
	// 0x59af40 - arity gate. `params[0]` is the parameter block size IN BYTES; the shipped
	// gate divides by 4 to get cells (0x59af4c `mov eax,[edi+4]`... no: `*param_2 >> 2`), then
	// compares against the expected count and, on mismatch, reports TWO messages in order and
	// returns 0. Both strings are byte-verbatim shipped, INCLUDING the misspellings
	// "paramters" (0x59af5f) and "recieved" (0x59af6c) - T-022 applies.
	char ScriptNativeCheckArity( int amx, const unsigned *params, int nCellsExpected, const char *pcFnName ) {
		int nCells = (int)( params[ 0 ] >> 2 );   // 0x59af4c

		if ( nCells == nCellsExpected ) {
			return 1;                              // 0x59af7f
		}
		ScriptNativeReportError( amx, "Invalid number of paramters passed to function %s().", pcFnName );   // 0x59af63
		ScriptNativeReportError( amx, "Expected %d parameters but recieved %d.", nCellsExpected, nCells );  // 0x59af70
		return 0;                                  // 0x59af79
	} // ScriptNativeCheckArity

	// ---------------------------------------------------------------------------
	// 0x59aef0 - entity resolve.
	//   out = meth_0x54be10(handle)              (0x59aef5 call, 0x59af03 store)
	//   out == 0            -> return 0          (0x59af05-0x59af09)
	//   checkActive flag is a BYTE arg           (0x59af0a `mov cl,[esp+arg_C]`)
	//   flag set && *(entity+0x3A0) == 0         -> REPORT "Attempt to use inactive entity in
	//   script." (0x59af12-0x59af26) AND STILL RETURN 1 (0x59af2e `mov al,1`).
	// The guard WARNS; it does not block. Every obvious reading of it is wrong, so the return
	// value is asserted in the suite rather than trusted to a comment.
	char ScriptNativeResolveEntity( int amx, unsigned hEntity, JE::Entity **ppEntity, char nCheckActive ) {
		JE::Entity *pFound = 0;
		int i;

		// The byte_70BE0C walk, against the registry stand-in (see top of file).
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].hHandle == (unsigned int)hEntity ) {
				pFound = s_registry[ i ].pEntity;
				break;
			}
		}

		*ppEntity = pFound;                        // shipped stores BEFORE the null test
		if ( !pFound ) {
			return 0;
		}
		if ( nCheckActive && pFound->mbr_0x3a0 == 0 ) {
			ScriptNativeReportError( amx, "Attempt to use inactive entity in script." );
		}
		return 1;                                  // 0x59af2e - even on the inactive warning
	} // ScriptNativeResolveEntity

	// ---------------------------------------------------------------------------
	// The getter family. One shape, five field offsets:
	//   arity(1 cell) -> resolve(params[1], active-check=1) -> creature cast -> field
	// Every failure path returns 0 (`xor eax,eax`) with no distinguishing code.
	//
	// THE OFFSET TRAP (why this file exists in bytes, not pseudocode): the decompiler rendered
	// these reads as `*(v3 + 47/50/52/53/54)` on a pointer it LABELS `char *`. Those numbers are
	// CELL indices, not bytes: 47*4=0xBC, 50*4=0xC8, 52*4=0xD0, 53*4=0xD4, 54*4=0xD8, and the
	// instructions say `mov eax,[eax+0BCh]` etc. A byte reading would have placed five adjacent
	// bytes in the GameEntity prologue and the port would have compiled happily and been wrong.
	// The offsets also land on members the port ALREADY carries anonymously at
	// src/F3/GameCreature.hpp:53-59 (mbr_0xbc..mbr_0xd8), so nothing here invents layout.
	// Resolve + creature-cast, exactly the shared middle of all five bodies. Returns the creature
	// pointer, or 0 after reporting (the shipped bodies all fail closed with `xor eax,eax`).
	//
	// WHY THE GETTERS BELOW USE NAMED MEMBERS RATHER THAN SHIPPED DISPLACEMENTS - a finding worth
	// the board's attention: the generated src/F3/*.hpp headers are name-SORTED excerpts, so the hex
	// in a member name cites the ORIGINAL layout and is NOT a position in the port. Measured on this
	// toolchain: F3::GameCreature's mbr_0xbc sits at port offset 0x4B4 (1212) and sizeof(GameCreature)
	// is 1252, while the shipped object is 3036 bytes with the field at 0xBC. Any port code that does
	// `(char*)obj + 0xBC` against these structs reads a DIFFERENT field than `obj->mbr_0xbc` does -
	// which is exactly what this slice's first draft did, and the suite caught it (10 reds) rather
	// than letting a green build hide it.
	static GameCreature *ResolveCreatureNative( int amx, int params_i, const char *pcFnName,
	                                            const char *pcResolveMsg, const char *pcCastMsg ) {
		const unsigned *params = (const unsigned *)params_i;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, params, 1, pcFnName ) ) {   // 0x59af54-era gate
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, params[ 1 ], &pEntity, 1 ) ) {
			ScriptNativeReportError( amx, "%s", pcResolveMsg );
			return 0;
		}
		// shipped: __RTDynamicCast(pEntity, 0, &Entity, &GameCreature, 0) - the port links without
		// RTTI, so the registry's recorded flag is what that cast would have decided.
		if ( !RegistryIsCreature( pEntity ) ) {
			ScriptNativeReportError( amx, "%s", pcCastMsg );
			return 0;
		}
		return (GameCreature *)pEntity;
	} // ResolveCreatureNative

	// 0x45abe0 - GetLevel: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read . Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetLevel( int amx, int params ) {
		GameCreature *pCreature = ResolveCreatureNative( amx, params, "GetLevel",
		                                                "GetLevel() - Unable to resolve the requested entity.",
		                                                "GetLevel() - Entity ID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xbc;   // shipped: mov eax,[eax+0xBCh] at 0x45ac5f
	} // ScriptNative_GetLevel

	// 0x45aeb0 - GetRace: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read: 0x45af2f `eax+0C8h`. Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetRace( int amx, int params ) {
		GameCreature *pCreature = ResolveCreatureNative( amx, params, "GetRace",
		                                                "GetRace() - Unable to resolve requested entity.",
		                                                "GetRace() - EntityID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xc8;   // shipped: mov eax,[eax+0xC8h] at 0x45af2f
	} // ScriptNative_GetRace

	// 0x45af40 - GetGender: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read: 0x45afbf `eax+0D0h`. Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetGender( int amx, int params ) {
		GameCreature *pCreature = ResolveCreatureNative( amx, params, "GetGender",
		                                                "GetGender() - Unable to resolve requested entity.",
		                                                "GetGender() - EntityID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xd0;   // shipped: mov eax,[eax+0xD0h] at 0x45afbf
	} // ScriptNative_GetGender

	// 0x45afd0 - GetEthnicity: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read: 0x45b057 `eax+0D4h`. Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetEthnicity( int amx, int params ) {
		GameCreature *pCreature = ResolveCreatureNative( amx, params, "GetEthnicity",
		                                                "GetEthnicity() - Unable to resolve requested entity.",
		                                                "GetEthnicity() - EntityID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xd4;   // shipped: mov eax,[eax+0xD4h] at 0x45b057
	} // ScriptNative_GetEthnicity

	// 0x45b070 - GetSubRace: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read: 0x45b0f7 `eax+0D8h`. Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetSubRace( int amx, int params ) {
		GameCreature *pCreature = ResolveCreatureNative( amx, params, "GetSubRace",
		                                                "GetSubRace() - Unable to resolve requested entity.",
		                                                "GetSubRace() - Entity ID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xd8;   // shipped: mov eax,[eax+0xD8h] at 0x45b0f7
	} // ScriptNative_GetSubRace

	// The {name, fn, expected cells} table in shipped EA order. NOT wired into any VM - the port
	// has no amx module table yet, and faking one would be inventing the thing being reversed.
	static const ScriptNativeEntry s_gameCreatureNatives[5] = {
		{ "GetLevel",    &ScriptNative_GetLevel,    1 },   // 0x45abe0
		{ "GetRace",     &ScriptNative_GetRace,     1 },   // 0x45aeb0
		{ "GetGender",   &ScriptNative_GetGender,   1 },   // 0x45af40
		{ "GetEthnicity",&ScriptNative_GetEthnicity,1 },   // 0x45afd0
		{ "GetSubRace",  &ScriptNative_GetSubRace,  1 }    // 0x45b070
	};

	const ScriptNativeEntry *ScriptNativeGameCreatureTable( int *pnCount ) {
		if ( pnCount ) {
			*pnCount = (int)( sizeof( s_gameCreatureNatives ) / sizeof( s_gameCreatureNatives[0] ) );
		}
		return s_gameCreatureNatives;
	} // ScriptNativeGameCreatureTable

} // namespace F3

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
#include "F3/ScriptInventory.hpp"
#include "F3/ScriptParty.hpp"
#include "F3/GameEntity.hpp"
#include "F3/GameItem.hpp"
#include "F3/GameUsableObject.hpp"
#include "F3/GameContainer.hpp"
#include "F3/GameDoor.hpp"  // +0xBD lock byte (byte mbr_0xbd, GameDoor.hpp:14)
#include "JE.hpp"

namespace F3 {

	// Last message formatted by ScriptNativeReportError (see the sink note there).
	static char s_szLastError[ 1028 ] = { 0 };

	// ---------------------------------------------------------------------------
	// Port-side handle registry - the byte_70BE0C stand-in.
	//
	// Shipped resolves a script entity handle in meth_0x54be10 (0x54be10). NAMING WARNING: the IDB's
	// "GameActionEquip::" prefix on this function (and on 0x46ee10 / 0x4629c0 / 0x46d620) is label
	// contamination - the bodies are right, the class is not; JadeBear attributes the family to
	// cls_0x616328, the same bogus label attached to the world-server singleton at 0x401150, and it is
	// server-side era. Quoted below only where the disassembly itself carries the name.:
	// walk the global entity list at byte_70BE0C (doubly-linked sentinel; node+8 = entity) and
	// compare *(entity+0x1DC) against the handle (0x54be30-0x54be39, the +476 decimal IDA prints
	// is 0x1DC in bytes - the cell-vs-byte trap again), else fall through to the container at
	// byte_70BE14 (0x54be41) and return node+0x10 (0x54be61 `local_4[4]`) or 0 (0x54be68).
	// That list has no port-side container yet, so this table is what the walk runs against.
	// DEVIATION, named: entries carry an `nClass` value recorded at insert, because the port links
	// WITHOUT RTTI and the shipped `__RTDynamicCast` sites (0x45af9f, 0x45b217 and siblings) cannot run here. MapEntitySpawn may adopt this registry when the real list lands; nothing
	// here pretends the list exists.
	struct RegistryEntry {
		unsigned int hHandle;
		JE::Entity *pEntity;
		int nClass;      // ScriptNativeClass BITMASK; the stand-in for what RTTI would decide
		int nUsed;
	};

	static RegistryEntry s_registry[ 64 ];
	static int s_registryCount = 0;

		// PORT-SIDE helper: no shipped twin - the shipped list is maintained by the engine's own entity
	// construction path (see the byte_70be0c walk at 0x54be10, which this table feeds). Insert only.
void ScriptNativeRegisterEntity( unsigned hEntity, JE::Entity *pEntity, unsigned nClassMask ) {
		int i;
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].pEntity == pEntity ) {
				s_registry[ i ].hHandle = (unsigned int)hEntity;   // re-publish under a new handle
				s_registry[ i ].nClass = (int)nClassMask;
				return;
			}
		}
		if ( s_registryCount >= (int)( sizeof( s_registry ) / sizeof( s_registry[0] ) ) ) {
			return;   // full: the shipped list is unbounded, this stand-in is not (deviation)
		}
		s_registry[ s_registryCount ].hHandle     = (unsigned int)hEntity;
		s_registry[ s_registryCount ].pEntity     = pEntity;
		s_registry[ s_registryCount ].nClass = (int)nClassMask;
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

	// PORT-SIDE stand-in for __RTDynamicCast. The shipped cast TARGET differs per family -
	// Entity->GameCreature at 0x45af9f / 0x45af0f / 0x45b037 / 0x45b0d7 / 0x45ac3f (the B1 getters)
	// and Entity->GameEntity at 0x45b217 / 0x45b177 (the HP pair) - and because GameCreature derives
	// from GameEntity the recorded nClass is an IS-A ladder: `nClass >= nClassNeeded` is the same
	// decision the two TypeDescriptor pairs implement.
	static int RegistryIsClass( JE::Entity *pEntity, unsigned nClassNeeded ) {
		int i;
		for ( i = 0; i < s_registryCount; ++i ) {
			if ( s_registry[ i ].nUsed && s_registry[ i ].pEntity == pEntity ) {
				return ( (unsigned)s_registry[ i ].nClass & nClassNeeded ) != 0 ? 1 : 0;
			}
		}
		return 0;
	} // RegistryIsClass

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

		// TEST SEAM, not a behavior: the shipped sink is JE::LogDebugString and this TU cannot
		// override a definition it links against, so exact-string assertions (the sibling-space
		// quirk in B13) need the formatted message readable here. It is a copy, so the sink call
		// above is untouched and nothing about shipped behavior changes.
		//
		// It ACCUMULATES rather than overwriting, because the arity gate legitimately reports TWO
		// messages in order (0x59af63 then 0x59af70) and a last-only capture made the first one
		// unassertable - which is how B18's arity-copy-paste assert failed its first run. One line
		// per report keeps both reachable without inventing anything.
		size_t nHave = strlen( s_szLastError );
		const size_t nNeed = strlen( szBuffer ) + 2;   // separator + terminator
		if ( nHave + nNeed >= sizeof( s_szLastError ) ) {
			// Full: restart with this message rather than silently STOPPING capturing. The first
			// version of this seam appended unconditionally and, once the 1028-byte buffer filled,
			// every later message assert downstream read stale text and failed - 10 reds from a
			// capture that had quietly gone dead. A capture that stops is worse than one that
			// forgets, because it looks like the code stopped reporting.
			nHave = 0;
			s_szLastError[ 0 ] = 0;
		}
		if ( nHave ) {
			s_szLastError[ nHave ] = '\n';
			++nHave;
			s_szLastError[ nHave ] = 0;
		}
		strncpy( s_szLastError + nHave, szBuffer, sizeof( s_szLastError ) - nHave - 1 );
		s_szLastError[ sizeof( s_szLastError ) - 1 ] = 0;
	} // ScriptNativeReportError

	// Clears the capture so a leg can be inspected without history. Exists for the ordered PAIR the
	// arity gate emits - "Invalid number of paramters passed to function %s()." (0x59af63) then
	// "Expected %d parameters but recieved %d." (0x59af70) - and for the copy-paste in UnequipItem's
	// gate name (0x45a72c), both of which need a known-empty buffer to assert against. No shipped
	// counterpart: the shipped sink is a log call with no readback.
	void ScriptNativeResetLastError() {
		s_szLastError[ 0 ] = 0;
	} // ScriptNativeResetLastError

	const char *ScriptNativeGetLastError() {
		return s_szLastError;
	} // ScriptNativeGetLastError

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

	// NOTE on shipped telemetry (banked in docs/notes/shipped-literals.md): the resolve-failure and
	// cast-failure strings are NOT consistent across this family - GetLevel says "the requested entity"
	// while its siblings say "requested entity", and "EntityID" vs "Entity ID" splits the cast lines. One
	// native even names a different function in its own error (GetPerkRank @0x45ab8e reports
	// "GetPosition() was unable to find the requested entity." - a shipped copy-paste bug). Bodies below
	// pass the strings VERBATIM, wrongness included; T-022 identity, not editorial licence.

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
	// nCells is the ROW'S OWN arity literal - the cell count shipped pushes into its gate immediate -
	// passed at each call site and equal to the table's nExpectedCells for that row. It was a
	// hardcoded 1 inside this helper until the set-A parameterisation (harness item 71: with the
	// literal hidden here, the table column was DECORATIVE on all 13 helper-mediated rows and no
	// audit could see their enforcement). Behavior is unchanged: all 13 rows gate exact at 1 cell
	// in shipped, and the suite proves it (zero changed outcomes); the fix removes the accident.
	//
	// GATE KIND, stated so nobody "completes" this: shipped has a SECOND gate - 0x59AF90 MINIMUM
	// (cmp/jge @0x59afa7, 5 args with an &out_actual, add esp,14h, "Expected at least %d parameters
	// but recieved %d."). NO F3 game-lib native uses it (FastNova 21886c1 census: the six min-gated
	// are all engine-core SendMessage-family, none ported into this table). So this helper is
	// EXACT-KIND ONLY BY EVIDENCE, not by omission: if a min-gated native is ever ported, it needs
	// a NEW gate function (jge semantics, the out-param, the third message string) - "at least" is
	// not a spelling borrowable from the exact gate, and faking it with the exact form would let a
	// legal 3- or 4-cell SendMessage call fail a gate shipped admits. Do not parameterise kind in
	// here; add the second form beside it when a row needs it.
	//
	// nCells == 0 is a REAL arity (Protagonist gates at 0 cells in shipped) and must never be
	// conflated with the table's -1 SENTINEL ("no gate exists", the quest/journal alias quad).
	// This helper is for gated rows only; a -1 row has no business calling it.
	//
	// WHY THE GETTERS BELOW USE NAMED MEMBERS RATHER THAN SHIPPED DISPLACEMENTS - a finding worth
	// the board's attention: the generated src/F3/*.hpp headers are name-SORTED excerpts, so the hex
	// in a member name cites the ORIGINAL layout and is NOT a position in the port. Measured on this
	// toolchain: F3::GameCreature's mbr_0xbc sits at port offset 0x4B4 (1212) and sizeof(GameCreature)
	// is 1252, while the shipped object is 3036 bytes with the field at 0xBC. Any port code that does
	// `(char*)obj + 0xBC` against these structs reads a DIFFERENT field than `obj->mbr_0xbc` does -
	// which is exactly what this slice's first draft did, and the suite caught it (10 reds) rather
	// than letting a green build hide it.
	static JE::Entity *ResolveNativeEntity( int amx, int params_i, int nCells, const char *pcFnName, unsigned nClassNeeded,
	                                        const char *pcResolveMsg, const char *pcCastMsg ) {
		const unsigned *params = (const unsigned *)params_i;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, params, nCells, pcFnName ) ) {   // 0x59AF40-era exact gate
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, params[ 1 ], &pEntity, 1 ) ) {
			ScriptNativeReportError( amx, "%s", pcResolveMsg );
			return 0;
		}
		// shipped: __RTDynamicCast(pEntity, 0, &Entity, &GameCreature, 0) - the port links without
		// RTTI, so the registry's recorded flag is what that cast would have decided.
		if ( !RegistryIsClass( pEntity, nClassNeeded ) ) {
			ScriptNativeReportError( amx, "%s", pcCastMsg );
			return 0;
		}
		return pEntity;
	} // ResolveNativeEntity

	// 0x45abe0 - GetLevel: arity 1 cell, resolve with the active-check flag, creature cast, then the
	// single field read . Every failure path returns 0 with no distinguishing code.
	int ScriptNative_GetLevel( int amx, int params ) {
		GameCreature *pCreature = (GameCreature *)ResolveNativeEntity( amx, params, 1, "GetLevel", F3::SNC_GAME_CREATURE,
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
		GameCreature *pCreature = (GameCreature *)ResolveNativeEntity( amx, params, 1, "GetRace", F3::SNC_GAME_CREATURE,
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
		GameCreature *pCreature = (GameCreature *)ResolveNativeEntity( amx, params, 1, "GetGender", F3::SNC_GAME_CREATURE,
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
		GameCreature *pCreature = (GameCreature *)ResolveNativeEntity( amx, params, 1, "GetEthnicity", F3::SNC_GAME_CREATURE,
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
		GameCreature *pCreature = (GameCreature *)ResolveNativeEntity( amx, params, 1, "GetSubRace", F3::SNC_GAME_CREATURE,
		                                                "GetSubRace() - Unable to resolve requested entity.",
		                                                "GetSubRace() - Entity ID does not belong to a game creature." );
		if ( !pCreature ) {
			return 0;
		}
		return (int)pCreature->mbr_0xd8;   // shipped: mov eax,[eax+0xD8h] at 0x45b0f7
	} // ScriptNative_GetSubRace

	// 0x45b1b0 - GetCurrentHP: arity 1, resolve, cast Entity->GameEntity (0x45b217), then
	// 0x45b237 `mov eax,[eax+24h]`. Shipped offset +0x24 (port: the named member).
	int ScriptNative_GetCurrentHP( int amx, int params ) {
		GameEntity *pEntity = (GameEntity *)ResolveNativeEntity( amx, params, 1, "GetCurrentHP",
		                                                         F3::SNC_GAME_ENTITY,
		                                                         "GetCurrentHP() - Unable to resolve requested entity.",
		                                                         "GetCurrentHP() - Entity ID does not belong to a GameEntity." );
		if ( !pEntity ) {
			return 0;
		}
		return (int)pEntity->mbr_0x24;   // shipped: mov eax,[eax+24h] at 0x45b237
	} // ScriptNative_GetCurrentHP

	// 0x45b110 - GetMaxHP: arity 1, resolve, cast Entity->GameEntity (0x45b177), then a VIRTUAL call,
	// not a field read: 0x45b197 `mov edx,[eax]; mov ecx,eax; call dword ptr [edx]` - vftable SLOT 0
	// with this = the entity. The slot's target is F3::GameEntity::virt_meth_0x4080c0 (0x666130[0]),
	// which is itself two instructions: `mov eax,[ecx+20h]; retn` at 0x4080c0. So max HP is the
	// GameEntity+0x20 accessor behind a virtual - which means a derived class CAN override it, and the
	// port calling the member directly loses that dispatch. DEVIATION, named: the port has no
	// GameEntity vftable yet, so the body goes through GameEntityMaxHp() - the single seam a future
	// vtable install can replace, rather than five inline field reads pretending to be virtuals.
	static int GameEntityMaxHp( GameEntity *pEntity ) {
		return (int)pEntity->mbr_0x20;   // shipped: virt_meth_0x4080c0 = mov eax,[ecx+20h] at 0x4080c0
	} // GameEntityMaxHp

	int ScriptNative_GetMaxHP( int amx, int params ) {
		GameEntity *pEntity = (GameEntity *)ResolveNativeEntity( amx, params, 1, "GetMaxHP",
		                                                         F3::SNC_GAME_ENTITY,
		                                                         "GetMaxHP() - Unable to resolve requested entity.",
		                                                         "GetMaxHP() - Entity ID does not belong to a GameEntity." );
		if ( !pEntity ) {
			return 0;
		}
		return GameEntityMaxHp( pEntity );
	} // ScriptNative_GetMaxHP

	// ---- the quest/journal alias quad (batch B slice B24) - ONE body, FOUR names -------------------
	// Shipped's F3 game-lib native table (the {name, fn} array at 0x6F3750, read from the PE itself rather
	// than from any database listing) holds FOUR entries - OpenQuest (idx 41), CloseQuest (42),
	// AddQuestEntry (43), AddJournalEntry (44) - whose fn pointer is the SAME dword 0x420990 in all four
	// slots. The body at 0x420990 is `b8 01 00 00 00 c3` = `mov eax,1; ret`: no arity gate, no params read,
	// no ecx, no branch, no callee. So quest and journal scripting is a hardcoded TRUE in the shipped demo,
	// and the four names are aliases of one function rather than four functions that happen to agree.
	//
	// NAMING WARNING carried with the EA: the IDB labels 0x420990 as `JE::GUI::Window::virt_meth_0x420990`
	// because it is ALSO the last slot of a GUI::Window vftable (@0x66e4d4 slot 74). That is an artifact of
	// how the DB names things from one usage; nothing about the body knows about windows. Anchor on the EA
	// and the five bytes, not on the label.
	//	
	// PORT SHAPE, and why it looks the way it does: ONE body, FOUR table rows. Collapsing the four names
	// into one row would misrepresent shipped's registration (a script calling `CloseQuest` resolves by
	// NAME against the module's native list at 0x5ccd60-era, so all four names must exist to be findable),
	// and giving them four bodies would invent four things that are one. So the rows share a pointer and
	// each keeps its own name - the one place in this batch where the table is doing aliasing on purpose.
	// nExpectedCells is -1 = SENTINEL "no gate exists", not "zero cells": a row that declared 0 would claim
	// the body compares params[0]>>2 against 0, which it does not do at all. The suite probes these rows at
	// several cell counts and asserts the arity message NEVER appears.
	int ScriptNative_QuestJournalAlias( int amx, int params ) {
		(void)amx;    // 0x420990 reads NOTHING: no gate, no cells, no entity, no singleton
		(void)params;
		return 1;     // 0x420995 `mov eax,1` - the only path in the function
	} // ScriptNative_QuestJournalAlias

	// ---- the weapon-combo pair (batch B slice B23) - two setters/getters that set and get nothing ----
	// 0x45b9f0 - the resolve-and-cast the pair shares. Shipped shape, byte-verified:
	//   45ba02 `call sub_59AEF0(amx, handle, &out, 1)`   - the ACTIVE CHECK IS ON (flag 1). NOT a
	//   distinguishing feature: of the 105 direct callers of this wrapper in the image, 104 pass 1 and the
	//   only flag-0 caller is JE::ActivateEntity_0x5c9180 @0x5c91ac, whose job is finding inactive entities.
	//   (Census from the PE .text, corroborated by NiceTiger's IDA CodeRefsTo pass. The path that skips the
	//   wrapper entirely is the server tail meth_0x46ee10 -> meth_0x54be10 @0x46ee19, which is why
	//   HpSetterChain below correctly passes 0: no wrapper means no flag and no inactive warning.)
	//   45ba23 `call ___RTDynamicCast` with &Entity -> &GameEntity
	//   45ba31 `mov [edi],eax` / 45ba33 `ret` (al = cast != 0)
	// The cast result is stored EVEN WHEN IT IS 0, and on the resolve-failure path the out param is
	// already 0 because 0x59aef0 stores 0 before returning (asserted in section 7 of the suite), so this
	// helper does NOT write on that path - mirroring shipped rather than tidying it.
	static int ResolveGameEntityForCombo( int amx, unsigned hHandle, JE::Entity **ppOut ) {
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeResolveEntity( amx, hHandle, &pEntity, 1 ) ) {   // 0x45ba02
			return 0;                                                       // 0x45ba0f - out already 0, untouched here
		}
		*ppOut = RegistryIsClass( pEntity, F3::SNC_GAME_ENTITY ) ? pEntity : 0;   // 0x45ba23 / 0x45ba31
		return *ppOut ? 1 : 0;                                                   // 0x45ba33
	} // ResolveGameEntityForCombo

	// 0x45a9f0 - GetWeaponCombo: arity 1, resolve, and then the resolved entity is DISCARDED. Every path
	// returns 0 (0x45aa34 `xor eax,eax`), so scripts reading a weapon combo have been reading a constant
	// for the whole life of the build - the port keeps the constant and the one message, and does NOT
	// "helpfully" return what it resolved (docs/f3/WeaponComboStubs.md).
	int ScriptNative_GetWeaponCombo( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pTarget = 0;

		if ( !ScriptNativeCheckArity( amx, p, 1, "GetWeaponCombo" ) ) {   // 0x45aa04
			return 0;                                                     // 0x45aa0e jz -> 0x45aa34, no further message
		}
		if ( !ResolveGameEntityForCombo( amx, p[ 1 ], &pTarget ) ) {      // 0x45aa1a
			ScriptNativeReportError( amx,
			                             "GetWeaponCombo() was unable to find the requested entity." );   // 0x45aa2c, "@0x65e1a4
		}
		(void)pTarget;   // shipped never reads it either - the resolve exists only to decide the message
		return 0;                                                        // 0x45aa34 - EVERY path
	} // ScriptNative_GetWeaponCombo

	// 0x45aa40 - SetWeaponCombo: arity 2, and the SECOND CELL IS NEVER READ. 0x45aa60 loads `[edi+4]`
	// (params[1]) and nothing loads `[edi+8]`; the callee's result goes nowhere; the return is 0 on every
	// path (0x45aa85). So shipped's weapon-combo SETTER SETS NOTHING - a two-cell gate, a resolve, an
	// optional error line, and zero. Kept verbatim because "the API exists and does nothing" is the
	// finding, and a port that wrote the combo would erase it. There is no combo field to blame: the
	// instruction simply is not there (see docs/f3/WeaponComboStubs.md for the absence argument).
	int ScriptNative_SetWeaponCombo( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pTarget = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "SetWeaponCombo" ) ) {   // 0x45aa54
			return 0;                                                     // 0x45aa5c jz -> 0x45aa84
		}
		if ( !ResolveGameEntityForCombo( amx, p[ 1 ], &pTarget ) ) {      // 0x45aa6a - params[1] ONLY
			ScriptNativeReportError( amx,
			                             "SetWeaponCombo() was unable to find the requested entity." );   // 0x45aa7c, "@0x65e1f0
		}
		(void)pTarget;   // params[2] - the combo value - is not read by shipped at all, so nothing here reads it
		return 0;                                                        // 0x45aa85 - EVERY path
	} // ScriptNative_SetWeaponCombo

	// ---- the task-journal trio (batch B slice B22) -------------------------------------------
	// Shipped body, byte-verified, and the SAME 24 instructions in all three (0x45b900 / 0x45b940 /
	// 0x45b980) except the gate name and the delegate EA:
	//   arity gate with 2 cells and its OWN name -> gate fail: return 0
	//   -> read params[2] into ecx (0x45b922) and params[1] into edx (0x45b925)
	//   -> load the subsystem manager and take the server from [mgr+0x18] (0x45b928/2e, NO null test)
	//   -> push ecx, push edx, call GameServer::meth_0x46eXX0 (cdecl order: (params[1], params[2]))
	//   -> `mov eax,1` (0x45b937) with no test of anything the callee did.
	// These are the ONLY batch-B natives that never resolve an entity - no 0x59aef0, no RTTI cast, no
	// error message on the success path. So the two cells travel as opaque values, which is why the
	// port's observable contract here is (gate outcome, return 1) and nothing else.
	//
	// NAMED-DEFERRED, and deferred as a CHAIN rather than leg by leg: each delegate is
	//   vbptr walk on world-server singleton 0x70bd98 -> vft slot 30 -> RTTI `Journal -> GameJournal`
	//   -> [journal write] + [GameServer packet tail], and BOTH of those need objects the port does not
	//   have (no GameJournal instance - GameWorld.cpp:78 allocates one but nothing wires the natives to
	//   it, and the singleton's slot-30 accessor is unported; no net::packet era). Journal write bodies
	//   are READ and recorded, not guessed: sub_46C7A0 upserts a JE::TaskStatus node keyed on params[1]
	//   storing params[2] with state 1 (0x46c805 `mov [v9+0Ch],1`), sub_46C840 is the SAME walk with
	//   state 2 (0x46c8a5/0x46c88e) - one function plus a literal, not two behaviors - and sub_46C6C0 is
	//   an add-if-absent over BOTH key fields with a `JournalEntry -> TaskEntry` cast in the scan.
	//   Packet tails: 0x46dad0 = id 97 (0x46db25), 0x46dbb0 = id 98 (0x46dc05), 0x46dc90 = id 99
	//   (0x46dce5), each serializing both cells then GameServer::meth_0x5a0a00(packet, 1).
	// Deliberately NOT built here: a stand-in journal list inside this TU. The party seam (B21) could
	// stand in because a party is ONE object the natives already reach by pointer; a journal would need
	// the port to pick which world owns it (GameWorld's +0xF8 or a server singleton that does not exist
	// here), and a guess would hide exactly that question - the same error class as the handle bridge
	// NativeWiringGaps.md gap 5 refuses to invent.
	//
	// SHIPPED LANDMINES KEPT (see the header block for the full statement): (1) the unconditional 1 -
	// a script with no journal object still gets success, and a failed `Journal -> GameJournal` cast
	// skips BOTH the write and the packet with no message (0x46ed06 jz -> 0x46ed28); (2) shipped's null
	// states are crashes, not failures - `mov ecx,[eax+18h]` faults at 0x45b92e on a null manager, and
	// the delegate's own null-singleton branch faults at 0x46ecea / 0x46ed4a / 0x46edaa one instruction
	// after setting the pointer to 0. The port has no manager and no singleton, so it cannot be in that
	// state at all; it answers 1, which is the value shipped's non-crashing path (failed cast) returns.
	// That is a DECLARED collapse of a shipped crash, not a claim that shipped returns 1 on a null server.

	// 0x45b900 - OpenTask.
	int ScriptNative_OpenTask( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		if ( !ScriptNativeCheckArity( amx, p, 2, "OpenTask" ) ) {   // 0x45b912 - and its OWN name (0x45b909)
			return 0;                                                // 0x45b91e
		}
		// 0x45b922-0x45b932: (params[1], params[2]) -> GameServer::meth_0x46ecd0 - NAMED-DEFERRED era.
		return 1;                                                    // 0x45b937 - unconditional
	} // ScriptNative_OpenTask

	// 0x45b940 - CloseTask.
	int ScriptNative_CloseTask( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		if ( !ScriptNativeCheckArity( amx, p, 2, "CloseTask" ) ) {  // 0x45b952 (name pushed at 0x45b949)
			return 0;                                               // 0x45b95e
		}
		// 0x45b962-0x45b972: (params[1], params[2]) -> GameServer::meth_0x46ed30 - NAMED-DEFERRED era.
		return 1;                                                   // 0x45b977 - unconditional
	} // ScriptNative_CloseTask

	// 0x45b980 - AddEntry. Same body; only the name (0x45b989) and the delegate (0x46ed90) differ.
	int ScriptNative_AddEntry( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		if ( !ScriptNativeCheckArity( amx, p, 2, "AddEntry" ) ) {  // 0x45b992
			return 0;                                               // 0x45b99e
		}
		// 0x45b9a2-0x45b9b2: (params[1], params[2]) -> GameServer::meth_0x46ed90 - NAMED-DEFERRED era.
		return 1;                                                   // 0x45b9b7 - unconditional
	} // ScriptNative_AddEntry

	// 0x45b9c0 - PlayMusic. Shipped stub: gate, report, zero. Note the report happens when the arity
	// gate PASSES (0x45b9d2 tests the gate and falls through to 0x45b9e4) - i.e. a well-formed call is
	// the one that gets told it is unimplemented, and a malformed call gets the arity message instead.
	int ScriptNative_PlayMusic( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		if ( ScriptNativeCheckArity( amx, p, 1, "PlayMusic" ) ) {   // 0x45b9d2
			ScriptNativeReportError( amx, "PlayMusic() unimplemented." );   // 0x45b9e4
		}
		return 0;                                                   // 0x45b9ee - the only return
	} // ScriptNative_PlayMusic

	// 0x45b2e0 / 0x45b390 - the shared tail both writers run through 0x46ee10.
	// Shipped leg order, cited: resolve(handle) 0x46ee19 -> null means return 0 with NO message
	// (0x46ee40 path, silent) -> RTTI Entity->GameEntity 0x46ee34 -> clamp via 0x4629c0 (0x46ee4f,
	// which stores value at +0x24 then caps it at +0x20) -> broadcast via 0x46d620 (0x46ee58, packet id
	// 88 - DEFERRED here, named) -> return 1 (0x46ee62-era).
	static int HpSetterChain( unsigned hHandle, int nValue ) {
		JE::Entity *pTarget = 0;

		if ( !ScriptNativeResolveEntity( 0, hHandle, &pTarget, 0 ) || !pTarget ) {
			return 0;   // 0x46ee23: silent, exactly as shipped
		}
		if ( !RegistryIsClass( pTarget, F3::SNC_GAME_ENTITY ) ) {
			return 0;   // 0x46ee3e: still silent - the cast failure carries no string either
		}

		F3::GameEntity *pEntity = (F3::GameEntity *)pTarget;
		int nMax = (int)pEntity->mbr_0x20;                       // 0x4629c4
		pEntity->mbr_0x24 = nValue;                              // 0x4629c9 (unconditional first)
		if ( nValue > nMax ) {
			pEntity->mbr_0x24 = nMax;                            // 0x4629ce: upper cap ONLY, no floor
		}
		// 0x46ee58 meth_0x46d620(handle, value) -> packet id 88 through GameServer::meth_0x5a0a00's
		// per-peer fan-out. NAMED-DEFERRED: the server era is unported, so HP is currently written
		// locally only. Do not "helpfully" log a substitute - the shipped call has no other effect.
		return 1;
	} // HpSetterChain

	// 0x45b2e0 - SetHP: arity 2, resolve params[1] (active-check flag set), cast GameEntity, then the
	// handle round-trip above with params[2] as the value.
	int ScriptNative_SetHP( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "SetHP" ) ) {          // 0x45b2fe
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {  // 0x45b314
			ScriptNativeReportError( amx, "SetHP() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_ENTITY ) ) {        // 0x45b347
			ScriptNativeReportError( amx, "SetHP() - Entity ID does not belong to a GameEntity." );
			return 0;
		}
		// 0x45b367-6d: read the entity's OWN handle out of the virtual-base slot (Entity-base+0x1DC)
		// and pass THAT to the chain - the shipped code addresses the target by handle, not pointer.
		return HpSetterChain( (unsigned)((JE::Entity *)pEntity)->mbr_0x1dc, (int)p[ 2 ] );
	} // ScriptNative_SetHP

	// 0x45b390 - AddHP: identical preamble; the value is computed as current + params[2] BEFORE the
	// chain (0x45b3de reads [entity+0x24]), so shipped "add HP" is a set-to-computed, and the result is
	// still capped once - it never caps the delta separately.
	int ScriptNative_AddHP( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "AddHP" ) ) {           // 0x45b3ae
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45b3c4
			ScriptNativeReportError( amx, "AddHP() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_ENTITY ) ) {         // 0x45b3f7
			ScriptNativeReportError( amx, "AddHP() - Entity ID does not belong to a GameEntity." );
			return 0;
		}
		F3::GameEntity *pFrom = (F3::GameEntity *)pEntity;
		return HpSetterChain( (unsigned)pFrom->mbr_0x1dc,
		                      (int)pFrom->mbr_0x24 + (int)p[ 2 ] );
	} // ScriptNative_AddHP

	// 0x45b4e0 - IsLocked: DOOR only (0x45b53f), read at +0xBD with movzx (0x45b55f), and 0 on every
	// failure path - so "unlocked" and "not a door" are indistinguishable to the script.
	int ScriptNative_IsLocked( int amx, int params ) {
		GameDoor *pDoor = (GameDoor *)ResolveNativeEntity( amx, params, 1, "IsLocked",
		                                                   F3::SNC_GAME_DOOR,
		                                                   "IsLocked() was unable to find the entity.",
		                                                   "IsLocked() - Entity is not a door" );
		if ( !pDoor ) {
			return 0;
		}
		return (int)(unsigned char)pDoor->mbr_0xbd;   // shipped: movzx eax,byte ptr [eax+0BDh] at 0x45b55f
	} // ScriptNative_IsLocked

	// 0x45a1b0 - LockDoor: same gate, same DOOR-only cast (0x45a20f), then stores 1 (0x45a230) and
	// returns 1. No packet, no broadcast, no event - unlike HP, locking a door is LOCAL state here.
	int ScriptNative_LockDoor( int amx, int params ) {
		GameDoor *pDoor = (GameDoor *)ResolveNativeEntity( amx, params, 1, "LockDoor",
		                                                   F3::SNC_GAME_DOOR,
		                                                   "LockDoor() was unable to find the entity.",
		                                                   "LockDoor() - Entity is not a door" );
		if ( !pDoor ) {
			return 0;
		}
		pDoor->mbr_0xbd = 1;   // shipped: mov byte ptr [eax+0BDh],1 at 0x45a230
		return 1;
	} // ScriptNative_LockDoor

	// 0x45a240 - UnlockDoor: the mirror image, storing 0 (0x45a2c0).
	int ScriptNative_UnlockDoor( int amx, int params ) {
		GameDoor *pDoor = (GameDoor *)ResolveNativeEntity( amx, params, 1, "UnlockDoor",
		                                                   F3::SNC_GAME_DOOR,
		                                                   "UnlockDoor() was unable to find the entity.",
		                                                   "UnlockDoor() - Entity is not a door" );
		if ( !pDoor ) {
			return 0;
		}
		pDoor->mbr_0xbd = 0;   // shipped: mov byte ptr [eax+0BDh],0 at 0x45a2c0
		return 1;
	} // ScriptNative_UnlockDoor

	// 0x409a60 - GameCreature::GetDerivedAttribute. Ported as a file-static helper: a 15-case switch
	// (cmp eax,0Eh at 0x409a67) over the primary band. Every formula below is the shipped arithmetic,
	// with the EA of its load or imul cited where the constant is not obvious. Notes worth keeping:
	//   - HIT_POINTS is NOT a field: it re-enters this function for ACTION_POINTS, converts to double,
	//     scales by 1/6000, and returns (int)(1.0 / that) - with the zero case producing 1000, not 0.
	//     The scaling is a MULTIPLY then a reciprocal, so writing 6000/ap is NOT the same computation
	//     in floating point and this port mirrors the shipped two steps deliberately.
	//   - the three resistances share one body that returns 0 (goto caseD_a in the pseudocode), so
	//     "no resistance implemented" and "zero resistance" are the same answer in shipped too.
	//   - PERK_RATE is the constant 3; PERK SIZING is not this function's business.
	//   - default (>14) logs via JE::LogDebugString_0x497120 and returns 0. That debug sink is
	//     UNPORTED - NAMED-DEFERRED like the packet-88 broadcast: the VALUE stays faithful (0), the
	//     log line is not emitted. Do not route it through ScriptNativeReportError: shipped does not
	//     write the script error string here, and conflating the two sinks would be an invention.
	static int CreatureDerivedAttribute( const GameCreature *pC, int nWhich ) {
		switch ( nWhich ) {
			case SNC_DA_ACTION_POINTS:                            // 0x409a77: [ecx+774h] + 20
				return pC->agility_0x774 + 20;
			case SNC_DA_CARRY_WEIGHT:                             // 0x409a86: 25 * ([ecx+760h] + 1)
				return 25 * (pC->strength_0x760 + 1);
			case SNC_DA_COMBAT_SEQUENCE:                          // 0x409a96: 2 * [ecx+764h]
				return 2 * pC->perception_0x764;
			case SNC_DA_CRITICAL_CHANCE:                          // 0x409aaa: [ecx+778h]
				return pC->luck_0x778;
			case SNC_DA_EVASION:                                  // 0x409ab6: SEPARATE field +0x7FC
				return pC->maybe_agility_0x7fc;
			case SNC_DA_HEALING_RATE:                             // 0x409ac2: [ecx+768h]
				return pC->endurance_0x768;
			case SNC_DA_MELEE_DAMAGE:                             // 0x409ace: [ecx+760h]
				return pC->strength_0x760;
			case SNC_DA_PERCEPTION_RANGE:                         // 0x409ade: 3 * [ecx+764h] + 40
				return 3 * pC->perception_0x764 + 40;
			case SNC_DA_PERK_RATE:                                // 0x409ae9: constant 3
				return 3;
			case SNC_DA_RADIATION_RESISTANCE:                     // all three share one body
			case SNC_DA_FLASH_RESISTANCE:                         // (goto caseD_a) returning 0
			case SNC_DA_BIOLOGICAL_RESISTANCE:
				return 0;
			case SNC_DA_SKILL_RATE:                               // 0x409af9: 2 * [ecx+770h] + 5
				return 2 * pC->intelligence_0x770 + 5;
			case SNC_DA_THROWING_RANGE:                           // 0x409b08: 3 * [ecx+760h]
				return 3 * pC->strength_0x760;
			case SNC_DA_HIT_POINTS: {                             // 0x409b1d: the AP reciprocal
				// WHAT THE ENGINE DOES (bytes, no argument): `fild` AP, `fmul ds:dbl_64D590` with the
				// stored constant 3F25D867C3ECE2A5 (nearest double to 1/6000, which sits BELOW it),
				// `fcom` against 0.0, on equality the 0x3E8 branch returns 1000, else `fld 1.0`,
				// `fdiv`, then `call __ftol2` at 0x409b38. Between FMUL and the call there is NO store
				// of a real: every intermediate stays in the 80-bit register stack, and __ftol2 reads
				// it directly (`fistp qword`), so the value never crosses the double grid.
				//
				// WHY THAT MEANS INTEGER DIVISION IS THE FAITHFUL PORT: with no rounding until the
				// truncation, AP * (1/6000) is exact-and-slightly-below AP/6000, so 1.0/that is
				// slightly ABOVE 6000/AP, and truncation toward zero returns floor(6000/AP) - at every
				// divisor (the extra ulps are far below 1.0) and at every non-divisor (the distance to
				// the next integer is >= 1/AP, which dwarfs any ulp). Verified over AP 1..4000 against
				// both a double-rounded chain and an extended-precision chain: zero disagreements with
				// floor(6000/AP).
				//
				// WHY THE FP-SHAPED PORT WAS WRONG (this is the instructive part): mirroring the
				// formula as `double scaled = ap * k; return (int)(1.0 / scaled);` gave 199 at AP=30
				// while the engine gives 200. Measured bit-for-bit: storing the product as a DOUBLE
				// snaps 30*k to 3F747AE147AE147B, which is ABOVE 0.005 - the store INVERTS the
				// below-constant direction - and then the reciprocal is consumed at extended precision
				// as 199.9999999999999958, which truncates DOWN to 199. Neither pure shape does that:
				// all-double rounds the reciprocal back to exactly 200.0, all-extended lands above 200.
				// The port's error was a MIXED chain - double at the store, extended at the use - which
				// no amount of "mirroring the formula" would have revealed.
				//
				// WHY NOT `long double`: on MSVC long double IS double, so the "keep it wide" fix is
				// not portable across the two toolchains this file must build in, and the port would
				// then depend on each compiler's spill decisions. Integer division is the only form
				// that is deterministic here AND provably equal to the engine's behavior over the whole
				// reachable domain. C++'s truncation-toward-zero also matches __ftol2 for negative AP
				// (agility below -20), which the asserts pin.
				const int nAp = CreatureDerivedAttribute( pC, SNC_DA_ACTION_POINTS );
				if ( nAp == 0 ) {
					return 1000;                       // 0x409b4c: the divide guard is the value
				}
				return 6000 / nAp;
			}
			default:
				// shipped: JE::LogDebugString_0x497120("GameCreature::GetDerivedAttribute() - Unknown
				// attribute requested: %d", nWhich) then 0. Debug sink NAMED-DEFERRED, value kept.
				return 0;
		}
	} // CreatureDerivedAttribute

	// 0x45ac70 - GetPrimaryAttribute: arity 2, resolve, cast GameCreature, then the ARRAY read
	// `mov eax,[eax+ecx*4+760h]` at 0x45acf2 off the POST-CAST base. There is NO bounds test on
	// params[2] anywhere in the native - index 99 reads past the band and returns whatever is there,
	// and that is shipped behavior, asserted rather than "fixed".
	int ScriptNative_GetPrimaryAttribute( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "GetPrimaryAttribute" ) ) {   // 0x45ac8e
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {         // 0x45ac9c
			ScriptNativeReportError( amx, "GetPrimaryAttribute() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {             // 0x45accf
			// SHIPPED SPELLING, byte-identical, including the SPACE in "Entity ID". Its sibling
			// GetDerivedAttribute prints the SAME sentence WITHOUT the space ("EntityID") - see the
			// B13 note in tasks/R-USERRE-finish-user-functions.md; do not unify them.
			ScriptNativeReportError( amx,
				"GetPrimaryAttribute() - Entity ID does not belong to a game creature." );
			return 0;
		}
		const GameCreature *pC = (const GameCreature *)pEntity;
		return ( (const int *)&pC->strength_0x760 )[ (int)p[ 2 ] ];   // 0x45acf2, unbounded like shipped
	} // ScriptNative_GetPrimaryAttribute

	// 0x45ad00 - GetDerivedAttribute: same preamble, same delegate shape, and the cast-failure string
	// differs from its sibling by that ONE SPACE. Shipped passes the index straight into the switch.
	int ScriptNative_GetDerivedAttribute( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "GetDerivedAttribute" ) ) {   // 0x45ad1e
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {         // 0x45ad2c
			ScriptNativeReportError( amx, "GetDerivedAttribute() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {             // 0x45ad5f
			ScriptNativeReportError( amx,
				"GetDerivedAttribute() - EntityID does not belong to a game creature." );   // NO space
			return 0;
		}
		return CreatureDerivedAttribute( (const GameCreature *)pEntity, (int)p[ 2 ] );
	} // ScriptNative_GetDerivedAttribute

	// 0x409824 / 0x409820 - GameCreature::BaseSkill(int): three instructions, `mov eax,[ecx+eax*4+0E0h]`
	// + `retn 4`. The cls_0x409820 label is the same IDB attribution artifact as GameActionEquip: the
	// `this` is a GameCreature, cast and passed unchanged by GetBaseSkill at 0x45adef/0x45ae13.
	static int CreatureBaseSkill( const GameCreature *pC, int nSkill ) {
		return pC->statBands_0xE0[ nSkill ];   // +0xE0..+0x110 = the 13 skills (0x409824)
	} // CreatureBaseSkill

	// 0x409830 - GameCreature::GetTotalSkill: base + a 13-case attribute formula. Every arm below is
	// the shipped lea/mov sequence with its EA, byte-checked against the jump table rather than the
	// pseudocode. Note the shape: the "weighted attribute" arms always total THREE attribute points
	// (1x + 2x) EXCEPT firearms/melee, which add a flat +50 instead, and science/barter-persuasion
	// which use a single attribute 3x. The default arm returns the BASE UNCHANGED and logs via
	// JE::LogDebugString_0x497120 - that sink is unported (named-deferred, same as the derived
	// default), so the value is faithful and the line is not emitted.
	static int CreatureTotalSkill( const GameCreature *pC, int nSkill ) {
		const int nBase = CreatureBaseSkill( pC, nSkill );   // 0x409837

		switch ( nSkill ) {
			case SNC_SKILL_FIREARMS:                        // 0x40988f: cases 0,1 -> mov esi,50
			case SNC_SKILL_MELEE:
				return nBase + 50;
			case SNC_SKILL_UNARMED:                         // 0x4098f2: [768h] + 2*[760h]
				return pC->endurance_0x768 + 2 * pC->strength_0x760 + nBase;
			case SNC_SKILL_BARTER:                          // 0x409850: cases 3,4 -> 3*[76Ch]
			case SNC_SKILL_PERSUASION:
				return 3 * pC->charisma_0x76c + nBase;
			case SNC_SKILL_DECEPTION:                       // 0x409861: [770h] + 2*[76Ch]
				return pC->intelligence_0x770 + 2 * pC->charisma_0x76c + nBase;
			case SNC_SKILL_MECHANICS:                       // 0x409878: cases 6,7 -> [764h]+2*[770h]
			case SNC_SKILL_MEDIC:
				return pC->perception_0x764 + 2 * pC->intelligence_0x770 + nBase;
			case SNC_SKILL_OUTDOORSMAN:                    // 0x40989c: [768h] + 2*[764h]
				return pC->endurance_0x768 + 2 * pC->perception_0x764 + nBase;
			case SNC_SKILL_SCIENCE:                        // 0x4098b3: 3*[770h]
				return 3 * pC->intelligence_0x770 + nBase;
			case SNC_SKILL_SNEAK:                          // 0x4098c4: cases 10,11 -> [774h]+2*[764h]
			case SNC_SKILL_SECURITY:
				return pC->agility_0x774 + 2 * pC->perception_0x764 + nBase;
			case SNC_SKILL_STEAL:                          // 0x4098db: [764h] + 2*[774h]
				return pC->perception_0x764 + 2 * pC->agility_0x774 + nBase;
			default:
				return nBase;   // 0x409917: base unchanged (plus the unported debug log)
		}
	} // CreatureTotalSkill

	// 0x45ab50 - GetPerkRank. SHIPPED COPY-PASTE PRESERVED VERBATIM: its resolve-failure line is
	// "GetPosition() was unable to find the requested entity." (0x45ab8e) - a different native's name
	// in GetPerkRank's message. T-022 says byte-identical, so it stays wrong exactly as shipped, and
	// the suite asserts it so nobody "corrects" it into a divergence.
	int ScriptNative_GetPerkRank( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "GetPerkRank" ) ) {   // 0x45ab6e
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) { // 0x45ab7c
			ScriptNativeReportError( amx, "GetPosition() was unable to find the requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {     // 0x45abaf
			ScriptNativeReportError( amx,
				"GetPerkRank() - Entity ID does not belong to a game creature." );   // WITH space
			return 0;
		}
		const GameCreature *pC = (const GameCreature *)pEntity;
		// 0x45abd2 indexes perks off +0x114, which in the port's merged band is offset 13. The
		// shipped code has NO bounds test, and because the bands are adjacent an out-of-range PERK
		// index reads whatever follows +0x148 in the creature - unmodeled past the merged array, and
		// stated rather than papered over.
		return pC->statBands_0xE0[ 13 + (int)p[ 2 ] ];
	} // ScriptNative_GetPerkRank

	// 0x45ad90 - GetBaseSkill: same preamble, "EntityID" WITHOUT the space, then the three-instruction
	// accessor at 0x409824.
	int ScriptNative_GetBaseSkill( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "GetBaseSkill" ) ) {   // 0x45adae
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {  // 0x45adbc
			ScriptNativeReportError( amx, "GetBaseSkill() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {      // 0x45adef
			ScriptNativeReportError( amx,
				"GetBaseSkill() - EntityID does not belong to a game creature." );   // NO space
			return 0;
		}
		return CreatureBaseSkill( (const GameCreature *)pEntity, (int)p[ 2 ] );
	} // ScriptNative_GetBaseSkill

	// 0x45ae20 - GetTotalSkill: the third sibling, "EntityID" without the space, delegating to the
	// 13-case formula above.
	int ScriptNative_GetTotalSkill( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "GetTotalSkill" ) ) {   // 0x45ae3e
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {  // 0x45ae4c
			ScriptNativeReportError( amx, "GetTotalSkill() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {      // 0x45ae7f
			ScriptNativeReportError( amx,
				"GetTotalSkill() - EntityID does not belong to a game creature." );   // NO space
			return 0;
		}
		return CreatureTotalSkill( (const GameCreature *)pEntity, (int)p[ 2 ] );
	} // ScriptNative_GetTotalSkill

	// ---- the string-param seam (stands in for sub_59AB20 @ 0x59ab20) --------------------------
	// The shipped fetch is amx_GetAddr (0x5ce350) + amx_StrLen (0x5ce3f0) + a strict len < bufSize +
	// amx_GetString (0x5ce510), called together from 0x59ab20. This seam replaces that data-segment
	// model with a registered C string. Capacity is a port limit, stated because shipped has none:
	// leg 1 validates the CELL OFFSET against the runtime's own bounds (0x5ce36b) and returns error
	// 5, which this seam reproduces as "offset not registered".
	struct ScriptStringParam {
		unsigned nOffset;
		const char *pcValue;
	};
	static ScriptStringParam s_strings[ 16 ] = { { 0, 0 } };
	static int s_nStringCount = 0;

	// Port-side registration for the seam above (0x59ab20 is the shipped consumer path).
	void ScriptNativeRegisterStringParam( unsigned nCellOffset, const char *pcValue ) {
		for ( int i = 0; i < s_nStringCount; ++i ) {
			if ( s_strings[ i ].nOffset == nCellOffset ) {
				s_strings[ i ].pcValue = pcValue;   // last registration wins, like a rewritten cell
				return;
			}
		}
		if ( s_nStringCount < 16 ) {
			s_strings[ s_nStringCount ].nOffset = nCellOffset;
			s_strings[ s_nStringCount ].pcValue = pcValue;
			++s_nStringCount;
		}
	} // ScriptNativeRegisterStringParam

	// The three shipped observables, each of which the suite pins: unknown offset fails; strlen >=
	// bufSize FAILS (0x59ab40's comparison is strict `<`, so a full buffer is a rejection); and pcOut
	// is untouched when the answer is no (legs 1 and 2 short-circuit before leg 3 ever writes).
	char ScriptNativeFetchString( int amx, unsigned nCellOffset, char *pcOut, unsigned nBufSize ) {
		const char *pcValue = 0;

		for ( int i = 0; i < s_nStringCount; ++i ) {
			if ( s_strings[ i ].nOffset == nCellOffset ) {
				pcValue = s_strings[ i ].pcValue;
				break;
			}
		}
		if ( pcValue == 0 ) {
			return 0;   // 0x5CE350's error-5 path: nothing written
		}
		if ( strlen( pcValue ) >= nBufSize ) {
			return 0;   // 0x59ab40: STRICT less-than against the CALLER's capacity
		}
		strcpy( pcOut, pcValue );
		return 1;
	} // ScriptNativeFetchString

	// 0x45b750 - IsItemEquipped. Five distinct failure messages, all shipped-verbatim, and a
	// name-keyed lookup whose real semantics live in the inventory model (case-insensitive match at
	// 0x46a777, then the SIX-slot equipped array at GameInventory+0x14, 0x46a8cc `while (v3 < 6)`).
	int ScriptNative_IsItemEquipped( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;
		char szName[ 128 ];   // shipped: v7[128], fetched with capacity 0x80 at 0x45b7cc

		// Shipped leaves this uninitialized and never reads it unless the fetch succeeded. The port
		// zeroes it because an uninitialized buffer is a warning magnet in both toolchains; the
		// observable is identical, since every failure path returns before any use.
		szName[ 0 ] = 0;

		if ( !ScriptNativeCheckArity( amx, p, 2, "IsItemEquipped" ) ) {   // 0x45b782
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45b79e
			ScriptNativeReportError( amx, "IsItemEquipped() was unable to find the entity." );
			return 0;
		}
		if ( !ScriptNativeFetchString( amx, p[ 2 ], szName, 0x80 ) ) {    // 0x45b7cc
			ScriptNativeReportError( amx, "IsItemEquipped() was unable to find the Item Name." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {       // 0x45b7fe
			ScriptNativeReportError( amx, "IsItemEquipped() - Entity is not a creature" );
			return 0;
		}
		ScriptInventory *pInv = ( (GameCreature *)pEntity )->inventory_0x168;   // 0x45b821 [esi+168h]
		if ( !pInv ) {
			ScriptNativeReportError( amx, "IsItemEquipped() - Inventory not Found" );
			return 0;
		}
		if ( !pInv->bIsGameInventory ) {
			// models the Inventory -> GameInventory RTTI attempt at 0x45b852 failing
			ScriptNativeReportError( amx, "IsItemEquipped() - Game Inventory not Found" );
			return 0;
		}
		return pInv->FindEquipped( szName ) ? 1 : 0;
	} // ScriptNative_IsItemEquipped

	// 0x45a6f0 - UnequipItem. Same preamble order as IsItemEquipped (resolve, fetch, cast) with its own
	// messages, then the slot split at 0x45a81d (`if (v3 != 7)`), which the model carries.
	int ScriptNative_UnequipItem( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;
		char szName[ 128 ];   // shipped: v15[128], capacity 0x80 at 0x45a76e

		szName[ 0 ] = 0;

		if ( !ScriptNativeCheckArity( amx, p, 3, "EquipItem" ) ) {   // 0x45a72c - SHIPPED: wrong name
			return 0;   // ...so this reports EquipItem's arity complaint, as shipped
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45a73e
			ScriptNativeReportError( amx,
				"UnequipItem() was unable to find the attacking entity." );   // 0x45a750, copy-paste
			return 0;
		}
		if ( !ScriptNativeFetchString( amx, p[ 2 ], szName, 0x80 ) ) {    // 0x45a76e
			ScriptNativeReportError( amx, "UnequipItem() was unable to find the Item Name." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {       // 0x45a7a5
			ScriptNativeReportError( amx, "UnequipItem() - Entity is not a creature" );
			return 0;
		}
		ScriptInventory *pInv = ( (GameCreature *)pEntity )->inventory_0x168;   // 0x45a7ca [v13+360]
		if ( !pInv ) {
			ScriptNativeReportError( amx, "UnequipItem() - Inventory not Found" );
			return 0;
		}
		if ( !pInv->bIsGameInventory ) {
			ScriptNativeReportError( amx, "UnequipItem() - Game Inventory not Found" );
			return 0;
		}
		// 0x45a81d: the slot-7 sentinel means "everywhere, case-insensitively"; anything else is one
		// named slot, compared case-SENSITIVELY, with the "*" wildcard clearing but answering 0.
		return pInv->UnequipNamed( szName, (int)p[ 3 ] ) ? 1 : 0;
	} // ScriptNative_UnequipItem

	// 0x45a580 - EquipItem. The preamble is identical to UnequipItem's - including the shipped gate name,
	// which is the ONLY thing that makes the two natives tellable apart before their payloads run.
	int ScriptNative_EquipItem( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;
		char szName[ 128 ];   // shipped: fetched into a 128-byte block, capacity 0x80 at 0x45a5f7

		szName[ 0 ] = 0;

		if ( !ScriptNativeCheckArity( amx, p, 3, "EquipItem" ) ) {   // 0x45a5bc
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45a5ca
			ScriptNativeReportError( amx,
				"EquipItem() was unable to find the attacking entity." );   // 0x45a5dc, Attack's wording
			return 0;
		}
		if ( !ScriptNativeFetchString( amx, p[ 2 ], szName, 0x80 ) ) {     // 0x45a5f7
			ScriptNativeReportError( amx, "EquipItem() was unable to find the Item Name." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {        // 0x45a62c
			ScriptNativeReportError( amx, "EquipItem() - Entity is not a creature" );
			return 0;
		}
		ScriptInventory *pInv = ( (GameCreature *)pEntity )->inventory_0x168;   // 0x45a668, pre-cast
		if ( !pInv ) {
			ScriptNativeReportError( amx, "EquipItem() - Inventory not Found" );
			return 0;
		}
		if ( !pInv->bIsGameInventory ) {
			ScriptNativeReportError( amx, "EquipItem() - Game Inventory not Found" );
			return 0;
		}
		// 0x45a6bb: vft slot 6 (0x55fda0) - case-INSENSITIVE name walk. A miss prints NOTHING and
		// answers 0; that silence is shipped behavior, unlike the four messages above.
		const ScriptInventory::Entry *pItem = pInv->FindByNameCI( szName );
		if ( !pItem ) {
			return 0;
		}
		// 0x46be8b: Equip asks slot 10 FIRST and refuses an already-equipped item, logging
		// "GameInventory::Equip() - Item that was already equipped trying to equip itself again."
		// through JE::LogDebugString_0x497120 (0x46be97) - that sink is unported, so the VALUE (0) is
		// kept and the line is not emitted, named-deferred like the packet-88 broadcast and the derived
		// default case. The observable that matters: re-equipping an equipped item FAILS with 0.
		if ( pInv->SlotOfItem( pItem->pItem ) >= 0 ) {
			return 0;
		}
		// 0x45a6ce: vft slot 14 (GameInventory::Equip 0x46be40), args (item, params[3] slot, 1).
		// THREE named deviations in the ported placement, because Equip's body is an era:
		//   (1) SHIPPED REMAPS THE SLOT. Cases 2/3/7 of its style switch (0x46bff4-0x46c051) translate
		//       the requested slot through the creature's weapon style at +0x98, so "equip into 7" can
		//       land in 2 or 3. The port places into the requested slot, bounded to the six cells.
		//   (2) SHIPPED RELOCATES THE DISPLACED ITEM (the unbounded cell read at 0x4693d4 plus the
		//       RB-tree bookkeeping around it). The port overwrites an occupied cell; the old item
		//       simply leaves the equipped set.
		//   (3) SHIPPED RUNS SIDE EFFECTS: ApplyEquipEffects gated on the server-context bit
		//       (0x46c1a1), a SetWeaponStance(0) reset (0x46c1e9), and meth_0x469cf0 accounting
		//       (0x46c1bc). The port performs NONE of them - so an equip here changes slot state only,
		//       which is a real behavioral difference for any later combat/stat parity work.
		// Declared rather than minimized: the return value and the slot write are what this native
		// observably does to inventory state, and everything else is named with its EA.
		const int nSlot = (int)p[ 3 ];
		if ( nSlot < 0 || nSlot >= ScriptInventory::SNCCI_EQUIPPED_SLOTS ) {
			return 0;   // port bound; see deviation (1) - shipped remaps or inspects past the cells
		}
		pInv->Equip( nSlot, pItem->pItem );
		return 1;
	} // ScriptNative_EquipItem

	// ---- the party seam ---------------------------------------------------------------------
	// Stands in for the 0x70bd98 singleton plus its slot-26 Party accessor. Port global, matching the
	// file's existing registry pattern rather than inventing a second mechanism.
	static ScriptParty *s_pActiveParty = 0;

	void ScriptNativeSetActiveParty( ScriptParty *pParty ) {
		s_pActiveParty = pParty;
	} // ScriptNativeSetActiveParty

	class ScriptParty *ScriptNativeGetActiveParty() {
		return s_pActiveParty;
	} // ScriptNativeGetActiveParty

	// 0x45a960 - Protagonist: arity 0, no entity argument - the ONLY batch-B native that returns a
	// handle it did not resolve from a parameter. On success it returns the leader's own +0x1DC
	// (bytes 8b 80 dc 01 00 00 at 0x45a9e0), which is the third era of the same field: Create stamps
	// it, SetHP resolves through it, Protagonist RETURNS it.
	//
	// Both failure lines say "Returning INVALID_ID." and return 0 - not -1, not any sentinel the
	// message implies. Kept exactly, because a script that tests the documented contract gets shipped
	// behavior and a script that tests the message gets a surprise; the mismatch is the engine's, and
	// a port that "returns INVALID_ID properly" would be a silent divergence in the other direction.
	int ScriptNative_Protagonist( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;

		if ( !ScriptNativeCheckArity( amx, p, 0, "Protagonist" ) ) {   // 0x45a97c
			return 0;
		}
		// 0x45a985-0x45a9a5: shipped builds the party through the singleton and dereferences it with
		// NO null test on the adjustment result, so a missing singleton is a crash. The port answers
		// the cast-failure message instead - declared, not hidden, because it collapses two shipped
		// states (no singleton / singleton that is not a GameParty) into one port state.
		if ( s_pActiveParty == 0 || !s_pActiveParty->bIsGameParty ) {
			ScriptNativeReportError( amx,
				"Protagonist() could not find GameParty. Returning INVALID_ID." );
			return 0;
		}
		if ( s_pActiveParty->pLeader == 0 ) {
			ScriptNativeReportError( amx,
				"Protagonist() Protagonist is NULL. Returning INVALID_ID." );
			return 0;
		}
		// 0x45a9c7 reads the leader at GameParty+0x14, 0x45a9e0 the handle at Entity+0x1DC of it.
		return (int)s_pActiveParty->pLeader->mbr_0x1dc;
	} // ScriptNative_Protagonist

	// 0x45a2d0 - AddCNPCToParty: arity 1, resolve (message says "entity", not "attacking"), then the
	// same party access, then GameParty VFT SLOT 0 with the entity as the pushed argument (0x45a370:
	// mov edx,[eax]; push entity; mov ecx,eax; call [edx]). The native's return value IS that call's
	// return, so whatever policy the model carries becomes script-observable - which is why ScriptParty
	// says plainly that "already a member" and "no room" are the only answers this call site proves.
	int ScriptNative_AddCNPCToParty( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;

		if ( !ScriptNativeCheckArity( amx, p, 1, "AddCNPCToParty" ) ) {   // 0x45a2ee
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45a2fc
			ScriptNativeReportError( amx, "AddCNPCToParty() was unable to find the entity." );
			return 0;
		}
		if ( s_pActiveParty == 0 || !s_pActiveParty->bIsGameParty ) {     // 0x45a323-0x45a351
			ScriptNativeReportError( amx, "AddCNPCToParty() was unable to find the party." );
			return 0;
		}
		return s_pActiveParty->Add( pEntity );
	} // ScriptNative_AddCNPCToParty

	// 0x45b620 - GetWeaponRange. Legs in shipped order; each cite is the instruction, not the print.
	int ScriptNative_GetWeaponRange( int amx, int params ) {
		const unsigned *p = (const unsigned *)params;
		JE::Entity *pEntity = 0;
		char szUnused[ 1 ];   // no string is fetched here - arity 1, handle only

		(void)szUnused;
		if ( !ScriptNativeCheckArity( amx, p, 1, "GetWeaponRange" ) ) {   // 0x45b63e
			return 0;
		}
		if ( !ScriptNativeResolveEntity( amx, p[ 1 ], &pEntity, 1 ) ) {   // 0x45b654
			ScriptNativeReportError( amx, "GetWeaponRange() - Unable to resolve requested entity." );
			return 0;
		}
		if ( !RegistryIsClass( pEntity, F3::SNC_GAME_CREATURE ) ) {        // 0x45b68d
			ScriptNativeReportError( amx, "GetWeaponRange() - Entity is not a creature" );
			return 0;
		}
		GameCreature *pC = (GameCreature *)pEntity;
		ScriptInventory *pInv = pC->inventory_0x168;                       // 0x45b6af, PRE-CAST base
		if ( !pInv ) {
			ScriptNativeReportError( amx, "GetWeaponRange() - Inventory not Found" );
			return 0;
		}
		if ( !pInv->bIsGameInventory ) {
			ScriptNativeReportError( amx, "GetWeaponRange() - Game Inventory not Found" );
			return 0;
		}
		// 0x45b702: slot = creature+0x98 (via the one-instruction getter at 0x463b60), then
		// 0x45b70a: item at that slot (0x4693d0, which RTTIs the cell to GameItem and yields 0 if not).
		const ScriptInventory::Entry *pItem = pInv->ItemAtSlot( pC->weaponSlot_0x98 );
		const int *pRangeRecord = 0;
		if ( pItem ) {
			if ( !pItem->bWeapon ) {
				// SHIPPED: 0x45b727 computes GameItem->GameWeapon and dereferences the result at +0x408
				// with NO test, so a non-weapon in the active slot is a null dereference in the engine.
				// The port answers 0 instead. Named deviation, not a fix: the difference is only visible
				// where shipped would have crashed, and a crash is not behavior worth reproducing.
				return 0;
			}
			pRangeRecord = pItem->pRangeRecord;         // the +0x408 pointer
		} else {
			pRangeRecord = pC->rangeFallback_0x6f4;     // 0x45b732: creature+0x6F4, POST-CAST base
		}
		if ( !pRangeRecord ) {
			return 0;                                    // 0x45b73a: no record, no range
		}
		return pRangeRecord[ 11 ];                       // 0x45b740: `mov eax,[eax+2Ch]' = int index 11
	} // ScriptNative_GetWeaponRange

	// 0x45b240 - GetItemType: cast Entity->GameItem (0x45b2a7), read [obj+0xA0] (0x45b2c7).
	// Shipped cast-failure line has NO trailing period: "GetItemType() - Entity is not an Item".
	int ScriptNative_GetItemType( int amx, int params ) {
		GameItem *pItem = (GameItem *)ResolveNativeEntity( amx, params, 1, "GetItemType",
		                                                   F3::SNC_GAME_ITEM,
		                                                   "GetItemType() - Unable to resolve requested entity.",
		                                                   "GetItemType() - Entity is not an Item" );
		if ( !pItem ) {
			return 0;
		}
		return (int)pItem->mbr_0xa0;   // shipped: mov eax,[eax+0A0h] at 0x45b2c7
	} // ScriptNative_GetItemType

	// 0x45b440 - GetUsableObjectType: cast Entity->GameUsableObject (0x45b4a7), SAME field +0xA0
	// (0x45b4c7). Shipped cast-failure line carries a typo and a grammar slip:
	// "GetUsableObjectType() - Entity is not an usable ubject" - reproduced verbatim (T-022).
	int ScriptNative_GetUsableObjectType( int amx, int params ) {
		GameUsableObject *pUsable = (GameUsableObject *)ResolveNativeEntity( amx, params, 1, "GetUsableObjectType",
		                                                                     F3::SNC_GAME_USABLE,
		                                                                     "GetUsableObjectType() - Unable to resolve requested entity.",
		                                                                     "GetUsableObjectType() - Entity is not an usable ubject" );
		if ( !pUsable ) {
			return 0;
		}
		return (int)pUsable->mbr_0xa0;   // shipped: mov eax,[eax+0A0h] at 0x45b4c7
	} // ScriptNative_GetUsableObjectType

	// 0x45b570 - GetLockDC: the DUAL-CAST native. Shipped tries Entity->GameContainer (0x45b5d3) and,
	// only if that fails, Entity->GameDoor (0x45b5fc), reading +0xC0 in either case (0x45b5dd/0x45b5e8) -
	// one field on their common base GameUsableObject, which is why two casts do one job. The port models
	// that as a single accept-set (CONTAINER|DOOR) rather than two ordered casts, and the third failure
	// line names BOTH alternatives: "GetLockDC() entity passed in was neither a container nor a door."
	// Note also its resolve-failure wording differs from the rest of the batch: "was unable to find the
	// entity." (like GetPerkRank/IsItemEquipped) rather than "Unable to resolve requested entity.".
	int ScriptNative_GetLockDC( int amx, int params ) {
		GameUsableObject *pUsable = (GameUsableObject *)ResolveNativeEntity( amx, params, 1, "GetLockDC",
		                                                             F3::SNC_GAME_CONTAINER | F3::SNC_GAME_DOOR,
		                                                             "GetLockDC() was unable to find the entity.",
		                                                             "GetLockDC() entity passed in was neither a container nor a door." );
		if ( !pUsable ) {
			return 0;
		}
		// Read through the OWNER type, not the accepted one, and this is not style: GameContainer.hpp:16
		// RE-DECLARES mbr_0xc0 although GameUsableObject.hpp:15 already owns it, and per item 69 each
		// generated header is a name-sorted excerpt - so the two declarations sit at DIFFERENT port
		// offsets and a container read via GameContainer* would disagree with a door read via its base.
		// Shipped has one slot at one address (+0xC0); naming the owner is how the port keeps that true.
		// Shipped BYTES at 0x45b5dd are `8b 80 c0 00 00 00` = mov eax,[eax+0C0h]. The pseudocode
		// printed `*(v3 + 48)` for BOTH return sites, which happens to be the same value here only
		// because the pointer it guessed was 4-byte typed - a cell-vs-byte rendering that could
		// just as easily have been wrong. Rule now in force in this file: offsets are adjudicated
		// against the instruction text, never against the decompiler's number (item 69's sibling).
		return (int)pUsable->mbr_0xc0;   // shipped: mov eax,[eax+0C0h] (bytes 8b 80 c0 00 00 00 @0x45b5dd)
	} // ScriptNative_GetLockDC

	// The {name, fn, expected cells} table in shipped EA order. NOT wired into any VM - the port
	// has no amx module table yet, and faking one would be inventing the thing being reversed.
	static const ScriptNativeEntry s_gameCreatureNatives[36] = {
		{ "GetLevel",    &ScriptNative_GetLevel,    1 },   // 0x45abe0
		{ "GetRace",     &ScriptNative_GetRace,     1 },   // 0x45aeb0
		{ "GetGender",   &ScriptNative_GetGender,   1 },   // 0x45af40
		{ "GetEthnicity",&ScriptNative_GetEthnicity,1 },   // 0x45afd0
		{ "GetSubRace",  &ScriptNative_GetSubRace,  1 },   // 0x45b070
		{ "GetCurrentHP",&ScriptNative_GetCurrentHP,1 },   // 0x45b1b0
		{ "GetMaxHP",    &ScriptNative_GetMaxHP,    1 },   // 0x45b110
		{ "GetItemType", &ScriptNative_GetItemType, 1 },   // 0x45b240
		{ "GetUsableObjectType", &ScriptNative_GetUsableObjectType, 1 }, // 0x45b440
		{ "GetLockDC",   &ScriptNative_GetLockDC,   1 },   // 0x45b570
		{ "PlayMusic",   &ScriptNative_PlayMusic,   1 },   // 0x45b9c0 - shipped stub, ported as a stub
		{ "SetHP",       &ScriptNative_SetHP,       2 },   // 0x45b2e0
		{ "AddHP",       &ScriptNative_AddHP,       2 },   // 0x45b390
		{ "LockDoor",    &ScriptNative_LockDoor,    1 },   // 0x45a1b0
		{ "UnlockDoor",  &ScriptNative_UnlockDoor,  1 },   // 0x45a240
		{ "IsLocked",    &ScriptNative_IsLocked,    1 },   // 0x45b4e0
		{ "GetPrimaryAttribute", &ScriptNative_GetPrimaryAttribute, 2 },   // 0x45ac70
		{ "GetDerivedAttribute", &ScriptNative_GetDerivedAttribute, 2 },   // 0x45ad00
		{ "GetPerkRank",  &ScriptNative_GetPerkRank,   2 },   // 0x45ab50
		{ "GetBaseSkill", &ScriptNative_GetBaseSkill,  2 },   // 0x45ad90
		{ "GetTotalSkill", &ScriptNative_GetTotalSkill, 2 },   // 0x45ae20
		{ "IsItemEquipped", &ScriptNative_IsItemEquipped, 2 },   // 0x45b750
		{ "UnequipItem", &ScriptNative_UnequipItem, 3 },         // 0x45a6f0
		{ "GetWeaponRange", &ScriptNative_GetWeaponRange, 1 },   // 0x45b620
		{ "EquipItem", &ScriptNative_EquipItem, 3 },             // 0x45a580
		{ "Protagonist", &ScriptNative_Protagonist, 0 },         // 0x45a960
		{ "AddCNPCToParty", &ScriptNative_AddCNPCToParty, 1 },   // 0x45a2d0
		{ "OpenTask",  &ScriptNative_OpenTask,  2 },   // 0x45b900 - journal era NAMED-DEFERRED, returns 1
		{ "CloseTask", &ScriptNative_CloseTask, 2 },   // 0x45b940
		{ "AddEntry",  &ScriptNative_AddEntry,  2 },   // 0x45b980
		{ "GetWeaponCombo", &ScriptNative_GetWeaponCombo, 1 },   // 0x45a9f0 - shipped stub: resolves, discards, returns 0
		{ "SetWeaponCombo", &ScriptNative_SetWeaponCombo, 2 },   // 0x45aa40 - shipped stub: the 2nd cell is never read
		// The alias quad: four shipped NAMES, one shared FUNCTION (0x420990). -1 = no arity gate exists.
		{ "OpenQuest",       &ScriptNative_QuestJournalAlias, -1 },   // table idx 41 -> 0x420990
		{ "CloseQuest",      &ScriptNative_QuestJournalAlias, -1 },   // table idx 42 -> 0x420990
		{ "AddQuestEntry",   &ScriptNative_QuestJournalAlias, -1 },   // table idx 43 -> 0x420990
		{ "AddJournalEntry", &ScriptNative_QuestJournalAlias, -1 }    // table idx 44 -> 0x420990
	};

	const ScriptNativeEntry *ScriptNativeGameCreatureTable( int *pnCount ) {
		if ( pnCount ) {
			*pnCount = (int)( sizeof( s_gameCreatureNatives ) / sizeof( s_gameCreatureNatives[0] ) );
		}
		return s_gameCreatureNatives;
	} // ScriptNativeGameCreatureTable

} // namespace F3

// Copyright 2024 OpenJE
// F3/GameWeapon.cpp - batch C GameWeapon shard, first slice: the Deduct ammo pair.
// Every constant and offset below was re-emitted from instruction text of the shipped
// image (artifacts/F3.exe.i64), not from decompiler rendering - see docs/f3/
// NativeAbiEvidence.md for the rendering-hazard history this convention exists because of.

#include "F3/GameWeapon.hpp"
#include "JE.hpp"            // JE::LogDebugString - the shipped report sink
#include <string.h>          // strlen for the capture seam below

namespace F3 {
	static WeaponAmmoSync   s_sync;        // 0-initialized: NULL seam skips the sync (port-defensive, named in the header)

	// LOG CAPTURE SEAM - test observability only, same precedent as ScriptNatives.cpp's
	// s_szLastError. The shipped reports go to JE::LogDebugString_0x497120; this TU cannot
	// override the global sink its suite links, so the formatted string is ALSO copied here
	// (the real call is untouched, behavior unchanged). Last-one-wins is deliberate here:
	// unlike the arity gate, neither Deduct body can emit two messages in one call.
	static char s_szLastLog[ 256 ];

	const char *GameWeaponGetLastLog() { return s_szLastLog; }
	void        GameWeaponResetLastLog() { s_szLastLog[ 0 ] = 0; }

	static void ReportDeduct( const char *pcMsg ) {
		static char szPassThrough[] = "%s";     // shipped sink takes non-const char* (JE.hpp), as in ScriptNatives
		JE::LogDebugString( (char *)0, szPassThrough, (char *)pcMsg );
		strncpy( s_szLastLog, pcMsg, sizeof( s_szLastLog ) - 1 );
		s_szLastLog[ sizeof( s_szLastLog ) - 1 ] = 0;
	}

	// 0x412b10. Bytes, for the record, because the quirks are the port:
	//   412b13  mov  eax,[esi+3ECh]      ; old count
	//   412b1e  cmp  edi,eax
	//   412b20  jbe  short loc_412B2F    ; over-draw skips ONLY the log - the subtract runs anyway
	//   412b22  push offset "GameWeapon::DeductPrimaryAmmo() - Attempt to deduct more ammo than the weapon had.\n"
	//   412b27  call JE__LogDebugString_0x497120
	//   412b2f  sub  [esi+3ECh],edi      ; the SUB itself sets ZF for the flag test below
	//   412b35  jnz  short loc_412B41    ; flag survives every non-zero result, INCLUDING the wrap
	//   412b37  mov  dword ptr [esi+424h],0
	//   412b41.. server-handle chain: [esi+4] delta base -> [edx+esi+1E0h]; RTTI probe @0x412b58
	//            result pushed then DISCARDED; tail @0x412b7e =
	//            GameServer::meth_0x46d700( handle, [esi+3B8h], [esi+3ECh] )  ; new count
	void GameWeaponDeductPrimaryAmmo( GameWeapon &w, unsigned int nAmount ) {
		if ( nAmount > w.mbr_0x3ec ) {
			ReportDeduct( "GameWeapon::DeductPrimaryAmmo() - Attempt to deduct more ammo than the weapon had.\n" );
		}
		w.mbr_0x3ec -= nAmount;                       // 0x412b2f: unconditional; wrap is shipped behavior
		if ( w.mbr_0x3ec == 0 ) {
			w.mbr_0x424 = 0;                          // 0x412b37, gated on the SUB's ZF (0x412b35)
		}
		if ( s_sync.pfnPrimary ) {
			s_sync.pfnPrimary( w.mbr_0x3b8, w.mbr_0x3ec );
		}
	} // GameWeaponDeductPrimaryAmmo

	// 0x412b90 - the secondary mirror, verified against its own instruction text (3F4 count,
	// 428 flag, 3C4 id, meth_0x46d7f0 tail), NOT by substitution into the primary. Same quirks,
	// byte-for-byte parallel: 412ba0-era jbe skips only the log; `sub` ZF gates the 428 clear;
	// sync fires with the post-decrement count either way.
	void GameWeaponDeductSecondaryAmmo( GameWeapon &w, unsigned int nAmount ) {
		if ( nAmount > w.mbr_0x3f4 ) {
			ReportDeduct( "GameWeapon::DeductSecondaryAmmo() - Attempt to deduct more ammo than the weapon had.\n" );
		}
		w.mbr_0x3f4 -= nAmount;
		if ( w.mbr_0x3f4 == 0 ) {
			w.mbr_0x428 = 0;
		}
		if ( s_sync.pfnSecondary ) {
			s_sync.pfnSecondary( w.mbr_0x3c4, w.mbr_0x3f4 );
		}
	} // GameWeaponDeductSecondaryAmmo

	void GameWeaponSetAmmoSync( const WeaponAmmoSync &sync ) { s_sync = sync; }

	// ---- batch C slice 2: GWAM record + readers --------------------------------------
	// Map authority: docs/f3/GWAMRecordMap.md (var-axis table, verifier base-pair
	// construction notes, reconciliation section). Every stream EA below is from the
	// windows walked in-thread; sizes are lea+push facts, NOT frame-table size hints
	// (three display-vs-identity instances this thread - see the map doc's rule block).

	static GWAMAppendFn s_gwamAppend = 0;   // NULL seam = skip append (PORT-DEFENSIVE,
	                                       // named: shipped always has a live list)

	void GameWeaponSetGWAMAppend( GWAMAppendFn fn ) { s_gwamAppend = fn; }

	// metal idiom: EVERY MemStreamRead in both readers is test al,al + fail-exit;
	// first-fail semantics, nothing appends on any failure path.
	static int GwamRead4( JE::ResourceSystem::MemStream *ms, unsigned int *pv ) {
		return JE::ResourceSystem::MemStreamRead_0x498170( pv, 4u, ms ) ? 1 : 0;
	}
	static int GwamRead1( JE::ResourceSystem::MemStream *ms, unsigned char *pv ) {
		return JE::ResourceSystem::MemStreamRead_0x498170( pv, 1u, ms ) ? 1 : 0;
	}

	// the certified 34-case ladder (both jump tables read from the exe; 31/31 equal
	// imm + case1 pinned-56-BY-INIT - the dead 62 arm is NOT ported, per the boundary
	// law: authorial intent informs the story, never the code). cases 3,5 write nothing.
	static const unsigned int s_gwamAnimLadder[ 35 ] = {
		0, 56, 59, 0, 61, 0, 60, 63, 64, 65,
		53, 54, 55, 83, 84, 85, 86, 87, 88, 89,
		90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
		100, 101, 102, 103, 104
	};

	int GameWeaponReadGwamRecordCore( JE::ResourceSystem::MemStream *ms, GWAMRecord &rec, unsigned int nGateVersion ) {
		unsigned int raw;
		unsigned char byte;

		rec.cells[ 0 ] = 3;                 // type init (creature @0x40dc0d / weapon local_78 era); readers pin 3,
		                                    // the WEAPON derivation may overwrite it after the core returns
		rec.counter = 0; rec.kind = 0; rec.ext44 = 0;
		rec.pcText = 0;
		rec.bytes[ 0 ] = 0; rec.bytes[ 1 ] = 0; rec.bytes[ 2 ] = 0; rec.bytes[ 3 ] = 0;
		rec.tail[ 0 ] = 0; rec.tail[ 1 ] = 0; rec.tail[ 2 ] = 0;
		                                    // zero-init of gated fields: PORT CHOICE - shipped leaves them
		                                    // stack survivors when the gate is closed (+0x10 ALWAYS; +0x39/
		                                    // 3A/3B + extension + >=2..5 fields when version<1). The suite
		                                    // distinguishes filled vs survivor; survivors must NOT be
		                                    // presented as stream data. Cells 3/4 and 15..17 keep 0 only
		                                    // because the port cannot mirror a stack address.
		rec.cells[ 3 ] = 0;                 // +0x0C: gated reach-back target - zero stand-in
		rec.cells[ 4 ] = 0;                 // +0x10: NEVER WRITTEN by any shipped reader (gated reach-back
		                                    // pushes exactly 4 into cells[3]); zero here stands in for
		                                    // "survivor", documented, never claimed as stream content.

		// unconditional 11-u32 run: +0x04,+0x08, step over +0x0C/+0x10, +0x14..+0x34
		if ( !GwamRead4( ms, &rec.cells[ 1 ] ) ) return 0;   // r1  @0x40dc01 / 0x414907
		if ( !GwamRead4( ms, &rec.cells[ 2 ] ) ) return 0;   // r2  @0x40dc42 / 0x414939
		if ( !GwamRead4( ms, &rec.cells[ 5 ] ) ) return 0;   // r3  @0x40dc6e / 0x41496x (first after step-over)
		if ( !GwamRead4( ms, &rec.cells[ 6 ] ) ) return 0;   // r4  @0x40dca6
		if ( !GwamRead4( ms, &rec.cells[ 7 ] ) ) return 0;   // r5  @0x40dcbb
		if ( !GwamRead4( ms, &rec.cells[ 8 ] ) ) return 0;   // r6  @0x40dcdc
		if ( !GwamRead4( ms, &rec.cells[ 9 ] ) ) return 0;   // r7  @0x40dcf7
		if ( !GwamRead4( ms, &rec.cells[ 10 ] ) ) return 0;  // r8  @0x40dd0c
		if ( !GwamRead4( ms, &rec.cells[ 11 ] ) ) return 0;  // r9  @0x40dd2d
		if ( !GwamRead4( ms, &rec.cells[ 12 ] ) ) return 0;  // r10 @0x40dd48
		if ( !GwamRead4( ms, &rec.cells[ 13 ] ) ) return 0;  // r11 +0x34 u32 UNCONDITIONAL @0x40dd5d
		                                    // (verifier's "gated" retracted: the jz she read is the
		                                    // stream-FAIL exit; version gate comes after)
		if ( !GwamRead1( ms, &byte ) ) return 0;             // @0x40dd86 u8 -> NORMALIZED store:
		rec.bytes[ 0 ] = (unsigned char)( byte != 0 );       // cmp/test+setnz chain (@0x40dd93-a0 creature,
		                                    // @0x414a91-99 weapon) - presence flag, NOT stream verbatim
		if ( nGateVersion == 0 ) goto ladder;               // gate AFTER the unconditional fills
		                                    // (cmp edi,1-era; creature jb @0x40dda4, weapon @0x414a9d)
		if ( !GwamRead4( ms, &rec.cells[ 3 ] ) ) return 0;   // extension reach-back +0x0C @0x40ddad
		if ( !GwamRead1( ms, &byte ) ) return 0;             // +0x39 CLAIMED flag  (v21 chain)
		rec.bytes[ 1 ] = (unsigned char)( byte != 0 );
		if ( !GwamRead4( ms, &rec.counter ) ) return 0;      // +0x3C counter cell (v24)
		if ( !GwamRead1( ms, &byte ) ) return 0;             // +0x3A  (v22)
		rec.bytes[ 2 ] = (unsigned char)( byte != 0 );
		if ( !GwamRead1( ms, &byte ) ) return 0;             // +0x3B  (v23)
		rec.bytes[ 3 ] = (unsigned char)( byte != 0 );
		if ( rec.bytes[ 2 ] && rec.bytes[ 3 ] ) {
			// @0x40de73 / @0x414b74. JE::FatalError (JE.cpp:15) models the shipped abort:
			// MessageBox + exit(1) - this call does NOT return in product or harness. The
			// return 0 below is compiler hygiene AFTER an unreachable point; the no-append
			// invariant holds because the process dies first. NOT EXERCISABLE IN-SUITE
			// (MessageBox would block under xvfb): certified by construction - 3-line arm,
			// verbatim string, both call EAs pinned. Suites must never reach this branch.
			JE::FatalError( "GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown." );
			return 0;
		}
		if ( nGateVersion >= 2 ) { if ( !GwamRead4( ms, &rec.kind ) ) return 0; }        // >=2 v25 @0x40..-era
		if ( nGateVersion >= 3 ) { if ( !GwamRead4( ms, &rec.ext44 ) ) return 0; }       // >=3 v26
		if ( nGateVersion >= 4 ) {                                                        // >=4 v27 string
			if ( !JE::ResourceSystem::StreamReadString_0x49c8d0( &rec.pcText, ms ) ) return 0;
		}
		if ( nGateVersion >= 5 ) {                                                        // >=5 two u8s
			if ( !GwamRead1( ms, &rec.tail[ 0 ] ) ) return 0;
			if ( !GwamRead1( ms, &rec.tail[ 1 ] ) ) return 0;
		}
	ladder:
		if ( rec.kind == 0 && rec.cells[ 1 ] >= 1 && rec.cells[ 1 ] <= 34 ) {
			if ( s_gwamAnimLadder[ rec.cells[ 1 ] ] != 0 || rec.cells[ 1 ] == 1 )
				rec.kind = s_gwamAnimLadder[ rec.cells[ 1 ] ];   // cases 3,5 leave 0 (no write @metal)
		}
		return 1;
	} // GameWeaponReadGwamRecordCore

	// WEAPON leg thin wrapper (0x4148a0): gate = CHUNK version [ebx+0Ch]; type
	// derivation = the weapon-only block @0x414c18-5c: byte[3] (metal local_3d, role
	// unnamed pair-member) -> 2, else item-type switch on w+0x3B0 (metal [ecx+3B0h]):
	// 1-5 -> 1, 6-8 -> 0, 9-10 -> edi(=3 pre-set); others leave the init 3.
	// append TARGET = weapon's own container at w+0x40C (owner-trio certified cell).
	int GameWeaponReadAttackModeData_Weapon( JE::ResourceSystem::MemStream *ms, unsigned int nChunkVersion,
	                                         unsigned int nWeaponItemType, void *pWeaponList ) {
		GWAMRecord rec;
		if ( !GameWeaponReadGwamRecordCore( ms, rec, nChunkVersion ) ) return 0;
		if ( rec.bytes[ 3 ] ) {
			rec.cells[ 0 ] = 2;
		} else if ( nWeaponItemType >= 1 && nWeaponItemType <= 5 ) {
			rec.cells[ 0 ] = 1;
		} else if ( nWeaponItemType >= 6 && nWeaponItemType <= 8 ) {
			rec.cells[ 0 ] = 0;
		} else if ( nWeaponItemType == 9 || nWeaponItemType == 10 ) {
			rec.cells[ 0 ] = 3;
		}                                        // else: stays at the core's init (shipped switch
		                                         // default jumps past the writes, ladder-era @0x414c41)
		if ( s_gwamAppend ) s_gwamAppend( pWeaponList, rec );
		return 1;
	} // GameWeaponReadAttackModeData_Weapon

	// CREATURE leg thin wrapper (0x40dba0 body; its WIRING is deliberately unowned -
	// this is the tested-twice half, not a GameCreature::ReadData port): gate = the
	// RECORD item-version byte 0x30 supplied by the caller; type stays pinned at 3
	// (no derivation block in this body - certified absence); append target per owner
	// (owner+0x6F8 when the creature seat exists).
	int GameWeaponReadAttackModeData_Creature( JE::ResourceSystem::MemStream *ms, unsigned int nRecordItemVersion, void *pCreatureList ) {
		GWAMRecord rec;
		if ( !GameWeaponReadGwamRecordCore( ms, rec, nRecordItemVersion ) ) return 0;
		if ( s_gwamAppend ) s_gwamAppend( pCreatureList, rec );
		return 1;
	} // GameWeaponReadAttackModeData_Creature

	// GameItem prefix reader - sub_412CD0 ported file-static per the no-blocker
	// ruling (13-u32 + string walk over BYTES 0x3B0..0x404; first read
	// [esi+3B0h] @0x412ce1; string at [3CC] via sub_49C8D0 = the metal's own cell
	// list; promotion to a shared GameItem method stays a QUESTION for a seat
	// that exists). NO comment-swallowed legs this time: one statement per line,
	// offsets as trailing block comments - a heredoc-era same-line // comment ate
	// pw+1 and pw+4 silently (they "worked" in the sense of compiling and in no
	// other sense), the first in-repo instance of comment-as-landmine on my own
	// code: a // region may not contain a second statement.
	// GameItem prefix reader - sub_412CD0 ported file-static per the no-blocker
	// ruling (13-u32 + string walk; first read [esi+3B0h] @0x412ce1; string cell
	// via sub_49C8D0; promotion to a shared GameItem method stays a QUESTION).
	// NAMED MEMBERS ONLY: the hpp class is a DB ledger, not byte-faithful layout
	// (probe: mbr_0x3b0 at actual 0x568; sizeof 1504 vs DB 2056) - raw
	// (char*)&w+off arithmetic silently mixes the two coordinate systems and was
	// today's real crash cause. Save-compat will require explicit padding; until
	// then one coordinate system per port, per the layout law.
	static int GwamItemPrefixRead( JE::ResourceSystem::MemStream *ms, GameWeapon &w ) {
		if ( !GwamRead4( ms, &w.mbr_0x3b0 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3b4 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3bc ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3c0 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3c8 ) ) return 0;
		if ( !JE::ResourceSystem::StreamReadString_0x49c8d0( (char **)&w.mbr_0x3cc, ms ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3e8 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3ec ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3f0 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3f4 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3f8 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x3fc ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x400 ) ) return 0;
		if ( !GwamRead4( ms, &w.mbr_0x404 ) ) return 0;   /* record count - caller's */
		return 1;
	} // GwamItemPrefixRead

	// ReadData 0x415190 (sole caller was the file-read thunk 0x415660): prefix ->
	// version-gated ammo-id pair -> count -> GWAM chunk loop. firstChunk = the
	// already-Read chunk header the file path parsed (its mbr_0xc gates the pair,
	// metal v4 = *(param_1+12) @0x4151b3).
	int GameWeaponReadData( JE::ResourceSystem::MemStream *ms, GameWeapon &w, const JE::Common::Chunk &firstChunk ) {
		if ( !GwamItemPrefixRead( ms, w ) ) return 0;
		if ( firstChunk.mbr_0xc != 0 ) {
			if ( !GwamRead4( ms, &w.mbr_0x3b8 ) ) return 0;   // primary modifier (the Deduct sync payload!)
			if ( !GwamRead4( ms, &w.mbr_0x3c4 ) ) return 0;   // secondary id    (same fields slice 1 reads)
		}
		if ( w.mbr_0x404 == 0 ) return 1;                      // metal: !*(this+1028) -> return 1 (not an error)
		// metal reloads the count cell EVERY iteration (@0x415254 mov eax,[esi+404h] after
		// SkipToEnd, inside the loop) - mirrored, not cached: the shipped reload says the
		// bound is live memory.
		for ( unsigned int i = 0; i < w.mbr_0x404; ++i ) {
			JE::Common::Chunk chunk;
			if ( !chunk.Read( ms ) ) return 0;
			if ( chunk.mbr_0x8 != 0x4D415747u ) {              // 'GWAM' (metal cmp @0x415238)
				// metal @0x415285: SINGLE-push call of sub_497120 with the message -
				// the port's a1 IS the format operand (JE.cpp formats from a1), so the
				// faithful two-param call passes the message as both.
				JE::LogDebugString( (char *)"GameWeapon::ReadData() - Unexpected chunk ID. Attack Mode ID 'GWAM' expected.\n",
				                    (char *)"GameWeapon::ReadData() - Unexpected chunk ID. Attack Mode ID 'GWAM' expected.\n" );
				return 0;
			}
			if ( !GameWeaponReadAttackModeData_Weapon( ms, chunk.mbr_0xc, w.mbr_0x3b0, (void *)&w.mbr_0x40c ) ) return 0;
			// shipped derives item type from [weapon+0x3B0] at call time - the prefix read
			// just filled it, so reading it here is the metal's own ordering. Append target
			// is the owner-trio CONTAINER cell, by NAME.
			chunk.SkipToEnd();
		}
		return 1;	} // GameWeaponReadData

} // namespace F3

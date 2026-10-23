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

} // namespace F3

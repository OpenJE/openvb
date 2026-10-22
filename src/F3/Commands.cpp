// Copyright 2024 OpenJE
// R-USERRE-A slice 1 implementation - camera console commands. See Commands.hpp for the
// chain (world=[0x70BD90] -> client +0x2C -> camera +0x84) and the argv link-cell convention.
// Every def cites its shipped EA (item 57); unported callee objects are named with their
// store sites, never invented.

#include "F3/Commands.hpp"

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "F3.hpp"
#include "JE.hpp"
#include "JE/cls_0x4d8d70.hpp"
#include "JE/cls_0x50db20.hpp"
#include "JE/Network.hpp"
#include "JE/Client.hpp"
#include "JE/CameraControl.hpp"
#include "tracing.hpp"

namespace F3 {

	namespace {

		// shipped chain, e.g. 0x588368: mov [0x70bd90],eax / mov 0x2c(eax) / mov 0x84(ecx)
		JE::CameraControl* Cam() {
			JE::Network* net = JE::Network::GetGameNetwork();   // [0x70BD90], store 0x5438fa
			if ( !net ) {
				return 0;   // shipped has NO null guard on this chain (a pre-boot "CameraTop"
			                // faults identically); guard lives here because the command EXECUTION
			                // dispatch site is not RE'd yet - the boot-only difference is noted.
			}
			JE::Client* client = net->GetGameClient();          // +0x2C
			if ( !client ) {
				return 0;
			}
			return client->GetCamera();                          // +0x84
		}

		// First argv token text, or 0. Shipped node shape (census 2026-10-22, three families):
		// nodes are 4-byte cells holding STRING POINTERS (node stride 4: the count math
		// (*(stop-first)>>2) works only for 4-byte cells), NOT std::string objects: the text is
		// the cell value itself, e.g. 0x58b480 'atof(*(v2+4))' with v2 = *(argv+4) = first cell.
		// (An earlier std::list<string>-of-nodes reading was a name-era hypothesis; the cell math
		// refutes it - the corpus's argv convention is a pointer-to-pointer char*[] chain.)
		const char* FirstArg( unsigned int argv ) {
			return reinterpret_cast<const char*>( *reinterpret_cast<const unsigned int*>( argv + 4 ) );
		}

		// Shipped arg-count test, e.g. 0x58b4c5: ((*(argv+8) - first) >> 2) > 1 - i.e. the
		// token chain holds more than one cell before its terminator.
		int HasArg( unsigned int argv ) {
			unsigned int first_addr = *reinterpret_cast<const unsigned int*>( argv + 4 );
			unsigned int stop_addr  = *reinterpret_cast<const unsigned int*>( argv + 8 );
			int count_cells = ( static_cast<int>( stop_addr ) - static_cast<int>( first_addr ) ) >> 2;
			return first_addr != 0 && count_cells > 1;   // shipped conjoins the node-non-null test
		}

		// Debug echo print, sub_59EAF0 (writes through client+0x88, the scripting-console
		// object installed in the net-boot era - store site not RE'd, zeroed at 0x58e9b9 by
		// JE::Client::ctor). Named-absence: no reimpl console object exists yet; skipping the
		// echo is the port-side deviation, the SET path below is byte-faithful.
		void EchoClient( JE::Network* net, const char* fmt, float v ) {
			tracing::instrument( tracing::LOCATION, "%s %f", fmt, v );
			(void) net;
		}

		// Debug-console echo (script family): JE::CDebugInterface::meth_0x5583e0 on
		// [client+0x8C] (the slot the shipped handlers load as [[0x70BD90]->0x2C]+140, e.g.
		// @0x58e4c3), called (msg, fg, bg, 0) with the documented shipped color quirk: Disable
		// and remote lines pass (0x80,0) @0x58e52a/@0x58e583, Enable passes (0,128) @0x58e63a,
		// OneScriptPass (0x80,128) @0x58e74e. Named-absence: the port's JE::CDebugInterface is a
		// vft stub; skipping the echo is the port-side deviation (EchoClient precedent, slice 1).
		void EchoConsole( JE::Network* net, const char* text ) {
			tracing::instrument( tracing::LOCATION, "%s", text );
			(void) net;
		}

		// Script-control subsystem flags, shipped [net->mbr_0x18]+0x70E4/0x70E5/0x70E6 (the
		// connection object's byte trio: toggle 28900 @0x5888a2, disabled 28901 @0x58e4dc/0x58e5ec,
		// one-pass 28902 via sub_5567F0 @0x5568a6). Port: typed off the untyped dword slot with the
		// slice-1 boot-only guard (shipped faults identically pre-connect; conn install is the
		// net-boot era, named-unresolved).
		byte* ScriptConn( JE::Network* net ) {
			return reinterpret_cast<byte*>( net->mbr_0x18 );
		}

		// 32-bit view of client+0x112 scroll-flag slot (shipped operand width is DWORD,
		// stores 0x588448-0x5884b8 era; the Client.hpp word alias is a narrower label on
		// the same storage - accessor keeps the port's reads/writes at shipped width).
		unsigned int& ScrollWord( JE::Client* c ) {
			return *reinterpret_cast<unsigned int*>( reinterpret_cast<char*>( c ) + 0x112 );   // @0x588448-era
		}

		// 32-bit view of camera+0xC2 flag word (stores 0x5884ce-0x58856e era).
		unsigned int& CamFlag( JE::CameraControl* cam ) {
			return *reinterpret_cast<unsigned int*>( reinterpret_cast<char*>( cam ) + 0xC2 );   // @0x5884ce-era
		}

		// Client accessor for the commands whose chain dies at the CLIENT, not the camera
		// (scroll words +0x112, and the mouse sub_587870 era). Shipped chain: mov [0x70bd90]
		// -> +0x2C with NO guard (raw 0x588448 shape).
		JE::Client* Client0() {
			JE::Network* net = JE::Network::GetGameNetwork();
			return net ? net->GetGameClient() : 0;
		}

	} // namespace

	// 0x58B450 - "CameraAzimuth <deg>": parse atof, gate on arg count, set azimuth.
	// shipped body: v2=*(argv+4); if (v2 && ((*(argv+8)-v2)>>2)>1) set_azimuth(cam, atof(*(v2+4)))
	void __cdecl CommandCameraAzimuth(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_azimuth_0x585ea0( static_cast<float>( atof( FirstArg( argv ) ) ) );
		}
	} // CommandCameraAzimuth

	// 0x588360 - "CameraAzimuthInc": local = cur_deg(mbr_0xa4) * 57.295776 * 0.022222223 + 1.0,
	// then * 45.0 -> set_azimuth (shipped floats at 0x647920/0x69c484/0x647880/0x69bef4).
	void __cdecl CommandCameraAzimuthInc() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( !cam ) { return; }
		float local_4 = cam->mbr_0xa4 * 57.295776f * 0.022222223f + 1.0f;
		local_4 = local_4 * 45.0f;
		cam->set_azimuth_0x585ea0( local_4 );
	} // CommandCameraAzimuthInc

	// 0x5883A0 - "CameraAzimuthDec": same product, MINUS 45.0 (0x5883cd: sub after the muls).
	void __cdecl CommandCameraAzimuthDec() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( !cam ) { return; }
		float local_4 = cam->mbr_0xa4 * 57.295776f * 0.022222223f * 45.0f - 45.0f;
		cam->set_azimuth_0x585ea0( local_4 );
	} // CommandCameraAzimuthDec

	// 0x58B4A0 - "CameraAzimuthSpeed <f>": with arg -> camera+0xC8 = atof; without ->
	// echo "Camera azimuth speed is %f" (echo half: named-absence, see EchoClient).
	void __cdecl CommandCameraAzimuthSpeed(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::Network* net = JE::Network::GetGameNetwork();
		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->mbr_0xc8 = static_cast<float>( atof( FirstArg( argv ) ) );   // 0x58b4d3
		} else if ( net ) {
			EchoClient( net, "Camera azimuth speed is %f", cam ? cam->mbr_0xc8 : 0.0f );   // 0x58b4f3
		}
	} // CommandCameraAzimuthSpeed

	// 0x58B500 - "CameraElevation <deg>": clamped animated set via CameraControl::meth_0x586010.
	void __cdecl CommandCameraElevation(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_elevation_0x586010( static_cast<float>( atof( FirstArg( argv ) ) ) );
		}
	} // CommandCameraElevation

	// 0x58B550 - "CameraElevationSpeed <f>": camera+0xD8 = atof (0x58b583), else echo.
	void __cdecl CommandCameraElevationSpeed(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::Network* net = JE::Network::GetGameNetwork();
		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->mbr_0xd8 = static_cast<float>( atof( FirstArg( argv ) ) );
		} else if ( net ) {
			EchoClient( net, "Camera elevation speed is %f", cam ? cam->mbr_0xd8 : 0.0f );   // 0x58b5a3
		}
	} // CommandCameraElevationSpeed

	// 0x588400 - "CameraTop": sub_5862F0(cam, 1): LOBYTE(param)!=0 branch (0x586300) sets
	// elevation to FIXED 90.0 through the animated setter (sub_5862f0:2f0/300/305 raw).
	void __cdecl CommandCameraTop() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			cam->set_elevation_0x586010( 90.0f );   // 0x586300 branch of sub_5862f0
		}
	} // CommandCameraTop

	// 0x5883E0 - "CameraTopDown": sub_586290(cam, 1) - target = (cur_elev_deg <= 45.0) ? 90.0 : 0.0,
	// then ANIMATED set (0x5862cd). The <=45 test is at 0x5862b0 (mbr_0xa8 * 57.295776).
	void __cdecl CommandCameraTopDown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			float target = ( cam->mbr_0xa8 * 57.295776f <= 45.0f ) ? 90.0f : 0.0f;   // 0x5862b0
			cam->set_elevation_0x586010( target );
		}
	} // CommandCameraTopDown

	// 0x588580 - "CameraTiltUp": xor the MODE DWORD (cam+0xC2, 32-bit operand per raw
	// 0x58858e *(cam+194)^=0x100) with 0x100. NOT an elevation set - the name lied, the bytes
	// did not. v3 key: the mode dword's low half IS mbr_0xc4 zoom speed; the flip zeroes it
	// (shipped overlap, our CamFlag matches the width by construction).
	void __cdecl CommandCameraTiltUp() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			CamFlag( cam ) ^= 0x100u;   // 0x58858e
		}
	} // CommandCameraTiltUp

	// 0x5885A0 - "CameraTiltDown": xor mode dword with 0x200 (0x5885ae).
	void __cdecl CommandCameraTiltDown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			CamFlag( cam ) ^= 0x200u;   // 0x5885ae
		}
	} // CommandCameraTiltDown

	// 0x58B5B0 - "CameraDistance <d>": arg-gated distance set via 0x586140 (0x58b5e0/0x58b5ee raw).
	void __cdecl CommandCameraDistance(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_distance_0x586140( (float)atof( FirstArg( argv ) ) );
		}
	} // CommandCameraDistance

	// 0x58B600 - "CameraDistanceSpeed <f>": sets cam+0xF8 (0x58b633) - THE SHIPPED SLOT that the
	// animated distance setter uses as TARGET; ported verbatim, collision documented in hpp.
	// No-arg branch echoes (named-absence echo, EchoClient).
	void __cdecl CommandCameraDistanceSpeed(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->mbr_0xf8 = (float)atof( FirstArg( argv ) );    // 0x58b633
		} else if ( cam ) {
			EchoClient( JE::Network::GetGameNetwork(), "Camera distance speed is %f", cam->mbr_0xf8 );   // 0x58b653
		}
	} // CommandCameraDistanceSpeed

	// SCROLL FLAG FAMILY (slice 2) - eight commands, DWORD operand at client+0x112 (shipped
	// stores are 32-bit or/and through *(client+274) despite the hpp's word alias; On = OR,
	// Off = AND-NOT. Every body: null-guarded client chain (shipped has no guard, raw 0x588448).
	// cppcheck-suppress syntaxError
	void __cdecl CommandCameraScrollLeft_On() {              // @0x588440 (store 0x588448 |= 1)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) |= 1u;
	} // CommandCameraScrollLeft_On

	void __cdecl CommandCameraScrollRight_On() {             // @0x588450 (store 0x588458 |= 2)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) |= 2u;
	} // CommandCameraScrollRight_On

	void __cdecl CommandCameraScrollUp_On() {                // @0x588460 (store 0x588468 |= 4)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) |= 4u;
	} // CommandCameraScrollUp_On

	void __cdecl CommandCameraScrollDown_On() {              // @0x588470 (store 0x588478 |= 8)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) |= 8u;
	} // CommandCameraScrollDown_On

	void __cdecl CommandCameraScrollLeft_Off() {             // @0x588480 (store 0x588488 &= ~1)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) &= ~1u;
	} // CommandCameraScrollLeft_Off

	void __cdecl CommandCameraScrollRight_Off() {            // @0x588490 (store 0x588498 &= ~2)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) &= ~2u;
	} // CommandCameraScrollRight_Off

	void __cdecl CommandCameraScrollUp_Off() {               // @0x5884a0 (store 0x5884a8 &= ~4)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) &= ~4u;
	} // CommandCameraScrollUp_Off

	void __cdecl CommandCameraScrollDown_Off() {             // @0x5884b0 (store 0x5884b8 &= ~8)
		tracing::instrument( tracing::LOCATION, "" );
		JE::Client* c = Client0(); if ( c ) ScrollWord(c) &= ~8u;
	} // CommandCameraScrollDown_Off

	// ZOOM/ROTATE FLAG FAMILY (slice 2) - six commands, DWORD operand at cam+0xC2 (shipped
	// 32-bit or/and/xor through *(cam+194)); Zoom pair OR/AND, Rotate pair XOR (hold-toggle).
	void __cdecl CommandCameraZoomIn_On() {                  // @0x5884c0 (store 0x5884ce |= 0x10)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) |= 0x10u;
	} // CommandCameraZoomIn_On

	void __cdecl CommandCameraZoomIn_Off() {                 // @0x5884e0 (store 0x5884ee &= ~0x10)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) &= ~0x10u;
	} // CommandCameraZoomIn_Off

	void __cdecl CommandCameraZoomOut_On() {                 // @0x588500 (store 0x58850e |= 0x20)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) |= 0x20u;
	} // CommandCameraZoomOut_On

	void __cdecl CommandCameraZoomOut_Off() {                // @0x588520 (store 0x58852e &= ~0x20)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) &= ~0x20u;
	} // CommandCameraZoomOut_Off

	void __cdecl CommandCameraRotateLeft() {                 // @0x588540 (store 0x58854e ^= 0x40)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) ^= 0x40u;
	} // CommandCameraRotateLeft

	void __cdecl CommandCameraRotateRight() {                // @0x588560 (store 0x58856e ^= 0x80)
		tracing::instrument( tracing::LOCATION, "" );
		JE::CameraControl* cam = Cam(); if ( cam ) CamFlag(cam) ^= 0x80u;
	} // CommandCameraRotateRight

	// 0x588420 - "CameraDown": sub_586310(cam, 1): LOBYTE true -> animated set 0.0 (0x586320),
	// the nadir twin of CameraTop/TopDown (instant branch is the param_1-false leg, 0x586325).
	void __cdecl CommandCameraDown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			cam->set_elevation_0x586010( 0.0f );   // 0x586320
		}
	} // CommandCameraDown

	// 0x58B7D0 - "CameraScrollSpeed <f>" (SLICE 3 UNBLOCKED: the tick consumer proved 0x110 is
	// scroll speed, NOT the view pointer - my slice-2 collision claim was a misread, corrected;
	// ctor stamps go to 0x118/0x11c raw @0x586fd9). Sets mbr_0x110 with arg, echoes without.
	void __cdecl CommandCameraScrollSpeed(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->mbr_0x110 = (float)atof( FirstArg( argv ) );   // 0x58b803
		} else if ( cam ) {
			EchoClient( JE::Network::GetGameNetwork(), "Camera scroll speed is %f", cam->mbr_0x110 );   // 0x58b823
		}
	} // CommandCameraScrollSpeed

	// 0x58B660 - "CameraFOV <deg>" (SLICE 3 UNBLOCKED): arg -> set_fov_0x5867c0 (clamp 104/108
	// when enable 0x114 set; the VIEW stamp trio 0x58681f-0x586828 is the named-unported callee
	// half, lands with the Gfx_Engine port); the shipped echo (0x58b6a2-0x58b6cf, degrees from
	// view mbr_0x118 era) is the named-absence half via EchoClient.
	void __cdecl CommandCameraFOV(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_fov_0x5867c0( (float)atof( FirstArg( argv ) ) );   // 0x58b695
		}
		EchoClient( JE::Network::GetGameNetwork(), "Camera FOV is %f", cam ? cam->mbr_0x118 * 57.295776f : 0.0f );   // 0x58b6cf (view-degrees read, port-side from 0x118)
	} // CommandCameraFOV

	// DEFERRED (named-absence per item 57, bodies read but not portable yet):
	//  CommandCameraFOV @0x58B660 - CameraControl::meth_0x5867c0 writes through the
	//    JE::Gfx_Engine singleton (global [0x707CE4], vft+0xA8 view getter, stores +0x4A=1,
	//    +0x110=rad FOV at 0x58681f-0x586828) — rides the now-LIVE device era (G1 positive,
	//    41b1a74): slice 3 validates it against a real device instead of guessing.
	//  CommandCameraReset @0x58A7F0 / CommandCameraSetTarget @0x58B6E0 - entity-list walk
	//    (byte_70BE0C sentinel loop, entity +0x1E8/+0x1DC keys, resolver meth_0x54be10).

	// ------------------------------------------------------------------
	// Script-control family (slice 6). Shipped shape, four commands, all gated on
	// [net]+0x04 & 1 (the local-client authority bit; shipped reads it through
	// JE::AreaMapIcon::Get_mbr_0x4_0x5b4f30 on the [0x70BD90] alias, e.g. @0x58e4d2 - the
	// mislabeled global is the Network object per slice 1). Flags are the conn byte trio;
	// echoes route through EchoConsole (named-absent deviation above).
	// ------------------------------------------------------------------

	// @0x588880 "ToggleScripts": gate-only, NO echo in either branch (unlike its siblings);
	// flips conn[+0x70E4] to the negation of its zero-test (@0x5888a2: b = (b == 0)).
	void __cdecl CommandToggleScripts() {
		JE::Network* net = JE::Network::GetGameNetwork();          // [0x70BD90] store 0x5438fa
		if ( !net ) {
			return;   // port boot-guard (slice-1 Cam() precedent; shipped has none)
		}
		if ( ( net->mbr_0x4 & 1 ) != 0 ) {                         // @0x58888d
			byte* conn = ScriptConn( net );
			if ( conn ) {
				conn[ 0x70E4 ] = ( conn[ 0x70E4 ] == 0 ) ? 1 : 0;   // @0x5888a2 (28900)
			}
		}
	}

	// @0x58e4a0 "DisableScripts": local -> conn[+0x70E5]=1 + "Scripts disabled.";
	// remote -> echo only ("Unable to disable scripts from remote client." @0x58e568).
	void __cdecl CommandDisableScripts() {
		JE::Network* net = JE::Network::GetGameNetwork();
		if ( !net ) {
			return;
		}
		if ( ( net->mbr_0x4 & 1 ) != 0 ) {                         // @0x58e4d2
			byte* conn = ScriptConn( net );
			if ( conn ) {
				conn[ 0x70E5 ] = 1;                                 // @0x58e4dc (28901)
			}
			EchoConsole( net, "Scripts disabled." );               // text @0x58e513, echo @0x58e52a
		} else {
			EchoConsole( net, "Unable to disable scripts from remote client." );   // @0x58e568/0x58e583
		}
	}

	// @0x58e5b0 "EnableScripts": mirror image; shipped color params differ from Disable's
	// ((0,128) @0x58e63a vs (0x80,0)) - the documented shipped quirk, carried by EchoConsole's
	// comment (the port echo ignores colors until the console TU lands).
	void __cdecl CommandEnableScripts() {
		JE::Network* net = JE::Network::GetGameNetwork();
		if ( !net ) {
			return;
		}
		if ( ( net->mbr_0x4 & 1 ) != 0 ) {                         // @0x58e5e2
			byte* conn = ScriptConn( net );
			if ( conn ) {
				conn[ 0x70E5 ] = 0;                                 // @0x58e5ec
			}
			EchoConsole( net, "Scripts enabled." );                // text @0x58e623, echo @0x58e63a
		} else {
			EchoConsole( net, "Unable to enable scripts from remote client." );    // @0x58e678/0x58e693
		}
	}

	// @0x58e6c0 "OneScriptPass": local -> sub_5567F0(conn, 1) @0x58e6fc, whose param_1==1 arm
	// is the unconditional store conn[+0x70E6]=1 (@0x5568a6; the clear/"Done" arm belongs to
	// the SCRIPT PUMP's caller era, not this command - not ported, not needed) + echo;
	// remote -> "Unable to control script execution from remote client." @0x58e793.
	void __cdecl CommandOneScriptPass() {
		JE::Network* net = JE::Network::GetGameNetwork();
		if ( !net ) {
			return;
		}
		if ( ( net->mbr_0x4 & 1 ) != 0 ) {                         // @0x58e6f0
			byte* conn = ScriptConn( net );
			if ( conn ) {
				conn[ 0x70E6 ] = 1;                                 // via sub_5567F0 @0x556817/@0x5568a6
			}
			EchoConsole( net, "Executing scripts for one game round." );   // @0x58e733/0x58e74e
		} else {
			EchoConsole( net, "Unable to control script execution from remote client." );  // @0x58e793
		}
	}

	// ------------------------------------------------------------------
	// Execute() mirror: the shipped command FIFO + frame drain. Shipped counterparts, all
	// raw-pinned in tasks/R-USERRE-finish-user-functions.md (R-USERRE-A slice 4e, 615c400):
	//   FIFO      vector<string> at [GameClient+0x7C]; the drain's cursor pair is its begin/end
	//             (mov ecx,[esi+7Ch] @0x587fd2 -> cls_0x580ba0::meth_0x580ba0 @0x580ba0).
	//   push      sub_580D10 (temp string @0x580d5f, push_back-at-end @0x580d74): hotkey fire
	//             0x59978f, console submit 0x59ea29, UI-literal menu feeds 0x5b6d54/0x41cb89.
	//   tokenizer sub_580B00: strtok (sub_61A7D1) against byte_671C60 = a SINGLE SPACE into a
	//             char* cell vector; the source is copied to a 256-byte local first
	//             (strcpy @0x580c20, local v17[256]).
	//   lookup    sub_59F030 -> sub_59EC80: lower_bound + _stricmp-equal, returns the node+0x28
	//             value (the handler) or 0 - ported verbatim as cls_0x4d8d70::FindValue
	//             (0x5ACE40). Case-insensitivity is the TREE's policy, not the tokenizer's.
	//   dispatch  0x580c46-0x580c4d: lea argv / push / call eax / add esp,4 - ONE physical
	//             argument, CALLER cleanup (cdecl): the shipped zero-arg handlers end in bare
	//             retn (CommandCameraDown @0x588435), the argv readers are cdecl(void*). This is
	//             why every command handler moved __stdcall -> __cdecl with this commit.
	//   miss      JE::LogDebugString_0x497120("Unknown command %s\n", name) @0x580c65.
	//   order     drain @0x587fd5 runs IMMEDIATELY BEFORE the camera tick @0x587fda - commands
	//             act before the camera animates, same frame. Pump wiring is a NAMED ABSENCE:
	//             F3::GameClient's pump (virt_meth_0x45f000) has no port TU yet; whoever ports
	//             it calls F3::ExecutePendingCommands() at the 0x587fd5 position, before the
	//             tick call. Until then the FIFO is fed only by explicit PushCommand callers.
	// ------------------------------------------------------------------

	namespace {

		std::vector<std::string> g_commandFifo;   // [GameClient+0x7C] counterpart

	} // namespace

	// sub_580D10 counterpart - every shipped feeder pushes through this one shape.
	void PushCommand( const char* line ) {
		g_commandFifo.push_back( line );
	}

	// cls_0x580ba0::meth_0x580ba0 counterpart: drains the WHOLE FIFO each call, FIFO order
	// (the shipped loop executes the front element, then erase-shifts left @0x580c70-0x580cb0
	// and pops the back), exactly one dispatch per queued line.
	void ExecutePendingCommands() {
		while ( !g_commandFifo.empty() ) {
			// 256-byte copy local (v17 @0x580c20). Shipped strcpy is length-blind - a line over
			// 255 would smash that frame; the port truncates instead. Deviation on a shipped UB
			// path, documented per the slice-1 null-guard precedent above.
			char buffer[ 256 ];
			strncpy( buffer, g_commandFifo.front().c_str(), sizeof( buffer ) - 1 );
			buffer[ sizeof( buffer ) - 1 ] = '\0';
			g_commandFifo.erase( g_commandFifo.begin() );

			// sub_580B00: strtok against the single-space delimiter, cells in queue order.
			std::vector<char*> cells;
			for ( char* tok = strtok( buffer, " " ); tok != 0; tok = strtok( 0, " " ) ) {
				cells.push_back( tok );
			}

			if ( cells.empty() ) {
				continue;   // shipped would dereference the empty vector's storage (*Block, @0x580c3a
				            // on Block==0) - undefined; no known feeder enqueues an all-space line.
			}

			// argv as the shipped handlers see it: {+0x00 unused, +0x04 first cell, +0x08 end
			// cell, +0x0C cap} - the executor's vector-object head; FirstArg() reads *(argv+4),
			// HasArg() computes (*(argv+8)-*(argv+4))>>2 (pinned per-handler in the rows above).
			struct ArgvShape { void* head; char** first; char** last; char** end; };
			ArgvShape argv;
			argv.head  = 0;
			argv.first = &cells[ 0 ];
			argv.last  = argv.first + cells.size();
			argv.end   = argv.last;

			// First token is the command name. SSO string built exactly like RegisterCommand's
			// temp (0x59f5fc-0x59f60c) so FindValue's query-shape holds.
			JE::cls_0x50db20 name;
			byte* raw = reinterpret_cast<byte*>( &name );
			raw[0] = 0;
			raw[4] = 0;
			*reinterpret_cast<dword*>( raw + 0x14 ) = 0;   // size
			*reinterpret_cast<dword*>( raw + 0x18 ) = 15;  // inline capacity
			name.meth_0x401bd0( cells[ 0 ], static_cast<int>( strlen( cells[ 0 ] ) ) );

			int handler = g_commandRegistryRoot.FindValue( &name );   // 0x5ACE40 (== 0 on miss)
			if ( handler != 0 ) {
				// push argv / call / add esp,4 (@0x580c4a-0x580c4d). void() handlers ignore the
				// physical slot; argv readers take cdecl(void*) - matches their compiled retn.
				reinterpret_cast<void (__cdecl *)( unsigned int )>( handler )(
				    static_cast<unsigned int>( reinterpret_cast<unsigned long>( &argv ) ) );
			} else {
				JE::LogDebugString( NULL, "Unknown command %s\n", cells[ 0 ] );   // @0x580c65
			}

			if ( *reinterpret_cast<dword*>( raw + 0x18 ) >= 0x10 ) {
				free( *reinterpret_cast<void**>( raw + 4 ) );   // long-name heap cleanup (0x59f6bd)
			}
		}
	} // ExecutePendingCommands

} // namespace F3

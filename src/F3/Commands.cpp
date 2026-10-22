// Copyright 2024 OpenJE
// R-USERRE-A slice 1 implementation - camera console commands. See Commands.hpp for the
// chain (world=[0x70BD90] -> client +0x2C -> camera +0x84) and the argv link-cell convention.
// Every def cites its shipped EA (item 57); unported callee objects are named with their
// store sites, never invented.

#include "F3/Commands.hpp"

#include <stdlib.h>

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

	} // namespace

	// 0x58B450 - "CameraAzimuth <deg>": parse atof, gate on arg count, set azimuth.
	// shipped body: v2=*(argv+4); if (v2 && ((*(argv+8)-v2)>>2)>1) set_azimuth(cam, atof(*(v2+4)))
	void __stdcall CommandCameraAzimuth(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_azimuth_0x585ea0( static_cast<float>( atof( FirstArg( argv ) ) ) );
		}
	} // CommandCameraAzimuth

	// 0x588360 - "CameraAzimuthInc": local = cur_deg(mbr_0xa4) * 57.295776 * 0.022222223 + 1.0,
	// then * 45.0 -> set_azimuth (shipped floats at 0x647920/0x69c484/0x647880/0x69bef4).
	void __stdcall CommandCameraAzimuthInc() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( !cam ) { return; }
		float local_4 = cam->mbr_0xa4 * 57.295776f * 0.022222223f + 1.0f;
		local_4 = local_4 * 45.0f;
		cam->set_azimuth_0x585ea0( local_4 );
	} // CommandCameraAzimuthInc

	// 0x5883A0 - "CameraAzimuthDec": same product, MINUS 45.0 (0x5883cd: sub after the muls).
	void __stdcall CommandCameraAzimuthDec() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( !cam ) { return; }
		float local_4 = cam->mbr_0xa4 * 57.295776f * 0.022222223f * 45.0f - 45.0f;
		cam->set_azimuth_0x585ea0( local_4 );
	} // CommandCameraAzimuthDec

	// 0x58B4A0 - "CameraAzimuthSpeed <f>": with arg -> camera+0xC8 = atof; without ->
	// echo "Camera azimuth speed is %f" (echo half: named-absence, see EchoClient).
	void __stdcall CommandCameraAzimuthSpeed(unsigned int argv) {
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
	void __stdcall CommandCameraElevation(unsigned int argv) {
		tracing::instrument( tracing::LOCATION, "argv=%08X", argv );

		JE::CameraControl* cam = Cam();
		if ( HasArg( argv ) && cam ) {
			cam->set_elevation_0x586010( static_cast<float>( atof( FirstArg( argv ) ) ) );
		}
	} // CommandCameraElevation

	// 0x58B550 - "CameraElevationSpeed <f>": camera+0xD8 = atof (0x58b583), else echo.
	void __stdcall CommandCameraElevationSpeed(unsigned int argv) {
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
	void __stdcall CommandCameraTop() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			cam->set_elevation_0x586010( 90.0f );   // 0x586300 branch of sub_5862f0
		}
	} // CommandCameraTop

	// 0x5883E0 - "CameraTopDown": sub_586290(cam, 1) - target = (cur_elev_deg <= 45.0) ? 90.0 : 0.0,
	// then ANIMATED set (0x5862cd). The <=45 test is at 0x5862b0 (mbr_0xa8 * 57.295776).
	void __stdcall CommandCameraTopDown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			float target = ( cam->mbr_0xa8 * 57.295776f <= 45.0f ) ? 90.0f : 0.0f;   // 0x5862b0
			cam->set_elevation_0x586010( target );
		}
	} // CommandCameraTopDown

	// 0x588580 - "CameraTiltUp": xor camera word mbr_0xc2 with 0x100 (raw 0x58858e:
	// mov ax,[eax+0xc2]; xor ax,0x100; mov [..],ax shape via *(cam+194)^=0x100). NOT an
	// elevation set - the name lied, the bytes did not.
	void __stdcall CommandCameraTiltUp() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			cam->mbr_0xc2 ^= 0x100;   // 0x58858e
		}
	} // CommandCameraTiltUp

	// 0x5885A0 - "CameraTiltDown": xor camera word mbr_0xc2 with 0x200 (0x5885ae).
	void __stdcall CommandCameraTiltDown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::CameraControl* cam = Cam();
		if ( cam ) {
			cam->mbr_0xc2 ^= 0x200;   // 0x5885ae
		}
	} // CommandCameraTiltDown

	// DEFERRED this slice (named-absence per item 57, bodies read but not portable yet):
	//  CommandCameraFOV @0x58B660 - CameraControl::meth_0x5867c0 writes through the
	//    JE::Gfx_Engine singleton (global [0x707CE4], vft+0xA8 view getter, stores +0x4A=1,
	//    +0x110=rad FOV at 0x58681f-0x586828) — rides the now-LIVE device era (G1 positive,
	//    41b1a74): slice 3 validates it against a real device instead of guessing.
	//  CommandCameraReset @0x58A7F0 / CommandCameraSetTarget @0x58B6E0 - entity-list walk
	//    (byte_70BE0C sentinel loop, entity +0x1E8/+0x1DC keys, resolver meth_0x54be10).

} // namespace F3

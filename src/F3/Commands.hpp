// Copyright 2024 OpenJE
// R-USERRE-A slice 1: F3 console command handlers (camera family).
// Batch ground truth: the 112 batch-A "orphans" are the RegisterCommand tails of
// JE::Client::ctor_0x58e900 (registrations 0x58ea4e..0x58eff1) and F3::GameClient::ctor_0x460b30
// (0x460b6f..0x460cbf) - discovered 2026-10-22, ledger in tasks/R-USERRE-finish-user-functions.md.
// Chain for every camera handler: world = [0x70BD90] (JE::Network ctor store 0x5438fa),
// client = world->mbr_0x2c (RTTI GameClient cast 0x43978a per discoveries 2025-10-20 rows),
// camera = client->mbr_0x84 (JE::CameraControl*, created with the client - install site is
// network-boot era, named-unresolved). argv convention: pointer to a 4-byte cell chain
// (std::list<string> nodes); first node = *(argv+4), terminator cell = *(argv+8);
// arg-count test in the shipped handlers is ((*(argv+8) - *(argv+4)) >> 2) > 1 with a
// non-null first node (e.g. 0x58b4c5 in CommandCameraAzimuthSpeed).

#ifndef F3_COMMANDS_HPP_
#define F3_COMMANDS_HPP_

#include "types.hpp"

namespace F3 {

	// Camera console command handlers, JE::Client::ctor registration era (slice 1 = the
	// set/Inc/Dec/Speed families that need no unported subsystem beyond JE::CameraControl).
	// Names are the user-era IDB claims; EAs are the contract (item 50 law).

	// @0x58b450  "CameraAzimuth <deg>" -> CameraControl::set_azimuth (0x585ea0)
	void __cdecl CommandCameraAzimuth(unsigned int argv);
	// @0x588360  "CameraAzimuthInc"  : cur_degrees*rad2deg*0.02222 + 1.0, *45 -> set_azimuth
	void __cdecl CommandCameraAzimuthInc();
	// @0x5883a0  "CameraAzimuthDec"  : same delta minus 45.0 -> set_azimuth
	void __cdecl CommandCameraAzimuthDec();
	// @0x58b4a0  "CameraAzimuthSpeed <f>" : set camera+0xc8; echo via client+0x88 print (unported)
	void __cdecl CommandCameraAzimuthSpeed(unsigned int argv);

	// @0x58b500  "CameraElevation <deg>" -> CameraControl meth_0x586010 (clamped animated set)
	void __cdecl CommandCameraElevation(unsigned int argv);
	// @0x58b550  "CameraElevationSpeed <f>" : set camera+0xd8
	void __cdecl CommandCameraElevationSpeed(unsigned int argv);

	// @0x588400  "CameraTop"   : sub_5862f0(cam, 1) -> meth_0x586010(90.0) (animated top-down)
	void __cdecl CommandCameraTop();
	// @0x5883e0  "CameraTopDown": CONFIRMED raw 0x5883e0 body (sub_5862f0(cam,0) shape)
	void __cdecl CommandCameraTopDown();
	// @0x588580  "CameraTiltUp"   : TOGGLE camera word mbr_0xc2 bit 0x100 (0x58858e: xor 0x100)
	void __cdecl CommandCameraTiltUp();
	// @0x5885a0  "CameraTiltDown" : TOGGLE camera word mbr_0xc2 bit 0x200 (0x5885ae: xor 0x200)
	void __cdecl CommandCameraTiltDown();

	// Slice 2 - camera tail (all raw-read, registrations land in JE::Client::ctor):
	void __cdecl CommandCameraDistance(unsigned int argv);        // @0x58b5b0 -> setter 0x586140
	void __cdecl CommandCameraDistanceSpeed(unsigned int argv);   // @0x58b600 (collides on cam+0xF8, documented)
	void __cdecl CommandCameraScrollLeft_On();                    // @0x588440  ... Off pairs:
	void __cdecl CommandCameraScrollRight_On();                   // @0x588450
	void __cdecl CommandCameraScrollUp_On();                      // @0x588460
	void __cdecl CommandCameraScrollDown_On();                    // @0x588470
	void __cdecl CommandCameraScrollLeft_Off();                   // @0x588480
	void __cdecl CommandCameraScrollRight_Off();                  // @0x588490
	void __cdecl CommandCameraScrollUp_Off();                     // @0x5884a0
	void __cdecl CommandCameraScrollDown_Off();                   // @0x5884b0
	void __cdecl CommandCameraZoomIn_On();                        // @0x5884c0
	void __cdecl CommandCameraZoomIn_Off();                       // @0x5884e0
	void __cdecl CommandCameraZoomOut_On();                       // @0x588500
	void __cdecl CommandCameraZoomOut_Off();                      // @0x588520
	void __cdecl CommandCameraRotateLeft();                       // @0x588540
	void __cdecl CommandCameraRotateRight();                      // @0x588560
	void __cdecl CommandCameraDown();                             // @0x588420 -> animated elev 0.0
	void __cdecl CommandCameraScrollSpeed(unsigned int argv);     // @0x58b7d0 (slice 3 unblocked)
	void __cdecl CommandCameraFOV(unsigned int argv);             // @0x58b660 (slice 3 unblocked; view-half named-unported)

	// DEFERRED (honest cuts, each cited; slice-1 list kept current):
	//  CommandCameraFOV @0x58b660  - needs the JE::Gfx_Engine singleton (global 0x707CE4,
	//    vft+0xA8 engine-view getter, ride of T-013 device era) inside CameraControl::
	//    meth_0x5867c0; not portable without the engine object.
	//  CommandCameraReset @0x58a7f0 / CommandCameraSetTarget @0x58b6e0 - need the entity list
	//    byte_70BE0C walk + entity +0x1DC/+0x1E8 handles + GameActionEquip::meth_0x54be10 resolver.
	//  Scroll/Zoom/MouseRotate/Rotate families (0x588440-0x5885d0) - scroll-target members ride
	//    the same engine/entity chain; census in slice 1 notes, port in slice 2.

	// Command FIFO, R-USERRE-A Execute() mirror (see Commands.cpp for the full shipped chain
	// and EAs): PushCommand feeds the queue (sub_580D10 counterpart - hotkey fire, console
	// submit, UI feeds all push through this one shape); ExecutePendingCommands drains and
	// dispatches it (cls_0x580ba0::meth_0x580ba0 counterpart: space-tokenize, registry
	// FindValue, __cdecl call with the argv cell-vector head pushed, miss logs
	// "Unknown command %s\n" @0x580c65). Pump call position NAMED-ABSENT until the
	// GameClient pump TU lands: 0x587fd5, immediately before the camera tick @0x587fda.
	void PushCommand( const char* line );
	void ExecutePendingCommands();

} // namespace F3

#endif // F3_COMMANDS_HPP_

#ifndef OPENVB_F3_STARTUPMOVIES_HPP
#define OPENVB_F3_STARTUPMOVIES_HPP
// src/F3/StartupMovies.hpp - the intro logo movies (Bink 1.x old-format .mve), M-MOVIES.
//
// WHAT SHIPPED ACTUALLY DOES, and where this file deliberately differs.
// Shipped's movie era is cls_0x5b4f50 (44 bytes, six methods), driven from the CLIENT-CONNECT path, not
// WinMain: JE::Client::virt_meth_0x58a620 logs "Client: connected to server.  My id is %u", checks a
// startup-movies flag (*(this+86)), then plays
//     movies\IPlogo.mve   at (192,224) size 640x320   [meth_0x5b5640 @0x58a6b9]
// and ENQUEUES
//     movies\BISlogo.mve  at (192,224) size 640x320   [meth_0x5b5b10 @0x58a700]
// The queue is popped by meth_0x5b5820: BinkClose the current handle, take the next node, re-open with the
// params stored in that node (+28,+32,+36,+40 pos/size; +44,+45,+48 flags; +52,+56 the two timeouts), and
// when the list is empty restore the cursor and GUI. So order is IPlogo THEN BISlogo, and "startup movies"
// really means "the logos that play when the client attaches".
//
// DELIBERATE DEVIATIONS, each visible in a digest or a screenshot rather than hidden:
//  1. ENTRY POINT. The port has no client-connect path that carries this class, so PlayStartupMovies() is
//     called from the boot path behind an opt-in env gate (OPENVB_MOVIES=1), the same arrangement as the
//     render slot. Moving it to the connect era is a later step; what is faithful here is the SEQUENCE and
//     the frame path, not the trigger.
//  2. SINK. Shipped renders each frame into a 1024x1024 X8R8G8B8 surface registered as the Gfx texture
//     "[MovieFrame]" (meth_0x5b5640 @0x5b56f8) and composited by a sprite object (vft+300 call @0x5b5a2e).
//     The port's Gfx_Texture_Manager is a name registry only - there is no texture upload or DrawPrimitive
//     path yet (no CreateTexture/LockRect anywhere in src/), so frames are copied straight into the D3D8
//     back buffer at shipped's rect. Same pixels, same position, different last hop.
//  3. The decode itself stays in binkw32.dll, exactly as shipped does: F3.exe imports these nine functions
//     and implements no Bink codec. Writing one would not be more faithful - it would be a different program.
//
// BINK STRUCT FIELDS COME FROM SHIPPED'S USE, not from the header: bink.h in dependencies/radtools-master is
// Bink 1.0p (2000-06-26) and keeps struct BINK opaque, while the runtime is 1.5u. meth_0x5b5ae0 reads
// +0 = frame width, +4 = frame height, +8 = total frames, +12 = current frame, and the end test is
// "current == total". Those four offsets are what the struct layout below encodes; if a run prints
// width/height that disagree with the 640x320 the .mve header carries, the layout is wrong and the movies
// will be visibly wrong, so the trace prints them instead of trusting them.

namespace F3 {
	namespace Movies {

		// Opt-in, like the render gate: unset means NOTHING happens, so CI and app-boot never enter here.
		bool Gated();

		// Play shipped's startup queue synchronously: IPlogo then BISlogo, 640x320 at (192,224).
		// Returns the number of movies opened. Never fails the process: a missing dll, missing file or
		// BinkOpen error is traced and skipped, because the intro must not be able to brick the boot.
		int PlayStartupMovies();

		// One movie, for the console command path (shipped has F3::CommandMovieTest at 0x588d2a calling
		// meth_0x5b5640 directly). Returns frames presented, or -1 if the file could not be opened.
		int PlayMovieFile( const char* path, int x, int y, int w, int h );

		// The audio recipe from bink.h: BinkSoundUseDirectSound(lpDS) expands to
		// BinkSetSoundSystem(BinkOpenDirectSound,(u32)lpDS) - and shipped calls exactly that with the IAT
		// SLOT ADDRESS of BinkOpenDirectSound (mov eax,ds:0x647338 @0x5b5302, call @0x5b530b). Passing the
		// FUNCTION ADDRESS instead is the classic mis-port; the value passed is a pointer-to-import-thunk.
		void SetupBinkSound();
	}
}
#endif

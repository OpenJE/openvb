// src/F3/StartupMovies.cpp - play the intro logo movies through binkw32.dll, M-MOVIES.
//
// THE SHAPE COMES FROM SHIPPED'S CALL SITES, which were read in the original IDB rather than reconstructed
// from the SDK docs (bink.h here is Bink 1.0p and the shipped runtime is 1.5u - the version delta has already
// bitten once in this lane: the header declares BinkGetTrackLargest, the DLL exports BinkGetTrackMaxSize).
//
//   open/close/queue  cls_0x5b4f50::meth_0x5b5640 @0x5b5640  - BinkOpen(path,0); on null handle it calls
//                     BinkGetError() and OutputDebugStringA() (@0x5b567b-0x5b5682), then bails. On success it
//                     registers the [MovieFrame] texture and, on the second movie, restores GUI/cursor.
//   advance           cls_0x5b4f50::meth_0x5b5820 @0x5b5820  - BinkClose, pop the next queue node, re-open
//                     with that node's stored rect/flags, and when the list empties restore the GUI.
//   pump              cls_0x5b4f50::meth_0x5b5ae0 @0x5b5ae0  - per tick:
//                        if ( !BinkWait(h) ) { BinkDoFrame(h); ... BinkCopyToBuffer(h, ptr, pitch,
//                              *(h+4), 0, 0, 0xF0000003); ...;
//                              if ( *(h+12) == *(h+8) ) advance; else BinkNextFrame(h); }
//   trigger           JE::Client::virt_meth_0x58a620 @0x58a620 - after "Client: connected to server", if the
//                     startup-movies flag (*(this+86)) is set: IPlogo.mve at (192,224) 640x320, then BISlogo
//                     queued at the same rect. So the visible ORDER is IPlogo then BISlogo.
//
// THE FOUR BINK STRUCT OFFSETS are the ones the pump reads (+0 width, +4 height, +8 total frames, +12 current
// frame). They are not in the 1.0p header, which keeps struct BINK opaque, so they are recorded here as
// measured-from-uses and printed on every run: if they were wrong the movies would be visibly wrong, and a
// printed width/height that disagrees with the 640x320 in the .mve header is the signal.
//
// CopyToBuffer's flag word is the literal shipped passes, 0xF0000003 (the decompiler shows it signed as
// -268435453). It is a packed conversion descriptor, not a boolean, and 1.0p does not name it - so it is
// carried verbatim rather than "translated" into a macro that may not exist in this runtime.

#include "F3/StartupMovies.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>
#include <d3d8.h>

#include "F3/RenderD3D8.hpp"

// ---- the nine imports shipped uses. Decorated names in src/F3/binkw32.def are _BinkXxx@N, and extern "C"
// ---- __stdcall produces exactly those references, so the import thunks land in the same .idata the
// original has (tools/pe-imports.py can compare the two tables). RADEXPLINK in the 1.0p header is __stdcall.
extern "C" {
	void*  __stdcall BinkOpen( const char* name, unsigned int flags );
	void   __stdcall BinkClose( void* h );
	int    __stdcall BinkDoFrame( void* h );
	void   __stdcall BinkNextFrame( void* h );
	int    __stdcall BinkWait( void* h );
	int    __stdcall BinkCopyToBuffer( void* h, void* dest, int destpitch, unsigned int destheight,
		                               unsigned int destx, unsigned int desty, unsigned int flags );
	char*  __stdcall BinkGetError();
	void*  __stdcall BinkOpenDirectSound( unsigned int param );
	int    __stdcall BinkSetSoundSystem( void* ( __stdcall* openfn )( unsigned int ), unsigned int param );
}

// The +0/+4/+8/+12 view described above. Only these four words are ever read.
struct BinkMeasured {
	unsigned int width;         // +0  (shipped: *(h+0)  in the draw call @0x5b5a2e)
	unsigned int height;        // +4  (shipped: *(h+4)  as CopyToBuffer's destheight)
	unsigned int frame_total;   // +8  (shipped: *(h+8)  end test)
	unsigned int frame_cur;     // +12 (shipped: *(h+12) end test)
};

#define BINK_COPY_FLAGS 0xF0000003u   // verbatim from @0x5b59f8

namespace F3 {
	namespace Movies {
		namespace {
			// Diagnostics on the same channel discipline as the render slot: OutputDebugString is invisible
			// under wine without a debugger, and fprintf(stderr) kills a GUI-subsystem image outright, so a
			// FILE channel is the only way this is checkable after the fact.
			void Note( const char* fmt, int a, int b ) {
				char line[ 512 ];
				FILE* f = 0;
				const char* path = getenv( "OPENVB_MOVIES_TRACE" );
				( void ) sprintf( line, fmt, a, b );
				OutputDebugStringA( line );
				if ( path && *path ) {
					f = fopen( path, "at" );
					if ( f ) { fprintf( f, "%s\n", line ); fclose( f ); }
				}
			}

			// PORT-SIDE diagnostic channel; shipped reports movie failure through OutputDebugStringA @0x5b5682.
			void NoteStr( const char* text ) {
				const char* path = getenv( "OPENVB_MOVIES_TRACE" );
				OutputDebugStringA( text );
				if ( path && *path ) {
					FILE* f = fopen( path, "at" );
					if ( f ) { fprintf( f, "%s\n", text ); fclose( f ); }
				}
			}

			// Copy one decoded frame into the back buffer, CLAMPED to the buffer. Returns 0 on failure; a blit
			// that cannot happen must not stop the movie loop or the process - it is logged instead.
			//
			// The clamp matters more than it looks. An earlier version put the bounds test INSIDE the row loop
			// as `if (x + w > d.Width) break;` - a condition constant across rows, so it aborted the copy at row
			// 0 on every frame while LockRect, UnlockRect and Present all returned S_OK. The trace read "frames
			// presented=229" with no failure line and the screen stayed black: a loop that faithfully decoded and
			// faithfully presented nothing at all. Bounds are computed once, outside the loop, and the copy is
			// the intersection, so an out-of-range rect degrades to a partial image instead of to a blank one.
			// PORT-SIDE sink: the shipped frame copy is BinkCopyToBuffer into a locked Gfx rect @0x5b59f8.
			int BlitFrame( void* hink, unsigned int w, unsigned int h, int x, int y ) {
				IDirect3DDevice8* dev = (IDirect3DDevice8*)F3::Render::Device();
				IDirect3DSurface8* bb = 0;
				D3DLOCKED_RECT lr;
				D3DSURFACE_DESC d;
				unsigned char* src = 0;
				unsigned int pitch = w * 4u;
				unsigned int row;
				int ok = 0;
				static const char* lastfail = "";

				if ( !dev ) { lastfail = "no device"; goto done; }
				// D3D8 arity: GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8**) - THREE parameters.
				// The fourth (MultiSampleQuality) is a D3D9 addition, and C2660 caught it live here exactly as
				// it caught the SetRenderTarget arity in RenderD3D8.cpp. Writing the d3d9 form against d3d8.h is
				// the default mistake because most reference material on the web is d3d9.
				if ( FAILED( dev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &bb ) ) || !bb ) {
					lastfail = "GetBackBuffer failed"; goto done;
				}
				if ( FAILED( bb->GetDesc( &d ) ) ) { lastfail = "GetDesc failed"; goto done; }
				if ( d.Format != D3DFMT_X8R8G8B8 && d.Format != D3DFMT_A8R8G8B8 ) {
					char m[ 96 ];
					( void ) sprintf( m, "back buffer format is 0x%02X, not a 32-bit BGRA we can copy into",
					                   (unsigned)d.Format );
					lastfail = m; goto done;
				}
				src = (unsigned char*)malloc( (size_t)( pitch * h ) );
				if ( !src ) { lastfail = "frame buffer alloc failed"; goto done; }

				// Exactly the shipped call: dest ptr, our pitch, frame height, origin 0,0, measured flags.
				if ( BinkCopyToBuffer( hink, src, (int)pitch, h, 0u, 0u, BINK_COPY_FLAGS ) ) {
					lastfail = "BinkCopyToBuffer returned error"; goto done;
				}
				if ( FAILED( bb->LockRect( &lr, 0, D3DLOCK_DISCARD ) ) ) { lastfail = "LockRect failed"; goto done; }
				{
					// Destination intersection, computed ONCE (see the BlitFrame note). A negative origin shifts
					// the source start instead of the destination pointer, so a partially off-screen rect still
					// shows whatever fits.
					int rows = (int)h, cols = (int)w, r, c0 = ( x < 0 ) ? -x : 0;
					if ( (int)d.Width  < x + (int)w ) { cols = (int)d.Width  - x; }
					if ( (int)d.Height < y + (int)h ) { rows = (int)d.Height - y; }
					cols -= c0;
					if ( cols <= 0 || rows <= 0 ) {
						lastfail = "dest rect does not intersect the back buffer";
						bb->UnlockRect();
						goto done;
					}
					for ( r = 0; r < rows; ++r ) {
						memcpy( (unsigned char*)lr.pBits + (size_t)( y + r ) * (size_t)lr.Pitch
						        + (size_t)( x + c0 ) * 4u,
						        src + (size_t)r * (size_t)pitch + (size_t)c0 * 4u,
						        (size_t)cols * 4u );
					}
				}
				bb->UnlockRect();
				if ( FAILED( dev->Present( 0, 0, 0, 0 ) ) ) { lastfail = "Present failed"; goto done; }
				ok = 1;
			done:
				if ( src ) { free( src ); }
				if ( bb ) { bb->Release(); }
				if ( !ok && *lastfail ) { NoteStr( lastfail ); }
				return ok;
			}

			// Destination origin centering a movie in the REAL back buffer, falling back to shipped's literal
			// (192,224) only when the buffer is the 1024x768 those numbers were written for. Shipped's rect is
			// not a constant: 192+640 = 832 does not fit an 800-wide client, so those values centre a 640x320
			// movie in a 1024x768 window and the caller passes whatever matches its own client size. Using the
			// literals on an 800x600 port window pushed the image mostly off-screen - the other half of why the
			// intro showed nothing.
			// PORT-SIDE: shipped passes the dest rect as size @0x5b57c4 (640,320) then pos @0x5b57d6 (192,224).
			void CenterRect( int w, int h, int* outX, int* outY ) {
				IDirect3DDevice8* dev = (IDirect3DDevice8*)F3::Render::Device();
				IDirect3DSurface8* bb = 0;
				D3DSURFACE_DESC d;
				int sw = 1024, sh = 768;
				if ( dev && SUCCEEDED( dev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &bb ) ) && bb ) {
					if ( SUCCEEDED( bb->GetDesc( &d ) ) ) { sw = (int)d.Width; sh = (int)d.Height; }
					bb->Release();
				}
				if ( sw == 1024 && sh == 768 ) { *outX = 192; *outY = 224; }
				else { *outX = ( sw - w ) / 2; *outY = ( sh - h ) / 2; }
				{
					char m[ 96 ];
					( void ) sprintf( m, "movie dest: buffer %dx%d -> rect (%d,%d) size %dx%d",
					                   sw, sh, *outX, *outY, w, h );
					NoteStr( m );
				}
			}

			// Open one file and pump it to completion. Returns frames presented, or -1 if it would not open.
			// PORT-SIDE pump, one movie: open @0x5b5640, wait/doframe/copy/next + end test @0x5b5ae0.
			int PlayOne( const char* path, int x, int y ) {
				void* hink;
				const BinkMeasured* m;
				unsigned int shown = 0;
				int tries;

				hink = BinkOpen( path, 0u );
				if ( !hink ) {
					char buf[ 256 ];
					( void) sprintf( buf, "BinkOpen('%s') failed", path );
					NoteStr( buf );
					NoteStr( BinkGetError() );   // shipped does exactly this pair @0x5b567b-0x5b5682
					return -1;
				}
				m = (const BinkMeasured*)hink;
				{
					char buf[ 128 ];
					( void ) sprintf( buf, "movie %s: %ux%u frames=%u", path, m->width, m->height,
					                   m->frame_total );
					NoteStr( buf );
				}
				// Bound the loop by WALL CLOCK, not by iteration count. The first version capped iterations at
				// 4000 and both movies hit it mid-playback (trace: "hit its 4000-frame guard", presented=68 of
				// 230) because BinkWait paces to real time and a skip iteration costs a Sleep(1) - 4000 loops is
				// ~4 seconds, while IPlogo alone is 230 frames (~9s at 25fps). An iteration cap measures the loop,
				// not the movie; the movie's own end condition is the shipped test below.
				const DWORD t0 = GetTickCount();
				while ( m->frame_cur < m->frame_total && ( GetTickCount() - t0 ) < 45000u ) {
					if ( BinkWait( hink ) ) { Sleep( 1 ); continue; }   // 0 == a frame is ready @0x5b5aeb
					BinkDoFrame( hink );
					for ( tries = 0; tries < 2; ++tries ) {
						if ( BlitFrame( hink, m->width, m->height, x, y ) ) { ++shown; break; }
					}
					if ( m->frame_cur >= m->frame_total ) { break; }     // shipped's end test @0x5b5a3e
					BinkNextFrame( hink );
				}
				if ( m->frame_cur < m->frame_total ) {
					Note( "movie ended early: cur=%u of total=%u (presented=%d) - if cur never moved, the +12 "
					    "offset is NOT the frame counter", (int)m->frame_cur, (int)m->frame_total );
				}
				Note( "movie done: frames presented=%d last cur seen=%d", (int)shown, (int)m->frame_cur );
				BinkClose( hink );   // @0x5b5830, and the handle slot is zeroed right after
				return (int)shown;
			}
		} // namespace

		// Gate semantics (changed 2026-10-23 after the user's normal run showed no logos): OPENVB_MOVIES was a
		// SECOND knob someone had to know about on top of the render gate, and ``OPENVB_RENDER=1 f3-run``
		// therefore showed a device but no intro - the user's report, verbatim. Now the movie gate DEFAULTS
		// to the render slot's decision: one knob (OPENVB_RENDER=1) is the whole story, because a movie
		// without a device can only ever decode into nothing. An explicit OPENVB_MOVIES still wins in both
		// directions - =0 keeps the intro off even with rendering on, =1 forces it even without (a decode-
		// only run). App-boot and CI stay exactly as they were: ungated means both off, and the boot check
		// never enters here.
		// PORT-SIDE gate — named absence, no shipped twin: the shipped game plays its intro logos
		// unconditionally (the trigger above fires right after "Client: connected to server"; there is
		// no environment-reading branch anywhere in the shipped open path cls_0x5b4f50::meth_0x5b5640).
		// OPENVB_MOVIES / OPENVB_RENDER are harness-only scaffolding added for device-gated boots and CI
		// (see the gate-semantics note below this block, changed 2026-10-23); ea-index has no "Gated" record.
		bool Gated() {
			char v[ 8 ];
			DWORD n = GetEnvironmentVariableA( "OPENVB_MOVIES", v, (DWORD)sizeof( v ) );
			if ( n != 0 && *v ) { return strcmp( v, "1" ) == 0; }
			n = GetEnvironmentVariableA( "OPENVB_RENDER", v, (DWORD)sizeof( v ) );
			return ( n != 0 && strcmp( v, "1" ) == 0 );
		}

		// PORT-SIDE wrapper over the shipped call @0x5b530b (arg loaded from the BinkOpenDirectSound IAT @0x5b5302).
		void SetupBinkSound() {
			// bink.h's recipe is `#define BinkSoundUseDirectSound(lpDS) BinkSetSoundSystem(BinkOpenDirectSound,
			// (u32)lpDS)` - the FIRST argument is the opener function, the second is the DirectSound argument.
			// Shipped calls it with the value loaded from the BinkOpenDirectSound IAT slot (mov eax,ds:0x647338
			// @0x5b5302, call @0x5b530b), which under a normal import lib IS the address of &BinkOpenDirectSound,
			// so writing the macro's expansion here reproduces it rather than approximating it. Passing
			// BinkOpenDirectSound's own address as the second arg, or 0 as the first, is the classic mis-port.
			// Movie audio then runs INSIDE binkw32.dll: shipped makes no BinkService calls at all.
			int r = BinkSetSoundSystem( &BinkOpenDirectSound, 0u );
			Note( "BinkSetSoundSystem(BinkOpenDirectSound,0) -> %d", r, 0 );
		}

		int PlayMovieFile( const char* path, int x, int y, int w, int h ) {
			( void ) w; ( void ) h;   // shipped passes the dest rect to the sprite object, not to the decoder;
			                          // the decoder always writes frame width/height. Kept in the signature
		                          // because the console command path has the rect to hand.
			return PlayOne( path, x, y );
		}

		// PORT-SIDE driver of shipped's queue: IPlogo opened @0x58a6b9, BISlogo enqueued @0x58a700.
		int PlayStartupMovies() {
			int opened = 0;
			if ( !Gated() ) { return 0; }

			// Shipped's strings, verbatim (0x69c74e-ish "movies\\BISlogo.mve" era, and "movies\\IPlogo.mve" at
			// the IPlogo call site @0x58a6b9). Lowercase directory. The bundle ships the directory as
			// "Movies", which is invisible on Windows but NOT on a case-sensitive host filesystem under wine,
			// so the fallback below exists: without it the port silently shows nothing on Linux and the
			// difference looks like a codec failure rather than a path failure. The fallback is tried only
			// after the faithful name fails, and the attempt is logged either way.
			static const char* kFirst[]  = { "movies\\IPlogo.mve", "Movies\\IPlogo.mve", 0 };
			static const char* kSecond[] = { "movies\\BISlogo.mve", "Movies\\BISlogo.mve", 0 };
			const char* p;
			int i;
			int frames;

			SetupBinkSound();

			for ( i = 0; i < 2; ++i ) {
				int dx = 0, dy = 0;
				CenterRect( 640, 320, &dx, &dy );
				p = ( i == 0 ) ? kFirst[0] : kSecond[0];
				frames = PlayOne( p, dx, dy );
				if ( frames < 0 ) {
					const char* alt = ( i == 0 ) ? kFirst[1] : kSecond[1];
					NoteStr( "retrying with the bundle's capitalised directory" );
					frames = PlayOne( alt, dx, dy );
				}
				if ( frames >= 0 ) { ++opened; }
			}
			Note( "PlayStartupMovies: opened %d of 2", opened, 0 );
			return opened;
		}
	} // namespace Movies
} // namespace F3

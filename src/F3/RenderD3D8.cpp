// Copyright 2024 OpenJE
// M-001 slice 1 implementation. Read the header comment first; every choice here is
// cited there. Slice 1 = device + clear + present and NOTHING else: no model, texture,
// GUI, or sprite code (the row's own discipline: "slice 1 lands alone or it is not slice 1").

#include "F3/RenderD3D8.hpp"

#include <windows.h>
#include <d3d8.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>



namespace F3 {
	namespace Render {

		static IDirect3D8*   g_d3d8   = 0;
		static IDirect3DDevice8* g_dev = 0;
		static int           g_state  = 0;      // 0 off / 1 live / 2 failed
		static unsigned long g_frames = 0;
		static FILE*         g_trace  = 0;

		// PORT-SIDE helper (M-001 slice 1): no shipped twin at 0x56B470 - the shipped window-era
		// gate 0x56B810 reads nCmdShow/registry, not env. Pure reimpl opt-in mechanics.
		static int Gated() {
			char v[ 8 ];
			DWORD n = GetEnvironmentVariableA( "OPENVB_RENDER", v, (DWORD)sizeof( v ) );
			return ( n != 0 && strcmp( v, "1" ) == 0 );   // the un-gated path: NOTHING happens
		}

		// PORT-SIDE helper (M-001 slice 1): diagnostics channel for a GUI-subsystem image whose
		// stderr is NULL (shipped logged via the OutputDebugString-era 0x56B390 stream, absent here).
		static void Trace( const char* fmt, int a, int b ) {
			if ( !g_trace ) {
				char path[ 512 ];
				DWORD n = GetEnvironmentVariableA( "OPENVB_RENDER_TRACE", path, (DWORD)sizeof( path ) );
				if ( n == 0 || n >= sizeof( path ) ) {
					if ( !Gated() ) { return; }   // un-gated and no explicit trace path: stay silent
					strcpy( path, "render-trace.log" );
				}
				g_trace = fopen( path, "a" );
				if ( !g_trace ) { return; }
			}
			fprintf( g_trace, "[render] " );
			fprintf( g_trace, fmt, a, b );
			fprintf( g_trace, "\n" );
			fflush( g_trace );
		}

		// The guard, both toolchains, deliberately narrow: Direct3DCreate8 and
		// GetAdapterDisplayMode SUCCEEDED on this host already (M-001 measurements);
		// CreateDevice is the call that faulted (exit 40) when wined3d had no GL context.
		// MSVC: real SEH. mingw-w64 has no __try; a SIGSEGV bridge (the CRT translates
		// the exception into the signal on its SEH/sjlj path) is best-effort and
		// documented as such - the un-gated worst case is exactly the crash the guard
		// exists to avoid, and the gate defaults OFF, so CI never enters here unarmed.
		static HRESULT GuardedCreateDevice( UINT adapter, D3DDEVTYPE type, HWND hwnd,
		                                    DWORD flags, D3DPRESENT_PARAMETERS* pp,   // NON-const: the 2002-generation SDK
		                                    // header declares CreateDevice that way (C2664 caught live)
		                                    IDirect3DDevice8** out ) {
			HRESULT hr = NOERROR;
#ifdef _MSC_VER
			__try {
				hr = g_d3d8->CreateDevice( adapter, type, hwnd, flags, pp, out );
			}
		// PORT-SIDE guard: the shipped counterpart had no such handler - measured evidence: the AV at
		// d3d8+0x01305A (six-digit form of the RVAS) in HappyEagle w12 run (2026-10-21) is that
// unguarded crash, byte-recorded; the shipped loader anchor era is 0x56B470 window-side.
			__except ( EXCEPTION_EXECUTE_HANDLER ) {
				hr = (HRESULT)GetExceptionCode();
				*out = 0;
			}
#else
			// mingw-w64 has no __try. POSIX bridge: SIGSEGV (mingw's CRT re-raises the
			// faulting exception as this signal on its i686 SEH/sjlj translator) longjmps
			// back onto the guarded call. BEST-EFFORT by explicit admission: if the bridge
			// does not fire, the outcome equals today's measured behavior (process exit 40)
			// and ONLY an opted-in run can reach this line - CI and app-boot stay un-gated.
			static sigjmp_buf g_renderJmp;
			static volatile sig_atomic_t g_renderArmed = 0;
			struct Bridge {
				static void onSignal( int sig ) { (void)sig; if ( g_renderArmed ) siglongjmp( g_renderJmp, 1 ); }
			};
			void ( *oldSegv )( int ) = signal( SIGSEGV, Bridge::onSignal );
			g_renderArmed = 1;
			if ( sigsetjmp( g_renderJmp, 1 ) == 0 ) {
				hr = g_d3d8->CreateDevice( adapter, type, hwnd, flags, pp, out );
				g_renderArmed = 0;
			} else {
				hr = (HRESULT)0xC0000005;   // access violation, synthesized
				*out = 0;
			}
			signal( SIGSEGV, oldSegv );
#endif
			return hr;
		}


		// PORT-SIDE phase hook (M-001 slice 1): earliest port-controlled point; runs before the
		// 0x56B470 window creation called from the 0x56B810 prologue era. No shipped twin.
		void PreInit() {
			if ( g_d3d8 || g_state ) { return; }
			// probe: log the verdict even when the gate reads false, so "gate invisible
			// at entry" vs "function never called" separate by the trace, not by theory
			if ( Gated() ) { Trace( "PHASE1 enter, gate OK (max-free VA)", 0, 0 ); }
			else           { Trace( "PHASE1 enter, GATE READS FALSE at Main() entry", 0, 0 ); return; }
			if ( 0 ) { return; }
			g_d3d8 = Direct3DCreate8( D3D_SDK_VERSION );
			if ( !g_d3d8 ) {
				Trace( "Direct3DCreate8 returned NULL at PRE-init too - VA hypothesis weakened", 0, 0 );
				g_state = 2;
				return;
			}
			g_state = 3;   // preloaded, device not yet created
			Trace( "Direct3DCreate8 SUCCEEDED pre-Startup (VA/ORDER was the wall)", 0, 0 );
		}

		// PORT-SIDE slot init (M-001 slice 1): runs at the 0x56B470 window-create return; the shipped
		// frame there goes to its Gfx subsystem (device era 0x509DC1 chain), not an env-gated path.
		void InitOnce( void* hwnd ) {
			HWND window = (HWND)hwnd;
			if ( g_state == 2 || g_state == 1 ) { return; }   // failed, or already live
			if ( !Gated() ) { return; }
			Trace( "PHASE2 opt-in enabled, hwnd=0x%08X", (int)(UINT_PTR)window, 0 );
			if ( !g_d3d8 ) {                              // PreInit skipped/failed to run early
				g_d3d8 = Direct3DCreate8( D3D_SDK_VERSION );
				if ( !g_d3d8 ) {
					Trace( "Direct3DCreate8 returned NULL", 0, 0 );
					g_state = 2;
					return;
				}
			}
			D3DDISPLAYMODE dm;
			memset( &dm, 0, sizeof( dm ) );
			HRESULT hrm = g_d3d8->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &dm );
			Trace( "GetAdapterDisplayMode hr=0x%08X format=0x%08X", (int)hrm, (int)dm.Format );
			D3DPRESENT_PARAMETERS pp;
			memset( &pp, 0, sizeof( pp ) );
			pp.Windowed               = TRUE;
			pp.hDeviceWindow          = window;
			pp.BackBufferWidth        = 0;    // 0 = client-area default in windowed mode
			pp.BackBufferHeight       = 0;
			pp.BackBufferFormat       = dm.Format;   // the demo's adapter format where knowable
			pp.BackBufferCount        = 1;
			pp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
			pp.MultiSampleType        = D3DMULTISAMPLE_NONE;
			pp.EnableAutoDepthStencil = FALSE;
			pp.Flags                  = 0;
			pp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;   // d3d8 NAME (the
            // 2002-era header has no PresentationInterval member - d3d9 renamed it; C2039 live-caught)
			IDirect3DDevice8* dev = 0;
			// SOFTWARE vertex processing: the clear needs no TnL hardware and the flag
			// widens backend reach (wined3d AND DXVK both take it) - slice 1 optimizes
			// for "a device exists at all", not for throughput.
			HRESULT hr = GuardedCreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
			                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &dev );
			if ( FAILED( hr ) || !dev ) {
				Trace( "CreateDevice FAILED hr=0x%08X - logged no-device state, not a crash",
				       (int)hr, 0 );
				g_state = 2;
				return;
			}
			g_dev = dev;
			g_state = 1;
			Trace( "CreateDevice OK; magenta clear+present armed per frame", (int)hr, 1 );
		}

		// PORT-SIDE frame hook: placed at the 0x5ACE00 GameStateLoop slot-walk return (shipped walk
		// anchor: the vft[3] call 0x5ACDEA). After-walk = after the world pump, per the row.
		void Tick() {
			if ( g_state != 1 || !g_dev ) { return; }
			IDirect3DSurface8* bb = 0;
			if ( FAILED( g_dev->GetRenderTarget( &bb ) ) || !bb ) {
				Trace( "GetRenderTarget failed - no-device from here on", 0, 0 );
				g_state = 2;
				return;
			}
			HRESULT hc = g_dev->SetRenderTarget( bb, 0 );   // d3d8 arity: (target, pNewWindow=NULL) - C2660 live-caught   // backbuffer as own target (slice-1 shape)
			hc = g_dev->Clear( 0, 0, D3DCLEAR_TARGET, 0xFFFF00FFu, 1.0f, 0 );   // magenta (6 args:
			                                                                   // Count, pRects, Flags,
			                                                                   // color, z, stencil)
			if ( SUCCEEDED( hc ) ) { hc = g_dev->Present( 0, 0, 0, 0 ); }
			bb->Release();
			if ( FAILED( hc ) ) {
				Trace( "clear/Present failed hr=0x%08X - no-device from here on", (int)hc, 0 );
				g_state = 2;
				return;
			}
			++g_frames;
			if ( g_frames <= 3u || ( g_frames % 300u ) == 0u ) {
				Trace( "presented frame %d (magenta)", (int)g_frames, 0 );
			}
		}

		// PORT-SIDE teardown: runs at the 0x56B1A0 DestroyGameWindow entry; shipped tears its Gfx
		// subsystem via the slot table before the window (different shape, same frame position for us).
		void Shutdown() {
			if ( g_dev ) { g_dev->Release(); g_dev = 0; }
			if ( g_d3d8 ) { g_d3d8->Release(); g_d3d8 = 0; }
			if ( g_state ) { Trace( "shutdown after %d presented frames", (int)g_frames, 0 ); }
			g_state = 0;
			if ( g_trace ) { fclose( g_trace ); g_trace = 0; }
		}

		// PORT-SIDE telemetry accessor (M-001): no shipped twin; shipped side is the byte_707CE4
		// ready-flag era, which this slot deliberately does not touch.
		int DeviceState() { return g_state; }

		// PORT-SIDE diagnostic (M-001): settles execution-reach questions with bytes. No shipped
// twin anywhere in the 0x56B810 prologue era; invented during the probe-vs-theory fight.
		void Probe( const char* line ) {
			if ( !Gated() && !GetEnvironmentVariableA( "OPENVB_RENDER_TRACE", 0, 0 ) ) { return; }
			if ( !g_trace ) {
				char path[ 512 ];
				DWORD n = GetEnvironmentVariableA( "OPENVB_RENDER_TRACE", path, (DWORD)sizeof( path ) );
				if ( n == 0 || n >= sizeof( path ) ) { strcpy( path, "render-trace.log" ); }
				g_trace = fopen( path, "a" );
				if ( !g_trace ) { return; }
			}
			fprintf( g_trace, "[render] probe: %s\n", line );
			fflush( g_trace );
		}

	} // namespace Render
} // namespace F3

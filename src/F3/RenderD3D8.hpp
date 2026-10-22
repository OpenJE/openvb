// Copyright 2024 OpenJE
//
// M-001 SLICE 1 - the d3d8 render slot. NOT a renderer: one create, one clear, one
// Present per frame, behind an opt-in env gate, so that "can we see anything" becomes
// a measured answer instead of a debate. Posture inherited from tools/render-device-
// probe.sh (ZenRaven, 16a3c8a): rendering is OPT-IN (OPENVB_RENDER=1), the device
// creation is GUARDED (a fault must land as a logged no-device state, never a crash),
// and diagnostics go to a FILE channel (OPENVB_RENDER_TRACE=<path>) because
// OutputDebugString is invisible under wine without a debugger and fprintf(stderr)
// kills F3.exe outright (GUI-subsystem image, NULL stderr). Default-off keeps every
// CI leg and app-boot exactly as they are: this file must never touch behavior on the
// un-gated path.
//
// Frame shape per the row: Present happens AFTER the world pump (the 8-slot walk),
// mirroring the original's message-loop-then-tick order; the slot is a port-side
// subsystem beside that walk, NOT a 9th entry in the byte-accurate 0x6FF6C8 table.

#ifndef F3_RENDERD3D8_HPP_
#define F3_RENDERD3D8_HPP_

namespace F3 {
	namespace Render {
		// Call once after the game window exists. No-op unless OPENVB_RENDER=1.
		// Takes the HWND as a parameter (void* to keep windows.h out of this header):
		// g_window in F3.hpp is a namespace-scope STATIC (decompiler-shape, internal
		// linkage per TU) - reaching for it from another TU is a different window, and
		// the link proved it (LNK2019 live-caught). Pass the value; touch nothing global.
		void InitOnce( void* hwnd );

		// Call once per frame AFTER the subsystem walk. No-op unless enabled and a device
		// exists; clears magenta (0xFFFF00FF - a colour that cannot be confused with an
		// uninitialised black buffer) and Presents exactly once.
		void Tick();

		void Shutdown();

		// 0 = not enabled, 1 = device live, 2 = enabled but no device (failure logged).
		int DeviceState();
	} // namespace Render
} // namespace F3

#endif // F3_RENDERD3D8_HPP_

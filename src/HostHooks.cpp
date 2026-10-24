// Copyright 2024 OpenJE
//
// JE::Host fs-hook definitions (one small TU so suite exes can link the
// engine<->game bridge without the monolithic game TU src/F3.cpp). The
// implementations delegate to the promoted non-static F3:: functions — the
// shipped bodies live once, in F3.cpp (0x619E42 + 0x4C56B0, both read whole).

#include "JE/HostHooks.hpp"
#include "F3.hpp"
#include <windows.h>
#include <io.h>

// The shipped fs bodies (moved here from src/F3.cpp 2026-10-24 — one
// definition, the TU the suite exes can link without the monolithic game TU).

namespace F3 {

	// 0x619E42 - CreateDirectoryA wrapper: 0 on success, -1 (with _dosmaperr) on failure.
	// NOTE: the original resolves to the CRT import thunks area; the reimplementation uses the
	// Win32 API directly, matching observed behavior (ERROR_SUCCESS -> 0).
	int CreateDirectoryWithErrorMapping( const char *path ) {
		DWORD lastError;

		if ( CreateDirectoryA( path, NULL ) ) {
			lastError = 0;
		} else {
			lastError = GetLastError();
		}
		if ( !lastError ) {
			return 0;
		}
		return -1;
	} // CreateDirectoryWithErrorMapping

	// 0x4C56B0 - "directory entry exists": _findfirst/_findnext over the path pattern, returning 1
	// when some match carries the directory attribute (0x10), 0 otherwise.
	char PathIsDirectoryExists( LPCSTR lpFileName ) {
		struct _finddata_t entry;
		long handle;

		handle = _findfirst( lpFileName, &entry );
		if ( handle == -1L ) {
			return 0;
		}
		while ( ( entry.attrib & _A_SUBDIR ) == 0 ) {
			if ( _findnext( handle, &entry ) == -1L ) {
				_findclose( handle );
				return 0;
			}
		}
		_findclose( handle );
		return 1;
	} // PathIsDirectoryExists

} // namespace F3

namespace JE {
	namespace Host {
		// the JE::Host hook (declared in JE/HostHooks.hpp; the shipped call fan
		// into the game-module F3 function @0x4c5830/0x4c588a in sub_4C5750).
		int CreateDirectoryWithErrorMapping( const char* path ) {
			return F3::CreateDirectoryWithErrorMapping( path );   // 0x619E42 (the shipped body above)
		}
		// the JE::Host hook (the shipped sub_4C56B0 exists-probe call @0x4c5830).
		bool PathIsDirectoryExists( const char* path ) {
			return F3::PathIsDirectoryExists( path ) != 0;        // 0x4C56B0 (the shipped body above)
		}
	} // namespace Host
} // namespace JE

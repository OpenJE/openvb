// Copyright 2024 OpenJE

#include <ios>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <shlobj.h>
#include <cstdio>

#include "JE.hpp"
#include "F3.hpp"
#include "JE/cls_0x4d8d70.hpp"
#include "JE/Subsystem.hpp"
#include "JE/cls_0x50db20.hpp"
#include "tracing.hpp"
#include "JE/MemorySystem.hpp"
#include "JE/Resources.hpp"
#include "JE/Scripting.hpp"
#include "JE/Strings.hpp"
#include "JE/Network.hpp"
#include "F3/RenderD3D8.hpp"   // M-001 slice 1 (opt-in render slot; un-gated path is a strict no-op)
#include "F3/GameAnimationCache.hpp"
#include "F3/GameWorld.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/ModelCache.hpp"
#include "F3/GameNetwork.hpp"
#include "F3/SimDigest.hpp"   // T-026 per-frame digest (header-only: no build-config change)
#include <stdlib.h>              // getenv for OPENVB_SIM_DIGEST
#include "JE/HostHooks.hpp"
#include "JE/SS/EngineStringSystem.hpp"
#include "JE/VFX_Parameter.hpp"

namespace F3 {
	// Global variable definitions

	// Mirror of the original .data table at 0x6FF6C8..0x6FF70C (8 interleaved {factory, object}
	// pairs, stride 8). Factory addresses observed in the original data section: 0x5B3270
	// (InitializeMemorySystem), 0x45E110, 0x45E1C0, 0x45E330 (InitializeStrings), 0x45E170,
	// 0x45E270 (InitializeModelCache), 0x45E2D0, 0x45E210 (InitializeGameWorld); the object slots
	// start zeroed and are filled by F3::Startup (0x5ACD60). See JE/Subsystem.hpp for the vft
	// contract (vft[1] Initialize after construction, vft[3] Tick per frame, vft[2] Terminate +
	// vft[0] deleting destructor at shutdown). Slot->initializer mapping for 0x45E110/0x45E170/
	// 0x45E1C0/0x45E2D0 is inferred (Resources/Scripting/Network/GameAnimationCache) pending
	// per-factory verification against the original.
	SubsystemSlot g_subsystemSlots[ 8 ] = {
		{ InitializeMemorySystem, NULL },
		{ InitializeResources, NULL },
		{ InitializeScripting, NULL },
		{ InitializeStrings, NULL },
		{ InitializeNetwork, NULL },
		{ InitializeModelCache, NULL },
		{ InitializeGameAnimationCache, NULL },
		{ InitializeGameWorld, NULL },
	};

	// 0x70C668 — the command registry. The original builds this during static initialisation
	// (F3::GLOBAL__I_command_registry @ 0x643f60 -> JE::cls_0x4d8d70::ctor_0x59f500, then _atexit of the
	// 0x646b90 thunk -> JE::cls_0x4d8d70::meth_0x59f4d0). It is a real object, never heap-allocated and
	// never NULL, so a namespace-scope instance with the reconstructed constructor/destructor is the
	// faithful model: MSVC runs the ctor before WinMain and registers the dtor with atexit, exactly like
	// the hand-rolled _GLOBAL__I_ walker in the original.
	JE::cls_0x4d8d70 g_commandRegistryRoot;

	// 0x70C2C0/0x70C2CC region (F3::global_vfx_parameter_*) - the engine parameter tree; the
	// original statically constructs it before WinMain (GLOBAL__I). sys.ini is parsed into it by
	// JE::Resources::ctor (after ResourceSystem init, matching 0x59c1e9 ordering).
	JE::VFX_Parameter g_vfxParameterTree;

	// 0x70C2D8 (F3::global_ess_0x70c2d8) - static EngineStringSystem; constructed by the
	// original GLOBAL__I stub at 0x643560 with _atexit thunk 0x646910.
	JE::SS::EngineStringSystem g_engineStringSystem;

    // Initialize* function implementations
    void* InitializeMemorySystem() {
    	JE::MemorySystem* memory_system = new JE::MemorySystem();
    	if ( memory_system ) {
    		memory_system->ctor_0x5b31c0();
    	}
    	return memory_system;
    }

    void* InitializeResources() {
    	JE::Resources* resources = new JE::Resources();
    	if ( resources ) {
    		resources->ctor_0x59c110();
    	}
    	return resources;
    }

    void* InitializeScripting() {
    	JE::Scripting* scripting = new JE::Scripting();
    	if ( scripting ) {
    		scripting->ctor_0x59c040();
    	}
    	return scripting;
    }

    void* InitializeStrings() {
    	JE::Strings* strings = new JE::Strings();
    	if ( strings ) {
    		strings->ctor_0x59c240();
    	}
    	return strings;
    }

    // 0x45E170 - original: new(0x30) + JE::Network::ctor_0x543890, then patches the vftptr to
    // F3::GameNetwork::vftable_0x663efc. Modeled directly as constructing the derived object.
    void* InitializeNetwork() {
    	F3::GameNetwork* network = new F3::GameNetwork();
    	if ( network ) {
    		network->ctor_0x543890();
    	}
    	return network;
    }

    // 0x45e270 - creates the JE::ModelCache singleton (operator new(0x20) + ctor_0x585c00)
    void* InitializeModelCache() {
    	JE::ModelCache* model_cache = new JE::ModelCache();
    	return model_cache;
    }

    void* InitializeGameAnimationCache() {
    	F3::GameAnimationCache* cache = new F3::GameAnimationCache();
    	if ( cache ) {
    		cache->ctor_0x45c6d0();
    	}
    	return cache;
    }

    // World-server singleton slot (original 0x70BD98 - GetWorldServerInstance_0x401150 reads
    // *(0x70BD98)+16; our GameWorld stands in until the full MI layout is reconstructed).
    static F3::GameWorld* g_worldServerInstance = NULL;
    F3::GameWorld* GetWorldServerInstance() { return g_worldServerInstance; }

    void* InitializeGameWorld() {
    	F3::GameWorld* world = new F3::GameWorld();
    	if ( world ) {
    		world->ctor_0x470340();
    		g_worldServerInstance = static_cast<F3::GameWorld*>( world );
    	}
    	return world;
    }

	// 0x619E42 - CreateDirectoryA wrapper: 0 on success, -1 (with _dosmaperr) on failure.
	// NOTE: the original resolves to the CRT import thunks area; the reimplementation uses the
	// Win32 API directly, matching observed behavior (ERROR_SUCCESS -> 0).
	static int CreateDirectoryWithErrorMapping( const char *path ) {
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
	static char PathIsDirectoryExists( LPCSTR lpFileName ) {
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

	// 0x4C5750 - recursive directory creation ("MakeAllDirectories"): strips a trailing separator,
	// skips the drive/UNC prefix, then creates every separator-delimited prefix that does not
	// exist yet; finally ensures the full path. Returns 1 when the full directory exists.
	static char CreateAllDirectories( const char *a1 ) {
		char  path[ MAX_PATH + 4 ];
		char *pPath;
		size_t length;

		strcpy( path, a1 );
		length = strlen( path );
		if ( path[ length ] == '\\' || path[ length ] == '/' ) {
			path[ length ] = 0;
		}

		pPath = path;
		if ( path[0] == '\\' || path[0] == '/' ) {
			if ( path[1] == '\\' || path[1] == '/' ) {
				// UNC \\server\share\...: skip two path components.
				int components = 2;
				pPath = path + 2;
				do {
					while ( *pPath != '\\' ) {
						if ( *pPath == '/' ) {
							break;
						}
						++pPath;
					}
					++pPath;
					--components;
				} while ( components );
			}
		} else {
			// Local "C:\...": step past the drive portion.
			char c = *pPath;
			while ( c != '/' ) {
				c = *++pPath;
				if ( c == '\\' ) {
					++pPath;
					break;
				}
			}
		}
		while ( *pPath == '\\' || *pPath == '/' ) {
			++pPath;
		}

		if ( *pPath ) {
			do {
				if ( *pPath == '\\' || *pPath == '/' ) {
					*pPath = 0;
					if ( !PathIsDirectoryExists( path ) && CreateDirectoryWithErrorMapping( path ) == -1 ) {
						return 0;
					}
					*pPath = '\\';
				}
			} while ( *++pPath );
		}
		return PathIsDirectoryExists( path ) || CreateDirectoryWithErrorMapping( path ) != -1;
	} // CreateAllDirectories

	// 0x56B000
	char SetupSaveDirectory( CHAR *pcModulePath ) {
		tracing::instrument( tracing::LOCATION, "pcModulePath=%s", pcModulePath );

		int   nCharIndexAbsolute;
		char  cCurrentCharAbsolute;
		int   nCharIndexRelative;
		char  cCurrentCharRelative;
		char* pcSaveDirectoryRelative;
		char  cResult;
		char  szPersonalFolderPath[ MAX_PATH ];

		if ( !getcwd( g_szCurrentWorkingDirectory, MAX_PATH ) ) {
			JE::FatalError( "Unable to retrieve current working directory." );
		}
		if ( SHGetFolderPath( (HWND)0x0, CSIDL_FLAG_CREATE | CSIDL_PERSONAL, (HANDLE)0x0, 0, szPersonalFolderPath ) ) {
			JE::FatalError( "Unable to retrieve personal folder." );
		}

		nCharIndexAbsolute = 0;
		do {
			cCurrentCharAbsolute = szPersonalFolderPath[ nCharIndexAbsolute ];
			g_szSaveDirectoryAbsolute[ nCharIndexAbsolute ] = cCurrentCharAbsolute;
			nCharIndexAbsolute = nCharIndexAbsolute + 1;
		} while ( cCurrentCharAbsolute );

		// 0x56B087: the save folder name is the MODULE FILE NAME (without extension), not the
		// personal folder again - _splitpath reuses the szPersonalFolderPath buffer as the file part.
		_splitpath( pcModulePath, NULL, NULL, szPersonalFolderPath, NULL );

		nCharIndexRelative = 0;
		do {
			cCurrentCharRelative = szPersonalFolderPath[ nCharIndexRelative ];
			g_szSaveDirectoryRelative[ nCharIndexRelative ] = cCurrentCharRelative;
			nCharIndexRelative = nCharIndexRelative + 1;
		} while ( cCurrentCharRelative );

		pcSaveDirectoryRelative = GetSaveDirectoryRelative();
		F3::PathConcat( g_szSaveDirectoryAbsolute, pcSaveDirectoryRelative );
		cResult = PathIsDirectoryExists( g_szSaveDirectoryAbsolute );
		if ( !cResult ) {
			cResult = CreateAllDirectories( g_szSaveDirectoryAbsolute );
			if ( !cResult ) {
				JE::FatalError( "Unable to create save folder." );
			}
		}
		return cResult;
	} // SetupSaveDirectory

	/*
	// 0x56B220
	void SetupConfigFile( uint nCmdShow ) {
	} // SetupConfigFile
	*/

	// 0x56B100
	int SetupLogFile() {
		tracing::instrument( tracing::LOCATION, "" );
		char* pcSaveDirectoryAbsolute;
		char  szFilePath[ MAX_PATH ];

		// The original inlines a strcpy of the absolute save dir into the stack buffer
		// (IDA shows the pointer-difference loop `pcSaveDirectoryAbsolute[nFileNameOffset]`).
		pcSaveDirectoryAbsolute = GetSaveDirectoryAbsolute();
		strcpy( szFilePath, pcSaveDirectoryAbsolute );
		char* pcSaveDirectoryRelative = GetSaveDirectoryRelative();
		PathConcat( szFilePath, pcSaveDirectoryRelative );
		// The original writes the extension at the NUL terminator of szFilePath: IDA shows
		// cExtensionBuffer at [ebp-0x109] - the byte directly in front of szFilePath[0] - and scans
		// `while (*++ptr)` from there, which terminates exactly at szFilePath's NUL. Well-defined
		// equivalent: append at strlen(szFilePath).
		strcpy( szFilePath + strlen( szFilePath ), ".log" );
		//return F3::OpenLogFile( szFilePath );   // 0x498000, not reconstructed yet
		return 0;
	} // SetupLogFile

	/*
	// 0x498000
	int OpenLogFile( const char* pcLogFilePath ) {
		std::ios* ios;
		std::ios::iostate ioState;
		std::ios::iostate nFlagsToSet;
		int nFileOpenResult;
		char szLogFile[ MAX_PATH ]
		unsigned int securityCookieXorResult;
		unsigned int retaddr;

		if ( !F3::g_LogFilebuf.open( pcLogFilePath, std::ios::out | std::ios::in ) ) {
			errorFlagsBuffer = &byte_707CF8[*(*byte_707CF8 + 4)];
			nFlagsToSet = ios->rdstate() | std::ios_base::eofbit;
			if ( !ios->good() )  {
				nFlagsToSet = ios->rdstate() | std::ios::failbit | std::ios::eofbit;
			}
			ios->clear( nFlagsToSet );
		}
		fileOpenResult = *(*byte_707CF8 + 4);
		if ( (byte_707CFC[fileOpenResult] & 6) != 0 ) {
			JE::FatalError( "Unable to open log file %s.", szLogFile );
		}
		return fileOpenResult;
	}
	*/

	// 0x56AF10
	char *GetSaveDirectoryRelative() {
		return g_szSaveDirectoryRelative;
	}

	// 0x56AF00
	char *GetSaveDirectoryAbsolute() {
		return g_szSaveDirectoryAbsolute;
	}

	// 0x4C58A0
	// NOTE on the original's typed decompilation: IDA renders the destination as
	// &string_a[strlen(string_a)] where string_a is viewed as `const char *`; the concrete instance
	// is always the writable global g_szSaveDirectoryAbsolute, i.e. the function appends
	// "\\" + string_b in place onto the first argument's buffer.
	unsigned int PathConcat( const char *pcPathA, const char *pcPathB ) {
		tracing::instrument( tracing::LOCATION, "pcPathA=\"%s\", pcPathB=%s", pcPathA, pcPathB );

		char *pcDst = (char *)pcPathA;
		unsigned int nPathALen;
		char cLastCharPathA;
		unsigned int nPathBLen;

		nPathALen = strlen( pcDst );
		cLastCharPathA = pcDst[ nPathALen - 1 ];
		if ( cLastCharPathA != '\\' && cLastCharPathA != '/' ) {
			strcpy( pcDst + nPathALen, "\\" );
		}
		nPathBLen = strlen( pcPathB ) + 1;
		memcpy( pcDst + strlen( pcDst ), pcPathB, nPathBLen );
		return nPathBLen;
	}

	// 0x48CCA0
	int SetStartupTime( int nTime ) {
		tracing::instrument( tracing::LOCATION, "nTime=%d", nTime );

		int result;

		result = nTime;
		g_nStartupTimeMS = nTime;
		g_bIsStartupTimeSet = true;
		return result;
	}

	// 0x5ACE00
	int Main() {
		tracing::instrument( tracing::LOCATION, "" );
		F3::Render::Probe( "F3::Main entered (loop driver)" );   // frame-shape breadcrumb
		F3::Render::PreInit();   // idempotent second chance if WinMain skipped it
		F3::Startup();

		// T-026: opt-in digest (default off - the app must not create files nobody asked for).
		if ( const char* digestPath = getenv( "OPENVB_SIM_DIGEST" ) ) {
			if ( digestPath[0] ) { JE::Host::EnableSimDigest( digestPath ); }
		}
		F3::RegisterCommand("quit", reinterpret_cast<void (__cdecl *)()>(F3::Quit));
		while ( ProcessMessagesAndUpdateTime() ) {
			F3::GameStateLoop();
		}
		F3::Shutdown();
		return 1;
	}

	// 0x56AF60
	bool ProcessMessagesAndUpdateTime() {
		tracing::instrument( tracing::LOCATION, "" );

		DWORD nCurrentTime;
		int nElapsedTimeDiff;
		struct tagMSG messageStruct;
		UINT nRemoveMessage1;
		UINT nRemoveMessage2;

		if ( PeekMessageA( &messageStruct, 0, 0, 1u, nRemoveMessage1 ) ) {
			while ( messageStruct.message != 18 ) {
				TranslateMessage( &messageStruct );
				DispatchMessageA( &messageStruct );
				if ( !PeekMessageA( &messageStruct, 0, 0, 1u, nRemoveMessage2 ) ) {
					nCurrentTime = timeGetTime();
					nElapsedTimeDiff = nCurrentTime - F3::g_nSystemTimeMS + F3::g_nElapsedTimeMS;
					F3::g_nSystemTimeMS= nCurrentTime;
					F3::g_nElapsedTimeMS = nElapsedTimeDiff;
					return true;
				}
			}
			return false;
		}
		else {
			nCurrentTime = timeGetTime();
			nElapsedTimeDiff = nCurrentTime - g_nSystemTimeMS + g_nElapsedTimeMS;
			g_nSystemTimeMS= nCurrentTime;
			g_nElapsedTimeMS = nElapsedTimeDiff;
			return true;
		}
	}

	// 0x433930
	static bool __cdecl sub_433930( int param1, int param2 ) {
		tracing::instrument( tracing::LOCATION, "param1=0x%p, param2=0x%p", param1, param2 );

		const char* str2;

		if ( *(int*)( param2 + 24 ) < 0x10u ) {
			str2 = reinterpret_cast<const char*>( param2 + 4 );
		} else {
			str2 = *reinterpret_cast<const char**>( param2 + 4 );
		}

		if ( *(int*)( param1 + 24 ) < 0x10u ) {
			return _stricmp( reinterpret_cast<const char*>( param1 + 4 ), str2 ) < 0;
		} else {
			return _stricmp( *reinterpret_cast<const char**>( param1 + 4 ), str2 ) < 0;
		}
	}

	// 0x5ACD60
	// Original walk (0x5ACD60): initialize = 0x6FF6C8; while (initialize < 0x6FF708): if (factory) {
	// obj = factory(); initialize[1] = obj (0x5ACD7B); call [obj_vft + 4] = vft[1] (0x5ACD82); }
	void* Startup() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::LogDebugString( NULL, "Startup()\r\n" );

		JE::Subsystem* result = NULL;

		for ( int i = 0; i < 8; ++i ) {
			if ( g_subsystemSlots[i].factory ) {
				JE::Subsystem* object = reinterpret_cast<JE::Subsystem*>( g_subsystemSlots[i].factory() );
				g_subsystemSlots[i].object = object;   // 0x5ACD7B: initialize[1] = result
				result = object;
				object->Initialize();                  // 0x5ACD82: (*(*v2 + 4))(v2) -> vft[1]
			}
		}

		return result;
	} // Startup

	// 0x59F5D0
	// Original flow (disasm 0x59f5d0..0x59f6c4):
	//   0x59f5fc-0x59f60c  build a stack tempString (cls_0x50db20: string sub-object at +4 with
	//                      capacity=15 @+0x18, size=0 @+0x14, buffer[0]=0 @+0x04)
	//   0x59f623           cls_0x50db20::meth_0x401bd0(ArgList, strlen(ArgList))  (assign)
	//   0x59f63a           v2 = cls_0x4d8d70::meth_0x545710 lower_bound over the registry (0x70C668)
	//   0x59f649           if (v2 == header) skip the duplicate test entirely (header key never read)
	//   0x59f674           _stricmp(query, nodeKey) >= 0 (lower_bound guarantees nodeKey >= query, so
	//                      this means equality) -> FatalError("Command %s already registered.")
	//   0x59f6b5           *cls_0x4d8d70::meth_0x59f540(&clone) = func_ptr  (insert-or-get returns the
	//                      node's value slot at node+0x28; the clone/insert path is folded into
	//                      InsertOrGet here)
	//   0x59f6bd           if (capacity >= 0x10) free(heap buffer)
	void RegisterCommand( const char* ArgList, void (__cdecl *func_ptr)() ) {
		tracing::instrument( tracing::LOCATION, "ArgList=%s", ArgList );

		// tempString in SSO state, mirroring 0x59f5fc-0x59f60c (object word, buffer[0], size, capacity).
		JE::cls_0x50db20 tempString;
		byte* raw = reinterpret_cast<byte*>( &tempString );
		raw[0] = 0;
		raw[4] = 0;
		*reinterpret_cast<dword*>( raw + 0x14 ) = 0;    // size
		*reinterpret_cast<dword*>( raw + 0x18 ) = 15;   // capacity (inline)

		tempString.meth_0x401bd0( ArgList, static_cast<int>( strlen( ArgList ) ) );

		void* node = g_commandRegistryRoot.LowerBound( &tempString );
		if ( node != reinterpret_cast<void*>( g_commandRegistryRoot.mbr_0x4 ) &&
		     _stricmp( ArgList, JE::cls_0x4d8d70::NodeKeyText( node ) ) == 0 ) {
			JE::FatalError( "Command %s already registered.", ArgList );
			return;
		}

		void** valueSlot = reinterpret_cast<void**>( g_commandRegistryRoot.InsertOrGet( &tempString ) );
		*valueSlot = reinterpret_cast<void*>( func_ptr );

		if ( *reinterpret_cast<dword*>( raw + 0x18 ) >= 0x10 ) {
			free( *reinterpret_cast<void**>( raw + 4 ) );   // 0x59f6c4: j__free(String1[0])
		}
	}

	// 0x5ACDE0
	// Original walk (0x5ACDE0): functions = 0x6FF6CC (first object slot); while (functions <
	// 0x6FF70C): result = (*(**functions + 12))(*functions) -> obj->vft[3](obj); functions += 8.
	// No null guard in the original - a failed construction faults identically there.
	void GameStateLoop() {
		tracing::instrument( tracing::LOCATION, "" );

		for ( int i = 0; i < 8; ++i ) {
			g_subsystemSlots[i].object->Tick();   // vft[3], 0x5ACDEA: call *[eax+0Ch]
		}

		// M-001 slice 1 render slot: SAME placement argument as the digest below - a port-side
		// hook OUTSIDE the byte-accurate 8-slot table (no 9th entry), positioned AFTER the walk
		// so the Present lands after the world pump (the row's frame-order requirement).
		// No-op unless opted in and a device is live.
		F3::Render::Tick();

		// T-026 digest, OUTSIDE the faithful 8-slot walk: the table at 0x6FF6C8..0x6FF70C is a
		// byte-accurate mirror and must not gain a 9th entry. No-op unless the env var opened a
		// file, so shipped behaviour is unchanged.
		JE::Host::DumpSimState();
	} // GameStateLoop

	// Original walk (0x5ACDA0): esi = 0x6FF70C; do { ecx = [-8] (obj); call [obj_vft + 8] (vft[2])
	// unconditionally at 0x5ACDB8; if (ecx) { push 1; call [obj_vft + 0] -> scalar deleting destructor
	// at 0x5ACDC6 } ; store slot = 0 (0x5ACDC8); esi -= 8; } while (esi > 0x6FF6CC).
	void Shutdown() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::LogDebugString( NULL, "Shutdown()\r\n" );

		JE::Host::CloseSimDigest();   // T-026: flush the digest before tearing the world down

		for ( int i = 7; i >= 0; --i ) {
			JE::Subsystem* object = g_subsystemSlots[i].object;
			// The original calls vft[2] unguarded (0x5ACDB8); a null object would already have faulted
			// inside Startup (0x5ACD82), so the guard below is behaviorally unreachable.
			if ( object ) {
				object->Terminate();                       // 0x5ACDB8: call [vft + 8]
				delete object;                             // 0x5ACDC6: vft[0] deleting destructor, flag 1
			}
			g_subsystemSlots[i].object = NULL;              // 0x5ACDC8: slot cleared
		}
	} // Shutdown

	/*
	// 0x497B70
	std::filebuf* ShutdownGlobalLogStream() {
		std::filebuf* result;
		std::ios* ios;
		std::ios::iostate ioState;

		result = g_bIsLogStreamInitialized;
		if ( g_bIsLogStreamInitialized ) {
			result = g_LogFilebuf.close();
			if ( !result ) {
				ios = g_LogIos;
				ioState = ios->rdstate() | std::ios_base::eofbit;
				if ( !ios->good() ) {
					ioState |= ios->rdstate() | std::ios::failbit | std::ios::eofbit;
				}
				ios->clear( ioState );
				return result;
			}
		}
		return result;
	}
	*/

	// 0x56AFF0
	void F3::Quit() {
		tracing::instrument( tracing::LOCATION, "" );
		PostQuitMessage( 1 );
	}

	namespace Display {
		// 0x56B1B0
		LRESULT CALLBACK WinProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) {
			tracing::instrument( tracing::LOCATION, "hWnd=0x%p,Msg=%i,wParam=%i,lParamnCmdShow=%d", hWnd, Msg, wParam, lParam );

			if ( hWnd != g_window ) {
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
			if ( Msg == 16 ) {
				Quit();
				return 0;
			}
			else {
				if ( Msg == 28 && !wParam ) {
					//cls_0x4b28a0::meth_0x4b2ad0_4b2ad0( this: &unk_707E48, param_1: 0 );
				}
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
		}

		// 0x56B470
		BOOL CreateGameWindow( int nCmdShow ) {

			tracing::instrument( tracing::LOCATION, "nCmdShow=0x%i", nCmdShow );

			char *pcSaveDirectoryRelativePath;
			int nNameIndex;
			char cCurrentChar;
			int *pnParameterValuePtr;
			const char *pcParameterNameBufferAddress;
			const char *pcParameterNameBuffer;
			char *pcArgListPointer;
			int nParameterValue;
			DWORD *pParameterStruct;
			int *pnParameterValueResult;
			bool bIsParameterMatch;
			const char *pcWidthParameterValue;
			int *pnHeightParameterValue;
			const char *pcHeightAdjustedValue;
			int nSystemMetrics;
			struct HWND__ *pwinWindow;
			int nWindowTopPosition;
			int nWindowRightPosition;
			int nWindowBottomPosition;
			char szArgList[4];
			int nWindowWidth;
			int nWindowHeight;
			char szBuffer[4];
			void *pBlock;
			int nBlockSize;
			unsigned int nBufferSize;
			struct tagRECT windowRect;
			int *pnParameterName;
			char szTempBuffer[4];
			char *pcParameterNameBuffer_1;
			int nParameterIndex;
			unsigned int nLoopCounter;
			WNDCLASSA WndClass;
			CHAR szClassName[MAX_PATH];

			pcSaveDirectoryRelativePath = F3::GetSaveDirectoryRelative();
			nNameIndex = (szClassName - pcSaveDirectoryRelativePath);
			do {
				cCurrentChar = *pcSaveDirectoryRelativePath;
				pcSaveDirectoryRelativePath[nNameIndex] = *pcSaveDirectoryRelativePath;
				++pcSaveDirectoryRelativePath;
			}
			while ( cCurrentChar );
			WndClass.style = 11;
			WndClass.lpfnWndProc = WinProc;
			WndClass.cbClsExtra = 0;
			WndClass.cbWndExtra = 0;
			WndClass.hInstance = g_instance;
			WndClass.hIcon = (HICON)LoadImage( F3::g_instance, szClassName, 1u, 0, 0, 0x40u );
			WndClass.hCursor = LoadCursor( 0, IDC_ARROW/*0x7F00*/ );
			WndClass.hbrBackground = (HBRUSH)GetStockObject( 4 );
			WndClass.lpszMenuName = 0;
			WndClass.lpszClassName = szClassName;
			if ( !RegisterClass( &WndClass ) ) {
				JE::FatalError( "Unable to register window class." );
			}
			nWindowWidth = 800;
			nWindowHeight = 600;
			nLoopCounter = 15;
			nParameterIndex = 0;

			/*
			LOBYTE( pcParameterNameBuffer_1 ) = 0;
			cls_0x50db20::meth_0x401bd0( this: szTempBuffer, param_1: "Graphics", param_2: strlen( a1: "Graphics" ) );
			pnParameterValuePtr = JE::VFX_Parameter::meth_0x4339b0_4339b0( this: &F3::global_vfx_parameter_0x70c2cc, param_1: szTempBuffer );
			pnParameterName = pnParameterValuePtr;
			if ( pnParameterValuePtr == F3::global_vfx_parameter_0x70c2cc.int_0x4 ) {
				goto LABEL_13;
			}
			if ( pnParameterValuePtr[9] < 0x10 ) {
				pcParameterNameBufferAddress = (pnParameterValuePtr + 4);
			}
			else {
				pcParameterNameBufferAddress = pnParameterValuePtr[4];
			}
			pcParameterNameBuffer = pcParameterNameBuffer_1;
			if ( nLoopCounter < 0x10 ) {
				pcParameterNameBuffer = &pcParameterNameBuffer_1;
			}
			if ( _stricmp( pcParameterNameBuffer, pcParameterNameBufferAddress ) < 0 ) {
		LABEL_13:
				*szArgList = F3::global_vfx_parameter_0x70c2cc.int_0x4;
				pcArgListPointer = szArgList;
			}
			else {
				pcArgListPointer = &pnParameterName;
			}
			nParameterValue = *pcArgListPointer;
			if ( nLoopCounter >= 0x10 ) {
				j__free( pcParameterNameBuffer_1 );
			}
			nLoopCounter = 15;
			nParameterIndex = 0;
			LOBYTE( pcParameterNameBuffer_1 ) = 0;
			if ( nParameterValue != F3::global_vfx_parameter_0x70c2cc.int_0x4 ) {
				pnParameterStruct = (nParameterValue + 40);
				nBufferSize = 15;
				nBlockSize = 0;
				LOBYTE( pBlock ) = 0;
				cls_0x50db20::meth_0x401bd0( szBuffer, "width", strlen( "width" ) );
				pnParameterValueResult = *sub_4DF070( pnParameterStruct, szArgList, szBuffer );
				if ( nBufferSize >= 0x10 ) {
					free( Block: pBlock );
				}
				bIsParameterMatch = pnParameterValueResult == pnParameterStruct[1];
				nBufferSize = 15;
				nBlockSize = 0;
				LOBYTE( pBlock ) = 0;
				if ( !bIsParameterMatch ) {
					if ( pnParameterValueResult[16] < 0x10 ) {
						pcWidthParameterValue = (pnParameterValueResult + 11);
					}
					else {
						pcWidthParameterValue = pnParameterValueResult[11];
					}
					nWindowWidth = atol( pcWidthParameterValue );
				}
				nBufferSize = 15;
				nBlockSize = 0;
				LOBYTE( pBlock ) = 0;
				cls_0x50db20::meth_0x401bd0( szBuffer, "height", strlen( "height" ) );
				pnHeightParameterValue = *sub_4DF070( pnParameterStruct, szArgList, szBuffer );
				if ( nBufferSize >= 0x10 ) {
					free( Block: pBlock );
				}
				bIsParameterMatch = pnHeightParameterValue == pnParameterStruct[1];
				nBufferSize = 15;
				nBlockSize = 0;
				LOBYTE( pBlock ) = 0;
				if ( !bIsParameterMatch ) {
					if ( pnHeightParameterValue[16] < 0x10 ) {
						pcHeightAdjustedValue = (pnHeightParameterValue + 11);
					}
					else {
						pcHeightAdjustedValue = pnHeightParameterValue[11];
					}
					nWindowHeight = j__atol( pcHeightAdjustedValue );
				}
			}
			*/

			SetRect( &windowRect, 0, 0, nWindowWidth, nWindowHeight );
			AdjustWindowRect( &windowRect, 0x10C80000u, 0 );
			nWindowBottomPosition = windowRect.bottom - windowRect.top;
			nWindowRightPosition = windowRect.right - windowRect.left;
			nWindowTopPosition = (windowRect.top - windowRect.bottom + GetSystemMetrics( 1 )) / 2;
			nSystemMetrics = GetSystemMetrics( 0 );
			pwinWindow = CreateWindowExA( 0, szClassName, szClassName, 0x10C80000u, (windowRect.left - windowRect.right + nSystemMetrics) / 2, nWindowTopPosition, nWindowRightPosition, nWindowBottomPosition, 0, 0, 0, 0 );
			g_window = pwinWindow;
			if ( !pwinWindow ) {
				JE::FatalError( "Could not create main window." );
			}
			ShowWindow( pwinWindow, nCmdShow );
			F3::Render::InitOnce( g_window );   // M-001: opt-in d3d8 device; NO-OP unless OPENVB_RENDER=1
			return UpdateWindow( g_window );
		}

		// 0x56B1A0
		void DestroyGameWindow() {
			F3::Render::Shutdown();   // M-001: device before window; no-op when never enabled
			tracing::instrument( tracing::LOCATION, "F3::g_window=0x%p", F3::g_window );
			DestroyWindow( F3::g_window );
		}
	} // namespace Display

} // namespace F3

// JE::Host hooks - the original engine code is linked into the same binary and touches the F3
// globals directly (sub_56AF20 / sub_56AF30); the reimplementation routes through this seam.
namespace JE {
	namespace Host {
		char* GetCurrentWorkingDirectory() { return F3::g_szCurrentWorkingDirectory; }
		void* GetParameterTree() { return &F3::g_vfxParameterTree; } // 0x70C2C0/0x70C2CC (sub_56AF30/56AF40)
		void* GetEngineStringSystem() { return &F3::g_engineStringSystem; } // 0x70C2D8
		int TickMilliseconds() { return (int)timeGetTime(); }               // 0x56AF50

		// ActionMoveTo job registration target (world-server subobject vft+100 in the
		// original). Our architecture: the GameWorld's PathManager at +0xF0 IS the queue owner.
		void TickWorldEntities( int ms ) { F3::TickWorldEntities( ms ); }

		bool IsServerContext() { return F3::GetWorldServerInstance() != NULL; }   // MapEntitySpawn.cpp

		JE::PathManager* GetPathJobQueue() {
			F3::GameWorld* w = F3::GetWorldServerInstance();
			return w ? w->m_pathManager : NULL;
		}

		// ---- T-026: per-frame simulation digest -----------------------------------
		// The app path is deliberately thin and OFF by default: nothing is opened, and therefore
		// nothing is written, unless OPENVB_SIM_DIGEST names a file. A game loop that silently
		// creates files is a fidelity change of exactly the kind the deferred-notes rule forbids.
		// `clock` and `tickMs` come from the same sources the sim already consumes
		// (GameWorld::ClockSeconds, Host::TickMilliseconds); draws are reported NA here because the
		// shipped-path generator has no counter - the headless harness injects one via T-027's seam.
		static FILE* g_simDigest = NULL;
		static unsigned g_simDigestFrame = 0;
		// Sampling interval in FRAMES. Not decoration: the game loop is uncapped, so an app run
		// produced 924k frame lines (260 MB) in a 10 s boot. A comparison run that needs every
		// frame must say so explicitly via OPENVB_SIM_DIGEST_EVERY=1.
		static unsigned g_simDigestEvery = 1;

		// PORT-SIDE (T-026): digest plumbing has NO shipped twin anywhere near the 0x5ACE00 loop era;
		// observer-only side effects behind OPENVB_SIM_DIGEST, byte-absent from the shipped exe.
		int EnableSimDigest( const char* path ) {
			if ( g_simDigest || !path || !*path ) { return g_simDigest ? 1 : 0; }
			const char* every = getenv( "OPENVB_SIM_DIGEST_EVERY" );
			if ( every && *every ) {
				int n = atoi( every );
				g_simDigestEvery = ( n > 0 ) ? (unsigned)n : 1u;
			}
			g_simDigest = fopen( path, "w" );
			if ( !g_simDigest ) {
				JE::LogDebugString( NULL, "Host::EnableSimDigest() - cannot open digest file.\r\n" );
				return 0;
			}
			g_simDigestFrame = 0;
			F3::Sim::ResetBaseClock();   // clockDelta is measured from THIS run's first frame
			F3::GameWorld* w = F3::GetWorldServerInstance();
			F3::Sim::Sources src;
			src.combat = w ? w->GetCombat() : NULL;
			src.paths  = w ? w->GetPathManager() : NULL;
			F3::Sim::WriteHeader( g_simDigest, src );
			return 1;
		}

		// PORT-SIDE (T-026): runs after the 0x5ACDEA vft[3] walk return in GameStateLoop (0x5ACE00);
		// no shipped code at that frame position exists - shipped frames end at Present (Gfx era 0x509DC1).
		void DumpSimState() {
			if ( !g_simDigest ) { return; }
			// The frame-0 clock is captured on the FIRST TICK, not on the first WRITTEN frame: with
			// EVERY>1 those differ, and `clockDelta` would then inherit an off-by-one sampling
			// artifact (measured: two boots gave 1 then 0 for the same sampled position).
			if ( F3::Sim::BaseClockPending() ) {
				F3::GameWorld* w0 = F3::GetWorldServerInstance();
				F3::Sim::SetBaseClock( w0 ? w0->ClockSeconds() : 0u );
			}
			if ( ( g_simDigestFrame % g_simDigestEvery ) != 0u ) { ++g_simDigestFrame; return; }
			F3::GameWorld* w = F3::GetWorldServerInstance();
			F3::Sim::Sources src;
			src.combat = w ? w->GetCombat() : NULL;
			src.paths  = w ? w->GetPathManager() : NULL;
			F3::Sim::WriteFrame( g_simDigest, src, g_simDigestFrame++,
			                     (unsigned)JE::Host::TickMilliseconds(),
			                     w ? w->ClockSeconds() : 0u, 0, NULL );
		}

		// PORT-SIDE (T-026): flush hook on the shutdown path; the 0x56B1A0 window-destroy era has no
		// digest counterpart in the shipped binary (named absence, not an unmapped EA).
		void CloseSimDigest() {
			if ( g_simDigest ) {
				fprintf( g_simDigest, "# END frames=%u every=%u\n", g_simDigestFrame, g_simDigestEvery );
				fclose( g_simDigest ); g_simDigest = NULL;
			}
		}
	} // namespace Host
} // namespace JE


// Copyright 2024 OpenJE

#include <ios>
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <cstdio>

#include "JE.hpp"
#include "F3.hpp"
#include "tracing.hpp"

namespace F3 {
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

		//sub_61ADFA( pcSaveDirectoryRelative, (char *)0x0, (char *)0x0, szPersonalFolderPath, (char *)0x0 );

		nCharIndexRelative = 0;
		do {
			cCurrentCharRelative = szPersonalFolderPath[ nCharIndexRelative ];
			g_szSaveDirectoryRelative[ nCharIndexRelative ] = cCurrentCharRelative;
			nCharIndexRelative = nCharIndexRelative + 1;
		} while ( cCurrentCharRelative );

		pcSaveDirectoryRelative = GetSaveDirectoryRelative();
		F3::PathConcat( g_szSaveDirectoryAbsolute, pcSaveDirectoryRelative );
		//cResult = sub_4C56B0( g_szSaveDirectoryAbsolute );
		if ( !cResult ) {
			//cResult = sub_4C5750( g_szSaveDirectoryAbsolute );
			if ( !cResult ) {
				JE::FatalError( "Unable to create save folder." );
			}
		}
		return cResult;
	} // SetupSaveDirectory

	/*
	// 0x56b220
	void SetupConfigFile( uint nCmdShow ) {
	} // SetupConfigFile
	*/

	// 0x56B100
	int SetupLogFile() {
		tracing::instrument( tracing::LOCATION, "" );
		char* pcSaveDirectoryAbsolute;
		int   nFileNameOffset;
		char  cCurrentChar;
		char* pcSaveDirectoryRelative;
		char* pcExtension;
		char  cExtension;
		char  szFilePath[ MAX_PATH ];

		pcSaveDirectoryAbsolute = GetSaveDirectoryAbsolute();
		nFileNameOffset = (int)( szFilePath - pcSaveDirectoryAbsolute );
		do {
			cCurrentChar = *pcSaveDirectoryAbsolute;
			pcSaveDirectoryAbsolute[ nFileNameOffset ] = *pcSaveDirectoryAbsolute;
			++pcSaveDirectoryAbsolute;
		}
		while ( cCurrentChar );
		pcSaveDirectoryRelative = GetSaveDirectoryRelative();
		PathConcat( szFilePath, pcSaveDirectoryRelative );
		pcExtension = &cExtension;
		while ( *++pcExtension ) {
			;
		}
		strcpy( pcExtension, ".log" );
		//return OpenLogFile( FileName );
		return 0;
	} // SetupLogFile

	/*
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

	// 0x56af10
	char *GetSaveDirectoryRelative() {
		return g_szSaveDirectoryRelative;
	}

	// 0x56af00
	char *GetSaveDirectoryAbsolute() {
		return g_szSaveDirectoryAbsolute;
	}

	// 0x4c58a0
	unsigned int PathConcat( const char *pcPathA, const char *pcPathB ) {
		tracing::instrument( tracing::LOCATION, "pcPathA=\"%s\", pcPathB=%s", pcPathA, pcPathB );

		unsigned int nPathALen;
		char cLastCharPathA;
		unsigned int nPathBLen;

		nPathALen = strlen( pcPathA );
		cLastCharPathA = pcPathA[ nPathALen - 1 ];
		if ( cLastCharPathA != '\\' && cLastCharPathA != '/' ) {
			strcpy( (char *)&pcPathA[ nPathALen ], "\\" );
		}
		nPathBLen = strlen( pcPathB ) + 1;
		memcpy( (void *)&pcPathB[ strlen( pcPathA ) ], pcPathB, nPathBLen );
		return nPathBLen;
	}

	int SetStartupTime( int nTime ) {
		tracing::instrument( tracing::LOCATION, "nTime=%d", nTime );

		int result;

		result = nTime;
		g_nStartupTimeMS = nTime;
		g_bIsStartupTimeSet = true;
		return result;
	}

	int Main() {
		tracing::instrument( tracing::LOCATION, "" );
		//F3::Startup();
		//F3::RegisterCommand( ArgList: aQuit, func_ptr: j_F3::Quit );
		while ( ProcessMessagesAndUpdateTime() ) {
			//F3::GameStateLoop();
		}
		//F3::Shutdown();
		return 1;
	}

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

	/*
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

	void F3::Quit() {
		tracing::instrument( tracing::LOCATION, "" );
		PostQuitMessage( 1 );
	}

	namespace Display {
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
			return UpdateWindow( g_window );
		}

		void DestroyGameWindow() {
			tracing::instrument( tracing::LOCATION, "F3::g_window=0x%p", F3::g_window );
			DestroyWindow( F3::g_window );
		}
	} // namespace Display
} // namespace F3

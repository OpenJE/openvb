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

	namespace Display {
		void DestroyGameWindow() {
			tracing::instrument( tracing::LOCATION, "F3::g_window=0x%p", F3::g_window );
			DestroyWindow( F3::g_window );
		}
	} // namespace Display
} // namespace F3

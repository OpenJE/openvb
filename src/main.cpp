// OpenJE

#include <windows.h>

#include <ctime>
#include "JE.hpp"
#include "F3.hpp"
#include "tracing.hpp"

// 0x56b810
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
	tracing::instrument( tracing::LOCATION,
		"hInstance=0x%p, hPrevInstance=0x%p, lpCmdLine=\"%s\", nCmdShow=%d",
		hInstance, hPrevInstance, lpCmdLine, nCmdShow
	);

	std::time_t timeStartupTime;
	int nMainResult;
	char szModulePath[ MAX_PATH ] = { 0 };
	int nExitStatus;
	int nSavedRegs;

	F3::g_instance = hInstance;
	if( !GetModuleFileName( (HMODULE)0x0, szModulePath, MAX_PATH ) ) {
		tracing::error( tracing::LOCATION, "Unable to get module file name." );
		JE::FatalError( "Unable to get module file name." );
	}
	tracing::info( tracing::LOCATION, "szModulePath=%s", szModulePath );

	F3::SetupSaveDirectory( szModulePath );
	//F3::SetupConfigFile( nCmdShow );
	//F3::SetupLogFile();

	timeBeginPeriod( 1 );
	F3::g_nSystemTimeMS = timeGetTime();
	tracing::info( tracing::LOCATION, "F3::g_nSystemTimeMS=%d", F3::g_nSystemTimeMS );

	F3::Display::CreateGameWindow( nCmdShow );
	timeStartupTime = std::time( 0 );
	F3::SetStartupTime( timeStartupTime );
	GetCommandLine();
	nMainResult = F3::Main();
	F3::Display::DestroyGameWindow();
	timeEndPeriod( 1 );
	//F3::ShutdownGlobalLogStream( &nSavedRegs );
	//F3::sub_56B390();
	return nMainResult;
} // WinMain

// OpenJE

#include <windows.h>

#include <ctime>
#include "JE.hpp"
#include "F3.hpp"
#include "tracing.hpp"
#include "F3/RenderD3D8.hpp"   // M-001 (gated; Probe/PreInit are no-ops un-gated)

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
	// Port-shape guard inside the 0x56B810 WinMain prologue era (module-path read is shipped-inline).
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

	F3::Render::PreInit();   // M-001 phase 1: D3D8 object VA reservation BEFORE window/subsystem
	                       // loading - the earliest port-controlled point in the REAL frame
	                       // driver (WinMain: the window is created HERE, not in F3::Main)
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

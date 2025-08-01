// OpenJE

#include <windows.h>

#include "JE.hpp"
#include "F3.hpp"
#include "Logger.hpp"

// 0x56b810
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
    LOG_INFO( LOG_LOCATION, "Starting F3.exe with command line: %s", lpCmdLine );
	CHAR f3_exe_path[256] = {0};
    //F3::StrStrLocale( lpCmdLine, "-leakcheck" );
	F3::global_hinstance = hInstance;
	if( !GetModuleFileName( (HMODULE)0x0, f3_exe_path, 260u ) ) {
	    LOG_ERROR( LOG_LOCATION, "Unable to get module file name." );
        JE::FatalError( "Unable to get module file name." );
	}
    LOG_INFO( LOG_LOCATION, "F3.exe path: %s\n", f3_exe_path );
	//F3::SetupSaveDirectory();
	//F3::SetupConfigFile( nCmdShow );
	//F3::SetupLogFile();
	//timeBeginPeriod( 1 );
	//F3::global_system_time_ms = timeGetTime();
	//F3::Display::CreateGameWindow( nCmdShow );
	//F3::SetStartupTime( (time_t)0 );
	//GetCommandLine();
	//int main_return = F3::Main();
	//F3::Display::DestroyGameWindow();
    //timeEndPeriod( 1 );
    //F3::sub_497B70();
    //F3::sub_56B390();
    //return main_return;
} // WinMain

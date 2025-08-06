// OpenJE

#include <windows.h>

#include "JE.hpp"
#include "F3.hpp"
#include "Logger.hpp"

// 0x56b810
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
    //int main_result;
    char module_filename[ 256 ] = { 0 };

    LOG_INFO(
        LOG_LOCATION,
        "Enter: WinMain( hInstance=0x%p, hPrevInstance=0x%p, lpCmdLine=\"%s\", nCmdShow=%d )",
        hInstance, hPrevInstance, lpCmdLine, nCmdShow
    );
	F3::g_hinstance = hInstance;
	if( !GetModuleFileName( (HMODULE)0x0, module_filename, 260u ) ) {
	    LOG_ERROR( LOG_LOCATION, "Unable to get module file name." );
        JE::FatalError( "Unable to get module file name." );
	}
    LOG_INFO( LOG_LOCATION, "F3.exe path: %s", module_filename );
	//F3::SetupSaveDirectory();
	//F3::SetupConfigFile( nCmdShow );
	//F3::SetupLogFile();
	timeBeginPeriod( 1 );
	F3::g_system_time_ms = timeGetTime();
    LOG_INFO( LOG_LOCATION, "System time: %d ms", F3::g_system_time_ms );
	//F3::Display::CreateGameWindow( nCmdShow );
	//F3::SetStartupTime( (time_t)0 );
	GetCommandLine();
	//int main_result = F3::Main();
	F3::Display::DestroyGameWindow();
    timeEndPeriod( 1 );
    //F3::sub_497B70();
    //F3::sub_56B390();
    //return main_result;
    LOG_INFO(
        LOG_LOCATION,
        "Exit: WinMain( hInstance=0x%p, hPrevInstance=0x%p, lpCmdLine=\"%s\", nCmdShow=%d )",
        hInstance, hPrevInstance, lpCmdLine, nCmdShow
    );
} // WinMain

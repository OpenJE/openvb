// Copyright 2024 OpenJE

#include <windows.h>
#include <stdio.h>
#include "OpenJE.hpp"
//#include "F3.hpp"
//#include "F3/Display/Display.hpp"

// 0x56b810
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
	CHAR f3_exe_path [260];
	strstr(  lpCmdLine, "-leakcheck" );
	F3::global_hinstance = hInstance;
	if( GetModuleFileName( (HMODULE)0x0, f3_exe_path, 260 ) == 0 ) {
		F3::FatalError( "Unable to get module file name." );
        OpenJE::Debug( "Unable to get module file name.\n" );
	}
    OpenJe::Debug("F3.exe path: %s\n", f3_exe_path);
	F3::SetupSaveDirectory();
	F3::SetupConfigFile( nCmdShow );
	F3::SetupLogFile();
	timeBeginPeriod( 1 );
	F3::global_system_time_ms = timeGetTime();
	F3::Display::CreateGameWindow( nCmdShow );
	F3::SetStartupTime( (time_t)0 );
	GetCommandLine();
	int main_return = F3::Main();
	F3::Display::DestroyGameWindow();
    timeEndPeriod( 1 );
    F3::sub_497B70();
    F3::sub_56B390();
    return main_return;
} // WinMain

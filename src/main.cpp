// Copyright 2024 OpenJE

#include "stdafx.h"

#include "f3.hpp"

// 0x56b810
int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
	CHAR f3_exe_path [268];
	f3::FindSubstringInString( lpCmdLine, "-leakcheck" );
	f3::global_hinstance = hInstance;
	if( GetModuleFileNameA( (HMODULE)0x0, f3_exe_path, 260 ) == 0 ) {
		f3::FailWithError( "Unable to get module file name." );
	}
	f3::SetupSaveDirectory();
	f3::SetupConfigFile( nCmdShow );
	f3::SetupLogFile();
	timeBeginPeriod( 1 );
	f3::global_system_time_ms = timeGetTime();
	f3::display::CreateGameWindow( nCmdShow );
	f3::SetStartupTime( (time_t)0 );
	GetCommandLineA();
	f3::Main();
	f3::display::DestroyGameWindow();
}
// Copyright 2024 OpenJE

#ifndef F3_H
#define F3_H

#include <windows.h>
#include <ctime>
#include <fstream>

// F3 Globals
namespace F3 {
	// 0x70BFA0 (F3::global_hInstance_0x70bfa0)
	static HINSTANCE g_instance;
	// 0x70C0AC (global_hWnd)
	static HWND g_window;
	// 0x70C2BC (F3::SYSTEM_TIME_MS_0x70c2bc)
	static unsigned long g_nSystemTimeMS;
	// 0x70C2B8 (F3::global_elapsed_time_0x70c2b8)
	static unsigned long g_nElapsedTimeMS;
	// 0x707C60 (F3::start_time)
	static std::time_t g_nStartupTimeMS;
	// 0x707C64 (startup-time flag byte_707C64)
	static bool g_bIsStartupTimeSet;
	// 0x707D60 (byte_707D60 - log stream initialized flag)
	static BOOL g_bIsLogStreamInitialized;
	// 0x707D04 (F3::g_FileBuf)
	static std::filebuf g_LogFilebuf;
	// 0x707CF8 (_dword_707CF8 - ios/stream state)
	static std::ios* g_LogIos;
	// 0x70C0B0 (F3::g_szSaveDirectoryAbsolute)
	static char g_szSaveDirectoryAbsolute[ MAX_PATH ];
	// 0x70C1B8 (F3::g_szSaveDirectoryRelative)
	static char g_szSaveDirectoryRelative[ MAX_PATH ];
	// 0x70C1B8 (original raw name)
	static char g_save_directory_relative_path_0x70c1b8[ MAX_PATH ];
	// 0x70C0B0 (original raw name)
	static char directory_path_0x70c0b0[ MAX_PATH ];
	// 0x70BFA8 (F3::g_szCurrentWorkingDirectory)
	static char g_szCurrentWorkingDirectory[ MAX_PATH ];
	// 0x707D60 (INT_00707d60)
	static int INT_00707d60;
	// 0x707CF0 (DAT_00707cf0)
	static bool DAT_00707cf0;
	// 0x70FD41 (CHAR_0070fd41)
	static char CHAR_0070fd41[ 2 ];

	// 0x56B000
	char SetupSaveDirectory( CHAR *pcModulePath );
	// 0x56B220
	//void SetupConfigFile();
	// 0x56B100
	//int SetupLogFile();
	// 0x498000
	//int OpenLogFile( const char* pcLogFilePath )
	// 0x56AF10
	char *GetSaveDirectoryRelative();
	// 0x56AF00
	char *GetSaveDirectoryAbsolute();
	// 0x4C58A0
	unsigned int PathConcat( const char *string_a, const char *string_b );
	// 0x48CCA0
	int SetStartupTime( int nTime );
	// 0x5ACE00
	int Main();
	// 0x56AFF0
	void Quit();
	// 0x5ACD60
	//void Startup();
	// 0x59F5D0
	//void RegisterCommand( const char* command, void (*func)() );
	// 0x5ACDE0
	//void GameStateLoop();
	// 0x56AF60
	bool ProcessMessagesAndUpdateTime();
	// 0x5ACDA0
	//void Shutdown();
	// 0x48CCA0
	//void SetStartupTime( time_t time );
	// 0x61B067
	//char* sub_61B067( char * param_1, size_t param_2 );
	// 0x61ADFA
	//void sub_61ADFA( char * some_string, char * drive_letter, char * directory_path, char * folder_path, char * file_extension );
	// 0x4C58A0
	//void sub_4C58A0( char * param_1, undefined4 * param_2 );
	// 0x4C56B0
	//char sub_4C56B0( LPCSTR param_1 );
	// 0x4C5750
	//char sub_4C5750( char * param_1 );
	// 0x497120
	//void sub_497120( char * format, ... );
	// 0x6174DA
	//undefined4 sub_6174DA( char * param_1, char * param_2 /*int ** param_3*/ );
	// 0x61F82F
	//undefined4 sub_61F82F( FILE * param_1, char * param_2 /*int ** param_3*/ );
	// 0x616E24
	//ulonglong sub_616E24();
	// 0x56B170
	//void sub_56B170();
	// 0x497B70
	//void sub_497B70();
	// 0x56B390
	//void sub_56B390();
	// 0x61AF79
	//char* sub_61AF79( LPSTR file_name, char *a2, int a3 );
	// 0x61AF42
	//BOOL sub_61AF42( LPCSTR root_path_name );
	// 0x61A6AA
	//char* sub_61A6AA( char* dest, const char* source, size_t count );

	namespace Display {
		// 0x56B1B0
		LRESULT CALLBACK WinProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
		// 0x56B470
		BOOL CreateGameWindow( int nCmdShow );
		// 0x56B1A0
		void DestroyGameWindow();
	} // namespace Display
}

#endif // F3_H

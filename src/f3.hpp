// Copyright 2024 OpenJE

#ifndef F3_H
#define F3_H

#include "stdafx.h"

#include "f3/global.hpp"
#include "f3/display.hpp"

namespace f3 {
	int Main();
	void Quit();
	void Startup();
	void RegisterCommand ( char* command, void (*func)(), undefined4 param_3 );
	void GameStateLoop();
	bool ProcessMessagesAndUpdateTime();
	void Shutdown();
	char* FindSubstringInString ( char * string, char * sub_string );
	void FailWithError( char * format, ... );
	void SetupSaveDirectory();
	void SetupConfigFile( uint nCmdShow );
	void SetupLogFile();
	void SetStartupTime( time_t time );
    char* FUN_0061b067( char * param_1, size_t param_2 );
    void FUN_0061adfa( char * some_string, char * drive_letter, char * directory_path, char * folder_path, char * file_extension );
    void FUN_004c58a0( char * param_1, undefined4 * param_2 );
    char FUN_004c56b0( LPCSTR param_1 );
    char FUN_004c5750( char * param_1 );
    void FUN_00497120( char * format, ... );
    char* get_some_directory_path_2();
    undefined4 FUN_006174da( char * param_1, char * param_2 /*int ** param_3*/ );
    undefined4 FUN_0061f82f( FILE * param_1, char * param_2 /*int ** param_3*/ );
    ulonglong FUN_00616e24();
    void FUN_0056b170();
    void FUN_00497b70();
    void FUN_0056B390();
    char* FUN_0061AF79( LPSTR file_name, char *a2, int a3 );
    BOOL FUN_0061AF42( LPCSTR root_path_name );
    char* FUN_0061a6aa( char* dest, const char* source, size_t count );
}

#endif // F3_H
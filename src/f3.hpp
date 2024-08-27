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
	void FailWithError( char * param_1 );
	void SetupSaveDirectory();
	void SetupConfigFile( unsigned int nCmdShow );
	void SetupLogFile();
	void SetStartupTime( time_t time );
    int FUN_0061b067( unsigned int * param_1, size_t param_2 );
    void FUN_0061adfa( char * some_string, char * drive_letter, char * directory_path, char * folder_path, char * file_extension );
    void FUN_004c58a0( char * param_1, undefined4 * param_2 );
    char FUN_004c56b0( LPCSTR param_1 );
    char FUN_004c5750( char * param_1 );
    void FUN_00497120( byte * param_1 );
    char* get_some_directory_path_2();
    undefined4 FUN_006174da( char * param_1, char * param_2 /*int ** param_3*/ );
    undefined4 FUN_0061f82f( FILE * param_1, char * param_2 /*int ** param_3*/ );
}

#endif // F3_H
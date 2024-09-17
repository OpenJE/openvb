// Copyright 2024 OpenJE

#ifndef F3_H
#define F3_H

#include "stdafx.h"

#include "f3/global.hpp"
#include "f3/display.hpp"

namespace F3 {
	int Main();
	void Quit();
	void Startup();
	void RegisterCommand ( const char* command, void (*func)() );
	void GameStateLoop();
	bool ProcessMessagesAndUpdateTime();
	void Shutdown();
	char* FindSubstringInString ( char * string, char * sub_string );
	void FailWithError( char * format, ... );
	void SetupSaveDirectory();
	void SetupConfigFile( uint nCmdShow );
	void SetupLogFile();
	void SetStartupTime( time_t time );
    char* sub_61B067( char * param_1, size_t param_2 );
    void sub_61ADFA( char * some_string, char * drive_letter, char * directory_path, char * folder_path, char * file_extension );
    void sub_4C58A0( char * param_1, undefined4 * param_2 );
    char sub_4C56B0( LPCSTR param_1 );
    char sub_4C5750( char * param_1 );
    void sub_497120( char * format, ... );
    char* get_some_directory_path_2();
    undefined4 sub_6174DA( char * param_1, char * param_2 /*int ** param_3*/ );
    undefined4 sub_61F82F( FILE * param_1, char * param_2 /*int ** param_3*/ );
    ulonglong sub_616E24();
    void sub_56B170();
    void sub_497B70();
    void sub_56B390();
    char* sub_61AF79( LPSTR file_name, char *a2, int a3 );
    BOOL sub_61AF42( LPCSTR root_path_name );
    char* sub_61A6AA( char* dest, const char* source, size_t count );
}

#endif // F3_H
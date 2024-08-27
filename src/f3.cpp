// Copyright 2024 OpenJE

#include "stdafx.h"

#include "f3.hpp"

namespace f3 {
	// 0x5ace00
	int Main() {
		bool continue_running;
		undefined4 unaff_retaddr = 0;

		Startup();
		RegisterCommand( "quit", &Quit, unaff_retaddr );
		continue_running = ProcessMessagesAndUpdateTime();
		while ( continue_running ) {
			GameStateLoop();
			continue_running = ProcessMessagesAndUpdateTime();
		}
		Shutdown();
		return 1;
	}

	void Quit() {
		PostQuitMessage( 1 );
		return;
	}

	// 0x5acd60
	void Startup() {

	}

	// 0x59f5d0
	void RegisterCommand ( char * command, void (*func)(), undefined4 param_3 ) {
	}

	// 0x5acde0
	void GameStateLoop() {
	}

	// 0x56af60
	bool ProcessMessagesAndUpdateTime() {
		static DWORD program_time_ms = 0;
		BOOL msg_available;
		tagMSG lp_msg;

		msg_available = PeekMessageA( &lp_msg, (HWND)0x0, 0, 0, 1 );
		while( true ) {
			if ( msg_available == false ) {
				DWORD curr_system_time_ms = timeGetTime();
				DWORD delta_system_time_ms = curr_system_time_ms - global_system_time_ms;
				global_system_time_ms = curr_system_time_ms;
				program_time_ms = program_time_ms + delta_system_time_ms;
				return true;
			}
			if ( lp_msg.message == WM_QUIT ) break;
			TranslateMessage( &lp_msg );
			DispatchMessageA( &lp_msg );
			msg_available = PeekMessageA( &lp_msg, (HWND)0x0, 0, 0, 1 );
		}
		return false;
	}

	// 0x5acda0
	void Shutdown() {
	}

	// 0x61ad4d
	char* FindSubstringInString ( char * string, char * sub_string ) {
		/*
		byte bVar1;
		_ptiddata ptid_data;
		int *tpxcptinfoptrs;
		char *sub_string_pos;
		size_t sub_string_length;
		size_t string_length;
		int iVar2;
		int iVar3;
		char *some_string;

		ptid_data = __getptd();
		tpxcptinfoptrs = (int *)ptid_data->_tpxcptinfoptrs;
		if (tpxcptinfoptrs != DAT_0070fd28) {
			tpxcptinfoptrs = FUN_00625e2a();
		}
		if (tpxcptinfoptrs[2] == 0) {
			sub_string_pos = str_find_pos(string,sub_string);
			return sub_string_pos;
		}
		// if string 2 is empty
		if (*sub_string == '\0') {
			return string;
		}
		sub_string_length = _strlen(sub_string);
		string_length = _strlen(string);
		// if string 1 is not empty
		if (*string != '\0') {
			iVar2 = (int)string - (int)sub_string;
			some_string = string;
			do {
			if (string + (string_length - sub_string_length) < some_string) {
				return (char *)0x0;
			}
			sub_string_pos = sub_string;
			bVar1 = *some_string;
			while (bVar1 != 0) {
				if (*sub_string_pos == '\0') {
				return some_string;
				}
				if (sub_string_pos[iVar2] != *sub_string_pos) break;
				sub_string_pos = sub_string_pos + 1;
				bVar1 = sub_string_pos[iVar2];
			}
			if (*sub_string_pos == '\0') {
				return some_string;
			}
			sub_string_pos = some_string + 1;
			iVar3 = iVar2 + 1;
			if ((*(byte *)((byte)*some_string + 0x1d + (int)tpxcptinfoptrs) & 4) != 0) {
				sub_string_pos = some_string + 2;
				iVar3 = iVar2 + 2;
			}
			iVar2 = iVar3;
			some_string = sub_string_pos;
			} while (*sub_string_pos != '\0');
		}
		*/
		return (char *)0x0;
	}

	// 0x48cc40
	void FailWithError( char * error_message ) {
        code *pcVar1;
        char text [ 1024 ];

        FUN_006174da( text, error_message /*(int **)&stack0x00000008*/ );
        FUN_00497120( (byte *)text );
        MessageBoxA( (HWND)0x0, text, "Fatal Error", MB_ICONERROR | MB_DEFBUTTON2 );
        exit( 1 );
	}

	// 0x56b000
	void SetupSaveDirectory() {
        HRESULT result;
        int path_index;
        char *possible_file_path;
        char personal_folder_path[ 64 ];
        char current_path_char;

        path_index = FUN_0061b067( (unsigned int *)&UINT_0070bfa8, 0x104);
        if ( path_index == 0 ) {
            FailWithError( "Unable to retrieve current working directory." );
        }
        result = SHGetFolderPathA( (HWND)0x0,CSIDL_PERSONAL_FOLDER, (HANDLE)0x0, 0, personal_folder_path );
        if ( result != 0 ) {
            FailWithError( "Unable to retrieve personal folder." );
        }

        path_index = 0;
        do {
            current_path_char = personal_folder_path[ path_index ];
            some_directory_path[ path_index ] = current_path_char;
            path_index = path_index + 1;
        } while ( current_path_char != '\0' );

        FUN_0061adfa( possible_file_path,(char *)0x0, (char *)0x0, personal_folder_path, (char *)0x0 );

        path_index = 0;
        do {
            current_path_char = personal_folder_path[ path_index ];
            some_directory_path_2[ path_index ] = current_path_char;
            path_index = path_index + 1;
        } while ( current_path_char != '\0' );

        //some_directory_path_2 = (undefined *)get_some_directory_path_2();
        //FUN_004c58a0( some_directory_path, some_directory_path_2 );
        current_path_char = FUN_004c56b0( some_directory_path );
        if ( current_path_char == '\0' ) {
            current_path_char = FUN_004c5750( some_directory_path );
            if ( current_path_char == '\0' ) {
            FailWithError( "Unable to create save folder." );
            }
        }
        return;
	}

	// 0x56b220
	void SetupConfigFile( unsigned int nCmdShow ) {
	}

	// 0x56b100
	void SetupLogFile() {
	}

	// 0x48cca0
	void SetStartupTime( time_t time32 ) {
	}

    // 0x61b067
    int FUN_0061b067( unsigned int * param_1, size_t param_2 ) {
        return 0; // temporary
    }

    // 0x61adfa
    void FUN_0061adfa( char * some_string, char * drive_letter, char * directory_path, char * folder_path, char * file_extension ) {
    }

    // 0x4c58a0
    void FUN_004c58a0( char * param_1, undefined4 * param_2 ) {
    }

    // 0x4c56b0
    char FUN_004c56b0( LPCSTR param_1 ) {
        return '\0'; // temporary
    }

    // 0x4c5750
    char FUN_004c5750( char * param_1 ) {
        return '\0'; // temporary
    }

    // 0x497120
    void FUN_00497120( byte * param_1 ) {

    }

    // 0x56af10
    char* get_some_directory_path_2() {
        return some_directory_path_2;
    }

    // 0x6174da
    undefined4 FUN_006174da( char * param_1, char * param_2 /*int ** param_3*/ )
    {
        undefined4 uVar1;
        FILE file;

        file._cnt = 0x7fffffff;
        file._flag = 0x42;
        file._base = param_1;
        file._ptr = param_1;
        uVar1 = FUN_0061f82f( &file, param_2 /*param_3*/ );
        if ( param_1 != (char *)0x0 ) {
            file._cnt = file._cnt + -1;
            if ( file._cnt < 0 ) {
            _flsbuf( 0, &file );
            }
            else {
            *file._ptr = '\0';
            }
        }
        return uVar1;
    }

    // 0x61f82f
    undefined4 FUN_0061f82f( FILE * param_1, char * param_2 /*int ** param_3*/ ) {
        return 0; // temporary
    }
} // namespace f3
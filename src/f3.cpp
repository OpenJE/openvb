// Copyright 2024 OpenJE

#include "stdafx.h"

//#include "std/std.hpp"
#include "f3.hpp"

void __fastcall _write_char( FILE *param_1 );

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
	} // Main

	void Quit() {
		PostQuitMessage( 1 );
		return;
	} // Quit

	// 0x5acd60
	void Startup() {

	} // Startup

	// 0x59f5d0
	void RegisterCommand( char * command, void (*func)(), undefined4 param_3 ) {
	} // RegisterCommand

	// 0x5acde0
	void GameStateLoop() {
	} // GameStateLoop

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
	} // ProcessMessagesAndUpdateTime

	// 0x5acda0
	void Shutdown() {
	} // Shutdown

	// 0x48cc40
	void FailWithError( char * format, ... ) {
        char buffer [ 1024 ];
        va_list arg_list;

        va_start( arg_list, format );
        vsprintf( buffer, format, arg_list );
        FUN_00497120( buffer );
        MessageBoxA( (HWND)0x0, buffer, "Fatal Error", MB_ICONERROR | MB_DEFBUTTON2 );
        exit( 1 );
	} // FailWithError

	// 0x56b000
	void SetupSaveDirectory() {
        HRESULT result;
        int path_index;
        char *possible_file_path;
        char personal_folder_path[ 64 ];
        char current_path_char;

        if ( !FUN_0061b067( CHAR_0070bfa8, 260 ) ) {
            FailWithError( "Unable to retrieve current working directory." );
        }
        result = SHGetFolderPathA( (HWND)0x0, CSIDL_PERSONAL_FOLDER, (HANDLE)0x0, 0, personal_folder_path );
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
	} // SetupSaveDirectory

	// 0x56b220
	void SetupConfigFile( uint nCmdShow ) {
	} // SetupConfigFile

	// 0x56b100
	void SetupLogFile() {
	} // SetupLogFile

	// 0x48cca0
	void SetStartupTime( time_t time32 ) {
	} // SetStartupTime

    // 0x61b067
    char* FUN_0061b067( char * param_1, size_t param_2 ) {
        char* local_20;
        local_20 = FUN_0061AF79( (LPSTR)0x0, param_1, param_2 );
        return local_20;
    } // FUN_0061b067

    // 0x61adfa
    void FUN_0061adfa( char * file_path, char * drive_letter, char * directory_path, char * folder_path, char * file_extension ) {
        size_t input_string_length;
        char *input_string_local;
        uint uVar1;
        char *input_string_local_2;
        char *input_string_local_3;
        byte input_string_local_2_char_pos;

        input_string_local_3 = (char *)0x0;
        input_string_length = strlen( file_path );
        if ( ( input_string_length == 0 ) || ( file_path[1] != ':' ) ) {
            input_string_local = file_path;
            if ( drive_letter != (char *)0x0 ) {
                *drive_letter = '\0';
            }
        }
        else {
            if ( drive_letter != (char *)0x0 ) {
                FUN_0061a6aa( drive_letter, file_path, 2 );
                drive_letter[2] = '\0';
            }
            input_string_local = file_path + 2;
        }
        file_path = (char *)0x0;
        input_string_local_2 = input_string_local;
        if ( *input_string_local != '\0' ) {
            do {
                input_string_local_2_char_pos = *input_string_local_2;
                if ( ( (&DAT_0070fd41)[input_string_local_2_char_pos] & 4 ) == 0 ) {
                    if ( ( input_string_local_2_char_pos == '/' ) || ( input_string_local_2_char_pos == '\\' ) ) {
                        file_path = (char *)( (byte *)input_string_local_2 + 1 );
                    }
                    else {
                        if (input_string_local_2_char_pos == '.') {
                            input_string_local_3 = input_string_local_2;
                        }
                    }
                } else {
                    input_string_local_2 = (char *)( (byte *)input_string_local_2 + 1 );
                }
                input_string_local_2 = (char *)( (byte *)input_string_local_2 + 1 );
            } while ( *input_string_local_2 != 0 );
            if ( file_path != (char *)0x0 ) {
                if ( directory_path != (char *)0x0 ) {
                    uVar1 = (int)file_path - (int)input_string_local;
                    if (0xfe < uVar1) {
                        uVar1 = 0xff;
                    }
                    FUN_0061a6aa( directory_path, input_string_local, uVar1 );
                    directory_path[uVar1] = '\0';
                }
                goto LAB_0061aec2;
            }
        }
        file_path = input_string_local;
        if ( directory_path != (char *)0x0 ) {
            *directory_path = '\0';
        }
        LAB_0061aec2:
        if ( ( input_string_local_3 == (char *)0x0 ) || ( input_string_local_3 < file_path ) ) {
            if ( folder_path != (char *)0x0 ) {
                uVar1 = 0xff;
                if ( (uint)( (int)input_string_local_2 - (int)file_path ) < 0xff ) {
                    uVar1 = (int)input_string_local_2 - (int)file_path;
                }
                FUN_0061a6aa( folder_path, file_path, uVar1 );
                folder_path[uVar1] = '\0';
            }
            if ( file_extension != (char *)0x0 ) {
                *file_extension = '\0';
            }
        } else {
            if ( folder_path != (char *)0x0 ) {
                uVar1 = (int)input_string_local_3 - (int)file_path;
                if (0xfe < uVar1) {
                    uVar1 = 0xff;
                }
                FUN_0061a6aa( folder_path, file_path, uVar1);
                folder_path[uVar1] = '\0';
            }
            if ( file_extension != (char *)0x0 ) {
                uVar1 = 0xff;
                if ( (uint)( (int)input_string_local_2 - (int)input_string_local_3 ) < 0xff ) {
                    uVar1 = (int)input_string_local_2 - (int)input_string_local_3;
                }
                FUN_0061a6aa( file_extension, input_string_local_3, uVar1 );
                file_extension[uVar1] = '\0';
            }
        }
        return;
    } // FUN_0061adfa(

    // 0x4c58a0
    void FUN_004c58a0( char * param_1, undefined4 * param_2 ) {
    } // FUN_004c58a0

    // 0x4c56b0
    char FUN_004c56b0( LPCSTR param_1 ) {
        return '\0'; // temporary
    } // FUN_004c56b0

    // 0x4c5750
    char FUN_004c5750( char * param_1 ) {
        return '\0'; // temporary
    } // FUN_004c5750

    // 0x497120
    void FUN_00497120( char * format, ... ) {
        CHAR buffer[1024];
        uint v3;
        uint retaddr;
        va_list arg_list;

        va_start( arg_list, format );
        if ( !DAT_00707cf0 )
        {
            DAT_00707cf0 = true;
            vsprintf( buffer, format, arg_list );
            if ( INT_00707d60 )
            {
            //sub_496E00(&unk_707D00, buffer);
            //sub_496990(&unk_707D00);
            }
            OutputDebugStringA( buffer );
            DAT_00707cf0 = 0;
        }
    } // FUN_00497120

    // 0x56af10
    char* get_some_directory_path_2() {
        return some_directory_path_2;
    } // get_some_directory_path_2

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
    } // FUN_006174da

    // 0x61f82f
    undefined4 FUN_0061f82f( FILE * param_1, char * param_2 /*int ** param_3*/ ) {
        return 0; // temporary
    } // FUN_0061f82f

    ulonglong FUN_00616e24() {
        ulonglong uVar1;
        uint uVar2;
        float fVar3;
        float10 in_ST0;
        uint local_20;
        float fStack_1c;

        uVar1 = (ulonglong)ROUND(in_ST0);
        local_20 = (uint)uVar1;
        fStack_1c = (float)(uVar1 >> 0x20);
        fVar3 = (float)in_ST0;
        if ((local_20 != 0) || (fVar3 = fStack_1c, (uVar1 & 0x7fffffff00000000) != 0)) {
            if ((int)fVar3 < 0) {
            uVar1 = uVar1 + (0x80000000 < ((uint)(float)(in_ST0 - (float10)uVar1) ^ 0x80000000));
            }
            else {
            uVar2 = (uint)(0x80000000 < (uint)(float)(in_ST0 - (float10)uVar1));
            uVar1 = CONCAT44((int)fStack_1c - (uint)(local_20 < uVar2),local_20 - uVar2);
            }
        }
        return uVar1;
    } // FUN_00616e24

    void FUN_0056b170() {
        /*
        int iVar1;
        int *piVar2;
        uint uVar3;

        if (INT_00707d60 != 0) {
            piVar2 = std::basic_filebuf<char>::meth_0x497930((std::basic_filebuf<char> *)&DAT_00707cfc.field_0x8);
            if (piVar2 == (int *)0x0) {
                iVar1 = *(int *)(DAT_00707cf8 + 4);
                uVar3 = *(uint *)(&DAT_00707cfc.field_0x0 + iVar1) | 2;
                if (*(int *)(&DAT_00707cfc.field_0x24 + iVar1) == 0) {
                    uVar3 = *(uint *)(&DAT_00707cfc.field_0x0 + iVar1) | 6;
                }
                std::ios_base::clear((std::ios_base *)((int)&DAT_00707cf8 + iVar1),uVar3,false);
            }
        }
        return;
        */
    } // FUN_0056b170

    void FUN_00497b70() {

    } // FUN_00497b70

    void FUN_0056B390() {

    } // FUN_0056B390

    char* FUN_0061AF79( LPSTR file_name, char *a2, int a3 )
    {
        char v3; // bl
        DWORD path_length; // eax
        int v6; // eax
        char *v7; // ecx
        CHAR path[260]; // [esp+4h] [ebp-10Ch] BYREF
        LPSTR file_name_component[2]; // [esp+108h] [ebp-8h] BYREF
        uint retaddr; // [esp+114h] [ebp+4h]

        if ( file_name ) {
            if ( !FUN_0061AF42( file_name ) ) {
                //*FUN_0061B79D() = 15;
                //*FUN_0061B794() = 13;
                return 0;
            }
            file_name = (LPSTR)CONCAT12( '.', CONCAT11( ':', file_name + '@' ) );
            path_length = GetFullPathNameA( file_name, 260, path, file_name_component );
        } else {
            path_length = GetCurrentDirectoryA( 260, path );
        }
        if ( !path_length ) {
            return 0;
        }
        v6 = path_length + 1;
        if ( (unsigned int)v6 > 260 ) {
            return 0;
        }
        v7 = a2;
        if ( a2 ) {
            if ( v6 > a3 ) {
                //*FUN_0061B794() = 34;
                return 0;
            }
        }
        else {
            if ( v6 <= a3 )
            v6 = a3;
            v7 = (char *)malloc( v6 );
            if ( !v7 ) {
                //*FUN_0061B794() = 12;
                return 0;
            }
        }
        return strcpy(v7, path);
    } // FUN_0061AF79

    BOOL FUN_0061AF42( LPCSTR root_path_name ) {
        BOOL result = 1; // eax
        if ( root_path_name ) {
            root_path_name = (LPCSTR)CONCAT12( '\\', CONCAT11( ':', root_path_name + '@' ) );
            if ( GetDriveTypeA( (LPCSTR)&root_path_name ) < 2 ) {
                return 0;
            }
        }
        return result;
    } // FUN_0061AF42

    char* FUN_0061a6aa( char* dest, const char* source, size_t count ) {
        byte bVar1;
        byte bVar2;
        _ptiddata p_Var3;
        int *piVar4;
        uint uVar5;
        uint uVar6;
        char *puVar7;
        char *puVar8;

        p_Var3 = __getptd();
        piVar4 = (int *)p_Var3->_tpxcptinfoptrs;
        if (piVar4 != DAT_0070fd28) {
            piVar4 = FUN_00625e2a();
        }
        if (piVar4[2] == 0) {
            dest = strncpy( dest, source, count );
        }
        else {
            puVar7 = dest;
            if (count != 0) {
            do {
                bVar1 = *source;
                uVar5 = count - 1;
                bVar2 = *(byte *)(bVar1 + 0x1d + (int)piVar4);
                *(byte *)puVar7 = bVar1;
                if ((bVar2 & 4) == 0) {
                puVar8 = (puVar7 + 1);
                source = source + 1;
                if (bVar1 == 0) {
        LAB_0061a70f:
                    if (uVar5 == 0) {
                    return dest;
                    }
                    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
                    *puVar8 = 0;
                    puVar8 = puVar8 + 1;
                    }
                    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
                    *(undefined *)puVar8 = 0;
                    puVar8 = (puVar8 + 1);
                    }
                    return dest;
                }
                }
                else {
                if (uVar5 == 0) {
                    *(undefined *)puVar7 = 0;
                    return dest;
                }
                bVar1 = source[1];
                uVar5 = count - 2;
                *(byte *)((int)puVar7 + 1) = bVar1;
                puVar8 = ((int)puVar7 + 2);
                source = source + 2;
                if (bVar1 == 0) {
                    *puVar7 = 0;
                    goto LAB_0061a70f;
                }
                }
                count = uVar5;
                puVar7 = puVar8;
            } while (uVar5 != 0);
            }
        }
        return dest;
    } // FUN_0061a6aa
} // namespace f3

void __fastcall _write_char( FILE *param_1 ) {
    int *piVar1;
    byte in_AL;
    uint uVar2;
    int *unaff_ESI;

    if (((*(byte *)&param_1->_flag & 0x40) == 0) || (param_1->_base != (char *)0x0)) {
        piVar1 = &param_1->_cnt;
        *piVar1 = *piVar1 + -1;
        if (*piVar1 < 0) {
        uVar2 = _flsbuf((int)(char)in_AL,param_1);
        }
        else {
        *param_1->_ptr = in_AL;
        param_1->_ptr = param_1->_ptr + 1;
        uVar2 = (uint)in_AL;
        }
        if (uVar2 == 0xffffffff) {
        *unaff_ESI = -1;
        return;
        }
    }
    *unaff_ESI = *unaff_ESI + 1;
    return;
}
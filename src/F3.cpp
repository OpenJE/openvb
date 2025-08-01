// Copyright 2024 OpenJE

#include <windows.h>
#include <stdio.h>

#include "JE.hpp"
#include "F3.hpp"

namespace F3 {
    /*
    // 0x56B000
	void SetupSaveDirectory() {
        HRESULT result;
        int path_index;
        char *possible_file_path;
        char personal_folder_path[ 64 ];
        char current_path_char;

        if ( !sub_61B067( CHAR_0070bfa8, 260 ) ) {
            JE::FatalError( "Unable to retrieve current working directory." );
        }
        result = SHGetFolderPath( (HWND)0x0, CSIDL_PERSONAL_FOLDER, (HANDLE)0x0, 0, personal_folder_path );
        if ( result != 0 ) {
            JE::FatalError( "Unable to retrieve personal folder." );
        }

        path_index = 0;
        do {
            current_path_char = personal_folder_path[ path_index ];
            some_directory_path[ path_index ] = current_path_char;
            path_index = path_index + 1;
        } while ( current_path_char != '\0' );

        sub_61ADFA( possible_file_path,(char *)0x0, (char *)0x0, personal_folder_path, (char *)0x0 );

        path_index = 0;
        do {
            current_path_char = personal_folder_path[ path_index ];
            some_directory_path_2[ path_index ] = current_path_char;
            path_index = path_index + 1;
        } while ( current_path_char != '\0' );

        //some_directory_path_2 = (undefined *)get_some_directory_path_2();
        //sub_4c58a0( some_directory_path, some_directory_path_2 );
        current_path_char = sub_4C56B0( some_directory_path );
        if ( current_path_char == '\0' ) {
            current_path_char = sub_4C5750( some_directory_path );
            if ( current_path_char == '\0' ) {
                JE::FatalError( "Unable to create save folder." );
            }
        }
        return;
	} // SetupSaveDirectory

	// 0x5ACE00
	int Main() {
		bool continue_running;
		undefined4 unaff_retaddr = 0;

		Startup();
		RegisterCommand( "quit", &Quit );
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

	// 0x5ACD60
	void Startup() {

	} // Startup

	// 0x59F5D0
	void RegisterCommand( const char * command, void (*func)() ) {
	} // RegisterCommand

	// 0x5ACDE0
	void GameStateLoop() {
	} // GameStateLoop

	// 0x56AF60
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

	// 0x5ACDA0
	void Shutdown() {
	} // Shutdown

	// 0x56B220
	void SetupConfigFile( uint nCmdShow ) {
	} // SetupConfigFile

	// 0x56B100
	void SetupLogFile() {
	} // SetupLogFile

	// 0x48CCA0
	void SetStartupTime( time_t time32 ) {
	} // SetStartupTime

    // 0x61B067
    char* sub_61B067( char * param_1, size_t param_2 ) {
        char* local_20;
        local_20 = sub_61AF79( (LPSTR)0x0, param_1, param_2 );
        return local_20;
    } // sub_61B067

    // 0x61ADFA
    unsigned __int8 *__cdecl sub_61ADFA(
        unsigned __int8 *Source,
        unsigned __int8 *Dest,
        unsigned __int8 *directory_path,
        unsigned __int8 *folder_path,
        unsigned __int8 *file_extension)
    {
        unsigned __int8 *v5; // ebx
        unsigned __int8 *result; // eax
        size_t v7; // esi
        unsigned __int8 v8; // cl
        size_t v9; // edi
        size_t v10; // edi
        unsigned int v11; // eax
        unsigned int v12; // eax
        unsigned __int8 *input_string_local_2; // [esp+Ch] [ebp-8h]
        unsigned __int8 *input_string_local_3; // [esp+10h] [ebp-4h]
        unsigned __int8 *some_string; // [esp+1Ch] [ebp+8h]

        input_string_local_3 = 0;
        v5 = Source;
        if ( strlen((const char *)Source) && Source[1] == 58 )
        {
            if ( Dest )
            {
            sub_61A6AA(Dest, Source, 2u);
            Dest[2] = 0;
            }
            v5 = Source + 2;
        }
        else if ( Dest )
        {
            *Dest = 0;
        }
        some_string = 0;
        result = v5;
        input_string_local_2 = v5;
        v7 = 255;
        if ( !*v5 )
            goto LABEL_23;
        do
        {
            v8 = *result;
            if ( ( CHAR_0070fd41[*result] & 4 ) != 0 )
            {
            ++result;
            }
            else if ( v8 == 47 || v8 == 92 )
            {
            some_string = result + 1;
            }
            else if ( v8 == 46 )
            {
            input_string_local_3 = result;
            }
            ++result;
        }
        while ( *result );
        input_string_local_2 = result;
        if ( some_string )
        {
            if ( directory_path )
            {
            v9 = some_string - v5;
            if ( (unsigned int)(some_string - v5) >= 0xFF )
                v9 = 255;
            sub_61A6AA(directory_path, v5, v9);
            directory_path[v9] = 0;
            result = input_string_local_2;
            }
            v5 = some_string;
        }
        else
        {
        LABEL_23:
            if ( directory_path )
            *directory_path = 0;
        }
        if ( input_string_local_3 && input_string_local_3 >= v5 )
        {
            if ( folder_path )
            {
            v10 = input_string_local_3 - v5;
            if ( (unsigned int)(input_string_local_3 - v5) >= 0xFF )
                v10 = 255;
            sub_61A6AA(folder_path, v5, v10);
            folder_path[v10] = 0;
            result = input_string_local_2;
            }
            if ( file_extension )
            {
            v11 = result - input_string_local_3;
            if ( v11 < 0xFF )
                v7 = v11;
            result = sub_61A6AA(file_extension, input_string_local_3, v7);
            file_extension[v7] = 0;
            }
        }
        else
        {
            if ( folder_path )
            {
            v12 = result - v5;
            if ( v12 < 0xFF )
                v7 = v12;
            sub_61A6AA(folder_path, v5, v7);
            folder_path[v7] = 0;
            }
            result = file_extension;
            if ( file_extension )
            *file_extension = 0;
        }
        return result;
    } // sub_61ADFA

    // 0x4C58A0
    void sub_4C58A0( char * param_1, undefined4 * param_2 ) {
    } // sub_4C58A0

    // 0x4C56B0
    char sub_4C56B0( LPCSTR param_1 ) {
        return '\0'; // temporary
    } // sub_4C56B0

    // 0x4C5750
    char sub_4C5750( char * param_1 ) {
        return '\0'; // temporary
    } // sub_4C5750

    // 0x497120
    void sub_497120( char * format, ... ) {
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
    } // sub_497120

    // 0x56AF10
    char* get_some_directory_path_2() {
        return some_directory_path_2;
    } // get_some_directory_path_2

    // 0x6174DA
    undefined4 sub_6174DA( char * param_1, char * param_2 //int ** param_3 )
    {
        undefined4 uVar1;
        FILE file;

        file._cnt = 0x7fffffff;
        file._flag = 0x42;
        file._base = param_1;
        file._ptr = param_1;
        uVar1 = sub_61F82F( &file, param_2 //param_3 );
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
    } // sub_6174DA

    // 0x61f82f
    undefined4 sub_61F82F( FILE * param_1, char * param_2 //int ** param_3 ) {
        return 0; // temporary
    } // sub_61F82F

    ulonglong sub_616E24() {
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
    } // sub_616E24

    void sub_56B170() {
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
    } // sub_56B170

    void sub_497B70() {

    } // sub_497B70

    void sub_56B390() {

    } // sub_56B390

    char* sub_61AF79( LPSTR file_name, char *a2, int a3 )
    {
        char v3; // bl
        DWORD path_length; // eax
        int v6; // eax
        char *v7; // ecx
        CHAR path[260]; // [esp+4h] [ebp-10Ch] BYREF
        LPSTR file_name_component[2]; // [esp+108h] [ebp-8h] BYREF
        uint retaddr; // [esp+114h] [ebp+4h]

        if ( file_name ) {
            if ( !sub_61AF42( file_name ) ) {
                //*sub_61B79D() = 15;
                //*sub_61B794() = 13;
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
                //*sub_61B794() = 34;
                return 0;
            }
        }
        else {
            if ( v6 <= a3 )
            v6 = a3;
            v7 = (char *)malloc( v6 );
            if ( !v7 ) {
                //*sub_61B794() = 12;
                return 0;
            }
        }
        return strcpy(v7, path);
    } // sub_61AF79

    BOOL sub_61AF42( LPCSTR root_path_name ) {
        BOOL result = 1; // eax
        if ( root_path_name ) {
            root_path_name = (LPCSTR)CONCAT12( '\\', CONCAT11( ':', root_path_name + '@' ) );
            if ( GetDriveTypeA( (LPCSTR)&root_path_name ) < 2 ) {
                return 0;
            }
        }
        return result;
    } // sub_61AF42

    char* sub_61A6AA( char* dest, const char* source, size_t count ) {
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
            piVar4 = sub_625E2A();
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
    } // sub_61A6AA
    */
} // namespace F3

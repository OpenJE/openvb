// Copyright 2024 OpenJE

#include <ios>
#include <windows.h>
#include <cstdio>

#include "JE.hpp"
#include "F3.hpp"
#include "Logger.hpp"

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

    // 0x56b220
	void SetupConfigFile( uint nCmdShow ) {
	} // SetupConfigFile
    */

	// 0x56B100
	int SetupLogFile() {
        LOG_INFO( LOG_LOCATION, "Enter: SetupLogFile()" );
        char *some_directory_path_0x56af00 = NULL;
        int diff = 0;
        char some_directory_path_char = '\0';
        char *save_directory_relative_path_0x56af10 = NULL;
        char *v4 = '\0';
        char v7 = '\0';
        char FileName[ 256 ];

        some_directory_path_0x56af00 = Get_some_directory_path();
        diff = (int)(FileName - some_directory_path_0x56af00);
        do
        {
            some_directory_path_char = *some_directory_path_0x56af00;
            some_directory_path_0x56af00[diff] = *some_directory_path_0x56af00;
            ++some_directory_path_0x56af00;
        }
        while ( some_directory_path_char );
        save_directory_relative_path_0x56af10 = Get_save_directory_relative_path();
        PathConcat(FileName, save_directory_relative_path_0x56af10);
        v4 = &v7;
        while ( *++v4 )
        {
            ;
        }
        strcpy(v4, ".log");
        //return sub_498000(FileName);
        LOG_INFO( LOG_LOCATION, "Exit: SetupLogFile()" );
        return 0;
	} // SetupLogFile

    // 0x56af10
    char *Get_save_directory_relative_path() {
        return g_save_directory_relative_path_0x70c1b8;
    }

    // 0x56af00
    char *Get_some_directory_path() {
        return directory_path_0x70c0b0;
    }

    // 0x4c58a0
    unsigned int PathConcat( const char *string_a, const char *string_b ) {
        unsigned int string_a_len = 0;
        char last_char_string_a = '\0';
        unsigned int string_b_size = 0;

        string_a_len = strlen( string_a );
        last_char_string_a = string_a[ string_a_len - 1 ];
        if ( last_char_string_a != '\\' && last_char_string_a != '/' ) {
            strcpy( (char *)&string_a[ string_a_len ], "\\" );
        }
        string_b_size = strlen( string_b ) + 1;
        memcpy( (void *)&string_a[ strlen( string_a ) ], string_b, string_b_size );
        return string_b_size;
    }

    int SetStartupTime( int time32 ) {
        int result = 0;

        result = time32;
        g_startup_time = time32;
        g_startup_time_set = true;
        return result;
    }

    namespace Display {
        void DestroyGameWindow() {
            DestroyWindow( F3::g_window );
        }
    } // namespace Display
} // namespace F3

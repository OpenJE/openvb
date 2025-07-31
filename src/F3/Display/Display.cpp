// Copyright 2024 OpenJE

#include "f3/Display/Display.hpp"

namespace F3 {
	namespace Display {
		// 0x56b470
		bool CreateGameWindow( uint nCmdShow ) {
            char* some_directory_path_2; // eax
            CHAR *v2; // edx
            CHAR v3; // cl
            int v4; // eax
            const char *v5; // eax
            const char *p_string1; // ecx
            int *v7; // eax
            int v8; // esi
            int v9; // esi
            int v10; // edi
            bool v11; // zf
            const char *v12; // edi
            int v13; // edi
            const char *v14; // edi
            int system_metrics; // eax
            struct HWND__ *window; // eax
            int screen_height_pixels; // [esp-2Ch] [ebp-1B0h]
            int nWidth; // [esp-28h] [ebp-1ACh]
            int nHeight; // [esp-24h] [ebp-1A8h]
            int v21; // [esp+0h] [ebp-184h] BYREF
            int xRight; // [esp+4h] [ebp-180h]
            int yBottom; // [esp+8h] [ebp-17Ch]
            char v24[4]; // [esp+Ch] [ebp-178h] BYREF
            void *Block; // [esp+10h] [ebp-174h]
            int v26; // [esp+20h] [ebp-164h]
            unsigned int v27; // [esp+24h] [ebp-160h]
            struct tagRECT rc; // [esp+28h] [ebp-15Ch] BYREF
            int v29; // [esp+38h] [ebp-14Ch] BYREF
            char v30[4]; // [esp+3Ch] [ebp-148h] BYREF
            char *string1; // [esp+40h] [ebp-144h] BYREF
            int v32; // [esp+50h] [ebp-134h]
            unsigned int v33; // [esp+54h] [ebp-130h]
            WNDCLASSA window_class; // [esp+58h] [ebp-12Ch] BYREF
            CHAR name[256]; // [esp+80h] [ebp-104h] BYREF
            unsigned int retaddr; // [esp+184h] [ebp+0h]

            /*
            some_directory_path_2 = get_some_directory_path_2();
            v2 = &name[ -some_directory_path_2 ];
            do
            {
                v3 = *(CHAR *)some_directory_path_2;
                v2[ some_directory_path_2 ] = *(CHAR *)some_directory_path_2;
                ++some_directory_path_2;
            } while ( v3 );
            */
            window_class.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
            window_class.lpfnWndProc = WndProc;
            window_class.cbClsExtra = 0;
            window_class.cbWndExtra = 0;
            window_class.hInstance = global_hinstance;
            window_class.hIcon = (HICON)LoadImageA( global_hinstance, name, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE );
            window_class.hCursor = LoadCursorA( 0, IDC_ARROW );
            window_class.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
            window_class.lpszMenuName = 0;
            window_class.lpszClassName = name;
            if ( !RegisterClassA( &window_class ) )
                FailWithError( "Unable to register window class." );
            xRight = 800;
            yBottom = 600;
            v33 = 15;
            v32 = 0;
            /*
            LOBYTE( string1 ) = 0;
            meth_0x401bd0( "Graphics", strlen("Graphics" ) );
            v4 = meth_0x4339b0( v30 );
            v29 = v4;
            if ( v4 == *(_DWORD *)byte_70C2D0 )
                goto LABEL_13;
            if ( *(_DWORD *)( v4 + 36 ) < 0x10u )
                v5 = (const char *)( v4 + 16 );
            else
                v5 = *(const char **)( v4 + 16 );
            p_string1 = string1;
            if ( v33 < 0x10 )
                p_string1 = (const char *)&string1;
            if ( _stricmp( p_string1, v5 ) < 0 )
            {
            LABEL_13:
                v21 = *(_DWORD *)byte_70C2D0;
                v7 = &v21;
            }
            else
            {
                v7 = &v29;
            }
            v8 = *v7;
            if ( v33 >= 0x10 )
                free( string1 );
            v33 = 15;
            v32 = 0;
            LOBYTE( string1 ) = 0;
            if ( v8 != *(_DWORD *)byte_70C2D0 )
            {
                v9 = v8 + 40;
                v27 = 15;
                v26 = 0;
                LOBYTE( Block ) = 0;
                meth_0x401bd0( "width", strlen( "width" ) );
                v10 = *(_DWORD *)sub_4DF070( &v21, v24 );
                if ( v27 >= 0x10 )
                free( Block );
                v11 = v10 == *(_DWORD *)( v9 + 4 );
                v27 = 15;
                v26 = 0;
                LOBYTE( Block ) = 0;
                if ( !v11 )
                {
                if ( *(_DWORD *)( v10 + 64 ) < 0x10u )
                    v12 = (const char *)( v10 + 44 );
                else
                    v12 = *(const char **)( v10 + 44 );
                xRight = atol(v12);
                }
                v27 = 15;
                v26 = 0;
                LOBYTE( Block ) = 0;
                meth_0x401bd0( "height", strlen( "height" ) );
                v13 = *(_DWORD *)sub_4DF070( &v21, v24 );
                if ( v27 >= 0x10 )
                free( Block );
                v11 = v13 == *(_DWORD *)( v9 + 4 );
                v27 = 15;
                v26 = 0;
                LOBYTE( Block ) = 0;
                if ( !v11 )
                {
                if ( *(_DWORD *)( v13 + 64 ) < 0x10u )
                    v14 = (const char *)( v13 + 44 );
                else
                    v14 = *(const char **)( v13 + 44 );
                yBottom = atol( v14 );
                }
            }
            */
            SetRect( &rc, 0, 0, xRight, yBottom );
            AdjustWindowRect( &rc, WS_VISIBLE | WS_CAPTION | WS_SYSMENU, FALSE );
            nHeight = rc.bottom - rc.top;
            nWidth = rc.right - rc.left;
            screen_height_pixels = ( rc.top - rc.bottom + GetSystemMetrics( 1 ) ) / 2;
            system_metrics = GetSystemMetrics( 0 );
            window = CreateWindowExA(
                0,
                name,
                name,
                0x10C80000u,
                (rc.left - rc.right + system_metrics) / 2,
                screen_height_pixels,
                nWidth,
                nHeight,
                0,
                0,
                0,
                0
            );
            global_window = window;
            if ( !window )
                FailWithError("Could not create main window.");
            ShowWindow( window, nCmdShow );
            return UpdateWindow( global_window );
		}

        static LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
            if ( hwnd != global_window )
                return DefWindowProcA(hwnd, msg, wparam, lparam);
            if ( msg == WM_CLOSE ) {
                Quit();
                return 0;
            } else {
                if ( msg == WM_ACTIVATEAPP && !wparam )
                //meth_0x4b2ad0( 0 );
                return DefWindowProcA( hwnd, msg, wparam, lparam );
            }
        }

		// 0x56b1a0
		void DestroyGameWindow (void) {
		}
	}
}

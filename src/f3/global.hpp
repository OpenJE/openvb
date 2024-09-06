// Copyright 2024 OpenJE

#ifndef F3_GLOBAL_H
#define F3_GLOBAL_H

#include "stdafx.h"
#include "types.hpp"
//#include "std/std.hpp"

struct _String_base {}; // PlaceHolder Structure

// F3 Defines
#define CSIDL_PERSONAL_FOLDER 0x8005

// F3 Globals
namespace f3 {
	static HINSTANCE global_hinstance;
	static HWND global_window;
	static DWORD global_system_time_ms;
    static char some_directory_path[268];
    static char some_directory_path_2[256];
    static char CHAR_0070bfa8[260];
    static int INT_00707d60;
    static bool DAT_00707cf0;
}

#endif // F3_GLOBAL_H
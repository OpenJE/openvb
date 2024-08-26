#ifndef F3_GLOBAL_H
#define F3_GLOBAL_H

#include "stdafx.h"

// Ghidra Types
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef unsigned long long undefined8;
typedef void code;

// F3 Defines
#define CSIDL_PERSONAL_FOLDER 0x8005

// F3 Globals
namespace f3 {
	static HINSTANCE global_hinstance;
	static HWND global_window;
	static DWORD global_system_time_ms;
    static char some_directory_path[268];
    static char some_directory_path_2[256];
    static unsigned int UINT_0070bfa8;
}

#endif // F3_GLOBAL_H
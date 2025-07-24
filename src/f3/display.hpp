// Copyright 2024 OpenJE

#ifndef F3_DISPLAY_H
#define F3_DISPLAY_H

#include "f3/global.hpp"

namespace F3 {
	namespace display {
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		bool CreateGameWindow( UINT nCmdShow );
		void DestroyGameWindow(void);
	}
}

#endif // F3_DISPLAY_H

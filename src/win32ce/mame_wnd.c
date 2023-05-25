// mame_wnd.c
 // AUG00 - Gandalf: borrowed some code from Mame32V36 ( mame036/src/Win32/MAME32.c)

/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse

    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  MAME32.c

 ***************************************************************************/

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "win32.h" /* Win32KeyToOSDKey */


/***************************************************************************
    Function prototypes
 ***************************************************************************/

static LRESULT CALLBACK     MAME32_MessageProc(HWND, UINT, WPARAM, LPARAM);


/***************************************************************************
    External variables
 ***************************************************************************/


/***************************************************************************
    Internal structures
 ***************************************************************************/

/***************************************************************************
    Internal variables
 ***************************************************************************/


/***************************************************************************
    External functions
 ***************************************************************************/



/***************************************************************************
    Internal functions
 ***************************************************************************/

/* static */ HWND MAME32_CreateWindow(void)
{
    static BOOL     bRegistered = FALSE;
    HINSTANCE       hInstance = GetModuleHandle(NULL);

    if (bRegistered == FALSE)
    {
        WNDCLASS    WndClass;

        WndClass.style          = 0; // GN // CS_SAVEBITS | CS_BYTEALIGNCLIENT | CS_OWNDC;
        WndClass.lpfnWndProc    = MAME32_MessageProc;
        WndClass.cbClsExtra     = 0;
        WndClass.cbWndExtra     = 0;
        WndClass.hInstance      = hInstance;
        WndClass.hIcon          = NULL; // GN // LoadIcon(hInstance, MAKEINTATOM(IDI_MAME32_ICON));
        WndClass.hCursor        = LoadCursor(NULL, IDC_WAIT); // GN // LoadCursor(NULL, IDC_ARROW);
        WndClass.hbrBackground  = (HBRUSH)GetStockObject(NULL_BRUSH);
        WndClass.lpszMenuName   = NULL;
        WndClass.lpszClassName  = TEXT("classMAME32"); // GN // (LPCSTR)"classMAME32";

        if (RegisterClass(&WndClass) == 0)
            return NULL;
        bRegistered = TRUE;
    }

    return CreateWindowEx(0,
                          TEXT("classMAME32"), // GN // "classMAME32",
                          TEXT("Mame32"), // GN // MAME32App.m_Name,
                          WS_VISIBLE, // WS_OVERLAPPED | WS_SYSMENU, // GN // WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME | WS_BORDER,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
//                          0, 0,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
}


/***************************************************************************/

static LRESULT CALLBACK MAME32_MessageProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
	case WM_CLOSE:
		/* Don't call DestroyWindow, it will be called by osd_exit. */
		return 0;
		break;

	case WM_PAINT:
		osd_win32_OnPaint(hWnd);
		return 0;
		break;

	case WM_KEYUP :
		if (Win32KeyToOSDKey(wParam) != 0)
		{
			key[Win32KeyToOSDKey(wParam)] = 0;
		}
		break;

	case WM_KEYDOWN:
		if (Win32KeyToOSDKey(wParam) != 0)
		{
			key[Win32KeyToOSDKey(wParam)] = 1;
		}
		break;

	case WM_SYSKEYDOWN :
		if (wParam == VK_MENU)
		{
			/* stupid alt key! */
			if (Win32KeyToOSDKey(wParam) != 0)
				key[Win32KeyToOSDKey(wParam)] = 1;
			return 0;
		}
		break;

	case WM_SYSKEYUP :
		if (wParam == VK_MENU)
		{
			/* stupid alt key! */
			if (Win32KeyToOSDKey(wParam) != 0)
				key[Win32KeyToOSDKey(wParam)] = 0;
			return 0;
		}
		break;
	}
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}

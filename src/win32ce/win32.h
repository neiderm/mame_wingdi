#ifndef _MAMEW32_H
#define _MAMEW32_H

#include <windows.h>

// translates keycodes on keyup/keydown events into Mame key codes
//int Win32KeyToOSDKey(UINT vk);

// key table for tracking notifications of keypresses from keyup/keydown msgs.
extern byte key[];

// this one lives in win32.c because of displayrect dependency
//void osd_win32_OnPaint(HWND hWnd);

#endif

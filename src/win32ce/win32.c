/***************************************************************************

  win32.c

  OS dependant stuff (display handling, keyboard scan...)
  This is the only file which should me modified in order to port the
  emulator to a different system.

  Created 6/29/97 by Christopher Kirmse (ckirmse@ricochet.net)

  Although MAME isn't ideally suited to the event driven windows model,
  we can poll for events on a regular basis (because the main loop calls
  us to check for keys all the time).  This makes the Windows version
  run quite well, and reduces many hardware compatibility problems!

  neiderm (2023): see original Windows port for DirectX in mame027s.zip
            This port to Windows GDI is derived from mame32v36 as well
            as the various MameCE ports.
***************************************************************************/
#include "mame.h" // NUM VOICES tmp
#include <stdio.h> /* debug file output */
#include "cesound.h" // #include "audio.h"
#include "osdepend.h" /* key def macros */

void osd_win32_sound_init();
void osd_win32_joy_init();
long CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//int Win32KeyToOSDKey(UINT vk); // declare this is WndProc is used
void dprintf(char *fmt,...);
void osd_win32_create_palette();
void osd_win32_mouse_moved(int x,int y);


int first_free_pen;
struct osd_bitmap *m_pMAMEBitmap; /* save pointer to DIB Section buffer to be free'd */

int win32_debug = 1;

typedef struct
{
    DWORD m_Top;
    DWORD m_Left;
    DWORD m_Right;
    DWORD m_Bottom;
    DWORD m_Width;
    DWORD m_Height;
} tRect;


/* joystick related stuff */
int osd_joy_up, osd_joy_down, osd_joy_left, osd_joy_right;
int osd_joy_b1, osd_joy_b2, osd_joy_b3, osd_joy_b4;


/* display related stuff */

//byte *buffer_ptr; // localized, doesn't need to be malloc'd
//int video_sync; // tbd
//int desired_width,desired_height; // tbd
int palette_offset; /* to work in window mode, must be 10 (because of 10 system colors) */
HWND hMain;

/* GDI drawing stuff */
static tRect        m_VisibleRect;
static tRect        m_DisplayRect;
static HDC          m_hMemDC;
static BITMAPINFO*  m_pInfo;
static HBITMAP      m_hBitmap;
static HBITMAP	    m_hOldBitmap;

/* audio related stuff */
int play_sound = 1;

//#define NUMVOICES 8
#define SAMPLE_RATE 44100
int MasterVolume = 100;

/* keyboard related stuff */
#ifndef OSD_KEY_NONE
#define OSD_KEY_NONE    0
#endif
#define OSD_NUMKEYS     100

static int key_code_table[] =
{
    /*                   0x00 */ OSD_KEY_NONE,
    /* VK_LBUTTON        0x01 */ OSD_KEY_NONE,
    /* VK_RBUTTON        0x02 */ OSD_KEY_NONE,
    /* VK_CANCEL         0x03 */ OSD_KEY_NONE,
    /* VK_MBUTTON        0x04 */ OSD_KEY_NONE,
    /*                   0x05 */ OSD_KEY_NONE,
    /*                   0x06 */ OSD_KEY_NONE,
    /*                   0x07 */ OSD_KEY_NONE,
    /* VK_BACK           0x08 */ OSD_KEY_BACKSPACE,
    /* VK_TAB            0x09 */ OSD_KEY_TAB,
    /*                   0x0A */ OSD_KEY_NONE,
    /*                   0x0B */ OSD_KEY_NONE,
    /* VK_CLEAR          0x0C */ OSD_KEY_5_PAD,
    /* VK_RETURN         0x0D */ OSD_KEY_ENTER,
    /*                   0x0E */ OSD_KEY_NONE,
    /*                   0x0F */ OSD_KEY_NONE,
    /* VK_SHIFT          0x10 */ OSD_KEY_LSHIFT,
    /* VK_CONTROL        0x11 */ OSD_KEY_CONTROL,
    /* VK_MENU           0x12 */ OSD_KEY_ALT,
    /* VK_PAUSE          0x13 */ OSD_KEY_NONE,
    /* VK_CAPITAL        0x14 */ OSD_KEY_CAPSLOCK,
    /*                   0x15 */ OSD_KEY_NONE,
    /*                   0x16 */ OSD_KEY_NONE,
    /*                   0x17 */ OSD_KEY_NONE,
    /*                   0x18 */ OSD_KEY_NONE,
    /*                   0x19 */ OSD_KEY_NONE,
    /*                   0x1A */ OSD_KEY_NONE,
    /* VK_ESCAPE         0x1B */ OSD_KEY_ESC,
    /*                   0x1C */ OSD_KEY_NONE,
    /*                   0x1D */ OSD_KEY_NONE,
    /*                   0x1E */ OSD_KEY_NONE,
    /*                   0x1F */ OSD_KEY_NONE,
    /* VK_SPACE          0x20 */ OSD_KEY_SPACE,
    /* VK_PRIOR          0x21 */ OSD_KEY_PGUP,
    /* VK_NEXT           0x22 */ OSD_KEY_PGDN,
    /* VK_END            0x23 */ OSD_KEY_END,
    /* VK_HOME           0x24 */ OSD_KEY_HOME,
    /* VK_LEFT           0x25 */ OSD_KEY_LEFT,
    /* VK_UP             0x26 */ OSD_KEY_UP,
    /* VK_RIGHT          0x27 */ OSD_KEY_RIGHT,
    /* VK_DOWN           0x28 */ OSD_KEY_DOWN,
    /* VK_SELECT         0x29 */ OSD_KEY_NONE,
    /* VK_PRINT          0x2A */ OSD_KEY_NONE,
    /* VK_EXECUTE        0x2B */ OSD_KEY_NONE,
    /* VK_SNAPSHOT       0x2C */ OSD_KEY_NONE,
    /* VK_INSERT         0x2D */ OSD_KEY_INSERT,
    /* VK_DELETE         0x2E */ OSD_KEY_DEL,
    /* VK_HELP           0x2F */ OSD_KEY_NONE,
    /*                   '0'  */ OSD_KEY_0,
    /*                   '1'  */ OSD_KEY_1,
    /*                   '2'  */ OSD_KEY_2,
    /*                   '3'  */ OSD_KEY_3,
    /*                   '4'  */ OSD_KEY_4,
    /*                   '5'  */ OSD_KEY_5,
    /*                   '6'  */ OSD_KEY_6,
    /*                   '7'  */ OSD_KEY_7,
    /*                   '8'  */ OSD_KEY_8,
    /*                   '9'  */ OSD_KEY_9,
    /*                   0x3A */ OSD_KEY_NONE,
    /*                   0x3B */ OSD_KEY_NONE,
    /*                   0x3C */ OSD_KEY_NONE,
    /*                   0x3D */ OSD_KEY_NONE,
    /*                   0x3E */ OSD_KEY_NONE,
    /*                   0x3F */ OSD_KEY_NONE,
    /*                   0x40 */ OSD_KEY_NONE,
    /*                   'A'  */ OSD_KEY_A,
    /*                   'B'  */ OSD_KEY_B,
    /*                   'C'  */ OSD_KEY_C,
    /*                   'D'  */ OSD_KEY_D,
    /*                   'E'  */ OSD_KEY_E,
    /*                   'F'  */ OSD_KEY_F,
    /*                   'G'  */ OSD_KEY_G,
    /*                   'H'  */ OSD_KEY_H,
    /*                   'I'  */ OSD_KEY_I,
    /*                   'J'  */ OSD_KEY_J,
    /*                   'K'  */ OSD_KEY_K,
    /*                   'L'  */ OSD_KEY_L,
    /*                   'M'  */ OSD_KEY_M,
    /*                   'N'  */ OSD_KEY_N,
    /*                   'O'  */ OSD_KEY_O,
    /*                   'P'  */ OSD_KEY_P,
    /*                   'Q'  */ OSD_KEY_Q,
    /*                   'R'  */ OSD_KEY_R,
    /*                   'S'  */ OSD_KEY_S,
    /*                   'T'  */ OSD_KEY_T,
    /*                   'U'  */ OSD_KEY_U,
    /*                   'V'  */ OSD_KEY_V,
    /*                   'W'  */ OSD_KEY_W,
    /*                   'X'  */ OSD_KEY_X,
    /*                   'Y'  */ OSD_KEY_Y,
    /*                   'Z'  */ OSD_KEY_Z,
    /* VK_LWIN           0x5B */ OSD_KEY_NONE,
    /* VK_RWIN           0x5C */ OSD_KEY_NONE,
    /* VK_APPS           0x5D */ OSD_KEY_NONE,
    /*                   0x5E */ OSD_KEY_NONE,
    /*                   0x5F */ OSD_KEY_NONE,
    /* VK_NUMPAD0        0x60 */ OSD_KEY_NONE,
    /* VK_NUMPAD1        0x61 */ OSD_KEY_NONE,
    /* VK_NUMPAD2        0x62 */ OSD_KEY_NONE,
    /* VK_NUMPAD3        0x63 */ OSD_KEY_NONE,
    /* VK_NUMPAD4        0x64 */ OSD_KEY_NONE,
    /* VK_NUMPAD5        0x65 */ OSD_KEY_NONE,
    /* VK_NUMPAD6        0x66 */ OSD_KEY_NONE,
    /* VK_NUMPAD7        0x67 */ OSD_KEY_NONE,
    /* VK_NUMPAD8        0x68 */ OSD_KEY_NONE,
    /* VK_NUMPAD9        0x69 */ OSD_KEY_NONE,
    /* VK_MULTIPLY       0x6A */ OSD_KEY_NONE,
    /* VK_ADD            0x6B */ OSD_KEY_NONE,
    /* VK_SEPARATOR      0x6C */ OSD_KEY_NONE,
    /* VK_SUBTRACT       0x6D */ OSD_KEY_NONE,
    /* VK_DECIMAL        0x6E */ OSD_KEY_NONE,
    /* VK_DIVIDE         0x6F */ OSD_KEY_NONE,
    /* VK_F1             0x70 */ OSD_KEY_F1,
    /* VK_F2             0x71 */ OSD_KEY_F2,
    /* VK_F3             0x72 */ OSD_KEY_F3,
    /* VK_F4             0x73 */ OSD_KEY_F4,
    /* VK_F5             0x74 */ OSD_KEY_F5,
    /* VK_F6             0x75 */ OSD_KEY_F6,
    /* VK_F7             0x76 */ OSD_KEY_F7,
    /* VK_F8             0x77 */ OSD_KEY_F8,
    /* VK_F9             0x78 */ OSD_KEY_F9,
    /* VK_F10            0x79 */ OSD_KEY_F10,
    /* VK_F11            0x7A */ OSD_KEY_F11,
    /* VK_F12            0x7B */ OSD_KEY_F12,
    /* VK_F13            0x7C */ OSD_KEY_NONE,
    /* VK_F14            0x7D */ OSD_KEY_NONE,
    /* VK_F15            0x7E */ OSD_KEY_NONE,
    /* VK_F16            0x7F */ OSD_KEY_NONE,
    /* VK_F17            0x80 */ OSD_KEY_NONE,
    /* VK_F18            0x81 */ OSD_KEY_NONE,
    /* VK_F19            0x82 */ OSD_KEY_NONE,
    /* VK_F20            0x83 */ OSD_KEY_NONE,
    /* VK_F21            0x84 */ OSD_KEY_NONE,
    /* VK_F22            0x85 */ OSD_KEY_NONE,
    /* VK_F23            0x86 */ OSD_KEY_NONE,
    /* VK_F24            0x87 */ OSD_KEY_NONE,
    /*                   0x88 */ OSD_KEY_NONE,
    /*                   0x89 */ OSD_KEY_NONE,
    /*                   0x8A */ OSD_KEY_NONE,
    /*                   0x8B */ OSD_KEY_NONE,
    /*                   0x8C */ OSD_KEY_NONE,
    /*                   0x8D */ OSD_KEY_NONE,
    /*                   0x8E */ OSD_KEY_NONE,
    /*                   0x8F */ OSD_KEY_NONE,
    /* VK_NUMLOCK        0x90 */ OSD_KEY_NUMLOCK,
    /* VK_SCROLL         0x91 */ OSD_KEY_SCRLOCK,
    /*                   0x92 */ OSD_KEY_NONE,
    /*                   0x93 */ OSD_KEY_NONE,
    /*                   0x94 */ OSD_KEY_NONE,
    /*                   0x95 */ OSD_KEY_NONE,
    /*                   0x96 */ OSD_KEY_NONE,
    /*                   0x97 */ OSD_KEY_NONE,
    /*                   0x98 */ OSD_KEY_NONE,
    /*                   0x99 */ OSD_KEY_NONE,
    /*                   0x9A */ OSD_KEY_NONE,
    /*                   0x9B */ OSD_KEY_NONE,
    /*                   0x9C */ OSD_KEY_NONE,
    /*                   0x9D */ OSD_KEY_NONE,
    /*                   0x9E */ OSD_KEY_NONE,
    /*                   0x9F */ OSD_KEY_NONE,
    /*                   0xA0 */ OSD_KEY_NONE,
    /*                   0xA1 */ OSD_KEY_NONE,
    /*                   0xA2 */ OSD_KEY_NONE,
    /*                   0xA3 */ OSD_KEY_NONE,
    /*                   0xA4 */ OSD_KEY_NONE,
    /*                   0xA5 */ OSD_KEY_NONE,
    /*                   0xA6 */ OSD_KEY_NONE,
    /*                   0xA7 */ OSD_KEY_NONE,
    /*                   0xA8 */ OSD_KEY_NONE,
    /*                   0xA9 */ OSD_KEY_NONE,
    /*                   0xAA */ OSD_KEY_NONE,
    /*                   0xAB */ OSD_KEY_NONE,
    /*                   0xAC */ OSD_KEY_NONE,
    /*                   0xAD */ OSD_KEY_NONE,
    /*                   0xAE */ OSD_KEY_NONE,
    /*                   0xAF */ OSD_KEY_NONE,
    /*                   0xB0 */ OSD_KEY_NONE,
    /*                   0xB1 */ OSD_KEY_NONE,
    /*                   0xB2 */ OSD_KEY_NONE,
    /*                   0xB3 */ OSD_KEY_NONE,
    /*                   0xB4 */ OSD_KEY_NONE,
    /*                   0xB5 */ OSD_KEY_NONE,
    /*                   0xB6 */ OSD_KEY_NONE,
    /*                   0xB7 */ OSD_KEY_NONE,
    /*                   0xB8 */ OSD_KEY_NONE,
    /*                   0xB9 */ OSD_KEY_NONE,
    /*                   0xBA */ OSD_KEY_COLON,
    /*                   0xBB */ OSD_KEY_EQUALS,
    /*                   0xBC */ OSD_KEY_COMMA,
    /*                   0xBD */ OSD_KEY_MINUS,
    /*                   0xBE */ OSD_KEY_NONE,
    /*                   0xBF */ OSD_KEY_SLASH,
    /*                   0xC0 */ OSD_KEY_NONE,
    /*                   0xC1 */ OSD_KEY_NONE,
    /*                   0xC2 */ OSD_KEY_NONE,
    /*                   0xC3 */ OSD_KEY_NONE,
    /*                   0xC4 */ OSD_KEY_NONE,
    /*                   0xC5 */ OSD_KEY_NONE,
    /*                   0xC6 */ OSD_KEY_NONE,
    /*                   0xC7 */ OSD_KEY_NONE,
    /*                   0xC8 */ OSD_KEY_NONE,
    /*                   0xC9 */ OSD_KEY_NONE,
    /*                   0xCA */ OSD_KEY_NONE,
    /*                   0xCB */ OSD_KEY_NONE,
    /*                   0xCC */ OSD_KEY_NONE,
    /*                   0xCD */ OSD_KEY_NONE,
    /*                   0xCE */ OSD_KEY_NONE,
    /*                   0xCF */ OSD_KEY_NONE,
    /*                   0xD0 */ OSD_KEY_NONE,
    /*                   0xD1 */ OSD_KEY_NONE,
    /*                   0xD2 */ OSD_KEY_NONE,
    /*                   0xD3 */ OSD_KEY_NONE,
    /*                   0xD4 */ OSD_KEY_NONE,
    /*                   0xD5 */ OSD_KEY_NONE,
    /*                   0xD6 */ OSD_KEY_NONE,
    /*                   0xD7 */ OSD_KEY_NONE,
    /*                   0xD8 */ OSD_KEY_NONE,
    /*                   0xD9 */ OSD_KEY_NONE,
    /*                   0xDA */ OSD_KEY_NONE,
    /*                   0xDB */ OSD_KEY_OPENBRACE,
    /*                   0xDC */ OSD_KEY_NONE,
    /*                   0xDD */ OSD_KEY_CLOSEBRACE,
    /*                   0xDE */ OSD_KEY_QUOTE,
    /*                   0xDF */ OSD_KEY_NONE,
    /*                   0xE0 */ OSD_KEY_NONE,
    /*                   0xE1 */ OSD_KEY_NONE,
    /*                   0xE2 */ OSD_KEY_NONE,
    /*                   0xE3 */ OSD_KEY_NONE,
    /*                   0xE4 */ OSD_KEY_NONE,
    /*                   0xE5 */ OSD_KEY_NONE,
    /*                   0xE6 */ OSD_KEY_NONE,
    /*                   0xE7 */ OSD_KEY_NONE,
    /*                   0xE8 */ OSD_KEY_NONE,
    /*                   0xE9 */ OSD_KEY_NONE,
    /*                   0xEA */ OSD_KEY_NONE,
    /*                   0xEB */ OSD_KEY_NONE,
    /*                   0xEC */ OSD_KEY_NONE,
    /*                   0xED */ OSD_KEY_NONE,
    /*                   0xEE */ OSD_KEY_NONE,
    /*                   0xEF */ OSD_KEY_NONE,
    /*                   0xF0 */ OSD_KEY_NONE,
    /*                   0xF1 */ OSD_KEY_NONE,
    /*                   0xF2 */ OSD_KEY_NONE,
    /*                   0xF3 */ OSD_KEY_NONE,
    /*                   0xF4 */ OSD_KEY_NONE,
    /*                   0xF5 */ OSD_KEY_NONE,
    /*                   0xF6 */ OSD_KEY_NONE,
    /*                   0xF7 */ OSD_KEY_NONE,
    /*                   0xF8 */ OSD_KEY_NONE,
    /*                   0xF9 */ OSD_KEY_NONE,
    /*                   0xFA */ OSD_KEY_NONE,
    /*                   0xFB */ OSD_KEY_NONE,
    /*                   0xFC */ OSD_KEY_NONE,
    /*                   0xFD */ OSD_KEY_NONE,
    /*                   0xFE */ OSD_KEY_NONE,
    /*                   0xFF */ OSD_KEY_NONE,
};

byte key[OSD_NUMKEYS];

LARGE_INTEGER uclocks_per_sec;

DWORDLONG osd_win32_uclock()
{
    LARGE_INTEGER t;

    if (uclocks_per_sec.QuadPart == 1000)
        return (DWORDLONG)timeGetTime();

    QueryPerformanceCounter(&t);
    return (DWORDLONG)t.QuadPart;
}

DWORDLONG osd_win32_uclocks_per_sec()
{
    return (DWORDLONG)uclocks_per_sec.QuadPart;
}

/* put here anything you need to do when the program is started. Return 0 if */
/* initialization was successful, nonzero otherwise. */
int osd_init(int argc,char **argv)
{
    int i;

    /* dprintf("osd_init\n"); */

    first_free_pen = 0;
    //ending = FALSE;

    if (!QueryPerformanceFrequency(&uclocks_per_sec))
        uclocks_per_sec.QuadPart = 1000;

    for (i = 1; i < argc; i++)
    {
        if (stricmp(argv[i],"-nosound") == 0)
        {
            play_sound = FALSE;
            continue;
        }
    }

//   if (is_window)
//      palette_offset = 10;
//   else
    palette_offset = 0;

    // Keep the ESC key from sticking during subsequent
    // emulation runs within the same Mame session
    for (i=0; i<OSD_NUMKEYS; i++)
        key[i]=FALSE;

    osd_win32_joy_init();

    //if (!is_window)
    //   ShowCursor(FALSE);

    // apparently we need to start sound here
    CESound_init();
    sound_start();

    return 0;
}

void osd_win32_sound_init()
{
    // scary not used
}

void osd_win32_joy_init()
{
    // not used
}

/* put here cleanup routines to be executed when the program is terminated. */
void osd_exit(void)
{
    BOOL res;

//   if (!is_window)
//      ShowCursor(TRUE);

    if (IsWindow(hMain))
    {
        res = DestroyWindow(hMain);
    }

    sound_stop();
    CESound_exit();
}

/* Create a bitmap. Also call clearbitmap() to appropriately initialize it to */
/* the background color. */
struct osd_bitmap *osd_create_bitmap(int width,int height)
{
    struct osd_bitmap *bitmap;

    // GN: allocation was too small and causing Windows to trash us when freeing the bitmap
    //if ((bitmap = malloc(sizeof(struct osd_bitmap) + (height-1)*sizeof(unsigned char *))) != 0)
    if ((bitmap = malloc(sizeof(struct osd_bitmap) + (height-0)*sizeof(unsigned char *))) != 0)
    {
        int i;
        unsigned char *bm;

        bitmap->width = width;
        bitmap->height = height;
        if ((bm = malloc(width * height * sizeof(unsigned char))) == 0)
        {
            free(bitmap);
            return 0;
        }

        for (i = 0; i < height; i++)
            bitmap->line[i] = &bm[i * width];

        bitmap->private = bm;

        for (i = 0; i < bitmap->height; i++)
            memset(bitmap->line[i],0,bitmap->width);
    }

    return bitmap;
}

void osd_free_bitmap(struct osd_bitmap *bitmap)
{
    if (bitmap)
    {
        free(bitmap->private);
        free(bitmap);
    }
}

/* Create a display screen, or window, large enough to accomodate a bitmap */
/* of the given dimensions. I don't do any test here (224x288 will just do */
/* for now) but one could e.g. open a window of the exact dimensions */
/* provided. Return a osd_bitmap pointer or 0 in case of error. */
struct osd_bitmap *osd_create_display(int width,int height)
{
    const unsigned short OSD_NUMPENS = 256u;
    int scale = 0; /* placeholder, not used */
    WNDCLASS wndclass;
    int style;
    int wndWidth,wndHeight;

    RECT wr;
    int pitch_space;
    struct osd_bitmap*   bitmap;

    int				i;
    HDC				hDC;
    BYTE*			buffer_ptr = 0;

    m_pInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) +
                                  sizeof(RGBQUAD) * OSD_NUMPENS);
    m_pInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    m_pInfo->bmiHeader.biWidth         = width;
    m_pInfo->bmiHeader.biHeight        = (-1) * height; /* Negative means "top down" */
    m_pInfo->bmiHeader.biPlanes        = 1;
    m_pInfo->bmiHeader.biBitCount      = 8; // This.m_nDepth;
    m_pInfo->bmiHeader.biCompression   = BI_RGB;
    m_pInfo->bmiHeader.biSizeImage     = 0;
    m_pInfo->bmiHeader.biXPelsPerMeter = 0;
    m_pInfo->bmiHeader.biYPelsPerMeter = 0;
    m_pInfo->bmiHeader.biClrUsed       = 0;
    m_pInfo->bmiHeader.biClrImportant  = 0;

    hDC = GetDC(NULL); // DC for DibSection
    m_hMemDC = CreateCompatibleDC(hDC);

    /* create the DIB and setup up a BM handle for Windows - [out]ppvBits is a pointer
     to a variable that receives a pointer to the location of the DIB bit values */
    m_hBitmap = CreateDIBSection(hDC, m_pInfo, DIB_RGB_COLORS, &buffer_ptr, NULL, 0);

    ReleaseDC(NULL, hDC);
    m_hOldBitmap = SelectObject(m_hMemDC, (HGDIOBJ)m_hBitmap);

    dprintf("osd_create_display width %i height %i\n",width,height);

    // why not height-1 (see win32.c), since you get one line ptr included with the sizeof osd_bitmap ;)
    // bitmap = malloc ( sizeof (struct osd_bitmap) + (height-1) * sizeof (unsigned char *) );
    bitmap = malloc( sizeof(struct osd_bitmap) + (height-0)*sizeof(unsigned char *) );

    if ( ! bitmap )
    {
        return (NULL);
    }

    bitmap->width = width;
    bitmap->height = height;

    /* see top for discussion of pitch_space */
    pitch_space = 0;

    for (i = 0; i < height; i++)
        bitmap->line[i] = &buffer_ptr[i * (width+pitch_space)];

    bitmap->private = buffer_ptr;

    m_pMAMEBitmap = bitmap; /* save pointer to DIB Section buffer to be free'd */

    char szAppName[]       = "MAME";
    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_CLASSDC;
    wndclass.lpfnWndProc   = WndProc; /* MAME32_MessageProc */
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = GetModuleHandle(NULL); // hInstance ?
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = szAppName;
    RegisterClass(&wndclass); /* if (RegisterClass(&WndClass) == 0) return NULL; */

    style = WS_OVERLAPPEDWINDOW;

    wndWidth = width*scale + 2*GetSystemMetrics(SM_CXFRAME);
    wndHeight = height*scale + 2*GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    hMain = CreateWindow(szAppName,szAppName,style,280,0,wndWidth,wndHeight,NULL,NULL,
                         wndclass.hInstance,NULL);

    if (!IsWindow(hMain))
    {
        return 1;
    }

    // having set the game window to some (arbitrary) size,
    // get the actual client rect so that we can adjust the window for the
    // title bar and border (there's probably some better way!)
    //GetClientRect(hMain, &wr);

    SetWindowPos(hMain,
                 HWND_NOTOPMOST,
                 0, 0,
                 240, //240 + (240 - wr.right),
                 320, //320 + (320 - wr.bottom), // add the offset for title bar
                 SWP_NOMOVE);

    GetClientRect(hMain, &wr);

    /* do the calcs for screen centering and clipping */
    m_VisibleRect.m_Left = (wr.right > width) ? 0 : (width - wr.right) / 2;
    m_VisibleRect.m_Left += 0x3; // round up
    m_VisibleRect.m_Left &= 0xfffffffc; // qw align

    m_VisibleRect.m_Right  = (wr.right > width) ? width : m_VisibleRect.m_Left + wr.right;
    m_VisibleRect.m_Right &= 0xfffffffc; // round down, qw align

    m_VisibleRect.m_Top = (wr.bottom > height) ? 0 : (height - wr.bottom) / 2;
    m_VisibleRect.m_Top  += 0x3; // round up
    m_VisibleRect.m_Top  &= 0xfffffffc; // qw align

    m_VisibleRect.m_Bottom = (wr.bottom > height) ? height : m_VisibleRect.m_Top + wr.bottom;
    m_VisibleRect.m_Bottom &= 0xfffffffc; // round down, qw align

    m_VisibleRect.m_Width  = m_VisibleRect.m_Right  - m_VisibleRect.m_Left;
    m_VisibleRect.m_Height = m_VisibleRect.m_Bottom - m_VisibleRect.m_Top;

    m_DisplayRect.m_Left = (wr.right - m_VisibleRect.m_Width) / 2;
    m_DisplayRect.m_Left &= 0xfffffffc; // round down, qw align
    m_DisplayRect.m_Right = m_DisplayRect.m_Left + m_VisibleRect.m_Width;

    m_DisplayRect.m_Top = (wr.bottom - m_VisibleRect.m_Height) / 2;
    m_DisplayRect.m_Top &= 0xfffffffc; // round down, qw align
    m_DisplayRect.m_Bottom = m_DisplayRect.m_Top + m_VisibleRect.m_Height;


    osd_win32_sound_init(); /* empty */

    ShowWindow(hMain,SW_SHOWNORMAL);

    return bitmap;
}

long CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT     ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        BitBlt(ps.hdc,
          m_DisplayRect.m_Left,               // x-coord of destination upper-left corner
          m_DisplayRect.m_Top,                // y-coord of destination upper-left corner
          m_pInfo->bmiHeader.biWidth,         //  width of destination rectangle
          (-1) * m_pInfo->bmiHeader.biHeight, // height of destination rectangle
          m_hMemDC,
          m_VisibleRect.m_Left,  // must remember that bitmap width and height are
          m_VisibleRect.m_Top,   // often given from the driver as some larger size
          SRCCOPY);              // than the driver visible area, such as 256x256
        /*
        GetClientRect(hwnd,&rect);
        FillRect(hdc,&rect,(HBRUSH)(1+COLOR_BTNFACE));
        */

        osd_update_display();

        EndPaint(hwnd, &ps);
        //return 0; // ?
        break;
    }
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

    case WM_KEYDOWN:
        if (Win32KeyToOSDKey(wParam) != 0)
        {
            key[Win32KeyToOSDKey(wParam)] = 1;
        }
        break;
    case WM_KEYUP :
        if (Win32KeyToOSDKey(wParam) != 0)
        {
            key[Win32KeyToOSDKey(wParam)] = 0;
        }
        break;

    case WM_LBUTTONDOWN :
        break;

    case WM_MOUSEMOVE :
        osd_win32_mouse_moved(LOWORD(lParam),HIWORD(lParam));
        break;

    case WM_CLOSE:
        //ending = TRUE;
        //DestroyWindow(hMain);
        /* Don't call DestroyWindow, it will be called by osd_exit. */
        //return 0; // ?
        break;

    case WM_ACTIVATEAPP :
        //is_active = (BOOL) wParam;
        break;

    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/* shut up the display */
void osd_close_display(void)
{
//   DirectDrawClose();

//   if (play_sound)
//      ACloseAudio();
//   osd_free_bitmap(bitmap);

    // cleanup the DIB and display bitmap
    SelectObject(m_hMemDC, (HGDIOBJ)m_hOldBitmap);
    DeleteObject(m_hBitmap);
    free(m_pMAMEBitmap);
    free(m_pInfo);
}

int osd_obtain_pen(unsigned char red, unsigned char green, unsigned char blue)
{
    int retval;

    /* set RGBs directly into DIB color table */
    RGBQUAD dibentry;
    dibentry.rgbRed = red;
    dibentry.rgbGreen = green;
    dibentry.rgbBlue = blue;
    SetDIBColorTable(m_hMemDC,first_free_pen, 1, &dibentry);

    /* dprintf("OOP %u %u %u : %i\n", red, green, blue, palette_offset+pal.palNumEntries); */
    retval = (palette_offset+first_free_pen) % 256;
    first_free_pen = (first_free_pen + 1)%256;

    return retval;
}

/* Update the display. */
/* No need to support saving the screen to a file--while running, just hit
   alt-print screen and the display is saved to the clipboard */
void osd_update_display()
{
    // if (!palette_created)
    // {
    //     palette_created = TRUE;
    //     osd_win32_create_palette();
    // }

    InvalidateRect( hMain, NULL, FALSE);
    UpdateWindow(hMain);

//	MAME32_ProcessMessages();

    /* dprintf("Updating display\n"); */

//   if (!is_active && !is_window)
//      return;

    return;
}

/***************************************************************************
    Sound
 ***************************************************************************/

int osd_start_audio_stream(int stereo)
{
    return CESound_start_audio_stream(stereo);
}

int osd_update_audio_stream(INT16* buffer)
{
    return CESound_update_audio_stream(buffer);
}

void osd_stop_audio_stream(void)
{
    CESound_stop_audio_stream(); // currently unimplemented, just here for completeness.
}

void osd_update_audio(void)
{
    if (play_sound == 0)
        return;

    sound_update(); // AUpdateAudio();
}

void osd_play_sample(int channel,unsigned char *data,int len,int freq,int volume,int loop)
{
    if (play_sound == 0 || channel >= NUMVOICES) return;
#if 0
    sample_start2(channel, data, len, freq, volume, loop);
#else
	if (channel >= numchannels)
	{
		if (errorlog) fprintf(errorlog,"error: sample_start() called with channel = %d, but only %d channels allocated\n",channel,numchannels);
		return;
	}

	mixer_play_sample(firstchannel + channel,
				data,
				len,
				freq,
				loop);
#endif
    sample_set_volume(channel, volume);
}

void osd_adjust_sample(int channel,int freq,int volume)
{
    if (play_sound == 0 || channel >= NUMVOICES)
    {
        return;
    }

//   Volumi[channel] = volume/4;
//   ASetVoiceFrequency(hVoice[channel],freq);
//   ASetVoiceVolume(hVoice[channel],MasterVolume*volume/400);
    sample_set_freq(channel, freq);
    sample_set_volume(channel, volume);
}

void osd_stop_sample( int channel )
{
    if (play_sound == 0 || channel >= NUMVOICES)
        return;
    sample_set_volume(channel, 0); //  AStopVoice(hVoice[channel]);
}

/* check if a key is pressed. The keycode is the standard PC keyboard code, as */
/* defined in osdepend.h. Return 0 if the key is not pressed, nonzero otherwise. */
int osd_key_pressed(int keycode)
{
    MSG msg;
    /* dprintf("Check ing for key %i\n",keycode); */

    while (PeekMessage(&msg,hMain,0,0,PM_REMOVE))
    {
        /* dprintf("OKP Handling message %i\n",msg.message); */
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

//   if (keycode == OSD_KEY_ESC && ending == TRUE)//tbd
//      return 1;//tbd

    /* dprintf("returning %i\n",key[keycode]); */
    return key[keycode];
}

/*
 * Wait until a key is pressed and return its code.
 */
int osd_read_key(void)
{
    MSG msg;
    int osd_key;

    while (GetMessage(&msg,hMain,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

//        if (ending) ///tbd
//          break; // tbd

        if (msg.message == WM_KEYDOWN ||
                (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_MENU))
        {
            osd_key = Win32KeyToOSDKey(msg.wParam);
            if (osd_key == 0)
                return OSD_KEY_1; /* in usrintrf.c, it crashes if we return 0 sometimes */
            else
                return osd_key;
        }
    }
    osd_exit();
    ExitProcess(1);
    return 1;
}

int Win32KeyToOSDKey(UINT vk)
{
    if (vk <= 255)
    {
        return key_code_table[vk];
    }

    return OSD_KEY_NONE;
}

void osd_poll_joystick(void)
{
}

void dprintf(char *fmt,...)
{
    char s[500];
    DWORD written;
    va_list marker;

    if (!win32_debug)
        return;

    va_start(marker,fmt);

    vsprintf(s,fmt,marker);

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,strlen(s),&written,NULL);
    {
        static FILE *junk;
        if (junk == NULL)
            junk = fopen("debug.txt","wt");
        if (junk != NULL)
            fprintf(junk, s);
    }
    va_end(marker);
}

void osd_win32_mouse_moved(int x,int y)
{
//    mouse_x = x;
//    mouse_y = y;
//    /* printf("%i %i\n",mouse_x,mouse_y); */
}


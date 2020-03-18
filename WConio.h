// Old Turbo-C CONIO.H compatibility library for LCC-Win32
// and GCC/EGCS Mingw32 compilers.
// Version 1.0 (September 1999).
// Created by Daniel Guerrero Miralles (daguer@geocities.com).
// This source code is public domain.

#include <Windows.h>
#include <conio.h>

#ifdef __LCC__
// Mingw32 does not include inps in its run-time lib.
#define inport inpw
#define inportb inp
#define outport outpw
#define outportw outp
#endif //__LCC__

extern int directvideo; // useless, only for compatibility.
extern int _wscroll; // also useless.


// CONTROL FUNCTIONS //////////////////////////////////////////////////////

// Get some information about the console state
struct text_info
{
	unsigned char winleft;
	unsigned char wintop;
	unsigned char winright;
	unsigned char winbottom;
	unsigned char attribute;
	unsigned char normattr;
	unsigned char currmode;
	unsigned char screenheight;
	unsigned char screenwidth;
	unsigned char curx;
	unsigned char cury;
};

// set the video mode
enum text_modes
{
	LASTMODE = -1,
	BW40,
	C40,
	BW80,
	C80,
	MONO = 7,
	C4350 = 64
};

// set the cursor shape
#define _NOCURSOR 0
#define _SOLIDCURSOR 1
#define _NORMALCURSOR 2


// COLOR FUNCTIONS ////////////////////////////////////////////////////////

// set the text foreground/background colors
#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED				4
#define MAGENTA			5
#define BROWN			6
#define LIGHTGRAY		7
#define DARKGRAY		8
#define LIGHTBLUE		9
#define LIGHTGREEN		10
#define LIGHTCYAN		11
#define LIGHTRED		12
#define LIGHTMAGENTA	13
#define YELLOW			14
#define WHITE			15
#define BLINK			128 // Useless, background uses high intensity colors.


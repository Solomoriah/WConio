// Old Turbo-C CONIO.H compatibility library for LCC-Win32
// and GCC/EGCS Mingw32 compilers.
// Version 1.0 (September 1999).
// Created by Daniel Guerrero Miralles (daguer@geocities.com).
// This source code is public domain.

#include <Python.h>
#include <Windows.h>
#include <malloc.h>
#include <conio.h>
#include "WConio.h"

#define NORM_ATTR LIGHTGRAY
#define NORM_CURSOR_SIZE 10
#define SOLID_CURSOR_SIZE 99

static PyObject *WConioError;

// INTERNAL FUNCTIONS /////////////////////////////////////////////////////


static HANDLE WINAPI GetConHandle (TCHAR * pszName)
{
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof (sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Using CreateFile we get the true console handle, avoiding
	// any redirection.

	return CreateFile (pszName,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		&sa,
		OPEN_EXISTING,
		(DWORD) 0,
		(HANDLE) 0);
}

#define GetConOut() GetConHandle(TEXT("CONOUT$"))
#define GetConIn() GetConHandle(TEXT("CONIN$"))
#define ReleaseConOut(h) CloseHandle(h)
#define ReleaseConIn(h) CloseHandle(h)

// clear the screen
static int int_clrscr (void)
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD coord;
	DWORD dwDummy;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo (hConOut, &csbi)) {
			coord.X = 0;
			coord.Y = 0;
			FillConsoleOutputCharacter (hConOut, (TCHAR)' ', 
                csbi.dwSize.X * csbi.dwSize.Y, coord, &dwDummy);
			FillConsoleOutputAttribute (hConOut, csbi.wAttributes, 
                csbi.dwSize.X * csbi.dwSize.Y, coord, &dwDummy);
			SetConsoleCursorPosition (hConOut, coord);
		}
		ReleaseConOut (hConOut);

        return 0;
	}

    return -1;
}


// set the text foreground/background colors
static int int_textattr(int newattr)
{
	HANDLE hConOut;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		SetConsoleTextAttribute (hConOut, (WORD) newattr);
		ReleaseConOut (hConOut);
        return 0;
	}

    return -1;
}


// set the cursor shape
static void int_setcursortype(int cur_t)
{
	HANDLE hConOut;
	CONSOLE_CURSOR_INFO cci;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		switch (cur_t)
		{
			case _NOCURSOR:
				cci.dwSize = NORM_CURSOR_SIZE;
				cci.bVisible = FALSE;
				break;
			case _SOLIDCURSOR:
				cci.dwSize = SOLID_CURSOR_SIZE;
				cci.bVisible = TRUE;
				break;
			case _NORMALCURSOR:
				cci.dwSize = NORM_CURSOR_SIZE;
				cci.bVisible = TRUE;
				break;
		}
		SetConsoleCursorInfo (hConOut, &cci);
		ReleaseConOut (hConOut);
	}
}


static BOOL WINAPI GetScreenInfo(PCONSOLE_SCREEN_BUFFER_INFO pCSBI)
{
	HANDLE hOut;
	BOOL fSuccess;

	hOut = GetConOut();
	if (hOut != INVALID_HANDLE_VALUE) {
		fSuccess = GetConsoleScreenBufferInfo (hOut, pCSBI);
		ReleaseConOut (hOut);
		return fSuccess;
	}
	return FALSE;
}


static void WINAPI ConPuts(const char * pszText)
{
	HANDLE hConOut;
	DWORD dwDummy;
	DWORD dwLen;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE) {
		dwLen = lstrlenA (pszText);
		WriteConsoleA (hConOut, pszText, dwLen, &dwDummy, NULL);
		ReleaseConOut (hConOut);
	}
}


// CONTROL FUNCTIONS //////////////////////////////////////////////////////

static PyObject *
gettextinfo(self, args)
PyObject *self;
PyObject *args;
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    
	if(GetScreenInfo (&csbi))
        return Py_BuildValue("(i,i,i,i,i,i,i,i,i,i,i)",
		    csbi.srWindow.Left,
		    csbi.srWindow.Top,
		    csbi.srWindow.Right,
		    csbi.srWindow.Bottom,
		    csbi.wAttributes,
		    NORM_ATTR, C80,
		    csbi.dwSize.Y,
		    csbi.dwSize.X,
		    csbi.dwCursorPosition.X,
		    csbi.dwCursorPosition.Y);

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetScreenInfo Failed"));
    return NULL;
}


// CURSOR FUNCTIONS ///////////////////////////////////////////////////////

// get cursor horizontal possition
static PyObject *
wherex(self, args)
PyObject *self;
PyObject *args;
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	if(GetScreenInfo(&csbi))
		return Py_BuildValue("i", csbi.dwCursorPosition.X);

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetScreenInfo Failed"));
    return NULL;
}


// get cursor vertical possition
static PyObject *
wherey(self, args)
PyObject *self;
PyObject *args;
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	if (GetScreenInfo(&csbi))
		return Py_BuildValue("i", csbi.dwCursorPosition.Y);

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetScreenInfo Failed"));
    return NULL;
}

// Change the cursor location
static PyObject *
gotoxy(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hOut;
	COORD coord;
    int x, y;

    if (!PyArg_ParseTuple(args, "ii", &x, &y))
        return NULL;

	hOut = GetConOut();
	if (hOut != INVALID_HANDLE_VALUE)
	{
		coord.X = x;
		coord.Y = y;
		SetConsoleCursorPosition(hOut, coord);
		ReleaseConOut(hOut);
        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


static PyObject *
setcursortype(self, args)
PyObject *self;
PyObject *args;
{
    int cur_t;

    if (!PyArg_ParseTuple(args, "i", &cur_t))
        return NULL;

    switch(cur_t) {
    case 0:
        int_setcursortype(_NOCURSOR);
        break;
    case 1:
        int_setcursortype(_NORMALCURSOR);
        break;
    case 2:
        int_setcursortype(_SOLIDCURSOR);
        break;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


// change text color to high intensity
static PyObject *
highvideo(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(hConOut, &csbi)) {
			SetConsoleTextAttribute (hConOut, 
                csbi.wAttributes | FOREGROUND_INTENSITY);
		}
		ReleaseConOut(hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}

// change text color to low intensity
static PyObject *
lowvideo(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo (hConOut, &csbi)) {
			SetConsoleTextAttribute(hConOut, 
                csbi.wAttributes & (~FOREGROUND_INTENSITY));
		}
		ReleaseConOut(hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


static PyObject *
textattr(self, args)
PyObject *self;
PyObject *args;
{
    int attr;

    if (!PyArg_ParseTuple(args, "i", &attr))
        return NULL;

    if(int_textattr(attr) != 0) {
        PyErr_SetObject(WConioError, 
            Py_BuildValue("s", "textattr Failed"));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


// TEXT I/O FUNCTIONS /////////////////////////////////////////////////////

//clear up to end-of-line
static PyObject *
clreol(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwDummy;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(hConOut, &csbi)) {
			FillConsoleOutputCharacter(hConOut, (TCHAR)' ', 
                csbi.dwSize.X - csbi.dwCursorPosition.X, 
                csbi.dwCursorPosition, &dwDummy);
			FillConsoleOutputAttribute(hConOut, csbi.wAttributes, 
                csbi.dwSize.X - csbi.dwCursorPosition.X, 
                csbi.dwCursorPosition, &dwDummy);
		}
		ReleaseConOut(hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


// clear the screen
static PyObject *
clrscr(self, args)
PyObject *self;
PyObject *args;
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	if(int_clrscr() != 0) {
        PyErr_SetObject(WConioError, 
            Py_BuildValue("s", "clrscr Failed"));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


// delete current line
static PyObject *
delline(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT srSource;
	COORD dwDest;
	CHAR_INFO ciFill;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo (hConOut, &csbi))
		{
			srSource.Top = csbi.dwCursorPosition.Y;
			srSource.Left = csbi.srWindow.Left;
			srSource.Bottom = csbi.srWindow.Bottom;
			srSource.Right = csbi.srWindow.Right;
			dwDest.X = csbi.srWindow.Left;
			dwDest.Y = csbi.dwCursorPosition.Y;
			// FIXME!
			// this may not work in an big-endian machine.
			ciFill.Char.UnicodeChar = (WCHAR)' ';
			ciFill.Attributes = csbi.wAttributes;
			ScrollConsoleScreenBuffer (hConOut, &srSource, NULL, dwDest, &ciFill);
		}
		ReleaseConOut (hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


// insert a line
static PyObject *
insline(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT srSource;
	COORD dwDest;
	CHAR_INFO ciFill;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo (hConOut, &csbi))
		{
			srSource.Top = csbi.dwCursorPosition.Y;
			srSource.Left = csbi.srWindow.Left;
			srSource.Bottom = csbi.srWindow.Bottom;
			srSource.Right = csbi.srWindow.Right;
			dwDest.X = csbi.srWindow.Left;
			dwDest.Y = csbi.dwCursorPosition.Y;
			// FIXME!
			// This may not work in a big-endian machine.
			ciFill.Char.UnicodeChar = (WCHAR)' ';
			ciFill.Attributes = csbi.wAttributes;
			ScrollConsoleScreenBuffer (hConOut, &srSource, NULL, dwDest, &ciFill);
		}
		ReleaseConOut (hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


// get text from a rectangular section of the screen
static PyObject *
gettext(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	PCHAR_INFO pCI;
	COORD dwBufferSize, dwBufferOrg;
	SMALL_RECT srSource;
	DWORD dwBuffLen, i;
	char destin[200*100*2]; // make it big and don't worry about it.
    char *pchDest;
    int left, top, right, bottom;

    if (!PyArg_ParseTuple(args, "iiii", &left, &top, &right, &bottom))
        return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE) {
		srSource.Left = left;
		srSource.Top = top;
		srSource.Right = right;
		srSource.Bottom = bottom;
		dwBufferSize.X = srSource.Right - srSource.Left + 1;
		dwBufferSize.Y = srSource.Bottom - srSource.Top + 1;
		dwBufferOrg.X = 0;
		dwBufferOrg.Y = 0;
		dwBuffLen = dwBufferSize.X * dwBufferSize.Y;

		pCI = malloc (sizeof (CHAR_INFO) * dwBuffLen);
		if (pCI != NULL) {
			if (ReadConsoleOutputA (hConOut, pCI, dwBufferSize, 
                dwBufferOrg, &srSource))
			{
				pchDest = (char *) destin;
				// convert to the expected output format
				for (i = 0; i < dwBuffLen; i++) {
					*pchDest = (char) pCI[i].Char.AsciiChar;
					pchDest++;
					*pchDest = (char) pCI[i].Attributes;
					pchDest++;
				}
			}
			free (pCI);
		}
		ReleaseConOut (hConOut);

        return Py_BuildValue("s#", destin, dwBuffLen*2);
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


// Set text in a rectangular section of the screen
static PyObject *
puttext(self, args)
PyObject *self;
PyObject *args;
{
	HANDLE hConOut;
	PCHAR_INFO pCI;
	COORD dwBufferSize, dwBufferOrg;
	SMALL_RECT srDest;
	DWORD dwBuffLen, i;
	char *pchSource;
    int left, top, right, bottom, len;
    char *source;

    if (!PyArg_ParseTuple(args, "iiiis#", 
        &left, &top, &right, &bottom, &source, &len))
            return NULL;

	hConOut = GetConOut();
	if (hConOut != INVALID_HANDLE_VALUE) {
		srDest.Left = left;
		srDest.Top = top;
		srDest.Right = right; // coords are inclusive
		srDest.Bottom = bottom;
		dwBufferSize.X = srDest.Right - srDest.Left + 1;
		dwBufferSize.Y = srDest.Bottom - srDest.Top + 1;
		dwBufferOrg.X = 0;
		dwBufferOrg.Y = 0;
		dwBuffLen = dwBufferSize.X * dwBufferSize.Y;

        if((dwBuffLen * 2) > len) {
            PyErr_SetObject(WConioError, 
                Py_BuildValue("s", "String Too Short"));
            return NULL;
        }

		pCI = malloc (sizeof (CHAR_INFO) * dwBuffLen);
		if (pCI != NULL) {
			pchSource = source;
			for (i = 0; i < dwBuffLen; i++) {
				pCI[i].Char.AsciiChar = (CHAR) *pchSource;
				pchSource++;
				pCI[i].Attributes = (WORD) *pchSource;
				pchSource++;
			}
			WriteConsoleOutputA (hConOut, pCI, dwBufferSize, 
                dwBufferOrg, &srDest);
			free (pCI);
		}
		ReleaseConOut (hConOut);

        Py_INCREF(Py_None);
        return Py_None;
	}

    PyErr_SetObject(WConioError, 
        Py_BuildValue("s", "GetConOut Failed"));
    return NULL;
}


static PyObject *
settitle(self, args)
PyObject *self;
PyObject *args;
{
    char *title;

    if(!PyArg_ParseTuple(args, "s", &title))
        return NULL;

    if(SetConsoleTitle(title) == 0) {
        PyErr_SetObject(WConioError,
            Py_BuildValue("s", "SetConsoleTitle Failed"));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
meth_kbhit(self, args)
PyObject *self;
PyObject *args;
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    return Py_BuildValue("i", kbhit());
}


static PyObject *
meth_getch(self, args)
PyObject *self;
PyObject *args;
{
    char buf[2];
    int rc;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    buf[0] = rc = getch();
    buf[1] = '\0';

    return Py_BuildValue("(is)", rc, buf);
}


static PyObject *
meth_putch(self, args)
PyObject *self;
PyObject *args;
{
    char buf;

    if (!PyArg_ParseTuple(args, "b", &buf)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "c", &buf))
            return NULL;
    }

    putch(buf);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
meth_ungetch(self, args)
PyObject *self;
PyObject *args;
{
    char buf;

    if(!PyArg_ParseTuple(args, "b", &buf)) {
        PyErr_Clear();

        if (!PyArg_ParseTuple(args, "c", &buf))
            return NULL;
    }

    ungetch(buf);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef WConioMethods[] = {
    {"gettextinfo",    gettextinfo,    METH_VARARGS},
    {"wherex",         wherex,         METH_VARARGS},
    {"wherey",         wherey,         METH_VARARGS},
    {"gotoxy",         gotoxy,         METH_VARARGS},
    {"highvideo",      highvideo,      METH_VARARGS},
    {"lowvideo",       lowvideo,       METH_VARARGS},
    {"textattr",       textattr,       METH_VARARGS},
    {"setcursortype",  setcursortype,  METH_VARARGS},
    {"clrscr",         clrscr,         METH_VARARGS},
    {"clreol",         clreol,         METH_VARARGS},
    {"insline",        insline,        METH_VARARGS},
    {"delline",        delline,        METH_VARARGS},
    {"gettext",        gettext,        METH_VARARGS},
    {"puttext",        puttext,        METH_VARARGS},
    {"settitle",       settitle,       METH_VARARGS},
    {"kbhit",          meth_kbhit,     METH_VARARGS},
    {"getch",          meth_getch,     METH_VARARGS},
    {"putch",          meth_putch,     METH_VARARGS},
    {"ungetch",        meth_ungetch,   METH_VARARGS},
    {NULL,             NULL,           0} /* Sentinel */
};


void
init_WConio()
{
    PyObject *m, *d;

    m = Py_InitModule("_WConio", WConioMethods);
    d = PyModule_GetDict(m);
    WConioError = PyErr_NewException("_WConio.error", NULL, NULL);
    PyDict_SetItemString(d, "error", WConioError);
}


// end of file.

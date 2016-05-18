// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* Debug.C
*
* Debug output routines
*
*****************************************************************************/

#ifdef   DEBUG
#include <windows.h>
#include <mmsystem.h>  
#include <stdarg.h>
#include <strsafe.h>

#include "global.h"
#include "debug.h"

PRIVATE VOID FAR CDECL DbgVPrintF(LPSTR szFmt, LPSTR va);

BOOL                        __gfDbgEnabled  = TRUE;
UINT                        __guDbgLevel    = 0;   

WORD                        wDebugLevel     = 0;

/*****************************************************************************
*
* WinAssert
*
* Cause a debug out on an assert; breaking to the debugger if installed.
*
* lpstrExp                  - Assert expression string
* lpstrFile                 - File of assert
* dwLine                    - Line number of assert
*
* Handle WM_CREATE message to main application window.
*
* HWND hWnd                 - Window handle
* CREATESTRUCT FAR* lpCreateStruct
*                           - Pointer to creation parameters for the window.
*
*****************************************************************************/
VOID WINAPI WinAssert(
    LPSTR                   lpstrExp,
    LPSTR                   lpstrFile,
    DWORD                   dwLine)
{
    static char szWork[256];
    static char BCODE szFormat[] =
        "!Assert: %s#%lu [%s]";

    dprintf(0, (LPSTR)szFormat, (LPSTR)lpstrFile, dwLine, (LPSTR)lpstrExp);
}

/*****************************************************************************
*
* DbgVPrintF
*
* Format and print a debug output string
*
* szFmt                     - Format string
* va                        - printf style argument list
*
* The following characters have special meanings when they are
* the first in the string:
*
*   !   Causes a break into the debugger after the output has been printed
*   `   Causes no module prefix to be printed
*   ~   Causes no CR/LF pair to follow the string
*
*****************************************************************************/
VOID FAR CDECL DbgVPrintF(
    LPSTR                   szFmt, 
    LPSTR                   va)
{
    char                    ach[DEBUG_MAX_LINE_LEN];
    BOOL                    fDebugBreak = FALSE;
    BOOL                    fPrefix     = TRUE;
    BOOL                    fCRLF       = TRUE;

    HRESULT                 hr          = S_OK;
    char                    *pDestEnd   = NULL;
    size_t                  cchRemaining = 0;


    ach[0] = '\0';


    for (;;)
    {
        switch(*szFmt)
        {
            case '!':
                fDebugBreak = TRUE;
                szFmt++;
                continue;

            case '`':
                fPrefix = FALSE;
                szFmt++;
                continue;

            case '~':
                fCRLF = FALSE;
                szFmt++;
                continue;
        }

        break;
    }

    if (fDebugBreak)
    {
        ach[0] = '\007';
        ach[1] = '\0';
    }


    if (fPrefix)
    {
        hr = StringCchCatExA(ach, DEBUG_MAX_LINE_LEN, DEBUG_MODULE_NAME ": ", &pDestEnd, &cchRemaining, STRSAFE_NULL_ON_FAILURE);
    }
    else
    {
        pDestEnd = ach;
        cchRemaining = DEBUG_MAX_LINE_LEN;
    }

    if (SUCCEEDED(hr))
    {
        hr = StringCchVPrintfA(pDestEnd, cchRemaining, szFmt, (va_list)va);
    }
        


    if (SUCCEEDED(hr))
    {
        if (fCRLF)
        {
            hr = StringCchCatA(ach, DEBUG_MAX_LINE_LEN, "\r\n");
        }
    }

    if (SUCCEEDED(hr))
    {
        OutputDebugStringA(ach);
    }

    if (fDebugBreak)
        DebugBreak();
}

/*****************************************************************************
*
* dprintf
*
* User-level function to print a debug string
*
* uDbgLevel                 - Debug level of this message
* szFmt                     - Format string for this message
* ...                       - printf style arguments
*
* The message will be printed only if debugging is enabled and the messages's
* debug level is less than or equal to the current debug level.
*
* This function is called by the DPF() macro in Debug.H.
*
*****************************************************************************/
void FAR CDECL dprintf(
    UINT                    uDbgLevel, 
    LPSTR                   szFmt, 
    ...)
{
    va_list                 va;

    if (!__gfDbgEnabled || (__guDbgLevel < uDbgLevel))
        return;

    va_start(va, szFmt);
    DbgVPrintF(szFmt, (LPSTR)va);
    va_end(va);
}


/*****************************************************************************
*
* DbgEnable
*
* Enable or disable debug output
*
* fEnable                   - Enable or disable output
*
* Returns the old state.
*
*****************************************************************************/
BOOL WINAPI DbgEnable(
    BOOL                    fEnable)
{
    BOOL                    fOldState;

    fOldState      = __gfDbgEnabled;
    __gfDbgEnabled = fEnable;

    return (fOldState);
}

/*****************************************************************************
*
* DbgSetLevel
*
* Set the current level for debug output
*
* uLevel                    - New level for debug output
*
* Returns the old level.
*
*****************************************************************************/
UINT WINAPI DbgSetLevel(
    UINT                    uLevel)
{
    UINT                    uOldLevel;

    uOldLevel    = __guDbgLevel;
    __guDbgLevel = wDebugLevel = uLevel;

    return (uOldLevel);
}


/*****************************************************************************
*
* DbgInitialize
*
* Get debug output settings from WIN.INI for this module
*
* fEnable                   - Enable or disable debug output
*
* Returns the current debug level
*
* The debug level will be read from the [debug] section of WIN.INI,
* from an entry with this module's name as #define'd in Debug.H.
*
* WIN.INI:
*  [Debug]
*  Module=3
*
*****************************************************************************/
UINT WINAPI DbgInitialize(BOOL fEnable)
{
    DbgSetLevel(GetProfileIntA(DEBUG_SECTION, DEBUG_MODULE_NAME, 0));
    DbgEnable(fEnable);

    return (__guDbgLevel);
} 


#endif

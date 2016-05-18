//------------------------------------------------------------------------------
// File: Main.cpp
//
// Desc: DirectShow sample code - simple movie player console application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//   This program uses the PlayCutscene() function provided in cutscene.cpp.  
//   It is only necessary to provide the name of a file and the application's 
//   instance handle.
//
//   If the file was played to the end, PlayCutscene returns S_OK.
//   If the user interrupted playback, PlayCutscene returns S_FALSE.
//   Otherwise, PlayCutscene will return an HRESULT error code.
//
//   Usage: cutscene <required file name>
//

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <strsafe.h>

#include "cutscene.h"

#define USAGE \
        TEXT("Cutscene is a console application that demonstrates\r\n")      \
        TEXT("playing a movie at the beginning of your game.\r\n\r\n")       \
        TEXT("Please provide a valid filename on the command line.\r\n")     \
        TEXT("\r\n            Usage: cutscene <filename>\r\n")               \


//
// Main program code
//
int APIENTRY wWinMain (
         HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         LPWSTR lpszMovie,
         int nCmdShow
         )
{
    HRESULT hr;

    // Prevent C4100: unreferenced formal parameter
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);


    // If no filename is specified, show an error message and exit
    if (lpszMovie[0] == TEXT('\0'))
    {
        MessageBox(NULL, USAGE, TEXT("Cutscene Error"), MB_OK | MB_ICONERROR);
        exit(1);
    }

    // Play movie
    hr = PlayCutscene(lpszMovie, hInstance);

    return (SUCCEEDED(hr) ? 0 : 1);
}



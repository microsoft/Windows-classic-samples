// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <windows.h>
#include <stdio.h>

/************************************************************************************************************
Note: This sample relies on FMAPI, which can only be used in the Windows Preinstallation Environment (WinPE).
*************************************************************************************************************/

//
//Define program specific values
//
#define VOLUME L"\\\\.\\C:"

//
//Define the needed FMAPI structures as documented in FMAPI
//
#define FILE_RESTORE_MAJOR_VERSION_2    0x0002
#define FILE_RESTORE_MINOR_VERSION_2    0x0000
#define FILE_RESTORE_VERSION_2          ((FILE_RESTORE_MAJOR_VERSION_2 << 16) | FILE_RESTORE_MINOR_VERSION_2)

typedef PVOID PFILE_RESTORE_CONTEXT;

typedef enum  {
  ContextFlagVolume                   = 0x00000001,
  ContextFlagDisk                     = 0x00000002,
  FlagScanRemovedFiles                = 0x00000004,
  FlagScanRegularFiles                = 0x00000008,
  FlagScanIncludeRemovedDirectories   = 0x00000010 
} RESTORE_CONTEXT_FLAGS;

typedef BOOL (WINAPI *FuncCreateFileRestoreContext) (
    _In_  PCWSTR                Volume,
    _In_  RESTORE_CONTEXT_FLAGS Flags,
    _In_  LONGLONG              StartSector,
    _In_  LONGLONG              BootSector,
    _In_  DWORD                 Version,
    _Out_ PFILE_RESTORE_CONTEXT* Context
    );

typedef BOOL (WINAPI *FuncCloseFileRestoreContext) (
  _In_  PFILE_RESTORE_CONTEXT Context
);

//
//Program entry point
//
void __cdecl wmain(void)
{
    HMODULE hLib;
    
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0); 

    //Load the FMAPI DLL
    hLib = ::LoadLibraryEx(L"fmapi.dll", NULL, NULL);    
    if ( !hLib )
    {
        wprintf(L"Could not load fmapi.dll, Error #%d.\n", GetLastError());
        return;
    }
    
    //Dynamically link to the needed FMAPI functions
    FuncCreateFileRestoreContext CreateFileRestoreContext;
    CreateFileRestoreContext = reinterpret_cast<FuncCreateFileRestoreContext>( GetProcAddress( hLib, "CreateFileRestoreContext" ) );
    
    FuncCloseFileRestoreContext CloseFileRestoreContext;
    CloseFileRestoreContext = reinterpret_cast<FuncCloseFileRestoreContext>( GetProcAddress( hLib, "CloseFileRestoreContext" ) );

    //Call CreateFileRestoreContext with the FMAPI version number we are expecting to use
    PFILE_RESTORE_CONTEXT context = NULL;    
    RESTORE_CONTEXT_FLAGS flags = (RESTORE_CONTEXT_FLAGS)(ContextFlagVolume | FlagScanRemovedFiles);
    if (CreateFileRestoreContext(VOLUME, flags, 0, 0, FILE_RESTORE_VERSION_2, &context))
    {
        wprintf(L"Version Check Succeeded.");
    }
    else
    {
        DWORD err = GetLastError();        
        if (ERROR_INVALID_PARAMETER == err)
        {
            wprintf(L"Version Check Failed.");
        } 
        else
        {
            wprintf(L"Failed to Create FileRestoreContext, Error #%d.\n", err);
        }
    }
    
    //Close the context
    if (context)
    {
        CloseFileRestoreContext(context);
        context = NULL;
    }

}

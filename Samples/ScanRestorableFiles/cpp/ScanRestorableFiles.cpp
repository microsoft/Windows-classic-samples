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
#define SCAN_PATH L"\\"

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

typedef struct _RESTORABLE_FILE_INFO
{
    ULONG           Size;
    DWORD           Version;
    ULONGLONG       FileSize;
    FILETIME        CreationTime;
    FILETIME        LastAccessTime;
    FILETIME        LastWriteTime;
    DWORD           Attributes;
    BOOL            IsRemoved;
    LONGLONG        ClustersUsedByFile;
    LONGLONG        ClustersCurrentlyInUse;
    ULONG           RestoreDataOffset;
    WCHAR           FileName[1];
} RESTORABLE_FILE_INFO, *PRESTORABLE_FILE_INFO;

typedef BOOL (WINAPI *FuncScanRestorableFiles) (
  _In_   PFILE_RESTORE_CONTEXT Context,
  _In_   PCWSTR Path,
  _In_   ULONG FileInfoSize,
  _Out_writes_bytes_opt_(FileInfoSize)  PRESTORABLE_FILE_INFO FileInfo,
  _Out_  PULONG FileInfoUsed
);

HMODULE hLib;

void Scan(_In_ PFILE_RESTORE_CONTEXT context, _In_ LPCWSTR path)
{
    //Dynamically link to the needed FMAPI functions
    FuncScanRestorableFiles ScanRestorableFiles;
    ScanRestorableFiles = reinterpret_cast<FuncScanRestorableFiles>( GetProcAddress( hLib, "ScanRestorableFiles" ) );
    
    ULONG neededBufferSize = 0;
    BOOL success = TRUE;
    RESTORABLE_FILE_INFO tempFileInfo;
	
    //Call ScanRestorableFiles the first time with a size of 0 to get the required buffer size
    if ( ! ScanRestorableFiles(context, path, 0, &tempFileInfo, &neededBufferSize) )
    {
        wprintf(L"Failed to retrieve required buffer size, Error: #%u\n", GetLastError());
        return;
    }
        
    //Create the buffer needed to hold restoration information
    BYTE *buffer = new BYTE[neededBufferSize];

    //Loops until an error occurs or no more files found
    while (success)
    {        
        PRESTORABLE_FILE_INFO fileInfo = reinterpret_cast<PRESTORABLE_FILE_INFO>(buffer);
        #pragma warning( push ) 
        #pragma warning( disable : 6386 ) /* warning is ignored since fileInfo grows in size by design */
        success = ScanRestorableFiles(context, path, neededBufferSize, fileInfo, &neededBufferSize);
        #pragma warning( pop )
        if (success)
        {                        
            if (fileInfo->IsRemoved)
            {
                wprintf(L"Restorable file found: %s\n", fileInfo->FileName);
            }
        } 
        else
        {
            DWORD err = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == err)
            {
                delete [] buffer;
                buffer = new BYTE[neededBufferSize];
                success = true;
            } 
            else if (ERROR_NO_MORE_FILES == err)
            {
                wprintf(L"Scanning Complete.\n");
                success = false;
            }
            else
            {
                wprintf(L"ScanRestorableFiles, Error #%u.\n", err);
            }
        }
    }
    
    delete [] buffer;
    buffer = NULL;
}

//
//program entry point
//
void __cdecl wmain(void)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0); 

    //Load the FMAPI DLL
    hLib = ::LoadLibraryEx(L"fmapi.dll", NULL, NULL);    
    if ( !hLib )
    {
        wprintf(L"Could not load fmapi.dll. Error #%u.\n", GetLastError());
        return;
    }
    
    //Dynamically link to the needed FMAPI functions
    FuncCreateFileRestoreContext CreateFileRestoreContext;
    CreateFileRestoreContext = reinterpret_cast<FuncCreateFileRestoreContext>( GetProcAddress( hLib, "CreateFileRestoreContext" ) );
    
    FuncCloseFileRestoreContext CloseFileRestoreContext;
    CloseFileRestoreContext = reinterpret_cast<FuncCloseFileRestoreContext>( GetProcAddress( hLib, "CloseFileRestoreContext" ) );

    //Create the FileRestoreContext
    PFILE_RESTORE_CONTEXT context = NULL;    
    RESTORE_CONTEXT_FLAGS flags = (RESTORE_CONTEXT_FLAGS)(ContextFlagVolume | FlagScanRemovedFiles);
    if ( ! CreateFileRestoreContext(VOLUME, flags, 0, 0, FILE_RESTORE_VERSION_2, &context) )
    {        
        DWORD err = GetLastError();        
        wprintf(L"Failed to Create FileRestoreContext, Error #%u.\n", err);
        return;
    }
    
    //Find restorable files starting at the given directory
    Scan(context, SCAN_PATH);
    
    //Close the context
    if (context)
    {
        CloseFileRestoreContext(context);
        context = NULL;
    }
}

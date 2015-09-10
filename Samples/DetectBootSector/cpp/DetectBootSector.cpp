// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/************************************************************************************************************
Note: This sample relies on FMAPI, which can only be used in the Windows Preinstallation Environment (WinPE).
*************************************************************************************************************/

#include <windows.h>
#include <stdio.h>

//
//Define program specific values
//
#define BOOT_SECTOR_SIZE 512
#define VOLUME L"\\\\.\\C:"

//
//Define the needed FMAPI structures as documented in FMAPI
//
typedef enum  {
  FileSystemUnknown,
  FileSystemFAT12,
  FileSystemFAT16,
  FileSystemFAT32,
  FileSystemNTFS 
} BOOT_SECTOR_FILE_SYSTEM_TYPE;

typedef struct _BOOT_SECTOR_INFO {
  LONGLONG                     TotalSectors;
  BOOT_SECTOR_FILE_SYSTEM_TYPE FileSystem;
  ULONG                        BytePerSector;
  ULONG                        SectorPerCluster;
  BOOL                         IsEncrypted;
} BOOT_SECTOR_INFO, *PBOOT_SECTOR_INFO;

typedef BOOL (WINAPI *FuncDetectBootSector) (
  _In_   CONST UCHAR* BootSector,
  _Out_  PBOOT_SECTOR_INFO BootSectorParams
);

//
//Read the first BOOT_SECTOR_SIZE bytes of a given volume
//
_Success_(return)
bool ReadVolumeBytes(_In_ LPCWSTR vol, _Out_writes_all_(BOOT_SECTOR_SIZE) BYTE* buffer)
{
    DWORD bytesRead;
    
    //Initialize buffer with nulls
    memset(buffer, 0, BOOT_SECTOR_SIZE);
	
    //Open the volume
    HANDLE fileHandle = CreateFileW(vol, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!fileHandle)
    {        
        return false;
    }
    
    //Read the fist BOOT_SECTOR_SIZE bytes
    BOOL success = ReadFile(fileHandle, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL);    
    
    //close the volume
    CloseHandle(fileHandle);
    
    return (success && BOOT_SECTOR_SIZE == bytesRead);    
}

//
//Program entry point
//
void __cdecl wmain(void)
{
    HMODULE hLib;
    BYTE bootSector[BOOT_SECTOR_SIZE];
    
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0); 

    //Load FMAPI.DLL
    hLib = ::LoadLibraryEx(L"fmapi.dll", NULL, NULL);
    if ( !hLib )
    {
        wprintf(L"Could not load fmapi.dll. Error #%d\n", GetLastError());
        return;
    }
    
    //Read the first BOOT_SECTOR_SIZE bytes on the volume
    if ( !ReadVolumeBytes(VOLUME, bootSector) )
    {
        wprintf(L"Error reading from volume, Error #%d\n", GetLastError());
        return;
    }
    
    //Dynamically link to the DetectBootSector function
    BOOT_SECTOR_INFO info;        
    FuncDetectBootSector DetectBootSector;
    DetectBootSector = reinterpret_cast<FuncDetectBootSector>( GetProcAddress( hLib, "DetectBootSector" ) );

    //Detect the boot sector information
    if (DetectBootSector(bootSector, &info))
    {        
        //Determine file system type
        LPCWSTR fileSystem;
        if ( info.FileSystem == FileSystemFAT32 ||
             info.FileSystem == FileSystemFAT16 ||
             info.FileSystem == FileSystemFAT12 )
        {
            fileSystem = L"FAT";
        }
        else if (info.FileSystem == FileSystemNTFS )
        {
            fileSystem = L"NTFS";
        } 
        else
        {
            fileSystem = L"UNKNOWN";
        }
        
        //Display the boot sector information        
        wprintf(L"File System Type:     %s\n", fileSystem);
        wprintf(L"Bytes Per Sector:     %d\n", info.BytePerSector);
        wprintf(L"Sectors Per Cluster:  %d\n", info.SectorPerCluster);
        wprintf(L"Total Sectors:        %I64d\n", info.TotalSectors);
    } 
    else
    {        
        wprintf(L"Boot sector not recognized.");        
    }
}

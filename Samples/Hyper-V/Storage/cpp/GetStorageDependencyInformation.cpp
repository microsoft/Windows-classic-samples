// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <virtdisk.h>

#include "Storage.h"

//
// This sample demonstrates how query storage dependency information.
//

DWORD
SampleGetStorageDependencyInformation(
    _In_    LPCWSTR     Disk)
{
    PSTORAGE_DEPENDENCY_INFO pInfo;
    DWORD infoSize;
    DWORD cbSize;
    HANDLE driveHandle;
    GET_STORAGE_DEPENDENCY_FLAG flags;
    BOOL isDisk;
    DWORD opStatus;
    DWORD entry;
    TCHAR szVolume[8] = L"\\\\.\\C:\\";
    TCHAR szDisk[19] = L"\\\\.\\PhysicalDrive0";

    if (Disk[0] >= L'0' && Disk[0] <= L'9')
    {
        //
        // Assume the user is specifying a disk between 0 and 9
        //
        
        isDisk = TRUE;
        szDisk[17] = Disk[0];
        flags = GET_STORAGE_DEPENDENCY_FLAG_PARENTS | GET_STORAGE_DEPENDENCY_FLAG_DISK_HANDLE;
    }
    else
    {
        //
        // Assume the user is specifying a drive letter between A: and Z:
        //
        
        isDisk = FALSE;
        szVolume[4] = Disk[0];
        flags = GET_STORAGE_DEPENDENCY_FLAG_PARENTS;
    }

    driveHandle = INVALID_HANDLE_VALUE;
    pInfo = NULL;

    //
    // Allocate enough memory for most basic case.
    //
    
    infoSize = sizeof(STORAGE_DEPENDENCY_INFO);
    
    pInfo = (PSTORAGE_DEPENDENCY_INFO)malloc(infoSize);
    if (pInfo == NULL)
    {
        opStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    
    memset(pInfo, 0, infoSize);

    //
    // Open the drive
    //
    
    driveHandle = CreateFile(
        (isDisk ? szDisk : szVolume),
        GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (driveHandle == INVALID_HANDLE_VALUE)
    {
        opStatus = GetLastError();
        goto Cleanup;
    }

    //
    // Deteremine the size actually required.
    //
    
    pInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
    cbSize = 0;
    
    opStatus = GetStorageDependencyInformation(
        driveHandle,
        flags,
        infoSize,
        pInfo,
        &cbSize);

    if (opStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        //
        // Allocate a large enough buffer.
        //
        
        free(pInfo);
        
        infoSize = cbSize;

        pInfo = (PSTORAGE_DEPENDENCY_INFO)malloc(infoSize);
        if (pInfo == NULL)
        {
            opStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        memset(pInfo, 0, infoSize);
        
        //
        // Retry with large enough buffer.
        //
        
        pInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
        cbSize = 0;
        
        opStatus = GetStorageDependencyInformation(
            driveHandle,
            GET_STORAGE_DEPENDENCY_FLAG_PARENTS,
            infoSize,
            pInfo,
            &cbSize);
    }

    if (opStatus != ERROR_SUCCESS)
    {
        //
        // This is most likely due to the disk not being a mounted VHD.
        //
        
        goto Cleanup;
    }

    //
    // Display the relationship between the specified volume and the underlying disks.
    //

    for (entry = 0; entry < pInfo->NumberEntries; entry++)
    {
        wprintf(L"%u:\n", entry);
        wprintf(L"   %u\n", pInfo->Version2Entries[entry].AncestorLevel);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependencyDeviceName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].HostVolumeName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependentVolumeName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependentVolumeRelativePath);
    }
 
Cleanup:

    if (opStatus == ERROR_SUCCESS)
    {
        wprintf(L"success\n");
    }
    else
    {
        wprintf(L"error = %u\n", opStatus);
    }

    if (driveHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(driveHandle);
    }

    if (pInfo != NULL)
    {
        free(pInfo);
    }

    return opStatus;
}

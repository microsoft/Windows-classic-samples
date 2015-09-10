// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <initguid.h>
#include <virtdisk.h>

#include "Storage.h"

//
// This sample demonstrates how to mirror a VHD or VHDX to a new location. The VHD or VHDX in the
// new location will be part of the active virtual disk chain once the operation completes
// successfully. The old files are not deleted after the operation completes.  
//

DWORD
SampleMirrorVirtualDisk(
    _In_    LPCWSTR     SourcePath,
    _In_    LPCWSTR     DestinationPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    MIRROR_VIRTUAL_DISK_PARAMETERS mirrorParameters;
    VIRTUAL_DISK_PROGRESS progress;
    VIRTUAL_STORAGE_TYPE storageType;
    OVERLAPPED overlapped;
    HANDLE vhdHandle;
    DWORD opStatus;

    vhdHandle = INVALID_HANDLE_VALUE;

    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (overlapped.hEvent == NULL)
    {
        opStatus = GetLastError();
        goto Cleanup;
    }

    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    //
    // Open the source VHD/VHDX.
    //
    // Only V2 handles can be used when mirroring VHDs.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NONE bypasses any special handling of the open.
    //
    
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;

    opStatus = OpenVirtualDisk(
        &storageType,
        SourcePath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Start mirror operation.
    //
    // MIRROR_VIRTUAL_DISK_VERSION_1 is the only version of the parameters currently supported.
    // MIRROR_VIRTUAL_DISK_FLAG_NONE forces the creation of a new file.
    //
    
    memset(&mirrorParameters, 0, sizeof(MIRROR_VIRTUAL_DISK_PARAMETERS));
    mirrorParameters.Version = MIRROR_VIRTUAL_DISK_VERSION_1;
    mirrorParameters.Version1.MirrorVirtualDiskPath = DestinationPath;

    opStatus = MirrorVirtualDisk(
        vhdHandle,
        MIRROR_VIRTUAL_DISK_FLAG_NONE,
        &mirrorParameters,
        &overlapped
        );

    if ((opStatus == ERROR_SUCCESS) || (opStatus == ERROR_IO_PENDING))
    {
        //
        // The mirror is completed once the "CurrentValue" reaches the "CompletionValue".
        //
        // Every subsequent write will be forward to both the source and destination until the
        // mirror is broken.
        //
        
        for (;;)
        {
            memset(&progress, 0, sizeof(progress));
            
            opStatus = GetVirtualDiskOperationProgress(vhdHandle, &overlapped, &progress);
            if (opStatus != ERROR_SUCCESS)
            {
                goto Cleanup;
            }

            opStatus = progress.OperationStatus;
            if (opStatus == ERROR_IO_PENDING)
            {
                if (progress.CurrentValue == progress.CompletionValue)
                {
                    break;
                }
            }
            else
            {
                //
                // Any status other than ERROR_IO_PENDING indicates the mirror failed.
                //
                goto Cleanup;
            }
        
            Sleep(1000);
        }        
    }
    else
    {
        goto Cleanup;
    }

    //
    // Break the mirror.  Breaking the mirror will activate the new target and cause it to be
    // utilized in place of the original VHD/VHDX.
    //

    opStatus = BreakMirrorVirtualDisk(vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    else 
    {        
        for (;;)
        {
            memset(&progress, 0, sizeof(progress));
            
            opStatus = GetVirtualDiskOperationProgress(vhdHandle, &overlapped, &progress);
            if (opStatus != ERROR_SUCCESS)
            {
                goto Cleanup;
            }

            opStatus = progress.OperationStatus;
            if (opStatus == ERROR_SUCCESS)
            {
                break;
            }
            else if (opStatus != ERROR_IO_PENDING)
            {
                goto Cleanup;
            }

            Sleep(1000);
        }
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
    
    if (vhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhdHandle);
    }

    if (overlapped.hEvent != NULL)
    {
        CloseHandle(overlapped.hEvent);
    }

    return opStatus;
 }

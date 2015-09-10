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
// This sample demonstrates how to open a VHD or VHDX file in Raw IO mode and how to issue
// raw reads and writes to the virtual hard disk sectors.
//

//
// This sample uses a 1MB buffer for reads/writes but any size that is a multiple of the sector
// size for both VHDs is acceptable.
//

#define SAMPLE_BUFFER_SIZE  (1024*1024)


DWORD
SampleRawIO(
    _In_    LPCWSTR     SourceVirtualDiskPath,
    _In_    LPCWSTR     DestinationVirtualDiskPath)
{
    HANDLE sourceVhdHandle;
    HANDLE destinationVhdHandle;
    VIRTUAL_STORAGE_TYPE storageType;
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    ATTACH_VIRTUAL_DISK_FLAG attachFlags;
    GET_VIRTUAL_DISK_INFO diskInfo;
    DWORD opStatus;
    OVERLAPPED overlapped;
    LARGE_INTEGER virtualDiskSize;
    LARGE_INTEGER offset;
    ULONG diskInfoSize;
    DWORD bytesRead;
    DWORD bytesWritten;
    PUCHAR buffer;

    sourceVhdHandle = INVALID_HANDLE_VALUE;
    destinationVhdHandle = INVALID_HANDLE_VALUE;
    offset.QuadPart = 0;

    buffer = (PUCHAR)malloc(SAMPLE_BUFFER_SIZE);

    if (buffer == NULL)
    {
        opStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //

    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.GetInfoOnly = FALSE;

    memset(&attachParameters, 0, sizeof(attachParameters));
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

    memset(&diskInfo, 0, sizeof(diskInfo));

    memset(&overlapped, 0, sizeof(overlapped));

    //
    // Open the source VHD in RAW IO mode to read directly from the virtual disk sectors.
    //

    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    //

    opStatus = OpenVirtualDisk(
        &storageType,
        SourceVirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &sourceVhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Deteremine the source virtual disk size.
    //

    diskInfo.Version = GET_VIRTUAL_DISK_INFO_SIZE;
    diskInfoSize = sizeof(diskInfo);

    opStatus = GetVirtualDiskInformation(
        sourceVhdHandle,
        &diskInfoSize,
        &diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    virtualDiskSize.QuadPart = diskInfo.Size.VirtualSize;

    //
    // ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST is required for RawIO.
    //
    
    attachFlags = ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST |
                  ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY;

    opStatus = AttachVirtualDisk(
        sourceVhdHandle,
        NULL,
        attachFlags,
        0,
        &attachParameters,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Open the destination VHD in RAW IO mode to write directly to the virtual disk sectors.
    //

    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    //

    opStatus = OpenVirtualDisk(
        &storageType,
        DestinationVirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &destinationVhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST is required for RawIO.
    //
    
    attachFlags = ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST;
     
    opStatus = AttachVirtualDisk(
        destinationVhdHandle,
        NULL,
        attachFlags,
        0,
        &attachParameters,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    while(offset.QuadPart < virtualDiskSize.QuadPart)
    {
        //
        // Read next chunk of source file
        //
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        if (!ReadFile(
                sourceVhdHandle,
                buffer,
                SAMPLE_BUFFER_SIZE,
                &bytesRead,
                &overlapped))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                if (!GetOverlappedResult(sourceVhdHandle, &overlapped, &bytesRead, TRUE))
                {
                    opStatus = GetLastError();
                }
            }

            if (opStatus != ERROR_SUCCESS)
            {
                break;
            }
        }

        //
        // Write next chunk to destination file
        //
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        if (!WriteFile(
                destinationVhdHandle,
                buffer,
                bytesRead,
                &bytesWritten,
                &overlapped))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                if (!GetOverlappedResult(destinationVhdHandle, &overlapped, &bytesWritten, TRUE))
                {
                    opStatus = GetLastError();
                }
            }

            if (opStatus != ERROR_SUCCESS)
            {
                break;
            }

            if (bytesWritten != bytesRead)
            {
                opStatus = ERROR_HANDLE_EOF;
                break;
            }
        }

        offset.QuadPart += bytesWritten;
    }

Cleanup:

    if (opStatus == ERROR_SUCCESS)
    {
        wprintf(L"success, bytes transferred = %I64d\n", offset.QuadPart);
    }
    else
    {
        wprintf(L"error = %u\n", opStatus);
    }
    
    if (sourceVhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(sourceVhdHandle);
    }
    
    if (destinationVhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(destinationVhdHandle);
    }

    if (buffer != NULL)
    {
        free(buffer);
    }
    
    return opStatus;
}

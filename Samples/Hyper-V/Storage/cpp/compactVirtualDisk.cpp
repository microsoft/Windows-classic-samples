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
// This sample demonstrates how to use the CompactVirtualDisk API to compact a VHD.
//
// To obtain the full benefit of compaction, the VHD should be mounted prior to and throughout
// the compaction.
//
// Mounting a VHD is demonstrated in the AttachVirtualDisk sample.
//
// The VHD can only be mounted read-only during a compaction.
//


DWORD
SampleCompactVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    COMPACT_VIRTUAL_DISK_PARAMETERS compactParmaters;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE vhdHandle;
    DWORD opStatus;

    vhdHandle = INVALID_HANDLE_VALUE;
    
    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    //
    // Open the VHD.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NONE bypasses any special handling of the open.
    //

    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    
    opStatus = OpenVirtualDisk(
        &storageType,
        VirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Perform the compaction.
    //
    // COMPACT_VIRTUAL_DISK_VERSION_1 is on the version currently supported.
    // COMPACT_VIRTUAL_DISK_FLAG_NONE specifies a full compaction, which only really full if
    // the VHD/VHDX has previously been mounted.
    //

    memset(&compactParmaters, 0, sizeof(compactParmaters));
    compactParmaters.Version = COMPACT_VIRTUAL_DISK_VERSION_1;

    opStatus = CompactVirtualDisk(
        vhdHandle,
        COMPACT_VIRTUAL_DISK_FLAG_NONE,
        &compactParmaters,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
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

    return opStatus;
}

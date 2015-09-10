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
#include <Shlwapi.h>
#include <virtdisk.h>

#include "Storage.h"

//
// This sample demonstrates how to unmount a VHD or ISO file.
//

DWORD
SampleDetachVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath)
{
    VIRTUAL_STORAGE_TYPE storageType;
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_DISK_ACCESS_MASK accessMask;
    LPCTSTR extension;
    HANDLE vhdHandle;
    DWORD opStatus;

    vhdHandle = INVALID_HANDLE_VALUE;

    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    memset(&openParameters, 0, sizeof(openParameters));
    
    extension = ::PathFindExtension(VirtualDiskPath);
    
    if (extension != NULL && _wcsicmp(extension, L".iso") == 0)
    {
        //
        // ISO files can only be opened using the V1 API.
        //
        
        openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
        accessMask = VIRTUAL_DISK_ACCESS_READ;
    }
    else
    {
        //
        // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
        // OPEN_VIRTUAL_DISK_FLAG_NONE bypasses any special handling of the open.
        //
        
        openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
        openParameters.Version2.GetInfoOnly = FALSE;
        accessMask = VIRTUAL_DISK_ACCESS_NONE;
    }
    
    //
    // Open the VHD/VHDX or ISO.
    //
    //

    opStatus = OpenVirtualDisk(
        &storageType,
        VirtualDiskPath,
        accessMask,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Detach the VHD/VHDX/ISO.
    //
    // DETACH_VIRTUAL_DISK_FLAG_NONE is the only flag currently supported for detach.
    //

    opStatus = DetachVirtualDisk(
        vhdHandle,
        DETACH_VIRTUAL_DISK_FLAG_NONE,
        0);

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

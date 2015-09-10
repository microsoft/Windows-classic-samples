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
// This sample demonstrates how to resize (shrink or expand) a VHD/VHDX.
//
// The VHD/VHDX can not be in use while performing this operation.
// 

DWORD
SampleResizeVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    ULONGLONG   FileSize)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    RESIZE_VIRTUAL_DISK_PARAMETERS resizeParameters;
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
    // Open the VHD/VHDX.
    //
    // Only V2 handles can be used to resize a VHD/VHDX.
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
    // Perform the resize (shrink or expand).
    //
    // RESIZE_VIRTUAL_DISK_VERSION_1 is the only version of the parameters currently supported.
    // RESIZE_VIRTUAL_DISK_FLAG_NONE prevents unsafe shrink where the new virtual disk size cannot
    // be less than the "SmallestSafeVirtualSize" reported through the GetVirtualDiskInformation API
    // with a "VirtualDiskInfo" of GET_VIRTUAL_DISK_INFO_SMALLEST_SAFE_VIRTUAL_SIZE.
    //

    memset(&resizeParameters, 0, sizeof(resizeParameters));
    resizeParameters.Version = RESIZE_VIRTUAL_DISK_VERSION_1;
    resizeParameters.Version1.NewSize = FileSize;

    opStatus = ResizeVirtualDisk(
        vhdHandle,
        RESIZE_VIRTUAL_DISK_FLAG_NONE,
        &resizeParameters,
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

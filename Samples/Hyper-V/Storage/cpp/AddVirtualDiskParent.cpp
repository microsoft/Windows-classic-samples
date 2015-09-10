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
// This sample demonstrates how to use the AddVirtualDiskParent API to build a diff chain by 
// explicitly setting the parent after opening the child.
//

DWORD
SampleAddVirtualDiskParent(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    LPCWSTR     ParentPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;

    HANDLE vhdHandle = INVALID_HANDLE_VALUE;
    DWORD opStatus;

    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;
    
    //
    // Open the VHD.
    //
    // The AddVirtualDiskParent requires a V2 handle open. 
    //
    // The child must be opened read-only prior to calling AddVirtualDiskParent.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_CUSTOM_DIFF_CHAIN must be specified when calling AddVirtualDiskParent.
    //

    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.ReadOnly = TRUE;

    opStatus = OpenVirtualDisk(
        &storageType,
        VirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_CUSTOM_DIFF_CHAIN,
        &openParameters,
        &vhdHandle);
    
    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    opStatus = AddVirtualDiskParent(vhdHandle, ParentPath);
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

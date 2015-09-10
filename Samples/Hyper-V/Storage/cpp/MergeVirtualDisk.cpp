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
// This sample demonstrates how to merge a differencing disk into its parent.
//

DWORD
SampleMergeVirtualDisk(
    _In_    LPCWSTR     LeafPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    MERGE_VIRTUAL_DISK_PARAMETERS mergeParameters;
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
        LeafPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NONE,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Perform the merge.
    //
    // MERGE_VIRTUAL_DISK_VERSION_2 allows merging of VHDs/VHDXs in use.
    // MERGE_VIRTUAL_DISK_FLAG_NONE is currently the only merge flag supported.
    //
    // DO NOT attempt to perform a live merge of a leaf (a)VHD or (a)VHDX of a VM as the
    // operation will not update the virtual machine configuration file.
    //

    memset(&mergeParameters, 0, sizeof(mergeParameters));
    mergeParameters.Version = MERGE_VIRTUAL_DISK_VERSION_2;

    // In this sample, the leaf is being merged so the source depth is 1.
    mergeParameters.Version2.MergeSourceDepth = 1;

    // In this sample, the leaf is being merged only to it's parent so the target depth is 2
    mergeParameters.Version2.MergeTargetDepth = 2;

    opStatus = MergeVirtualDisk(
        vhdHandle,
        MERGE_VIRTUAL_DISK_FLAG_NONE,
        &mergeParameters,
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

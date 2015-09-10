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
// This sample demonstrates how to set a metadata item from a VHDX file.
//
// User metadata is not applicable to VHD files.
//

//
// This sample metadata structure is for demonstration purposes only.  Any structure can be used
// for metadata.
//

typedef struct _SAMPLE_METADATA
{
    DWORD ID;
} SAMPLE_METADATA;

DWORD
SampleSetUserMetaData(
    _In_ LPCWSTR VHDPath,
    _In_ DWORD ID)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE vhdHandle;
    SAMPLE_METADATA userMeta;
    GUID uniqueId;
    DWORD status;

    vhdHandle = NULL;
    status = ERROR_SUCCESS;

    if (VHDPath == NULL)
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    
    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    //
    // Only V2 handles can be used to query/set/delete user metadata.
    //
    
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;

    //
    // Open the VHD/VHDX.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS indicates the parent chain should not be opened.
    //
    
    status = OpenVirtualDisk(
        &storageType,
        VHDPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &vhdHandle);
    
    if (status != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // The specified ID is used as the contents of the metadata but any structure could be
    // used.
    //
    
    userMeta.ID=ID;
    
    //
    // This GUID is arbitrary and any GUID can be utilized.
    //

    uniqueId.Data1 = 0x34a631f3;
    uniqueId.Data2 = 0xa39d;
    uniqueId.Data3 = 0x4e45;
    uniqueId.Data4[0] = 0xbb;
    uniqueId.Data4[1] = 0x2e;
    uniqueId.Data4[2] = 0x98;
    uniqueId.Data4[3] = 0xcf;
    uniqueId.Data4[4] = 0x2d;
    uniqueId.Data4[5] = 0xfe;
    uniqueId.Data4[6] = 0x4f;
    uniqueId.Data4[7] = 0x3d;

    status = SetVirtualDiskMetadata(
        vhdHandle,
        &uniqueId,
        sizeof(userMeta),
        (PVOID)&userMeta);

    if (status != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

Cleanup:

    if (status == ERROR_SUCCESS)
    {
        wprintf(L"success\n");
    }
    else
    {
        wprintf(L"error = %u\n", status);
    }
    
    if (vhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhdHandle);
    }

    return status;
 }

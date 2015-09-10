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
// This sample demonstrates how to get a metadata item from a VHDX file.
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
SampleGetUserMetaData(
    _In_ LPCWSTR VHDPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE vhdHandle;
    GUID uniqueId;
    SAMPLE_METADATA userMeta;
    ULONG metaDataSize;
    DWORD status;

    vhdHandle = NULL;
    metaDataSize = 0;
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
    // A "GetInfoOnly" handle is a handle that can only be used to query properties or
    // metadata.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS indicates the parent chain should not be opened.
    //
    
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.GetInfoOnly = TRUE;
    
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
    
    metaDataSize = sizeof(userMeta);

    //
    // Use the same GUID specified in SampleSetUserMetaData.  This GUID is arbitrary and any
    // GUID can be utilized.
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
    
    status = GetVirtualDiskMetadata(
        vhdHandle,
        &uniqueId,
        &metaDataSize,
        (PVOID)&userMeta);
    
    if (status != ERROR_SUCCESS)
    {
        if (status == ERROR_MORE_DATA)
        {
            wprintf(L"Get: more data available\n");
        }
        else
        {
            goto Cleanup;
        }
    }
    else
    {
        wprintf(L"Get metadata: %d\n", userMeta.ID);
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

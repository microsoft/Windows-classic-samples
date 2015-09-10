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
// This sample shows how to enumberate the available metadata items of a VHDX file. 
//
// User metadata is not applicable to VHD files.
//

DWORD
SampleEnumerateUserMetaData(
    _In_ LPCWSTR VHDPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE vhdHandle;
    ULONG numberOfItems;
    GUID *items;
    DWORD status;

    vhdHandle = NULL;
    numberOfItems = 0;
    items = NULL;
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

    numberOfItems = 0;    

    //
    // EnumerateVirtualDiskMetadata returns the number of user metadata items in the VHDX.
    //
    // NULL can be specified for the third parameter when trying to determine the number
    // of items to expect.
    //
    
    status = EnumerateVirtualDiskMetadata(vhdHandle, &numberOfItems, NULL);
    if (status != ERROR_SUCCESS)
    {
        if (status == ERROR_MORE_DATA)
        {
            wprintf(L"Enumerate: more data available\n");
        }
        else
        {
            goto Cleanup;
        }
    }
    
    wprintf(L"%d user defined metadata items are available\n", numberOfItems);

    //
    // Each user metadata item is specified by a unique GUID.
    //
    
    items = (GUID *) malloc(numberOfItems * sizeof(GUID));
    if (items == NULL)
    {
        status = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
        
    status = EnumerateVirtualDiskMetadata(vhdHandle, &numberOfItems, items);
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

    if (items != NULL)
    {
        free(items);
    }
    
    if (vhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhdHandle);
    }

    return status;
 }

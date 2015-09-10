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
// This sample demonstrates how to set properties of a VHD/VHDX.
//

DWORD
SampleSetVirtualDiskInformation(
    _In_    LPCWSTR     ChildPath,
    _In_    LPCWSTR     ParentPath,
    _In_    DWORD       PhysicalSectorSize)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    GET_VIRTUAL_DISK_INFO childDiskInfo;
    GET_VIRTUAL_DISK_INFO parentDiskInfo;
    SET_VIRTUAL_DISK_INFO setInfo;
    DWORD diskInfoSize;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE childVhdHandle;
    HANDLE parentVhdHandle;
    DWORD opStatus;

    childVhdHandle = INVALID_HANDLE_VALUE;
    parentVhdHandle = INVALID_HANDLE_VALUE;

    //
    // Specify UNKNOWN for both device and vendor so the system will use the
    // file extension to determine the correct VHD format.
    //
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    //
    // Open the parent so it's properties can be queried.
    //
    // A "GetInfoOnly" handle is a handle that can only be used to query properties or
    // metadata.
    //

    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.GetInfoOnly = TRUE;

    //
    // Open the VHD/VHDX.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS indicates the parent chain should not be opened.
    //
    
    opStatus = OpenVirtualDisk(
        &storageType,
        ParentPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &parentVhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Get the disk ID of the parent.
    //
    
    diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);
    parentDiskInfo.Version = GET_VIRTUAL_DISK_INFO_IDENTIFIER;

    opStatus = GetVirtualDiskInformation(
        parentVhdHandle,
        &diskInfoSize,
        &parentDiskInfo,
        NULL);
    
    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Open the VHD/VHDX so it's properties can be queried.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS indicates the parent chain should not be opened.
    //
    
    opStatus = OpenVirtualDisk(
        &storageType,
        ChildPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &childVhdHandle);
    
    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    
    //
    // Get the disk ID expected for the parent.
    //
    
    childDiskInfo.Version = GET_VIRTUAL_DISK_INFO_PARENT_IDENTIFIER;
    diskInfoSize = sizeof(childDiskInfo);
    
    opStatus = GetVirtualDiskInformation(
        childVhdHandle,
        &diskInfoSize,
        &childDiskInfo,
        NULL);
    
    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Verify the disk IDs match.
    //

    if (memcmp(&parentDiskInfo.Identifier,
               &childDiskInfo.ParentIdentifier,
               sizeof(GUID)))
    {
        opStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Reset the parent locators in the child.
    //

    CloseHandle(childVhdHandle);
    childVhdHandle = INVALID_HANDLE_VALUE;

    //
    // This cannot be a "GetInfoOnly" handle because the intent is to alter the properties of the 
    // VHD/VHDX.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    // OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS indicates the parent chain should not be opened.
    //

    openParameters.Version2.GetInfoOnly = FALSE;

    opStatus = OpenVirtualDisk(
        &storageType,
        ChildPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &childVhdHandle);
    
    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Update the path to the parent.
    //

    setInfo.Version = SET_VIRTUAL_DISK_INFO_PARENT_PATH_WITH_DEPTH;
    setInfo.ParentPathWithDepthInfo.ChildDepth = 1;
    setInfo.ParentPathWithDepthInfo.ParentFilePath = ParentPath;

    opStatus = SetVirtualDiskInformation(childVhdHandle, &setInfo);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Set the physical sector size.
    //
    // This operation will only succeed on VHDX.
    //

    setInfo.Version = SET_VIRTUAL_DISK_INFO_PHYSICAL_SECTOR_SIZE;
    setInfo.VhdPhysicalSectorSize = PhysicalSectorSize;

    opStatus = SetVirtualDiskInformation(childVhdHandle, &setInfo);

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
    
    if (childVhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(childVhdHandle);
    }

    if (parentVhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(parentVhdHandle);
    }

    return opStatus;
 }

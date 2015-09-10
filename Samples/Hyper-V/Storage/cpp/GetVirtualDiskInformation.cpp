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
#include <strsafe.h>
#include <virtdisk.h>

#include "Storage.h"

DWORD
SampleGetVirtualDiskInformation(
    _In_    LPCWSTR     VirtualDiskPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;
    PGET_VIRTUAL_DISK_INFO diskInfo;
    ULONG diskInfoSize;
    DWORD opStatus;

    HANDLE vhdHandle;
    
    UINT32 driveType;
    UINT32 driveFormat;

    GUID identifier;
    
    ULONGLONG physicalSize;
    ULONGLONG virtualSize;
    ULONGLONG minInternalSize;
    ULONG blockSize;
    ULONG sectorSize;
    ULONG physicalSectorSize;
    LPCWSTR parentPath;
    size_t parentPathSize;
    size_t parentPathSizeRemaining;
    HRESULT stringLengthResult;
    GUID parentIdentifier;
    ULONGLONG fragmentationPercentage;

    vhdHandle = INVALID_HANDLE_VALUE;
    diskInfo = NULL;
    diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);

    diskInfo = (PGET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
    if (diskInfo == NULL)
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

    //
    // Open the VHD for query access.
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

    opStatus = OpenVirtualDisk(
        &storageType,
        VirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    //
    // Get the VHD/VHDX type.
    //

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    driveType = diskInfo->ProviderSubtype;

    wprintf(L"driveType = %d", driveType);

    if (driveType == 2)
    {
        wprintf(L" (fixed)\n");
    }
    else if (driveType == 3)
    {
        wprintf(L" (dynamic)\n");
    }
    else if (driveType == 4)
    {
        wprintf(L" (differencing)\n");
    }
    else
    {
        wprintf(L"\n");
    }
    
    //
    // Get the VHD/VHDX format.
    //
    
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_VIRTUAL_STORAGE_TYPE;
    
    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    driveFormat = diskInfo->VirtualStorageType.DeviceId;

    wprintf(L"driveFormat = %d", driveFormat);

    if (driveFormat == VIRTUAL_STORAGE_TYPE_DEVICE_VHD)
    {
        wprintf(L" (vhd)\n");
    }
    else if (driveFormat == VIRTUAL_STORAGE_TYPE_DEVICE_VHDX)
    {
        wprintf(L" (vhdx)\n");
    }
    else
    {
        wprintf(L"\n");
    }

    //
    // Get the VHD/VHDX virtual disk size.
    //

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_SIZE;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    physicalSize = diskInfo->Size.PhysicalSize;
    virtualSize = diskInfo->Size.VirtualSize;
    sectorSize = diskInfo->Size.SectorSize;
    blockSize = diskInfo->Size.BlockSize;

    wprintf(L"physicalSize = %I64u\n", physicalSize);
    wprintf(L"virtualSize = %I64u\n", virtualSize);
    wprintf(L"sectorSize = %u\n", sectorSize);
    wprintf(L"blockSize = %u\n", blockSize);

    //
    // Get the VHD physical sector size.
    //

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_VHD_PHYSICAL_SECTOR_SIZE;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    physicalSectorSize = diskInfo->VhdPhysicalSectorSize;

    wprintf(L"physicalSectorSize = %u\n", physicalSectorSize);

    //
    // Get the virtual disk ID.
    //
    
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_IDENTIFIER;
    
    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    
    identifier = diskInfo->Identifier;

    wprintf(L"identifier = {%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}\n",
        identifier.Data1, identifier.Data2, identifier.Data3,
        identifier.Data4[0], identifier.Data4[1], identifier.Data4[2], identifier.Data4[3],
        identifier.Data4[4], identifier.Data4[5], identifier.Data4[6], identifier.Data4[7]);

    //
    // Get the VHD parent path.
    //

    if (driveType == 0x4)
    {
        diskInfo->Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;

        opStatus = GetVirtualDiskInformation(
            vhdHandle,
            &diskInfoSize,
            diskInfo,
            NULL);

        if (opStatus != ERROR_SUCCESS)
        {
            if (opStatus != ERROR_INSUFFICIENT_BUFFER)
            {
                goto Cleanup;
            }

            free(diskInfo);

            diskInfo = (PGET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
            if (diskInfo == NULL)
            {
                opStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            diskInfo->Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;

            opStatus = GetVirtualDiskInformation(
                vhdHandle,
                &diskInfoSize,
                diskInfo,
                NULL);

            if (opStatus != ERROR_SUCCESS)
            {
                goto Cleanup;
            }
        }

        parentPath = diskInfo->ParentLocation.ParentLocationBuffer;
        parentPathSizeRemaining = diskInfoSize - FIELD_OFFSET(GET_VIRTUAL_DISK_INFO, 
                                                              ParentLocation.ParentLocationBuffer);
        
        if (diskInfo->ParentLocation.ParentResolved)
        {
            wprintf(L"parentPath = '%s'\n", parentPath);
        }
        else
        {
            //
            // If the parent is not resolved, the buffer is a MULTI_SZ
            //
            
            wprintf(L"parentPath:\n");

            while((parentPathSizeRemaining >= sizeof(parentPath[0])) && (*parentPath != 0))
            {
                stringLengthResult = StringCbLengthW(
                    parentPath, 
                    parentPathSizeRemaining,
                    &parentPathSize);

                if (FAILED(stringLengthResult))
                {
                    goto Cleanup;
                }

                wprintf(L"    '%s'\n", parentPath);

                parentPathSize += sizeof(parentPath[0]);
                parentPath = parentPath + (parentPathSize / sizeof(parentPath[0]));
                parentPathSizeRemaining -= parentPathSize;
            }
        }

        //
        // Get parent ID.
        //
        
        diskInfo->Version = GET_VIRTUAL_DISK_INFO_PARENT_IDENTIFIER;
        
        opStatus = GetVirtualDiskInformation(
            vhdHandle,
            &diskInfoSize,
            diskInfo,
            NULL);

        if (opStatus != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
        
        parentIdentifier = diskInfo->ParentIdentifier;
        
        wprintf(L"parentIdentifier = {%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}\n",
            parentIdentifier.Data1, parentIdentifier.Data2, parentIdentifier.Data3,
            parentIdentifier.Data4[0], parentIdentifier.Data4[1], 
            parentIdentifier.Data4[2], parentIdentifier.Data4[3],
            parentIdentifier.Data4[4], parentIdentifier.Data4[5], 
            parentIdentifier.Data4[6], parentIdentifier.Data4[7]);
    }
    
    //
    // Get the VHD minimum internal size.
    //

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_SMALLEST_SAFE_VIRTUAL_SIZE;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);
    
    if (opStatus == ERROR_SUCCESS)
    {
        minInternalSize = diskInfo->SmallestSafeVirtualSize;

        wprintf(L"minInternalSize = %I64u\n", minInternalSize);
    }
    else
    {
        opStatus = ERROR_SUCCESS;
    }

    //
    // Get the VHD fragmentation percentage.
    //

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_FRAGMENTATION;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);
    
    if (opStatus == ERROR_SUCCESS)
    {
        fragmentationPercentage = diskInfo->FragmentationPercentage;

        wprintf(L"fragmentationPercentage = %I64u\n", fragmentationPercentage);
    }
    else
    {
        opStatus = ERROR_SUCCESS;
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

    if (diskInfo != NULL)
    {
        free(diskInfo);
    }

    return opStatus;
}

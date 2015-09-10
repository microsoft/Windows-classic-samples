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
#include <rpc.h>

#include "Storage.h"

//
// This sample demonstrates how to create VHDs and VHDXs.
//

DWORD
SampleCreateVirtualDisk(
    _In_        LPCWSTR                     VirtualDiskPath,
    _In_opt_    LPCWSTR                     ParentPath,
    _In_        CREATE_VIRTUAL_DISK_FLAG    Flags,
    _In_        ULONGLONG                   FileSize,
    _In_        DWORD                       BlockSize,
    _In_        DWORD                       LogicalSectorSize,
    _In_        DWORD                       PhysicalSectorSize)
{
    VIRTUAL_STORAGE_TYPE storageType;
    CREATE_VIRTUAL_DISK_PARAMETERS parameters;
    HANDLE vhdHandle = INVALID_HANDLE_VALUE;
    DWORD opStatus;
    GUID uniqueId;

    if (RPC_S_OK != UuidCreate((UUID*)&uniqueId))
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

    memset(&parameters, 0, sizeof(parameters));

    //
    // CREATE_VIRTUAL_DISK_VERSION_2 allows specifying a richer set a values and returns
    // a V2 handle.
    //
    // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
    //
    // Valid BlockSize values are as follows (use 0 to indicate default value):
    //      Fixed VHD: 0
    //      Dynamic VHD: 512kb, 2mb (default)
    //      Differencing VHD: 512kb, 2mb (if parent is fixed, default is 2mb; if parent is dynamic or differencing, default is parent blocksize)
    //      Fixed VHDX: 0
    //      Dynamic VHDX: 1mb, 2mb, 4mb, 8mb, 16mb, 32mb (default), 64mb, 128mb, 256mb
    //      Differencing VHDX: 1mb, 2mb (default), 4mb, 8mb, 16mb, 32mb, 64mb, 128mb, 256mb
    //
    // Valid LogicalSectorSize values are as follows (use 0 to indicate default value):
    //      VHD: 512 (default)
    //      VHDX: 512 (for fixed or dynamic, default is 512; for differencing, default is parent logicalsectorsize), 4096
    //
    // Valid PhysicalSectorSize values are as follows (use 0 to indicate default value):
    //      VHD: 512 (default)
    //      VHDX: 512, 4096 (for fixed or dynamic, default is 4096; for differencing, default is parent physicalsectorsize)
    //
    parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;
    parameters.Version2.UniqueId = uniqueId;
    parameters.Version2.MaximumSize = FileSize;
    parameters.Version2.BlockSizeInBytes = BlockSize;
    parameters.Version2.SectorSizeInBytes = LogicalSectorSize;
    parameters.Version2.PhysicalSectorSizeInBytes = PhysicalSectorSize;    
    parameters.Version2.ParentPath = ParentPath;
            
    opStatus = CreateVirtualDisk(
        &storageType,
        VirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        NULL,
        Flags,
        0,
        &parameters,
        NULL,
        &vhdHandle);

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

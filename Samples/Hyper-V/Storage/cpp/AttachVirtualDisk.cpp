// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <Shlwapi.h>
#include <initguid.h>
#include <virtdisk.h>
#include <sddl.h>

#include "Storage.h"

//
// This sample demonstrates how to mount a VHD or ISO file.
//

DWORD
SampleAttachVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    BOOLEAN     ReadOnly)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_DISK_ACCESS_MASK accessMask;
    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    PSECURITY_DESCRIPTOR sd;
    VIRTUAL_STORAGE_TYPE storageType;
    LPCTSTR extension;
    HANDLE vhdHandle;
    ATTACH_VIRTUAL_DISK_FLAG attachFlags;
    DWORD opStatus;

    vhdHandle = INVALID_HANDLE_VALUE;
    sd = NULL;

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
        // ISO files can only be mounted read-only and using the V1 API.
        //
        
        if (ReadOnly != TRUE)
        {
            opStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }
        
        openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
        accessMask = VIRTUAL_DISK_ACCESS_READ;
    }
    else
    {
        //
        // VIRTUAL_DISK_ACCESS_NONE is the only acceptable access mask for V2 handle opens.
        //
        
        openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
        openParameters.Version2.GetInfoOnly = FALSE;
        accessMask = VIRTUAL_DISK_ACCESS_NONE;
    }
    
    //
    // Open the VHD or ISO.
    //
    // OPEN_VIRTUAL_DISK_FLAG_NONE bypasses any special handling of the open.
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
    // Create the world-RW SD.
    //

    if (!::ConvertStringSecurityDescriptorToSecurityDescriptor(
            L"O:BAG:BAD:(A;;GA;;;WD)",
            SDDL_REVISION_1,
            &sd,
            NULL))
    {
        opStatus = ::GetLastError();
        goto Cleanup;
    }

    //
    // Attach the VHD/VHDX or ISO.
    //

    memset(&attachParameters, 0, sizeof(attachParameters));
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

    //
    // A "Permanent" surface persists even when the handle is closed.
    //
    
    attachFlags = ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME;
    
    if (ReadOnly)
    {
        // ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY specifies a read-only mount.
        attachFlags |= ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY;
    }

    opStatus = AttachVirtualDisk(
        vhdHandle,
        sd,
        attachFlags,
        0,
        &attachParameters,
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

    if (sd != NULL)
    {
        LocalFree(sd);
        sd = NULL;
    }

    if (vhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhdHandle);
    }

    return opStatus;
}

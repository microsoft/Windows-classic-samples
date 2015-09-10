// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <virtdisk.h>

#include "Storage.h"

void
ShowUsage(
    _In_    PWCHAR  ModuleName);

int __cdecl wmain(_In_ int argc, _In_reads_(argc) WCHAR *argv[])
{
    DWORD rc=1;

    if (argc == 2)
    {
        if (_wcsicmp(argv[1], L"GetAllAttachedVirtualDiskPhysicalPaths") == 0)
        {
            rc = SampleGetAllAttachedVirtualDiskPhysicalPaths();
        }
        else
        {
            ShowUsage(argv[0]);
        }
    }
    else if (argc == 3)
    {
        if (_wcsicmp(argv[1], L"GetVirtualDiskInformation") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];

            rc = SampleGetVirtualDiskInformation(virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"DetachVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];

            rc = SampleDetachVirtualDisk(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"MergeVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];

            rc = SampleMergeVirtualDisk(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"CompactVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];

            rc = SampleCompactVirtualDisk(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"EnumerateUserMetaData") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];

            rc = SampleEnumerateUserMetaData(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"GetUserMetaData") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            
            rc = SampleGetUserMetaData(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"DeleteUserMetaData") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            
            rc = SampleDeleteUserMetaData(
                    virtualDiskPath);
        }
        else if (_wcsicmp(argv[1], L"GetStorageDependencyInformation") == 0)
        {
            LPCWSTR Disk = argv[2];
            
            rc = SampleGetStorageDependencyInformation(
                    Disk);
        }
        else
        {
            ShowUsage(argv[0]);
        }
    }
    else if (argc == 4)
    {
        if (_wcsicmp(argv[1], L"AttachVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR readOnly = argv[3];

            rc = SampleAttachVirtualDisk(
                    virtualDiskPath,
                    (readOnly[0] == 't' || readOnly[0] == 'T'));
        }
        else if (_wcsicmp(argv[1], L"MirrorVirtualDisk") == 0)
        {
            LPCWSTR sourcePath = argv[2];
            LPCWSTR destinationPath = argv[3];

            rc = SampleMirrorVirtualDisk(
                    sourcePath,
                    destinationPath);
        }
        else if (_wcsicmp(argv[1], L"CreateDifferencingVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR parentPath = argv[3];

            rc = SampleCreateVirtualDisk(
                    virtualDiskPath,
                    parentPath,
                    CREATE_VIRTUAL_DISK_FLAG_NONE,
                    0,
                    0,
                    0,
                    0);
        }
        else if (_wcsicmp(argv[1], L"RawIO") == 0)
        {
            LPCWSTR sourcePath = argv[2];
            LPCWSTR destinationPath = argv[3];

            rc = SampleRawIO(
                    sourcePath,
                    destinationPath);
        }
        else if (_wcsicmp(argv[1], L"ResizeVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            ULONGLONG fileSize = _wtoi64(argv[3]);


            rc = SampleResizeVirtualDisk(
                    virtualDiskPath,
                    fileSize);
        }
        else if (_wcsicmp(argv[1], L"SetUserMetaData") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            DWORD ID = _wtoi(argv[3]);

            rc = SampleSetUserMetaData(
                    virtualDiskPath, 
                    ID);
        }
        else if (_wcsicmp(argv[1], L"AddVirtualDiskParent") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR parentPath = argv[3];

            rc = SampleAddVirtualDiskParent(
                    virtualDiskPath,
                    parentPath);
        }
        else
        {
            ShowUsage(argv[0]);
        }
    }
    else if (argc == 5)
    {
        if (_wcsicmp(argv[1], L"SetVirtualDiskInformation") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR parentPath = argv[3];
            DWORD physicalSectorSize = _wtoi(argv[4]);

            rc = SampleSetVirtualDiskInformation(
                    virtualDiskPath,
                    parentPath,
                    physicalSectorSize);
        }
        else
        {
            ShowUsage(argv[0]);
        }
    }
    else if (argc == 7)
    {
        if (_wcsicmp(argv[1], L"CreateFixedVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR parentPath = NULL;

            ULONGLONG fileSize = _wtoi64(argv[3]);
            DWORD blockSize = _wtoi(argv[4]);
            DWORD logicalSectorSize = _wtoi(argv[5]);
            DWORD physicalSectorSize = _wtoi(argv[6]);

            rc = SampleCreateVirtualDisk(
                    virtualDiskPath,
                    parentPath,
                    CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
                    fileSize,
                    blockSize,
                    logicalSectorSize,
                    physicalSectorSize);
        }
        else if (_wcsicmp(argv[1], L"CreateDynamicVirtualDisk") == 0)
        {
            LPCWSTR virtualDiskPath = argv[2];
            LPCWSTR parentPath = NULL;
             
            ULONGLONG fileSize = _wtoi64(argv[3]);
            DWORD blockSize = _wtoi(argv[4]);
            DWORD logicalSectorSize = _wtoi(argv[5]);
            DWORD physicalSectorSize = _wtoi(argv[6]);

            rc = SampleCreateVirtualDisk(
                    virtualDiskPath,
                    parentPath,
                    CREATE_VIRTUAL_DISK_FLAG_NONE,
                    fileSize,
                    blockSize,
                    logicalSectorSize,
                    physicalSectorSize);
        }
        else
        {
            ShowUsage(argv[0]);
        }
    }
    else
    {
        ShowUsage(L"");
    }

    return rc;
}

void
ShowUsage(
    _In_    PWCHAR  ModuleName)
{
    wprintf(L"\nUsage:\t%s <SampleName> <Arguments>\n", ModuleName);

    wprintf(L"Supported SampleNames and Arguments:\n");
    wprintf(L"   GetVirtualDiskInformation <path>\n");
    wprintf(L"   CreateFixedVirtualDisk <path> <file size> <block size> <logical sector size> <physical sector size>\n");
    wprintf(L"   CreateDynamicVirtualDisk <path> <file size> <block size> <logical sector size> <physical sector size>\n");
    wprintf(L"   CreateDifferencingVirtualDisk <path> <parent path>\n");
    wprintf(L"   AttachVirtualDisk <path> <readonly>\n");
    wprintf(L"   DetachVirtualDisk <path>\n");
    wprintf(L"   SetVirtualDiskInformation <child> <parent> <physical sector size>\n");
    wprintf(L"   MergeVirtualDisk <path>\n");
    wprintf(L"   CompactVirtualDisk <path>\n");
    wprintf(L"   ResizeVirtualDisk <path> 2147483648\n");
    wprintf(L"   MirrorVirtualDisk <source> <destination>\n");
    wprintf(L"   RawIO <source> <destination>\n");
    wprintf(L"   EnumerateUserMetaData <path>\n");
    wprintf(L"   SetUserMetaData <path> <metadata int>\n");
    wprintf(L"   GetUserMetaData <path>\n");
    wprintf(L"   DeleteUserMetaData <path>\n");
    wprintf(L"   AddVirtualDiskParent <path> <parent path>\n");
    wprintf(L"   GetStorageDependencyInformation [<volume> | <disk>]\n");
    wprintf(L"   GetAllAttachedVirtualDiskPhysicalPaths\n");
    

    wprintf(L"\nExamples:\n");
    wprintf(L"   %s GetVirtualDiskInformation c:\\fixed.vhd\n", ModuleName);
    wprintf(L"   %s CreateFixedVirtualDisk c:\\fixed.vhd 1073741824 0 0 0\n", ModuleName);
    wprintf(L"   %s CreateDynamicVirtualDisk c:\\dynamic.vhdx 1073741824 0 0 0\n", ModuleName);
    wprintf(L"   %s CreateDifferencingVirtualDisk c:\\diff.vhdx c:\\dynamic.vhdx\n", ModuleName);
    wprintf(L"   %s AttachVirtualDisk c:\\fixed.vhd true\n", ModuleName);
    wprintf(L"   %s DetachVirtualDisk c:\\fixed.vhd\n", ModuleName);
    wprintf(L"   %s SetVirtualDiskInformation c:\\diff.vhd c:\\fixed.vhd 4096\n", ModuleName);
    wprintf(L"   %s MergeVirtualDisk c:\\diff.vhd\n", ModuleName);
    wprintf(L"   %s CompactVirtualDisk c:\\dynamic.vhd\n", ModuleName);
    wprintf(L"   %s ResizeVirtualDisk c:\\dynamic.vhd 2147483648\n", ModuleName);
    wprintf(L"   %s MirrorVirtualDisk c:\\fixed.vhd c:\\fixed2.vhd\n", ModuleName);
    wprintf(L"   %s RawIO c:\\source.vhdx c:\\destination.vhdx\n", ModuleName);
    wprintf(L"   %s EnumerateUserMetaData c:\\fixed.vhdx\n", ModuleName);
    wprintf(L"   %s SetUserMetaData c:\\fixed.vhdx 1234\n", ModuleName);
    wprintf(L"   %s GetUserMetaData c:\\fixed.vhdx\n", ModuleName);
    wprintf(L"   %s DeleteUserMetaData c:\\fixed.vhdx\n", ModuleName);
    wprintf(L"   %s AddVirtualDiskParent c:\\diff.vhdx c:\\dynamic.vhdx\n", ModuleName); 
    wprintf(L"   %s GetStorageDependencyInformation C:\n", ModuleName); 
    wprintf(L"   %s GetAllAttachedVirtualDiskPhysicalPaths\n", ModuleName);
        
    wprintf(L"\n\n");
}


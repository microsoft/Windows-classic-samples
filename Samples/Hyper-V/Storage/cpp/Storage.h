// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

DWORD
SampleAddVirtualDiskParent(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    LPCWSTR     ParentPath);

DWORD
SampleGetVirtualDiskInformation(
    _In_    LPCWSTR     VirtualDiskPath);

DWORD
SampleCreateVirtualDisk(
    _In_        LPCWSTR                     VirtualDiskPath,
    _In_opt_    LPCWSTR                     ParentPath,
    _In_        CREATE_VIRTUAL_DISK_FLAG    Flags,
    _In_        ULONGLONG                   FileSize,
    _In_        DWORD                       BlockSize,
    _In_        DWORD                       LogicalSectorSize,
    _In_        DWORD                       PhysicalSectorSize);

DWORD
SampleSetVirtualDiskInformation(
    _In_    LPCWSTR     ChildPath,
    _In_    LPCWSTR     ParentPath,
    _In_    DWORD       PhysicalSectorSize);

DWORD
SampleAttachVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    BOOLEAN     ReadOnly);

DWORD
SampleDetachVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath);

DWORD
SampleMergeVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath);

DWORD
SampleCompactVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath);

DWORD
SampleResizeVirtualDisk(
    _In_    LPCWSTR     VirtualDiskPath,
    _In_    ULONGLONG   FileSize);

DWORD
SampleMirrorVirtualDisk(
    _In_    LPCWSTR     SourcePath,
    _In_    LPCWSTR     DestinationPath);

DWORD
SampleRawIO(
    _In_    LPCWSTR     SourcePath,
    _In_    LPCWSTR     DestinationPath);

DWORD
SampleSetUserMetaData(
    _In_    LPCWSTR     VHDPath,
    _In_    DWORD       ID);

DWORD
SampleGetUserMetaData(
    _In_    LPCWSTR     VHDPath);

DWORD
SampleDeleteUserMetaData(
    _In_    LPCWSTR     VHDPath);

DWORD
SampleEnumerateUserMetaData(
    _In_    LPCWSTR     VHDPath);

DWORD
SampleGetStorageDependencyInformation(
    _In_    LPCWSTR     Disk);

DWORD
SampleGetAllAttachedVirtualDiskPhysicalPaths(
    );

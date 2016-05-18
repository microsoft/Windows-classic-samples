/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    VstortInterface.h

Abstract:

    Declaration of interfaces for utilizing VirtualStorage 
    driver.

--*/

#ifndef _VSTORINTERFACE_H_
#define _VSTORINTERFACE_H_


typedef enum 
{
    VIRTUAL_NONE = 0,
    VIRTUAL_CDROM,
    VIRTUAL_CDR,
    VIRTUAL_CDRW,
    VIRTUAL_DVDROM,
    VIRTUAL_DVDRAM,
    VIRTUAL_REMOVABLE_DISK,
    VIRTUAL_FIXED_DISK,
    VIRTUAL_INVALID_DEVICE_TYPE = 0xFFFFFFFF
}
VIRTUAL_DEVICE_TYPE;

typedef struct tagVirtualStorageVersion
{
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG Build;
    ULONG QFE;
}
VIRTUAL_STORAGE_VERSION_INFORMATION, *PVIRTUAL_STORAGE_VERSION_INFORMATION;

typedef struct tagNewVirtualDriveDescription
{
    ULONG  Length;
    ULONG  Flags;
    VIRTUAL_DEVICE_TYPE  DeviceType;
    GUID    DriveID;
    ULONG   BlockSize;
    ULONG   NumberOfBlocks;
    USHORT  FileNameOffset;  // offset in the buffer
    USHORT  FileNameLength;
    USHORT  StorageDeviceIdDescOffset;
    USHORT  StorageDeviceIdDescLength;
    UCHAR   Buffer[1];
}
NEW_VIRTUAL_DRIVE_DESCRIPTION, *PNEW_VIRTUAL_DRIVE_DESCRIPTION;

typedef struct tagVirtualDriveInformation
{
    GUID    DriveID;
    ULONG   Flags;
    VIRTUAL_DEVICE_TYPE DeviceType;
    ULONG   BlockSize;
    ULONG	NumberOfBlocks;
    BOOLEAN MediaInserted;
}
VIRTUAL_DRIVE_INFORMATION, *PVIRTUAL_DRIVE_INFORMATION;


namespace VstorInterface
{

class __declspec(dllexport) VirtualBus
{

public:
    typedef struct
    {
        ULONG  DeviceType;
        ULONG  DeviceNumber;
    }STORAGE_INFORMATION;
    
    VirtualBus();
    ~VirtualBus();

    bool  IsValid();

    HRESULT 
    QueryVersion(
        __out VIRTUAL_STORAGE_VERSION_INFORMATION&);
        
    HRESULT 
    CreateDriveEx(
        __in PNEW_VIRTUAL_DRIVE_DESCRIPTION  pDriveDesc,
        __out VIRTUAL_DRIVE_INFORMATION&  infoDrive);
        
    HRESULT 
    RemoveDrive(
        __in const GUID& guidDrive,
        __in bool bSurprise = false);

    HRESULT 
    QueryMountedImage(
        __in const GUID& guidDrive,
        __out_ecount(DWORD) LPWSTR strImage,
        __in DWORD dwChars);

    HRESULT 
    QueryDriveInterface(
        __in const GUID& guidDrive,
        __out_ecount(DWORD) LPWSTR strInterface,
        __in DWORD dwChars);
    
    HRESULT 
    QueryStorageInformation(
        __in const GUID& guidDrive,
        __out STORAGE_INFORMATION& infoStorage);

    HRESULT 
    QueryStorageInformation(
        __in LPCWSTR strInterface,
        __out STORAGE_INFORMATION& infoStorage);

    HRESULT 
    QueryStorageInformation(
        __in HANDLE hDrive,
        __out STORAGE_INFORMATION& infoStorage);

    HRESULT 
	ReSync(
		__in const GUID& guidDrive,
		__out LPCWSTR lpwstrSourceLun);
    
private:

    HRESULT Open();
    void    Close();
    HANDLE  m_hBus;
};

} // end of namespace VstorInterface

#endif

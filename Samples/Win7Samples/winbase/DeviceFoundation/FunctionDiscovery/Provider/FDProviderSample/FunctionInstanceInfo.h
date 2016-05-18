// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Define class TFunctionInstanceInfo.  A class to store information 
//      representing a function instance to be returned to a client.

#pragma once

class TFunctionInstanceInfo
{
public:
    VOID AddRef();
    VOID Release();

    static HRESULT CreateInstance(
        __in GUID* pDeviceId,
        __in_opt TDeviceInfo* pDeviceInfo,
        __in SOCKADDR_STORAGE* pFromAddr,
        INT FromAddrLen,
        ULONG InterfacIndex,
        __deref_out TFunctionInstanceInfo** ppFunctionInstanceInfo);

    PCWSTR GetFunctionInstanceId();

    HRESULT PopulatePropertyStore(
        IPropertyStore* pPropertyStore);

protected:
    TFunctionInstanceInfo();
    ~TFunctionInstanceInfo();  // Force use of Release

    LONG m_cRef;
    PWSTR m_pszDeviceId;
    WCHAR m_szIPAddress[INET6_ADDRSTRLEN];
    ULONG m_PhysicalAddressLength;
    UCHAR m_PhysicalAddress[IF_MAX_PHYS_ADDRESS_LENGTH];
    TDeviceInfo m_DeviceInfo;
    PWSTR* m_ppszDeviceCategories;
    ULONG m_cDeviceCategoriesCount;
}; // TFunctionInstanceInfo
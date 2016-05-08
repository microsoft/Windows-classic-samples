// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Implement class TFunctionInstanceInfo.  A class to store information 
//      representing a function instance to be returned to a client.

#include "stdafx.h"

TFunctionInstanceInfo::TFunctionInstanceInfo():
    m_cRef(1),
    m_pszDeviceId(NULL),
    m_PhysicalAddressLength(0),
    m_ppszDeviceCategories(NULL),
    m_cDeviceCategoriesCount(0)
{
    ZeroMemory(m_szIPAddress, sizeof(m_szIPAddress));
    ZeroMemory(m_PhysicalAddress, sizeof(m_PhysicalAddress));
    ZeroMemory(&m_DeviceInfo, sizeof(m_DeviceInfo));
}  // TFunctionInstanceInfo::TFunctionInstanceInfo

TFunctionInstanceInfo::~TFunctionInstanceInfo()
{
    if (m_pszDeviceId)
    {
        RpcStringFree((RPC_WSTR*) &m_pszDeviceId);
        m_pszDeviceId = NULL;
    }
    if (m_ppszDeviceCategories)
    {
        free(m_ppszDeviceCategories);
        m_ppszDeviceCategories = NULL;
    }
}  // TFunctionInstanceInfo::~TFunctionInstanceInfo

VOID TFunctionInstanceInfo::AddRef()
{
    InterlockedIncrement(&m_cRef);
}  // TFunctionInstanceInfo::AddRef

VOID TFunctionInstanceInfo::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
        delete this;
    }
}  // TFunctionInstanceInfo::Release

HRESULT TFunctionInstanceInfo::CreateInstance(
        __in GUID* pDeviceId,
        __in_opt TDeviceInfo* pDeviceInfo,
        __in SOCKADDR_STORAGE* pFromAddr,
        INT FromAddrLen,
        ULONG InterfaceIndex,
        __deref_out TFunctionInstanceInfo** ppFunctionInstanceInfo)
{
    HRESULT hr = S_OK;
    TFunctionInstanceInfo* pFunctionInstanceInfo = NULL;
    MIB_IPNET_ROW2 IpNetRow2 = {0};
    int err = 0;
    static const WCHAR szDelimeters[] = L" ,;";
    PWSTR pszToken = NULL;
    PWSTR pszNextToken = NULL;
    ULONG iCategoryIndex = 0;

    *ppFunctionInstanceInfo = NULL;

    // Create new Function Instance info
    if (S_OK == hr)
    {
        pFunctionInstanceInfo = new(std::nothrow) TFunctionInstanceInfo();
        if (!pFunctionInstanceInfo)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Get the MAC address for the From address
    if (S_OK == hr)
    {
        memcpy(
            &IpNetRow2.Address, 
            pFromAddr, 
            (FromAddrLen <= sizeof(IpNetRow2.Address)) ? FromAddrLen : sizeof(IpNetRow2.Address));
        IpNetRow2.InterfaceIndex = InterfaceIndex;
        err = GetIpNetEntry2(&IpNetRow2);
        if (NO_ERROR != err)
        {
            // Could not find the MAC address in the cache, lets hit the wire.
             err = ResolveIpNetEntry2(&IpNetRow2, NULL);
        }

        if (NO_ERROR == err)
        {
            memcpy(pFunctionInstanceInfo->m_PhysicalAddress, IpNetRow2.PhysicalAddress, IpNetRow2.PhysicalAddressLength);
            pFunctionInstanceInfo->m_PhysicalAddressLength = IpNetRow2.PhysicalAddressLength;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }

    // Convert the From Address to String form.
    if (S_OK == hr)
    {
        err = GetNameInfo(
            (PSOCKADDR) pFromAddr,
            FromAddrLen,
            pFunctionInstanceInfo->m_szIPAddress,
            ARRAYSIZE(pFunctionInstanceInfo->m_szIPAddress),
            NULL, 
            0,
            NI_NUMERICHOST | NI_NOFQDN);
        if (err != 0)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Convert the GUID into String Form
    if (S_OK == hr)
    {
        err = UuidToString(
            pDeviceId,
            (RPC_WSTR*) &pFunctionInstanceInfo->m_pszDeviceId);
        if (RPC_S_OK != err)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Copy device info
    if (S_OK == hr)
    {
        if (pDeviceInfo)
        {
            pFunctionInstanceInfo->m_DeviceInfo = *pDeviceInfo;
        }
    }

    // Create an array of device categories
    if (   (S_OK == hr)
        && *pFunctionInstanceInfo->m_DeviceInfo.szDeviceCategory)
    {
        // Count the number of delimeters in the string
        pFunctionInstanceInfo->m_cDeviceCategoriesCount = 1;
        pszToken = pFunctionInstanceInfo->m_DeviceInfo.szDeviceCategory;
        while (*pszToken)
        {
            if (wcschr(szDelimeters, *pszToken))
            {
                ++pFunctionInstanceInfo->m_cDeviceCategoriesCount;
            }
            ++pszToken;
        }

        // Allocate an array to hold the string pointers
        pFunctionInstanceInfo->m_ppszDeviceCategories = (PWSTR*) malloc(pFunctionInstanceInfo->m_cDeviceCategoriesCount * sizeof(PWSTR));

        if (pFunctionInstanceInfo->m_ppszDeviceCategories)
        {
            // Tokenize the device categories and save the pointers

            pszToken = wcstok_s(pFunctionInstanceInfo->m_DeviceInfo.szDeviceCategory,
                szDelimeters,
                &pszNextToken);

            while (   pszToken
                   && (iCategoryIndex < pFunctionInstanceInfo->m_cDeviceCategoriesCount))
            {
                pFunctionInstanceInfo->m_ppszDeviceCategories[iCategoryIndex] = pszToken;
                ++iCategoryIndex;
                  
                pszToken = wcstok_s(
                    NULL,
                    szDelimeters,
                    &pszNextToken);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    }

    // Cleanup
    if (S_OK == hr)
    {
        *ppFunctionInstanceInfo = pFunctionInstanceInfo;
    }
    else
    {
        delete pFunctionInstanceInfo;
    }

    return hr;
}  // TFunctionInstanceInfo::CreateInstance

PCWSTR TFunctionInstanceInfo::GetFunctionInstanceId()
{
    return m_pszDeviceId;
}

HRESULT TFunctionInstanceInfo::PopulatePropertyStore(
    IPropertyStore* pPropertyStore)
{
    HRESULT hr = S_OK;
    PROPVARIANT PropVar;

    PropVariantInit(&PropVar);

    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
    //
    // Set the correct PKEYs in the Function Instance for the device.
    // See http://www.microsoft.com/whdc/Rally/pnpx-spec.mspx for more details.
    //
    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO

    // Set the PKEY_PNPX_GlobalIdentity property.
    // This property is used to identify this instance of the device 
    // (Piece of plastic) on the network.
    // Every Function Instance must have a property store with at least the 
    // Global Identity set.
    PropVar.vt = VT_LPWSTR;
    PropVar.pwszVal = m_pszDeviceId;

    hr = pPropertyStore->SetValue(PKEY_PNPX_GlobalIdentity, PropVar);

    // ID
    // The sample device does not have internal subdevices, so we'll re-use the
    // device ID as the PNPX_ID
    if (S_OK == hr)
    {
        hr = pPropertyStore->SetValue(PKEY_PNPX_ID, PropVar);
    }

    // DeviceCategory
    // Device category is used to group devices in the UI.
    // See the PNP-X implementes guilde
    if (   (S_OK == hr)
        && m_ppszDeviceCategories)
    {
        PropVar.vt = VT_VECTOR | VT_LPWSTR;
        PropVar.calpwstr.cElems = m_cDeviceCategoriesCount;
        PropVar.calpwstr.pElems = m_ppszDeviceCategories;

        hr = pPropertyStore->SetValue(PKEY_PNPX_DeviceCategory, PropVar);
    }

    // FriendlyName
    // NOTE: Both PKEY_PNPX_FriendlyName 
    // and PKEY_Device_FriendlyName must be set
    if (   (S_OK == hr)
        && *m_DeviceInfo.szFriendlyName)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szFriendlyName;

        hr = pPropertyStore->SetValue(PKEY_PNPX_FriendlyName, PropVar);

        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_Device_FriendlyName, PropVar);
        }
    }

    // Manufacturer
    // NOTE: Both PKEY_PNPX_Manufacturer 
    // and PKEY_Device_Manufacturer must be set
    if (   (S_OK == hr)
        && *m_DeviceInfo.szManufacturer)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szManufacturer;

        hr = pPropertyStore->SetValue(PKEY_PNPX_Manufacturer, PropVar);

        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_Device_Manufacturer, PropVar);
        }
    }

    // ManufacturerUrl
    if (   (S_OK == hr)
        && *m_DeviceInfo.szManufacturerUrl)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szManufacturerUrl;

        hr = pPropertyStore->SetValue(PKEY_PNPX_ManufacturerUrl, PropVar);
        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_DrvPkg_VendorWebSite, PropVar);
        }
    }

    // ModelName
    if (   (S_OK == hr)
        && *m_DeviceInfo.szModelName)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szModelName;

        hr = pPropertyStore->SetValue(PKEY_PNPX_ModelName, PropVar);

        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_Device_Model, PropVar);
        }
    }

    // ModelNumber
    if (   (S_OK == hr)
        && *m_DeviceInfo.szModelNumber)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szModelNumber;

        hr = pPropertyStore->SetValue(PKEY_PNPX_ModelNumber, PropVar);
    }

    // ModelUrl
    if (   (S_OK == hr)
        && *m_DeviceInfo.szModelUrl)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szModelUrl;
        
        hr = pPropertyStore->SetValue(PKEY_PNPX_ModelUrl, PropVar);

        // For the sample we'll assume that the URL to
        // where users could download the driver is the same as
        // the Model's URL
        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_DriverPackage_VendorWebSite, PropVar);
        }
    }

    // UPC
    if (   (S_OK == hr)
        && *m_DeviceInfo.szUpc)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szUpc;
        
        hr = pPropertyStore->SetValue(PKEY_PNPX_Upc, PropVar);
    }

    // FirmwareVersion
    if (   (S_OK == hr)
        && *m_DeviceInfo.szFirmwareVersion)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szFirmwareVersion;

        hr = pPropertyStore->SetValue(PKEY_PNPX_FirmwareVersion, PropVar);

        if (S_OK == hr)
        {
            hr = pPropertyStore->SetValue(PKEY_Device_BIOSVersion, PropVar);
        }
    }

    // SerialNumber
    if (   (S_OK == hr)
        && *m_DeviceInfo.szSerialNumber)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szSerialNumber;

        hr = pPropertyStore->SetValue(PKEY_PNPX_SerialNumber, PropVar);
    }

    // Presentation URL
    // Typically this is a administration page hosted by the device.
    if (   (S_OK == hr)
        && *m_DeviceInfo.szPresentationUrl)
    {
        PropVar.vt = VT_LPWSTR;
        PropVar.pwszVal = m_DeviceInfo.szPresentationUrl;

        hr = pPropertyStore->SetValue(PKEY_PNPX_PresentationUrl, PropVar);
    }

    // PhysicalAddress
    // MAC address(es) of the device.
    if (S_OK == hr)
    {
        PropVar.vt = VT_VECTOR | VT_UI1;
        PropVar.caub.cElems = m_PhysicalAddressLength;
        PropVar.caub.pElems = m_PhysicalAddress;
        
        hr = pPropertyStore->SetValue(PKEY_PNPX_PhysicalAddress, PropVar);
    }

    // IpAddress
    // NOTE:  Multiple addresses can be specified
    if (S_OK == hr)
    {
        PWSTR ppszIPAddresses[1] = { m_szIPAddress };

        PropVar.vt = VT_VECTOR | VT_LPWSTR;
        PropVar.calpwstr.cElems = 1;
        PropVar.calpwstr.pElems = ppszIPAddresses;

        hr = pPropertyStore->SetValue(PKEY_PNPX_IpAddress, PropVar);
    }

    // HardwareId
    // A Function Instance must have PKEY_Device_HardwareIds
    // and optionally PKEY_Device_CompatibleIds to support driver installation.
    if (   (S_OK == hr)
        && *m_DeviceInfo.szPnPHardwareId)
    {
        PWSTR ppszHardwareIds[1] = { m_DeviceInfo.szPnPHardwareId };

        PropVar.vt = VT_VECTOR | VT_LPWSTR;
        PropVar.calpwstr.cElems = 1;
        PropVar.calpwstr.pElems = ppszHardwareIds;

        hr = pPropertyStore->SetValue(PKEY_Device_HardwareIds, PropVar);
    }

    // PKEY_PNPX_Installable must be set to mark this
    // function instance as installable in the UI
    if (   (S_OK == hr)
        && *m_DeviceInfo.szPnPHardwareId)  // Can only install a device that have Hardware or Compatible IDs
    {
        PropVar.vt = VT_BOOL;
        PropVar.boolVal = VARIANT_TRUE;

        hr = pPropertyStore->SetValue(PKEY_PNPX_Installable, PropVar);
    }

    // Compatable types
    // This list is matched, in order, to entries under the HKCR\NetworkExplorerPlugins
    // to find the Network Explorer UI extensions for this device.  This value must be supplied
    // by a FI but if it does not match an entry in the registry, default UI will be used.
    // See http://www.microsoft.com/whdc/device/network/netexplorer.mspx for details.
    if (S_OK == hr)
    {
        // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
        //
        // Use a Class or model specific CompatableType(s) here
        //
        // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO

        PWSTR ppszCompatibleTypes[1] = { L"FDProviderSample Types" };

        PropVar.vt = VT_VECTOR | VT_LPWSTR;
        PropVar.calpwstr.cElems = 1;
        PropVar.calpwstr.pElems = ppszCompatibleTypes;  

        hr = pPropertyStore->SetValue(PKEY_PNPX_CompatibleTypes, PropVar);
    }
    

    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
    //
    // Set any additional PKEY_Device_*, PKEY_PNPX_* 
    // or Provider specific custom properties here.
    //
    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO

    return hr;
}


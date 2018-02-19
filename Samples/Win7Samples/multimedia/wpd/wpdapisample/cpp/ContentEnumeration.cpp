// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>

// This number controls how many object identifiers are requested during each call
// to IEnumPortableDeviceObjectIDs::Next()
//<SnippetContentEnum2>
#define NUM_OBJECTS_TO_REQUEST  10

// Recursively called function which enumerates using the specified
// object identifier as the parent.
void RecursiveEnumerate(
    PCWSTR                  pszObjectID,
    IPortableDeviceContent* pContent)
{
    CComPtr<IEnumPortableDeviceObjectIDs> pEnumObjectIDs;

    // Print the object identifier being used as the parent during enumeration.
    printf("%ws\n",pszObjectID);

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    HRESULT hr = pContent->EnumObjects(0,               // Flags are unused
                                       pszObjectID,     // Starting from the passed in object
                                       NULL,            // Filter is unused
                                       &pEnumObjectIDs);
    if (FAILED(hr))
    {
        printf("! Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent, hr = 0x%lx\n",hr);
    }

    // Loop calling Next() while S_OK is being returned.
    while(hr == S_OK)
    {
        DWORD  cFetched = 0;
        PWSTR  szObjectIDArray[NUM_OBJECTS_TO_REQUEST] = {0};
        hr = pEnumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST,   // Number of objects to request on each NEXT call
                                  szObjectIDArray,          // Array of PWSTR array which will be populated on each NEXT call
                                  &cFetched);               // Number of objects written to the PWSTR array
        if (SUCCEEDED(hr))
        {
            // Traverse the results of the Next() operation and recursively enumerate
            // Remember to free all returned object identifiers using CoTaskMemFree()
            for (DWORD dwIndex = 0; dwIndex < cFetched; dwIndex++)
            {
                RecursiveEnumerate(szObjectIDArray[dwIndex],pContent);

                // Free allocated PWSTRs after the recursive enumeration call has completed.
                CoTaskMemFree(szObjectIDArray[dwIndex]);
                szObjectIDArray[dwIndex] = NULL;
            }
        }
    }
}
//</SnippetContentEnum2>
// Enumerate all content on the device starting with the
// "DEVICE" object
//<SnippetContentEnum1>
void EnumerateAllContent(
    IPortableDevice* pDevice)
{
    HRESULT                         hr = S_OK;
    CComPtr<IPortableDeviceContent> pContent;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = pDevice->Content(&pContent);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Enumerate content starting from the "DEVICE" object.
    if (SUCCEEDED(hr))
    {
        printf("\n");
        RecursiveEnumerate(WPD_DEVICE_OBJECT_ID, pContent);
    }
}
//</SnippetContentEnum1>
// Recursively called function which enumerates using the specified
// object identifier as the parent and populates the returned object
// identifiers into an IPortableDevicePropVariantCollection object.
void RecursiveEnumerateAndCopyToCollection(
    PCWSTR                                pszObjectID,
    IPortableDeviceContent*               pContent,
    IPortableDevicePropVariantCollection* pObjectIDs)
{
    HRESULT                               hr = S_OK;
    CComPtr<IEnumPortableDeviceObjectIDs> pEnumObjectIDs;

    // Add the object identifier being used as the parent during enumeration
    // to the collection.
    PROPVARIANT pv = {0};
    PropVariantInit(&pv);

    // Allocated a new string in a PROPVARIANT so we can add it to our
    // collection object.
    pv.vt      = VT_LPWSTR;
    pv.pwszVal = AtlAllocTaskWideString(pszObjectID);

    // Add the object identifier...
    hr = pObjectIDs->Add(&pv);

    // Free the allocated string in the PROPVARIANT
    PropVariantClear(&pv);

    // If we failed to add the object identifier, return immediately.
    if (FAILED(hr))
    {
        printf("! Failed to add object identifier '%ws' to the IPortableDevicePropVariantCollection, hr = 0x%lx\n",pszObjectID, hr);
        return;
    }

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    hr = pContent->EnumObjects(0,               // Flags are unused
                               pszObjectID,     // Starting from the passed in object
                               NULL,            // Filter is unused
                               &pEnumObjectIDs);
    if (FAILED(hr))
    {
        printf("! Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent, hr = 0x%lx\n",hr);
    }

    // Loop calling Next() while S_OK is being returned.
    while(hr == S_OK)
    {
        DWORD  cFetched = 0;
        PWSTR  szObjectIDArray[NUM_OBJECTS_TO_REQUEST] = {0};
        hr = pEnumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST,   // Number of objects to request on each NEXT call
                                  szObjectIDArray,          // Array of PWSTR array which will be populated on each NEXT call
                                  &cFetched);               // Number of objects written to the PWSTR array
        if (SUCCEEDED(hr))
        {
            // Traverse the results of the Next() operation and recursively enumerate
            // Remember to free all returned object identifiers using CoTaskMemFree()
            for (DWORD dwIndex = 0; dwIndex < cFetched; dwIndex++)
            {
                RecursiveEnumerateAndCopyToCollection(szObjectIDArray[dwIndex], pContent, pObjectIDs);

                // Free allocated PWSTRs after the recursive enumeration call has completed.
                CoTaskMemFree(szObjectIDArray[dwIndex]);
                szObjectIDArray[dwIndex] = NULL;
            }
        }
    }
}

// Enumerate all content on the device starting with the
// "DEVICE" object and populates the returned object identifiers
// into an IPortableDevicePropVariantCollection
HRESULT CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(
    IPortableDeviceContent* pContent,
    IPortableDevicePropVariantCollection** ppObjectIDs)
{
    CComPtr<IPortableDevicePropVariantCollection> pObjectIDs;

    // CoCreate an IPortableDevicePropVariantCollection interface to hold the the object identifiers
    HRESULT hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectIDs));
    if (SUCCEEDED(hr))
    {
        if (pObjectIDs != NULL)
        {
            RecursiveEnumerateAndCopyToCollection(WPD_DEVICE_OBJECT_ID, pContent, pObjectIDs);
        }
    }

    hr = pObjectIDs->QueryInterface(IID_PPV_ARGS(ppObjectIDs));
    if (FAILED(hr))
    {
        printf("! Failed to QueryInterface for IPortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
    }

    return hr;
}

#define VerifyHrOrThrow(x)                  \
    do {                                    \
        HRESULT hr = (x);                   \
        if (FAILED(hr))                     \
        {                                   \
            printf(#x ", hr = 0x%lx\n",hr); \
            AtlThrow(hr);              \
        }                                   \
    }                                       \
    while (0);

namespace Helpers
{
    class SimplePropVariantWrapper : public PROPVARIANT
    {
    public:
        SimplePropVariantWrapper()
        {
            PropVariantInit(this);
        }
        SimplePropVariantWrapper(const PROPVARIANT& other) throw(...)
        {
            PropVariantInit(this);
            VerifyHrOrThrow(PropVariantCopy(this, &other));
        }
        SimplePropVariantWrapper& operator=(const PROPVARIANT& other) throw(...)
        {
            if (this != &other)
            {
                PropVariantClear(this);
                VerifyHrOrThrow(PropVariantCopy(this, &other));
            }
        }
        ~SimplePropVariantWrapper()
        {
            PropVariantClear(this);
        }
    };

}
    
void ReadHintLocations(
    IPortableDevice* pDevice)
{
    _ATLTRY
    {
        // We will store the list of folder IDs here
        CAtlList<CAtlStringW> folderList;

        // Get the device content
        CComPtr<IPortableDeviceContent> spPortableDeviceContent;
        VerifyHrOrThrow(pDevice->Content(&spPortableDeviceContent));

        // Get the device properties
        CComPtr<IPortableDeviceProperties> spPortableDeviceProperties;
        VerifyHrOrThrow(spPortableDeviceContent->Properties(&spPortableDeviceProperties));

        // Loop through some typical content types supported by Portable Devices
        GUID formatTypes[] = {WPD_CONTENT_TYPE_IMAGE, WPD_CONTENT_TYPE_VIDEO};
        PWSTR formatTypeStrings[] = {L"WPD_CONTENT_TYPE_IMAGE", L"WPD_CONTENT_TYPE_VIDEO"};
        for (int nCurrentType = 0; nCurrentType < ARRAYSIZE(formatTypes); ++nCurrentType)
        {
            // Create and initialize the command parameters
            CComPtr<IPortableDeviceValues> spParams;

            printf("Folders for content type '%ws':\n", formatTypeStrings[nCurrentType]);

            VerifyHrOrThrow(
                spParams.CoCreateInstance(CLSID_PortableDeviceValues)
                );
            VerifyHrOrThrow(
                spParams->SetGuidValue(
                    WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                    WPD_COMMAND_DEVICE_HINTS_GET_CONTENT_LOCATION.fmtid
                    )
                );
            VerifyHrOrThrow(
                spParams->SetUnsignedIntegerValue(
                    WPD_PROPERTY_COMMON_COMMAND_ID,
                    WPD_COMMAND_DEVICE_HINTS_GET_CONTENT_LOCATION.pid
                    )
                );
            VerifyHrOrThrow(
                spParams->SetGuidValue(
                    WPD_PROPERTY_DEVICE_HINTS_CONTENT_TYPE,
                    formatTypes[nCurrentType]
                    )
                );

            // Send the command
            CComPtr<IPortableDeviceValues> spResults;
            if (S_OK == pDevice->SendCommand(0, spParams, &spResults))
            {
                // Get the collection
                CComPtr<IPortableDevicePropVariantCollection> spPortableDevicePropVariantCollection;
                if (S_OK == spResults->GetIPortableDevicePropVariantCollectionValue(
                        WPD_PROPERTY_DEVICE_HINTS_CONTENT_LOCATIONS,
                        &spPortableDevicePropVariantCollection
                        )
                    )
                {
                    // Get the count of folders
                    DWORD dwCount = 0;
                    VerifyHrOrThrow(spPortableDevicePropVariantCollection->GetCount(&dwCount));

                    // Loop through each of the folders
                    for (DWORD dwCurrentSubFolder = 0; dwCurrentSubFolder < dwCount; ++dwCurrentSubFolder)
                    {
                        // Get the folder id
                        Helpers::SimplePropVariantWrapper propVarCurrentSubFolder;
                        VerifyHrOrThrow(
                            spPortableDevicePropVariantCollection->GetAt(
                                dwCurrentSubFolder,
                                &propVarCurrentSubFolder
                                )
                            );
                        if (propVarCurrentSubFolder.vt == VT_BSTR ||
                            propVarCurrentSubFolder.vt == VT_LPWSTR
                            )
                        {
                            // Get the values for this item
                            CComPtr<IPortableDeviceValues> spPortableDeviceValues;
                            VerifyHrOrThrow(
                                spPortableDeviceProperties->GetValues(
                                    propVarCurrentSubFolder.pwszVal,
                                    NULL,
                                    &spPortableDeviceValues)
                                    );

                            // Get the PUOID
                            CComHeapPtr<WCHAR> spszPersistentUniqueId;
                            VerifyHrOrThrow(
                                spPortableDeviceValues->GetStringValue(
                                    WPD_OBJECT_PERSISTENT_UNIQUE_ID,
                                    &spszPersistentUniqueId
                                    )
                                );

                            // Convert it to a string
                            CAtlStringW strPersistentUniqueId(spszPersistentUniqueId);

                            printf(" '%ws' (%ws)\n",
                                propVarCurrentSubFolder.pwszVal,
                                strPersistentUniqueId.GetString()
                                );
                        }
                        else
                        {
                            printf("Driver returned unexpected PROVARIANT Type: %d\n", propVarCurrentSubFolder.vt);
                        }
                    }
                }
            }
        }
    }
    _ATLCATCH(e)
    {
        HRESULT hr = e;
        printf("Exception caught, error = 0x%lx\n", hr);
    }
}



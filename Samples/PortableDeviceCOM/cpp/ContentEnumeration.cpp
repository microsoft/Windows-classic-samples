// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

// This number controls how many object identifiers are requested during each call
// to IEnumPortableDeviceObjectIDs::Next()
//<SnippetContentEnum2>
#define NUM_OBJECTS_TO_REQUEST  10

// Recursively called function which enumerates using the specified
// object identifier as the parent.
void RecursiveEnumerate(
    _In_ PCWSTR                  objectID,
    _In_ IPortableDeviceContent* content)
{
    ComPtr<IEnumPortableDeviceObjectIDs> enumObjectIDs;

    // Print the object identifier being used as the parent during enumeration.
    wprintf(L"%ws\n", objectID);

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    HRESULT hr = content->EnumObjects(0,                // Flags are unused
                                      objectID,         // Starting from the passed in object
                                      nullptr,          // Filter is unused
                                      &enumObjectIDs);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent, hr = 0x%lx\n", hr);
    }

    // Loop calling Next() while S_OK is being returned.
    while(hr == S_OK)
    {
        DWORD  numFetched = 0;
        PWSTR  objectIDArray[NUM_OBJECTS_TO_REQUEST] = {0};
        hr = enumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST,    // Number of objects to request on each NEXT call
                                 objectIDArray,             // Array of PWSTR array which will be populated on each NEXT call
                                 &numFetched);              // Number of objects written to the PWSTR array
        if (SUCCEEDED(hr))
        {
            // Traverse the results of the Next() operation and recursively enumerate
            // Remember to free all returned object identifiers using CoTaskMemFree()
            for (DWORD index = 0; (index < numFetched) && (objectIDArray[index] != nullptr); index++)
            {
                RecursiveEnumerate(objectIDArray[index], content);

                // Free allocated PWSTRs after the recursive enumeration call has completed.
                CoTaskMemFree(objectIDArray[index]);
                objectIDArray[index] = nullptr;
            }
        }
    }
}
//</SnippetContentEnum2>
// Enumerate all content on the device starting with the
// "DEVICE" object
//<SnippetContentEnum1>
void EnumerateAllContent(
    _In_ IPortableDevice* device)
{
    HRESULT                         hr = S_OK;
    ComPtr<IPortableDeviceContent>  content;

    // Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = device->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Enumerate content starting from the "DEVICE" object.
    if (SUCCEEDED(hr))
    {
        wprintf(L"\n");
        RecursiveEnumerate(WPD_DEVICE_OBJECT_ID, content.Get());
    }
}
//</SnippetContentEnum1>
// Recursively called function which enumerates using the specified
// object identifier as the parent and populates the returned object
// identifiers into an IPortableDevicePropVariantCollection object.
void RecursiveEnumerateAndCopyToCollection(
    _In_ PCWSTR                                objectID,
    _In_ IPortableDeviceContent*               content,
    _In_ IPortableDevicePropVariantCollection* objectIDs)
{
    HRESULT                               hr = S_OK;
    ComPtr<IEnumPortableDeviceObjectIDs>  enumObjectIDs;

    // Add the object identifier being used as the parent during enumeration
    // to the collection.
    PROPVARIANT pv;

    // Allocated a new string in a PROPVARIANT so we can add it to our
    // collection object.
    hr = InitPropVariantFromString(objectID, &pv);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to copy object identifier '%ws', hr = 0x%lx\n", objectID, hr);
        return;
    }

    // Add the object identifier...
    hr = objectIDs->Add(&pv);

    // Free the allocated string in the PROPVARIANT
    PropVariantClear(&pv);

    // If we failed to add the object identifier, return immediately.
    if (FAILED(hr))
    {
        wprintf(L"! Failed to add object identifier '%ws' to the IPortableDevicePropVariantCollection, hr = 0x%lx\n", objectID, hr);
        return;
    }

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    hr = content->EnumObjects(0,               // Flags are unused
                              objectID,        // Starting from the passed in object
                              nullptr,         // Filter is unused
                              &enumObjectIDs);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent, hr = 0x%lx\n", hr);
    }

    // Loop calling Next() while S_OK is being returned.
    while(hr == S_OK)
    {
        DWORD  numFetched = 0;
        PWSTR  objectIDArray[NUM_OBJECTS_TO_REQUEST] = {0};
        hr = enumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST,    // Number of objects to request on each Next() call
                                 objectIDArray,             // PWSTR array which will be populated on each Next() call
                                 &numFetched);              // Number of objects written to the PWSTR array
        if (SUCCEEDED(hr))
        {
            // Traverse the results of the Next() operation and recursively enumerate
            // Remember to free all returned object identifiers using CoTaskMemFree()
            for (DWORD index = 0; (index < numFetched) && (objectIDArray[index] != nullptr); index++)
            {
                RecursiveEnumerateAndCopyToCollection(objectIDArray[index], content, objectIDs);

                // Free allocated PWSTRs after the recursive enumeration call has completed.
                CoTaskMemFree(objectIDArray[index]);
                objectIDArray[index] = nullptr;
            }
        }
    }
}

// Enumerate all content on the device starting with the
// "DEVICE" object and populates the returned object identifiers
// into an IPortableDevicePropVariantCollection
HRESULT CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(
    _In_         IPortableDeviceContent*                content,
    _COM_Outptr_ IPortableDevicePropVariantCollection** objectIDs)
{
    *objectIDs = nullptr;
    ComPtr<IPortableDevicePropVariantCollection> objectIDsTemp;

    // CoCreate an IPortableDevicePropVariantCollection interface to hold the the object identifiers
    HRESULT hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectIDsTemp));
    if (SUCCEEDED(hr))
    {
        RecursiveEnumerateAndCopyToCollection(WPD_DEVICE_OBJECT_ID, content, objectIDsTemp.Get());
        *objectIDs = objectIDsTemp.Detach();
    }

    return hr;
}

HRESULT SendHintsCommand(
    _In_         IPortableDevice*           device,
    _In_         REFGUID                    contentType,
    _COM_Outptr_ IPortableDeviceValues**    results)
{
    *results = nullptr;

    // Create and initialize the command parameters
    ComPtr<IPortableDeviceValues> params;
    ComPtr<IPortableDeviceValues> resultsTemp;

    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&params));

    if (SUCCEEDED(hr))
    {
        hr = params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                                  WPD_COMMAND_DEVICE_HINTS_GET_CONTENT_LOCATION.fmtid);

        if (SUCCEEDED(hr))
        {
            hr = params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                                 WPD_COMMAND_DEVICE_HINTS_GET_CONTENT_LOCATION.pid);

            if (SUCCEEDED(hr))
            {
                hr = params->SetGuidValue(WPD_PROPERTY_DEVICE_HINTS_CONTENT_TYPE,
                                          contentType);

                if (SUCCEEDED(hr))
                {
                    // Send the command
                    hr = device->SendCommand(0, params.Get(), &resultsTemp);
                    if (SUCCEEDED(hr))
                    {
                        *results = resultsTemp.Detach();
                    }
                }
            }
        }
    }
    return hr;
}

void ReadHintsResults(
    _In_ IPortableDeviceProperties* properties,
    _In_ IPortableDeviceValues*     results)
{
    ComPtr<IPortableDevicePropVariantCollection> folderIDs;

    // Get the collection
    if (S_OK == results->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_DEVICE_HINTS_CONTENT_LOCATIONS,
                                                                      &folderIDs))
    {
        // Get the count of folders
        DWORD numFolderIDs = 0;
        if (SUCCEEDED(folderIDs->GetCount(&numFolderIDs)))
        {
            // Loop through each of the folders
            for (DWORD index = 0; index < numFolderIDs; ++index)
            {
                // Get the folder id
                PROPVARIANT folderID = {0};

                if (SUCCEEDED(folderIDs->GetAt(index, &folderID)))
                {
                    if (folderID.vt == VT_LPWSTR)
                    {
                        // Get the properties for this item
                        ComPtr<IPortableDeviceValues> folderProperties;

                        if (SUCCEEDED(properties->GetValues(folderID.pwszVal, nullptr, &folderProperties)))
                        {
                            // Get the persistent unique object id
                            PWSTR folderPersistentUniqueID = nullptr;
                            if (SUCCEEDED(folderProperties->GetStringValue(WPD_OBJECT_PERSISTENT_UNIQUE_ID,
                                                                           &folderPersistentUniqueID)))
                            {
                                wprintf(L" '%ws' (%ws)\n", folderID.pwszVal, folderPersistentUniqueID);

                                CoTaskMemFree(folderPersistentUniqueID);
                            }
                        }
                    }
                    else
                    {
                        wprintf(L"Driver returned unexpected PROVARIANT Type: %u\n", folderID.vt);
                    }
                }

                PropVariantClear(&folderID);
            }
        }
    }
}

void ReadHintLocations(
    _In_ IPortableDevice* device)
{
    // Get the device content
    ComPtr<IPortableDeviceContent> content;
    HRESULT hr = device->Content(&content);
    if (SUCCEEDED(hr))
    {
        // Get the device properties
        ComPtr<IPortableDeviceProperties> properties;

        hr = content->Properties(&properties);
        if (SUCCEEDED(hr))
        {
            // Loop through some typical content types supported by Portable Devices
            const GUID* formatTypes[]  = {&WPD_CONTENT_TYPE_IMAGE,   &WPD_CONTENT_TYPE_VIDEO};
            PCWSTR formatTypeStrings[] = {L"WPD_CONTENT_TYPE_IMAGE", L"WPD_CONTENT_TYPE_VIDEO"};

            for (DWORD index = 0; index < ARRAYSIZE(formatTypes); ++index)
            {
                ComPtr<IPortableDeviceValues> results;

                wprintf(L"Folders for content type '%ws':\n", formatTypeStrings[index]);

                if (SUCCEEDED(SendHintsCommand(device, *formatTypes[index], &results)))
                {
                    ReadHintsResults(properties.Get(), results.Get());
                }
            }
        }
    }
}

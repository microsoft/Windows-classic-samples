// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

// This number controls how many object identifiers are requested during each call
// to IEnumPortableDeviceObjectIDs::Next()
#define NUM_OBJECTS_TO_REQUEST  10

//<SnippetContentEnumeration2>
// Recursively called function which enumerates using the specified
// object identifier as the parent.
void RecursiveEnumerate(
    _In_ PCWSTR                   objectID,
    _In_ IPortableDeviceContent2* content)
{
    ComPtr<IEnumPortableDeviceObjectIDs> enumObjectIDs;

    // Print the object identifier being used as the parent during enumeration.
    wprintf(L"%ws\n", objectID);

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    HRESULT hr = content->EnumObjects(0,               // Flags are unused
                                      objectID,     // Starting from the passed in object
                                      nullptr,         // Filter is unused
                                      &enumObjectIDs);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent2, hr = 0x%lx\n", hr);
    }

    // Loop calling Next() while S_OK is being returned.
    while(hr == S_OK)
    {
        DWORD  numFetched = 0;
        PWSTR  objectIDArray[NUM_OBJECTS_TO_REQUEST] = {0};
        hr = enumObjectIDs->Next(ARRAYSIZE(objectIDArray),   // Number of objects to request on each NEXT call
                                 objectIDArray,              // Array of PWSTR array which will be populated on each NEXT call
                                 &numFetched);               // Number of objects written to the PWSTR array
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
//</SnippetContentEnumeration2>
//<SnippetContentEnumeration1>
// Enumerate all content on the service starting with the
// "DEVICE" object
void EnumerateAllContent(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceContent2>  content;

    // Get an IPortableDeviceContent2 interface from the IPortableDeviceService interface to
    // access the content-specific methods.
    HRESULT hr = service->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent2 from IPortableDeviceService, hr = 0x%lx\n", hr);
    }
    else
    {
        // Enumerate content starting from the "DEVICE" object.
        wprintf(L"\n");
        RecursiveEnumerate(WPD_DEVICE_OBJECT_ID, content.Get());
    }
}
//</SnippetContentEnumeration1>

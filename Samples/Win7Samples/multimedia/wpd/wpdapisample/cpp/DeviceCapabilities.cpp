// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

// Displays a friendly name for a passed in functional
// category.  If the category is not known by this function
// the GUID will be displayed in string form.
void DisplayFunctionalCategory(
    REFGUID guidCategory)
{
    if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_STORAGE, guidCategory))
    {
        printf("WPD_FUNCTIONAL_CATEGORY_STORAGE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_STILL_IMAGE_CAPTURE, guidCategory))
    {
        printf("WPD_FUNCTIONAL_CATEGORY_STILL_IMAGE_CAPTURE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_AUDIO_CAPTURE, guidCategory))
    {
        printf("WPD_FUNCTIONAL_CATEGORY_AUDIO_CAPTURE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_SMS, guidCategory))
    {
        printf("WPD_FUNCTIONAL_CATEGORY_SMS");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION, guidCategory))
    {
        printf("WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION");
    }
    else
    {
        printf("%ws", (PWSTR)CGuidToString(guidCategory));
    }
}

// Displays a friendly name for a passed in event
// If the event is not known by this function
// the GUID will be displayed in string form.
void DisplayEvent(
    REFGUID guidEvent)
{
    if (IsEqualGUID(WPD_EVENT_OBJECT_ADDED, guidEvent))
    {
        printf("WPD_EVENT_OBJECT_ADDED");
    }
    else if (IsEqualGUID(WPD_EVENT_OBJECT_REMOVED, guidEvent))
    {
        printf("WPD_EVENT_OBJECT_REMOVED");
    }
    else if (IsEqualGUID(WPD_EVENT_OBJECT_UPDATED, guidEvent))
    {
        printf("WPD_EVENT_OBJECT_UPDATED");
    }
    else if (IsEqualGUID(WPD_EVENT_DEVICE_RESET, guidEvent))
    {
        printf("WPD_EVENT_DEVICE_RESET");
    }
    else if (IsEqualGUID(WPD_EVENT_DEVICE_CAPABILITIES_UPDATED, guidEvent))
    {
        printf("WPD_EVENT_DEVICE_CAPABILITIES_UPDATED");
    }
    else if (IsEqualGUID(WPD_EVENT_STORAGE_FORMAT, guidEvent))
    {
        printf("WPD_EVENT_STORAGE_FORMAT");
    }
    else
    {
        printf("%ws", (PWSTR)CGuidToString(guidEvent));
    }
}

// Displays a friendly name for a passed in content type
// If the content type is not known by this function
// the GUID will be displayed in string form.
void DisplayContentType(
    REFGUID guidContentType)
{
    if (IsEqualGUID(WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_FOLDER, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_FOLDER");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_IMAGE, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_IMAGE");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_DOCUMENT, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_DOCUMENT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_CONTACT, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_CONTACT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_AUDIO, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_AUDIO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_VIDEO, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_VIDEO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_TASK, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_TASK");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_APPOINTMENT, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_APPOINTMENT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_EMAIL, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_EMAIL");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_MEMO, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_MEMO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_UNSPECIFIED, guidContentType))
    {
        printf("WPD_CONTENT_TYPE_UNSPECIFIED");
    }
    else
    {
        printf("%ws", (PWSTR)CGuidToString(guidContentType));
    }
}

// Display the basic event options for the passed in event.
void DisplayEventOptions(
    IPortableDeviceCapabilities* pCapabilities,
    REFGUID                      guidEvent)
{
    CComPtr<IPortableDeviceValues> pOptions;
    HRESULT hr = pCapabilities->GetEventOptions(guidEvent, &pOptions);
    if (FAILED(hr))
    {
        printf("! Failed to get even options, hr = 0x%lx\n",hr);
    }

    if (hr == S_OK)
    {
        printf("Event Options:\n");
        // Read the WPD_EVENT_OPTION_IS_BROADCAST_EVENT value to see if the event is
        // a broadcast event. If the read fails, assume FALSE
        BOOL  bIsBroadcastEvent = FALSE;
        hr = pOptions->GetBoolValue(WPD_EVENT_OPTION_IS_BROADCAST_EVENT, &bIsBroadcastEvent);
        if (SUCCEEDED(hr))
        {
            printf("\tWPD_EVENT_OPTION_IS_BROADCAST_EVENT = %ws\n",bIsBroadcastEvent ? L"TRUE" : L"FALSE");
        }
        else
        {
            printf("! Failed to get WPD_EVENT_OPTION_IS_BROADCAST_EVENT (assuming FALSE), hr = 0x%lx\n",hr);
        }
    }
}

// Display all functional object identifiers contained in an IPortableDevicePropVariantCollection
// NOTE: These values are assumed to be in VT_LPWSTR VarType format.
void DisplayFunctionalObjectIDs(
    IPortableDevicePropVariantCollection* pFunctionalObjectIds)
{
    DWORD dwNumObjectIDs = 0;

    // Get the total number of object identifiers in the collection.
    HRESULT hr = pFunctionalObjectIds->GetCount(&dwNumObjectIDs);
    if (SUCCEEDED(hr))
    {
        // Loop through the collection and displaying each object identifier found.
        // This loop prints a comma-separated list of the object identifiers.
        for (DWORD dwObjectIDIndex = 0; dwObjectIDIndex < dwNumObjectIDs; dwObjectIDIndex++)
        {
            PROPVARIANT pvObjectID = {0};
            PropVariantInit(&pvObjectID);
            hr = pFunctionalObjectIds->GetAt(dwObjectIDIndex, &pvObjectID);
            if (SUCCEEDED(hr))
            {
                // We have a functional object identifier.  It is assumed that
                // object identifiers are returned as VT_LPWSTR
                // VarTypes.

                if ((pvObjectID.pwszVal != NULL)        &&
                    (pvObjectID.vt      == VT_LPWSTR))
                {
                    // Display the object identifiers separated by commas
                    printf("%ws", pvObjectID.pwszVal);
                    if ((dwObjectIDIndex + 1) < dwNumObjectIDs)
                    {
                        printf(", ");
                    }
                }
                else
                {
                    printf("! Invalid functional object identifier found\n");
                }
            }

            PropVariantClear(&pvObjectID);
        }
    }
}

// List all functional objects on the device
void ListFunctionalObjects(
    IPortableDevice* pDevice)
{
	//<SnippetCapabilities7>
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities>            pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pCategories;
    DWORD dwNumCategories = 0;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to enumerate functional objects
    // that fall within them.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetFunctionalCategories(&pCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get functional categories from the device, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = pCategories->GetCount(&dwNumCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get number of functional categories, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Functional Categories Found on the device\n\n", dwNumCategories);

    // Loop through each functional category and get the list of
    // functional object identifiers associated with a particular
    // category.
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumCategories; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pCategories->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID
                // VarTypes.
                if ((pv.puuid != NULL)      &&
                    (pv.vt    == VT_CLSID))
                {
                    // Display the functional category name
                    printf("Functional Category: ");
                    DisplayFunctionalCategory(*pv.puuid);
                    printf("\n");

                    // Display the object identifiers for all
                    // functional objects within this category
                    CComPtr<IPortableDevicePropVariantCollection> pFunctionalObjectIds;
                    hr = pCapabilities->GetFunctionalObjects(*pv.puuid, &pFunctionalObjectIds);
                    if (SUCCEEDED(hr))
                    {
                        printf("Functional Objects: ");
                        DisplayFunctionalObjectIDs(pFunctionalObjectIds);
                        printf("\n\n");
                    }
                    else
                    {
                        printf("! Failed to get functional objects, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    printf("! Invalid functional category found\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
	//</SnippetCapabilities7>
}

// Display all content types contained in an IPortableDevicePropVariantCollection
// NOTE: These values are assumed to be in VT_CLSID VarType format.
void DisplayContentTypes(
    IPortableDevicePropVariantCollection* pContentTypes)
{
    if (pContentTypes == NULL)
    {
        printf("! A NULL IPortableDevicePropVariantCollection interface pointer was received\n");
        return;
    }

    HRESULT hr = S_OK;
    DWORD dwNumContentTypes = 0;

    // Get the total number of content types in the collection.
    hr = pContentTypes->GetCount(&dwNumContentTypes);
    if (SUCCEEDED(hr))
    {
        // Loop through the collection and displaying each content type found.
        // This loop prints a comma-separated list of the content types.
        for (DWORD dwContentTypeIndex = 0; dwContentTypeIndex < dwNumContentTypes; dwContentTypeIndex++)
        {
            PROPVARIANT pvContentType = {0};
            PropVariantInit(&pvContentType);
            hr = pContentTypes->GetAt(dwContentTypeIndex, &pvContentType);
            if (SUCCEEDED(hr))
            {
                // We have a content type.  It is assumed that
                // content types are returned as VT_CLSID
                // VarTypes.

                if ((pvContentType.puuid != NULL)        &&
                    (pvContentType.vt    == VT_CLSID))
                {
                    // Display the content types separated by commas
                    DisplayContentType(*pvContentType.puuid);

                    if ((dwContentTypeIndex + 1) < dwNumContentTypes)
                    {
                        printf(", ");
                    }
                }
                else
                {
                    printf("! Invalid content type found\n");
                }
            }

            PropVariantClear(&pvContentType);
        }
    }
}

// List all functional categories on the device
void ListFunctionalCategories(
    IPortableDevice* pDevice)
{
	//<SnippetCapabilities1>
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities>            pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pCategories;
    DWORD dwNumCategories = 0;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all functional categories supported by the device.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetFunctionalCategories(&pCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get functional categories from the device, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetCapabilities1>
    // Get the number of functional categories found on the device.
	//<SnippetCapabilities2>
    if (SUCCEEDED(hr))
    {
        hr = pCategories->GetCount(&dwNumCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get number of functional categories, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Functional Categories Found on the device\n\n", dwNumCategories);

    // Loop through each functional category and display its name
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumCategories; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pCategories->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID
                // VarTypes.

                if (pv.puuid != NULL)
                {
                    // Display the functional category name
                    DisplayFunctionalCategory(*pv.puuid);
                    printf("\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
	//</SnippetCapabilities2>
}

// List supported content types the device supports
void ListSupportedContentTypes(
    IPortableDevice* pDevice)
{
	//</SnippetCapabilities3>
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities>            pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pCategories;
    DWORD dwNumCategories   = 0;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to enumerate functional objects
    // that fall within them.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetFunctionalCategories(&pCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get functional categories from the device, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = pCategories->GetCount(&dwNumCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get number of functional categories, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Functional Categories Found on the device\n\n", dwNumCategories);

    // Loop through each functional category and display its name and supported content types.
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumCategories; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pCategories->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID
                // VarTypes.

                if ((pv.puuid != NULL)      &&
                    (pv.vt    == VT_CLSID))
                {
                    // Display the functional category name
                    printf("Functional Category: ");
                    DisplayFunctionalCategory(*pv.puuid);
                    printf("\n");

                    // Display the content types supported for this category
                    CComPtr<IPortableDevicePropVariantCollection> pContentTypes;
                    hr = pCapabilities->GetSupportedContentTypes(*pv.puuid, &pContentTypes);
                    if (SUCCEEDED(hr))
                    {
                        printf("Supported Content Types: ");
                        DisplayContentTypes(pContentTypes);
                        printf("\n\n");
                    }
                    else
                    {
                        printf("! Failed to get supported content types from the device, hr = 0x%lx\n",hr);
                    }
                }
                else
                {
                    printf("! Invalid functional category found\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
	//</SnippetCapabilities3>
}

// Determines if a device supports a particular functional category.
BOOL SupportsFunctionalCategory(
    IPortableDevice*    pDevice,
    REFGUID             guidCategory)
{
    CComPtr<IPortableDeviceCapabilities>            pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pCategories;
    BOOL  bSupported      = FALSE;
    DWORD dwNumCategories = 0;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    HRESULT hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to search for a particular functional category.
    // There is typically only 1 of these types of functional categories.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetFunctionalCategories(&pCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get functional categories from the device, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = pCategories->GetCount(&dwNumCategories);
        if (FAILED(hr))
        {
            printf("! Failed to get number of functional categories, hr = 0x%lx\n",hr);
        }
    }

    // Loop through each functional category and find the passed in category
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumCategories; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pCategories->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID
                // VarTypes.

                if ((pv.puuid != NULL)      &&
                    (pv.vt    == VT_CLSID))
                {
                    bSupported = IsEqualGUID(guidCategory, *pv.puuid);
                }
                else
                {
                    printf("! Invalid functional category found\n");
                }
            }

            PropVariantClear(&pv);

            // If the device supports the category, exit the for loop.
            // NOTE: We are exiting after calling PropVariantClear to make
            // sure we free any allocated data in the PROPVARIANT returned
            // from the GetAt() method call.
            if (bSupported == TRUE)
            {
                break;
            }
        }
    }

    return bSupported;
}

// Determines if a device supports a particular command.
BOOL SupportsCommand(
    IPortableDevice*    pDevice,
    REFPROPERTYKEY      keyCommand)
{
    CComPtr<IPortableDeviceCapabilities>    pCapabilities;
    CComPtr<IPortableDeviceKeyCollection>   pCommands;
    BOOL  bSupported      = FALSE;
    DWORD dwNumCommands   = 0;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    HRESULT hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all commands supported by the device.
    // We will use these commands to search for a particular functional category.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetSupportedCommands(&pCommands);
        if (FAILED(hr))
        {
            printf("! Failed to get supported commands from the device, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of supported commands found on the device.
    if (SUCCEEDED(hr))
    {
        hr = pCommands->GetCount(&dwNumCommands);
        if (FAILED(hr))
        {
            printf("! Failed to get number of supported commands, hr = 0x%lx\n",hr);
        }
    }

    // Loop through each functional category and find the passed in category
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumCommands; dwIndex++)
        {
            PROPERTYKEY key = WPD_PROPERTY_NULL;
            hr = pCommands->GetAt(dwIndex, &key);
            if (SUCCEEDED(hr))
            {
                bSupported = IsEqualPropertyKey(keyCommand, key);
            }

            // If the device supports the category, exit the for loop.
            if (bSupported == TRUE)
            {
                break;
            }
        }
    }

    return bSupported;
}

// Reads the WPD_RENDERING_INFORMATION_PROFILES properties on the device.
HRESULT ReadProfileInformationProperties(
    IPortableDevice*                       pDevice,
    PCWSTR                                 pszFunctionalObjectID,
    IPortableDeviceValuesCollection**      ppRenderingInfoProfiles)
{
    CComPtr<IPortableDeviceValuesCollection> pRenderingInfoProfiles;
    CComPtr<IPortableDeviceContent>          pContent;
    CComPtr<IPortableDeviceProperties>       pProperties;
    CComPtr<IPortableDeviceKeyCollection>    pPropertiesToRead;
    CComPtr<IPortableDeviceValues>           pObjectProperties;

    // Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    HRESULT hr = pDevice->Content(&pContent);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read WPD_RENDERING_INFORMATION_PROFILES)
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pPropertiesToRead));
    if (SUCCEEDED(hr))
    {
        // Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (pPropertiesToRead != NULL)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(WPD_RENDERING_INFORMATION_PROFILES);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_RENDERING_INFORMATION_PROFILES to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }
        }
    }

    // Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetValues(pszFunctionalObjectID, // The object whose properties we are reading
                                    pPropertiesToRead,     // The properties we want to read
                                    &pObjectProperties);   // Driver supplied property values for the specified object
        if (FAILED(hr))
        {
            printf("! Failed to get all properties for object '%ws', hr= 0x%lx\n", pszFunctionalObjectID, hr);
        }
    }

    // Read the WPD_RENDERING_INFORMATION_PROFILES
    if (SUCCEEDED(hr))
    {
        hr = pObjectProperties->GetIPortableDeviceValuesCollectionValue(WPD_RENDERING_INFORMATION_PROFILES,
                                                                        &pRenderingInfoProfiles);
        if (FAILED(hr))
        {
            printf("! Failed to get WPD_RENDERING_INFORMATION_PROFILES from rendering information, hr= 0x%lx\n",  hr);
        }
    }

    // QueryInterface the interface into the out-going parameters.
    if (SUCCEEDED(hr))
    {
        hr = pRenderingInfoProfiles->QueryInterface(IID_PPV_ARGS(ppRenderingInfoProfiles));
        if (FAILED(hr))
        {
            printf("! Failed to QueryInterface for IPortableDeviceValuesCollection (Rendering information profiles), hr= 0x%lx\n",  hr);
        }
    }

    return hr;
}

void DisplayExpectedValues(
    IPortableDeviceValues* pExpectedValues)
{
    // 1) Determine what type of valid values should be displayed by reading the
    //    WPD_PROPERTY_ATTRIBUTE_FORM property.
    DWORD dwAttributeForm = WPD_PROPERTY_ATTRIBUTE_FORM_UNSPECIFIED;
    HRESULT hr = pExpectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_FORM, &dwAttributeForm);
    if (FAILED(hr))
    {
        printf("! Failed to get WPD_PROPERTY_ATTRIBUTE_FORM from expected value set, hr = 0x%lx\n", hr);
    }

    // 2) Switch on the attribute form to determine what expected value properties to read.
    if (SUCCEEDED(hr))
    {
        switch(dwAttributeForm)
        {
            case WPD_PROPERTY_ATTRIBUTE_FORM_RANGE:
                {
                    DWORD dwMin  = 0;
                    DWORD dwMax  = 0;
                    DWORD dwStep = 0;

                    hr = pExpectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_MIN, &dwMin);
                    if (FAILED(hr))
                    {
                        printf("! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_MIN from expected values collection, hr = 0x%lx\n", hr);
                    }
                    hr = pExpectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_MAX, &dwMax);
                    if (FAILED(hr))
                    {
                        printf("! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_MAX from expected values collection, hr = 0x%lx\n", hr);
                    }
                    hr = pExpectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_STEP, &dwStep);
                    if (FAILED(hr))
                    {
                        printf("! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_STEP from expected values collection, hr = 0x%lx\n", hr);
                    }

                    printf("MIN: %d, MAX: %d, STEP: %d\n", dwMin, dwMax, dwStep);
                }
                break;
            default:
                printf("* DisplayExpectedValues helper function did not display attributes for form %d", dwAttributeForm);
                break;
        }
    }
}

// Displays a rendering profile.
void DisplayRenderingProfile(
    IPortableDeviceValues* pProfile)
{
    DWORD dwTotalBitrate    = 0;
    DWORD dwChannelCount    = 0;
    DWORD dwAudioFormatCode = 0;
    GUID  guidFormat        = WPD_OBJECT_FORMAT_UNSPECIFIED;

    // Display WPD_MEDIA_TOTAL_BITRATE
    HRESULT hr = pProfile->GetUnsignedIntegerValue(WPD_MEDIA_TOTAL_BITRATE, &dwTotalBitrate);
    if (SUCCEEDED(hr))
    {
        printf("Total Bitrate: %d\n", dwTotalBitrate);
    }

    // If we fail to read the total bitrate as a single value, then it must be
    // a valid value set.  (i.e. returning IPortableDeviceValues as the value which
    // contains properties describing the valid values for this property.)
    if (hr == DISP_E_TYPEMISMATCH)
    {
        CComPtr<IPortableDeviceValues> pExpectedValues;
        hr = pProfile->GetIPortableDeviceValuesValue(WPD_MEDIA_TOTAL_BITRATE, &pExpectedValues);
        if (SUCCEEDED(hr))
        {
            printf("Total Bitrate: ");
            DisplayExpectedValues(pExpectedValues);
        }
    }

    // If we are still a failure here, report the error
    if (FAILED(hr))
    {

        printf("! Failed to get WPD_MEDIA_TOTAL_BITRATE from rendering profile, hr = 0x%lx\n",hr);
    }

    // Display WPD_AUDIO_CHANNEL_COUNT
    hr = pProfile->GetUnsignedIntegerValue(WPD_AUDIO_CHANNEL_COUNT, &dwChannelCount);
    if (SUCCEEDED(hr))
    {
        printf("Channel Count: %d\n", dwChannelCount);
    }
    else
    {
        printf("! Failed to get WPD_AUDIO_CHANNEL_COUNT from rendering profile, hr = 0x%lx\n",hr);
    }

    // Display WPD_AUDIO_FORMAT_CODE
    hr = pProfile->GetUnsignedIntegerValue(WPD_AUDIO_FORMAT_CODE,   &dwAudioFormatCode);
    if (SUCCEEDED(hr))
    {
        printf("Audio Format Code: %d\n", dwAudioFormatCode);
    }
    else
    {
        printf("! Failed to get WPD_AUDIO_FORMAT_CODE from rendering profile, hr = 0x%lx\n",hr);
    }

    // Display WPD_OBJECT_FORMAT
    hr = pProfile->GetGuidValue(WPD_OBJECT_FORMAT, &guidFormat);
    if (SUCCEEDED(hr))
    {
        printf("Object Format: %ws\n", (PWSTR)CGuidToString(guidFormat));
    }
    else
    {
        printf("! Failed to get WPD_OBJECT_FORMAT from rendering profile, hr = 0x%lx\n",hr);
    }
}

// List rendering capabilities the device supports
void ListRenderingCapabilityInformation(
    IPortableDevice* pDevice)
{
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities>          pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection> pRenderingInfoObjects;
    CComPtr<IPortableDeviceValuesCollection>      pRenderingInfoProfiles;
    CAtlStringW                                   strRenderingInfoObjectID;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    if (SupportsFunctionalCategory(pDevice, WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION) == FALSE)
    {
        printf("This device does not support device rendering information to display\n");
        return;
    }

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get the functional object identifier for the rendering information object
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetFunctionalObjects(WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION, &pRenderingInfoObjects);
        if (FAILED(hr))
        {
            printf("! Failed to get functional objects, hr = 0x%lx\n", hr);
        }
    }

    // Assume the device only has one rendering information object for this example.
    // We are going to request the first Object Identifier found in the collection.
    if (SUCCEEDED(hr))
    {
        PROPVARIANT pv = {0};
        PropVariantInit(&pv);
        hr = pRenderingInfoObjects->GetAt(0, &pv);
        if ((SUCCEEDED(hr))    &&
            (pv.vt== VT_LPWSTR) )
        {
            strRenderingInfoObjectID = pv.pwszVal;
        }
        else
        {
            printf("! Failed to get first rendering object's identifier, hr = 0x%lx\n", hr);
        }

        PropVariantClear(&pv);
    }

    if (SUCCEEDED(hr))
    {
        hr = ReadProfileInformationProperties(pDevice,
                                              strRenderingInfoObjectID,
                                              &pRenderingInfoProfiles);
        // Error output statements are performed by the helper function, so they
        // are omitted here.
    }

    // Display all rendering profiles
    if (SUCCEEDED(hr))
    {
        // Get the number of profiles supported by the device
        DWORD dwNumProfiles = 0;
        hr = pRenderingInfoProfiles->GetCount(&dwNumProfiles);
        if (FAILED(hr))
        {
            printf("! Failed to get number of profiles supported by the device, hr = 0x%lx\n",hr);
        }

        printf("%d Rendering Profiles are supported by this device\n",dwNumProfiles);

        if (SUCCEEDED(hr))
        {
            for (DWORD dwIndex = 0; dwIndex < dwNumProfiles; dwIndex++)
            {
                CComPtr<IPortableDeviceValues> pProfile;
                hr = pRenderingInfoProfiles->GetAt(dwIndex, &pProfile);
                if (SUCCEEDED(hr))
                {
                    printf("\nProfile #%d:\n", dwIndex);
                    DisplayRenderingProfile(pProfile);
                    printf("\n\n");
                }
                else
                {
                    printf("! Failed to get rendering profile at index '%d', hr = 0x%lx\n", dwIndex, hr);
                }
            }
        }
    }
}

// List all supported events on the device
void ListSupportedEvents(
    IPortableDevice* pDevice)
{
	//<SnippetCapabilities4>
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities>            pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pEvents;
    DWORD dwNumEvents = 0;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }
	//</SnippetCapabilities4>

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = pDevice->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // Get all events supported by the device.
	//<SnippetCapabilities5>
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetSupportedEvents(&pEvents);
        if (FAILED(hr))
        {
            printf("! Failed to get supported events from the device, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetCapabilities5>
    // Get the number of supported events found on the device.
	//<SnippetCapabilities6>
    if (SUCCEEDED(hr))
    {
        hr = pEvents->GetCount(&dwNumEvents);
        if (FAILED(hr))
        {
            printf("! Failed to get number of supported events, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Supported Events Found on the device\n\n", dwNumEvents);

    // Loop through each event and display its name
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumEvents; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pEvents->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have an event.  It is assumed that
                // events are returned as VT_CLSID VarTypes.

                if (pv.puuid != NULL)
                {
                    // Display the event name
                    DisplayEvent(*pv.puuid);
                    printf("\n");
                    // Display the event options
                    DisplayEventOptions(pCapabilities, *pv.puuid);
                    printf("\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
	//</SnippetCapabilities6>
}

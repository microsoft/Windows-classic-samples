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
    _In_ REFGUID functionalCategory)
{
    if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_STORAGE, functionalCategory))
    {
        wprintf(L"WPD_FUNCTIONAL_CATEGORY_STORAGE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_STILL_IMAGE_CAPTURE, functionalCategory))
    {
        wprintf(L"WPD_FUNCTIONAL_CATEGORY_STILL_IMAGE_CAPTURE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_AUDIO_CAPTURE, functionalCategory))
    {
        wprintf(L"WPD_FUNCTIONAL_CATEGORY_AUDIO_CAPTURE");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_SMS, functionalCategory))
    {
        wprintf(L"WPD_FUNCTIONAL_CATEGORY_SMS");
    }
    else if (IsEqualGUID(WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION, functionalCategory))
    {
        wprintf(L"WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION");
    }
    else
    {
        wprintf(L"%ws", (PCWSTR)CGuidToString(functionalCategory));
    }
}

// Displays a friendly name for a passed in event
// If the event is not known by this function
// the GUID will be displayed in string form.
void DisplayEvent(
    _In_ REFGUID event)
{
    if (IsEqualGUID(WPD_EVENT_OBJECT_ADDED, event))
    {
        wprintf(L"WPD_EVENT_OBJECT_ADDED");
    }
    else if (IsEqualGUID(WPD_EVENT_OBJECT_REMOVED, event))
    {
        wprintf(L"WPD_EVENT_OBJECT_REMOVED");
    }
    else if (IsEqualGUID(WPD_EVENT_OBJECT_UPDATED, event))
    {
        wprintf(L"WPD_EVENT_OBJECT_UPDATED");
    }
    else if (IsEqualGUID(WPD_EVENT_DEVICE_RESET, event))
    {
        wprintf(L"WPD_EVENT_DEVICE_RESET");
    }
    else if (IsEqualGUID(WPD_EVENT_DEVICE_CAPABILITIES_UPDATED, event))
    {
        wprintf(L"WPD_EVENT_DEVICE_CAPABILITIES_UPDATED");
    }
    else if (IsEqualGUID(WPD_EVENT_STORAGE_FORMAT, event))
    {
        wprintf(L"WPD_EVENT_STORAGE_FORMAT");
    }
    else
    {
        wprintf(L"%ws", (PCWSTR)CGuidToString(event));
    }
}

// Displays a friendly name for a passed in content type
// If the content type is not known by this function
// the GUID will be displayed in string form.
void DisplayContentType(
    _In_ REFGUID contentType)
{
    if (IsEqualGUID(WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_FOLDER, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_FOLDER");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_IMAGE, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_IMAGE");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_DOCUMENT, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_DOCUMENT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_CONTACT, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_CONTACT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_AUDIO, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_AUDIO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_VIDEO, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_VIDEO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_TASK, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_TASK");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_APPOINTMENT, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_APPOINTMENT");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_EMAIL, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_EMAIL");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_MEMO, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_MEMO");
    }
    else if (IsEqualGUID(WPD_CONTENT_TYPE_UNSPECIFIED, contentType))
    {
        wprintf(L"WPD_CONTENT_TYPE_UNSPECIFIED");
    }
    else
    {
        wprintf(L"%ws", (PCWSTR)CGuidToString(contentType));
    }
}

// Display the basic event options for the passed in event.
void DisplayEventOptions(
    _In_ IPortableDeviceCapabilities* capabilities,
    _In_ REFGUID                      event)
{
    ComPtr<IPortableDeviceValues> eventOptions;
    HRESULT hr = capabilities->GetEventOptions(event, &eventOptions);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get even options, hr = 0x%lx\n", hr);
    }
    else
    {
        wprintf(L"Event Options:\n");
        // Read the WPD_EVENT_OPTION_IS_BROADCAST_EVENT value to see if the event is
        // a broadcast event. If the read fails, assume FALSE
        BOOL  isBroadcastEvent = FALSE;
        hr = eventOptions->GetBoolValue(WPD_EVENT_OPTION_IS_BROADCAST_EVENT, &isBroadcastEvent);
        if (SUCCEEDED(hr))
        {
            wprintf(L"\tWPD_EVENT_OPTION_IS_BROADCAST_EVENT = %ws\n", isBroadcastEvent ? L"TRUE" : L"FALSE");
        }
        else
        {
            wprintf(L"! Failed to get WPD_EVENT_OPTION_IS_BROADCAST_EVENT (assuming FALSE), hr = 0x%lx\n", hr);
        }
    }
}

// Display all functional object identifiers contained in an IPortableDevicePropVariantCollection
// NOTE: These values are assumed to be in VT_LPWSTR VarType format.
void DisplayFunctionalObjectIDs(
    _In_ IPortableDevicePropVariantCollection* functionalObjectIDs)
{
    DWORD   numObjectIDs = 0;
    // Get the total number of object identifiers in the collection.
    HRESULT hr           = functionalObjectIDs->GetCount(&numObjectIDs);
    if (SUCCEEDED(hr))
    {
        // Loop through the collection and displaying each object identifier found.
        // This loop prints a comma-separated list of the object identifiers.
        for (DWORD objectIDIndex = 0; objectIDIndex < numObjectIDs; objectIDIndex++)
        {
            PROPVARIANT objectID = {0};
            hr = functionalObjectIDs->GetAt(objectIDIndex, &objectID);
            if (SUCCEEDED(hr))
            {
                // We have a functional object identifier.  It is assumed that
                // object identifiers are returned as VT_LPWSTR varTypes.
                if (objectID.vt      == VT_LPWSTR &&
                    objectID.pwszVal != nullptr)
                {
                    // Display the object identifiers separated by commas
                    wprintf(L"%ws", objectID.pwszVal);
                    if ((objectIDIndex + 1) < numObjectIDs)
                    {
                        wprintf(L", ");
                    }
                }
                else
                {
                    wprintf(L"! Invalid functional object identifier found\n");
                }
            }

            PropVariantClear(&objectID);
        }
    }
}

// List all functional objects on the device
void ListFunctionalObjects(
    _In_ IPortableDevice* device)
{
    //<SnippetCapabilities7>
    ComPtr<IPortableDeviceCapabilities>            capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   functionalCategories;
    DWORD   numFunctionalCategories = 0;
    HRESULT hr                      = S_OK;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to enumerate functional objects
    // that fall within them.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetFunctionalCategories(&functionalCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get functional categories from the device, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = functionalCategories->GetCount(&numFunctionalCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of functional categories, hr = 0x%lx\n", hr);
        }
    }

    wprintf(L"\n%u Functional Categories Found on the device\n\n", numFunctionalCategories);

    // Loop through each functional category and get the list of
    // functional object identifiers associated with a particular
    // category.
    if (SUCCEEDED(hr))
    {
        for (DWORD index = 0; index < numFunctionalCategories; index++)
        {
            PROPVARIANT pv = {0};
            hr = functionalCategories->GetAt(index, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID varTypes.
                if (pv.vt    == VT_CLSID &&
                    pv.puuid != nullptr)
                {
                    // Display the functional category name
                    wprintf(L"Functional Category: ");
                    DisplayFunctionalCategory(*pv.puuid);
                    wprintf(L"\n");

                    // Display the object identifiers for all
                    // functional objects within this category
                    ComPtr<IPortableDevicePropVariantCollection> functionalObjectIDs;
                    hr = capabilities->GetFunctionalObjects(*pv.puuid, &functionalObjectIDs);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"Functional Objects: ");
                        DisplayFunctionalObjectIDs(functionalObjectIDs.Get());
                        wprintf(L"\n\n");
                    }
                    else
                    {
                        wprintf(L"! Failed to get functional objects, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    wprintf(L"! Invalid functional category found\n");
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
    _In_ IPortableDevicePropVariantCollection* contentTypes)
{
    if (contentTypes == nullptr)
    {
        wprintf(L"! A nullptr IPortableDevicePropVariantCollection interface pointer was received\n");
        return;
    }

    // Get the total number of content types in the collection.
    DWORD   numContentTypes = 0;
    HRESULT hr              = contentTypes->GetCount(&numContentTypes);
    if (SUCCEEDED(hr))
    {
        // Loop through the collection and displaying each content type found.
        // This loop prints a comma-separated list of the content types.
        for (DWORD contentTypeIndex = 0; contentTypeIndex < numContentTypes; contentTypeIndex++)
        {
            PROPVARIANT contentType = {0};
            hr = contentTypes->GetAt(contentTypeIndex, &contentType);
            if (SUCCEEDED(hr))
            {
                // We have a content type.  It is assumed that
                // content types are returned as VT_CLSID varTypes.
                if (contentType.vt    == VT_CLSID &&
                    contentType.puuid != nullptr)
                {
                    // Display the content types separated by commas
                    DisplayContentType(*contentType.puuid);

                    if ((contentTypeIndex + 1) < numContentTypes)
                    {
                        wprintf(L", ");
                    }
                }
                else
                {
                    wprintf(L"! Invalid content type found\n");
                }
            }

            PropVariantClear(&contentType);
        }
    }
}

// List all functional categories on the device
void ListFunctionalCategories(
    _In_ IPortableDevice* device)
{
    //<SnippetCapabilities1>
    ComPtr<IPortableDeviceCapabilities>            capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   functionalCategories;
    DWORD   numCategories   = 0;
    HRESULT hr              = S_OK;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get all functional categories supported by the device.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetFunctionalCategories(&functionalCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get functional categories from the device, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetCapabilities1>
    // Get the number of functional categories found on the device.
    //<SnippetCapabilities2>
    if (SUCCEEDED(hr))
    {
        hr = functionalCategories->GetCount(&numCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of functional categories, hr = 0x%lx\n", hr);
        }
    }

    wprintf(L"\n%u Functional Categories Found on the device\n\n", numCategories);

    // Loop through each functional category and display its name
    if (SUCCEEDED(hr))
    {
        for (DWORD index = 0; index < numCategories; index++)
        {
            PROPVARIANT pv = {0};
            hr = functionalCategories->GetAt(index, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID varTypes.
                if (pv.vt    == VT_CLSID &&
                    pv.puuid != nullptr)
                {
                    // Display the functional category name
                    DisplayFunctionalCategory(*pv.puuid);
                    wprintf(L"\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
    //</SnippetCapabilities2>
}

// List supported content types the device supports
void ListSupportedContentTypes(
    _In_ IPortableDevice* device)
{
    //<SnippetCapabilities3>
    ComPtr<IPortableDeviceCapabilities>            capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   functionalCategories;
    DWORD   numCategories = 0;
    HRESULT hr            = S_OK;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to enumerate functional objects
    // that fall within them.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetFunctionalCategories(&functionalCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get functional categories from the device, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = functionalCategories->GetCount(&numCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of functional categories, hr = 0x%lx\n", hr);
        }
    }

    wprintf(L"\n%u Functional Categories Found on the device\n\n", numCategories);

    // Loop through each functional category and display its name and supported content types.
    if (SUCCEEDED(hr))
    {
        for (DWORD index = 0; index < numCategories; index++)
        {
            PROPVARIANT pv = {0};
            hr = functionalCategories->GetAt(index, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID varTypes.
                if (pv.vt    == VT_CLSID &&
                    pv.puuid != nullptr)
                {
                    // Display the functional category name
                    wprintf(L"Functional Category: ");
                    DisplayFunctionalCategory(*pv.puuid);
                    wprintf(L"\n");

                    // Display the content types supported for this category
                    ComPtr<IPortableDevicePropVariantCollection> contentTypes;
                    hr = capabilities->GetSupportedContentTypes(*pv.puuid, &contentTypes);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"Supported Content Types: ");
                        DisplayContentTypes(contentTypes.Get());
                        wprintf(L"\n\n");
                    }
                    else
                    {
                        wprintf(L"! Failed to get supported content types from the device, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    wprintf(L"! Invalid functional category found\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
    //</SnippetCapabilities3>
}

// Determines if a device supports a particular functional category.
BOOL SupportsFunctionalCategory(
    _In_ IPortableDevice*    device,
    _In_ REFGUID             functionalCategory)
{
    ComPtr<IPortableDeviceCapabilities>            capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   functionalCategories;
    BOOL  isSupported   = FALSE;
    DWORD numCategories = 0;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    HRESULT hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get all functional categories supported by the device.
    // We will use these categories to search for a particular functional category.
    // There is typically only 1 of these types of functional categories.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetFunctionalCategories(&functionalCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get functional categories from the device, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of functional categories found on the device.
    if (SUCCEEDED(hr))
    {
        hr = functionalCategories->GetCount(&numCategories);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of functional categories, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each functional category and find the passed in category
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < numCategories; dwIndex++)
        {
            PROPVARIANT pv = {0};
            hr = functionalCategories->GetAt(dwIndex, &pv);
            if (SUCCEEDED(hr))
            {
                // We have a functional category.  It is assumed that
                // functional categories are returned as VT_CLSID varTypes.
                if (pv.vt    == VT_CLSID &&
                    pv.puuid != nullptr)
                {
                    isSupported = IsEqualGUID(functionalCategory, *pv.puuid);
                }
                else
                {
                    wprintf(L"! Invalid functional category found\n");
                }
            }

            PropVariantClear(&pv);

            // If the device supports the category, exit the for loop.
            // NOTE: We are exiting after calling PropVariantClear to make
            // sure we free any allocated data in the PROPVARIANT returned
            // from the GetAt() method call.
            if (isSupported == TRUE)
            {
                break;
            }
        }
    }

    return isSupported;
}

// Determines if a device supports a particular command.
BOOL SupportsCommand(
    _In_ IPortableDevice*    device,
    _In_ REFPROPERTYKEY      command)
{
    ComPtr<IPortableDeviceCapabilities>    capabilities;
    ComPtr<IPortableDeviceKeyCollection>   commands;
    BOOL  isSupported   = FALSE;
    DWORD numCommands   = 0;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    HRESULT hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get all commands supported by the device.
    // We will use these commands to search for a particular functional category.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetSupportedCommands(&commands);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get supported commands from the device, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of supported commands found on the device.
    if (SUCCEEDED(hr))
    {
        hr = commands->GetCount(&numCommands);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of supported commands, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each functional category and find the passed in category
    if (SUCCEEDED(hr))
    {
        for (DWORD index = 0; index < numCommands; index++)
        {
            PROPERTYKEY key = WPD_PROPERTY_NULL;
            hr = commands->GetAt(index, &key);
            if (SUCCEEDED(hr))
            {
                isSupported = IsEqualPropertyKey(command, key);
            }

            // If the device supports the category, exit the for loop.
            if (isSupported == TRUE)
            {
                break;
            }
        }
    }

    return isSupported;
}

// Reads the WPD_RENDERING_INFORMATION_PROFILES properties on the device.
HRESULT ReadProfileInformationProperties(
    _In_         IPortableDevice*                  device,
    _In_         PCWSTR                            functionalObjectID,
    _COM_Outptr_ IPortableDeviceValuesCollection** renderingInfoProfiles)
{
    *renderingInfoProfiles = nullptr;
    ComPtr<IPortableDeviceValuesCollection> renderingInfoProfilesTemp;
    ComPtr<IPortableDeviceContent>          content;
    ComPtr<IPortableDeviceProperties>       properties;
    ComPtr<IPortableDeviceKeyCollection>    propertiesToRead;
    ComPtr<IPortableDeviceValues>           objectProperties;

    // Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    HRESULT hr = device->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read WPD_RENDERING_INFORMATION_PROFILES)
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&propertiesToRead));
    if (SUCCEEDED(hr))
    {
        // Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        HRESULT tempHr = S_OK;
        tempHr = propertiesToRead->Add(WPD_RENDERING_INFORMATION_PROFILES);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_RENDERING_INFORMATION_PROFILES to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }
    }

    // Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = properties->GetValues(functionalObjectID,        // The object whose properties we are reading
                                   propertiesToRead.Get(),    // The properties we want to read
                                   &objectProperties);        // Driver supplied property values for the specified object
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get all properties for object '%ws', hr= 0x%lx\n", functionalObjectID, hr);
        }
    }

    // Read the WPD_RENDERING_INFORMATION_PROFILES
    if (SUCCEEDED(hr))
    {
        hr = objectProperties->GetIPortableDeviceValuesCollectionValue(WPD_RENDERING_INFORMATION_PROFILES,
                                                                       &renderingInfoProfilesTemp);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get WPD_RENDERING_INFORMATION_PROFILES from rendering information, hr= 0x%lx\n", hr);
        }
    }

    // QueryInterface the interface into the out-going parameters.
    if (SUCCEEDED(hr))
    {
        *renderingInfoProfiles = renderingInfoProfilesTemp.Detach();
    }

    return hr;
}

void DisplayExpectedValues(
    _In_ IPortableDeviceValues* expectedValues)
{
    // 1) Determine what type of valid values should be displayed by reading the
    //    WPD_PROPERTY_ATTRIBUTE_FORM property.
    DWORD formAttribute = WPD_PROPERTY_ATTRIBUTE_FORM_UNSPECIFIED;
    HRESULT hr = expectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_FORM, &formAttribute);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get WPD_PROPERTY_ATTRIBUTE_FORM from expected value set, hr = 0x%lx\n", hr);
    }

    // 2) Switch on the attribute form to determine what expected value properties to read.
    if (SUCCEEDED(hr))
    {
        switch(formAttribute)
        {
            case WPD_PROPERTY_ATTRIBUTE_FORM_RANGE:
                {
                    DWORD rangeMin  = 0;
                    DWORD rangeMax  = 0;
                    DWORD rangeStep = 0;

                    hr = expectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_MIN, &rangeMin);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_MIN from expected values collection, hr = 0x%lx\n", hr);
                    }
                    hr = expectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_MAX, &rangeMax);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_MAX from expected values collection, hr = 0x%lx\n", hr);
                    }
                    hr = expectedValues->GetUnsignedIntegerValue(WPD_PROPERTY_ATTRIBUTE_RANGE_STEP, &rangeStep);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to get WPD_PROPERTY_ATTRIBUTE_RANGE_STEP from expected values collection, hr = 0x%lx\n", hr);
                    }

                    wprintf(L"MIN: %u, MAX: %u, STEP: %u\n", rangeMin, rangeMax, rangeStep);
                }
                break;
            default:
                wprintf(L"* DisplayExpectedValues helper function did not display attributes for form %u", formAttribute);
                break;
        }
    }
}

// Displays a rendering profile.
void DisplayRenderingProfile(
    _In_ IPortableDeviceValues* profile)
{
    DWORD totalBitrate    = 0;
    DWORD channelCount    = 0;
    DWORD audioFormatCode = 0;
    GUID  objectFormat    = WPD_OBJECT_FORMAT_UNSPECIFIED;

    // Display WPD_MEDIA_TOTAL_BITRATE
    HRESULT hr = profile->GetUnsignedIntegerValue(WPD_MEDIA_TOTAL_BITRATE, &totalBitrate);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Total Bitrate: %u\n", totalBitrate);
    }

    // If we fail to read the total bitrate as a single value, then it must be
    // a valid value set.  (i.e. returning IPortableDeviceValues as the value which
    // contains properties describing the valid values for this property.)
    if (hr == DISP_E_TYPEMISMATCH)
    {
        ComPtr<IPortableDeviceValues> expectedValues;
        hr = profile->GetIPortableDeviceValuesValue(WPD_MEDIA_TOTAL_BITRATE, &expectedValues);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Total Bitrate: ");
            DisplayExpectedValues(expectedValues.Get());
        }
    }

    // If we are still a failure here, report the error
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get WPD_MEDIA_TOTAL_BITRATE from rendering profile, hr = 0x%lx\n", hr);
    }

    // Display WPD_AUDIO_CHANNEL_COUNT
    hr = profile->GetUnsignedIntegerValue(WPD_AUDIO_CHANNEL_COUNT, &channelCount);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Channel Count: %u\n", channelCount);
    }
    else
    {
        wprintf(L"! Failed to get WPD_AUDIO_CHANNEL_COUNT from rendering profile, hr = 0x%lx\n", hr);
    }

    // Display WPD_AUDIO_FORMAT_CODE
    hr = profile->GetUnsignedIntegerValue(WPD_AUDIO_FORMAT_CODE, &audioFormatCode);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Audio Format Code: %u\n", audioFormatCode);
    }
    else
    {
        wprintf(L"! Failed to get WPD_AUDIO_FORMAT_CODE from rendering profile, hr = 0x%lx\n", hr);
    }

    // Display WPD_OBJECT_FORMAT
    hr = profile->GetGuidValue(WPD_OBJECT_FORMAT, &objectFormat);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Object Format: %ws\n", (PCWSTR)(PCWSTR)CGuidToString(objectFormat));
    }
    else
    {
        wprintf(L"! Failed to get WPD_OBJECT_FORMAT from rendering profile, hr = 0x%lx\n", hr);
    }
}

// List rendering capabilities the device supports
void ListRenderingCapabilityInformation(
    _In_ IPortableDevice* device)
{
    HRESULT hr = S_OK;
    ComPtr<IPortableDeviceCapabilities>          capabilities;
    ComPtr<IPortableDevicePropVariantCollection> renderingInfoObjects;
    ComPtr<IPortableDeviceValuesCollection>      renderingInfoProfiles;

    if (SupportsFunctionalCategory(device, WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION) == FALSE)
    {
        wprintf(L"This device does not support device rendering information to display\n");
        return;
    }

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // Get the functional object identifier for the rendering information object
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetFunctionalObjects(WPD_FUNCTIONAL_CATEGORY_RENDERING_INFORMATION, &renderingInfoObjects);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get functional objects, hr = 0x%lx\n", hr);
        }
    }

    // Assume the device only has one rendering information object for this example.
    // We are going to request the first Object Identifier found in the collection.
    if (SUCCEEDED(hr))
    {
        PROPVARIANT pv = {0};
        hr = renderingInfoObjects->GetAt(0, &pv);
        if (SUCCEEDED(hr)           &&
            pv.vt      == VT_LPWSTR &&
            pv.pwszVal != nullptr)
        {
            hr = ReadProfileInformationProperties(device,
                                                  pv.pwszVal,
                                                  &renderingInfoProfiles);
            // Error output statements are performed by the helper function, so they
            // are omitted here.

            // Display all rendering profiles
            if (SUCCEEDED(hr))
            {
                // Get the number of profiles supported by the device
                DWORD numProfiles = 0;
                hr = renderingInfoProfiles->GetCount(&numProfiles);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to get number of profiles supported by the device, hr = 0x%lx\n", hr);
                }

                wprintf(L"%u Rendering Profiles are supported by this device\n", numProfiles);

                if (SUCCEEDED(hr))
                {
                    for (DWORD index = 0; index < numProfiles; index++)
                    {
                        ComPtr<IPortableDeviceValues> profile;
                        hr = renderingInfoProfiles->GetAt(index, &profile);
                        if (SUCCEEDED(hr))
                        {
                            wprintf(L"\nProfile #%u:\n", index);
                            DisplayRenderingProfile(profile.Get());
                            wprintf(L"\n\n");
                        }
                        else
                        {
                            wprintf(L"! Failed to get rendering profile at index '%u', hr = 0x%lx\n", index, hr);
                        }
                    }
                }
            }
        }
        else
        {
            wprintf(L"! Failed to get first rendering object's identifier, hr = 0x%lx\n", hr);
        }

        PropVariantClear(&pv);
    }
}

// List all supported events on the device
void ListSupportedEvents(
    _In_ IPortableDevice* device)
{
    //<SnippetCapabilities4>
    ComPtr<IPortableDeviceCapabilities>            capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   events;
    DWORD   numEvents = 0;
    HRESULT hr        = S_OK;

    // Get an IPortableDeviceCapabilities interface from the IPortableDevice interface to
    // access the device capabilities-specific methods.
    hr = device->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceCapabilities from IPortableDevice, hr = 0x%lx\n", hr);
    }
    //</SnippetCapabilities4>
    // Get all events supported by the device.
    //<SnippetCapabilities5>
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetSupportedEvents(&events);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get supported events from the device, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetCapabilities5>
    // Get the number of supported events found on the device.
    //<SnippetCapabilities6>
    if (SUCCEEDED(hr))
    {
        hr = events->GetCount(&numEvents);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of supported events, hr = 0x%lx\n", hr);
        }
    }

    wprintf(L"\n%u Supported Events Found on the device\n\n", numEvents);

    // Loop through each event and display its name
    if (SUCCEEDED(hr))
    {
        for (DWORD index = 0; index < numEvents; index++)
        {
            PROPVARIANT pv = {0};
            hr = events->GetAt(index, &pv);
            if (SUCCEEDED(hr))
            {
                // We have an event.  It is assumed that
                // events are returned as VT_CLSID varTypes.
                if (pv.vt    == VT_CLSID &&
                    pv.puuid != nullptr)
                {
                    // Display the event name
                    DisplayEvent(*pv.puuid);
                    wprintf(L"\n");
                    // Display the event options
                    DisplayEventOptions(capabilities.Get(), *pv.puuid);
                    wprintf(L"\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
    //</SnippetCapabilities6>
}

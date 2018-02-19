// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

// Displays a property assumed to be in error code form.
void DisplayErrorResultProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName)
{
    HRESULT error = S_OK;

    HRESULT hr = properties->GetErrorValue(key, &error);
    if (SUCCEEDED(hr))
    {
        wprintf(L"%ws: HRESULT = (0x%lx)\n", keyName, error);
    }
    else
    {
        wprintf(L"%ws: <Not Found>\n", keyName);
    }
}


// Displays a property assumed to be in string form.
void DisplayStringProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName)
{
    PWSTR   value = nullptr;

    HRESULT hr = properties->GetStringValue(key, &value);
    if (SUCCEEDED(hr))
    {
        // Get the length of the string value so we
        // can output <empty string value> if one
        // is encountered.
        if (wcsnlen(value, MAX_PATH) > 0)
        {
            wprintf(L"%ws: %ws\n", keyName, value);
        }
        else
        {
            wprintf(L"%ws: <empty string value>\n", keyName);
        }
    }
    else
    {
        wprintf(L"%ws: <Not Found>\n", keyName);
    }

    // Free the allocated string returned from the
    // GetStringValue method
    CoTaskMemFree(value);
    value = nullptr;
}


// Displays a property assumed to be in GUID form.
void DisplayGuidProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName)
{
    GUID    value = GUID_NULL;

    HRESULT hr = properties->GetGuidValue(key, &value);
    if (SUCCEEDED(hr))
    {
        wprintf(L"%ws: %ws\n", keyName, (PCWSTR)CGuidToString(value));
    }
    else
    {
        wprintf(L"%ws: <Not Found>\n", keyName);
    }
}

//<SnippetReadContentProperties1>
// Reads properties for the user specified object.
void ReadContentProperties(
    _In_ IPortableDeviceService*    service)
{
    WCHAR                                 selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceProperties>     properties;
    ComPtr<IPortableDeviceValues>         objectProperties;
    ComPtr<IPortableDeviceContent2>       content;
    ComPtr<IPortableDeviceKeyCollection>  propertiesToRead;

    // Prompt user to enter an object identifier on the device to read properties from.
    wprintf(L"Enter the identifier of the object you wish to read properties from.\n>");
    HRESULT hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting property reading\n");
    }

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDeviceService interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = service->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent2 from IPortableDeviceService, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent2 interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n", hr);
        }
    }

    // 3) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&propertiesToRead));
    if (SUCCEEDED(hr))
    {
        // 4) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        HRESULT tempHr = S_OK;
        tempHr = propertiesToRead->Add(PKEY_GenericObj_ParentID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add PKEY_GenericObj_ParentID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(PKEY_GenericObj_Name);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add PKEY_GenericObj_Name to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(PKEY_GenericObj_PersistentUID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add PKEY_GenericObj_PersistentUID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(PKEY_GenericObj_ObjectFormat);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add PKEY_GenericObj_ObjectFormat to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }
    }

    // 5) Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = properties->GetValues(selection,               // The object whose properties we are reading
                                   propertiesToRead.Get(),  // The properties we want to read
                                   &objectProperties);      // Driver supplied property values for the specified object
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get all properties for object '%ws', hr= 0x%lx\n", selection, hr);
        }
    }

    // 6) Display the returned property values to the user
    if (SUCCEEDED(hr))
    {
        DisplayStringProperty(objectProperties.Get(), PKEY_GenericObj_ParentID,        NAME_GenericObj_ParentID);
        DisplayStringProperty(objectProperties.Get(), PKEY_GenericObj_Name,            NAME_GenericObj_Name);
        DisplayStringProperty(objectProperties.Get(), PKEY_GenericObj_PersistentUID,   NAME_GenericObj_PersistentUID);
        DisplayGuidProperty  (objectProperties.Get(), PKEY_GenericObj_ObjectFormat,    NAME_GenericObj_ObjectFormat);
    }
}
//</SnippetReadContentProperties1>

//<SnippetWriteContentProperties1>
// Writes properties on the user specified object.
void WriteContentProperties(
    _In_ IPortableDeviceService*    service)
{
    BOOL                                  canWrite = FALSE;
    WCHAR                                 selection[SELECTION_BUFFER_SIZE]      = {0};
    WCHAR                                 newObjectName[SELECTION_BUFFER_SIZE]  = {0};
    ComPtr<IPortableDeviceProperties>     properties;
    ComPtr<IPortableDeviceContent2>       content;
    ComPtr<IPortableDeviceValues>         objectPropertiesToWrite;
    ComPtr<IPortableDeviceValues>         propertyWriteResults;
    ComPtr<IPortableDeviceValues>         attributes;

    // Prompt user to enter an object identifier on the device to write properties on.
    wprintf(L"Enter the identifier of the object you wish to write properties on.\n>");
    HRESULT hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting property reading\n");
    }

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDeviceService interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = service->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent2 from IPortableDeviceService, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent2 interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n", hr);
        }
    }

    // 3) Check the property attributes to see if we can write/change the NAME_GenericObj_Name property.
    if (SUCCEEDED(hr))
    {
        hr = properties->GetPropertyAttributes(selection,
                                               PKEY_GenericObj_Name,
                                               &attributes);
        if (SUCCEEDED(hr))
        {
            hr = attributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &canWrite);
            if (SUCCEEDED(hr))
            {
                if (canWrite)
                {
                    wprintf(L"The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for PKEY_GenericObj_Name reports TRUE\nThis means that the property can be changed/updated\n\n");
                }
                else
                {
                    wprintf(L"The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for PKEY_GenericObj_Name reports FALSE\nThis means that the property cannot be changed/updated\n\n");
                }
            }
            else
            {
                wprintf(L"! Failed to get the WPD_PROPERTY_ATTRIBUTE_CAN_WRITE value for PKEY_GenericObj_Name on object '%ws', hr = 0x%lx\n", selection, hr);
            }
        }
        else
        {
            wprintf(L"! Failed to retrieve property attributes for object '%ws', hr = 0x%lx\n", selection, hr);
        }
        
    }

    // 4) Prompt the user for the new value of the NAME_GenericObj_Name property only if the property attributes report
    // that it can be changed/updated.
    if (canWrite)
    {
        wprintf(L"Enter the new PKEY_GenericObj_Name property for the object '%ws'.\n>", selection);
        hr = StringCchGetsW(newObjectName, ARRAYSIZE(newObjectName));
        if (FAILED(hr))
        {
            wprintf(L"An invalid PKEY_GenericObj_Name was specified, aborting property writing\n");
        }

        // 5) CoCreate an IPortableDeviceValues interface to hold the the property values
        // we wish to write.
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectPropertiesToWrite));
            if (SUCCEEDED(hr))
            {
                hr = objectPropertiesToWrite->SetStringValue(PKEY_GenericObj_Name, newObjectName);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to add PKEY_GenericObj_Name to IPortableDeviceValues, hr= 0x%lx\n", hr);
                }
            }
        }

        // 6) Call SetValues() passing the collection of specified PROPERTYKEYs.
        if (SUCCEEDED(hr))
        {
            hr = properties->SetValues(selection,                       // The object whose properties we are reading
                                       objectPropertiesToWrite.Get(),   // The properties we want to read
                                       &propertyWriteResults);          // Driver supplied property result values for the property read operation
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set properties for object '%ws', hr= 0x%lx\n", selection, hr);
            }
            else
            {
                wprintf(L"The PKEY_GenericObj_Name property on object '%ws' was written successfully (Read the properties again to see the updated value)\n", selection);
            }
        }
    }
}
//</SnippetWriteContentProperties1>

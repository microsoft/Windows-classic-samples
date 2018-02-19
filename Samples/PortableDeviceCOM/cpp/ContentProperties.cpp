// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

HANDLE g_bulkPropertyOperationEvent = nullptr;

// A helper function contained in ContentEnumeration.cpp which
// will recursively enumerate all objects and return an
// IPortableDevicePropVariantCollection containing the values.
HRESULT CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(
    _In_         IPortableDeviceContent*                content,
    _COM_Outptr_ IPortableDevicePropVariantCollection** objectIDs);

// Displays a property assumed to be in error code form.
void DisplayErrorResultProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName)
{
    HRESULT error = S_OK;
    HRESULT hr    = properties->GetErrorValue(key, &error);
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
    HRESULT hr    = properties->GetStringValue(key, &value);
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
    HRESULT hr    = properties->GetGuidValue(key, &value);
    if (SUCCEEDED(hr))
    {
        wprintf(L"%ws: %ws\n", keyName, (PCWSTR)CGuidToString(value));
    }
    else
    {
        wprintf(L"%ws: <Not Found>\n", keyName);
    }
}

// Reads properties for the user specified object.
void ReadContentProperties(
    _In_ IPortableDevice*    device)
{
    HRESULT                               hr = S_OK;
    WCHAR                                 selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceProperties>     properties;
    ComPtr<IPortableDeviceValues>         objectProperties;
    ComPtr<IPortableDeviceContent>        content;
    ComPtr<IPortableDeviceKeyCollection>  propertiesToRead;

    // Prompt user to enter an object identifier on the device to read properties from.
    wprintf(L"Enter the identifier of the object you wish to read properties from.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting property reading\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 3) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    //<SnippetContentProp1>
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&propertiesToRead));
    if (SUCCEEDED(hr))
    {
        // 4) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        HRESULT tempHr = propertiesToRead->Add(WPD_OBJECT_PARENT_ID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_NAME);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_FORMAT);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }
    }
    //</SnippetContentProp1>
    // 5) Call GetValues() passing the collection of specified PROPERTYKEYs.
    //<SnippetContentProp2>
    if (SUCCEEDED(hr))
    {
        hr = properties->GetValues(selection,                // The object whose properties we are reading
                                   propertiesToRead.Get(),   // The properties we want to read
                                   &objectProperties);       // Driver supplied property values for the specified object
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get all properties for object '%ws', hr= 0x%lx\n", selection, hr);
        }
    }
    //</SnippetContentProp2>
    // 6) Display the returned property values to the user
    if (SUCCEEDED(hr))
    {
        DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_PARENT_ID,            L"WPD_OBJECT_PARENT_ID");
        DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_NAME,                 L"WPD_OBJECT_NAME");
        DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_PERSISTENT_UNIQUE_ID, L"WPD_OBJECT_PERSISTENT_UNIQUE_ID");
        DisplayGuidProperty  (objectProperties.Get(), WPD_OBJECT_CONTENT_TYPE,         L"WPD_OBJECT_CONTENT_TYPE");
        DisplayGuidProperty  (objectProperties.Get(), WPD_OBJECT_FORMAT,               L"WPD_OBJECT_FORMAT");
    }
}

// Writes properties on the user specified object.
void WriteContentProperties(
    _In_ IPortableDevice*    device)
{
    //<SnippetContentProp3>
    HRESULT                               hr       = S_OK;
    BOOL                                  canWrite = FALSE;
    WCHAR                                 selection[SELECTION_BUFFER_SIZE]     = {0};
    WCHAR                                 newObjectName[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceProperties>     properties;
    ComPtr<IPortableDeviceContent>        content;
    ComPtr<IPortableDeviceValues>         objectPropertiesToWrite;
    ComPtr<IPortableDeviceValues>         propertyWriteResults;
    ComPtr<IPortableDeviceValues>         attributes;

    // Prompt user to enter an object identifier on the device to write properties on.
    wprintf(L"Enter the identifier of the object you wish to write properties on.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting property reading\n");
    }
    //</SnippetContentProp3>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetContentProp4>
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 3) Check the property attributes to see if we can write/change the WPD_OBJECT_NAME property.
    if (SUCCEEDED(hr))
    {
        hr = properties->GetPropertyAttributes(selection,
                                               WPD_OBJECT_NAME,
                                               &attributes);
        if (SUCCEEDED(hr))
        {
            hr = attributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &canWrite);
            if (SUCCEEDED(hr))
            {
                if (canWrite)
                {
                    wprintf(L"The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for the WPD_OBJECT_NAME reports TRUE\nThis means that the property can be changed/updated\n\n");
                }
                else
                {
                    wprintf(L"The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for the WPD_OBJECT_NAME reports FALSE\nThis means that the property cannot be changed/updated\n\n");
                }
            }
            else
            {
                wprintf(L"! Failed to get the WPD_PROPERTY_ATTRIBUTE_CAN_WRITE value from WPD_OBJECT_NAME on object '%ws', hr = 0x%lx\n", selection, hr);
            }
        }
    }
    //</SnippetContentProp4>

    // 4) Prompt the user for the new value of the WPD_OBJECT_NAME property only if the property attributes report
    // that it can be changed/updated.
    //<SnippetContentProp5>
    if (canWrite)
    {
        wprintf(L"Enter the new WPD_OBJECT_NAME for the object '%ws'.\n>", selection);
        hr = StringCchGetsW(newObjectName, ARRAYSIZE(newObjectName));
        if (FAILED(hr))
        {
            wprintf(L"An invalid object name was specified, aborting property writing\n");
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
                if (objectPropertiesToWrite != nullptr)
                {
                    hr = objectPropertiesToWrite->SetStringValue(WPD_OBJECT_NAME, newObjectName);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to add WPD_OBJECT_NAME to IPortableDeviceValues, hr= 0x%lx\n", hr);
                    }
                }
            }
        }
        //</SnippetContentProp5>
        // 6) Call SetValues() passing the collection of specified PROPERTYKEYs.
        //<SnippetContentProp6>
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
                wprintf(L"The WPD_OBJECT_NAME property on object '%ws' was written successfully (Read the properties again to see the updated value)\n", selection);
            }
        }
        //</SnippetContentProp6>
    }
}

// Retreives the object identifier for the persistent unique identifier
void GetObjectIdentifierFromPersistentUniqueIdentifier(
    _In_ IPortableDevice* device)
{
    HRESULT                                         hr = S_OK;
    ComPtr<IPortableDeviceContent>                  content;
    ComPtr<IPortableDevicePropVariantCollection>    persistentUniqueIDs;
    ComPtr<IPortableDevicePropVariantCollection>    objectIDs;
    WCHAR                                           selection[SELECTION_BUFFER_SIZE] = {0};

    //<SnippetContentProp7>
    // Prompt user to enter an unique identifier to convert to an object idenifier.
    wprintf(L"Enter the Persistant Unique Identifier of the object you wish to convert into an object identifier.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid persistent object identifier was specified, aborting the query operation\n");
    }
    //</SnippetContentProp7>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetContentProp8>
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetContentProp8>

    // 2) CoCreate an IPortableDevicePropVariantCollection interface to hold the the Unique Identifiers
    // to query for Object Identifiers.
    //
    // NOTE: This is a collection interface so more than 1 identifier can be requested at a time.
    //       This sample only requests a single unique identifier.
    //<SnippetContentProp9>
    hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&persistentUniqueIDs));
    //</SnippetContentProp9>
    //<SnippetContentProp10>
    if (SUCCEEDED(hr))
    {
        // Initialize a PROPVARIANT structure with the persistent unique identifier string
        // that the user selected above. This memory will be freed when PropVariantClear() is
        // called below.
        PROPVARIANT persistentUniqueID = {0};
        hr = InitPropVariantFromString(selection, &persistentUniqueID);
        if (SUCCEEDED(hr))
        {
            // Add the object identifier to the objects-to-delete list
            // (We are only deleting 1 in this example)
            hr = persistentUniqueIDs->Add(&persistentUniqueID);
            if (SUCCEEDED(hr))
            {
                // 3) Attempt to get the unique idenifier for the object from the device
                hr = content->GetObjectIDsFromPersistentUniqueIDs(persistentUniqueIDs.Get(),
                                                                    &objectIDs);
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT objectID = {0};
                    hr = objectIDs->GetAt(0, &objectID);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"The persistent unique identifier '%ws' relates to object identifier '%ws' on the device.\n", selection, objectID.pwszVal);
                    }
                    else
                    {
                        wprintf(L"! Failed to get the object identifier for '%ws' from the IPortableDevicePropVariantCollection, hr = 0x%lx\n", selection, hr);
                    }

                    // Free the returned allocated string from the GetAt() call
                    PropVariantClear(&objectID);
                }
                else
                {
                    wprintf(L"! Failed to get the object identifier from persistent object idenifier '%ws', hr = 0x%lx\n", selection, hr);
                }
            }
            else
            {
                wprintf(L"! Failed to get the object identifier from persistent object idenifier because we could no add the persistent object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"! Failed to get the object identifier because we could no allocate memory for the persistent object identifier string, hr = 0x%lx\n", hr);
        }

        // Free any allocated values in the PROPVARIANT before exiting
        PropVariantClear(&persistentUniqueID);
    }
    //</SnippetContentProp10>
}

// IPortableDevicePropertiesBulkCallback implementation for use with
// IPortableDevicePropertiesBulk operations.
class CGetBulkValuesCallback : public IPortableDevicePropertiesBulkCallback
{
public:
    CGetBulkValuesCallback() : m_ref(0)
    {

    }

    ~CGetBulkValuesCallback()
    {

    }

    IFACEMETHODIMP QueryInterface(
        _In_         REFIID  riid,
        _COM_Outptr_ void**  ppv)
    {
        static const QITAB qitab[] = 
        {
            QITABENT(CGetBulkValuesCallback, IPortableDevicePropertiesBulkCallback),
            { },
        };

        return QISearch(this, qitab, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_ref);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long ref = InterlockedDecrement(&m_ref);
        if (ref == 0)
        {
            delete this;
        }
        return ref;
    }

    IFACEMETHODIMP OnStart(
        _In_ REFGUID context)
    {
        wprintf(L"** BULK Property operation starting, context = %ws **\n", (PCWSTR)CGuidToString(context));

        return S_OK;
    }

    IFACEMETHODIMP OnProgress(
        _In_ REFGUID                            context,
        _In_ IPortableDeviceValuesCollection*   values)
    {
        DWORD   numValues = 0;
        HRESULT hr = values->GetCount(&numValues);

        // Display the returned properties to the user.
        // NOTE: We are reading for expected properties, which were setup in the
        // QueueGetXXXXXX bulk operation call.
        if (SUCCEEDED(hr))
        {
            wprintf(L"Received next batch of %u object value elements..., context = %ws\n", numValues, (PCWSTR)CGuidToString(context));

            for (DWORD index = 0; index < numValues; index++)
            {
                ComPtr<IPortableDeviceValues> objectProperties;
                hr = values->GetAt(index, &objectProperties);
                if (SUCCEEDED(hr))
                {
                    DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_PARENT_ID,            L"WPD_OBJECT_PARENT_ID");
                    DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_NAME,                 L"WPD_OBJECT_NAME");
                    DisplayStringProperty(objectProperties.Get(), WPD_OBJECT_PERSISTENT_UNIQUE_ID, L"WPD_OBJECT_PERSISTENT_UNIQUE_ID");
                    DisplayGuidProperty  (objectProperties.Get(), WPD_OBJECT_CONTENT_TYPE,         L"WPD_OBJECT_CONTENT_TYPE");
                    DisplayGuidProperty  (objectProperties.Get(), WPD_OBJECT_FORMAT,               L"WPD_OBJECT_FORMAT");
                    wprintf(L"\n\n");
                }
                else
                {
                    wprintf(L"! Failed to get IPortableDeviceValues from IPortableDeviceValuesCollection at index '%u'\n", index);
                }
            }
        }
        else
        {
            wprintf(L"! Failed to get number of elements from IPortableDeviceValuesCollection, hr = 0x%lx\n", hr);
        }

        return hr;
    }

    IFACEMETHODIMP OnEnd(
        _In_ REFGUID context,
             HRESULT status)
    {
        wprintf(L"** BULK Property operation ending, status = 0x%lx, context = %ws **\n", status, (PCWSTR)CGuidToString(context));

        // This assumes that we are only performing a single operation
        // at a time, so no check is needed on the context when setting
        // the operation complete event.
        if (g_bulkPropertyOperationEvent != nullptr)
        {
            SetEvent(g_bulkPropertyOperationEvent);
        }

        return S_OK;
    }

private:
    long m_ref;
};

// IPortableDevicePropertiesBulkCallback implementation for use with
// IPortableDevicePropertiesBulk operations.
class CSetBulkValuesCallback : public IPortableDevicePropertiesBulkCallback
{
public:
    CSetBulkValuesCallback() : m_ref(0)
    {

    }

    ~CSetBulkValuesCallback()
    {

    }

    IFACEMETHODIMP QueryInterface(
        _In_         REFIID  riid,
        _COM_Outptr_ void**  ppv)
    {
        static const QITAB qitab[] = 
        {
            QITABENT(CGetBulkValuesCallback, IPortableDevicePropertiesBulkCallback),
            { },
        };

        return QISearch(this, qitab, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_ref);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long ref = InterlockedDecrement(&m_ref);
        if (ref == 0)
        {
            delete this;
        }
        return ref;
    }

    IFACEMETHODIMP OnStart(
        _In_ REFGUID context)
    {
        wprintf(L"** BULK Property operation starting, context = %ws **\n", (PCWSTR)CGuidToString(context));

        return S_OK;
    }

    IFACEMETHODIMP OnProgress(
        _In_ REFGUID                            context,
        _In_ IPortableDeviceValuesCollection*   values)
    {
        DWORD   numElements = 0;
        HRESULT hr = values->GetCount(&numElements);

        // Display the returned properties set operation results to the user.
        // NOTE: We are reading for expected properties, which were setup in the
        // QueueSetXXXXXX bulk operation call.  The values returned are in the
        // form VT_ERROR holding the HRESULT for the set operation.
        if (SUCCEEDED(hr))
        {
            wprintf(L"Received next batch of %u object value elements..., context = %ws\n", numElements, (PCWSTR)CGuidToString(context));

            for (DWORD index = 0; index < numElements; index++)
            {
                ComPtr<IPortableDeviceValues> objectProperties;
                hr = values->GetAt(index, &objectProperties);
                if (SUCCEEDED(hr))
                {
                    DisplayStringProperty     (objectProperties.Get(), WPD_OBJECT_ID,     L"WPD_OBJECT_ID");
                    DisplayErrorResultProperty(objectProperties.Get(), WPD_OBJECT_NAME,   L"WPD_OBJECT_NAME");
                    wprintf(L"\n\n");
                }
                else
                {
                    wprintf(L"! Failed to get IPortableDeviceValues from IPortableDeviceValuesCollection at index '%u'\n", index);
                }
            }
        }
        else
        {
            wprintf(L"! Failed to get number of elements from IPortableDeviceValuesCollection, hr = 0x%lx\n", hr);
        }
        return hr;
    }

    IFACEMETHODIMP OnEnd(
        _In_ REFGUID context,
             HRESULT status)
    {
        wprintf(L"** BULK Property operation ending, status = 0x%lx, context = %ws **\n", status, (PCWSTR)CGuidToString(context));

        // This assumes that we are only performing a single operation
        // at a time, so no check is needed on the context when setting
        // the operation complete event.
        if (g_bulkPropertyOperationEvent != nullptr)
        {
            SetEvent(g_bulkPropertyOperationEvent);
        }

        return S_OK;
    }

private:
    long m_ref;
};

// Reads a set of properties for all objects.
void ReadContentPropertiesBulk(
    _In_ IPortableDevice*    device)
{
    HRESULT                                       hr      = S_OK;
    GUID                                          context = GUID_NULL;
    ComPtr<CGetBulkValuesCallback>                callback;
    ComPtr<IPortableDeviceProperties>             properties;
    ComPtr<IPortableDevicePropertiesBulk>         propertiesBulk;
    ComPtr<IPortableDeviceValues>                 objectProperties;
    ComPtr<IPortableDeviceContent>                content;
    ComPtr<IPortableDeviceKeyCollection>          propertiesToRead;
    ComPtr<IPortableDevicePropVariantCollection>  objectIDs;


    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = device->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = properties.As(&propertiesBulk);
        if (FAILED(hr))
        {
            wprintf(L"This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&propertiesToRead));
        if (FAILED(hr))
        {
            wprintf(L"! Failed to CoCreate IPortableDeviceKeyCollection to hold the property keys to read, hr = 0x%lx\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // 5) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        HRESULT tempHr = S_OK;
        tempHr = propertiesToRead->Add(WPD_OBJECT_PARENT_ID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_NAME);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_FORMAT);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }

        tempHr = propertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        callback = new (std::nothrow) CGetBulkValuesCallback();
        if (callback == nullptr)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"! Failed to allocate CGetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call our helper function CreateIPortableDevicePropVariantCollectionWithAllObjectIDs
    // to enumerate and create an IPortableDevicePropVariantCollection with the object
    // identifiers needed to perform the bulk operation on.
    if (SUCCEEDED(hr))
    {
        hr = CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(content.Get(),
                                                                        &objectIDs);
    }


    // 8) Call QueueGetValuesByObjectList to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        hr = propertiesBulk->QueueGetValuesByObjectList(objectIDs.Get(),
                                                        propertiesToRead.Get(),
                                                        callback.Get(),
                                                        &context);
        // 9) Call Start() to actually begin the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                CloseHandle(g_bulkPropertyOperationEvent);
                g_bulkPropertyOperationEvent = nullptr;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_bulkPropertyOperationEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                // Call Start() to actually begin the Asynchronous bulk operation.
                hr = propertiesBulk->Start(context);
                if(FAILED(hr))
                {
                    wprintf(L"! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                wprintf(L"! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            wprintf(L"! QueueGetValuesByObjectList Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_bulkPropertyOperationEvent != nullptr)
        {
            WaitForSingleObject(g_bulkPropertyOperationEvent, INFINITE);
        }
    }

    // Cleanup any created global event handles before exiting..
    if (g_bulkPropertyOperationEvent != nullptr)
    {
        CloseHandle(g_bulkPropertyOperationEvent);
        g_bulkPropertyOperationEvent = nullptr;
    }
}

// Writes a set of properties for all objects.
void WriteContentPropertiesBulk(
    IPortableDevice*    device)
{
    HRESULT                                       hr           = S_OK;
    DWORD                                         numObjectIDs = 0;
    GUID                                          context      = GUID_NULL;
    ComPtr<CSetBulkValuesCallback>                callback;
    ComPtr<IPortableDeviceProperties>             properties;
    ComPtr<IPortableDevicePropertiesBulk>         propertiesBulk;
    ComPtr<IPortableDeviceValues>                 objectProperties;
    ComPtr<IPortableDeviceContent>                content;
    ComPtr<IPortableDeviceValuesCollection>       propertiesToWrite;
    ComPtr<IPortableDevicePropVariantCollection>  objectIDs;

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = device->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = properties.As(&propertiesBulk);
        if (FAILED(hr))
        {
            wprintf(L"This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceValuesCollection interface to hold the the properties
    // we wish to write.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceValuesCollection,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&propertiesToWrite));
        if (FAILED(hr))
        {
            wprintf(L"! Failed to CoCreate IPortableDeviceValuesCollection for bulk property values, hr = 0x%lx\n", hr);
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        callback = new (std::nothrow) CSetBulkValuesCallback();
        if (callback == nullptr)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"! Failed to allocate CSetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call our helper function CreateIPortableDevicePropVariantCollectionWithAllObjectIDs
    // to enumerate and create an IPortableDevicePropVariantCollection with the object
    // identifiers needed to perform the bulk operation on.
    if (SUCCEEDED(hr))
    {
        hr = CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(content.Get(),
                                                                        &objectIDs);
    }

    if (SUCCEEDED(hr))
    {
        hr = objectIDs->GetCount(&numObjectIDs);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of objectIDs from IPortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
        }
    }

    // 8) Iterate through object list and add appropriate IPortableDeviceValues to collection
    if (SUCCEEDED(hr))
    {
        for(DWORD index = 0; (index < numObjectIDs) && (hr == S_OK); index++)
        {
            ComPtr<IPortableDeviceValues>   newvalues;
            PROPVARIANT                     objectID = {0};

            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&newvalues));
            if (FAILED(hr))
            {
                wprintf(L"! Failed to CoCreate CLSID_PortableDeviceValues, hr = 0x%lx\n", hr);
            }

            // Get the Object ID whose properties we will set
            if (hr == S_OK)
            {
                hr = objectIDs->GetAt(index, &objectID);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to get next Object ID from list, hr = 0x%lx\n", hr);
                }
            }

            // Save them into the IPortableDeviceValues so the driver knows which object this proeprty set belongs to
            if (hr == S_OK)
            {
                hr = newvalues->SetStringValue(WPD_OBJECT_ID, objectID.pwszVal);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to set WPD_OBJECT_ID, hr = 0x%lx\n", hr);
                }
            }

            // Set the new values.  In this sample, we attempt to set the name property.
            if (hr == S_OK)
            {
                WCHAR newName[MAX_PATH] = {0};
                hr = StringCchPrintfW(newName, ARRAYSIZE(newName), L"NewName%u", index);

                if (hr == S_OK)
                {
                    hr = newvalues->SetStringValue(WPD_OBJECT_NAME, newName);
                }

                if (FAILED(hr))
                {
                    wprintf(L"! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
                }
            }

            // Add this property set to the collection
            if (hr == S_OK)
            {
                hr = propertiesToWrite->Add(newvalues.Get());
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to add values to collection, hr = 0x%lx\n", hr);
                }
            }
            PropVariantClear(&objectID);
        }
    }

    // 9) Call QueueSetValuesByObjectList to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        hr = propertiesBulk->QueueSetValuesByObjectList(propertiesToWrite.Get(),
                                                        callback.Get(),
                                                        &context);
        // 10) Call Start() to actually begin the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                CloseHandle(g_bulkPropertyOperationEvent);
                g_bulkPropertyOperationEvent = nullptr;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_bulkPropertyOperationEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                // Call Start() to actually begin the Asynchronous bulk operation.
                hr = propertiesBulk->Start(context);
                if(FAILED(hr))
                {
                    wprintf(L"! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                wprintf(L"! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            wprintf(L"! QueueSetValuesByObjectList Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_bulkPropertyOperationEvent != nullptr)
        {
            WaitForSingleObject(g_bulkPropertyOperationEvent, INFINITE);
        }
    }

    // Cleanup any created global event handles before exiting..
    if (g_bulkPropertyOperationEvent != nullptr)
    {
        CloseHandle(g_bulkPropertyOperationEvent);
        g_bulkPropertyOperationEvent = nullptr;
    }
}

// Reads a set of properties for all objects of a particular format.
void ReadContentPropertiesBulkFilteringByFormat(
    _In_ IPortableDevice*    device)
{
    HRESULT                                       hr      = S_OK;
    GUID                                          context = GUID_NULL;
    ComPtr<CGetBulkValuesCallback>                callback;
    ComPtr<IPortableDeviceProperties>             properties;
    ComPtr<IPortableDevicePropertiesBulk>         propertiesBulk;
    ComPtr<IPortableDeviceValues>                 objectProperties;
    ComPtr<IPortableDeviceContent>                content;
    ComPtr<IPortableDeviceKeyCollection>          propertiesToRead;


    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = device->Content(&content);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = properties.As(&propertiesBulk);
        if (FAILED(hr))
        {
            wprintf(L"This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&propertiesToRead));
        if (FAILED(hr))
        {
            wprintf(L"! Failed to CoCreate IPortableDeviceKeyCollection to hold the property keys to read, hr = 0x%lx\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // 5) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (propertiesToRead != nullptr)
        {
            HRESULT tempHr = S_OK;
            tempHr = propertiesToRead->Add(WPD_OBJECT_PARENT_ID);
            if (FAILED(tempHr))
            {
                wprintf(L"! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
            }

            tempHr = propertiesToRead->Add(WPD_OBJECT_NAME);
            if (FAILED(tempHr))
            {
                wprintf(L"! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
            }

            tempHr = propertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
            if (FAILED(tempHr))
            {
                wprintf(L"! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
            }

            tempHr = propertiesToRead->Add(WPD_OBJECT_FORMAT);
            if (FAILED(tempHr))
            {
                wprintf(L"! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
            }

            tempHr = propertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
            if (FAILED(tempHr))
            {
                wprintf(L"! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
            }
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        callback = new (std::nothrow) CGetBulkValuesCallback();
        if (callback == nullptr)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"! Failed to allocate CGetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call QueueGetValuesByObjectFormat to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        static const DWORD DEPTH = 100;
        hr = propertiesBulk->QueueGetValuesByObjectFormat(WPD_OBJECT_FORMAT_MP3,
                                                          WPD_DEVICE_OBJECT_ID,
                                                          DEPTH,
                                                          propertiesToRead.Get(),
                                                          callback.Get(),
                                                          &context);
        // 9) Call Start() to actually begin the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                CloseHandle(g_bulkPropertyOperationEvent);
                g_bulkPropertyOperationEvent = nullptr;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_bulkPropertyOperationEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_bulkPropertyOperationEvent != nullptr)
            {
                // Call Start() to actually begin the Asynchronous bulk operation.
                hr = propertiesBulk->Start(context);
                if(FAILED(hr))
                {
                    wprintf(L"! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                wprintf(L"! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            wprintf(L"! QueueGetValuesByObjectFormat Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_bulkPropertyOperationEvent != nullptr)
        {
            WaitForSingleObject(g_bulkPropertyOperationEvent, INFINITE);
        }
    }

    // Cleanup any created global event handles before exiting..
    if (g_bulkPropertyOperationEvent != nullptr)
    {
        CloseHandle(g_bulkPropertyOperationEvent);
        g_bulkPropertyOperationEvent = nullptr;
    }
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>

HANDLE g_hBulkPropertyOperationEvent = NULL;

// A helper function contained in ContentEnumeration.cpp which
// will recursively enumerate all objects and return an
// IPortableDevicePropVariantCollection containing the values.
HRESULT CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(
    IPortableDeviceContent* pContent,
    IPortableDevicePropVariantCollection** ppObjectIDs);

// Displays a property assumed to be in error code form.
void DisplayErrorResultProperty(
    IPortableDeviceValues*  pProperties,
    REFPROPERTYKEY          key,
    PCWSTR                  pszKey)
{
    HRESULT hError   = S_OK;
    HRESULT hr = pProperties->GetErrorValue(key,&hError);
    if (SUCCEEDED(hr))
    {
        printf("%ws: HRESULT = (0x%lx)\n", pszKey, hError);
    }
    else
    {
        printf("%ws: <Not Found>\n", pszKey);
    }
}

// Displays a property assumed to be in string form.
void DisplayStringProperty(
    IPortableDeviceValues*  pProperties,
    REFPROPERTYKEY          key,
    PCWSTR                  pszKey)
{
    PWSTR   pszValue = NULL;
    HRESULT hr = pProperties->GetStringValue(key,&pszValue);
    if (SUCCEEDED(hr))
    {
        // Get the length of the string value so we
        // can output <empty string value> if one
        // is encountered.
        CAtlStringW strValue;
        strValue = pszValue;
        if (strValue.GetLength() > 0)
        {
            printf("%ws: %ws\n",pszKey, pszValue);
        }
        else
        {
            printf("%ws: <empty string value>\n", pszKey);
        }
    }
    else
    {
        printf("%ws: <Not Found>\n", pszKey);
    }

    // Free the allocated string returned from the
    // GetStringValue method
    CoTaskMemFree(pszValue);
    pszValue = NULL;
}

// Displays a property assumed to be in GUID form.
void DisplayGuidProperty(
    IPortableDeviceValues*  pProperties,
    REFPROPERTYKEY          key,
    PCWSTR                  pszKey)
{
    GUID    guidValue = GUID_NULL;
    HRESULT hr = pProperties->GetGuidValue(key,&guidValue);
    if (SUCCEEDED(hr))
    {
        printf("%ws: %ws\n",pszKey, (PWSTR)CGuidToString(guidValue));
    }
    else
    {
        printf("%ws: <Not Found>\n", pszKey);
    }
}

// Reads properties for the user specified object.
void ReadContentProperties(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                               hr               = S_OK;
    WCHAR                                 szSelection[81]  = {0};
    CComPtr<IPortableDeviceProperties>    pProperties;
    CComPtr<IPortableDeviceValues>        pObjectProperties;
    CComPtr<IPortableDeviceContent>       pContent;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;

    // Prompt user to enter an object identifier on the device to read properties from.
    printf("Enter the identifier of the object you wish to read properties from.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting property reading\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 3) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
	//<SnippetContentProp1>
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pPropertiesToRead));
    if (SUCCEEDED(hr))
    {
        // 4) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (pPropertiesToRead != NULL)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PARENT_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_NAME);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_FORMAT);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }
        }
    }
	//</SnippetContentProp1>
	// 5) Call GetValues() passing the collection of specified PROPERTYKEYs.
	//<SnippetContentProp2>
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetValues(szSelection,         // The object whose properties we are reading
                                    pPropertiesToRead,   // The properties we want to read
                                    &pObjectProperties); // Driver supplied property values for the specified object
        if (FAILED(hr))
        {
            printf("! Failed to get all properties for object '%ws', hr= 0x%lx\n", szSelection, hr);
        }
    }
	//</SnippetContentProp2>
    // 6) Display the returned property values to the user
    if (SUCCEEDED(hr))
    {
        DisplayStringProperty(pObjectProperties, WPD_OBJECT_PARENT_ID,            L"WPD_OBJECT_PARENT_ID");
        DisplayStringProperty(pObjectProperties, WPD_OBJECT_NAME,                 L"WPD_OBJECT_NAME");
        DisplayStringProperty(pObjectProperties, WPD_OBJECT_PERSISTENT_UNIQUE_ID, L"WPD_OBJECT_PERSISTENT_UNIQUE_ID");
        DisplayGuidProperty  (pObjectProperties, WPD_OBJECT_CONTENT_TYPE,         L"WPD_OBJECT_CONTENT_TYPE");
        DisplayGuidProperty  (pObjectProperties, WPD_OBJECT_FORMAT,               L"WPD_OBJECT_FORMAT");
    }
}

// Writes properties on the user specified object.
void WriteContentProperties(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }
	//<SnippetContentProp3>
    HRESULT                               hr                   = S_OK;
    WCHAR                                 szSelection[81]      = {0};
    WCHAR                                 szNewObjectName[81]  = {0};
    CComPtr<IPortableDeviceProperties>    pProperties;
    CComPtr<IPortableDeviceContent>       pContent;
    CComPtr<IPortableDeviceValues>        pObjectPropertiesToWrite;
    CComPtr<IPortableDeviceValues>        pPropertyWriteResults;
    CComPtr<IPortableDeviceValues>        pAttributes;
    BOOL                                  bCanWrite            = FALSE;

    // Prompt user to enter an object identifier on the device to write properties on.
    printf("Enter the identifier of the object you wish to write properties on.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting property reading\n");
    }
	//</SnippetContentProp3>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
	//<SnippetContentProp4>
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 3) Check the property attributes to see if we can write/change the WPD_OBJECT_NAME property.
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetPropertyAttributes(szSelection,
                                                WPD_OBJECT_NAME,
                                                &pAttributes);
        if (SUCCEEDED(hr))
        {
            hr = pAttributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &bCanWrite);
            if (SUCCEEDED(hr))
            {
                if (bCanWrite)
                {
                    printf("The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for the WPD_OBJECT_NAME reports TRUE\nThis means that the property can be changed/updated\n\n");
                }
                else
                {
                    printf("The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for the WPD_OBJECT_NAME reports FALSE\nThis means that the property cannot be changed/updated\n\n");
                }
            }
            else
            {
                printf("! Failed to get the WPD_PROPERTY_ATTRIBUTE_CAN_WRITE value from WPD_OBJECT_NAME on object '%ws', hr = 0x%lx\n",szSelection, hr);
            }
        }
    }
	//</SnippetContentProp4>

    // 4) Prompt the user for the new value of the WPD_OBJECT_NAME property only if the property attributes report
    // that it can be changed/updated.
	//<SnippetContentProp5>
    if (bCanWrite)
    {
        printf("Enter the new WPD_OBJECT_NAME for the object '%ws'.\n>",szSelection);
        hr = StringCbGetsW(szNewObjectName,sizeof(szNewObjectName));
        if (FAILED(hr))
        {
            printf("An invalid object name was specified, aborting property writing\n");
        }

        // 5) CoCreate an IPortableDeviceValues interface to hold the the property values
        // we wish to write.
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectPropertiesToWrite));
            if (SUCCEEDED(hr))
            {
                if (pObjectPropertiesToWrite != NULL)
                {
                    hr = pObjectPropertiesToWrite->SetStringValue(WPD_OBJECT_NAME, szNewObjectName);
                    if (FAILED(hr))
                    {
                        printf("! Failed to add WPD_OBJECT_NAME to IPortableDeviceValues, hr= 0x%lx\n", hr);
                    }
                }
            }
        }
		//</SnippetContentProp5>
        // 6) Call SetValues() passing the collection of specified PROPERTYKEYs.
		//<SnippetContentProp6>
        if (SUCCEEDED(hr))
        {
            hr = pProperties->SetValues(szSelection,                // The object whose properties we are reading
                                        pObjectPropertiesToWrite,   // The properties we want to read
                                        &pPropertyWriteResults);    // Driver supplied property result values for the property read operation
            if (FAILED(hr))
            {
                printf("! Failed to set properties for object '%ws', hr= 0x%lx\n", szSelection, hr);
            }
            else
            {
                printf("The WPD_OBJECT_NAME property on object '%ws' was written successfully (Read the properties again to see the updated value)\n", szSelection);
            }
        }
		//</SnippetContentProp6>
    }
}

// Retreives the object identifier for the persistent unique identifier
void GetObjectIdentifierFromPersistentUniqueIdentifier(
    IPortableDevice* pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                                         hr                  = S_OK;
    WCHAR                                           szSelection[81]     = {0};
    CComPtr<IPortableDeviceContent>                 pContent;
    CComPtr<IPortableDevicePropVariantCollection>   pPersistentUniqueIDs;
    CComPtr<IPortableDevicePropVariantCollection>   pObjectIDs;
	//<SnippetContentProp7>
    // Prompt user to enter an unique identifier to convert to an object idenifier.
    printf("Enter the Persistant Unique Identifier of the object you wish to convert into an object identifier.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid persistent object identifier was specified, aborting the query operation\n");
    }
	//</SnippetContentProp7>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
	//<SnippetContentProp8>
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
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
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pPersistentUniqueIDs));
	//</SnippetContentProp9>
	//<SnippetContentProp10>
    if (SUCCEEDED(hr))
    {
        if (pPersistentUniqueIDs != NULL)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);

            // Initialize a PROPVARIANT structure with the object identifier string
            // that the user selected above. Notice we are allocating memory for the
            // PWSTR value.  This memory will be freed when PropVariantClear() is
            // called below.
            pv.vt      = VT_LPWSTR;
            pv.pwszVal = AtlAllocTaskWideString(szSelection);
            if (pv.pwszVal != NULL)
            {
                // Add the object identifier to the objects-to-delete list
                // (We are only deleting 1 in this example)
                hr = pPersistentUniqueIDs->Add(&pv);
                if (SUCCEEDED(hr))
                {
                    // 3) Attempt to get the unique idenifier for the object from the device
                    hr = pContent->GetObjectIDsFromPersistentUniqueIDs(pPersistentUniqueIDs,
                                                                       &pObjectIDs);
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT pvId = {0};
                        hr = pObjectIDs->GetAt(0, &pvId);
                        if (SUCCEEDED(hr))
                        {
                            printf("The persistent unique identifier '%ws' relates to object identifier '%ws' on the device.\n", szSelection, pvId.pwszVal);
                        }
                        else
                        {
                            printf("! Failed to get the object identifier for '%ws' from the IPortableDevicePropVariantCollection, hr = 0x%lx\n",szSelection, hr);
                        }

                        // Free the returned allocated string from the GetAt() call
                        PropVariantClear(&pvId);
                    }
                    else
                    {
                        printf("! Failed to get the object identifier from persistent object idenifier '%ws', hr = 0x%lx\n",szSelection, hr);
                    }
                }
                else
                {
                    printf("! Failed to get the object identifier from persistent object idenifier because we could no add the persistent object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
                printf("! Failed to get the object identifier because we could no allocate memory for the persistent object identifier string, hr = 0x%lx\n",hr);
            }

            // Free any allocated values in the PROPVARIANT before exiting
            PropVariantClear(&pv);
        }
    }
	//</SnippetContentProp10>
}

// IPortableDevicePropertiesBulkCallback implementation for use with
// IPortableDevicePropertiesBulk operations.
class CGetBulkValuesCallback : public IPortableDevicePropertiesBulkCallback
{
public:
    CGetBulkValuesCallback() : m_cRef(1)
    {

    }

    ~CGetBulkValuesCallback()
    {

    }

    HRESULT __stdcall QueryInterface(
        REFIID  riid,
        LPVOID* ppvObj)
    {
        HRESULT hr = S_OK;
        if (ppvObj == NULL)
        {
            hr = E_INVALIDARG;
            return hr;
        }

        if ((riid == IID_IUnknown) ||
            (riid == IID_IPortableDevicePropertiesBulkCallback))
        {
            AddRef();
            *ppvObj = this;
        }
        else
        {
            *ppvObj = NULL;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

    ULONG __stdcall AddRef()
    {
        InterlockedIncrement((long*) &m_cRef);
        return m_cRef;
    }

    ULONG __stdcall Release()
    {
        ULONG ulRefCount = m_cRef - 1;

        if (InterlockedDecrement((long*) &m_cRef) == 0)
        {
            delete this;
            return 0;
        }
        return ulRefCount;
    }

    HRESULT __stdcall OnStart(
        REFGUID Context)
    {
        printf("** BULK Property operation starting, Context = %ws **\n", (PWSTR)CGuidToString(Context));

        return S_OK;
    }

    HRESULT __stdcall OnProgress(
        REFGUID                            Context,
        IPortableDeviceValuesCollection*   pValues)
    {
        DWORD   dwNumElements = 0;
        HRESULT hr = pValues->GetCount(&dwNumElements);
        if (FAILED(hr))
        {
            printf("! Failed to get number of elements from IPortableDeviceValuesCollection, hr = 0x%lx\n",hr);
        }

        printf("Received next batch of %d object value elements..., Context = %ws\n", dwNumElements, (PWSTR)CGuidToString(Context));

        // Display the returned properties to the user.
        // NOTE: We are reading for expected properties, which were setup in the
        // QueueGetXXXXXX bulk operation call.
        if (SUCCEEDED(hr))
        {
            for (DWORD dwIndex = 0; dwIndex < dwNumElements; dwIndex++)
            {
                CComPtr<IPortableDeviceValues> pObjectProperties;
                hr = pValues->GetAt(dwIndex, &pObjectProperties);
                if (SUCCEEDED(hr))
                {
                    DisplayStringProperty(pObjectProperties, WPD_OBJECT_PARENT_ID,            L"WPD_OBJECT_PARENT_ID");
                    DisplayStringProperty(pObjectProperties, WPD_OBJECT_NAME,                 L"WPD_OBJECT_NAME");
                    DisplayStringProperty(pObjectProperties, WPD_OBJECT_PERSISTENT_UNIQUE_ID, L"WPD_OBJECT_PERSISTENT_UNIQUE_ID");
                    DisplayGuidProperty  (pObjectProperties, WPD_OBJECT_CONTENT_TYPE,         L"WPD_OBJECT_CONTENT_TYPE");
                    DisplayGuidProperty  (pObjectProperties, WPD_OBJECT_FORMAT,               L"WPD_OBJECT_FORMAT");
                    printf("\n\n");
                }
                else
                {
                    printf("! Failed to get IPortableDeviceValues from IPortableDeviceValuesCollection at index '%d'\n", dwIndex);
                }
            }
        }

        return hr;
    }

    HRESULT __stdcall OnEnd(
        REFGUID Context,
        HRESULT hrStatus)
    {
        printf("** BULK Property operation ending, hrStatus = 0x%lx, Context = %ws **\n", hrStatus, (PWSTR)CGuidToString(Context));

        // This assumes that we are only performing a single operation
        // at a time, so no check is needed on the context when setting
        // the operation complete event.
        if (g_hBulkPropertyOperationEvent != NULL)
        {
            SetEvent(g_hBulkPropertyOperationEvent);
        }

        return S_OK;
    }

    ULONG m_cRef;
};

// IPortableDevicePropertiesBulkCallback implementation for use with
// IPortableDevicePropertiesBulk operations.
class CSetBulkValuesCallback : public IPortableDevicePropertiesBulkCallback
{
public:
    CSetBulkValuesCallback() : m_cRef(1)
    {

    }

    ~CSetBulkValuesCallback()
    {

    }

    HRESULT __stdcall QueryInterface(
        REFIID  riid,
        LPVOID* ppvObj)
    {
        HRESULT hr = S_OK;
        if (ppvObj == NULL)
        {
            hr = E_INVALIDARG;
            return hr;
        }

        if ((riid == IID_IUnknown) ||
            (riid == IID_IPortableDevicePropertiesBulkCallback))
        {
            AddRef();
            *ppvObj = this;
        }
        else
        {
            *ppvObj = NULL;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

    ULONG __stdcall AddRef()
    {
        InterlockedIncrement((long*) &m_cRef);
        return m_cRef;
    }

    ULONG __stdcall Release()
    {
        ULONG ulRefCount = m_cRef - 1;

        if (InterlockedDecrement((long*) &m_cRef) == 0)
        {
            delete this;
            return 0;
        }
        return ulRefCount;
    }

    HRESULT __stdcall OnStart(
        REFGUID Context)
    {
        printf("** BULK Property operation starting, Context = %ws **\n", (PWSTR)CGuidToString(Context));

        return S_OK;
    }

    HRESULT __stdcall OnProgress(
        REFGUID                            Context,
        IPortableDeviceValuesCollection*   pValues)
    {
        DWORD dwNumElements = 0;
        HRESULT hr = pValues->GetCount(&dwNumElements);
        if (FAILED(hr))
        {
            printf("! Failed to get number of elements from IPortableDeviceValuesCollection, hr = 0x%lx\n",hr);
        }

        printf("Received next batch of %d object value elements..., Context = %ws\n", dwNumElements, (PWSTR)CGuidToString(Context));

        // Display the returned properties set operation results to the user.
        // NOTE: We are reading for expected properties, which were setup in the
        // QueueSetXXXXXX bulk operation call.  The values returned are in the
        // form VT_ERROR holding the HRESULT for the set operation.
        if (SUCCEEDED(hr))
        {
            for (DWORD dwIndex = 0; dwIndex < dwNumElements; dwIndex++)
            {
                CComPtr<IPortableDeviceValues> pObjectProperties;
                hr = pValues->GetAt(dwIndex, &pObjectProperties);
                if (SUCCEEDED(hr))
                {
                    DisplayStringProperty(pObjectProperties, WPD_OBJECT_ID, L"WPD_OBJECT_ID");
                    DisplayErrorResultProperty(pObjectProperties, WPD_OBJECT_NAME, L"WPD_OBJECT_NAME");
                    printf("\n\n");
                }
                else
                {
                    printf("! Failed to get IPortableDeviceValues from IPortableDeviceValuesCollection at index '%d'\n", dwIndex);
                }
            }
        }
        return hr;
    }

    HRESULT __stdcall OnEnd(
        REFGUID Context,
        HRESULT hrStatus)
    {
        printf("** BULK Property operation ending, hrStatus = 0x%lx, Context = %ws **\n", hrStatus, (PWSTR)CGuidToString(Context));

        // This assumes that we are only performing a single operation
        // at a time, so no check is needed on the context when setting
        // the operation complete event.
        if (g_hBulkPropertyOperationEvent != NULL)
        {
            SetEvent(g_hBulkPropertyOperationEvent);
        }

        return S_OK;
    }

    ULONG m_cRef;
};

// Reads a set of properties for all objects.
void ReadContentPropertiesBulk(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                                       hr                = S_OK;
    GUID                                          guidContext       = GUID_NULL;
    CGetBulkValuesCallback*                       pCallback         = NULL;
    CComPtr<IPortableDeviceProperties>            pProperties;
    CComPtr<IPortableDevicePropertiesBulk>        pPropertiesBulk;
    CComPtr<IPortableDeviceValues>                pObjectProperties;
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDeviceKeyCollection>         pPropertiesToRead;
    CComPtr<IPortableDevicePropVariantCollection> pObjectIDs;


    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = pDevice->Content(&pContent);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = pProperties->QueryInterface(IID_PPV_ARGS(&pPropertiesBulk));
        if (FAILED(hr))
        {
            printf("This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pPropertiesToRead));
        if (FAILED(hr))
        {
            printf("! Failed to CoCreate IPortableDeviceKeyCollection to hold the property keys to read, hr = 0x%lx\n",hr);
        }
    }                              

    if (SUCCEEDED(hr))
    {
        // 5) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (pPropertiesToRead != NULL)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PARENT_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_NAME);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_FORMAT);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        pCallback = new (std::nothrow) CGetBulkValuesCallback();
        if (pCallback == NULL)
        {
            hr = E_OUTOFMEMORY;
            printf("! Failed to allocate CGetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call our helper function CreateIPortableDevicePropVariantCollectionWithAllObjectIDs
    // to enumerate and create an IPortableDevicePropVariantCollection with the object
    // identifiers needed to perform the bulk operation on.
    if (SUCCEEDED(hr))
    {
        hr = CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(pContent,
                                                                        &pObjectIDs);
    }


    // 8) Call QueueGetValuesByObjectList to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        hr = pPropertiesBulk->QueueGetValuesByObjectList(pObjectIDs,
                                                         pPropertiesToRead,
                                                         pCallback,
                                                         &guidContext);
        // 9) Call Start() to actually being the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                CloseHandle(g_hBulkPropertyOperationEvent);
                g_hBulkPropertyOperationEvent = NULL;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_hBulkPropertyOperationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                // Call Start() to actually being the Asynchronous bulk operation.
                hr = pPropertiesBulk->Start(guidContext);
                if(FAILED(hr))
                {
                    printf("! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                printf("! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            printf("! QueueGetValuesByObjectList Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_hBulkPropertyOperationEvent != NULL)
        {
            WaitForSingleObject(g_hBulkPropertyOperationEvent, INFINITE);
        }
    }

    if (pCallback != NULL)
    {
        pCallback->Release();
        pCallback = NULL;
    }

    // Cleanup any created global event handles before exiting..
    if (g_hBulkPropertyOperationEvent != NULL)
    {
        CloseHandle(g_hBulkPropertyOperationEvent);
        g_hBulkPropertyOperationEvent = NULL;
    }
}

// Writes a set of properties for all objects.
void WriteContentPropertiesBulk(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                                       hr                = S_OK;
    GUID                                          guidContext       = GUID_NULL;
    CSetBulkValuesCallback*                       pCallback         = NULL;
    CComPtr<IPortableDeviceProperties>            pProperties;
    CComPtr<IPortableDevicePropertiesBulk>        pPropertiesBulk;
    CComPtr<IPortableDeviceValues>                pObjectProperties;
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDeviceValuesCollection>      pPropertiesToWrite;
    CComPtr<IPortableDevicePropVariantCollection> pObjectIDs;
    DWORD                                         cObjectIDs        = 0;


    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = pDevice->Content(&pContent);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = pProperties->QueryInterface(IID_PPV_ARGS(&pPropertiesBulk));
        if (FAILED(hr))
        {
            printf("This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceValuesCollection interface to hold the the properties
    // we wish to write.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceValuesCollection,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pPropertiesToWrite));
        if (FAILED(hr))
        {
            printf("! Failed to CoCreate IPortableDeviceValuesCollection for bulk property values, hr = 0x%lx\n", hr);
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        pCallback = new (std::nothrow) CSetBulkValuesCallback();
        if (pCallback == NULL)
        {
            hr = E_OUTOFMEMORY;
            printf("! Failed to allocate CSetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call our helper function CreateIPortableDevicePropVariantCollectionWithAllObjectIDs
    // to enumerate and create an IPortableDevicePropVariantCollection with the object
    // identifiers needed to perform the bulk operation on.
    if (SUCCEEDED(hr))
    {
        hr = CreateIPortableDevicePropVariantCollectionWithAllObjectIDs(pContent,
                                                                        &pObjectIDs);
    }

    if (SUCCEEDED(hr))
    {
        hr = pObjectIDs->GetCount(&cObjectIDs);
        if (FAILED(hr))
        {
            printf("! Failed to get number of objectIDs from IPortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
        }
    }

    // 8) Iterate through object list and add appropriate IPortableDeviceValues to collection
    if (SUCCEEDED(hr))
    {
        for(DWORD dwIndex = 0; (dwIndex < cObjectIDs) && (hr == S_OK); dwIndex++)
        {
            CComPtr<IPortableDeviceValues>  pValues;
            PROPVARIANT                     pv = {0};

            PropVariantInit(&pv);
            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pValues));
            if (FAILED(hr))
            {
                printf("! Failed to CoCreate CLSID_PortableDeviceValues, hr = 0x%lx\n", hr);
            }

            // Get the Object ID whose properties we will set
            if (hr == S_OK)
            {
                hr = pObjectIDs->GetAt(dwIndex, &pv);
                if (FAILED(hr))
                {
                    printf("! Failed to get next Object ID from list, hr = 0x%lx\n", hr);
                }
            }

            // Save them into the IPortableDeviceValues so the driver knows which object this proeprty set belongs to
            if (hr == S_OK)
            {
                hr = pValues->SetStringValue(WPD_OBJECT_ID, pv.pwszVal);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_ID, hr = 0x%lx\n", hr);
                }
            }

            // Set the new values.  In this sample, we attempt to set the name property.
            if (hr == S_OK)
            {
                CAtlStringW strValue;
                strValue.Format(L"NewName%d", dwIndex);

                hr = pValues->SetStringValue(WPD_OBJECT_NAME, strValue.GetString());
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
                }
            }

            // Add this property set to the collection
            if (hr == S_OK)
            {
                hr = pPropertiesToWrite->Add(pValues);
                if (FAILED(hr))
                {
                    printf("! Failed to add values to collection, hr = 0x%lx\n", hr);
                }
            }
            PropVariantClear(&pv);
        }
    }

    // 9) Call QueueSetValuesByObjectList to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        hr = pPropertiesBulk->QueueSetValuesByObjectList(pPropertiesToWrite,
                                                         pCallback,
                                                         &guidContext);
        // 10) Call Start() to actually being the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                CloseHandle(g_hBulkPropertyOperationEvent);
                g_hBulkPropertyOperationEvent = NULL;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_hBulkPropertyOperationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                // Call Start() to actually being the Asynchronous bulk operation.
                hr = pPropertiesBulk->Start(guidContext);
                if(FAILED(hr))
                {
                    printf("! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                printf("! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            printf("! QueueSetValuesByObjectList Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_hBulkPropertyOperationEvent != NULL)
        {
            WaitForSingleObject(g_hBulkPropertyOperationEvent, INFINITE);
        }
    }

    if (pCallback != NULL)
    {
        pCallback->Release();
        pCallback = NULL;
    }

    // Cleanup any created global event handles before exiting..
    if (g_hBulkPropertyOperationEvent != NULL)
    {
        CloseHandle(g_hBulkPropertyOperationEvent);
        g_hBulkPropertyOperationEvent = NULL;
    }
}

// Reads a set of properties for all objects of a particular format.
void ReadContentPropertiesBulkFilteringByFormat(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                                       hr                = S_OK;
    GUID                                          guidContext       = GUID_NULL;
    CGetBulkValuesCallback*                       pCallback         = NULL;
    CComPtr<IPortableDeviceProperties>            pProperties;
    CComPtr<IPortableDevicePropertiesBulk>        pPropertiesBulk;
    CComPtr<IPortableDeviceValues>                pObjectProperties;
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDeviceKeyCollection>         pPropertiesToRead;


    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = pDevice->Content(&pContent);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 3) Check to see if the driver supports BULK property operations by call QueryInterface
    // on the IPortableDeviceProperties interface for IPortableDevicePropertiesBulk
    if (SUCCEEDED(hr))
    {
        hr = pProperties->QueryInterface(IID_PPV_ARGS(&pPropertiesBulk));
        if (FAILED(hr))
        {
            printf("This driver does not support BULK property operations.\n");
        }
    }

    // 4) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pPropertiesToRead));
        if (FAILED(hr))
        {
            printf("! Failed to CoCreate IPortableDeviceKeyCollection to hold the property keys to read, hr = 0x%lx\n",hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // 5) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (pPropertiesToRead != NULL)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PARENT_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_NAME);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_FORMAT);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }
        }
    }

    // 6) Create an instance of the IPortableDevicePropertiesBulkCallback object.
    if (SUCCEEDED(hr))
    {
        pCallback = new (std::nothrow) CGetBulkValuesCallback();
        if (pCallback == NULL)
        {
            hr = E_OUTOFMEMORY;
            printf("! Failed to allocate CGetBulkValuesCallback, hr = 0x%lx\n", hr);
        }
    }

    // 7) Call QueueGetValuesByObjectFormat to initialize the Asynchronous
    // property operation.
    if (SUCCEEDED(hr))
    {
        hr = pPropertiesBulk->QueueGetValuesByObjectFormat(WPD_OBJECT_FORMAT_WMA,
                                                           WPD_DEVICE_OBJECT_ID,
                                                           100,
                                                           pPropertiesToRead,
                                                           pCallback,
                                                           &guidContext);
        // 9) Call Start() to actually being the property operation
        if(SUCCEEDED(hr))
        {
            // Cleanup any previously created global event handles.
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                CloseHandle(g_hBulkPropertyOperationEvent);
                g_hBulkPropertyOperationEvent = NULL;
            }

            // In order to create a simpler to follow example we create and wait infinitly
            // for the bulk property operation to complete and ignore any errors.
            // Production code should be written in a more robust manner.
            // Create the global event handle to wait on for the bulk operation
            // to complete.
            g_hBulkPropertyOperationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (g_hBulkPropertyOperationEvent != NULL)
            {
                // Call Start() to actually being the Asynchronous bulk operation.
                hr = pPropertiesBulk->Start(guidContext);
                if(FAILED(hr))
                {
                    printf("! Failed to start property operation, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                printf("! Failed to create the global event handle to wait on for the bulk operation. Aborting operation.\n");
            }
        }
        else
        {
            printf("! QueueGetValuesByObjectFormat Failed, hr = 0x%lx\n", hr);
        }
    }

    // In order to create a simpler to follow example we will wait infinitly for the operation
    // to complete and ignore any errors.  Production code should be written in a more
    // robust manner.
    if (SUCCEEDED(hr))
    {
        if (g_hBulkPropertyOperationEvent != NULL)
        {
            WaitForSingleObject(g_hBulkPropertyOperationEvent, INFINITE);
        }
    }

    if (pCallback != NULL)
    {
        pCallback->Release();
        pCallback = NULL;
    }

    // Cleanup any created global event handles before exiting..
    if (g_hBulkPropertyOperationEvent != NULL)
    {
        CloseHandle(g_hBulkPropertyOperationEvent);
        g_hBulkPropertyOperationEvent = NULL;
    }
}

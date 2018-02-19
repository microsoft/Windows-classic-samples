// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>


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
    IPortableDeviceService*    pService)
{
    if (pService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    HRESULT                               hr              = S_OK;
    WCHAR                                 szSelection[81] = {0};
    CComPtr<IPortableDeviceProperties>    pProperties;
    CComPtr<IPortableDeviceValues>        pObjectProperties;
    CComPtr<IPortableDeviceContent2>      pContent;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;

    // Prompt user to enter an object identifier on the device to read properties from.
    printf("Enter the identifier of the object you wish to read properties from.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting property reading\n");
    }

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDeviceService interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pService->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent2 from IPortableDeviceService, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent2 interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n",hr);
        }
    }

    // 3) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
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
            hrTemp = pPropertiesToRead->Add(PKEY_GenericObj_ParentID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add PKEY_GenericObj_ParentID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(PKEY_GenericObj_Name);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add PKEY_GenericObj_Name to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(PKEY_GenericObj_PersistentUID);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add PKEY_GenericObj_PersistentUID to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

            hrTemp = pPropertiesToRead->Add(PKEY_GenericObj_ObjectFormat);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add PKEY_GenericObj_ObjectFormat to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }

        }
    }

    // 5) Call GetValues() passing the collection of specified PROPERTYKEYs.
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

    // 6) Display the returned property values to the user
    if (SUCCEEDED(hr))
    {
        DisplayStringProperty(pObjectProperties, PKEY_GenericObj_ParentID,        NAME_GenericObj_ParentID);
        DisplayStringProperty(pObjectProperties, PKEY_GenericObj_Name,            NAME_GenericObj_Name);
        DisplayStringProperty(pObjectProperties, PKEY_GenericObj_PersistentUID,   NAME_GenericObj_PersistentUID);
        DisplayGuidProperty  (pObjectProperties, PKEY_GenericObj_ObjectFormat,    NAME_GenericObj_ObjectFormat);
    }
}


// Writes properties on the user specified object.
void WriteContentProperties(
    IPortableDeviceService*    pService)
{
    if (pService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    HRESULT                               hr                  = S_OK;
    WCHAR                                 szSelection[81]     = {0};
    WCHAR                                 szNewObjectName[81] = {0};
    CComPtr<IPortableDeviceProperties>    pProperties;
    CComPtr<IPortableDeviceContent2>      pContent;
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

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDeviceService interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pService->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent2 from IPortableDeviceService, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent2 interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n",hr);
        }
    }

    // 3) Check the property attributes to see if we can write/change the NAME_GenericObj_Name property.
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetPropertyAttributes(szSelection,
                                                PKEY_GenericObj_Name,
                                                &pAttributes);
        if (SUCCEEDED(hr))
        {
            hr = pAttributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &bCanWrite);
            if (SUCCEEDED(hr))
            {
                if (bCanWrite)
                {
                    printf("The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for PKEY_GenericObj_Name reports TRUE\nThis means that the property can be changed/updated\n\n");
                }
                else
                {
                    printf("The attribute WPD_PROPERTY_ATTRIBUTE_CAN_WRITE for PKEY_GenericObj_Name reports FALSE\nThis means that the property cannot be changed/updated\n\n");
                }
            }
            else
            {
                printf("! Failed to get the WPD_PROPERTY_ATTRIBUTE_CAN_WRITE value for PKEY_GenericObj_Name on object '%ws', hr = 0x%lx\n", szSelection, hr);
            }
        }
        else
        {
            printf("! Failed to retrieve property attributes for object '%ws', hr = 0x%lx\n", szSelection, hr);
        }
        
    }

    // 4) Prompt the user for the new value of the NAME_GenericObj_Name property only if the property attributes report
    // that it can be changed/updated.
    if (bCanWrite)
    {
        printf("Enter the new PKEY_GenericObj_Name property for the object '%ws'.\n>",szSelection);
        hr = StringCbGetsW(szNewObjectName,sizeof(szNewObjectName));
        if (FAILED(hr))
        {
            printf("An invalid PKEY_GenericObj_Name was specified, aborting property writing\n");
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
                    hr = pObjectPropertiesToWrite->SetStringValue(PKEY_GenericObj_Name, szNewObjectName);
                    if (FAILED(hr))
                    {
                        printf("! Failed to add PKEY_GenericObj_Name to IPortableDeviceValues, hr= 0x%lx\n", hr);
                    }
                }
            }
        }

        // 6) Call SetValues() passing the collection of specified PROPERTYKEYs.
        if (SUCCEEDED(hr))
        {
            hr = pProperties->SetValues(szSelection,               // The object whose properties we are reading
                                        pObjectPropertiesToWrite,   // The properties we want to read
                                        &pPropertyWriteResults);    // Driver supplied property result values for the property read operation
            if (FAILED(hr))
            {
                printf("! Failed to set properties for object '%ws', hr= 0x%lx\n", szSelection, hr);
            }
            else
            {
                printf("The PKEY_GenericObj_Name property on object '%ws' was written successfully (Read the properties again to see the updated value)\n", szSelection);
            }
        }
    }
}

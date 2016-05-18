// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

void DisplayVarType(
    VARTYPE vartype)
{
    switch(vartype)
    {
        case VT_BOOL:
            printf("VT_BOOL");
            break;
        case VT_UI4:
            printf("VT_UI4");
            break;
        case VT_R4:
            printf("VT_R4");
            break;
        case VT_UI8:
            printf("VT_UI8");
            break;
        case VT_UI1|VT_VECTOR:
            printf("VT_UI1|VT_VECTOR");
            break;
        case VT_DATE:
            printf("VT_DATE");
            break;
        case VT_FILETIME:
            printf("VT_FILETIME");
            break;
        case VT_LPWSTR:
            printf("VT_LPWSTR");
            break;
        case VT_CLSID:
            printf("VT_CLSID");
            break;
        case VT_ERROR:
            printf("VT_ERROR");
            break;
        case VT_UNKNOWN:
            printf("VT_UNKNOWN");
            break;
        // add more VARTYPEs as needed
        default:
            printf("%d", vartype);
            break;
    }
}

void DisplayParameterForm(
    DWORD   dwForm) 
{
    switch(static_cast<WpdParameterAttributeForm>(dwForm))
    {
        case WPD_PARAMETER_ATTRIBUTE_FORM_RANGE:
            printf("Range");
            break;
            
        case WPD_PARAMETER_ATTRIBUTE_FORM_ENUMERATION:
            printf("Enumeration");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_REGULAR_EXPRESSION:
            printf("Regular Expression");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_OBJECT_IDENTIFIER:
            printf("Object Identifier");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED:
        default:
            printf("Unspecified");
            break;
    }
}

// Display the basic event options.
void DisplayEventOptions(
    IPortableDeviceValues* pOptions)
{
    printf("\tEvent Options:\n");
    // Read the WPD_EVENT_OPTION_IS_BROADCAST_EVENT value to see if the event is
    // a broadcast event. If the read fails, assume FALSE
    BOOL  bIsBroadcastEvent = FALSE;
    pOptions->GetBoolValue(WPD_EVENT_OPTION_IS_BROADCAST_EVENT, &bIsBroadcastEvent);
    printf("\tWPD_EVENT_OPTION_IS_BROADCAST_EVENT = %ws\n",bIsBroadcastEvent ? L"TRUE" : L"FALSE");
}


// Display the event parameters.
void DisplayEventParameters(
    IPortableDeviceServiceCapabilities* pCapabilities,
    REFGUID                             Event,
    IPortableDeviceKeyCollection*       pParameters)
{
    DWORD   dwNumParameters = 0;

    // Get the number of parameters for this event.
    HRESULT hr = pParameters->GetCount(&dwNumParameters);
    if (FAILED(hr))
    {
        printf("! Failed to get number of parameters, hr = 0x%lx\n",hr);
    }

    printf("\n\t%d Event Parameters:\n", dwNumParameters);

    // Loop through each parameter and display it
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumParameters; dwIndex++)
        {
            PROPERTYKEY parameter;
            hr = pParameters->GetAt(dwIndex, &parameter);

            if (SUCCEEDED(hr))
            {
                CComPtr<IPortableDeviceValues> pAttributes;

                // Display the parameter's Vartype and Form
                hr = pCapabilities->GetEventParameterAttributes(Event, parameter, &pAttributes);
                if (FAILED(hr))
                {
                    printf("! Failed to get the event parameter attributes, hr = 0x%lx\n",hr);
                }

                if (SUCCEEDED(hr))
                {
                    printf("\t\tPROPERTYKEY: %ws.%d\n", (PWSTR)CGuidToString(parameter.fmtid), parameter.pid);
            
                    DWORD dwAttributeVarType = 0;
                    DWORD dwAttributeForm    = WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED;

                    // Read the WPD_PARAMETER_ATTRIBUTE_VARTYPE value.
                    // If the read fails, we don't display it
                    hr = pAttributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_VARTYPE, &dwAttributeVarType);
                    if (SUCCEEDED(hr))
                    {
                        printf("\t\tVARTYPE: ");
                        DisplayVarType(static_cast<VARTYPE>(dwAttributeVarType));
                        printf("\n");
                    }

                    // Read the WPD_PARAMETER_ATTRIBUTE_FORM value.
                    // If the read fails, we don't display it
                    hr = pAttributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_FORM, &dwAttributeForm);
                    if (SUCCEEDED(hr))
                    {
                        printf("\t\tForm: ");
                        DisplayParameterForm(dwAttributeForm);
                        printf("\n");
                    }
                }
                
                printf("\n");
            }
        }
    }
}


// Display basic information about an event
void DisplayEvent(
    IPortableDeviceServiceCapabilities* pCapabilities,
    REFGUID                             Event)
{
    CComPtr<IPortableDeviceValues> pAttributes;

    // Get the event attributes which describe the event
    HRESULT hr = pCapabilities->GetEventAttributes(Event, &pAttributes);
    if (FAILED(hr))
    {
        printf("! Failed to get the event attributes, hr = 0x%lx\n",hr);
    }

    if (SUCCEEDED(hr))
    {
        PWSTR pszFormatName = NULL;
        CComPtr<IPortableDeviceValues>          pOptions;
        CComPtr<IPortableDeviceKeyCollection>   pParameters;

        // Display the name of the event if it is available. Otherwise, fall back to displaying the GUID.
        hr = pAttributes->GetStringValue(WPD_EVENT_ATTRIBUTE_NAME, &pszFormatName);
        if (SUCCEEDED(hr))
        {
            printf("%ws\n", pszFormatName);
        }
        else
        {
            printf("%ws\n", (PWSTR)CGuidToString(Event));
        }       

        // Display the event options
        hr = pAttributes->GetIPortableDeviceValuesValue(WPD_EVENT_ATTRIBUTE_OPTIONS, &pOptions);
        if (SUCCEEDED(hr))
        {
            DisplayEventOptions(pOptions);
        }
        else
        {
            printf("! Failed to get the event options, hr = 0x%lx\n", hr);
        }       

        // Display the event parameters
        hr = pAttributes->GetIPortableDeviceKeyCollectionValue(WPD_EVENT_ATTRIBUTE_PARAMETERS, &pParameters);
        if (SUCCEEDED(hr))
        {
            DisplayEventParameters(pCapabilities, Event, pParameters);
        }
        else
        {
            printf("! Failed to get the event parameters, hr = 0x%lx\n", hr);
        }       
        
        CoTaskMemFree(pszFormatName);
        pszFormatName = NULL;
    }
}


// Display basic information about a format
void DisplayFormat(
    IPortableDeviceServiceCapabilities* pCapabilities,
    REFGUID                             Format)
{
    CComPtr<IPortableDeviceValues> pAttributes;

    HRESULT hr = pCapabilities->GetFormatAttributes(Format, &pAttributes);

    if (SUCCEEDED(hr))
    {
        PWSTR pszFormatName = NULL;
        hr = pAttributes->GetStringValue(WPD_FORMAT_ATTRIBUTE_NAME, &pszFormatName);

        // Display the name of the format if it is available, otherwise fall back to displaying the GUID.
        if (SUCCEEDED(hr))
        {
            printf("%ws", pszFormatName);
        }
        else
        {
            printf("%ws", (PWSTR)CGuidToString(Format));
        }       

        CoTaskMemFree(pszFormatName);
        pszFormatName = NULL;
    }
}


// List all supported formats on the service
void ListSupportedFormats(
    IPortableDeviceService* pService)
{
    HRESULT hr              = S_OK;
    DWORD   dwNumFormats    = 0;
    CComPtr<IPortableDeviceServiceCapabilities>     pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pFormats;

    if (pService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    hr = pService->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n",hr);
    }

    // Get all formats supported by the service.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetSupportedFormats(&pFormats);
        if (FAILED(hr))
        {
            printf("! Failed to get supported formats from the service, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of supported formats found on the service.
    if (SUCCEEDED(hr))
    {
        hr = pFormats->GetCount(&dwNumFormats);
        if (FAILED(hr))
        {
            printf("! Failed to get number of supported formats, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Supported Formats Found on the service\n\n", dwNumFormats);

    // Loop through each format and display it
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumFormats; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pFormats->GetAt(dwIndex, &pv);

            if (SUCCEEDED(hr))
            {
                // We have a format.  It is assumed that
                // formats are returned as VT_CLSID VarTypes.
                if (pv.puuid != NULL)
                {
                    DisplayFormat(pCapabilities, *pv.puuid);
                    printf("\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
}


// List all supported events on the service
void ListSupportedEvents(
    IPortableDeviceService* pService)
{
    HRESULT hr          = S_OK;
    DWORD   dwNumEvents = 0;
    CComPtr<IPortableDeviceServiceCapabilities>     pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pEvents;

    if (pService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    hr = pService->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n",hr);
    }

    // Get all events supported by the service.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetSupportedEvents(&pEvents);
        if (FAILED(hr))
        {
            printf("! Failed to get supported events from the service, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of supported events found on the service.
    if (SUCCEEDED(hr))
    {
        hr = pEvents->GetCount(&dwNumEvents);
        if (FAILED(hr))
        {
            printf("! Failed to get number of supported events, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Supported Events Found on the service\n\n", dwNumEvents);

    // Loop through each event and display it
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
                    DisplayEvent(pCapabilities, *pv.puuid);
                    printf("\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
}


// List the abstract services implemented by the current service.
// Abstract services represent functionality that a service supports, and are used by applications to discover
// that a service supports formats, properties and methods associated with that functionality.
// For example, a Contacts service may implement an abstract service that represents the synchronization models supported 
// on that device, such as the FullEnumerationSync or AnchorSync Device Services
void ListAbstractServices(
    IPortableDeviceService* pService)
{
    HRESULT hr              = S_OK;
    DWORD   dwNumServices   = 0;
    CComPtr<IPortableDeviceServiceCapabilities>     pCapabilities;
    CComPtr<IPortableDevicePropVariantCollection>   pAbstractServices;

    if (pService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    hr = pService->Capabilities(&pCapabilities);
    if (FAILED(hr))
    {
        printf("! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n",hr);
    }

    // Get all abstract services implemented by the service.
    if (SUCCEEDED(hr))
    {
        hr = pCapabilities->GetInheritedServices(WPD_SERVICE_INHERITANCE_IMPLEMENTATION, &pAbstractServices);
        if (FAILED(hr))
        {
            printf("! Failed to get the implememted services from the service, hr = 0x%lx\n",hr);
        }
    }

    // Get the number of abstract services implemented by the service.
    if (SUCCEEDED(hr))
    {
        hr = pAbstractServices->GetCount(&dwNumServices);
        if (FAILED(hr))
        {
            printf("! Failed to get number of abstract services, hr = 0x%lx\n",hr);
        }
    }

    printf("\n%d Abstract Services Implemented by the service\n\n", dwNumServices);

    // Loop through each abstract service and display it
    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwNumServices; dwIndex++)
        {
            PROPVARIANT pv = {0};
            PropVariantInit(&pv);
            hr = pAbstractServices->GetAt(dwIndex, &pv);

            if (SUCCEEDED(hr))
            {
                if (*pv.puuid == SERVICE_FullEnumSync)
                {
                    printf("\t%ws\n", NAME_FullEnumSyncSvc);
                }
                else if (*pv.puuid == SERVICE_AnchorSync)
                {
                    printf("\t%ws\n", NAME_AnchorSyncSvc);
                }
                else
                {
                    printf("\t%ws\n", (PWSTR)CGuidToString(*pv.puuid));
                }
            }

            PropVariantClear(&pv);
        }
    }    
}

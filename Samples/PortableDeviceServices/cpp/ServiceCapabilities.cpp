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
            wprintf(L"VT_BOOL");
            break;
        case VT_UI4:
            wprintf(L"VT_UI4");
            break;
        case VT_R4:
            wprintf(L"VT_R4");
            break;
        case VT_UI8:
            wprintf(L"VT_UI8");
            break;
        case VT_UI1|VT_VECTOR:
            wprintf(L"VT_UI1|VT_VECTOR");
            break;
        case VT_DATE:
            wprintf(L"VT_DATE");
            break;
        case VT_FILETIME:
            wprintf(L"VT_FILETIME");
            break;
        case VT_LPWSTR:
            wprintf(L"VT_LPWSTR");
            break;
        case VT_CLSID:
            wprintf(L"VT_CLSID");
            break;
        case VT_ERROR:
            wprintf(L"VT_ERROR");
            break;
        case VT_UNKNOWN:
            wprintf(L"VT_UNKNOWN");
            break;
        case VT_NULL:
            wprintf(L"VT_NULL");
            break;
        // add more VARTYPEs as needed
        default:
            wprintf(L"%u", vartype);
            break;
    }
}

void DisplayParameterForm(
    DWORD   form) 
{
    switch(static_cast<WpdParameterAttributeForm>(form))
    {
        case WPD_PARAMETER_ATTRIBUTE_FORM_RANGE:
            wprintf(L"Range");
            break;
            
        case WPD_PARAMETER_ATTRIBUTE_FORM_ENUMERATION:
            wprintf(L"Enumeration");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_REGULAR_EXPRESSION:
            wprintf(L"Regular Expression");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_OBJECT_IDENTIFIER:
            wprintf(L"Object Identifier");
            break;

        case WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED:
        default:
            wprintf(L"Unspecified");
            break;
    }
}

//<SnippetDisplayEventOptions1>
// Display the basic event options.
void DisplayEventOptions(
    _In_ IPortableDeviceValues* eventOptions)
{
    wprintf(L"\tEvent Options:\n");
    // Read the WPD_EVENT_OPTION_IS_BROADCAST_EVENT value to see if the event is
    // a broadcast event. If the read fails, assume FALSE
    BOOL  isBroadcastEvent = FALSE;
    eventOptions->GetBoolValue(WPD_EVENT_OPTION_IS_BROADCAST_EVENT, &isBroadcastEvent);
    wprintf(L"\tWPD_EVENT_OPTION_IS_BROADCAST_EVENT = %ws\n", isBroadcastEvent ? L"TRUE" : L"FALSE");
}
//</SnippetDisplayEventOptions1>

//<SnippetDisplayEventParameters1>
// Display the event parameters.
void DisplayEventParameters(
    _In_ IPortableDeviceServiceCapabilities* capabilities,
    _In_ REFGUID                             event,
    _In_ IPortableDeviceKeyCollection*       parameters)
{
    // Get the number of parameters for this event.
    DWORD   numParameters = 0;
    HRESULT hr = parameters->GetCount(&numParameters);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get number of parameters, hr = 0x%lx\n", hr);
    }
    else
    {
        wprintf(L"\n\t%u Event Parameters:\n", numParameters);

        // Loop through each parameter and display it
        for (DWORD index = 0; index < numParameters; index++)
        {
            PROPERTYKEY parameter;
            hr = parameters->GetAt(index, &parameter);

            if (SUCCEEDED(hr))
            {
                // Display the parameter's Vartype and Form
                ComPtr<IPortableDeviceValues> attributes;

                hr = capabilities->GetEventParameterAttributes(event, parameter, &attributes);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to get the event parameter attributes, hr = 0x%lx\n", hr);
                }
                else
                {
                    wprintf(L"\t\tPROPERTYKEY: %ws.%u\n", (PCWSTR)CGuidToString(parameter.fmtid), parameter.pid);

                    DWORD attributeVarType = 0;
                    DWORD attributeForm    = WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED;

                    // Read the WPD_PARAMETER_ATTRIBUTE_VARTYPE value.
                    // If the read fails, we don't display it
                    hr = attributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_VARTYPE, &attributeVarType);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\t\tVARTYPE: ");
                        DisplayVarType(static_cast<VARTYPE>(attributeVarType));
                        wprintf(L"\n");
                    }

                    // Read the WPD_PARAMETER_ATTRIBUTE_FORM value.
                    // If the read fails, we don't display it
                    hr = attributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_FORM, &attributeForm);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\t\tForm: ");
                        DisplayParameterForm(attributeForm);
                        wprintf(L"\n");
                    }
                }
                
                wprintf(L"\n");
            }
        }
    }
}
//</SnippetDisplayEventParameters1>

//<SnippetDisplayEvent1>
// Display basic information about an event
void DisplayEvent(
    _In_ IPortableDeviceServiceCapabilities* capabilities,
    _In_ REFGUID                             event)
{
    ComPtr<IPortableDeviceValues> attributes;

    // Get the event attributes which describe the event
    HRESULT hr = capabilities->GetEventAttributes(event, &attributes);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get the event attributes, hr = 0x%lx\n", hr);
    }
    else
    {
        PWSTR formatName = nullptr;
        ComPtr<IPortableDeviceValues>          options;
        ComPtr<IPortableDeviceKeyCollection>   parameters;

        // Display the name of the event if it is available. Otherwise, fall back to displaying the GUID.
        hr = attributes->GetStringValue(WPD_EVENT_ATTRIBUTE_NAME, &formatName);
        if (SUCCEEDED(hr))
        {
            wprintf(L"%ws\n", formatName);
        }
        else
        {
            wprintf(L"%ws\n", (PCWSTR)CGuidToString(event));
        }

        // Display the event options, if any
        hr = attributes->GetIPortableDeviceValuesValue(WPD_EVENT_ATTRIBUTE_OPTIONS, &options);
        if (SUCCEEDED(hr))
        {
            DisplayEventOptions(options.Get());
        }
        else
        {
            wprintf(L"! Failed to get the event options, hr = 0x%lx\n", hr);
        }

        // Display the event parameters, if any
        hr = attributes->GetIPortableDeviceKeyCollectionValue(WPD_EVENT_ATTRIBUTE_PARAMETERS, &parameters);
        if (SUCCEEDED(hr))
        {
            DisplayEventParameters(capabilities, event, parameters.Get());
        }
        else
        {
            wprintf(L"! Failed to get the event parameters, hr = 0x%lx\n", hr);
        }

        CoTaskMemFree(formatName);
        formatName = nullptr;
    }
}
//</SnippetDisplayEvent1>

//<SnippetDisplayFormat1>
// Display basic information about a format
void DisplayFormat(
    _In_ IPortableDeviceServiceCapabilities* capabilities,
    _In_ REFGUID                             format)
{
    ComPtr<IPortableDeviceValues> attributes;

    HRESULT hr = capabilities->GetFormatAttributes(format, &attributes);
    if (SUCCEEDED(hr))
    {
        PWSTR formatName = nullptr;
        hr = attributes->GetStringValue(WPD_FORMAT_ATTRIBUTE_NAME, &formatName);

        // Display the name of the format if it is available, otherwise fall back to displaying the GUID.
        if (SUCCEEDED(hr))
        {
            wprintf(L"%ws", formatName);
        }
        else
        {
            wprintf(L"%ws", (PCWSTR)CGuidToString(format));
        }

        CoTaskMemFree(formatName);
        formatName = nullptr;
    }
}
//</SnippetDisplayFormat1>
//<SnippetListSupportedFormats1>
// List all supported formats on the service
void ListSupportedFormats(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceServiceCapabilities>     capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   formats;
    DWORD   numFormats = 0;

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    HRESULT hr = service->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n", hr);
    }
    else
    {
        // Get all formats supported by the service.
        hr = capabilities->GetSupportedFormats(&formats);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get supported formats from the service, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of supported formats found on the service.
    if (SUCCEEDED(hr))
    {
        hr = formats->GetCount(&numFormats);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of supported formats, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each format and display it
    if (SUCCEEDED(hr))
    {
        wprintf(L"\n%u Supported Formats Found on the service\n\n", numFormats);

        for (DWORD index = 0; index < numFormats; index++)
        {
            PROPVARIANT format = {0};
            hr = formats->GetAt(index, &format);

            if (SUCCEEDED(hr))
            {
                // We have a format.  It is assumed that
                // formats are returned as VT_CLSID VarTypes.
                if (format.vt    == VT_CLSID && 
                    format.puuid != nullptr)
                {
                    DisplayFormat(capabilities.Get(), *format.puuid);
                    wprintf(L"\n");
                }
            }

            PropVariantClear(&format);
        }
    }
}
//</SnippetListSupportedFormats1>
//<SnippetListSupportedEvents1>
// List all supported events on the service
void ListSupportedEvents(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceServiceCapabilities>     capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   events;
    DWORD   numEvents = 0;

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    HRESULT hr = service->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n", hr);
    }

    // Get all events supported by the service.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetSupportedEvents(&events);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get supported events from the service, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of supported events found on the service.
    if (SUCCEEDED(hr))
    {
        hr = events->GetCount(&numEvents);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of supported events, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each event and display it
    if (SUCCEEDED(hr))
    {
        wprintf(L"\n%u Supported Events Found on the service\n\n", numEvents);

        for (DWORD index = 0; index < numEvents; index++)
        {
            PROPVARIANT event = {0};
            hr = events->GetAt(index, &event);

            if (SUCCEEDED(hr))
            {
                // We have an event.  It is assumed that
                // events are returned as VT_CLSID VarTypes.
                if (event.vt    == VT_CLSID && 
                    event.puuid != nullptr)
                {
                    DisplayEvent(capabilities.Get(), *event.puuid);
                    wprintf(L"\n");
                }
            }

            PropVariantClear(&event);
        }
    }
}
//</SnippetListSupportedEvents1>


// List the abstract services implemented by the current service.
// Abstract services represent functionality that a service supports, and are used by applications to discover
// that a service supports formats, properties and methods associated with that functionality.
// For example, a Contacts service may implement an abstract service that represents the synchronization models supported 
// on that device, such as the FullEnumerationSync or AnchorSync Device Services
void ListAbstractServices(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceServiceCapabilities>     capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   abstractServices;
    DWORD   numServices = 0;

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    HRESULT hr = service->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n", hr);
    }

    // Get all abstract services implemented by the service.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetInheritedServices(WPD_SERVICE_INHERITANCE_IMPLEMENTATION, &abstractServices);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get the implememted services from the service, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of abstract services implemented by the service.
    if (SUCCEEDED(hr))
    {
        hr = abstractServices->GetCount(&numServices);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of abstract services, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each abstract service and display it
    if (SUCCEEDED(hr))
    {
        wprintf(L"\n%u Abstract Services Implemented by the service\n\n", numServices);

        for (DWORD index = 0; index < numServices; index++)
        {
            PROPVARIANT abstractService = {0};
            hr = abstractServices->GetAt(index, &abstractService);

            if (SUCCEEDED(hr))
            {
                if (*abstractService.puuid == SERVICE_FullEnumSync)
                {
                    wprintf(L"\t%ws\n", NAME_FullEnumSyncSvc);
                }
                else if (*abstractService.puuid == SERVICE_AnchorSync)
                {
                    wprintf(L"\t%ws\n", NAME_AnchorSyncSvc);
                }
                else
                {
                    wprintf(L"\t%ws\n", (PCWSTR)CGuidToString(*abstractService.puuid));
                }
            }

            PropVariantClear(&abstractService);
        }
    }
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

HANDLE g_methodCompleteEvent = nullptr;

void DisplayParameterUsage(
    DWORD   usage)
{
    switch(static_cast<WPD_PARAMETER_USAGE_TYPES>(usage))
    {
        case WPD_PARAMETER_USAGE_RETURN:
            wprintf(L"Return Value");
            break;
        case WPD_PARAMETER_USAGE_IN:
            wprintf(L"Input Parameter");
            break;
        case WPD_PARAMETER_USAGE_OUT:
            wprintf(L"Output Parameter");
            break;
        case WPD_PARAMETER_USAGE_INOUT:
            wprintf(L"Input/Output Parameter");
            break;
        default:
            wprintf(L"Unknown Parameter Usage");
            break;
    }
}

//<SnippetDisplayMethodAccess1>
void DisplayMethodAccess(
    DWORD   access)
{
    switch(static_cast<WPD_COMMAND_ACCESS_TYPES>(access))
    {
        case WPD_COMMAND_ACCESS_READ:
            wprintf(L"Read");
            break;
            
        case WPD_COMMAND_ACCESS_READWRITE:
            wprintf(L"Read/Write");
            break;

        default:
            wprintf(L"Unknown Access");
            break;
    }
}
//</SnippetDisplayMethodAccess1>

//<SnippetDisplayMethodParameters1>
// Display the method parameters.
void DisplayMethodParameters(
    _In_ IPortableDeviceServiceCapabilities* capabilities,
    _In_ REFGUID                             method,
    _In_ IPortableDeviceKeyCollection*       parameters)
{
    DWORD   numParameters = 0;

    // Get the number of parameters for this event.
    HRESULT hr = parameters->GetCount(&numParameters);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get number of parameters, hr = 0x%lx\n", hr);
    }
    else
    {
        wprintf(L"\n\t%u Method Parameters:\n", numParameters);

        // Loop through each parameter and display it
        for (DWORD index = 0; index < numParameters; index++)
        {
            PROPERTYKEY parameter;
            hr = parameters->GetAt(index, &parameter);

            if (SUCCEEDED(hr))
            {
                ComPtr<IPortableDeviceValues> attributes;

                // Display the parameter's Name, Usage, Vartype, and Form
                hr = capabilities->GetMethodParameterAttributes(method, parameter, &attributes);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to get the method parameter attributes, hr = 0x%lx\n", hr);
                }
                else
                {
                    PWSTR   parameterName    = nullptr;
                    DWORD   attributeVarType = 0;
                    DWORD   attributeForm    = WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED;
                    DWORD   attributeUsage   = (DWORD)-1;

                    hr = attributes->GetStringValue(WPD_PARAMETER_ATTRIBUTE_NAME, &parameterName);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\t\tName: %ws\n", parameterName);
                    }
                    else
                    {
                        wprintf(L"! Failed to get the method parameter name, hr = 0x%lx\n", hr);
                    }

                    // Read the WPD_PARAMETER_ATTRIBUTE_USAGE value, if specified. 
                    hr = attributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_USAGE, &attributeUsage);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\t\tUsage: ");
                        DisplayParameterUsage(attributeUsage);
                        wprintf(L"\n");
                    }
                    else
                    {
                        wprintf(L"! Failed to get the method parameter usage, hr = 0x%lx\n", hr);
                    }

                    hr = attributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_VARTYPE, &attributeVarType);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"\t\tVARTYPE: ");
                        DisplayVarType(static_cast<VARTYPE>(attributeVarType));
                        wprintf(L"\n");
                    }
                    else
                    {
                        wprintf(L"! Failed to get the method parameter VARTYPE, hr = 0x%lx\n", hr);
                    }

                    // Read the WPD_PARAMETER_ATTRIBUTE_FORM value.
                    hr = attributes->GetUnsignedIntegerValue(WPD_PARAMETER_ATTRIBUTE_FORM, &attributeForm);
                    if (FAILED(hr))
                    {
                        // If the read fails, assume WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED
                        attributeForm = WPD_PARAMETER_ATTRIBUTE_FORM_UNSPECIFIED;
                        hr = S_OK;
                    }

                    wprintf(L"\t\tForm: ");
                    DisplayParameterForm(attributeForm);
                    wprintf(L"\n");

                    CoTaskMemFree(parameterName);
                    parameterName = nullptr;
                }
                
                wprintf(L"\n");
            }
        }
    }
}
//</SnippetDisplayMethodParameters1>
//<SnippetDisplayMethod1>
// Display basic information about a method
void DisplayMethod(
    _In_ IPortableDeviceServiceCapabilities* capabilities,
    _In_ REFGUID                             method)
{
    ComPtr<IPortableDeviceValues> attributes;

    // Get the method attributes which describe the method
    HRESULT hr = capabilities->GetMethodAttributes(method, &attributes);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get the method attributes, hr = 0x%lx\n", hr);
    }

    if (SUCCEEDED(hr))
    {
        PWSTR   methodName       = nullptr;
        DWORD   methodAccess     = WPD_COMMAND_ACCESS_READ;
        GUID    associatedFormat = GUID_NULL;

        ComPtr<IPortableDeviceKeyCollection>   parameters;

        // Display the name of the method if available. Otherwise, fall back to displaying the GUID.
        hr = attributes->GetStringValue(WPD_METHOD_ATTRIBUTE_NAME, &methodName);
        if (SUCCEEDED(hr))
        {
            wprintf(L"%ws", methodName);
        }
        else
        {
            wprintf(L"%ws", (PCWSTR)CGuidToString(method));
        }

        // Display the method access if available, otherwise default to WPD_COMMAND_ACCESS_READ access
        hr = attributes->GetUnsignedIntegerValue(WPD_METHOD_ATTRIBUTE_ACCESS, &methodAccess);
        if (FAILED(hr))
        {
            methodAccess = WPD_COMMAND_ACCESS_READ;
            hr = S_OK;
        }
        wprintf(L"\n\tAccess: ");
        DisplayMethodAccess(methodAccess);

        // Display the associated format if specified.
        // Methods that have an associated format may only be supported for that format.
        // Methods that don't have associated formats generally apply to the entire service.
        hr = attributes->GetGuidValue(WPD_METHOD_ATTRIBUTE_ASSOCIATED_FORMAT, &associatedFormat);
        if (SUCCEEDED(hr))
        {
            wprintf(L"\n\tAssociated Format: ");
            DisplayFormat(capabilities, associatedFormat);
        }

        // Display the method parameters, if available
        hr = attributes->GetIPortableDeviceKeyCollectionValue(WPD_METHOD_ATTRIBUTE_PARAMETERS, &parameters);
        if (SUCCEEDED(hr))
        {
            DisplayMethodParameters(capabilities, method, parameters.Get());
        }
        
        CoTaskMemFree(methodName);
        methodName = nullptr;

    }
}
//</SnippetDisplayMethod1>
//<SnippetListSupportedMethods1>
// List all supported methods on the service
void ListSupportedMethods(
    _In_ IPortableDeviceService* service)
{
    DWORD   dwNumMethods    = 0;
    ComPtr<IPortableDeviceServiceCapabilities>     capabilities;
    ComPtr<IPortableDevicePropVariantCollection>   methods;

    // Get an IPortableDeviceServiceCapabilities interface from the IPortableDeviceService interface to
    // access the service capabilities-specific methods.
    HRESULT hr = service->Capabilities(&capabilities);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceCapabilities from IPortableDeviceService, hr = 0x%lx\n", hr);
    }

    // Get all methods supported by the service.
    if (SUCCEEDED(hr))
    {
        hr = capabilities->GetSupportedMethods(&methods);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get supported methods from the service, hr = 0x%lx\n", hr);
        }
    }

    // Get the number of supported methods found on the service.
    if (SUCCEEDED(hr))
    {
        hr = methods->GetCount(&dwNumMethods);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of supported methods, hr = 0x%lx\n", hr);
        }
    }

    // Loop through each method and display it
    if (SUCCEEDED(hr))
    {
        wprintf(L"\n%u Supported Methods Found on the service\n\n", dwNumMethods);

        for (DWORD index = 0; index < dwNumMethods; index++)
        {
            PROPVARIANT pv = {0};
            hr = methods->GetAt(index, &pv);

            if (SUCCEEDED(hr))
            {
                // We have a method.  It is assumed that
                // methods are returned as VT_CLSID VarTypes.
                if (pv.puuid != nullptr)
                {
                    DisplayMethod(capabilities.Get(), *pv.puuid);
                    wprintf(L"\n");
                }
            }

            PropVariantClear(&pv);
        }
    }
}
//</SnippetListSupportedMethods1>

//<SnippetCMethodCallback1>
class CMethodCallback : public IPortableDeviceServiceMethodCallback
{
public:
   CMethodCallback () : m_ref(0)
   {
   }

   ~CMethodCallback ()
   {
   }

public:
    // IPortableDeviceServiceMethodCallback::QueryInterface
    IFACEMETHODIMP OnComplete(
             HRESULT                 hrStatus,
        _In_ IPortableDeviceValues*  /*pResults*/) // We are ignoring results as our methods will not return any results
    {
        wprintf(L"** Method completed, status HRESULT = 0x%lx **\n", hrStatus);

        if (g_methodCompleteEvent != nullptr)
        {
            SetEvent(g_methodCompleteEvent);
        }
        return S_OK;
    }

    IFACEMETHODIMP QueryInterface(
        _In_         REFIID  riid,
        _COM_Outptr_ void**  ppv)
    {
        static const QITAB qitab[] =
        {
            QITABENT(CMethodCallback, IPortableDeviceServiceMethodCallback),
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

private:
    long m_ref;
};
//</SnippetCMethodCallback1>
//<SnippetInvokeMethods1>
// Invoke methods on the Contacts Service.
// BeginSync and EndSync are methods defined by the FullEnumerationSync Device Service.
void InvokeMethods(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceServiceMethods> methods;

    // Get an IPortableDeviceServiceMethods interface from the IPortableDeviceService interface to
    // invoke methods.
    HRESULT hr = service->Methods(&methods);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceMethods from IPortableDeviceService, hr = 0x%lx\n", hr);
    }
    else
    {
        // Invoke() the BeginSync method synchronously
        // This method does not take any parameters or results, so we pass in nullptr
        hr = methods->Invoke(METHOD_FullEnumSyncSvc_BeginSync, nullptr, nullptr);
        if (SUCCEEDED(hr))
        {
            wprintf(L"%ws called, hr = 0x%lx\n", NAME_FullEnumSyncSvc_BeginSync, hr);
        }
        else
        {
            wprintf(L"! Failed to invoke %ws, hr = 0x%lx\n", NAME_FullEnumSyncSvc_BeginSync, hr);
        }
    }

    // Invoke the EndSync method synchronously
    if (SUCCEEDED(hr))
    {
        // This method does not take any parameters or results, so we pass in nullptr
        hr = methods->Invoke(METHOD_FullEnumSyncSvc_EndSync, nullptr, nullptr);
        if (SUCCEEDED(hr))
        {
            wprintf(L"%ws called, hr = 0x%lx\n", NAME_FullEnumSyncSvc_EndSync, hr);
        }
        else
        {
            wprintf(L"! Failed to invoke %ws, hr = 0x%lx\n", NAME_FullEnumSyncSvc_EndSync, hr);
        } 
    }
}
//</SnippetInvokeMethods1>

HRESULT InvokeMethodAsync(
    _In_     IPortableDeviceServiceMethods*  methods,
    _In_     REFGUID                         method,
    _In_opt_ IPortableDeviceValues*          parameters)
{
    HRESULT hr = S_OK;
    
    // Cleanup any previously created global event handles.
    if (g_methodCompleteEvent != nullptr)
    {
        CloseHandle(g_methodCompleteEvent);
        g_methodCompleteEvent = nullptr;
    }  

    // In order to create a simpler to follow example we create and wait infinitely
    // for the method to complete and ignore any errors.
    // Production code should be written in a more robust manner.
    // Create the global event handle to wait on for the method to complete.
    g_methodCompleteEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_methodCompleteEvent != nullptr)
    {
        // Assign the callback pointer to a ComPtr so that it will automatically release
        // when out of scope
        ComPtr<CMethodCallback> callback = new (std::nothrow) CMethodCallback();

        if (callback == nullptr)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"! Failed to get allocate CMethodCallback, hr = 0x%lx\n", hr);
        }
        else
        {
            // Call InvokeAsync() to begin the Asynchronous method operation.
            // Each pCallback parameter is used as the method context, and therefore must be unique.
            hr = methods->InvokeAsync(method, parameters, callback.Get());
            if(FAILED(hr))
            {
                wprintf(L"! Failed to invoke method asynchronously, hr = 0x%lx\n", hr);
            }
            else
            {
                // In order to create a simpler to follow example we will wait infinitly for the operation
                // to complete and ignore any errors.  Production code should be written in a more
                // robust manner.
                if (g_methodCompleteEvent != nullptr)
                {
                    WaitForSingleObject(g_methodCompleteEvent, INFINITE);
                }
            }
        }

        // Cleanup any previously created global event handles.
        if (g_methodCompleteEvent != nullptr)
        {
            CloseHandle(g_methodCompleteEvent);
            g_methodCompleteEvent = nullptr;
        }
    }
    else
    {
        wprintf(L"! Failed to create the global event handle to wait on for the method operation. Aborting operation.\n");
    }

    return hr;
}

//<SnippetInvokeMethodsAsync1>
// Invoke methods on the Contacts Service asynchornously.
// BeginSync and EndSync are methods defined by the FullEnumerationSync Device Service.
void InvokeMethodsAsync(
    _In_ IPortableDeviceService* service)
{
    ComPtr<IPortableDeviceServiceMethods> methods;

    // Get an IPortableDeviceServiceMethods interface from the IPortableDeviceService interface to
    // invoke methods.
    HRESULT hr = service->Methods(&methods);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get IPortableDeviceServiceMethods from IPortableDeviceService, hr = 0x%lx\n", hr);
    }
    else
    {
        // Invoke the BeginSync method asynchronously
        wprintf(L"Invoking %ws asynchronously...\n", NAME_FullEnumSyncSvc_BeginSync);

        // This method does not take any parameters, so we pass in nullptr
        hr = InvokeMethodAsync(methods.Get(), METHOD_FullEnumSyncSvc_BeginSync, nullptr);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to invoke %ws asynchronously, hr = 0x%lx\n", NAME_FullEnumSyncSvc_BeginSync, hr);
        }
    }

    // Invoke the EndSync method asynchronously
    if (SUCCEEDED(hr))
    {
        wprintf(L"Invoking %ws asynchronously...\n", NAME_FullEnumSyncSvc_EndSync);

        hr = InvokeMethodAsync(methods.Get(), METHOD_FullEnumSyncSvc_EndSync, nullptr);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to invoke %ws asynchronously, hr = 0x%lx\n", NAME_FullEnumSyncSvc_EndSync, hr);
        }
    }
}
//</SnippetInvokeMethodsAsync1>

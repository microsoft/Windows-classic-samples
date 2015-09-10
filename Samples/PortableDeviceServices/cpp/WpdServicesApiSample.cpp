// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Defines the entry point for the console application.
//

#include "stdafx.h"

void EumerateContactsServices(_Out_ vector<PWSTR>& contactsServices);
void FreeServicePnPIDs(_In_ vector<PWSTR>& contactsServices);
void ChooseDeviceService(_Outptr_result_maybenull_ IPortableDeviceService** service);
void EnumerateAllContent(_In_ IPortableDeviceService* device);
void ReadContentProperties(_In_ IPortableDeviceService* service);
void WriteContentProperties(_In_ IPortableDeviceService* service);
void ListSupportedFormats(_In_ IPortableDeviceService* service);
void ListSupportedEvents(_In_ IPortableDeviceService* service);
void ListSupportedMethods(_In_ IPortableDeviceService* service);
void ListAbstractServices(_In_ IPortableDeviceService* service);
void InvokeMethods(_In_ IPortableDeviceService* service);
void InvokeMethodsAsync(_In_ IPortableDeviceService* service);

void DoMenu()
{
    HRESULT hr              = S_OK;
    UINT    selectionIndex  = 0;
    WCHAR   selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceService> portableDeviceService;

    ChooseDeviceService(&portableDeviceService);

    if (portableDeviceService == nullptr)
    {
        // No device service was selected, so exit immediately.
        return;
    }

    while (selectionIndex != 99)
    {
        ZeroMemory(selection, sizeof(selection));
        wprintf(L"\n\n");
        wprintf(L"WPD Services Sample Application \n");
        wprintf(L"=======================================\n\n");
        wprintf(L"0.  Enumerate all Contacts device services\n");
        wprintf(L"1.  Choose a Contacts service\n");
        wprintf(L"2.  Enumerate all content on the service\n");
        wprintf(L"3.  List all formats supported by the service\n");
        wprintf(L"4.  List all events supported by the service\n");
        wprintf(L"5.  List all methods supported by the service\n");
        wprintf(L"6.  List all abstract services implemented by the service\n");
        wprintf(L"7.  Read properties on a content object\n");
        wprintf(L"8.  Write properties on a content object\n");
        wprintf(L"9.  Invoke methods on the service\n");
        wprintf(L"10. Invoke methods asynchronously on the service\n");

        wprintf(L"99. Exit\n");
        hr = StringCchGetsW(selection, ARRAYSIZE(selection));
        if (SUCCEEDED(hr))
        {
            selectionIndex = static_cast<UINT>(_wtoi(selection));
            switch (selectionIndex)
            {
                case 0:
                {
                    vector<PWSTR> contactsServices;

                    EumerateContactsServices(contactsServices);
                    FreeServicePnPIDs(contactsServices);
                    break;
                }
                case 1:
                    // Release the old IPortableDeviceService interface before
                    // obtaining a new one.
                    portableDeviceService = nullptr;
                    ChooseDeviceService(&portableDeviceService);
                    break;
                case 2:
                    EnumerateAllContent(portableDeviceService.Get());
                    break;
                case 3:
                    ListSupportedFormats(portableDeviceService.Get());
                    break;
                case 4:
                    ListSupportedEvents(portableDeviceService.Get());
                    break;
                case 5:
                    ListSupportedMethods(portableDeviceService.Get());
                    break;
                case 6:
                    ListAbstractServices(portableDeviceService.Get());
                    break;
                case 7:
                    ReadContentProperties(portableDeviceService.Get());
                    break;
                case 8:
                    WriteContentProperties(portableDeviceService.Get());
                    break;
                case 9:
                    InvokeMethods(portableDeviceService.Get());
                    break;
                case 10:
                    InvokeMethodsAsync(portableDeviceService.Get());
                    break;
                default:
                    break;
            }
        }
        else
        {
            wprintf(L"! Failed to read menu selection string input, hr = 0x%lx\n", hr);
        }
    }
}

int _cdecl wmain(int /*argc*/, _In_ wchar_t* /*argv[]*/)
{
    // Enable the heap manager to terminate the process on heap error.
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

    // Initialize COM for COINIT_MULTITHREADED
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        // Enter the menu processing loop
        DoMenu();

        // Uninitialize COM
        CoUninitialize();
    }

    return 0;
}


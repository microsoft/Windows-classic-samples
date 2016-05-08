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
#include <strsafe.h>

void EumerateContactsServices(CAtlArray<PWSTR>& ContactsServicePnpIDs);
void FreeServicePnPIDs(CAtlArray<PWSTR>& ServicePnpIDs);
void ChooseDeviceService(IPortableDeviceService** ppService);
void EnumerateAllContent(IPortableDeviceService* pDevice);
void ReadContentProperties(IPortableDeviceService*  pService);
void WriteContentProperties(IPortableDeviceService* pService);
void ListSupportedFormats(IPortableDeviceService* pService);
void ListSupportedEvents(IPortableDeviceService* pService);
void ListSupportedMethods(IPortableDeviceService* pService);
void ListAbstractServices(IPortableDeviceService* pService);
void InvokeMethods(IPortableDeviceService* pService);
void InvokeMethodsAsync(IPortableDeviceService* pService);


void DoMenu()
{
    HRESULT hr              = S_OK;
    UINT    uiSelection     = 0;
    CHAR    szSelection[81] = {0};
    CComPtr<IPortableDeviceService> pIPortableDeviceService;

    ChooseDeviceService(&pIPortableDeviceService);

    if (pIPortableDeviceService == NULL)
    {
        // No device service was selected, so exit immediately.
        return;
    }

    while (uiSelection != 99)
    {
        ZeroMemory(szSelection, sizeof(szSelection));
        printf("\n\n");
        printf("WPD Services Sample Application \n");
        printf("=======================================\n\n");
        printf("0.  Enumerate all Contacts device services\n");
        printf("1.  Choose a Contacts service\n");
        printf("2.  Enumerate all content on the service\n");
        printf("3.  List all formats supported by the service\n");
        printf("4.  List all events supported by the service\n");
        printf("5.  List all methods supported by the service\n");
        printf("6.  List all abstract services implemented by the service\n");
        printf("7.  Read properties on a content object\n");
        printf("8.  Write properties on a content object\n");
        printf("9.  Invoke methods on the service\n");
        printf("10. Invoke methods asynchronously on the service\n");

        printf("99. Exit\n");
        hr = StringCbGetsA(szSelection,sizeof(szSelection));
        if (SUCCEEDED(hr))
        {
            uiSelection = (UINT) atoi(szSelection);
            switch (uiSelection)
            {
                case 0:
                {
                    CAtlArray<PWSTR> ContactsServicesArray;
                    EumerateContactsServices(ContactsServicesArray);
                    FreeServicePnPIDs(ContactsServicesArray);
                    break;
                }
                case 1:
                    // Release the old IPortableDeviceService interface before
                    // obtaining a new one.
                    pIPortableDeviceService = NULL;
                    ChooseDeviceService(&pIPortableDeviceService);
                    break;
                case 2:
                    EnumerateAllContent(pIPortableDeviceService);
                    break;
                case 3:
                    ListSupportedFormats(pIPortableDeviceService);
                    break;
                case 4:
                    ListSupportedEvents(pIPortableDeviceService);
                    break;
                case 5:
                    ListSupportedMethods(pIPortableDeviceService);
                    break;
                case 6:
                    ListAbstractServices(pIPortableDeviceService);
                    break;
                case 7:
                    ReadContentProperties(pIPortableDeviceService);
                    break;
                case 8:
                    WriteContentProperties(pIPortableDeviceService);
                    break;
                case 9:
                    InvokeMethods(pIPortableDeviceService);
                    break;
                case 10:
                    InvokeMethodsAsync(pIPortableDeviceService);
                    break;
                default:
                    break;
            }            
        }
        else
        {
            printf("! Failed to read menu selection string input, hr = 0x%lx\n",hr);
        }
    }
}

int _cdecl wmain(int /*argc*/, wchar_t* /*argv[]*/)
{
    // Enable the heap manager to terminate the process on heap error.
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize COM for COINIT_MULTITHREADED
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        // Enter the menu processing loop
        DoMenu();

        // Uninitialize COM
        CoUninitialize();
    }

    return 0;
}


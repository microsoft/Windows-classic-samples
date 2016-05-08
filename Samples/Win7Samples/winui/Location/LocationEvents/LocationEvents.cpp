// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


wchar_t const APP_DESCRIPTION[] = L"Location API SDK Asynchronous Sample";
// The goal of this application is to show you how to use the Location API in an asynchronous fashion.
// You will initialize the Location API and pass it your callback interface. Then, any time the location
// changes, it will call our interface to let us know the change.


#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h> // This is the main Location API header
#include "LocationCallback.h" // This is our callback interface that receives Location reports.

class CInitializeATL : public CAtlExeModuleT<CInitializeATL>{};
CInitializeATL g_InitializeATL; // Initializes ATL for this application. This also does CoInitialize for us

int wmain()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);;
    if (SUCCEEDED(hr))
    {
        wprintf(L"%s.\n\n", APP_DESCRIPTION);

        CComPtr<ILocation> spLocation; // This is the main Location interface
        CComObject<CLocationEvents>* pLocationEvents = NULL; // This is our callback object for location reports
        IID REPORT_TYPES[] = { IID_ILatLongReport }; // Array of report types of interest. Other ones include IID_ICivicAddressReport

        hr = spLocation.CoCreateInstance(CLSID_Location); // Create the Location object

        if (SUCCEEDED(hr))
        {
            hr = CComObject<CLocationEvents>::CreateInstance(&pLocationEvents); // Create the callback object
            if (NULL != pLocationEvents)
            {
                pLocationEvents->AddRef();
            }
        }

        if (SUCCEEDED(hr))
        {
            // Request permissions for this user account to receive location data for all the
            // types defined in REPORT_TYPES (which is currently just one report)
            if (FAILED(spLocation->RequestPermissions(NULL, REPORT_TYPES, ARRAYSIZE(REPORT_TYPES), FALSE))) // FALSE means an asynchronous request
            {
                wprintf(L"Warning: Unable to request permissions.\n");
            }

            // Tell the Location API that we want to register for reports (which is currently just one report)
            for (DWORD index = 0; index < ARRAYSIZE(REPORT_TYPES); index++)
            {
                hr = spLocation->RegisterForReport(pLocationEvents, REPORT_TYPES[index], 0);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Wait until user presses a key to exit app. During this time the Location API
            // will send reports to our callback interface on another thread.
            system("pause");

            // Unregister from reports from the Location API
            for (DWORD index = 0; index < ARRAYSIZE(REPORT_TYPES); index++)
            {
                spLocation->UnregisterForReport(REPORT_TYPES[index]);
            }
        }

        // Cleanup
        if (NULL != pLocationEvents)
        {
            pLocationEvents->Release();
            pLocationEvents = NULL;
        }

        CoUninitialize();
    }

    return 0;
}

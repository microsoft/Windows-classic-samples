// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

wchar_t const APP_DESCRIPTION[] = L"Location API SDK Synchronous Sample";

// The goal of this application is to show you how to use the Location API in a synchronous fashion.
// You will initialize the Location API. Then, on a length of time of your choosing, you will
// poll the Location API for the current location.

#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h>
#include <conio.h> // For _kbhit() and _getch()

// Helper function that allows us to print a GUID to the console in a friendly format
PCWSTR GUIDToString(REFGUID guid, PWSTR psz, UINT cch)
{
    StringFromGUID2(guid, psz, cch);
    return psz;
}

 // Initializes ATL for this application. This also does CoInitialize for us
class CInitializeATL : public CAtlExeModuleT<CInitializeATL>{};
CInitializeATL g_InitializeATL;

int wmain()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        wprintf(L"%s.\nPress any key to exit.\n\n", APP_DESCRIPTION);

        CComPtr<ILocation> spLocation; // This is the main Location interface

        if (SUCCEEDED(spLocation.CoCreateInstance(CLSID_Location))) // Create the Location object
        {
             // Array of report types of interest. Other ones include IID_ICivicAddressReport
            IID REPORT_TYPES[] = { IID_ILatLongReport };

            // Request permissions for this user account to receive location data for all the
            // types defined in REPORT_TYPES (which is currently just one report)
            if (FAILED(spLocation->RequestPermissions(NULL, REPORT_TYPES, ARRAYSIZE(REPORT_TYPES), TRUE))) // TRUE means a synchronous request
            {
                wprintf(L"Warning: Unable to request permissions.\n");
            }

            while (!_kbhit()) // While the user hasn't pressed anything on the keyboard
            {
                LOCATION_REPORT_STATUS status = REPORT_NOT_SUPPORTED; // The LOCATION_REPORT_STATUS enumeration is defined in LocApi.h in the SDK

                if (SUCCEEDED(spLocation->GetReportStatus(IID_ILatLongReport, &status))) // Get the status of this report type
                {
                    bool fCachedData = true;
                    switch (status) // If there is an error, print the error
                    {
                    case REPORT_RUNNING:
                        // If the status for the current report is running, then don't print any additional message
                        fCachedData = false;
                        break;
                    case REPORT_NOT_SUPPORTED:
                        wprintf(L"\nNo devices detected.\n");
                        break;
                    case REPORT_ERROR:
                        wprintf(L"\nReport error.\n");
                        break;
                    case REPORT_ACCESS_DENIED:
                        wprintf(L"\nAccess denied to reports.\n");
                        break;
                    case REPORT_INITIALIZING:
                        wprintf(L"\nReport is initializing.\n");
                        break;
                    }

                    if (true == fCachedData)
                    {
                        wprintf(L"The following is cached data:\n");
                    }
                }

                CComPtr<ILocationReport> spLocationReport; // This is our location report object
                CComPtr<ILatLongReport> spLatLongReport; // This is our LatLong report object

                // Get the current location report,
                // then get the ILatLongReport interface from ILocationReport,
                // then ensure it isn't NULL
                if ((SUCCEEDED(spLocation->GetReport(IID_ILatLongReport, &spLocationReport))) &&
                    (SUCCEEDED(spLocationReport->QueryInterface(&spLatLongReport))))
                {
                    // Print the Report Type GUID
                    wchar_t szGUID[64];
                    wprintf(L"\nReportType: %s", GUIDToString(IID_ILatLongReport, szGUID, ARRAYSIZE(szGUID)));

                    // Print the Timestamp
                    SYSTEMTIME systemTime;
                    if (SUCCEEDED(spLatLongReport->GetTimestamp(&systemTime)))
                    {
                        wprintf(L"\nTimestamp: YY:%d, MM:%d, DD:%d, HH:%d, MM:%d, SS:%d, MS:%d\n",
                            systemTime.wYear,
                            systemTime.wMonth,
                            systemTime.wDay,
                            systemTime.wHour,
                            systemTime.wMinute,
                            systemTime.wSecond,
                            systemTime.wMilliseconds);
                    }

                    // Print the Sensor ID GUID
                    GUID sensorID = {0};
                    if (SUCCEEDED(spLatLongReport->GetSensorID(&sensorID)))
                    {
                        wprintf(L"SensorID: %s\n", GUIDToString(sensorID, szGUID, ARRAYSIZE(szGUID)));
                    }

                    DOUBLE latitude = 0, longitude = 0, altitude = 0, errorRadius = 0, altitudeError = 0;
                    // Print the Latitude
                    if (SUCCEEDED(spLatLongReport->GetLatitude(&latitude)))
                    {
                        wprintf(L"Latitude: %f\n", latitude);
                    }

                    // Print the Longitude
                    if (SUCCEEDED(spLatLongReport->GetLongitude(&longitude)))
                    {
                        wprintf(L"Longitude: %f\n", longitude);
                    }

                    // Print the Altitude
                    if (SUCCEEDED(spLatLongReport->GetAltitude(&altitude)))
                    {
                        wprintf(L"Altitude: %f\n", altitude);
                    }
                    else
                    {
                        // Altitude is optional and may not be available
                        wprintf(L"Altitude: Not available.\n");
                    }

                    // Print the Error Radius
                    if (SUCCEEDED(spLatLongReport->GetErrorRadius(&errorRadius)))
                    {
                        wprintf(L"Error Radius: %f\n", errorRadius);
                    }

                    // Print the Altitude Error
                    if (SUCCEEDED(spLatLongReport->GetAltitudeError(&altitudeError)))
                    {
                        wprintf(L"Altitude Error: %f\n", altitudeError);
                    }
                    else
                    {
                        // Altitude Error is optional and may not be available
                        wprintf(L"Altitude Error: Not available.\n");
                    }
                }

                Sleep(1000); // Sleep for one second before checking for the next report
            }

            while (_kbhit())
            {
                _getch(); // While there is something in the keyboard buffer, get (and discard) each keypress
            }
        }
        
        CoUninitialize();
    }
    
    return 0;
}
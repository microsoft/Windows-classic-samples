// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "LocationCallback.h"

// Helper function that allows us to print a GUID to the console in a friendly format
PCWSTR GUIDToString(REFGUID guid, PWSTR psz, UINT cch)
{
    StringFromGUID2(guid, psz, cch);
    return psz;
}

// This is called when there is a new location report
STDMETHODIMP CLocationEvents::OnLocationChanged(REFIID reportType, ILocationReport* pLocationReport)
{
    // If the report type is a Latitude/Longitude report (as opposed to IID_ICivicAddressReport or another type)
    if (IID_ILatLongReport == reportType)
    {
        CComPtr<ILatLongReport> spLatLongReport;

        // Get the ILatLongReport interface from ILocationReport
        if ((SUCCEEDED(pLocationReport->QueryInterface(IID_PPV_ARGS(&spLatLongReport)))) && (NULL != spLatLongReport.p))
        {
            // Print the Report Type GUID
            wchar_t szGUID[64];
            wprintf(L"\nReportType: %s", GUIDToString(IID_ILatLongReport, szGUID, ARRAYSIZE(szGUID)));

            // Print the Timestamp and the time since the last report
            SYSTEMTIME systemTime;
            if (SUCCEEDED(spLatLongReport->GetTimestamp(&systemTime)))
            {
                // Compute the number of 100ns units that difference between the current report's time and the previous report's time.
                ULONGLONG currentTime = 0, diffTime = 0;
                if (TRUE == SystemTimeToFileTime(&systemTime, (FILETIME*)&currentTime))
                {
                    diffTime = (currentTime > m_previousTime) ? (currentTime - m_previousTime) : 0;
                }

                wprintf(L"\nTimestamp: YY:%d, MM:%d, DD:%d, HH:%d, MM:%d, SS:%d, MS:%d [%I64d]\n",
                    systemTime.wYear,
                    systemTime.wMonth,
                    systemTime.wDay,
                    systemTime.wHour,
                    systemTime.wMinute,
                    systemTime.wSecond,
                    systemTime.wMilliseconds,
                    diffTime / 10000); // Display in milliseconds

                m_previousTime = currentTime; // Set the previous time to the current time for the next report.
            }

            // Print the Sensor ID GUID
            GUID sensorID = {0};
            if (SUCCEEDED(spLatLongReport->GetSensorID(&sensorID)))
            {
                wchar_t szGUID[64];
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
    }

    return S_OK;
}


// This is called when the status of a report type changes.
// The LOCATION_REPORT_STATUS enumeration is defined in LocApi.h in the SDK
STDMETHODIMP CLocationEvents::OnStatusChanged(REFIID reportType, LOCATION_REPORT_STATUS status)
{
    if (IID_ILatLongReport == reportType)
    {
        switch (status)
        {
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
        case REPORT_RUNNING:
            wprintf(L"\nRunning.\n");
            break;
        }
    }
    else if (IID_ICivicAddressReport == reportType)
    {
    }

    return S_OK;
}
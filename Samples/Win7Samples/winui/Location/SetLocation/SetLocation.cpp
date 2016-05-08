// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

const char APP_DESCRIPTION[] = "Location API SDK Default Location Sample";
// The goal of this application is to show you how to set
// the default location using the Location API.

// This application prompts the user to specify location data that
// is used to create a civic address location report and a 
// latitude/longitude report, and prints the report fields.
// It then sets the default location, which can then be viewed in 
// Control Panel.

// This sample must be run as administrator for the call to
// IDefaultLocation::SetReport to succeed. 


#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h>
#include <assert.h>
#include <conio.h>        // For  _getch()

#include "LocationReportObject.h"

// Function for prompting the user to enter a new location.
HRESULT EnterLocation(CLocationReport * pLocationReport);
// Function to print the values in a civic address report.
void PrintCivicAddress(ICivicAddressReport * pDefaultCivicAddressReport);
// Function to print the values in a latitude/longitude report.
void PrintLatLong(ILatLongReport *pLatLongReport);
// Function to print the GUID associated with a location report.
void PrintGUID(REFGUID guid);
// Function to print the system time associated with a location report.
void PrintTime(const SYSTEMTIME systime);
// Function to read a line from standard input and remove the newline character.
WCHAR * SafeGetws(wchar_t* dest, int size);

const wchar_t PROMPT_ADDRESS1[] = 
    L"\nEnter the first line of the street address to set. \nInput that is not less than 80 characters will be truncated\n";
const wchar_t PROMPT_ADDRESS2[] = 
    L"\nEnter the second line of the street address to set.\n";
const wchar_t PROMPT_CITY[] = 
    L"\nEnter the city to set.\n";
const wchar_t PROMPT_STATEPROVINCE[] = 
    L"\nEnter the state/province to set.\n";
const wchar_t PROMPT_POSTALCODE[] = 
    L"\nEnter the postal code to set.\n";
const wchar_t PROMPT_COUNTRYREGION[] = 
    L"\nEnter a valid ISO-3166 two-letter country code.\n";
const wchar_t PROMPT_LATITUDE[] = 
    L"\nEnter a valid latitude between -90 and 90.\n";
const wchar_t PROMPT_LONGITUDE[] = 
    L"\nEnter a valid longitude between -180 and 180.\n";
const wchar_t PROMPT_ERRORRADIUS[] = 
    L"\nEnter a non-zero error radius for the latitude/longitude location, in meters.\n";
const wchar_t PROMPT_ALTITUDE[] = 
    L"\nEnter a value for altitude.\n";
const wchar_t PROMPT_ALTITUDEERROR[] = 
    L"\nEnter a value for the altitude margin of error, in meters.\n";
const int ADDRESS_FIELD_LENGTH = 80;
const int COUNTRYREGION_FIELD_LENGTH = 4; // 2-digit or 3-digit code + newline
class CInitializeATL : public CAtlExeModuleT<CInitializeATL>{};
// Initializes ATL for this application. This also does CoInitialize for us
CInitializeATL g_InitializeATL; 

int wmain()
{
    printf("%s.\n", APP_DESCRIPTION);

    // The location report object
    CComObject<CLocationReport> *pNewLocationReport = NULL;
    // Interface for civic address reports
    CComPtr<ICivicAddressReport> spCivicAddressReport; 
    // Interface for latitude/longitude reports
    CComPtr<ILatLongReport> spLatLongReport; 
    // Interface for setting default location
    CComPtr<IDefaultLocation> spDefaultLocation; 

    // Create the new location report.
    HRESULT hr = CComObject<CLocationReport>::CreateInstance(&pNewLocationReport); 
    if (SUCCEEDED(hr))
    {
        pNewLocationReport->AddRef();
        hr = EnterLocation(pNewLocationReport);    
        if (SUCCEEDED(hr))
        {
            // CoCreate the default location object for getting and setting default location
            hr = spDefaultLocation.CoCreateInstance(CLSID_DefaultLocation);
        }		
        if (SUCCEEDED(hr))
        {
            hr = pNewLocationReport->QueryInterface(IID_PPV_ARGS(&spCivicAddressReport));
        }
        if (SUCCEEDED(hr))
        {             
            // set the civic address fields of the Default Location
            hr = spDefaultLocation->SetReport(IID_ICivicAddressReport, spCivicAddressReport);
            if (E_INVALIDARG == hr)
            {
                wprintf(L"The civic address report has invalid data. ");
                wprintf(L"Country/region must be a valid ISO-3166 2-letter or 3-letter code.\n");
            }
            else if (E_ACCESSDENIED == hr)
            {
                wprintf(L"Administrator privilege required.\n");
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = pNewLocationReport->QueryInterface(IID_PPV_ARGS(&spLatLongReport));
        }
        if (SUCCEEDED(hr))
        {
            hr = spDefaultLocation->SetReport(IID_ILatLongReport, spLatLongReport);
            if (E_INVALIDARG == hr)
            {
                wprintf(L"The latitude/longitude report has invalid data. ");
                wprintf(L"Latitude must be between -90 and 90.\n Longitude must be between -180 and 180.\n");
            }
            else if (E_ACCESSDENIED == hr)
            {
                wprintf(L"Administrator privilege required.\n");
            }         
        } 

        pNewLocationReport->Release();

    }
    return 0;
}
// Gets new location data from the user, and prints the report data.
// The location data includes fields for both a civic address report 
// and a latitude/longitude report.
// The error radius, altitude, and altitude error fields are not
// displayed in the Default Location Control Panel, but they are 
// available from the Location API.
HRESULT EnterLocation(CLocationReport * pLocationReport)
{
    WCHAR address1[ADDRESS_FIELD_LENGTH];
    WCHAR address2[ADDRESS_FIELD_LENGTH]; 
    WCHAR city[ADDRESS_FIELD_LENGTH];
    WCHAR stateprovince[ADDRESS_FIELD_LENGTH];
    WCHAR zipcode[ADDRESS_FIELD_LENGTH];
    WCHAR countryregion[COUNTRYREGION_FIELD_LENGTH]; // ISO-3166 2-digit or 3-digit code
    DOUBLE latitude = 0;
    DOUBLE longitude = 0;
    DOUBLE errorradius = 0;
    DOUBLE altitude = 0;
    DOUBLE altitudeerror = 0;
    int numberfieldsread = 0; // number of fields successfully read by wscanf_s

    // Interface for civic address report
    CComPtr<ICivicAddressReport> spCivicAddressReport; 
    // Interface for latitude/longitude report
    CComPtr<ILatLongReport> spLatLongReport; 

    // prompt user to enter line 1 of the street address.
    wprintf(PROMPT_ADDRESS1);
    SafeGetws(address1, ARRAYSIZE(address1));
    fflush(stdin);
    // Set line 1 of the street address in the new report
    CComBSTR bstrAddress1(address1);
    HRESULT hr = pLocationReport->SetAddressLine1(bstrAddress1);    
    
    if (SUCCEEDED(hr))
    {
        // prompt user to enter line 2 of the street address.
        wprintf(PROMPT_ADDRESS2);
        SafeGetws(address2, ARRAYSIZE(address2));
        fflush(stdin);
        // Set line 2 of the street address in the new report
        CComBSTR bstrAddress2(address2);
        hr = pLocationReport->SetAddressLine2(bstrAddress2);
    }
    if (SUCCEEDED(hr))
    {
        // prompt user to enter the city.
        wprintf(PROMPT_CITY);
        SafeGetws(city, ARRAYSIZE(city));
        fflush(stdin);
        // Set the city in the new report
        CComBSTR bstrCity(city);
        hr = pLocationReport->SetCity(bstrCity);
    }
    if (SUCCEEDED(hr))
    {
        // prompt user to enter the state/province.
        wprintf(PROMPT_STATEPROVINCE);
        SafeGetws(stateprovince, ARRAYSIZE(city));
        fflush(stdin);
        // Set the state/province in the new report
        CComBSTR bstrStateProvince(stateprovince);
        hr = pLocationReport->SetStateProvince(bstrStateProvince);
    }
    if (SUCCEEDED(hr))
    {
        // prompt user to enter the postal code.
        wprintf(PROMPT_POSTALCODE);
        SafeGetws(zipcode, ARRAYSIZE(zipcode));
        fflush(stdin);
        // Set the postal code in the new report
        CComBSTR bstrZipCode(zipcode);
        hr = pLocationReport->SetPostalCode(bstrZipCode);    
    }
    if (SUCCEEDED(hr))
    {
        // prompt user to enter the country code.
        wprintf(PROMPT_COUNTRYREGION);
        SafeGetws(countryregion, ARRAYSIZE(countryregion));
        fflush(stdin);
        // Set the country/region in the new report
        CComBSTR bstrCountryRegion(countryregion);
        hr = pLocationReport->SetCountryRegion(bstrCountryRegion);
    }
    if (SUCCEEDED(hr))
    {
        // prompt the user to enter latitude
        wprintf(PROMPT_LATITUDE);
        numberfieldsread = wscanf_s(L"%lf", &latitude);
        fflush(stdin);
        if (0 == numberfieldsread) 
        {
            wprintf(L"Error reading input. The report field will be set to zero.\n");
        }
        else 
        {
            // Set the latitude
            hr = pLocationReport->SetLatitude(latitude);
        }
    }
    if (SUCCEEDED(hr))
    {
        // prompt the user to enter longitude
        wprintf(PROMPT_LONGITUDE);
        numberfieldsread = wscanf_s(L"%lf", &longitude);
        fflush(stdin);
        if (0 == numberfieldsread) 
        {
            wprintf(L"Error reading input. The report field will be set to zero.\n");
        }
        else 
        {
            // Set the longitude
            hr = pLocationReport->SetLongitude(longitude);
        }
    }
    if (SUCCEEDED(hr))
    {
        // prompt the user to enter the error radius for the latitude/longitude 
        wprintf(PROMPT_ERRORRADIUS);
        numberfieldsread = wscanf_s(L"%lf", &errorradius);
        fflush(stdin);
        if (0 == numberfieldsread) 
        {
            wprintf(L"Error reading input. The report field will be set to a default value.\n");
        }
        else 
        {
            // Set the error radius
            hr = pLocationReport->SetErrorRadius(errorradius);
        }
    }
    if (SUCCEEDED(hr))
    {
        // prompt the user to enter altitude
        wprintf(PROMPT_ALTITUDE);
        numberfieldsread = wscanf_s(L"%lf", &altitude);
        fflush(stdin);
        if (0 == numberfieldsread) 
        {
            wprintf(L"Error reading input. The report field will be set to zero.\n");
        }
        else 
        {
            // Set the altitude
            hr = pLocationReport->SetAltitude(altitude);
        }
    }
    if (SUCCEEDED(hr))
    {
        // prompt the user to enter altitude error
        wprintf(PROMPT_ALTITUDEERROR);
        numberfieldsread = wscanf_s(L"%lf", &altitudeerror);
        fflush(stdin);
        if (0 == numberfieldsread) 
        {
            wprintf(L"Error reading input. The report field will be set to zero.\n");
        }
        else
        {
            // Set the altitude error
            hr = pLocationReport->SetAltitudeError(altitudeerror);
        }
    }
    if (SUCCEEDED(hr))
    {
        // Print the GUID 
        SENSOR_ID SensorID;
        hr = pLocationReport->GetSensorID(&SensorID);
        PrintGUID(SensorID);
    }
    if (SUCCEEDED(hr))
    {
        // Print the timestamp
        SYSTEMTIME systime;
        hr = pLocationReport->GetTimestamp(&systime); 
        PrintTime(systime);
    }
    if (SUCCEEDED(hr))
    {
        hr = pLocationReport->QueryInterface(IID_PPV_ARGS(&spCivicAddressReport));
    }
    if (SUCCEEDED(hr))
    {
        PrintCivicAddress(spCivicAddressReport);
    }
    if (SUCCEEDED(hr))
    {
        hr = pLocationReport->QueryInterface(IID_PPV_ARGS(&spLatLongReport));
    }
    if (SUCCEEDED(hr))
    {
        PrintLatLong(spLatLongReport);
    }
    return hr;
}

// Prints the fields of a civic address location report.
// Empty fields are not printed.
void PrintCivicAddress(ICivicAddressReport *pCivicAddressReport)
{
    CComBSTR bstrAddress1;
    CComBSTR bstrAddress2;
    CComBSTR bstrPostalCode;
    CComBSTR bstrCity;
    CComBSTR bstrStateProvince;
    CComBSTR bstrCountryRegion;
    
    assert(pCivicAddressReport != NULL);

    wprintf(L"\n\n");

        HRESULT hr = pCivicAddressReport->GetAddressLine1(&bstrAddress1);
        if (SUCCEEDED(hr) && (bstrAddress1.Length() != 0))
        { 
            wprintf(L"\tAddress Line 1:\t%s\n", bstrAddress1);
        }
        hr = pCivicAddressReport->GetAddressLine2(&bstrAddress2);            
        if (SUCCEEDED(hr) && (bstrAddress2.Length() != 0))
        {   
            wprintf(L"\tAddress Line 2:\t%s\n", bstrAddress2);
        }        
        hr = pCivicAddressReport->GetPostalCode(&bstrPostalCode);
        if (SUCCEEDED(hr) && (bstrPostalCode.Length() != 0))
        {                
            wprintf(L"\tPostal Code:\t%s\n", bstrPostalCode);
        }
        hr = pCivicAddressReport->GetCity(&bstrCity);
        if (SUCCEEDED(hr) && (bstrCity.Length() != 0))
        {
            wprintf(L"\tCity:\t\t%s\n", bstrCity);
        }
        hr = pCivicAddressReport->GetStateProvince(&bstrStateProvince);
        if (SUCCEEDED(hr) && (bstrStateProvince.Length() != 0))
        {
            wprintf(L"\tState/Province:\t%s\n", bstrStateProvince);
        }
        hr = pCivicAddressReport->GetCountryRegion(&bstrCountryRegion);
        if (SUCCEEDED(hr)) 
        {
            // Country/Region is an ISO-3166 two-letter or three-letter code.
           wprintf(L"\tCountry/Region:\t%s\n\n", bstrCountryRegion);    
        }        
}

// Prints the latitude and longitude from a latitude/longitude report
void PrintLatLong(ILatLongReport *pLatLongReport)
{
    DOUBLE latitude;
    DOUBLE longitude;
    DOUBLE errorradius;
    DOUBLE altitude;
    DOUBLE altitudeerror;

    assert(pLatLongReport != NULL);

    wprintf(L"\n\n");

    HRESULT hr = pLatLongReport->GetLatitude(&latitude);
    if (SUCCEEDED(hr))
    { 
        wprintf(L"\tLatitude:\t%lf\n", latitude);
    }
    hr = pLatLongReport->GetLongitude(&longitude);            
    if (SUCCEEDED(hr))
    {   
        wprintf(L"\tLongitude:\t%lf\n", longitude);
    }
    hr = pLatLongReport->GetErrorRadius(&errorradius);
    if (SUCCEEDED(hr))
    { 
        wprintf(L"\tError Radius:\t%lf\n", errorradius);
    }
    hr = pLatLongReport->GetAltitude(&altitude);
    if (SUCCEEDED(hr))
    { 
        wprintf(L"\tAltitude:\t%lf\n", altitude);
    }
    hr = pLatLongReport->GetAltitudeError(&altitudeerror);            
    if (SUCCEEDED(hr))
    {   
        wprintf(L"\tAltitude Error:\t%lf\n", altitudeerror);
    }  
}

// Prints a GUID in a friendly format.
void PrintGUID(REFGUID guid)
{        
    wprintf(L"\nThe GUID of the location provider is: ");
    wchar_t szGUID[64];
    StringFromGUID2(guid, szGUID, ARRAYSIZE(szGUID));
    wprintf(szGUID);
}

// Prints the hour, minute, and second from a timestamp.
// The time is Coordinated Universal Time (UTC).
void PrintTime(const SYSTEMTIME st)
{
    wprintf(L"\nThe report timestamp is M:%02d D:%02d %02d:%02d:%02d UTC\n", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

// Helper function that strips the newline character 
// from the line obtained by fgetws. 
WCHAR * SafeGetws(wchar_t* dest, int size)
{
    fgetws( dest, size, stdin);
    int i = 0;
    while (i < size)
    {
        if (dest[i] == L'\n')
        {
            dest[i] = '\0';
            return dest;            
        }
        i++;
    }
    return dest;
}
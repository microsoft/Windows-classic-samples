// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This is the header for the declaration of the report object
// that implements both ICivicAddressReport and ILatLongReport

#pragma once


#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h>

// Report object that implements ICivicAddressReport and ILatLongReport
class CLocationReport :
    public CComObjectRoot,
    public ICivicAddressReport, 
    public ILatLongReport 
{
public:
    CLocationReport()
    { 
        m_postalcode.Attach(SysAllocString(L""));
        m_countryregion.Attach(SysAllocString(L"US"));
        // This timestamp is not the same timestamp that is returned 
        // in a location report retrieved by using ILocation::GetReport
        GetSystemTime(&m_systemtime);
        // The GUID of the default location provider is
        // {682F38CA-5056-4A58-B52E-B516623CF02F}
        // However, the SENSOR_ID is not required to set default location
        // and this value is not the same as the SENSOR_ID in a 
        // location report retrieved by using ILocation::GetReport
        class __declspec(uuid("682F38CA-5056-4A58-B52E-B516623CF02F")) default_location_provider;
            m_sensorid= __uuidof(default_location_provider);
        m_errorradius = 1000.0; // error radius should be non-zero
        m_latitude    = 0.0;
        m_longitude   = 0.0;
        m_altitude    = 0.0;
        m_altitudeerror   = 0.0;
    }
    virtual ~CLocationReport(){}

    DECLARE_NOT_AGGREGATABLE(CLocationReport)

    BEGIN_COM_MAP(CLocationReport)
        COM_INTERFACE_ENTRY(ICivicAddressReport)
        COM_INTERFACE_ENTRY(ILatLongReport)
    END_COM_MAP()

    // ILocationReport
        IFACEMETHODIMP GetSensorID(SENSOR_ID *pSensorID);
        IFACEMETHODIMP GetTimestamp(SYSTEMTIME *pCreationTime);
        IFACEMETHODIMP GetValue(REFPROPERTYKEY pKey, PROPVARIANT *pValue);
    // ICivicAddressReport
        IFACEMETHODIMP GetAddressLine1(BSTR *pbstrAddress1);        
        IFACEMETHODIMP GetAddressLine2(BSTR *pbstrAddress2);        
        IFACEMETHODIMP GetCity(BSTR *pbstrCity);        
        IFACEMETHODIMP GetStateProvince(BSTR *pbstrStateProvince);
        IFACEMETHODIMP GetPostalCode(BSTR *pbstrPostalCode);
        IFACEMETHODIMP GetCountryRegion(BSTR *pbstrCountryRegion);
        IFACEMETHODIMP GetDetailLevel(DWORD *pDetailLevel);
    // ILatLongReport
        IFACEMETHODIMP GetLatitude(DOUBLE *pLatitude);        
        IFACEMETHODIMP GetLongitude(DOUBLE *pLatitude);        
        IFACEMETHODIMP GetErrorRadius(DOUBLE *pErrorRadius);  
        IFACEMETHODIMP GetAltitude(DOUBLE *pAltitude);        
        IFACEMETHODIMP GetAltitudeError(DOUBLE *pAltitudeError);  
    // CLocationReport
        IFACEMETHODIMP SetAddressLine1(BSTR bstrAddress1);        
        IFACEMETHODIMP SetAddressLine2(BSTR bstrAddress2);        
        IFACEMETHODIMP SetCity(BSTR bstrCity);        
        IFACEMETHODIMP SetStateProvince(BSTR bstrStateProvince);
        IFACEMETHODIMP SetPostalCode(BSTR bstrPostalCode);
        IFACEMETHODIMP SetCountryRegion(BSTR bstrCountryRegion);

        IFACEMETHODIMP SetLatitude(DOUBLE Latitude);        
        IFACEMETHODIMP SetLongitude(DOUBLE Longitude);        
        IFACEMETHODIMP SetErrorRadius(DOUBLE ErrorRadius);
        IFACEMETHODIMP SetAltitude(DOUBLE Altitude);        
        IFACEMETHODIMP SetAltitudeError(DOUBLE AltitudeError);		

private:

    // Variables for civic address fields
    CComBSTR m_address1;
    CComBSTR m_address2;
    CComBSTR m_city;
    CComBSTR m_stateprovince;
    CComBSTR m_postalcode;
    CComBSTR m_countryregion;

    // Variables for latitude/longitude report data
    DOUBLE m_latitude;
    DOUBLE m_longitude;
    DOUBLE m_errorradius;
    DOUBLE m_altitude;
    DOUBLE m_altitudeerror;

    // Variables for other values that are used in a location report.
    SYSTEMTIME m_systemtime;
    SENSOR_ID m_sensorid;
};

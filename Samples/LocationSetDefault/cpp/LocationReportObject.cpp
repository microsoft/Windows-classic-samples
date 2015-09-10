// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Implementation of a location report object that implements both
// ICivicAddressReport and ILatLongReport.

#include "LocationReportObject.h"
#include <sensors.h> // for PROPERTYKEY definitions
#include <propvarutil.h>


STDMETHODIMP CLocationReport::GetSensorID(__RPC__out SENSOR_ID *pSensorID)
{
    *pSensorID = m_sensorid;
    return S_OK;
}
STDMETHODIMP CLocationReport::GetTimestamp(__RPC__out SYSTEMTIME *pCreationTime)
{
    *pCreationTime = m_systemtime;
    return S_OK;
}

STDMETHODIMP CLocationReport::GetValue(__RPC__in REFPROPERTYKEY pKey, __RPC__out PROPVARIANT *pValue)
{
    HRESULT hr = S_OK;

    // properties for civic address reports
    if (pKey.pid == SENSOR_DATA_TYPE_ADDRESS1.pid)
    {
        hr = InitPropVariantFromString(m_address1, pValue);
    } 
    else if (pKey.pid == SENSOR_DATA_TYPE_ADDRESS2.pid)
    {    
        hr = InitPropVariantFromString(m_address2, pValue);
    }
    else if (pKey.pid == SENSOR_DATA_TYPE_CITY.pid)
    {
        hr = InitPropVariantFromString(m_city, pValue);
    }
    else if (pKey.pid == SENSOR_DATA_TYPE_STATE_PROVINCE.pid)
    {
        hr = InitPropVariantFromString(m_stateprovince, pValue);
    } 
    else if (pKey.pid == SENSOR_DATA_TYPE_POSTALCODE.pid)
    {
        hr = InitPropVariantFromString(m_postalcode, pValue);
    }
    else if (pKey.pid == SENSOR_DATA_TYPE_COUNTRY_REGION.pid)    
    {
        hr = InitPropVariantFromString(m_countryregion, pValue);
    }
    // properties for latitude/longitude reports
    else if (pKey.pid == SENSOR_DATA_TYPE_LATITUDE_DEGREES.pid)
    {
        hr = InitPropVariantFromDouble(m_latitude, pValue);
    } 
    else if (pKey.pid == SENSOR_DATA_TYPE_LONGITUDE_DEGREES.pid)
    {
        hr = InitPropVariantFromDouble(m_longitude, pValue);
    }
    else if (pKey.pid == SENSOR_DATA_TYPE_ERROR_RADIUS_METERS.pid)    
    {
        hr = InitPropVariantFromDouble(m_errorradius, pValue);
    }
    else 
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_DATA);
        PropVariantInit(pValue);
    }

    return hr;
}

// ICivicAddressReport
STDMETHODIMP CLocationReport::GetAddressLine1(__RPC__deref_out_opt BSTR *pbstrAddress1)
{
    HRESULT hr = m_address1.CopyTo(pbstrAddress1);
    if (nullptr == *pbstrAddress1)
    {
        *pbstrAddress1 = L" ";
    }

    return hr;
}

STDMETHODIMP CLocationReport::GetAddressLine2(__RPC__deref_out_opt BSTR *pbstrAddress2)
{
    HRESULT hr = m_address2.CopyTo(pbstrAddress2);
    if (nullptr == *pbstrAddress2)
    {
        *pbstrAddress2 = L" ";
    }

    return hr;
}

STDMETHODIMP CLocationReport::GetCity(__RPC__deref_out_opt BSTR *pbstrCity)
{
    HRESULT hr = m_city.CopyTo(pbstrCity);
    if (nullptr == *pbstrCity)
    {
        *pbstrCity = L" ";
    }

    return hr;
}

STDMETHODIMP CLocationReport::GetStateProvince(__RPC__deref_out_opt BSTR *pbstrStateProvince)
{
    HRESULT hr = m_stateprovince.CopyTo(pbstrStateProvince);
    if (nullptr == *pbstrStateProvince)
    {
        *pbstrStateProvince = L" ";
    }

    return hr;
}

STDMETHODIMP CLocationReport::GetPostalCode(__RPC__deref_out_opt BSTR *pbstrPostalCode)
{
    HRESULT hr = m_postalcode.CopyTo(pbstrPostalCode);
    if (nullptr == *pbstrPostalCode)
    {
        *pbstrPostalCode = L" ";
    }

    return hr;
}

STDMETHODIMP CLocationReport::GetCountryRegion(__RPC__deref_out_opt BSTR *pbstrCountryRegion)
{
    HRESULT hr = m_countryregion.CopyTo(pbstrCountryRegion);
    if (nullptr == *pbstrCountryRegion)
    {
        *pbstrCountryRegion = L" ";
    }

    return hr;
}

// ILatLongReport
STDMETHODIMP CLocationReport::GetLatitude(__RPC__out DOUBLE *pLatitude)
{
    if (NULL == pLatitude)
    {
        return E_POINTER;
    }
    *pLatitude = m_latitude;
    return S_OK;
}

STDMETHODIMP CLocationReport::GetLongitude(__RPC__out DOUBLE *pLongitude)
{
    if (NULL == pLongitude)
    {
        return E_POINTER;
    }
    *pLongitude = m_longitude;
    return S_OK;
}

// The error radius field is not displayed in the 
// Default Location Control Panel, but is available from the Location API.
STDMETHODIMP CLocationReport::GetErrorRadius(__RPC__out DOUBLE *pErrorRadius)
{
    if (NULL == pErrorRadius)
    {
        return E_POINTER;
    }
    *pErrorRadius = m_errorradius;
    return S_OK;
}

// The altitude field is not displayed in the 
// Default Location Control Panel, but is available from the Location API.
STDMETHODIMP CLocationReport::GetAltitude(__RPC__out DOUBLE *pAltitude)
{
    if (NULL == pAltitude)
    {
        return E_POINTER;
    }
    *pAltitude = m_altitude;
    return S_OK;
}

// The altitude error field is not displayed in the 
// Default Location Control Panel, but is available from the Location API.
STDMETHODIMP CLocationReport::GetAltitudeError(__RPC__out DOUBLE *pAltitudeError)
{
    if (NULL == pAltitudeError)
    {
        return E_POINTER;
    }
    *pAltitudeError = m_altitudeerror;
    return S_OK;
}
// CLocationReport methods
STDMETHODIMP CLocationReport::SetAddressLine1(BSTR bstrAddress1)
{
    m_address1.Attach(SysAllocString(bstrAddress1));
    return m_address1 ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLocationReport::SetAddressLine2(BSTR bstrAddress2)
{
    m_address2.Attach(SysAllocString(bstrAddress2));
    return m_address2 ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLocationReport::SetCity(BSTR bstrCity)
{
    m_city.Attach(SysAllocString(bstrCity));
    return m_city ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLocationReport::SetStateProvince(BSTR bstrStateProvince)
{
    m_stateprovince.Attach(SysAllocString(bstrStateProvince));
    return m_stateprovince ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLocationReport::SetPostalCode(BSTR bstrPostalCode)
{
    m_postalcode.Attach(SysAllocString(bstrPostalCode));
    return m_postalcode ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLocationReport::SetCountryRegion(BSTR bstrCountryRegion)
{
    m_countryregion.Attach(SysAllocString(bstrCountryRegion));
    return m_countryregion ? S_OK : E_OUTOFMEMORY;
}

// GetDetailLevel is not used in this sample
STDMETHODIMP CLocationReport::GetDetailLevel(__RPC__out DWORD *pDetailLevel)
{
    pDetailLevel;
    return E_NOTIMPL;
}

STDMETHODIMP CLocationReport::SetLatitude(DOUBLE latitude)
{
    m_latitude = latitude;
    return S_OK;
}

STDMETHODIMP CLocationReport::SetLongitude(DOUBLE longitude)
{
    m_longitude = longitude;
    return S_OK;
}

STDMETHODIMP CLocationReport::SetErrorRadius(DOUBLE errorRadius)
{
    m_errorradius = errorRadius;
    return S_OK;
}

STDMETHODIMP CLocationReport::SetAltitude(DOUBLE altitude)
{
    m_altitude = altitude;
    return S_OK;
}

STDMETHODIMP CLocationReport::SetAltitudeError(DOUBLE altitudeError)
{
    m_altitudeerror = altitudeError;
    return S_OK;
}




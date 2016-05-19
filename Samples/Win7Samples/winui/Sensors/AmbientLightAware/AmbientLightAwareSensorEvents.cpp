//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareSensorEvents.cpp
//
// Description:
//        Implementation of ISensorManagerEvents
//
// Comments: 
//        This is a standard c++ class, but needs to support IUnknown methods
//        to be properly implemented.  This class overrides ISensorEvents 
//        functions to receive updated information from the Sensors API when it
//        is published.
//
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Resource.h"
#include "AmbientLightAwareSensorEvents.h"
#include "AmbientLightAwareDlg.h"
#include "AmbientLightAwareSensorManagerEvents.h"

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::CAmbientLightAwareSensorEvents
//
// Description of function/method:
//        Constructor.
//
// Parameters:
//        CAmbientLightAwareDlg* dlg: Parent dialog for callbacks
//        CAmbientLightAwareSensorManagerEvents* sensorManagerEvents:
//            Parent class for callbacks
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CAmbientLightAwareSensorEvents::CAmbientLightAwareSensorEvents(CAmbientLightAwareDlg* dlg, CAmbientLightAwareSensorManagerEvents* sensorManagerEvents)
{
    m_lRefCount = 1; //ref count initialized to 1
    m_pParentDlg = dlg;
    m_pSensorManagerEvents = sensorManagerEvents;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::~CAmbientLightAwareSensorEvents
//
// Description of function/method:
//        Destructor. Clean up stored data.
//
// Parameters:
//        none
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CAmbientLightAwareSensorEvents::~CAmbientLightAwareSensorEvents()
{
    m_mapLux.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::QueryInterface
//
// Description of function/method:
//        IUnknown method, need to implement to support COM classes that
//        are inherited.
//
// Parameters:
//        REFIID riid:     Input. ID of the interface being requested. Either
//                         IUnknown or one of the two classes we inherit.
//        void** ppObject: Output. Address of pointer variable that receives
//                         the interface pointer requested in riid. Upon 
//                         successful return, *ppvObject contains the requested
//                         interface pointer to the object. If the object does
//                         not support the interface specified in iid,
//                         *ppvObject is set to NULL.
//
// Return Values:
//        S_OK on success, else E_NOINTERFACE
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAmbientLightAwareSensorEvents::QueryInterface(REFIID riid, void** ppObject)
{
    HRESULT hr = S_OK;

    *ppObject = NULL;
    if (riid == __uuidof(ISensorEvents))
    {
        *ppObject = reinterpret_cast<ISensorEvents*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppObject = reinterpret_cast<IUnknown*>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        (reinterpret_cast<IUnknown*>(*ppObject))->AddRef();
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::AddRef
//
// Description of function/method:
//        Increments the reference count for an interface on an object.
//
// Parameters:
//        none
//
// Return Values:
//        Returns an integer from 1 to n, the value of the new reference count.
//
///////////////////////////////////////////////////////////////////////////////
ULONG _stdcall CAmbientLightAwareSensorEvents::AddRef()
{
    m_lRefCount++;
    return m_lRefCount; 
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::Release
//
// Description of function/method:
//        Decrements the reference count for the calling interface on a object.
//        If the reference count on the object falls to 0, the object is freed
//        from memory.
//
// Parameters:
//        none
//
// Return Values:
//        Returns an integer from 1 to n, the value of the new reference count.
//
///////////////////////////////////////////////////////////////////////////////
ULONG _stdcall CAmbientLightAwareSensorEvents::Release()
{
    ULONG lRet = --m_lRefCount;

    if (m_lRefCount == 0)
    {
        delete this;
    }

    return lRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::OnStateChanged
//
// Description of function/method:
//        Implementation of ISensor.OnStateChanged. Called when permissions of
//        the sensor have changed, such as the sensor being disabled in control
//        panel. If the sensor is not SENSOR_STATE_READY then its lux value
//        should be ignored.
//
// Parameters:
//        ISensor* pSensor:  Sensor that has changed
//        SensorState state: State of the sensor
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::OnStateChanged(ISensor *pSensor, SensorState state)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor)
    {
        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);
        if (SUCCEEDED(hr))
        {
            if (SENSOR_STATE_READY == state)
            {
                hr = GetSensorData(pSensor);
            }
            else
            {
                // If the sensor is not ready, its lux value is ignored
                m_mapLux[idSensor] = -1.0;
                hr = UpdateLux();
            }
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::OnDataUpdated
//
// Description of function/method:
//        Implementation of ISensor.OnDataUpdated.  Called when the sensor has
//        published new data.  This reads in the lux value from the report.
//
// Parameters:
//        ISensor* pSensor:            Sensor that has updated data.
//        ISensorDataReport *pNewData: New data to be read
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::OnDataUpdated(ISensor *pSensor, ISensorDataReport *pNewData)
{
    HRESULT hr = S_OK;

    if ((NULL != pSensor) && (NULL != pNewData))
    {
        hr = GetSensorData(pSensor, pNewData);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::OnEvent
//
// Description of function/method:
//        Implementation of ISensor.OnEevent, a generic or custom sensor event.
//        OnDataUpdated is the event this sample uses to get sensor
//        information.
//
// Parameters:
//        ISensor* pSensor:                  Sensor that has the event.
//        GUID& eventID:                     Type of event.
//        IPortableDeviceValues *pEventData: Data to be read.
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::OnEvent(ISensor* /*pSensor*/, REFGUID /*eventID*/, IPortableDeviceValues* /*pEventData*/)
{
    // Not implemented
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::OnLeave
//
// Description of function/method:
//        Implementation of ISensor.OnLeave.  This sensor is being removed, so
//        it needs to be deleted and freed.
//
// Parameters:
//        REFSENSOR_ID sensorID: 
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::OnLeave(REFSENSOR_ID sensorID)
{
    HRESULT hr = S_OK;

    hr = m_pSensorManagerEvents->RemoveSensor(sensorID); // Callback into parent
    if (SUCCEEDED(hr))
    {
        // Remove the data for this device
        if (m_mapLux.RemoveKey(sensorID))
        {
            hr = UpdateLux();
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::UpdateLux
//
// Description of function/method:
//        Helper function, calculates average lux and does a callback to the
//        parent dialog.
//
// Parameters:
//        none
//
// Return Values:
//        none
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::UpdateLux()
{
    HRESULT hr = S_OK;

    float fpLux = 0.0;
    float fpLuxTemp = 0.0;
    float count = 0;

    POSITION pos = m_mapLux.GetStartPosition();
    while (NULL != pos)
    {
        fpLuxTemp = m_mapLux.GetNextValue(pos);
        if (fpLuxTemp >= 0)
        {
            count++;
            fpLux += fpLuxTemp;
        }
    }

    if (count > 0)
    {
        fpLux = fpLux / count;
    }

    hr = m_pParentDlg->UpdateLux(fpLux, (int) count);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::GetSensorData
//
// Description of function/method:
//        Helper function, get data from a sensor and updates the lux
//
// Parameters:
//        ISensor* pSensor: Input sensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::GetSensorData(ISensor* pSensor)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor)
    {
        CComPtr<ISensorDataReport> spDataReport;
        hr = pSensor->GetData(&spDataReport);
        if (SUCCEEDED(hr))
        {
            hr = GetSensorData(pSensor, spDataReport);
        }
        else
        {
            SENSOR_ID idSensor = GUID_NULL;
            hr = pSensor->GetID(&idSensor);
            if (SUCCEEDED(hr))
            {
                m_mapLux[idSensor] = -1.0;
                hr = UpdateLux();
            }
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorEvents::GetSensorData
//
// Description of function/method:
//        Helper function, get data from a sensor and updates the lux
//
// Parameters:
//        ISensor *pSensor:               Input sensor
//        ISensorDataReport* pDataReport: The data to be read
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorEvents::GetSensorData(ISensor *pSensor, ISensorDataReport *pDataReport)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor && NULL != pDataReport)
    {
        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT pvLux;
            PropVariantInit(&pvLux);
            hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_LIGHT_LEVEL_LUX, &pvLux);
            if (SUCCEEDED(hr))
            {
                // Save the lux value into our member variable
                m_mapLux[idSensor] = V_R4(&pvLux);

                hr = UpdateLux();
            }
            PropVariantClear(&pvLux);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
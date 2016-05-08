//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareSensorManagerEvents.cpp
//
// Description:
//        Implementation of ISensorManagerEvents
//
// Comments: 
//        This is a standard c++ class, but needs to support IUnknown methods
//        to be properly implemented.  This class overrides 
//        ISensorManagerEvents functions to receive updated information from
//        the Sensors API when it is published.
//
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Resource.h"
#include "AmbientLightAwareSensorEvents.h"
#include "AmbientLightAwareSensorManagerEvents.h"
#include "AmbientLightAwareDlg.h"

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::CAmbientLightAwareSensorManagerEvents
//
// Description of function/method:
//        Constructor.
//
// Parameters:
//        CAmbientLightAwareDlg* dlg: pointer to parent dialog for callbacks
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CAmbientLightAwareSensorManagerEvents::CAmbientLightAwareSensorManagerEvents(CAmbientLightAwareDlg* dlg)
{
    m_lRefCount = 1; //ref count initialized to 1
    m_pSensorEvents = new CAmbientLightAwareSensorEvents(dlg, this);
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::~CAmbientLightAwareSensorManagerEvents
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
CAmbientLightAwareSensorManagerEvents::~CAmbientLightAwareSensorManagerEvents()
{
    if (NULL != m_pSensorEvents)
    {
        delete m_pSensorEvents;
        m_pSensorEvents = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::QueryInterface
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
STDMETHODIMP CAmbientLightAwareSensorManagerEvents::QueryInterface(REFIID riid, void** ppObject)
{
    HRESULT hr = S_OK;

    *ppObject = NULL;
    if (riid == __uuidof(ISensorManagerEvents))
    {
        *ppObject = reinterpret_cast<ISensorManagerEvents*>(this);
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
// CAmbientLightAwareSensorManagerEvents::AddRef
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
ULONG _stdcall CAmbientLightAwareSensorManagerEvents::AddRef()
{
    m_lRefCount++;
    return m_lRefCount; 
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::Release
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
ULONG _stdcall CAmbientLightAwareSensorManagerEvents::Release()
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
// CAmbientLightAwareSensorManagerEvents::Initialize
//
// Description of function/method:
//        Initialize the sensor data.
//
// Parameters:
//        none
//
// Return Values:
//        HRESULT S_OK on success
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::Initialize()
{
    HRESULT hr;

    hr = m_spISensorManager.CoCreateInstance(CLSID_SensorManager);
    if (SUCCEEDED(hr))
    {
        hr = m_spISensorManager->SetEventSink(this);
        if (SUCCEEDED(hr))
        {
            // Find all Ambient Light Sensors
            CComPtr<ISensorCollection> spSensors;
            hr = m_spISensorManager->GetSensorsByType(SENSOR_TYPE_AMBIENT_LIGHT, &spSensors);
            if (SUCCEEDED(hr) && NULL != spSensors)
            {
                ULONG ulCount = 0;
                hr = spSensors->GetCount(&ulCount);
                if (SUCCEEDED(hr))
                {
                    for(ULONG i=0; i < ulCount; i++)
                    {
                        CComPtr<ISensor> spSensor;
                        hr = spSensors->GetAt(i, &spSensor);
                        if (SUCCEEDED(hr))
                        {
                            hr = AddSensor(spSensor);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_pSensorEvents->GetSensorData(spSensor);
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::Uninitialize
//
// Description of function/method:
//        Uninitialize the sensor data.
//
// Parameters:
//        none
//
// Return Values:
//        HRESULT S_OK on success
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::Uninitialize()
{
    HRESULT hr = S_OK;

    POSITION pos = m_Sensors.GetStartPosition();
    while (NULL != pos)
    {
        ISensor* pSensor = m_Sensors.GetNextValue(pos);
        RemoveSensor(pSensor);
    }

    if (NULL != m_spISensorManager)
    {
        hr = m_spISensorManager->SetEventSink(NULL);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::OnSensorEnter
//
// Description of function/method:
//        Implementation of ISensorManager.OnSensorEnter. Check if the sensor
//        is an Ambient Light Sensor and if so then add the sensor.
//
// Parameters:
//        ISensor* pSensor:  Sensor that has been installed
//        SensorState state: State of the sensor
//
// Return Values:
//        S_OK on success, else an error.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::OnSensorEnter(ISensor* pSensor, SensorState state)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor)
    {
        SENSOR_TYPE_ID idType = GUID_NULL;
        hr = pSensor->GetType(&idType);
        if (SUCCEEDED(hr))
        {
            if (IsEqualIID(idType, SENSOR_TYPE_AMBIENT_LIGHT))
            {
                hr = AddSensor(pSensor);
                if (SUCCEEDED(hr))
                {
                    if (SENSOR_STATE_READY == state)
                    {
                        hr = m_pSensorEvents->GetSensorData(pSensor);
                    }
                }
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
// CAmbientLightAwareSensorManagerEvents::AddSensor
//
// Description of function/method:
//        Helper function, sets up event sinking for a sensor and saves the
//        sensor.
//
// Parameters:
//        ISensor *pSensor: Input sensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::AddSensor(ISensor *pSensor)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor)
    {
        hr = pSensor->SetEventSink(m_pSensorEvents);
        if (SUCCEEDED(hr))
        {
            // Get the sensor's ID to be used as a key to store the sensor
            SENSOR_ID idSensor = GUID_NULL;
            hr = pSensor->GetID(&idSensor);
            if (SUCCEEDED(hr))
            {
                // Enter the sensor into the map and take the ownership of its lifetime
                pSensor->AddRef(); // the sensor is released in the destructor
                m_Sensors[idSensor] = pSensor;
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
// CAmbientLightAwareSensorManagerEvents::RemoveSensor
//
// Description of function/method:
//        Helper function, clears the event sink for the sensor and then
//        releases the sensor.
//
// Parameters:
//        ISensor *pSensor: Input sensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::RemoveSensor(ISensor* pSensor)
{
    HRESULT hr = S_OK;

    // Release the event and ISensor objecets
    if (NULL != pSensor)
    {
        hr = pSensor->SetEventSink(NULL); // This also decreases the ref count of the sink object.

        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);
        if (SUCCEEDED(hr))
        {
            m_Sensors.RemoveKey(idSensor);
        }

        pSensor->Release();
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareSensorManagerEvents::RemoveSensor
//
// Description of function/method:
//        Helper function, clears the event sink for the sensor and then
//        releases the sensor.
//
// Parameters:
//        REFSENSOR_ID sensorID: Input sensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareSensorManagerEvents::RemoveSensor(REFSENSOR_ID sensorID)
{
    HRESULT hr = S_OK;

    if (m_Sensors.Lookup(sensorID))
    {
        ISensor* pSensor = m_Sensors[sensorID];
        m_Sensors.RemoveKey(sensorID);
        pSensor->Release();
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return hr;
}
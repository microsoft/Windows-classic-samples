//-----------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareSensorManagerEvents.h
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
//-----------------------------------------------------------------------
#pragma once

// Forward declaration.
class CAmbientLightAwareDlg;

class CAmbientLightAwareSensorManagerEvents :
    public ISensorManagerEvents
{
public:
    // These three methods are for IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppObject );
    ULONG _stdcall AddRef();
    ULONG _stdcall Release();

    // Constructor and destructor
    CAmbientLightAwareSensorManagerEvents(CAmbientLightAwareDlg* dlg);
    virtual ~CAmbientLightAwareSensorManagerEvents();

    // Initialize and Uninitialize called by parent dialog
    HRESULT Initialize();
    HRESULT Uninitialize();

    // ISensorManagerEvents method override
    STDMETHOD(OnSensorEnter)(ISensor* pSensor, SensorState state);

    // Is a callback from child SensorEvents
    HRESULT RemoveSensor(REFSENSOR_ID sensorID);

private:
    // Member variable to implement IUnknown reference count
    LONG m_lRefCount;

    // Helper functions
    HRESULT AddSensor(ISensor* pSensor);
    HRESULT RemoveSensor(ISensor* pSensor);

    CAmbientLightAwareSensorEvents* m_pSensorEvents; // Sensor Events class used for event sinking
    CComPtr<ISensorManager> m_spISensorManager;      // Global to keep reference for life of class
    CAtlMap<SENSOR_ID, ISensor*> m_Sensors;          // Map to store sensors for life of class
};

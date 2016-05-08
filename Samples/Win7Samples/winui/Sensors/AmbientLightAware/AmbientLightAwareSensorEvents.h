//-----------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareSensorEvents.h
//
// Description:
//        Implementation of ISensorEvents
//
// Comments: 
//        This is a standard c++ class, but needs to support IUnknown methods
//        to be properly implemented.  This class overrides ISensorEvents and
//        functions to receive updated information from Sensors API when it
//        is published.
//
//-----------------------------------------------------------------------
#pragma once

// Forward declarations.
class CAmbientLightAwareDlg;
class CAmbientLightAwareSensorManagerEvents;

class CAmbientLightAwareSensorEvents :
    public ISensorEvents
{
public:
    // These three methods are for IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppObject );
    ULONG _stdcall AddRef();
    ULONG _stdcall Release();

    // Constructor and destructor
    CAmbientLightAwareSensorEvents(CAmbientLightAwareDlg* dlg, CAmbientLightAwareSensorManagerEvents* sensorManagerEvents);
    virtual ~CAmbientLightAwareSensorEvents();

    // ISensorEvents method overrides
    STDMETHOD(OnStateChanged)(ISensor* pSensor, SensorState state);
    STDMETHOD(OnDataUpdated)(ISensor* pSensor, ISensorDataReport* pNewData);
    STDMETHOD(OnEvent)(ISensor* pSensor, REFGUID eventID, IPortableDeviceValues* pEventData);
    STDMETHOD(OnLeave)(REFSENSOR_ID sensorID);

    // Helper functions, also called by parent SensorManagerEvents class
    HRESULT GetSensorData(ISensor* pSensor);

private:
    // Member variable to implement IUnknown reference count
    LONG m_lRefCount;

    // Helper functions
    HRESULT UpdateLux();
    HRESULT GetSensorData(ISensor* pSensor, ISensorDataReport* pDataReport);

    CAmbientLightAwareDlg* m_pParentDlg;                           // Parent dialog used for callbacks
    CAmbientLightAwareSensorManagerEvents* m_pSensorManagerEvents; // Parent class for callbacks
    CAtlMap<SENSOR_ID, float> m_mapLux;                           // Map to store lux values for each sensor
};

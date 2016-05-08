// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CManipulationEventSink.h
//
// The CManipulationEventSink class implements the _IManipulationEvents interface 
// for eventing. The events from this class will be called by the 
// ManipulationProcessor object. 

#pragma once

// COM header files
#include <ole2.h>
#include <ocidl.h>

// Manipulation header files
#include <manipulations.h>

// Application specific header files
#include "CDrawingObject.h"

class CManipulationEventSink : public _IManipulationEvents
{
public:
    CManipulationEventSink(CDrawingObject* pcDrawingObject);
    virtual ~CManipulationEventSink();

    bool Connect(IManipulationProcessor *pManipProc);
    bool Disconnect();

    // IManipulationEvents methods
    virtual HRESULT STDMETHODCALLTYPE ManipulationStarted(
        FLOAT x,
        FLOAT y);
    
    virtual HRESULT STDMETHODCALLTYPE ManipulationDelta( 
        FLOAT x,
        FLOAT y,
        FLOAT translationDeltaX,
        FLOAT translationDeltaY,
        FLOAT scaleDelta,
        FLOAT expansionDelta,
        FLOAT rotationDelta,
        FLOAT cumulativeTranslationX,
        FLOAT cumulativeTranslationY,
        FLOAT cumulativeScale,
        FLOAT cumulativeExpansion,
        FLOAT cumulativeRotation);
    
    virtual HRESULT STDMETHODCALLTYPE ManipulationCompleted( 
        FLOAT x,
        FLOAT y,
        FLOAT cumulativeTranslationX,
        FLOAT cumulativeTranslationY,
        FLOAT cumulativeScale,
        FLOAT cumulativeExpansion,
        FLOAT cumulativeRotation);

    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

private:
    // Reference count of the COM object.
    LONG m_cRefCount;

    // The IConnectionPoint interface supports connection points for 
    // connectable objects.
    IConnectionPoint* m_pConnection;

    // Cookie of the connection.
    DWORD m_dwCookie;

    // Pointer to the CDrawingObject (contains the rectangle information)
    CDrawingObject* m_pcDrawingObject;
};     
    

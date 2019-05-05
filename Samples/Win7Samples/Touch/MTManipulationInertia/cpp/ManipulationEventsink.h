// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef MANIPULATIONEVENTSINK_H
#define MANIPULATIONEVENTSINK_H

#include <ocidl.h>
#include <manipulations.h>

#define DESIRED_MILLISECONDS 10

// Forward declaration of CCoreObject
class CCoreObject;

class CManipulationEventSink : _IManipulationEvents
{
public:
    CManipulationEventSink(HWND hWnd, CCoreObject* coRef, int iTimerId, BOOL inertia);
   
    // Handles event when the manipulation begins
    virtual HRESULT STDMETHODCALLTYPE ManipulationStarted(
        FLOAT x, 
        FLOAT y);

    // Handles event when the manipulation is progress
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

    // Handles event when the manipulation ends
    virtual HRESULT STDMETHODCALLTYPE ManipulationCompleted(
        FLOAT x,
        FLOAT y,
        FLOAT cumulativeTranslationX,
        FLOAT cumulativeTranslationY,
        FLOAT cumulativeScale,
        FLOAT cumulativeExpansion,
        FLOAT cumulativeRotation);
    
    // Helper for creating a connection point
    BOOL SetupConnPt(IUnknown* manipulationProc);
    VOID RemoveConnPt();

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

private:
    HRESULT SetupInertia(IInertiaProcessor* ip, IManipulationProcessor* mp);
    
    IConnectionPoint* m_pConnPoint;
    volatile unsigned int m_cRefCount;
    DWORD m_uID;
    
    // Reference to the object to manipulate
    CCoreObject* m_coRef;

    // Handle to the window
    HWND m_hWnd;

    // Unique timer identifier for this processor
    int m_iTimerId;

    // Flag the determines if this event sink handles inertia
    BOOL m_bInertia;
};

#endif
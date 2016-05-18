// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ManipulationEventsink.h"
#include "CoreObject.h"
#include <math.h>

CManipulationEventSink::CManipulationEventSink(HWND hWnd, CCoreObject *coRef, int iTimerId, BOOL inertia): 
    m_coRef(coRef), 
    m_hWnd(hWnd), 
    m_iTimerId(iTimerId), 
    m_bInertia(inertia), 
    m_pConnPoint(NULL),
    m_cRefCount(1)
{
}

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationStarted(
    FLOAT x,
    FLOAT y)
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);

    // Stop object if it is in the state of inertia

    m_coRef->bIsInertiaActive = FALSE;
    KillTimer(m_hWnd, m_iTimerId);
    
    m_coRef->doDrawing->RestoreRealPosition();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationDelta(
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
    FLOAT cumulativeRotation)
{
    UNREFERENCED_PARAMETER(cumulativeRotation);
    UNREFERENCED_PARAMETER(cumulativeExpansion);
    UNREFERENCED_PARAMETER(cumulativeScale);
    UNREFERENCED_PARAMETER(cumulativeTranslationX);
    UNREFERENCED_PARAMETER(cumulativeTranslationY);
    UNREFERENCED_PARAMETER(expansionDelta);

    CDrawingObject* dObj = m_coRef->doDrawing;
    IManipulationProcessor* mp= m_coRef->manipulationProc;

    HRESULT hr = S_OK;

    // Apply transformation based on rotationDelta (in radians)
    FLOAT rads = 180.0f / 3.14159f;
    
    dObj->SetManipulationOrigin(x, y);

    dObj->Rotate(rotationDelta*rads);

    // Apply translation based on scaleDelta
    dObj->Scale(scaleDelta);

    // Apply translation based on translationDelta
    dObj->Translate(translationDeltaX, translationDeltaY, m_bInertia);

    if(!m_bInertia) 
    {
        // Set values for one finger rotations

        FLOAT fPivotRadius = (FLOAT)(sqrt(pow(dObj->GetWidth()/2, 2)+ pow(dObj->GetHeight()/2, 2)))*0.4f;
        FLOAT fPivotPtX = dObj->GetCenterX();
        FLOAT fPivotPtY = dObj->GetCenterY();
        
        HRESULT hrPPX = mp->put_PivotPointX(fPivotPtX);
        HRESULT hrPPY = mp->put_PivotPointY(fPivotPtY);
        HRESULT hrPR  = mp->put_PivotRadius(fPivotRadius);
        
        if(FAILED(hrPPX) || FAILED(hrPPY) || FAILED(hrPR))
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationCompleted(
    FLOAT x,
    FLOAT y,
    FLOAT cumulativeTranslationX,
    FLOAT cumulativeTranslationY,
    FLOAT cumulativeScale,
    FLOAT cumulativeExpansion,
    FLOAT cumulativeRotation)
{
    UNREFERENCED_PARAMETER(cumulativeRotation);
    UNREFERENCED_PARAMETER(cumulativeExpansion);
    UNREFERENCED_PARAMETER(cumulativeScale);
    UNREFERENCED_PARAMETER(cumulativeTranslationX);
    UNREFERENCED_PARAMETER(cumulativeTranslationY);
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);

    HRESULT hr = S_OK;
    
    IInertiaProcessor* ip = m_coRef->inertiaProc;
    IManipulationProcessor* mp = m_coRef->manipulationProc;

    if(!m_bInertia)
    {    
        HRESULT hrSI = SetupInertia(ip, mp);
        HRESULT hrCO = S_OK;

        if(FAILED(hrSI) || FAILED(hrCO))
        {
            hr = E_FAIL;
        }

        // Set the core objects inertia state to TRUE so it can
        // be processed when another object is being manipulated
        m_coRef->bIsInertiaActive = TRUE;
        
        // Kick off timer that handles inertia
        SetTimer(m_hWnd, m_iTimerId, DESIRED_MILLISECONDS, NULL);
    } 
    else
    {
        m_coRef->bIsInertiaActive = FALSE;

        // Stop timer that handles inertia
        KillTimer(m_hWnd, m_iTimerId);
    }
    return hr;
}

HRESULT CManipulationEventSink::SetupInertia(IInertiaProcessor* ip, IManipulationProcessor* mp)
{
    HRESULT hr = S_OK;

    // Set desired properties for inertia events

    // Deceleration for tranlations in pixel / msec^2
    HRESULT hrPutDD = ip->put_DesiredDeceleration(0.003f);

    // Deceleration for rotations in radians / msec^2
    HRESULT hrPutDAD = ip->put_DesiredAngularDeceleration(0.000015f);

    // Set initial origins

    HRESULT hrPutIOX = ip->put_InitialOriginX(m_coRef->doDrawing->GetCenterX());
    HRESULT hrPutIOY = ip->put_InitialOriginY(m_coRef->doDrawing->GetCenterY());
    
    FLOAT fVX;
    FLOAT fVY;
    FLOAT fVR;

    HRESULT hrPutVX = mp->GetVelocityX(&fVX);
    HRESULT hrGetVY = mp->GetVelocityY(&fVY);
    HRESULT hrGetAV = mp->GetAngularVelocity(&fVR);

    // Set initial velocities for inertia processor

    HRESULT hrPutIVX = ip->put_InitialVelocityX(fVX);
    HRESULT hrPutIVY = ip->put_InitialVelocityY(fVY);
    HRESULT hrPutIAV = ip->put_InitialAngularVelocity(fVR);

    if(FAILED(hrPutDD) || FAILED(hrPutDAD) || FAILED(hrPutIOX) || FAILED(hrPutIOY)
        || FAILED(hrPutVX) || FAILED(hrGetVY) || FAILED(hrGetAV) || FAILED(hrPutIVX)
        || FAILED(hrPutIVY) || FAILED(hrPutIAV))
    {
        hr = E_FAIL;
    }

    return hr;
}

ULONG CManipulationEventSink::AddRef()
{
    return ++m_cRefCount;
}

ULONG CManipulationEventSink::Release()
{
    m_cRefCount--;

    if(m_cRefCount == 0)
    {
        delete this;
        return 0;
    }
    return m_cRefCount;
}

HRESULT CManipulationEventSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = S_OK;

    if(ppvObj == NULL)
    {
        hr = E_POINTER;
    }

    if(!FAILED(hr))
    {
        *ppvObj = NULL;

        if (IID__IManipulationEvents == riid)
        {
            *ppvObj = static_cast<_IManipulationEvents*>(this);
        }
        else if (IID_IUnknown == riid)
        {
            *ppvObj = static_cast<IUnknown*>(this);
        }
        
        if(*ppvObj)
        {
            AddRef();
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

// Set up the connection to a manipulation or inertia processor
BOOL CManipulationEventSink::SetupConnPt(IUnknown *manipulationProc)
{
    BOOL success = FALSE;
    IConnectionPointContainer* pConPointContainer = NULL;

    // Only connect if there isn't already an active connection
    if (m_pConnPoint == NULL)
    {
        // Check if supports connectable objects
        success = SUCCEEDED(manipulationProc->QueryInterface(IID_IConnectionPointContainer, 
            (LPVOID*)&(pConPointContainer)));

        // Get connection point interface
        if(success)
        {
            success = SUCCEEDED(pConPointContainer->FindConnectionPoint(
                _uuidof(_IManipulationEvents), 
                &(m_pConnPoint)));
        }

        // Clean up connection point container
        if (pConPointContainer != NULL)
        {
            pConPointContainer->Release();
            pConPointContainer = NULL;
        }
        
        // Hook event object to the connection point
        IUnknown* pUnk = NULL;
        if(success)
        {
            // Get pointer to manipulation event sink's IUnknown pointer
            success = SUCCEEDED(QueryInterface(IID_IUnknown, (LPVOID*)&pUnk));
        }

        // Establish connection point to callback interface
        if(success)
        {
            success = SUCCEEDED(m_pConnPoint->Advise(pUnk, &(m_uID)));
        }
       
        // Clean up IUnknown pointer
        if(pUnk != NULL)
        {
            pUnk->Release();
        }

        if (!success && m_pConnPoint != NULL)
        {
            m_pConnPoint->Release();
            m_pConnPoint = NULL;
        }
    }

    return success;
}

VOID CManipulationEventSink::RemoveConnPt()
{
    // Clean up the connection point associated to this event sink
    if(m_pConnPoint)
    {
       m_pConnPoint->Unadvise(m_uID);
       m_pConnPoint->Release();
       m_pConnPoint = NULL;
    }
}
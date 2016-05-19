// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "CoreObject.h"
#include <manipulations_i.c>
#include <new>

CCoreObject::CCoreObject(HWND hwnd, int iTimerId, CD2DDriver* d2dDriver) : 
    m_hWnd(hwnd), 
    m_iTimerId(iTimerId), 
    m_d2dDriver(d2dDriver),
    doDrawing(NULL),
    manipulationProc(NULL),
    inertiaProc(NULL)
{
}

BOOL CCoreObject::Initialize()
{   
    BOOL success = SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

     // Create an instance of the COM manipulation processor
    if(success)
    {
        success = SUCCEEDED(CoCreateInstance(CLSID_ManipulationProcessor,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IUnknown,
            (VOID**)(&manipulationProc)));
    }

    // Create an instance of the COM inertia processor
    if(success)
    {
        success = SUCCEEDED(CoCreateInstance(CLSID_InertiaProcessor,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IUnknown,
            (VOID**)(&inertiaProc)));
    }

    // Create graphical representation of this core object
    if(success)
    {
        doDrawing = new (std::nothrow) CDrawingObject(m_hWnd, m_d2dDriver);

        if(doDrawing == NULL)
        {
            success = FALSE;
        }
    }

    // Create manipulation event sink for this core object 
    // and setup connection point to manipulation processor
    if(success)
    {
        manipulationEventSink = new (std::nothrow) CManipulationEventSink(m_hWnd, this, m_iTimerId, FALSE);
        
        if(manipulationEventSink == NULL)
        {
            success = FALSE;
        }

        if(success)
        {
            success = manipulationEventSink->SetupConnPt(manipulationProc);
        }
    }

    // Create inertia event sink for this core object 
    // and setup connection point to inertia processor
    if(success)
    {
        inertiaEventSink = new (std::nothrow) CManipulationEventSink(m_hWnd, this, m_iTimerId, TRUE);

        if(inertiaEventSink == NULL)
        {
            success = FALSE;
        }

        if(success)
        {
            success = inertiaEventSink->SetupConnPt(inertiaProc);
        }
    }

    // Initially object does not have inertia
    if(success)
    {
        bIsInertiaActive = FALSE;
    }

    return success;
}

CCoreObject::~CCoreObject()
{

    // Clean up graphical representation, event sinks and processors

    if(doDrawing)
    {
        delete doDrawing;
    }

    if(manipulationEventSink)
    {
        manipulationEventSink->RemoveConnPt();
        manipulationEventSink->Release();
        manipulationEventSink = NULL;
    }

    if(inertiaEventSink)
    {
        inertiaEventSink->RemoveConnPt();
        inertiaEventSink->Release();
        inertiaEventSink = NULL;
    }

    if(manipulationProc)
    {
        manipulationProc->Release();
    }

    if(inertiaProc)
    {
        inertiaProc->Release();
    }

    CoUninitialize();
}

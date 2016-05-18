// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef COREOBJECT_H
#define COREOBJECT_H

#include "ManipulationEventsink.h"
#include "DrawingObject.h"

class CCoreObject {
public:
    CCoreObject(HWND hwnd, int iTimerId, CD2DDriver* d2dDriver);
    ~CCoreObject();
    BOOL Initialize();

    // Rendered object
    CDrawingObject* doDrawing;
    
    // Manipulation and Inertia event sinks
    CManipulationEventSink* manipulationEventSink;
    CManipulationEventSink* inertiaEventSink;
   
    // Manipulation and Inertia processors
    IManipulationProcessor* manipulationProc;
    IInertiaProcessor* inertiaProc;

    // This flag is set by the manipulation event sink
    // when the ManipulationCompleted method is invoked
    BOOL bIsInertiaActive;

private:
    HWND m_hWnd;

    // Direct2D driver to pass to drawing object
    CD2DDriver* m_d2dDriver; 

    // Unique timer ID to pass to event sink
    int m_iTimerId;
};

#endif
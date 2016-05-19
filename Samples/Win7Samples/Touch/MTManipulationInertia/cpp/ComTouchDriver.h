// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef COMTOUCHDRIVER_H
#define COMTOUCHDRIVER_H

#include "CoreObject.h"
#include <map>
#include <list>

#define NUM_CORE_OBJECTS 4
#define MOUSE_CURSOR_ID 0
#define DEFAULT_PPI 96.0f

class CComTouchDriver {
public:
    CComTouchDriver(HWND hWnd);
    ~CComTouchDriver();
    
    // Initializes Core Objects, Manipulation Processors and Inertia Processors
    BOOL Initialize();

    // Processes the input information and activates the appropriate processor
    VOID ProcessInputEvent(const TOUCHINPUT* inData);

    // Sets up the initial state of the objects
    VOID RenderInitialState(const int iCWidth, const int iCHeight);

    // Processes all changes that include active inertia processors
    // Note: ProcessChanges automatically calls RenderObjects()
    VOID ProcessChanges();
    
    // Localizes point for high-DPI
    int GetLocalizedPointX(int ptX);
    int GetLocalizedPointY(int ptY);

private:
    // Renders the objects to the screen
    VOID RenderObjects();

    // Event helpers for processing input events

    VOID DownEvent(CCoreObject* coRef, const TOUCHINPUT* inData, BOOL* bFound);
    VOID MoveEvent(const TOUCHINPUT* inData);
    VOID UpEvent(const TOUCHINPUT* inData);

    // Map of cursor ids and core obejcts
    std::map<DWORD, CCoreObject*> m_mCursorObject;
  
    // List of core objects to be manipulated
    std::list<CCoreObject*> m_lCoreObjects;

    // The client width and height
    int m_iCWidth;
    int m_iCHeight;

    // Scale for converting between dpi's
    FLOAT m_dpiScaleX;
    FLOAT m_dpiScaleY;

    // Keeps track of the number contacts being processed
    unsigned int m_uNumContacts;

    CD2DDriver* m_d2dDriver;

    // Handle to window
    HWND m_hWnd;
};

#endif
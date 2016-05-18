// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ComTouchDriver.h"

CComTouchDriver::CComTouchDriver(HWND hWnd):
    m_hWnd(hWnd), 
    m_uNumContacts(0),
    m_dpiScaleX(1.0f), 
    m_dpiScaleY(1.0f)
{
}

BOOL CComTouchDriver::Initialize()
{
    BOOL success = TRUE;

    // Calculate dpi for high-DPI systems
 
    HDC hdcScreen = GetDC(m_hWnd);
    
    if(hdcScreen)
    {
        // Direct2D automatically does work in logical, so compute the
        // scale to convert from physical to logical coordinates

        m_dpiScaleX = (FLOAT)(DEFAULT_PPI / GetDeviceCaps(hdcScreen, LOGPIXELSX));
        m_dpiScaleY = (FLOAT)(DEFAULT_PPI / GetDeviceCaps(hdcScreen, LOGPIXELSY));
        DeleteDC(hdcScreen);
    }
    
    // Create and initialize D2DDriver

    m_d2dDriver = new (std::nothrow) CD2DDriver(m_hWnd);
    if(m_d2dDriver == NULL)
    {
        success = FALSE;
    }

    if(success)
    {
        success = SUCCEEDED(m_d2dDriver->Initialize());
    }

    // Create and initialize core objects

    if(success)
    {
        for(int i = 0; i < NUM_CORE_OBJECTS; i++)
        {
            CCoreObject* object = NULL;
            
            object = new (std::nothrow) CCoreObject(m_hWnd, i, m_d2dDriver);

            if(object == NULL)
            {
                success = FALSE;
            }
            
            if(success)
            {
                success = object->Initialize();
            }
            
            // Append core object to the list
            if(success)
            {
                try
                {
                    m_lCoreObjects.push_front(object);
                }
                catch(std::bad_alloc)
                {
                    success = FALSE;
                }
            }

            // Clean up and leave if initialization failed
            if(!success)
            {
                if(object)
                {
                    delete object;
                }
                break;
            }
        }
    }
    return success;
}

CComTouchDriver::~CComTouchDriver()
{
    std::list<CCoreObject*>::iterator it;

    // Clean up all core objects in the list
    for(it = m_lCoreObjects.begin(); it != m_lCoreObjects.end(); ++it)
    {
        if(*it)
        {
            delete *it;
        }
    }

    m_lCoreObjects.clear();

    // Clean up d2d driver
    if(m_d2dDriver)
    {
        delete m_d2dDriver;
    }
}

VOID CComTouchDriver::ProcessInputEvent(const TOUCHINPUT* inData)
{
    DWORD dwCursorID = inData->dwID;
    DWORD dwEvent = inData->dwFlags;
    BOOL bFoundObj = FALSE;	

    
    // Check if contacts should be incremented
    if((dwEvent & TOUCHEVENTF_DOWN) && (dwCursorID != MOUSE_CURSOR_ID))
    {
        m_uNumContacts++;
    }

    // Screen the types of inputs and the number of contacts
    if((m_uNumContacts == 0) && (dwCursorID != MOUSE_CURSOR_ID))
    {
        return;
    }
    else if((m_uNumContacts > 0) && (dwCursorID == MOUSE_CURSOR_ID))
    {
        return;
    }

    // Check if contacts should be decremented
    if((dwEvent & TOUCHEVENTF_UP) && (dwCursorID != MOUSE_CURSOR_ID))
    {
        m_uNumContacts--;
    }    
    
    // Find the object and associate the cursor id with the object
    if(dwEvent & TOUCHEVENTF_DOWN)
    {
        std::list<CCoreObject*>::iterator it;
        for(it = m_lCoreObjects.begin(); it != m_lCoreObjects.end(); it++)
        {
            DownEvent(*it, inData, &bFoundObj);
            if(bFoundObj) break;
        }
    }
    else if(dwEvent & TOUCHEVENTF_MOVE)
    {
        if(m_mCursorObject.count(inData->dwID) > 0)
        {
            MoveEvent(inData);
        }
    }
    else if(dwEvent & TOUCHEVENTF_UP)
    {
        if(m_mCursorObject.count(inData->dwID) > 0)
        {
            UpEvent(inData);
        }
    }
}

// The subsequent methods are helpers for processing the event input

VOID CComTouchDriver::DownEvent(CCoreObject* coRef, const TOUCHINPUT* inData, BOOL* bFound)
{
    DWORD dwCursorID = inData->dwID;
    DWORD dwPTime = inData->dwTime;
    int x = GetLocalizedPointX(inData->x);
    int y = GetLocalizedPointY(inData->y);
    BOOL success = TRUE;

    // Check that the user has touched within an objects region and feed to the objects manipulation processor

    if(coRef->doDrawing->InRegion(x, y))
    {
        // Feed values to the Manipulation Processor
        success = SUCCEEDED(coRef->manipulationProc->ProcessDownWithTime(dwCursorID, (FLOAT)x, (FLOAT)y, dwPTime));
        
        if(success)
        {
            try
            {
                // Add to the cursor id -> object mapping
                m_mCursorObject.insert(std::pair<DWORD, CCoreObject*>(dwCursorID, coRef));
            }
            catch(std::bad_alloc)
            {
                coRef->manipulationProc->CompleteManipulation();
                success = FALSE;
            }
        }

        if(success)
        {
            // Make the current object the new head of the list
            m_lCoreObjects.remove(coRef);
            m_lCoreObjects.push_front(coRef);
            
            *bFound = TRUE;
            
            // Renders objects to bring new object to front
            RenderObjects();
        }
    }
    else
    {
        *bFound = FALSE;
    }
}

VOID CComTouchDriver::MoveEvent(const TOUCHINPUT* inData)
{
    DWORD dwCursorID  = inData->dwID;
    DWORD dwPTime = inData->dwTime;
    int x = GetLocalizedPointX(inData->x);
    int y = GetLocalizedPointY(inData->y);

    // Get the object associated with this cursor id
    std::map<DWORD, CCoreObject*>::iterator it = m_mCursorObject.find(dwCursorID);
    if(it != m_mCursorObject.end())
    {
        CCoreObject* coRef = (*it).second;

        // Feed values into the manipulation processor
        coRef->manipulationProc->ProcessMoveWithTime(dwCursorID, (FLOAT)x, (FLOAT)y, dwPTime);
    }
}

VOID CComTouchDriver::UpEvent(const TOUCHINPUT* inData)
{
    DWORD dwCursorID = inData->dwID;
    DWORD dwPTime = inData->dwTime;
    int x = GetLocalizedPointX(inData->x);
    int y = GetLocalizedPointY(inData->y);
    BOOL success = FALSE;

    // Get the CoreObject associated with this cursor id
    std::map<DWORD, CCoreObject*>::iterator it = m_mCursorObject.find(dwCursorID);
    if(it != m_mCursorObject.end())
    {
        CCoreObject* coRef = (*it).second;

        // Feed values into the manipulation processor
        success = SUCCEEDED(coRef->manipulationProc->ProcessUpWithTime(dwCursorID, (FLOAT)x, (FLOAT)y, dwPTime));
    }

    // Remove the cursor, object mapping
    if(success)
    {
        m_mCursorObject.erase(dwCursorID);
    }
}

// Handler for activating the inerita processor
VOID CComTouchDriver::ProcessChanges()
{
    // Run through the list of core objects and process any of its active inertia processors
    std::list<CCoreObject*>::iterator it;

    for(it = m_lCoreObjects.begin(); it != m_lCoreObjects.end(); ++it)
    {
        if((*it)->bIsInertiaActive == TRUE)
        {
            BOOL bCompleted = FALSE;
            (*it)->inertiaProc->Process(&bCompleted);
        }
    }
    // Render all the changes
    RenderObjects();
}

// The subsequent set of methods are for handling the rendering work

VOID CComTouchDriver::RenderObjects()
{
    m_d2dDriver->BeginDraw();
    m_d2dDriver->RenderBackground((FLOAT)m_iCWidth, (FLOAT)m_iCHeight);	
    
    std::list<CCoreObject*>::reverse_iterator it;

    for(it = m_lCoreObjects.rbegin(); it != m_lCoreObjects.rend(); ++it) 
    {
        (*it)->doDrawing->Paint();
    }

    m_d2dDriver->EndDraw();
}

VOID CComTouchDriver::RenderInitialState(const int iCWidth, const int iCHeight)
{
    m_iCWidth = iCWidth;
    m_iCHeight = iCHeight;

    int widthScaled = GetLocalizedPointX(iCWidth);
    int heightScaled = GetLocalizedPointY(iCHeight);

    // Defines default position for objects
    POINTF pfObjPos[NUM_CORE_OBJECTS];
    CDrawingObject::DrawingColor uObjColor[NUM_CORE_OBJECTS];
    int i = 0;

    pfObjPos[0].x = widthScaled  / 2.0f-205.0f;
    pfObjPos[0].y = heightScaled / 2.0f-205.0f;
    pfObjPos[1].x = widthScaled  / 2.0f+5.0f;
    pfObjPos[1].y = heightScaled / 2.0f-205.0f;
    pfObjPos[2].x = widthScaled  / 2.0f-205.0f;
    pfObjPos[2].y = heightScaled / 2.0f+5.0f;
    pfObjPos[3].x = widthScaled  / 2.0f+5.0f;
    pfObjPos[3].y = heightScaled / 2.0f+5.0f;
    
    // Defines color for objects

    uObjColor[0] = CDrawingObject::Red;
    uObjColor[1] = CDrawingObject::Green;
    uObjColor[2] = CDrawingObject::Blue;
    uObjColor[3] = CDrawingObject::Orange;


    // Assign the setup defined above to each of the core objects
    std::list<CCoreObject*>::iterator it;

    for(it = m_lCoreObjects.begin(); it != m_lCoreObjects.end(); ++it)
    {
        (*it)->doDrawing->ResetState(pfObjPos[i].x, pfObjPos[i].y, iCWidth, iCHeight, widthScaled, heightScaled, uObjColor[i]);
        i++;
    }

    RenderObjects();
}

int CComTouchDriver::GetLocalizedPointX(int ptX)
{
    return (int)(ptX * m_dpiScaleX);
}

int CComTouchDriver::GetLocalizedPointY(int ptY)
{
    return (int)(ptY * m_dpiScaleY);
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// GestureEngine.cpp: implementation of CGestureEngine class

// C RunTime header files
#include <assert.h>
#define ASSERT assert

#define _USE_MATH_DEFINES // has to be defined to activate definition of M_PI
#include <math.h>

#include "GestureEngine.h" // contains definition of this class
#ifndef GID_PRESSANDTAP
    #define GID_PRESSANDTAP GID_ROLLOVER
#endif

// One of the fields in GESTUREINFO structure is type of ULONGLONG (8 bytes).
// The relevant gesture information is stored in lower 4 bytes. This
// macro is used to read gesture information from this field.
#define LODWORD(ull) ((DWORD)((ULONGLONG)(ull) & 0x00000000ffffffff))

// Default constructor of the class.
CGestureEngine::CGestureEngine()
:   _dwArguments(0)
{
}

// Destructor
CGestureEngine::~CGestureEngine()
{
}

// Main function of this class decodes gesture information
// in:
//      hWnd        window handle
//      wParam      message parameter (message-specific)
//      lParam      message parameter (message-specific)
LRESULT CGestureEngine::WndProc(HWND hWnd, WPARAM /* wParam */, LPARAM lParam)
{
    // helper variables
    POINT ptZoomCenter;
    double k;

    GESTUREINFO gi;
    gi.cbSize = sizeof(gi);
    BOOL bResult = GetGestureInfo((HGESTUREINFO)lParam, &gi);

    if (!bResult)
    {
        ASSERT(L"Error in execution of GetGestureInfo" && 0);
        return FALSE;
    }

    switch (gi.dwID)
    {
    case GID_BEGIN:
        break;

    case GID_END:
        break;
    
    case GID_ZOOM:
        switch (gi.dwFlags)
        {
        case GF_BEGIN:
            _dwArguments = LODWORD(gi.ullArguments);
            _ptFirst.x = gi.ptsLocation.x;
            _ptFirst.y = gi.ptsLocation.y;
            ScreenToClient(hWnd,&_ptFirst);
            break;

        default:
            // We read here the second point of the gesture. This is middle point between 
            // fingers in this new position.
            _ptSecond.x = gi.ptsLocation.x;
            _ptSecond.y = gi.ptsLocation.y;
            ScreenToClient(hWnd,&_ptSecond);

            // We have to calculate zoom center point 
            ptZoomCenter.x = (_ptFirst.x + _ptSecond.x)/2;
            ptZoomCenter.y = (_ptFirst.y + _ptSecond.y)/2;           
            
            // The zoom factor is the ratio between the new and the old distance. 
            // The new distance between two fingers is stored in gi.ullArguments 
            // (lower DWORD) and the old distance is stored in _dwArguments.
            k = (double)(LODWORD(gi.ullArguments))/(double)(_dwArguments);

            // Now we process zooming in/out of the object
            ProcessZoom(k, ptZoomCenter.x, ptZoomCenter.y);

            InvalidateRect(hWnd, NULL, TRUE);

            // Now we have to store new information as a starting information 
            // for the next step in this gesture.
            _ptFirst = _ptSecond;
            _dwArguments = LODWORD(gi.ullArguments);
            break;
        }
        break;
    
    case GID_PAN:
        switch (gi.dwFlags)
        {
        case GF_BEGIN:
            _ptFirst.x = gi.ptsLocation.x;
            _ptFirst.y = gi.ptsLocation.y;
            ScreenToClient(hWnd, &_ptFirst);
            break;

        default:
            // We read the second point of this gesture. It is a middle point
            // between fingers in this new position
            _ptSecond.x = gi.ptsLocation.x;
            _ptSecond.y = gi.ptsLocation.y;
            ScreenToClient(hWnd, &_ptSecond);

            // We apply move operation of the object
            ProcessMove(_ptSecond.x-_ptFirst.x, _ptSecond.y-_ptFirst.y);

            InvalidateRect(hWnd, NULL, TRUE);

            // We have to copy second point into first one to prepare
            // for the next step of this gesture.
            _ptFirst = _ptSecond;
            break;
        }
        break;

    case GID_ROTATE:
        switch (gi.dwFlags)
        {
        case GF_BEGIN:
            _dwArguments = 0;
            break;

        default:
            _ptFirst.x = gi.ptsLocation.x;
            _ptFirst.y = gi.ptsLocation.y;
            ScreenToClient(hWnd, &_ptFirst);           
            // Gesture handler returns cumulative rotation angle. However we
            // have to pass the delta angle to our function responsible 
            // to process the rotation gesture.
            ProcessRotate(
                GID_ROTATE_ANGLE_FROM_ARGUMENT(LODWORD(gi.ullArguments)) 
                - GID_ROTATE_ANGLE_FROM_ARGUMENT(_dwArguments),
                _ptFirst.x,_ptFirst.y
            );
            InvalidateRect(hWnd, NULL, TRUE);
            _dwArguments = LODWORD(gi.ullArguments);
            break;
        }
        break;

    case GID_TWOFINGERTAP:
        ProcessTwoFingerTap();
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case GID_PRESSANDTAP:
        switch (gi.dwFlags)
        {
        case GF_BEGIN:
            ProcessPressAndTap();
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        break;
    }

    CloseGestureInfoHandle((HGESTUREINFO)lParam);

    return TRUE;
}

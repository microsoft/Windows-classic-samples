// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// CGestureEngine.h
//
// Definition of helper classes for processing of gestures. During this process
// driver sends subsequent finger position information (reference point) and 
// in the some cases other information.

#pragma once
#include <windows.h>

// CGestureEngine class is abstract class that is responsible for 
// properly decoding information from multi-touch driver. There are
// few pure virtual functions that are responsible for manipulation 
// of the object.
class CGestureEngine
{
public:
    CGestureEngine();
    ~CGestureEngine();

    // Process WM_GESTURE messages
    virtual LRESULT WndProc(HWND hWnd, WPARAM wParam, LPARAM lParam);

    // This function is called when press and tap gesture is recognized
    virtual void ProcessPressAndTap() = 0;

    // This function is invoked when two finger tap gesture is recognized
    virtual void ProcessTwoFingerTap() = 0;

    // This function is called constantly through duration of zoom in/out gesture
    virtual void ProcessZoom(const double dZoomFactor, const LONG lZx, const LONG lZy) = 0;

    // This function is called throughout the duration of the panning/inertia gesture
    virtual void ProcessMove(const LONG ldx, const LONG ldy) = 0;

    // This function is called throughout the duration of the rotation gesture
    virtual void ProcessRotate(const double dAngle, const LONG lOx, const LONG lOy) = 0;

private:
    POINT _ptFirst;        // first significant point of the gesture
    POINT _ptSecond;       // second significant point of the gesture
    DWORD _dwArguments;    // 4 bytes long argument
};

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// CMyGestureEngine.h
//
// Definition of derived class that handles gesture operations. This
// class gets pointer to the rectangle object through constructor and
// then it just invokes coresponding function from CDrawingObject class

#pragma once
#include "GestureEngine.h"
#include "DrawingObject.h"

class CMyGestureEngine : public CGestureEngine
{
public:
    CMyGestureEngine(CDrawingObject* pcRect);
    ~CMyGestureEngine();

    // Functions that are handling gesture commands
    virtual void ProcessPressAndTap();
    virtual void ProcessTwoFingerTap();
    virtual void ProcessMove(const LONG ldx, const LONG ldy);
    virtual void ProcessZoom(const double dZoomFactor, const LONG lZx, const LONG lZy);
    virtual void ProcessRotate(const double dAngle, const LONG lOx, const LONG lOy);

private:
    CDrawingObject* _pcRect;
};

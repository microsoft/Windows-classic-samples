//////////////////////////////////////////////////////////////////////////
// Button.cpp: Button control class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// File: Button.cpp
// Desc: Button control classes
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "wincontrol.h"
#include "button.h"

/****************************** Button class ****************************/

//-----------------------------------------------------------------------------
// Name: CreateText
// Description: Creates a simple text button
//
// hParent:   Parent window
// szCaption: Text on the button
// nID:       Command ID.
// rcBound:   Bounding rectangle.
//-----------------------------------------------------------------------------

HRESULT Button::CreateText(HWND hParent, const TCHAR *szCaption, int nID, 
                               const Rect& rcBound)
{
    CREATESTRUCT create;
	ZeroMemory(&create, sizeof(CREATESTRUCT));

    create.x = rcBound.left;
    create.y = rcBound.top;
    create.cx = rcBound.right - create.x;
    create.cy = rcBound.bottom - create.y;

    create.hwndParent = hParent;
    create.lpszName = szCaption;
    create.hMenu = (HMENU)(INT_PTR)nID;
    create.lpszClass = TEXT("BUTTON");
    create.style = BS_PUSHBUTTON | BS_FLAT;
    return Control::Create(create);
}

//-----------------------------------------------------------------------------
// Name: CreateBitmap
// Description: Creates a simple bitmap button
//
// hParent: Parent window
// nImgID:  Resource ID of the bitmap
// nID:     Command ID.
// rcBound: Bounding rectangle.
//-----------------------------------------------------------------------------

HRESULT Button::CreateBitmap(HWND hParent, int nImgID, int nID, const Rect& rcSize)
{
    HRESULT hr = CreateText(hParent, NULL, nID, rcSize);
    if (SUCCEEDED(hr))
    {
        SetImage((WORD)nImgID);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Name: SetImage
// Description: Set a bitmap for the button
//
// nImgID:  Resource ID of the bitmap
//-----------------------------------------------------------------------------

BOOL Button::SetImage(WORD nImgId)
{
    AddStyle(BS_BITMAP);
    HBITMAP hBitmap = SetBitmapImg(GetInstance(), nImgId, m_hwnd);
    return (hBitmap ? TRUE : FALSE);
}

//////////////////////////////////////////////////////////////////////////
// Control.cpp: Base control class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "wincontrol.h"

//-----------------------------------------------------------------------------
// Name: Create
// Desc: Create a new instance of the control.
//
// create: Creation parameters.
//-----------------------------------------------------------------------------

HRESULT Control::Create(const CREATESTRUCT& create)
{
    if (m_hwnd != NULL)
    {
        // The control was already created.
        return E_FAIL;
    }

    if (create.hwndParent == NULL)
    {
        return E_INVALIDARG;
    }

    HINSTANCE hinst = create.hInstance;

    if (hinst == NULL)
    {
        hinst = GetInstance();
    }

    if (hinst == NULL)
    {
        return E_INVALIDARG;
    }


    HWND hwnd = CreateWindowEx(
        create.dwExStyle, create.lpszClass, create.lpszName, 
        create.style | WS_CHILD | WS_VISIBLE,
        create.x, create.y, create.cx, create.cy, create.hwndParent, create.hMenu,
        hinst, create.lpCreateParams);

    if (hwnd== 0)
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    SetWindow(hwnd);
    return S_OK;
};


CreateStruct::CreateStruct()
{
    ZeroMemory(this, sizeof(*this));
}

void CreateStruct::SetBoundingRect(const Rect& rc)
{
    x = rc.left;
    y = rc.top;
    cx = rc.right - x;
    cy = rc.bottom - y;
}




HINSTANCE GetInstance()
{
	return (HINSTANCE)GetModuleHandle(NULL); 
}



// SetBitmapImg - Set a bitmap image on a window
HBITMAP SetBitmapImg(HINSTANCE hinst, WORD nImgId, HWND hwnd)
{
    HBITMAP hBitmap = LoadBitmap(hinst, MAKEINTRESOURCE(nImgId));
    if (hBitmap)
    {
        SendMessage(hwnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
        // BM_SETIMAGE returns the handle to the _previous_ bitmap.
        return hBitmap;
    }
    return 0;
}

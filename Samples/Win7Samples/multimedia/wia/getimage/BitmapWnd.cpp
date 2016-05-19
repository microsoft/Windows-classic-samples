/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#include "stdafx.h"
#include "resource.h"

#include "BitmapUtil.h"
#include "BitmapWnd.h"

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::CBitmapWnd
//

CBitmapWnd::CBitmapWnd(IStream *pStream) : m_Image(pStream)
{
    m_cRef = 0;
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::CBitmapWnd
//

CBitmapWnd::CBitmapWnd(PCWSTR pszFileName) : m_Image(pszFileName)
{
    m_cRef = 0;
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::QueryInterface
//

STDMETHODIMP CBitmapWnd::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == NULL)
    {
	    return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
	    *ppvObj = (IUnknown *) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

	AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::AddRef
//

STDMETHODIMP_(ULONG) CBitmapWnd::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::Release
//

STDMETHODIMP_(ULONG) CBitmapWnd::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::Register
//

ATOM CBitmapWnd::Register()
{
    WNDCLASSEX wcex = { 0 };

    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowProc;
    wcex.hInstance      = g_hInstance;
    wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName  = _T("BitmapWindow");

    return RegisterClassEx(&wcex);
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::WindowProc
//

LRESULT 
CALLBACK
CBitmapWnd::WindowProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;

            LPMDICREATESTRUCT pmdics = (LPMDICREATESTRUCT) pcs->lpCreateParams;

            CBitmapWnd *that = (CBitmapWnd *) pmdics->lParam;

            that->AddRef();

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) that);

            break;
        }

        case WM_DESTROY:
        {
            CBitmapWnd *that = (CBitmapWnd *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

            that->Release();

            break;
        }

        case WM_PAINT:
        {
            CBitmapWnd *that = (CBitmapWnd *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

            that->OnPaint(hWnd);

            break;
        }
    }

    return DefMDIChildProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd::OnPaint
//

LRESULT CBitmapWnd::OnPaint(HWND hWnd)
{
    // Get the destination DC

    PAINTSTRUCT ps;

    HDC hDC = BeginPaint(hWnd, &ps);

    if (hDC != NULL)
    {
        Gdiplus::Graphics graphics(hDC);

        // Get the size of the window

        RECT r;

        GetClientRect(hWnd, &r);

        UINT nWindowWidth = r.right;
        UINT nWindowHeight = r.bottom;

        // Get the size of the image

        UINT nBitmapWidth = m_Image.GetWidth();
        UINT nBitmapHeight = m_Image.GetHeight();

        if (nBitmapWidth != 0 && nBitmapHeight != 0)
        {
            // Calculate the coordinates and the size of the image

            Gdiplus::Rect rDest;

            if (nBitmapWidth <= nWindowWidth && nBitmapHeight <= nWindowHeight)
            {
                // If the image is smaller than the window, center the image

                rDest.X      = (nWindowWidth - nBitmapWidth) / 2;
                rDest.Y      = (nWindowHeight - nBitmapHeight) / 2;
                rDest.Width  = nBitmapWidth;
                rDest.Height = nBitmapHeight;
            }
            else
            {
                // If the image is larger than the window, resize and center 
                // the image while keeping the aspect ratio

                UINT nStretchedWidth  = nWindowWidth;
                UINT nStretchedHeight = MulDiv(nBitmapHeight, nWindowWidth, nBitmapWidth);

			    if (nStretchedHeight > nWindowHeight)
			    {
				    nStretchedWidth  = MulDiv(nBitmapWidth, nWindowHeight, nBitmapHeight);
				    nStretchedHeight = nWindowHeight;
			    }

                rDest.X      = (nWindowWidth - nStretchedWidth) / 2;
                rDest.Y      = (nWindowHeight - nStretchedHeight) / 2;
                rDest.Width  = nStretchedWidth;
                rDest.Height = nStretchedHeight;
            }

            // Paint the image with a white background

            graphics.DrawImage(&m_Image, rDest);

            graphics.ExcludeClip(rDest);
        }

        graphics.Clear((DWORD)Gdiplus::Color::White);
        
        EndPaint(hWnd, &ps);
    }

    return 0;
}



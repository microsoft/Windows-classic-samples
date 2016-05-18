
/************************************************************************
 *
 * File: GdiInterop.cpp
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#include "GdiInterop.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// DirectWrite global variables.
IDWriteFactory* g_pDWriteFactory = NULL;
IDWriteTextFormat* g_pTextFormat = NULL;
IDWriteGdiInterop* g_pGdiInterop = NULL;
IDWriteTextLayout* g_pTextLayout = NULL;
IDWriteBitmapRenderTarget* g_pBitmapRenderTarget = NULL;
IDWriteRenderingParams* g_pRenderingParams = NULL;

// For our custom renderer class.
IDWriteTextRenderer* g_pGdiTextRenderer = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void GdiOnPaint(HDC hdc, wchar_t *text, HFONT hfont);
HRESULT DWriteOnPaint(HDC hdc, wchar_t *text, HFONT hfont);
HRESULT DWriteCreateResources(HDC hdc, wchar_t *text, HFONT hfont);

/******************************************************************
*                                                                 *
*  OnPaint                                                        *
*                                                                 *
*  Creates a handle to a LOGFONT and a string and then calls the  *
*  GDI and DirectWrite OnPaint functions to display actual text.  *
*                                                                 *
******************************************************************/

void OnPaint(HDC hdc)
{
    HRESULT hr = S_OK;

    HFONT hfont = NULL;
    long height = 0;
    HDC memoryHdc = NULL;
    SIZE size = {};
    
    // Set the height, equivalent to 50 em for DirectWrite.
    height = -MulDiv(50, GetDeviceCaps(hdc, LOGPIXELSY), 96);

    hfont = CreateFontW(height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");

    wchar_t dwriteText[ ] = L"Sample text using DirectWrite.";  
    wchar_t gdiText[ ] = L"Sample text using GDI TextOut."; 

    // The DirectWrite objects need only be created once and can be reused each time the window paints
    hr = DWriteCreateResources(hdc, dwriteText, hfont);

    // Get the memory device context, the drawing is done offscreen.
    if (SUCCEEDED(hr))
    {
        memoryHdc = g_pBitmapRenderTarget->GetMemoryDC();
        SetBoundsRect(memoryHdc, NULL, DCB_ENABLE|DCB_RESET);
    }

    // Get the size of the target.
    if (SUCCEEDED(hr))
    {
        hr = g_pBitmapRenderTarget->GetSize(&size);
    }

    // Clear the background
    if (SUCCEEDED(hr))
    {
        SetDCBrushColor(memoryHdc, 0xFFFFFF);
        SelectObject(memoryHdc, GetStockObject(NULL_PEN));
        SelectObject(memoryHdc, GetStockObject(DC_BRUSH));
        Rectangle(memoryHdc, 0, 0, size.cx + 1, size.cy + 1);
    }

    // Draw the string to the memory HDC using GDI.
    if (SUCCEEDED(hr))
    {
        GdiOnPaint(memoryHdc, gdiText, hfont);
    }

    // Draw the same string below the first to the memory HDC using DirectWrite.
    if (SUCCEEDED(hr))
    {
        hr = DWriteOnPaint(memoryHdc, dwriteText, hfont);
    }

    // Transfer from DWrite's rendering target to the window.
    BitBlt(
        hdc,
        0, 0,
        size.cx + 1, size.cy + 1,
        memoryHdc,
        0, 0, 
        SRCCOPY
        );

    // If the DirectWrite drawing failed, exit and display an error.
    if (FAILED(hr))
    {
        wchar_t error[255];

        swprintf_s(error, 254, L"HRESULT = %x", hr);

        MessageBox(0, error, L"Error, exiting.", 0);

        exit(1);
    }
}

/******************************************************************
*                                                                 *
*  GdiOnPaint                                                     *
*                                                                 *
*  Draw the text using GDI and the TextOut function.              *
*                                                                 *
*                                                                 *
******************************************************************/

void GdiOnPaint(HDC hdc, wchar_t *text, HFONT hfont)
{
    HFONT hf;
    UINT32 textLength;

    textLength = UINT32(wcslen(text));    
    
    hf = (HFONT) SelectObject(hdc, hfont);

    SetTextColor(hdc, RGB(0, 200, 255));

    TextOut(hdc, 0, 0, text, textLength);

    SelectObject(hdc, hf);
}

/******************************************************************
*                                                                 *
*  DWriteOnPaint                                                  *
*                                                                 *
*  Clears the memory DC of the bitmap render target, renders to   *
*  it using a custom text renderer, and then copies it to the     *
*  window device context using the BitBlt function.               *
*                                                                 *
******************************************************************/

HRESULT DWriteOnPaint(HDC hdc, wchar_t *text, HFONT hfont)
{
    HRESULT hr = S_OK;
    
    // Draw the text below the GDI text
    if (SUCCEEDED(hr))
    {
        hr = g_pTextLayout->Draw(NULL, g_pGdiTextRenderer, 0, 65);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DWriteCreateResources                                          *
*                                                                 *
*  Creates the global DirectWrite objects needed for rendering,   *
*  including the IDWriteGdiInterop and IDWriteBitmapRenderTarget  *
*  interfaces.                                                    *
*                                                                 *
*  It also converts the GDI LOGFONT structure to a IDWriteFont    *
*  object and creates a IDWriteTextLayout object using this       *
*  information.                                                   *
*                                                                 *
******************************************************************/

HRESULT DWriteCreateResources(HDC hdc, wchar_t *text, HFONT hfont)
{
    HRESULT hr = S_OK;

    // If the DirectWrite factory doesn't exist, create the resources,
    // only create these resources once.
    if (!g_pDWriteFactory)
    {
        HWND hwnd;
        RECT r;

        // DirectWrite variables.
        IDWriteFontFamily* pFontFamily = NULL;
        IDWriteFont* pFont = NULL;
        IDWriteLocalizedStrings* pFamilyNames = NULL;
        
        // Logical (GDI) font.
        LOGFONT lf = {};

        UINT32 length = 0;
        UINT32 index = 0;
        float fontSize = 0;

        // length of the string
        UINT32 textLength = 0;

        wchar_t *name = NULL;

        // Get a handle to the DC and the window rect.
        hwnd = WindowFromDC(hdc);
        GetClientRect(hwnd, &r);

        // Calculate the string length.
        textLength = UINT32(wcslen(text));

        // Create the DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
            );

        // Create a GDI interop interface.
        if (SUCCEEDED(hr))
        {
            hr = g_pDWriteFactory->GetGdiInterop(&g_pGdiInterop);
        }
        
        if (SUCCEEDED(hr))
        {
            // Get a logical font from the font handle.
            GetObject(hfont, sizeof(LOGFONT), &lf);
        }

        // Convert to a DirectWrite font.
        if (SUCCEEDED(hr))
        {
            hr = g_pGdiInterop->CreateFontFromLOGFONT(&lf, &pFont);
        }
        
        // Get the font family.
        if (SUCCEEDED(hr))
        {
            hr = pFont->GetFontFamily(&pFontFamily);
        }

        // Get a list of localized family names.
        if (SUCCEEDED(hr))
        {
            hr = pFontFamily->GetFamilyNames(&pFamilyNames);
        }

        // Select the first locale.  This is OK, because we are not displaying the family name.
        index = 0;
        
        // Get the length of the family name.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetStringLength(index, &length);
        }

        if (SUCCEEDED(hr))
        {
            // Allocate a new string.
            name = new (std::nothrow) wchar_t[length+1];
		    if (name == NULL)
            {
			    hr = E_OUTOFMEMORY;
            }
        }

        // Get the actual family name.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetString(index, name, length+1);
        }

        if (SUCCEEDED(hr))
        {
            // Calculate the font size.
            fontSize = (float) -MulDiv(lf.lfHeight, 96, GetDeviceCaps(hdc, LOGPIXELSY));
        }

        // Create a text format using the converted font information.
        if (SUCCEEDED(hr))
        {
            hr = g_pDWriteFactory->CreateTextFormat(
                name,                // Font family name.
                NULL,                        
                pFont->GetWeight(),
                pFont->GetStyle(),
                pFont->GetStretch(),
                fontSize,
                L"en-us",
                &g_pTextFormat
                );
        }

        // Create a text layout.
        if (SUCCEEDED(hr))
        {
            hr = g_pDWriteFactory->CreateTextLayout(
                text,
                textLength,
                g_pTextFormat,
                1024.0f,
                480.0f,
                &g_pTextLayout
                );
        }

        // Underline and strikethrough are part of a LOGFONT structure, but are not
        // part of a DWrite font object so we must set them using the text layout.
        if(lf.lfUnderline)
        {
            DWRITE_TEXT_RANGE textRange = {0, textLength};
            g_pTextLayout->SetUnderline(true, textRange);
        }

        if(lf.lfStrikeOut)
        {
            DWRITE_TEXT_RANGE textRange = {0, textLength};
            g_pTextLayout->SetStrikethrough(true, textRange);
        }
        
        // Create a bitmap render target for our custom renderer.
        if (SUCCEEDED(hr))
        {
            hr = g_pGdiInterop->CreateBitmapRenderTarget(hdc, r.right, r.bottom, &g_pBitmapRenderTarget);
        }
        
        // Create default rendering params for our custom renderer.
        if (SUCCEEDED(hr))
        {
            hr = g_pDWriteFactory->CreateRenderingParams(&g_pRenderingParams);
        }

        if (SUCCEEDED(hr))
        {
            // Initialize the custom renderer class.
		    g_pGdiTextRenderer = new (std::nothrow) GdiTextRenderer(g_pBitmapRenderTarget, g_pRenderingParams);
        }

        // Clean up local interfaces.
        SafeRelease(&pFontFamily);
        SafeRelease(&pFont);
        SafeRelease(&pFamilyNames);
    }

    return hr;
}


int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_GDIINTEROP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GDIINTEROP));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = NULL;
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDC_GDIINTEROP);
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND    - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY    - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnPaint(hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_SIZE:
        if (g_pBitmapRenderTarget)
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            g_pBitmapRenderTarget->Resize(width, height);
        }
        break;
    case WM_DESTROY:
        // The window is closing, release the DirectWrite variables we created for drawing.
        SafeRelease(&g_pDWriteFactory);
        SafeRelease(&g_pTextFormat);
        SafeRelease(&g_pGdiInterop);
        SafeRelease(&g_pTextLayout);
        SafeRelease(&g_pBitmapRenderTarget);
        SafeRelease(&g_pRenderingParams);
        SafeRelease(&g_pGdiTextRenderer);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


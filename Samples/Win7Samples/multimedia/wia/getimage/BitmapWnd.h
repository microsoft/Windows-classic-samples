/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __BITMAPWND__
#define __BITMAPWND__

//////////////////////////////////////////////////////////////////////////
//
// CBitmapWnd
//

/*++

    CBitmapWnd is an MDI child window that displays a bitmap using GDI+. 
    It is descended from IUnknown for reference counting.

Methods

    Register
        Registers the window class

    WindowProc
        Processes window messages.

    OnPaint
        Centers and displays the image using GDI+ functions.

--*/

class CBitmapWnd : public IUnknown
{
public:
    CBitmapWnd(IStream *pStream);
    CBitmapWnd(PCWSTR pszFileName);

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // CBitmapWnd Methods

    static ATOM Register();

private:
    static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

    LRESULT OnPaint(HWND hWnd);

private:
    LONG           m_cRef;
    HGLOBAL        m_hBitmap;
    Gdiplus::Image m_Image;
};

//////////////////////////////////////////////////////////////////////////
//
// CGdiplusInit
//

/*++

    CGdiplusInit is a wrapper class that automatically initializes and 
    shuts down the GDI+ library. GDI+ initialization is necessary for 
    any program that uses GDI+ functions.

Methods

    CGdiplusInit
        Initializes the GDI+ library.

    ~CGdiplusInit
        Shuts down the GDI+ library.

--*/

class CGdiplusInit : public Gdiplus::GdiplusStartupOutput
{
public:
    CGdiplusInit(
        Gdiplus::DebugEventProc debugEventCallback       = 0,
        BOOL                    suppressBackgroundThread = FALSE,
        BOOL                    suppressExternalCodecs   = FALSE
    )
    {
        Gdiplus::GdiplusStartupInput StartupInput(
            debugEventCallback,
            suppressBackgroundThread,
            suppressExternalCodecs
        );

        StartupStatus = Gdiplus::GdiplusStartup(
            &Token, 
            &StartupInput, 
            this
        );
    }

    ~CGdiplusInit()
    {
        if (StartupStatus == Gdiplus::Ok)
        {
            Gdiplus::GdiplusShutdown(Token);
        }
    }

private:
    Gdiplus::Status StartupStatus;
    ULONG_PTR       Token;
};

#endif //__BITMAPWND__


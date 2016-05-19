//------------------------------------------------------------------------------
// File: VideoTxt.cpp
//
// Desc: DirectShow sample code - implements DirectShow video renderer.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// This is a sample renderer that displays video in a window. What is special
// about that is that the window is non rectangular - the region it has is a
// word (whatever VideoString is defined to be as shown below). Using various
// GDI methods we create a region from the word and select it into our window,
// Windows will then handle clipping and mouse interactions, so all we have
// to do is handle the painting and drawing. This sample can work if it uses
// either its own allocator (the preferred approach) or a source filter's allocator
// (like when it connects to the tee filter). It has separate code paths to
// handle the drawing, depending whose allocator it is using on this connection.

#include "sampvid.h"

#include <tchar.h>
#include <strsafe.h>

// Global data
TCHAR VideoString[] = TEXT("ActiveX\0");


//
// Constructor
//
CVideoText::CVideoText(TCHAR *pName,                 // Object description
                       LPUNKNOWN pUnk,               // Normal COM ownership
                       HRESULT *phr,                 // OLE failure code
                       CCritSec *pInterfaceLock,     // Main critical section
                       CVideoRenderer *pRenderer) :  // Delegates locking to

    CBaseControlVideo(pRenderer,pInterfaceLock,pName,pUnk,phr),
    CBaseControlWindow(pRenderer,pInterfaceLock,pName,pUnk,phr),
    m_pRenderer(pRenderer)
{
    PrepareWindow();
    InitWindowRegion(VideoString);

} // (Constructor)


//
// Destructor
//
CVideoText::~CVideoText()
{
    InactivateWindow();
    DoneWithWindow();

} // (Destructor)


//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP CVideoText::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IVideoWindow) 
    {
        return CBaseVideoWindow::NonDelegatingQueryInterface(riid,ppv);
    } 
    else 
    {
        ASSERT(riid == IID_IBasicVideo);
        return CBaseBasicVideo::NonDelegatingQueryInterface(riid,ppv);
    }
}


//
// CreateVideoFont
//
// This is called when we initialise the mask bitmap used when drawing. We
// create and install a larger font into the device context. The font used
// is 72 point Arial. It doesn't matter that the width of the letters are
// bigger than the video as we can stretch the video during the draw. This
// makes us go slower but it looks good and the quality management in the
// base renderer class should take care of reducing the source frame rate
//
HFONT CVideoText::CreateVideoFont()
{
    LOGFONT lf;

    int iLogPelsY = GetDeviceCaps(m_hdc,LOGPIXELSY);
    ZeroMemory(&lf,sizeof(LOGFONT));

    lf.lfHeight         = (-72 * iLogPelsY) / 72;
    lf.lfWeight         = 700;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = PROOF_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    StringCchCopy(lf.lfFaceName, NUMELMS(lf.lfFaceName), TEXT("Arial\0"));

    return CreateFontIndirect(&lf);

} // CreateVideoFont


//
// InitWindowRegion
//
// We display the video in a window that has a region selected into it that
// matches the word we are passed in. By doing this we let Windows manage
// all the clipping and mouse technology. The trick is in creating a region
// that matches the word, which is done by using paths. We create a path for
// a temporary HDC, draw the word and then end the path. After that, we can then
// ask Windows for a region that describes that path. That gives us a region
// for the outside of the word, so we bitwise "not" it to get the word region.
//
HRESULT CVideoText::InitWindowRegion(TCHAR *pStringName)
{
    ASSERT(pStringName);

    OSVERSIONINFO VersionInfo;
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    EXECUTE_ASSERT(GetVersionEx(&VersionInfo));

    // Set a window region according to the OS capabilities

    if ((VersionInfo.dwPlatformId & VER_PLATFORM_WIN32_NT) == 0) 
    {
        m_Size.cx = 320;
        m_Size.cy = 240;
        return NOERROR;
    }

    // Get the text extents the word passed in will require based on the
    // font and bitmap selected in the current device context. For it to
    // be displayed in a different font it must be selected into the HDC

    HDC hdc = CreateCompatibleDC(m_hdc);
    HFONT hFont = CreateVideoFont();
    SelectObject(hdc,hFont);

    GetTextExtentPoint32((HDC) hdc,             // The output device context
                         pStringName,           // The string we'll be using
                         lstrlen(pStringName),  // Number of characters in it
                         (LPSIZE) &m_Size);     // Filled in with the extents

    // Create a bitmap that matches the current format

    HBITMAP hMaskBitmap = CreateCompatibleBitmap(hdc,m_Size.cx,m_Size.cy);
    if (hMaskBitmap == NULL) {
        ASSERT(hMaskBitmap);
        return E_UNEXPECTED;
    }

    // Select the monochrome bitmap into the device context

    HBITMAP hBitmap = (HBITMAP) SelectObject(hdc,hMaskBitmap);
    EXECUTE_ASSERT(BeginPath(hdc));

    // Draw the string into the monochrome bitmap

    ExtTextOut((HDC) hdc,               // Target device context
               (int) 0,                 // x coordinate reference
               (int) 0,                 // Likewise y coordinate
               (DWORD) 0,               // No special flags to set
               NULL,                    // No clipping rectangle
               pStringName,             // Pointer to text words
               lstrlen(pStringName),    // Number of characters
               NULL);                   // Intercharacter spacing

    EXECUTE_ASSERT(EndPath(hdc));

    HRGN hOutside = PathToRegion(hdc);
    HRGN hFullWindow = CreateRectRgn(0,0,m_Size.cx,m_Size.cy);
    HRGN hWordRegion = CreateRectRgn(0,0,1,1);
    CombineRgn(hWordRegion,hFullWindow,hOutside,RGN_DIFF);
    SetWindowRgn(m_hwnd,hWordRegion,TRUE);

    // Clear up the regions we created

    DeleteObject(hWordRegion);
    DeleteObject(hOutside);
    DeleteObject(hFullWindow);

    // Delete the HDC and text bitmap

    SelectObject(hdc,hBitmap);
    HFONT hDefault = (HFONT) GetStockObject(SYSTEM_FONT);
    SelectObject(hdc,hDefault);

    DeleteObject(hFont);
    DeleteObject(hMaskBitmap);
    DeleteDC(hdc);

    return NOERROR;

} // InitWindowRegion


//
// GetClassWindowStyles
//
// When we call PrepareWindow in our constructor it will call this method as
// it is going to create the window to get our window and class styles. The
// return code is the class name and must be allocated in static storage. We
// specify a normal window during creation although the window styles as well
// as the extended styles may be changed by the application via IVideoWindow
//
LPTSTR CVideoText::GetClassWindowStyles(DWORD *pClassStyles,
                                        DWORD *pWindowStyles,
                                        DWORD *pWindowStylesEx)
{
    CheckPointer(pClassStyles,NULL);
    CheckPointer(pWindowStyles,NULL);
    CheckPointer(pWindowStylesEx,NULL);

    OSVERSIONINFO VersionInfo;
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    EXECUTE_ASSERT(GetVersionEx(&VersionInfo));

    // Default styles for Windows NT systems

    *pClassStyles    = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
    *pWindowStyles   = WS_POPUP | WS_CLIPCHILDREN;
    *pWindowStylesEx = (DWORD) 0;

    // Make a normal window on Win95 systems

    if ((VersionInfo.dwPlatformId & VER_PLATFORM_WIN32_NT) == 0)
        *pWindowStyles = WS_POPUP | WS_CAPTION | WS_CLIPCHILDREN;

    return TEXT("VideoTextRenderer\0");

} // GetClassWindowStyles


//
// GetDefaultRect
//
// Return the default window rectangle
//
RECT CVideoText::GetDefaultRect()
{
    RECT DefaultRect = {0,0,m_Size.cx,m_Size.cy};
    ASSERT(m_hwnd);
    ASSERT(m_hdc);

    return DefaultRect;

} // GetDefaultRect


//
// OnReceiveMessage
//
// This is the derived class window message handler methods
//
LRESULT CVideoText::OnReceiveMessage(HWND hwnd,          // Window handle
                                     UINT uMsg,          // Message ID
                                     WPARAM wParam,      // First parameter
                                     LPARAM lParam)      // Other parameter
{
    IBaseFilter *pFilter = NULL;
    RECT ClientRect;

    // Blank out the window background

    if (uMsg == WM_ERASEBKGND) 
    {
        EXECUTE_ASSERT(GetClientRect(m_hwnd,&ClientRect));
        HBRUSH hBrush = CreateSolidBrush(RGB(0,0,0));

        EXECUTE_ASSERT(FillRect(m_hdc,&ClientRect,hBrush));
        EXECUTE_ASSERT(DeleteObject(hBrush));
        return (LRESULT) 0;
    }

    // Handle WM_CLOSE by aborting the playback

    if (uMsg == WM_CLOSE) 
    {
        m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
        DoShowWindow(SW_HIDE);

        return CBaseWindow::OnClose();
    }

    // We pass on WM_ACTIVATEAPP messages to the filtergraph so that the
    // IVideoWindow plug in distributor can switch us out of fullscreen
    // mode where appropriate. These messages may also be used by the
    // resource manager to keep track of which renderer has the focus

    if (uMsg == WM_ACTIVATEAPP) 
    {
        NOTE1("Notification of EC_ACTIVATE (%d)",(BOOL) wParam);

        m_pRenderer->QueryInterface(IID_IBaseFilter,(void **) &pFilter);
        if (pFilter)
        {
            m_pRenderer->NotifyEvent(EC_ACTIVATE, wParam, (LPARAM) pFilter);
            pFilter->Release();
        }

        return (LRESULT) 0;
    }

    // Treat clicks on text as requests to move window

    if (uMsg == WM_NCHITTEST) 
    {
        LRESULT Result = DefWindowProc(hwnd,uMsg,wParam,lParam);
        if (Result == HTCLIENT)
            Result = HTCAPTION;

        return Result;
    }

    // The base class that implements IVideoWindow looks after a flag
    // that says whether or not the cursor should be hidden. If so we
    // hide the cursor and return (LRESULT) 1. Otherwise we pass to
    // the DefWindowProc to show the cursor as normal. This is used
    // when our window is made fullscreen to imitate the Modex filter

    if (uMsg == WM_SETCURSOR) 
    {
        if (IsCursorHidden() == TRUE) 
        {
            SetCursor(NULL);
            return (LRESULT) 1;
        }
    }

    // When we detect a display change we send an EC_DISPLAY_CHANGED
    // message along with our input pin. The filtergraph will stop
    // everyone and reconnect our input pin. When being reconnected
    // we can then accept the media type that matches the new display
    // mode since we may no longer be able to draw the current format

    if (uMsg == WM_DISPLAYCHANGE) 
    {
        m_pRenderer->m_Display.RefreshDisplayType(NULL);
        m_pRenderer->OnDisplayChange();
        NOTE("Sent EC_DISPLAY_CHANGED event");

        return (LRESULT) 0;
    }

    return CBaseWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


//
// SetDefaultTargetRect
//
// This is called when we reset the default target rectangle
//
HRESULT CVideoText::SetDefaultTargetRect()
{
    // VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pRenderer->m_mtIn.Format();
    // BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

    RECT TargetRect = {0,0,m_Size.cx,m_Size.cy};
    m_pRenderer->m_DrawImage.SetTargetRect(&TargetRect);

    return NOERROR;

} // SetDefaultTargetRect


//
// IsDefaultTargetRect
//
// Return S_OK if using the default target otherwise S_FALSE
//
HRESULT CVideoText::IsDefaultTargetRect()
{
    RECT TargetRect;

    // VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pRenderer->m_mtIn.Format();
    // BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

    m_pRenderer->m_DrawImage.GetTargetRect(&TargetRect);

    // Check the destination matches the initial client area

    if (TargetRect.left != 0  || 
        TargetRect.top  != 0  ||
        TargetRect.right  != m_Size.cx ||
        TargetRect.bottom != m_Size.cy) 
    {
        return S_FALSE;
    }

    return S_OK;

} // IsDefaultTargetRect


//
// SetTargetRect
//
// This is called to set the target rectangle in the video window, it will be
// called whenever a WM_SIZE message is retrieved from the message queue. We
// simply store the rectangle and use it later when we do the drawing calls
//
HRESULT CVideoText::SetTargetRect(RECT *pTargetRect)
{
    m_pRenderer->m_DrawImage.SetTargetRect(pTargetRect);
    return NOERROR;

} // SetTargetRect


//
// GetTargetRect
//
// Return the current destination rectangle
//
HRESULT CVideoText::GetTargetRect(RECT *pTargetRect)
{
    ASSERT(pTargetRect);

    m_pRenderer->m_DrawImage.GetTargetRect(pTargetRect);
    return NOERROR;

} // GetTargetRect


//
// SetDefaultSourceRect
//
// This is called when we reset the default source rectangle
//
HRESULT CVideoText::SetDefaultSourceRect()
{
    SIZE VideoSize = m_pRenderer->m_VideoSize;
    RECT SourceRect = {0,0,VideoSize.cx,VideoSize.cy};

    m_pRenderer->m_DrawImage.SetSourceRect(&SourceRect);
    return NOERROR;

} // SetDefaultSourceRect


//
// IsDefaultSourceRect
//
// Return S_OK if using the default source otherwise S_FALSE
//
HRESULT CVideoText::IsDefaultSourceRect()
{
    RECT SourceRect;

    // Does the source match the native video size

    SIZE VideoSize = m_pRenderer->m_VideoSize;
    CAutoLock cWindowLock(&m_WindowLock);
    m_pRenderer->m_DrawImage.GetSourceRect(&SourceRect);

    // Check the coordinates match the video dimensions

    if (SourceRect.right == VideoSize.cx) 
    {
        if (SourceRect.bottom == VideoSize.cy) 
        {
            if (SourceRect.left == 0) 
            {
                if (SourceRect.top == 0) 
                {
                    return S_OK;
                }
            }
        }
    }

    return S_FALSE;

} // IsDefaultSourceRect


//
// SetSourceRect
//
// This is called when we want to change the section of the image to draw. We
// use this information in the drawing operation calls later on. We must also
// see if the source and destination rectangles have the same dimensions. If
// not we must stretch during the rendering rather than a direct pixel copy
//
HRESULT CVideoText::SetSourceRect(RECT *pSourceRect)
{
    m_pRenderer->m_DrawImage.SetSourceRect(pSourceRect);
    return NOERROR;

} // SetSourceRect


//
// GetSourceRect
//
// Return the current source rectangle
//
HRESULT CVideoText::GetSourceRect(RECT *pSourceRect)
{
    ASSERT(pSourceRect);

    m_pRenderer->m_DrawImage.GetSourceRect(pSourceRect);
    return NOERROR;

} // GetSourceRect


//
// GetStaticImage
//
// Return a copy of the current image in the video renderer
//
HRESULT CVideoText::GetStaticImage(long *pBufferSize,long *pDIBImage)
{
    NOTE("Entering GetStaticImage");

    IMediaSample *pMediaSample;
    pMediaSample = m_pRenderer->GetCurrentSample();
    RECT SourceRect;

    // Is there an image available

    if (pMediaSample == NULL)
        return E_UNEXPECTED;

    // Find a scaled source rectangle for the current bitmap

    m_pRenderer->m_DrawImage.GetSourceRect(&SourceRect);
    SourceRect = m_pRenderer->m_DrawImage.ScaleSourceRect(&SourceRect);
    VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pRenderer->m_mtIn.Format();

    // Call the base class helper method to do the work

    HRESULT hr = CopyImage(pMediaSample,        // Buffer containing image
                           pVideoInfo,          // Type representing bitmap
                           pBufferSize,         // Size of buffer for DIB
                           (BYTE*) pDIBImage,   // Data buffer for output
                           &SourceRect);        // Current source position

    pMediaSample->Release();
    return hr;

} // GetStaticImage


//
// GetVideoFormat
//
// Derived classes must override this to return a VIDEOINFOHEADER representing
// the video format. We cannot call IPin ConnectionMediaType to get this
// format because various filters dynamically change the type when using
// DirectDraw such that the format shows the position of the logical
// bitmap in a frame buffer surface, so the size might be returned as
// 1024x768 pixels instead of 320x240 which are the real video dimensions
//
VIDEOINFOHEADER *CVideoText::GetVideoFormat()
{
    return (VIDEOINFOHEADER *)m_pRenderer->m_mtIn.Format();

} // GetVideoFormat


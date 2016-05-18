//------------------------------------------------------------------------------
// File: DvdCore.cpp
//
// Desc: Actual implementation of DVD Playback capabilities
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#define _WIN32_WINNT 0x0500

// Set warning level 3 to avoid numerous /W4 compiler warnings in
// the <sstream> library, which is beyond our control.
#pragma warning(push, 3)
#include <sstream> // STL String library
using namespace std;
#pragma warning(pop)

#include "resource.h"
#include "DvdCore.h"
#include "StringUtil.h"
#include <strsafe.h>

#pragma warning(disable:4710)   // C4710: function not inlined (optimization)


// Error message displayed if no DVD decoder is installed
#define MSG_NO_DECODER  \
        TEXT("The DVD sample could not locate a DVD decoder on your system.\r\n\r\n")     \
        TEXT("You must install a third-party DirectShow-compatible MPEG-2 decoder,\r\n")  \
        TEXT("either software or hardware-based. Microsoft does not supply an MPEG2\r\n") \
        TEXT("decoder with DirectShow or as an operating system component.\r\n\r\n")      \
        TEXT("Please contact your DVD or PC manufacturer for a DVD decoder.\r\n\r\n")     \
        TEXT("This sample will now exit.\0")



//------------------------------------------------------------------------------
// File-specific data
//------------------------------------------------------------------------------

namespace // empty namespace means these are only available within this .cpp file
{
    const WCHAR g_szBookmarkFilename[] = L"c:\\DVDSample.bmk\0";
    const WCHAR g_szBookmarkName[] = L"DVDSampleBookmarks\0";
    HHOOK g_hMouse = 0;
    const UINT MOUSETIMER = 1; // the number is arbitrary, it just can't be 0.
    const DWORD MOUSE_TIMEOUT = 5000; // the mouse should disappear after 5 seconds
}


//------------------------------------------------------------------------------
// Name: CDvdCore::CDvdCore()
// Desc: This is our constructor.  It initializes all of our variables and initializes
//       the COM subsystem.
//------------------------------------------------------------------------------

CDvdCore::CDvdCore(HINSTANCE hInstance, IDvdCallback * pIDC) :
                       m_dwRenderFlags(AM_DVD_HWDEC_PREFER),
                       m_eState(Uninitialized),
                       m_pIDvdGB(NULL),
                       m_pIDvdC2(NULL),
                       m_pIDvdI2(NULL),
                       m_pGraph(NULL),
                       m_pIVW(NULL),
                       m_pIME(NULL),
                       m_pIMC(NULL),
                       m_hInstance(hInstance),
                       m_pIL21Dec(NULL),
                       m_hWnd(NULL),
                       m_dwMouseMoveTime(0),
                       m_bFirstPlay(true),
                       m_bFullScreenOn(false),
                       m_ulCurTitle(0),
                       m_ulCurChapter(0),
                       m_bMessageSink(false),
                       m_bUseVMR9(false)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::CDvdCore()"))) ;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    ZeroMemory(m_szDiscPath, sizeof(m_szDiscPath)) ;
    ZeroMemory(&m_CurTime, sizeof(m_CurTime));

    // if we are given a valid pointer we will use it, otherwise we point to ourself.
    if (NULL != pIDC)
        m_pCallback = pIDC;
    else
        m_pCallback = this;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::~CDvdCore()
// Desc: This is our destructor.  It release all of our COM interfaces and closes
//       down the COM subsystem.
//------------------------------------------------------------------------------

CDvdCore::~CDvdCore()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::~CDvdCore()"))) ;

    if (m_bMessageSink) // we set up the message sink and so we need to tear it down
        UnInitMessageSink();

    ReleaseInterfaces();
    CoUninitialize();
}


//------------------------------------------------------------------------------
// Name: CDvdCore::BuildGraph()
// Desc: This method builds the DVD graph and initializes all of the interface pointers
//       we will need.
//------------------------------------------------------------------------------

bool CDvdCore::BuildGraph()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::BuildGraph()"))) ;

    // First release any existing interface pointer(s)
    ReleaseInterfaces() ;
    SetState(Uninitialized) ;

    HRESULT hr;
    hr = CoCreateInstance(  CLSID_DvdGraphBuilder,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IDvdGraphBuilder,
                            reinterpret_cast<void**>(&m_pIDvdGB));
    if (FAILED(hr)) // IDvdGraphBuilder not successfully instantiated
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("ERROR: CoCreateInstance() for CLSID_DvdGraphBuilder failed (Error 0x%x)"),
            hr)) ;
        return false;
    }

    //  the user may have indicated VMR9 to be used in the graph
    if (m_bUseVMR9)
    {
        //  to use VMR9, we verify the ability of a VMR9 filter to be inserted into the graph
	    IVMRFilterConfig*	pConfig = 0;
	    hr = m_pIDvdGB->GetDvdInterface(IID_IVMRFilterConfig9, (void**)&pConfig);
	    if (FAILED(hr))
        {
            m_bUseVMR9 = false;
		    if (IDCANCEL == MessageBox(0,
                                       TEXT("Cannot use VMR9 in the DVD graph.\nSelect 'OK' to proceed with the default option, 'Cancel' to close the application."),
                                       TEXT("Non-critical error"), MB_ICONEXCLAMATION | MB_OKCANCEL))
            {
                OnClose();      //  indicate we wish to exit the application
                return false;
            }
        }
	    else
	    {
		    pConfig->Release();
		    pConfig = 0;
            //  the graph allows the use of VMR9; set the rendering flags accordingly
		    m_dwRenderFlags = AM_DVD_SWDEC_PREFER | AM_DVD_VMR9_ONLY;
	    }
    }   //  use VMR9

    hr = m_pIDvdGB->GetFiltergraph(&m_pGraph);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            TEXT("Cannot get the IGraphBuilder interface.")
            TEXT("Sorry, can't play DVD-Video titles."),
            TEXT("DVD Error!"), MB_OK) ;

        return false ;
    }

    // It is important to QI for IMediaEventEx *before* building the graph so that
    // the app can catch all of the DVD Navigator's initialization events.  Once
    // an app QI's for IMediaEventEx, DirectShow will start queuing up events and
    // the app will receive them when it set the notify window (in InitMessageSink
    // for this sample).  If the app does not QI for IMediaEventEx before building
    // the graph, these events will just be lost.
    hr = m_pGraph->QueryInterface(IID_IMediaEventEx, reinterpret_cast<void**>(&m_pIME)) ;
    if (FAILED(hr))
    {
        MessageBox(NULL,
            TEXT("Cannot get the MediaEventEx interface.")
            TEXT("Sorry, can't play DVD-Video titles."),
            TEXT("DVD Error!"), MB_OK) ;

        return false ;
    }

    // Check if a DVD-Video volume name has been specified; if so, use that
    WCHAR    szwDiscPath[MAX_PATH] ;
    PCWSTR   pszwDiscPath = NULL ;  // by default
    if (lstrlen(m_szDiscPath) > 0)  // if something was specified before
    {
#ifdef UNICODE
        StringCchCopy(szwDiscPath, MAX_PATH, m_szDiscPath) ;
#else
        MultiByteToWideChar(CP_ACP, 0, m_szDiscPath, -1, szwDiscPath, NUMELMS(szwDiscPath)-1) ;
#endif // UNICODE

        szwDiscPath[MAX_PATH-1] = TEXT('\0');   // NULL-terminate
        pszwDiscPath = szwDiscPath ;
    }

    // Build the Graph
    AM_DVD_RENDERSTATUS buildStatus;
    DbgLog((LOG_TRACE, 5, TEXT("Calling RenderDvdVideoVolume(<%s>, 0x%lx, 0x%lx)"),
        m_szDiscPath, m_dwRenderFlags, &buildStatus)) ;

    hr = m_pIDvdGB->RenderDvdVideoVolume(pszwDiscPath, m_dwRenderFlags, &buildStatus);

    if (FAILED(hr)) // total failure
    {
        // If there is no DVD decoder, give a user-friendly message
        if (hr == 0x8004027b)
        {
            MessageBox(NULL, MSG_NO_DECODER, TEXT("Can't initialize DVD sample"), MB_OK);
        }
        else
        {
            TCHAR szBuffer[100];
            AMGetErrorText(hr, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
            MessageBox(NULL, szBuffer, TEXT("RenderDvdVideoVolume Failed!"), MB_OK);
        }
        DbgLog((LOG_ERROR, 0,
            TEXT("ERROR: RenderDvdVideoVolume() completely failed (Error 0x%x - %s)"),
            hr, szBuffer)) ;
        return false;
    }

    if (S_FALSE == hr) // partial success
    {
        TCHAR szStatusText[1000] ;
        if (0 == GetStatusText(&buildStatus, szStatusText,
                               sizeof(szStatusText) / sizeof(szStatusText[0])))
        {
            hr = StringCchCopy(szStatusText, NUMELMS(szStatusText), TEXT("An unknown error has occurred!\0")) ;
        }

        hr = StringCchCat(szStatusText, NUMELMS(szStatusText),TEXT("\n\nDo you still want to continue?\0")) ;
        if (IDNO == MessageBox(NULL, szStatusText, TEXT("Warning"), MB_YESNO))
        {
            return false ;
        }
    }

    // The graph was successfully rendered in some form if we get this far
    // We will now instantiate all of the necessary interfaces

    hr = m_pIDvdGB->GetDvdInterface(IID_IDvdInfo2, reinterpret_cast<void**>(&m_pIDvdI2));
    if (FAILED(hr))
    {
        IDvdInfo  *pDvdI ;
        hr = m_pIDvdGB->GetDvdInterface(IID_IDvdInfo, reinterpret_cast<void**>(&pDvdI)) ;
        if (SUCCEEDED(hr))
        {
            MessageBox(NULL,
                TEXT("This version of the application cannot run using\n")
                TEXT("the current DirectShow components. Please use the\n")
                TEXT("DVDSampl.exe application."),
                TEXT("DVD Error!"), MB_OK) ;
            pDvdI->Release() ;
        }
        else
        {
            MessageBox(NULL,
                TEXT("Cannot get the IDvdInfo/IDvdInfo2 interface.")
                TEXT("Sorry, can't play DVD-Video titles."),
                TEXT("DVD Error!"), MB_OK) ;
        }

        return false ;
    }

    hr = m_pIDvdGB->GetDvdInterface(IID_IDvdControl2, reinterpret_cast<void**>(&m_pIDvdC2));
    if (FAILED(hr))
    {
        IDvdControl  *pDvdC ;
        hr = m_pIDvdGB->GetDvdInterface(IID_IDvdControl, reinterpret_cast<void**>(&pDvdC)) ;
        if (SUCCEEDED(hr))
        {
            MessageBox(NULL,
                TEXT("This version of the application cannot run using\n")
                TEXT("the current DirectShow components. Please use the\n")
                TEXT("DVDSampl.exe application."),
                TEXT("DVD Error!"), MB_OK) ;
            pDvdC->Release() ;
        }
        else
        {
            MessageBox(NULL,
                TEXT("Cannot get the IDvdControl/IDvdControl2 interface.")
                TEXT("Sorry, can't play DVD-Video titles."),
                TEXT("DVD Error!"), MB_OK) ;
        }

        return false ;
    }

    hr = m_pGraph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&m_pIMC));
    if (FAILED(hr))
    {
        MessageBox(NULL,
            TEXT("Cannot get the IMediaControl interface.")
            TEXT("Sorry, can't play DVD-Video titles."),
            TEXT("DVD Error!"), MB_OK) ;

        return false ;
    }

    hr = m_pIDvdGB->GetDvdInterface(IID_IVideoWindow, reinterpret_cast<void**>(&m_pIVW));
    if (FAILED(hr))
    {
        MessageBox(NULL,
            TEXT("Cannot get the IVideoWindow interface.")
            TEXT("Sorry, can't play DVD-Video titles."),
            TEXT("DVD Error!"), MB_OK) ;

        return false ;
    }

    // this one may or may not be present
    m_pIDvdGB->GetDvdInterface(IID_IAMLine21Decoder, reinterpret_cast<void**>(&m_pIL21Dec));


    SetState(Graph_Stopped2);
    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::ReleaseInterfaces()
// Desc: This method releases all of our COM interfaces
//       This will be called before building a graph and during destruction of the class
//------------------------------------------------------------------------------

void CDvdCore::ReleaseInterfaces()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::ReleaseInterfaces()"))) ;

    if (NULL != m_pGraph)
    {
        m_pGraph->Release();
        m_pGraph = NULL;
    }

    if (NULL != m_pIDvdC2)
    {
        m_pIDvdC2->Release();
        m_pIDvdC2 = NULL;
    }

    if (NULL != m_pIDvdGB)
    {
        m_pIDvdGB->Release();
        m_pIDvdGB = NULL;
    }

    if (NULL != m_pIDvdI2)
    {
        m_pIDvdI2->Release();
        m_pIDvdI2 = NULL;
    }

    if (NULL != m_pIL21Dec)
    {
        m_pIL21Dec->Release();
        m_pIL21Dec = NULL;
    }

    if (NULL != m_pIMC)
    {
        m_pIMC->Release();
        m_pIMC = NULL;
    }

    if (NULL != m_pIME)
    {
        m_pIME->SetNotifyWindow(NULL, WM_DVD_EVENT, 0); // we don't want messages any more
        m_pIME->Release();
        m_pIME = NULL;
    }

    if (NULL != m_pIVW)
    {
        m_pIVW->put_Owner(NULL); // make sure we send messages back to the right place
        m_pIVW->Release();
        m_pIVW = NULL;
    }

    SetState(Uninitialized);
}


//------------------------------------------------------------------------------
// Name: CDvdCore::Init()
// Desc: This method initializes the DVD Playback Core.  It provides a means to
//       divide the work among several functions.
//------------------------------------------------------------------------------

bool CDvdCore::Init()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::Init()"))) ;

    if (false == BuildGraph())
    {
        ReleaseInterfaces();
        return false;
    }
    if (false == InitMessageSink())
        return false;

    return SetPlaybackOptions();
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetStatusText()
// Desc: This method parses AM_DVD_RENDERSTATUS and returns a text description of the error
//------------------------------------------------------------------------------

DWORD CDvdCore::GetStatusText(AM_DVD_RENDERSTATUS *pStatus, PTSTR pszStatusText,
                              DWORD dwMaxText)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::GetStatusText()"))) ;

    TCHAR szBuffer[1000] ;
    size_t BuffSize = NUMELMS(szBuffer);

    int iChars ;
    HRESULT hr;
    PTSTR pszBuff = szBuffer ;
    ZeroMemory(szBuffer, sizeof(TCHAR) * BuffSize) ;

    if (pStatus->iNumStreamsFailed > 0)
    {
        hr = StringCchPrintf(pszBuff, BuffSize,
            TEXT("* %d out of %d DVD-Video streams failed to render properly\n\0"),
            pStatus->iNumStreamsFailed, pStatus->iNumStreams) ;
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(pszBuff);
        pszBuff += iChars ;
        BuffSize -= iChars;


        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_VIDEO)
        {
            hr = StringCchPrintf(pszBuff, BuffSize, TEXT("    - video stream\n\0")) ;
            if (S_OK != hr)
            {
                return 0;
            }
            iChars = lstrlen(pszBuff);
            pszBuff += iChars ;
            BuffSize -= iChars;
        }
        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_AUDIO)
        {
            hr = StringCchPrintf(pszBuff, BuffSize, TEXT("    - audio stream\n\0")) ;
            if (S_OK != hr)
            {
                return 0;
            }
            iChars = lstrlen(pszBuff);
            pszBuff += iChars ;
            BuffSize -= iChars;
        }
        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_SUBPIC)
        {
            hr = StringCchPrintf(pszBuff, BuffSize, TEXT("    - subpicture stream\n\0")) ;
            if (S_OK != hr)
            {
                return 0;
            }
            iChars = lstrlen(pszBuff);
            pszBuff += iChars ;
            BuffSize -= iChars;
        }
    }

    if (FAILED(pStatus->hrVPEStatus))
    {
        hr = StringCchCat(pszBuff, BuffSize, TEXT("* ")) ;
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(TEXT("* ")) ;
        pszBuff += iChars;
        BuffSize -= iChars;

        iChars = AMGetErrorText(pStatus->hrVPEStatus, pszBuff, DWORD(BuffSize) ) ;
        pszBuff += iChars ;
        BuffSize -= iChars;

        hr = StringCchCat(pszBuff, BuffSize, TEXT("\n\0")) ;
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(TEXT("\n\0")) ;
        pszBuff += iChars;
        BuffSize -= iChars;
    }

    if (pStatus->bDvdVolInvalid)
    {
        hr = StringCchPrintf(pszBuff, BuffSize, TEXT("* Specified DVD-Video volume was invalid\n\0"));
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(pszBuff);
        pszBuff += iChars ;
        BuffSize -= iChars;
    }
    else if (pStatus->bDvdVolUnknown)
    {
        hr  = StringCchPrintf(pszBuff, BuffSize,TEXT("* No valid DVD-Video volume could be located\n\0"));
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(pszBuff);
        pszBuff += iChars ;
        BuffSize -= iChars;
    }

    if (pStatus->bNoLine21In)
    {
        hr = StringCchPrintf(pszBuff, BuffSize, TEXT("* The video decoder doesn't produce closed captioning data\n\0"));
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(pszBuff);
        pszBuff += iChars ;
        BuffSize -= iChars;
    }
    if (pStatus->bNoLine21Out)
    {
        hr = StringCchPrintf(pszBuff, BuffSize, TEXT("* Decoded closed caption data not rendered properly\n\0"));
        if (S_OK != hr)
        {
            return 0;
        }
        iChars = lstrlen(pszBuff);
        pszBuff += iChars ;
        BuffSize -= iChars;
    }

    hr = StringCchCopy(pszStatusText, dwMaxText, szBuffer) ;
    DWORD dwLenght = 0;
    if (S_OK == hr)
    {
        dwLenght = lstrlen(pszStatusText);
    }

    return dwLenght ;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::Play()
// Desc: This method begins DVD Playback
//------------------------------------------------------------------------------

HRESULT CDvdCore::Play()
{
    HRESULT hr=S_OK;

    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::Play()"))) ;

    if (true == m_bFirstPlay)
    {
        // now show the playback window
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
        UpdateWindow(m_hWnd) ;
        m_bFirstPlay = false;
    }

    switch (m_eState)
    {
        case Uninitialized:
            return E_FAIL;

        case Graph_Stopped2:
            // This call will likely fail due to timing issues.  Ignore it.
            m_pIDvdC2->PlayForwards(1.0, 0, NULL);

            hr = m_pIMC->Run();
            if (SUCCEEDED(hr))
                SetState(Playing);
            return hr;

        case Nav_Stopped:
        case Graph_Stopped1:
            hr = m_pIMC->Run();
            if (SUCCEEDED(hr))
                SetState(Playing);
            return hr;

        case Nav_Paused:
        case Graph_Paused:
            m_pIDvdC2->PlayForwards(1.0, 0, NULL);

            hr = m_pIMC->Run();
            if (SUCCEEDED(hr))
                SetState(Playing);
            return hr;

        case Playing:
            if (true == m_bStillOn) // we are at a still without buttons
            {
                hr = m_pIDvdC2->StillOff();
                if (SUCCEEDED(hr))
                    m_bStillOn = false;
            }
            return hr;

        case Scanning:
            hr = m_pIDvdC2->PlayForwards(1.0, 0, NULL);
            if (SUCCEEDED(hr))
                SetState(Playing);
            return hr;
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::InitMessageSink()
// Desc: This method creates a parent window and initializes sinks for mouse
//       messages and DVD events.
//       We create a new window instead of just using the IVideoWindow window is
//       that we lose the ability to resize the Video Window once we set up a
//       message drain.  The message drain is necessary to handle mouse events for
//       the menus but causes the automatic resizing and movement of the original
//       window to stop working.
//------------------------------------------------------------------------------

bool CDvdCore::InitMessageSink()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::InitMessageSink()"))) ;

    // register playback window
    WNDCLASSEX wndclass = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        CDvdCore::WndProc,
        0,
        0,
        m_hInstance,
        NULL,
        LoadCursor(NULL, IDC_ARROW),
        HBRUSH(COLOR_WINDOW+1),
        NULL,
        TEXT("EventWindow"),
        NULL
    };

    RegisterClassEx(&wndclass);

    // create the playback window

    RECT rPlayback = GetPlaybackWindowRect();

    m_hWnd = CreateWindow(
        TEXT("EventWindow"),
        TEXT("Video Window"),
        WS_OVERLAPPEDWINDOW,
        rPlayback.left,
        rPlayback.top,
        rPlayback.right - rPlayback.left,
        rPlayback.bottom - rPlayback.top,
        NULL,
        NULL,
        m_hInstance,
        this); // we send the this pointer here so we can retrieve it from WM_CREATE later

    // store the pointer to this instance of CDvdCore.
    // this is so we can refer to it in the OnDvdEvent message loop
    // For win64 compatibily, use SetWindowLongPtr...

    _SetWindowLongPtr(m_hWnd, GWLP_USERDATA, this);

    // set up the event notification so that the dummy window gets
    // informed about DVD events about during playback.
    HRESULT hr;
    hr = m_pIME->SetNotifyWindow(reinterpret_cast<OAHWND>(m_hWnd), WM_DVD_EVENT, 0);
    ASSERT(SUCCEEDED(hr));

    // Make the video window a child of the playback window.  We do this so we can trap
    // mouse events and still be able to manipulate the window.
    long lStyle;
    m_pIVW->get_WindowStyle(&lStyle);
    m_pIVW->put_WindowStyle( (lStyle | WS_CHILD) & ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME) );
    m_pIVW->put_Owner(reinterpret_cast<OAHWND>(m_hWnd));
    RECT rect;
    GetClientRect(m_hWnd, &rect);
    m_pIVW->SetWindowPosition(0, 0, rect.right, rect.bottom);

    // This will cause our playback window to get mouse and keyboard events from the
    // Video Window
    hr = m_pIVW->put_MessageDrain(reinterpret_cast<OAHWND>(m_hWnd)) ;
    ASSERT(SUCCEEDED(hr)) ;

    m_bMessageSink = true; // flag so we will turn this off later

    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::WndProc()
// Desc: This method is a message loop designed to handle mouse, key, and Dvd event messages
//       Because there is no 'this' pointer in a static method, we do the actual work
//       in CDvdCore non-static methods.
//------------------------------------------------------------------------------

LRESULT CALLBACK CDvdCore::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    // retrieve the pointer to the instance of CDvdCore that this window belongs to
    // because OnDvdEvent is a static method, it doesn't have a this pointer
    CDvdCore * pCore = NULL;

    // For Win64 compatibility, use GetWindowLongPtr (Platform SDK)

    pCore = _GetWindowLongPtr<CDvdCore*>(hWnd, GWLP_USERDATA);

    if (NULL == pCore)
    // this is really only necessary if you want to react to WM_CREATE messages
    {
        if (WM_CREATE == uMessage)
            // WM_CREATE messages return the last parameter to CreateWindow()
        {
            CREATESTRUCT * pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
            pCore = reinterpret_cast<CDvdCore *>(pCreate->lpCreateParams);
        }
        else return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }


    switch (uMessage)
    {
        case WM_TIMER:
            return pCore->OnMouseTimer(wParam, lParam);

        case WM_SIZE:
            return pCore->OnSize(wParam, lParam);

        case WM_KEYUP:
            return pCore->OnKeyEvent(wParam, lParam);

        case WM_DVD_EVENT:
            return pCore->OnDvdEvent(uMessage, wParam, lParam);

        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            return pCore->OnMouseEvent(uMessage, wParam, lParam);

        case WM_CLOSE:
            return pCore->OnClose();
    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}


//------------------------------------------------------------------------------
// Name: CDvdCore::SetPlaybackOptions()
// Desc: This method sets our default line21, ResetOnStop, Parental controls, etc.
//------------------------------------------------------------------------------

bool CDvdCore::SetPlaybackOptions()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::SetPlaybackOptions()"))) ;

    HRESULT hr;

    if (m_pIL21Dec)
    {
        hr = m_pIL21Dec->SetServiceState(AM_L21_CCSTATE_Off) ; // Line21 off by default
        if (FAILED(hr))
            return false;
    }
    else
    {
        DbgLog((LOG_TRACE, 2, TEXT("Line21 Decoder interface not found. Can't CC"))) ;
    }

    hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, FALSE); // don't reset on stop
    if (FAILED(hr))
        return false;

    hr = m_pIDvdC2->SetOption(DVD_NotifyParentalLevelChange, TRUE); // notify us when parental level changes
    if (FAILED(hr))
        return false;

    hr = m_pIDvdC2->SetOption(DVD_HMSF_TimeCodeEvents, TRUE); // use new HMSF timecode format
    if (FAILED(hr))
        return false;

    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::NextChapter()
// Desc: This method is a wrapper for the PlayNextChapter method.
//       It is not necessary to use the IDvdCmd->WaitForEnd() method here.  It is
//       done merely to demonstrate how to use this blocking method.
//------------------------------------------------------------------------------

bool CDvdCore::NextChapter()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::NextChapter()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (Playing == m_eState)
    {
        HRESULT hr;
        IDvdCmd * cmdObj;

        hr = m_pIDvdC2->PlayNextChapter(DVD_CMD_FLAG_None, &cmdObj);
        if (FAILED(hr))
            return false;

        cmdObj->WaitForEnd();
        cmdObj->Release();
        if (SUCCEEDED(hr))
            return true;
    }

    return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::PrevChapter()
// Desc: This method is a wrapper for the PlayPrevChapter method.
//       It is not necessary to use the DVD_CMD_FLAG_Block flag.  It is done
//       merely to demonstrate how to use this blocking method.
//------------------------------------------------------------------------------

bool CDvdCore::PrevChapter()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::PrevChapter()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (Playing == m_eState)
    {
        HRESULT hr;
        hr = m_pIDvdC2->PlayPrevChapter(DVD_CMD_FLAG_Block, NULL);
        if (SUCCEEDED(hr))
            return true;
    }

    return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::Stop()
// Desc: This method merely stops playback.  We use IMediaControl->Stop rather than
//       IDvdControl2->Stop because it it easier to recover from.
//------------------------------------------------------------------------------

bool CDvdCore::Stop()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::Stop()"))) ;
    HRESULT hr;

    switch (m_eState)
    {
        case Uninitialized:
            return false;

        case Graph_Stopped2:
            hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, TRUE);  // do a complete reset
            hr = m_pIMC->Stop();                               // just to be sure
            hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, FALSE); // restore previous settings
            break;

        case Playing:
        case Scanning:
        case Nav_Paused:
        case Graph_Paused:
            hr = m_pIMC->Pause();
            SetState(Graph_Stopped1);
            break;

        case Nav_Stopped:
        case Graph_Stopped1:
            hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, TRUE);  // do a complete reset
            hr = m_pIMC->Stop();                               // stop the entire graph
            hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, FALSE); // restore previous settings
            SetState(Graph_Stopped2);
            break;
    }

    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::Rewind()
// Desc: This method begins fast forwarding the disc.  It only tries if the disc
//       is playing or FF/RW'ing already.
//------------------------------------------------------------------------------

bool CDvdCore::Rewind()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::Rewind()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (Playing == m_eState || Scanning == m_eState)
    {
        if (FAILED(m_pIDvdC2->PlayBackwards(8.0, 0, NULL)))
            return false;

        SetState(Scanning);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Name: CDvdCore::FastForward()
// Desc: This method begins rewinding the disc.  It only tries if the disc
//       is playing or FF/RW'ing already.
//------------------------------------------------------------------------------

bool CDvdCore::FastForward()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::FastForward()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (Playing == m_eState || Scanning == m_eState)
    {
        if (FAILED(m_pIDvdC2->PlayForwards(8.0, 0, NULL)))
            return false;

        SetState(Scanning);
        return true;
    }

    return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::Pause()
// Desc: This method pauses if we aren't already paused and unpauses if we are.
//------------------------------------------------------------------------------

bool CDvdCore::Pause(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::Pause()"))) ;

    if (Uninitialized == m_eState)
        return false;

    // if we are paused at the graph level, call run on the graph
    if (Graph_Paused == m_eState)
    {
        if (FAILED(m_pIMC->Run()))
            return false;
        else
        {
            SetState(Playing);
            return true;
        }
    }
    // if we are paused at the Navigator level, call Pause(false)
    if (Nav_Paused == m_eState)
    {
        if (FAILED(m_pIMC->Run()))
            return false;
        else
        {
            SetState(Playing);
            return true;
        }
    }
    // Anything else, we try to pause at the Nav Level
    // this will fail if we are stopped.
    if (FAILED(m_pIMC->Pause()))
        return false;
    else
    {
        SetState(Nav_Paused);
        return true;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::RootMenu()
// Desc: This method will either bring us into the root menu or call resume if
//       we are already in a menu.  The one caveat is that if we are in the title menu
//       we will resume instead of going to the root menu.
//------------------------------------------------------------------------------

bool CDvdCore::RootMenu()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::RootMenu()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (true == m_bMenuOn)
    {
        // We use flush here merely to show how it might be used.
        // You can call Resume() without it.
        if (FAILED(m_pIDvdC2->Resume(DVD_CMD_FLAG_Flush, NULL)))
            return false;
        else
        {
            m_bMenuOn = false;
            SetState(Playing);
            return true;
        }
    }
    else if (Playing == m_eState || Scanning == m_eState)
    {
        IDvdCmd * pObj;

        // It is a good idea to block on Menu commands.  This is just one way to do so.
        if (FAILED(m_pIDvdC2->ShowMenu(DVD_MENU_Root, DVD_CMD_FLAG_Flush, &pObj)))
        {
            return false;
        }
        else
        {
            pObj->WaitForEnd();
            pObj->Release();
            m_bMenuOn = true;
            SetState(Playing);
            return true;
        }
    }

    return false; // we can't go to a menu from a stop or paused state
}


//------------------------------------------------------------------------------
// Name: CDvdCore::EnableFullScreen()
// Desc: This method will enter and exit fullscreen mode depending on the bool
//       value.  We don't use IVideoWindow->putFullScreenMode because
//       it won't handle the mouse correctly.
//------------------------------------------------------------------------------

bool CDvdCore::ToggleFullScreen()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::ToggleFullScreen()"))) ;

    if (false == m_bFullScreenOn)
        return StartFullScreen();
    else
        return StopFullScreen();
}


//------------------------------------------------------------------------------
// Name: CDvdCore::TitleMenu()
// Desc: This method will either bring us into the root menu or call resume if
//       we are already in a menu.  The one caveat is that if we are in the root menu
//       we will resume instead of going to the title menu.
//------------------------------------------------------------------------------

bool CDvdCore::TitleMenu()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::TitleMenu()"))) ;

    if (Uninitialized == m_eState)
        return false;

    if (true == m_bMenuOn)
    {
        // we use flush here merely to show how it might be used.  You can call Resume without it.
        if (FAILED(m_pIDvdC2->Resume(DVD_CMD_FLAG_Flush, NULL)))
            return false;
        else
        {
            SetState(Playing);
            m_bMenuOn = false;
            return true;
        }
    }
    else if (Playing == m_eState || Scanning == m_eState)
    {
        IDvdCmd * pObj;

        // It is a good idea to block on Menu commands.  This is just one way to do so.
        HRESULT hr = m_pIDvdC2->ShowMenu(DVD_MENU_Title, DVD_CMD_FLAG_Flush, &pObj);
        if (FAILED(hr))
        {
            if (VFW_E_DVD_OPERATION_INHIBITED == hr)
                m_pCallback->Prohibited();
            DbgLog((LOG_ERROR, 1, TEXT("CDvdCore::TitleMenu() failed: %#x"), hr)) ;
            return false;
        }
        else
        {
            pObj->WaitForEnd();
            pObj->Release();
            SetState(Playing);
            m_bMenuOn = true;
            return true;
        }
    }

    return false; // we can't go to a menu from a stop or paused state
}


//------------------------------------------------------------------------------
// Name: CDvdCore::EnableCaptions()
// Desc: This method turns on or off closed captions (Line 21)
//------------------------------------------------------------------------------

bool CDvdCore::EnableCaptions(bool bOn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::EnableCaptions(%s)"), bOn ? TEXT("True") :
        TEXT("False"))) ;

    if (NULL == m_pIL21Dec) // if this decoder doesn't support Line 21
        return false;

    if (SUCCEEDED(m_pIL21Dec->SetServiceState(bOn ? AM_L21_CCSTATE_On : AM_L21_CCSTATE_Off)))
        return true;
    else
        return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::SetVideoWindowTitle()
// Desc: This method sets the Title of the IVideoWindow window
//------------------------------------------------------------------------------

bool CDvdCore::SetVideoWindowTitle(TCHAR *pszTitle)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::SetVideoWindowTitle()"))) ;

    if (TRUE == SetWindowText(m_hWnd, pszTitle))
        return true;
    else
        return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::SaveBookmark()
// Desc: This method saves a bookmark on the DVD to the hard drive.
//------------------------------------------------------------------------------

bool CDvdCore::SaveBookmark()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::SaveBookmark()"))) ;

    // we begin by getting a copy of the DVD Navigator's internal state
    HRESULT hr;
    IDvdState * pBookmark;

    hr = m_pIDvdI2->GetState (&pBookmark);
    if (FAILED(hr))
        return false;

    // Next we serialize (persist) the data to the disc.
    // This always saves to the same bookmark file.  It could just as easily
    // save to a different one each time.
    //
    // We could also serialize the data to memory using IPersistMemory if we
    // don't want to persist across different executions of the program.
    IStorage * pStorage;
    IStream * pStream;
    bool bCreateStream = false;

    // first try to open an existing file
    hr = StgOpenStorage(g_szBookmarkFilename, NULL, STGM_WRITE | STGM_SHARE_EXCLUSIVE,
        NULL, 0, &pStorage);
    if (SUCCEEDED(hr))
    {
        // we then open a stream object within that file
        hr = pStorage->OpenStream(g_szBookmarkName, NULL, STGM_WRITE |
            STGM_SHARE_EXCLUSIVE, 0, &pStream);
        if (FAILED(hr))
        {
            // Create a new stream so that old bookmarks can be overwritten
            hr = pStorage->CreateStream(g_szBookmarkName,
                    STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
                    0, 0, &pStream);
            if (FAILED(hr))
            {
                pBookmark->Release();
                pStorage->Release();
                return false;
            }
        }
    }
    else // if that fails, try to create a new file
    {
        hr = StgCreateDocfile(g_szBookmarkFilename, STGM_CREATE | STGM_WRITE |
            STGM_SHARE_EXCLUSIVE, 0, &pStorage);
        if (FAILED(hr))
        {
            pBookmark->Release();
            return false;
        }
        // we then create a stream object within that file
        hr = pStorage->CreateStream(g_szBookmarkName, STGM_CREATE | STGM_WRITE |
            STGM_SHARE_EXCLUSIVE, 0, 0, &pStream);
        if (FAILED(hr))
        {
            pBookmark->Release();
            pStorage->Release();
            return false;
        }
    }

    // now we tell the bookmark to persist itself into the stream we just created
    IPersistStream * pPersistStream;
    hr = pBookmark->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersistStream));
    if (SUCCEEDED(hr))
    {
        hr = OleSaveToStream(pPersistStream, pStream);
        pPersistStream->Release();
        pStorage->Release();
        pStream->Release();
        pBookmark->Release();

        if (SUCCEEDED(hr))
            return true;
        else
            return false;
    }
    else
    {
        pBookmark->Release();
        pStorage->Release();
        pStream->Release();
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::RestoreBookmark()
// Desc: This method reads a bookmark file from the disc and restores playback
//       at that location.
//------------------------------------------------------------------------------

bool CDvdCore::RestoreBookmark()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::RestoreBookmark()"))) ;

    // Begin by opening the bookmark file on the drive.
    IStorage * pStorage;
    HRESULT hr = StgOpenStorage(g_szBookmarkFilename, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE,
        NULL, 0, &pStorage);
    if (FAILED(hr))
        return false;

    // open a stream object within that file
    IStream * pStream;
    hr = pStorage->OpenStream(g_szBookmarkName, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0,
        &pStream);
    if (FAILED(hr))
    {
        pStorage->Release();
        return false;
    }

    // load the bookmark in from the stream
    IDvdState * pBookmark;
    hr = OleLoadFromStream(pStream, IID_IDvdState, reinterpret_cast<void**>(&pBookmark));

    pStream->Release();
    pStorage->Release();
    if (FAILED(hr))
        return false;

    // restore bookmark's state in the Navigator
    hr = m_pIDvdC2->SetState(pBookmark, DVD_CMD_FLAG_Block, NULL);
    pBookmark->Release();

    if (SUCCEEDED(hr))
        return true;
    else
        return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::StartFullScreen()
// Desc: This method begins fullscreen playback.  It saves the original window
//       information, then stretches the window full screen.  We use this rather
//       than IVideoWindow::put_FullScreen() because we have greater control over
//       the mouse this way and can show and hide it at will.
//------------------------------------------------------------------------------

bool CDvdCore::StartFullScreen()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::StartFullScreen()"))) ;

    if (NULL == m_pIVW)
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("CDvdCore::StartFullScreen() -- no IVideoWindow pointer yet"))) ;
        return false ;
    }

    HRESULT   hr ;

    // store the original window position
    LONG  lLeft, lTop, lWidth, lHeight ;
    hr = m_pIVW->GetWindowPosition(&lLeft, &lTop, &lWidth, &lHeight) ;
    ASSERT(SUCCEEDED(hr)) ;
    SetRect(&m_RectOrigVideo, lLeft, lTop, lLeft + lWidth, lTop + lHeight) ;

    // save the original window style
    hr = m_pIVW->get_WindowStyle(&m_lOrigStyle) ;
    ASSERT(SUCCEEDED(hr)) ;
    hr = m_pIVW->get_WindowStyleEx(&m_lOrigStyleEx) ;
    ASSERT(SUCCEEDED(hr)) ;

    // the window can't be a child while it is full screen or it can't be stretched beyond the
    // parent window's borders
    hr = m_pIVW->put_Owner(NULL);
    ASSERT(SUCCEEDED(hr)) ;

    // modify the window's style
    hr = m_pIVW->put_WindowStyle(m_lOrigStyle &
        ~(WS_BORDER | WS_CAPTION | WS_THICKFRAME)) ;  // remove these styles
    ASSERT(SUCCEEDED(hr)) ;
    hr = m_pIVW->put_WindowStyleEx(m_lOrigStyleEx &
        ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE |
        WS_EX_DLGMODALFRAME) |  // remove these extended styles
        WS_EX_TOPMOST) ;
    ASSERT(SUCCEEDED(hr)) ;

    // stretch the window to the full size of the screen
    LONG  lScrnWidth  = GetSystemMetrics(SM_CXSCREEN) ;
    LONG  lScrnHeight = GetSystemMetrics(SM_CYSCREEN) ;
    hr = m_pIVW->SetWindowPosition(0, 0, lScrnWidth, lScrnHeight) ;
    ASSERT(SUCCEEDED(hr)) ;

    // start a timer for mouse blanking
    m_dwMouseMoveTime = timeGetTime();
    SetTimer(m_hWnd, MOUSETIMER, 2500, NULL);

    m_bFullScreenOn = true;

    return true ;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::StopFullScreen()
// Desc: This method restores the Video Window to its original size and style.
//------------------------------------------------------------------------------

bool CDvdCore::StopFullScreen()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::StopFullScreen()"))) ;
    HRESULT hr ;

    if (NULL == m_pIVW)
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("CDvdCore::StopFullScreen() -- no IVideoWindow pointer yet"))) ;
        return false ;
    }

    // restore the window's position
    RECT rect;
    GetClientRect(m_hWnd, &rect);
    hr = m_pIVW->SetWindowPosition(0, 0, rect.right, rect.bottom);
    ASSERT(SUCCEEDED(hr)) ;

    // make it a child window again
    hr = m_pIVW->put_Owner(reinterpret_cast<OAHWND>(m_hWnd)) ;
    ASSERT(SUCCEEDED(hr)) ;

    // restore the window's styles
    hr = m_pIVW->put_WindowStyle(m_lOrigStyle) ;      // restore the styles
    ASSERT(SUCCEEDED(hr)) ;
    hr = m_pIVW->put_WindowStyleEx(m_lOrigStyleEx) ;  // restore the extended styles
    ASSERT(SUCCEEDED(hr)) ;

    KillTimer(m_hWnd, MOUSETIMER); // remove the mouse blanking timer
    ShowMouseCursor(true) ;        // make sure to show mouse now

    m_bFullScreenOn = false;

    return true ;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnDvdEvent()
// Desc: This method receives DVD messages from our WndProc and acts on them
//------------------------------------------------------------------------------
LRESULT CDvdCore::OnDvdEvent(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::OnDvdEvent()"))) ;

    long lEvent;
	LONG_PTR lParam1, lParam2;
    long lTimeOut = 0;

    while (SUCCEEDED(m_pIME->GetEvent(&lEvent, &lParam1, &lParam2, lTimeOut)))
    {
        DbgLog((LOG_TRACE, 5, TEXT("Event: %#x"), lEvent)) ;

        HRESULT hr;
        switch(lEvent)
        {
            case EC_DVD_CURRENT_HMSF_TIME:
            {
                DVD_HMSF_TIMECODE * pTC = reinterpret_cast<DVD_HMSF_TIMECODE *>(&lParam1);
                m_CurTime = *pTC;
                m_pCallback->UpdateStatus(); // inform our client that something changed
            }
            break;

            case EC_DVD_CHAPTER_START:
                m_ulCurChapter = static_cast<ULONG>(lParam1);
                break;

            case EC_DVD_TITLE_CHANGE:
                m_ulCurTitle = static_cast<ULONG>(lParam1);
                break;

            case EC_DVD_NO_FP_PGC:
                hr = m_pIDvdC2->PlayTitle(1, DVD_CMD_FLAG_None, NULL);
                ASSERT(SUCCEEDED(hr));
                break;

            case EC_DVD_DOMAIN_CHANGE:
                switch (lParam1)
                {
                    case DVD_DOMAIN_FirstPlay:  // = 1
                    case DVD_DOMAIN_Stop:       // = 5
                        break ;

                    case DVD_DOMAIN_VideoManagerMenu:  // = 2
                    case DVD_DOMAIN_VideoTitleSetMenu: // = 3
                        // Inform the app to update the menu option to show "Resume" now
                        DbgLog((LOG_TRACE, 3, TEXT("DVD Event: Domain changed to VMGM(2)/VTSM(3) (%ld)"),
							static_cast<ULONG>(lParam1))) ;
                        m_bMenuOn = true;  // now menu is "On"
                        break ;

                    case DVD_DOMAIN_Title:      // = 4
                        // Inform the app to update the menu option to show "Menu" again
                        DbgLog((LOG_TRACE, 3, TEXT("DVD Event: Title Domain (%ld)"),
							static_cast<ULONG>(lParam1))) ;
                        m_bMenuOn = false ; // we are no longer in a menu
                        break ;
                } // end of domain change switch
                break;


            case EC_DVD_PARENTAL_LEVEL_CHANGE:
                {
                    TCHAR szMsg[1000];
                    hr = StringCchPrintf( szMsg, NUMELMS(szMsg),TEXT("Accept Parental Level Change Request to level %ld?"),
						static_cast<ULONG>(lParam1) );
                    if (IDYES == MessageBox(m_hWnd, szMsg,  TEXT("Accept Change?"), MB_YESNO))
                    {
                        m_pIDvdC2->AcceptParentalLevelChange(TRUE);
                    }
                    else
                    {
                        m_pIDvdC2->AcceptParentalLevelChange(FALSE);
                    }
                    break;
                }

            case EC_DVD_ERROR:
                DbgLog((LOG_TRACE, 3, TEXT("DVD Event: Error event received (code %ld)"),
					static_cast<ULONG>(lParam1))) ;
                switch (lParam1)
                {
                case DVD_ERROR_Unexpected:
                    MessageBox(m_hWnd,
                        TEXT("An unexpected error (possibly incorrectly authored content)")
                        TEXT("\nwas encountered.")
                        TEXT("\nCan't playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_CopyProtectFail:
                    MessageBox(m_hWnd,
                        TEXT("Key exchange for DVD copy protection failed.")
                        TEXT("\nCan't playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_InvalidDVD1_0Disc:
                    MessageBox(m_hWnd,
                        TEXT("This DVD-Video disc is incorrectly authored for v1.0  of the spec.")
                        TEXT("\nCan't playback this disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_InvalidDiscRegion:
                    MessageBox(m_hWnd,
                        TEXT("This DVD-Video disc cannot be played, because it is not")
                        TEXT("\nauthored to play in the current system region.")
                        TEXT("\nThe region mismatch may be fixed by changing the")
                        TEXT("\nsystem region (with DVDRgn.exe)."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    ChangeDvdRegion();
                    break ;

                case DVD_ERROR_LowParentalLevel:
                    MessageBox(m_hWnd,
                        TEXT("Player parental level is set lower than the lowest parental")
                        TEXT("\nlevel available in this DVD-Video content.")
                        TEXT("\nCannot playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_MacrovisionFail:
                    MessageBox(m_hWnd,
                        TEXT("This DVD-Video content is protected by Macrovision.")
                        TEXT("\nThe system does not satisfy Macrovision requirement.")
                        TEXT("\nCan't continue playing this disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_IncompatibleSystemAndDecoderRegions:
                    MessageBox(m_hWnd,
                        TEXT("No DVD-Video disc can be played on this system, because ")
                        TEXT("\nthe system region does not match the decoder region.")
                        TEXT("\nPlease contact the manufacturer of this system."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;

                case DVD_ERROR_IncompatibleDiscAndDecoderRegions:
                    MessageBox(m_hWnd,
                        TEXT("This DVD-Video disc cannot be played on this system, because it is")
                        TEXT("\nnot authored to be played in the installed decoder's region."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                    m_pIMC->Stop() ;
                    break ;
                }  // end of switch (lParam1)
                break ;

            // Next is warning
            case EC_DVD_WARNING:
                switch (lParam1)
                {
                case DVD_WARNING_InvalidDVD1_0Disc:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: Current disc is not v1.0 spec compliant"))) ;
                    break ;

                case DVD_WARNING_FormatNotSupported:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: The decoder does not support the new format."))) ;
                    break ;

                case DVD_WARNING_IllegalNavCommand:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: An illegal navigation command was encountered."))) ;
                    break ;

                case DVD_WARNING_Open:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: Could not open a file on the DVD disc."))) ;
                    MessageBox(m_hWnd,
                        TEXT("A file on the DVD disc could not be opened. Playback may not continue."),
                        TEXT("Warning"), MB_OK | MB_ICONINFORMATION) ;
                    break ;

                case DVD_WARNING_Seek:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: Could not seek in a file on the DVD disc."))) ;
                    MessageBox(m_hWnd,
                        TEXT("Could not move to a different part of a file on the DVD disc. Playback may not continue."),
                        TEXT("Warning"), MB_OK | MB_ICONINFORMATION) ;
                    break ;

                case DVD_WARNING_Read:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: Could not read a file on the DVD disc."))) ;
                    MessageBox(m_hWnd,
                        TEXT("Could not read part of a file on the DVD disc. Playback may not continue."),
                        TEXT("Warning"), MB_OK | MB_ICONINFORMATION) ;
                    break ;

                default:
                    DbgLog((LOG_ERROR, 1, TEXT("DVD Warning: An unknown (%ld) warning received."),
						static_cast<ULONG>(lParam1))) ;
                    break ;
                }
                break ;

            case EC_DVD_BUTTON_CHANGE:
               break;

            case EC_DVD_STILL_ON:
                if (TRUE == lParam1) // if there is a still without buttons, we can call StillOff
                    m_bStillOn = true;
                break;

            case EC_DVD_STILL_OFF:
                m_bStillOn = false; // we are no longer in a still
                break;

        } // end of switch(lEvent)

        m_pIME->FreeEventParams(lEvent, lParam1, lParam2) ;

    } // end of while(GetEvent())

    return 0;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnMouseEvent()
// Desc: This method acts on mouse events sent to it from the WndProc method
//------------------------------------------------------------------------------

LRESULT CDvdCore::OnMouseEvent(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    if (true == m_bFullScreenOn) // if any mouse activity happens while in full screen mode
    {
        ShowMouseCursor(true);
    }

    switch (uMessage)
    {
        case WM_MOUSEMOVE:
        {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                m_pIDvdC2->SelectAtPosition(pt);
                return 0;
        }

        case WM_LBUTTONUP:
        {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                m_pIDvdC2->ActivateAtPosition(pt);
                return 0;
        }
    }

    return DefWindowProc(m_hWnd, uMessage, wParam, lParam);
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnKeyEvent()
// Desc: This method processes keyboard events forwarded to it by the WndProc.
//------------------------------------------------------------------------------

LRESULT CDvdCore::OnKeyEvent(WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::OnKeyEvent()"))) ;

    switch (wParam)
    {
        case VK_ESCAPE: // exit full screen
            if (true == m_bFullScreenOn) // only do this if we are in fullscreen mode
                StopFullScreen();
            return 0;

        case VK_RETURN: // activate the currently selected button
            m_pIDvdC2->ActivateButton();
            return 0;

        case VK_LEFT: // select the left button
            m_pIDvdC2->SelectRelativeButton(DVD_Relative_Left);
            return 0;

        case VK_RIGHT: // select the right button
            m_pIDvdC2->SelectRelativeButton(DVD_Relative_Right);
            return 0;

        case VK_UP: // select the upper button
            m_pIDvdC2->SelectRelativeButton(DVD_Relative_Upper);
            return 0;

        case VK_DOWN: // select the lower button
            m_pIDvdC2->SelectRelativeButton(DVD_Relative_Lower);
            return 0;
    }

    return DefWindowProc(m_hWnd, WM_KEYUP, wParam, lParam);
}


//------------------------------------------------------------------------------
// Name: CDvdCore::ShowMouseCursor()
// Desc: This method is used to show or hide the cursor during fullscreen mode
//------------------------------------------------------------------------------

void CDvdCore::ShowMouseCursor(bool bShow)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::ShowMouseCursor(%s)"),
        bShow ? TEXT("true") : TEXT("false"))) ;

    if (true == bShow)   // display cursor
    {
        while (m_iMouseShowCount < 0)
        {
            m_iMouseShowCount = ::ShowCursor(TRUE) ; // keep adding until the count is 0 or higher
        }
        m_dwMouseMoveTime = timeGetTime() ;  // it has just been moved
    }
    else // hide cursor
    {
        while (m_iMouseShowCount >= 0)
        {
            m_iMouseShowCount = ::ShowCursor(FALSE) ; // keep subtracting until the count is below 0
        }
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnMouseTimer()
// Desc: This method will hide the pointer if the mouse hasn't moved for
//       MOUSE_TIMEOUT milliseconds.
//------------------------------------------------------------------------------

LRESULT CDvdCore::OnMouseTimer(WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::OnMouseTimer()"))) ;

    if (MOUSETIMER != wParam ) // if it isn't our timer we won't do anything with it
        return (DefWindowProc(m_hWnd, WM_TIMER, wParam, lParam));

    // If we are in fullscreen mode, check if mouse should be hidden now
    if (true == m_bFullScreenOn) // we should be but just to make sure
    {
        if (timeGetTime() - m_dwMouseMoveTime > MOUSE_TIMEOUT)
        {
            ShowMouseCursor(false) ;
        }
    }

    return 0 ;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnSize()
// Desc: This method is called whenever we receive WM_SIZE.  It will resize the
//       IVideoWindow window to match our new client space.
//------------------------------------------------------------------------------

LRESULT CDvdCore::OnSize(WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::OnSize()"))) ;

    // set the video window to fill the client space
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    HRESULT hr = m_pIVW->SetWindowPosition(0, 0, rect.right, rect.bottom);
    ASSERT(SUCCEEDED(hr)) ;

    return 0;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetParentalLevel()
// Desc: This method returns the current parental level setting of the DVD Navigator
//------------------------------------------------------------------------------

ULONG CDvdCore::GetParentalLevel()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::GetParentalLevel()"))) ;

    ULONG ulLevel = 0;
    UCHAR cCountry[2];

    HRESULT hr;
    hr = m_pIDvdI2->GetPlayerParentalLevel(&ulLevel, cCountry);
    ASSERT(SUCCEEDED(hr));

    return ulLevel;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::SetParentalLevel()
// Desc: This method sets the Navigator's parental level
//------------------------------------------------------------------------------

bool CDvdCore::SetParentalLevel(ULONG ulLevel)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::SetParentalLevel() - Level: %u"), ulLevel)) ;

    if (Graph_Stopped2 != GetState())
    {
        // we need to be in the stop state to change parental levels.
        // we can either stop from IMediaControl or stop from IDvdControl2
        // we choose to stop from the IMediaControl because it makes the play code more clean
        m_pIDvdC2->SetOption(DVD_ResetOnStop, TRUE);  // do a complete reset
        m_pIMC->Stop();                               // stop the entire graph
        m_pIDvdC2->SetOption(DVD_ResetOnStop, FALSE); // restore previous settings
        SetState(Graph_Stopped2);
    }

    HRESULT hr = m_pIDvdC2->SelectParentalLevel(ulLevel);
    if (SUCCEEDED(hr))
        return true;
    else
        return false;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::UpdateStatus()
// Desc: This method is just a stub to make our work with the callback cleaner.
//       We will point our m_pCallback to this unless given a different pointer by
//       the calling function
//------------------------------------------------------------------------------

void CDvdCore::UpdateStatus(void)
{
    //intentionally left blank
}


//------------------------------------------------------------------------------
// Name: CDvdCore::SetDirectory()
// Desc: This method is a wrapper for SetDVDDirectory.  It changes the current
//       directory used for video playback.  It is called by the Select Disc
//       menu function.
//------------------------------------------------------------------------------

bool CDvdCore::SetDirectory(TCHAR *szDirectory)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::SetDirectory()"))) ;

    HRESULT hr = m_pIDvdC2->Stop() ;  // first ask player to stop the playback.
    if (SUCCEEDED(hr))
        SetState(Nav_Stopped);

    // Convert TCHAR filename to WCHAR for COM
    WCHAR   szwFileName[MAX_PATH] ;

#ifdef UNICODE
    StringCchCopy(szwFileName, MAX_PATH, szDirectory) ;
#else
    MultiByteToWideChar(CP_ACP, 0, szDirectory, -1, szwFileName, NUMELMS(szwFileName)-1) ;
#endif // UNICODE
    szwFileName[MAX_PATH-1] = TEXT('\0');   // Null-terminate

    hr = m_pIDvdC2->SetDVDDirectory(szwFileName) ;
    if (SUCCEEDED(hr))  // if the new file name is valid DVD-video volume
    {
        return true;
    }
    else
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: SetDVDDirectory(%s) failed (Error 0x%lx)"),
            szDirectory, hr)) ;
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::FrameStep()
// Desc: This method will pause the graph and step forward one step at a time
//------------------------------------------------------------------------------

bool CDvdCore::FrameStep()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::FrameStep()"))) ;

    if (Uninitialized == GetState())
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: DVDCore is not initialized!"),
            szDirectory, hr)) ;
        return false;
    }

    // Get the Frame Stepping Interface
    IVideoFrameStep* pFS;
    HRESULT hr;
    hr = m_pGraph->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFS);

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: Can't get IVideoFrameStep interface!"),
            szDirectory, hr)) ;
        return false;
    }

    // check if this decoder can step
    hr = pFS->CanStep(0L, NULL);

    if (S_FALSE == hr)
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: Decoder can't step"),
            szDirectory, hr)) ;
        pFS->Release();
        return false;
    }

    // this will only work while the graph is paused
    m_pIMC->Pause();
    SetState(Graph_Paused);

    // step one frame
    hr = pFS->Step(1, NULL);

    bool retVal = SUCCEEDED(hr) ? true : false;

    // clean up after ourselves
    pFS->Release();

    return retVal;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::PlayChapter()
// Desc: This method plays a chapter in the current title
//------------------------------------------------------------------------------

bool CDvdCore::PlayChapter(ULONG ulChap)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::PlayChapter() %u"), ulChap)) ;

    HRESULT hr = m_pIDvdC2->PlayChapter(ulChap, DVD_CMD_FLAG_Block, NULL);
    if (SUCCEEDED(hr))
        return true;
    else
    {
        if (VFW_E_DVD_OPERATION_INHIBITED == hr)
            m_pCallback->Prohibited();
        DbgLog((LOG_ERROR, 1, TEXT("CDvdCore::PlayChapter() failed: %#x"), hr)) ;
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::PlayChapterInTitle()
// Desc: This method plays a chapter in the specified title
//------------------------------------------------------------------------------

bool CDvdCore::PlayChapterInTitle(ULONG ulTitle, ULONG ulChapter)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::PlayChapterInTitle() Title: %u, "
        "Chapter: %u"), ulTitle, ulChapter)) ;

    HRESULT hr = m_pIDvdC2->PlayChapterInTitle(ulTitle, ulChapter, DVD_CMD_FLAG_Block, NULL);
    if (SUCCEEDED(hr))
        return true;
    else
    {
        if (VFW_E_DVD_OPERATION_INHIBITED == hr)
            m_pCallback->Prohibited();
        DbgLog((LOG_ERROR, 1, TEXT("CDvdCore::PlayChapterInTitle() failed: %#x"), hr)) ;
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::PlayTime()
// Desc: This method plays a time in the current title
//------------------------------------------------------------------------------

bool CDvdCore::PlayTime(DVD_HMSF_TIMECODE time)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDvdCore::PlayTime() hh:mm:ss %u:%u:%u"),
        time.bHours, time.bMinutes, time.bSeconds)) ;

    HRESULT hr = m_pIDvdC2->PlayAtTime(&time, DVD_CMD_FLAG_Block, NULL);
    if (SUCCEEDED(hr))
        return true;
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("CDvdCore::PlayTime() failed: %#x"), hr)) ;
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::ChangeDvdRegion()
// Desc: This method changes the DVD Region
//
// Returns:
//      true:   if the drive region change is successful
//      false:  if drive region change fails (probably
//          because no drive was found with a valid
//          DVD-V disc).
//
//------------------------------------------------------------------------------

bool CDvdCore::ChangeDvdRegion()
{
    typedef BOOL (APIENTRY * DVDPPLAUNCHER) (HWND hWnd, CHAR DriveLetter);

    HRESULT hr;
    bool    bRegionChanged = false;
    TCHAR   szCmdLine[MAX_PATH + 32]={0};  // Allow for adding of \storprop.dll

    //
    // First find out which drive is a DVD drive with a
    // valid DVD-V disc.
    //
    TCHAR   szDVDDrive[4]={0};
    if (! GetDriveLetter (szDVDDrive, NUMELMS(szDVDDrive) ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("No DVD drive was found with DVD-Video disc.\0")
        TEXT("Cannot change DVD region of the drive.\0")));
        return false;
    }

    //
    // Detect which OS we are running on.  For Windows NT,
    // use the storprop.dll, while for Windows 9x platform
    // use the DVDRgn.exe application to change the region.
    //
    OSVERSIONINFO   ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);

    if (VER_PLATFORM_WIN32_NT  == ver.dwPlatformId)
    {
        // Windows NT platform
        HINSTANCE       hInstDLL;
        DVDPPLAUNCHER   dvdPPLauncher;
        CHAR            szDVDDriveA[4]={0};

    #ifdef UNICODE
        WideCharToMultiByte(0, 0, szDVDDrive, -1,
                            szDVDDriveA, 3, NULL, NULL );
    #else
        StringCchCopyNA(szDVDDriveA, NUMELMS(szDVDDriveA), szDVDDrive, 3);
    #endif  // UNICODE
        szDVDDrive[3] = 0;  // Ensure NULL termination

        if (!GetSystemDirectory(szCmdLine, MAX_PATH))
            return false;

        hr = StringCchCat(szCmdLine, NUMELMS(szCmdLine), TEXT("\\storprop.dll\0"));

        hInstDLL = LoadLibrary(szCmdLine);
        if (hInstDLL)
        {
            dvdPPLauncher = (DVDPPLAUNCHER) GetProcAddress(hInstDLL, "DvdLauncher");
            if (dvdPPLauncher)
            {
                if (TRUE == dvdPPLauncher(m_hWnd, szDVDDriveA[0]))
                    bRegionChanged = true;
                else
                    bRegionChanged = false;
            }

            FreeLibrary(hInstDLL);
        }
    }  // end of region change code for Windows NT platform
    else   // Windows 98 platform
    {
        TCHAR szAppName[MAX_PATH];

        // Get path of \windows\dvdrgn.exe for command line string
        if (! GetWindowsDirectory(szCmdLine, MAX_PATH - lstrlen(TEXT("\\DVDRgn.exe \0"))))
            return false;

        hr = StringCchCat(szCmdLine, NUMELMS(szCmdLine), TEXT("\\DVDRgn.exe \0"));

        // Copy the application name for use with CreateProcess.  This enhances
        // security by ensuring that the expected version of DVDRgn.exe is used
        // (the copy in the Windows directory)
        StringCchCopy(szAppName, NUMELMS(szAppName), szCmdLine);

        // Add only the drive letter as command line parameter
        hr = StringCchCat(szCmdLine, 1, szDVDDrive);

        // Prepare to execute DVDRgn.exe
        STARTUPINFO          StartupInfo;
        PROCESS_INFORMATION  ProcessInfo;
        StartupInfo.cb              = sizeof(StartupInfo);
        StartupInfo.dwFlags         = STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow     = SW_SHOWNORMAL;
        StartupInfo.lpReserved      = NULL;
        StartupInfo.lpDesktop       = NULL;
        StartupInfo.lpTitle         = NULL;
        StartupInfo.cbReserved2     = 0;
        StartupInfo.lpReserved2     = NULL;

        if (CreateProcess(szAppName, szCmdLine, NULL, NULL, TRUE,
                    NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo) )
        {
            // Wait until DVDRgn.exe finishes
            WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
            DWORD dwRet = 1;
            BOOL bRet = GetExitCodeProcess(ProcessInfo.hProcess,
                                                     &dwRet);
            // If the user changed the drive region
            // successfully, the exit code of DVDRgn.exe is 0.
            if (bRet && 0 == dwRet)
            {
                bRegionChanged = true;
            }
        }
    }  // end of region change code for Windows 9x platform

    if (bRegionChanged)   // if region changed successfully
    {
        // We now cycle through stop.  We have to turn on ResetOnStop or we won't reset
        // the internals of the navigator when we stop.  After we have stopped, we reset
        // this option to its original state.
        HRESULT hr;
        hr = m_pIMC->Pause();
        hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, TRUE);
        hr = m_pIMC->Stop(); // come to a complete stop.
        hr = m_pIDvdC2->SetOption(DVD_ResetOnStop, FALSE);
        hr = m_pIMC->Run();
        return true;
    }
    else   // DVD region didn't happen
    {
        DbgLog((LOG_ERROR, 1, TEXT("DVD drive region could not be changed.")) );
        return false;
    }
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetDriveLetter()
// Desc: This method gets the drive letter of the currently active DVD drive
//
// Returns:
//      true:   if a DVD drive is found (with valid disc)
//      false:  if no DVD drive was found with valid DVD-V
//          disc.
//
//------------------------------------------------------------------------------

bool CDvdCore::GetDriveLetter(TCHAR *pszDrive, DWORD cchDrive)
{
    WCHAR   szwPath[MAX_PATH];
    ULONG   ulActualSize;

    pszDrive[0] = pszDrive[3] = TEXT('\0');

    //
    // Get the current root directory
    //
    if (SUCCEEDED(m_pIDvdI2->GetDVDDirectory(szwPath, MAX_PATH, &ulActualSize)))
    {
#ifdef UNICODE
        StringCchCopyNW(pszDrive, cchDrive, szwPath, 3);
#else
        CHAR szDrive[MAX_PATH];
        WideCharToMultiByte(CP_ACP, NULL, szwPath, -1, szDrive, MAX_PATH-1, NULL, NULL);
        szDrive[MAX_PATH-1] = TEXT('\0');
        StringCchCopyNA(pszDrive, cchDrive, szDrive, 3);
#endif;
        return true;
    }

    return false; // couldn't find a directory
}


//------------------------------------------------------------------------------
// Name: CDvdCore::DoesFileExist()
// Desc: This method determines if the given filename is an existing file
//------------------------------------------------------------------------------

bool CDvdCore::DoesFileExist(PTSTR pszFile)
{
    HANDLE      hFile = NULL ;

    //
    // We don't want any error message box to pop up when
    // we try to test if the required file is available to
    // open for read.
    //
    UINT uErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS |
     SEM_NOOPENFILEERRORBOX);
    hFile = CreateFile(pszFile, GENERIC_READ,
                       FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);
    SetErrorMode(uErrorMode);  // restore error mode
    if (INVALID_HANDLE_VALUE  == hFile)
        return FALSE ;

    CloseHandle(hFile);
    return TRUE;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::UnInitMessageSink()
// Desc: This method undoes what InitMessageSink does.  It is important that we remove
//       the child window bit from the video window before we close the parent window.
//------------------------------------------------------------------------------

void CDvdCore::UnInitMessageSink()
{
    // Make the sure the video window is no longer child of the playback window.
    if (m_pIVW)
    {
        long lStyle;
        m_pIVW->get_WindowStyle(&lStyle);
        m_pIVW->put_WindowStyle(lStyle & ~WS_CHILD);
        m_pIVW->put_Owner(NULL);
    }

    DestroyWindow(m_hWnd);
}


//------------------------------------------------------------------------------
// Name: CDvdCore::OnClose()
// Desc: This method is called when the playback window is closed.  In response it
// calls the currently registered IDvdCallback object's Exit method.
//------------------------------------------------------------------------------

LRESULT CDvdCore::OnClose()
{
    m_pCallback->Exit();
    return 0;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetPlaybackWindowRect()
// Desc: This method will check the size of the desktop and scale the size of
//       the playback window's accordingly.
//------------------------------------------------------------------------------

RECT CDvdCore::GetPlaybackWindowRect()
{
    RECT rDesktop, rWindow, rApp;
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rDesktop, NULL);
    rApp = m_pCallback->GetAppWindow();

    rWindow.left = rDesktop.left + 5; // leave a 5 pixel gap
    rWindow.top = rDesktop.top + 5; // leave a 5 pixel gap
    rWindow.bottom = rApp.top; // place this window above the top of player window
    rWindow.right = rDesktop.left + ((rWindow.bottom - rWindow.top) * 4)/3; //maintain 4:3 aspect ratio

    return rWindow;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetDvdText()
// Desc: This method will read the Dvd Text Info off of a disc and output it to
//       a messagebox.  We will be using GetDvdTextStringAsUnicode.  There is
//       also a "native" version but we don't use it here.  The Unicode version
//       will work better with non-English text.
//
//       This is done only to show how to read the text info and
//       display it.  In a real application, you would probably
//       create your own data structure to hold the text information.
//------------------------------------------------------------------------------

bool CDvdCore::GetDvdText()
{
    ULONG ulNumStrings, ulNumLang;
    LCID LangCode;
    DVD_TextCharSet CharSet;

    HRESULT hr;

    // first, we check if there is any text on this disc
    hr = m_pIDvdI2->GetDVDTextNumberOfLanguages(&ulNumLang);
    if (0 == ulNumLang || FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("There is no text on this disc"), TEXT("No Text Present"),
                   MB_OK);
        return false;
    }

    ULONG ulLangIndex = 0; // We will just grab the first language.  You might want to
                           //ask the user which language they want to see or try to
                           //choose one that closest matches the locale of the machine

    // next we get the number of strings available
    hr = m_pIDvdI2->GetDVDTextLanguageInfo(ulLangIndex, &ulNumStrings,
                                           &LangCode, &CharSet);

    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("There was an error getting the number of Strings"),
            TEXT("Error!"), MB_OK);
        return false;
    }

#ifdef UNICODE
    wstringstream aString;
#else
    stringstream aString;
#endif

    WCHAR buffer[1024] = {L"\0"}; // should be ample space

    for (ULONG ulStringIndex = 0; ulStringIndex < ulNumStrings; ulStringIndex++)
    {
        ULONG ulActualSize;
        DVD_TextStringType type;

        hr = m_pIDvdI2->GetDVDTextStringAsUnicode(ulLangIndex, ulStringIndex,
                        buffer, sizeof(buffer)/sizeof(*buffer),
                        &ulActualSize, &type);

        if (SUCCEEDED(hr))
        {
#ifdef UNICODE
            // Any string type less than 0x20 is an empty string used
            // only to identify the logical structure or stream.
            // Here, we display the string types to illustrate how
            // they are used to organize text strings on the disc.
            if(type > DVD_Channel_Audio) //0x20
                aString << AsStr(type) << TEXT(" ") << buffer << endl;
            else
                aString << AsStr(type) << TEXT(" ") << endl;
#else
            // we need to convert the Unicode to ANSI so we can output it
            // stringstream can't handle unicode characters
            CHAR myBuffer[1024];
            WideCharToMultiByte(CP_ACP, 0, buffer, -1, myBuffer, 1024, NULL, NULL);
            myBuffer[1023] = 0;     // Null-terminate

            // Any string type less than 0x20 is an empty string used
            // only to identify the logical structure or stream.
            // Here, we display the string types to illustrate how
            // they are used to organize text strings on the disc.
            if(type > DVD_Channel_Audio) //0x20
                aString << AsStr(type) << " " << myBuffer << endl;
            else
                aString << AsStr(type) << " " << endl;
#endif
        }
    }

    MessageBox(m_hWnd, aString.str().c_str(), TEXT("Dvd Text Info"), MB_OK);
    return true; // once everything is over, if we haven't failed, we must have passed
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetAudioAttributes()
// Desc: This method will parse the audio attributes of the current audio stream
//       and output it to a messagebox.
//
//       You probably don't want to dump raw data like this on the user.  This is
//       merely to demonstrate how to parse the information.  The same information
//       found here can be used to describe the audio in a more user-friendly
//       manner.  In a real application, this function should probably return
//       a string to CApp rather than displaying it itself.
//------------------------------------------------------------------------------

bool CDvdCore::GetAudioAttributes()
{
    HRESULT hr;

    DVD_AudioAttributes audioAtr;
    hr = m_pIDvdI2->GetAudioAttributes(DVD_STREAM_DATA_CURRENT, &audioAtr);

    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("GetAudioAttributes Failed"), TEXT("Error!"), MB_OK);
        return false;
    }

#ifdef UNICODE
    wstringstream aString;
#else
    stringstream aString;
#endif

    aString << TEXT("Audio Attributes :") << endl << endl << TEXT("Audio Application Mode =")
            << AsStr(audioAtr.AppMode) << endl;

    aString << TEXT("Audio Format = ") << AsStr(audioAtr.AudioFormat) << endl;

    aString << TEXT("Multichannel Extension = ")
            << AsStr( audioAtr.fHasMultichannelInfo != FALSE ) << endl;

    aString << TEXT("Number of Audio Channels = ") <<
        static_cast<ULONG>(audioAtr.bNumberOfChannels) << endl; // the cast is only necessary to
                                                                // allow IOStream to understand it

    aString << TEXT("Sampling Frequency = ") << audioAtr.dwFrequency << endl;

    aString << TEXT("Sampling Quantization = ") << static_cast<ULONG>(audioAtr.bQuantization) << endl;

    aString << TEXT("Language Code LCID = ") << audioAtr.Language << endl;

    aString << TEXT("Language Code Extension = ") << AsStr(audioAtr.LanguageExtension) << endl;

    if( DVD_AudioMode_Karaoke == audioAtr.AppMode )
    {
        DVD_KaraokeAttributes karaokeAtr;

        hr = m_pIDvdI2->GetKaraokeAttributes(DVD_STREAM_DATA_CURRENT, &karaokeAtr );
        if (SUCCEEDED(hr))
        {
            aString << endl << TEXT("Karaoke Attributes :") << endl << endl;
            aString << TEXT("Version = ") << static_cast<ULONG>(karaokeAtr.bVersion) << endl;

            aString << TEXT("MasterOfCeremoniesInGuideVocal1 = ") <<
                    AsStr( karaokeAtr.fMasterOfCeremoniesInGuideVocal1 != FALSE ) << endl;

            aString << TEXT("Duet = ") << AsStr( karaokeAtr.fDuet != FALSE ) << endl;

            aString << TEXT("ChannelAssignment = ") << AsStr( karaokeAtr.ChannelAssignment ) << endl;


            if( VFW_S_DVD_CHANNEL_CONTENTS_NOT_AVAILABLE != hr)
            {
                aString << endl << TEXT("Channel contents:") << endl << endl;

                for( UINT i=0; i < audioAtr.bNumberOfChannels; i++)
                {
                    aString << TEXT("Channel ") << i << TEXT(":");
                    WORD c = karaokeAtr.wChannelContents[i];
                    if( c & DVD_Karaoke_GuideVocal1 ) aString << TEXT(" GuideVocal1");
                    if( c & DVD_Karaoke_GuideVocal2 ) aString << TEXT(" GuideVocal2");
                    if( c & DVD_Karaoke_GuideMelody1 ) aString << TEXT(" GuideMelody1");
                    if( c & DVD_Karaoke_GuideMelody2 ) aString << TEXT(" GuideMelody2");
                    if( c & DVD_Karaoke_GuideMelodyA ) aString << TEXT(" GuideMelodyA");
                    if( c & DVD_Karaoke_GuideMelodyB ) aString << TEXT(" GuideMelodyB");
                    if( c & DVD_Karaoke_SoundEffectA ) aString << TEXT(" SoundEffectA");
                    if( c & DVD_Karaoke_SoundEffectB ) aString << TEXT(" SoundEffectB");
                    aString << endl;
                }
            }
        }
    }

    MessageBox(m_hWnd, aString.str().c_str(), TEXT("Audio Attributes"), MB_OK);
    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetVideoAttributes()
// Desc: This method will parse the video attributes of the current video stream
//       and output it to a messagebox.
//
//       You probably don't want to dump raw data like this on the user.  This is
//       merely to demonstrate how to parse the information.  The same information
//       found here can be used to describe the video in a more user-friendly
//       manner.  In a real application, this function should probably return
//       a string to CApp rather than displaying it itself.
//------------------------------------------------------------------------------

bool CDvdCore::GetVideoAttributes()
{
    DVD_VideoAttributes atrVideo;
    HRESULT hr;

    hr = m_pIDvdI2->GetCurrentVideoAttributes(&atrVideo);
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("GetCurrentVideoAttributes Failed"), TEXT("Error!"), MB_OK);
        return false;
    }

#ifdef UNICODE
    wstringstream aString;
#else
    stringstream aString;
#endif

    aString << TEXT("Video Attributes:") << endl << endl;

    aString << TEXT("Pan-Scan permitted: ") << AsStr(atrVideo.fPanscanPermitted) << endl;

    aString << TEXT("Letterbox permitted: ") << AsStr(atrVideo.fLetterboxPermitted) << endl;

    aString << TEXT("Aspect: " << atrVideo.ulAspectX << TEXT(" x ") << atrVideo.ulAspectY << endl;

    aString << TEXT("Frame Rate: ") << atrVideo.ulFrameRate << endl;

    aString << TEXT("Frame Height: ") << atrVideo.ulFrameHeight << endl;

    aString << TEXT("Compression Mode: ") << AsStr(atrVideo.Compression) << endl;

    aString << TEXT("Line21 Field 1: ") << AsStr(atrVideo.fLine21Field1InGOP) << endl;

    aString << TEXT("Line21 Field 2: ") << AsStr(atrVideo.fLine21Field2InGOP) << endl;

    aString << TEXT("Source Resolution: ") << atrVideo.ulSourceResolutionX << TEXT(" x ") <<
           atrVideo.ulSourceResolutionY) << endl;

    aString << TEXT("Is Source Letterboxed? ") << AsStr(atrVideo.fIsSourceLetterboxed) << endl;

    aString << TEXT("Is Film Mode? ") << AsStr(atrVideo.fIsFilmMode) << endl;

    MessageBox(m_hWnd, aString.str().c_str(), TEXT("Video Attributes"), MB_OK);
    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::GetSPAttributes()
// Desc: This method will parse the subpicture attributes of the current video stream
//       and output it to a messagebox.
//
//       You probably don't want to dump raw data like this on the user.  This is
//       merely to demonstrate how to parse the information.  The same information
//       found here can be used to describe the subpicture in a more user-friendly
//       manner.  In a real application, this function should probably return
//       a string to CApp rather than displaying it itself.
//------------------------------------------------------------------------------

bool CDvdCore::GetSPAttributes()
{
    DVD_SubpictureAttributes atrSP;
    HRESULT hr;

    hr = m_pIDvdI2->GetSubpictureAttributes(DVD_STREAM_DATA_CURRENT, &atrSP);
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("GetSubpictureAttributes Failed"), TEXT("Error!"), MB_OK);
        return false;
    }

#ifdef UNICODE
    wstringstream aString;
#else
    stringstream aString;
#endif

    aString << TEXT("SubPicture Attributes:") << endl << endl;
    aString << TEXT("Subpicture Type = ") << AsStr(atrSP.Type) << endl;

    aString << TEXT("Subpicture Coding Mode = ") << AsStr(atrSP.CodingMode) << endl;

    TCHAR pszCode[100];
    if (0 == GetLocaleInfo(atrSP.Language, LOCALE_SLANGUAGE, pszCode, 100))
    {
        aString << TEXT("GetLocaleInfo Failed !!!") << endl;
    }
    else
    {
        aString << TEXT("Language Code = ") << pszCode << endl;
    }

    aString << TEXT("Language Code Extension = ") << AsStr(atrSP.LanguageExtension) << endl;

    MessageBox(m_hWnd, aString.str().c_str(), TEXT("Video Attributes"), MB_OK);
    return true;
}


//------------------------------------------------------------------------------
// Name: CDvdCore::ToggleVMR9AndRebuildGraph()
// Desc: This method attempts to build the dvd graph using VMR9. If the graph
//       exists already, it is released and re-created. If VMR9 cannot be used,
//       the user has the option of proceeding with the default setting or leaving
//       the application.
//       The function returns the current status of using VMR9 (true/false).
//
//       You probably don't want to dump raw data like this on the user.  This is
//       merely to demonstrate how to parse the information.  The same information
//       found here can be used to describe the subpicture in a more user-friendly
//       manner.  In a real application, this function should probably return
//       a string to CApp rather than displaying it itself.
//------------------------------------------------------------------------------
bool CDvdCore::ToggleVMR9AndRebuildGraph(void)
{
    //  switch state variable
    m_bUseVMR9 = !m_bUseVMR9;

    //  stop if playing
    if ((Playing == m_eState) && !Stop())
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: failed to stop the DVD graph."))) ;
        return (m_bUseVMR9 = false);
    }

    //  release message sink
    if (m_bMessageSink)
    {
        UnInitMessageSink();
    }

    //  UnInitMessageSink destroys the container window and therefore the next play
    //  command will be (again) the first
    m_bFirstPlay = true;

    //  re-init
    //  BuildGraph also releases interfaces before building the graph etc
    if (!Init())
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: failed to initialize the DVD player object."))) ;
        m_bUseVMR9 = false;
    }

    return m_bUseVMR9;
}   //  CDvdCore::ToggleVMR9AndRebuildGraph

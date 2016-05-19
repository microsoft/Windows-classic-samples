//------------------------------------------------------------------------------
// File: vcdplyer.cpp
//
// Desc: DirectShow sample code
//       - A VMR-enabled player application
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "project.h"

#include <stdarg.h>
#include <initguid.h>
#include <strsafe.h>

extern int FrameStepCount;


/******************************Public*Routine******************************\
* CMovie
*
* Constructors and destructors
*
\**************************************************************************/
CMovie::CMovie(HWND hwndApplication) :
      m_hwndApp(hwndApplication),
      m_MediaEvent(NULL),
      m_Mode(MOVIE_NOTOPENED),
      m_Fg(NULL),
      m_Gb(NULL),
      m_Mc(NULL),
      m_Ms(NULL),
      m_Me(NULL),
      m_Wc(NULL),
      m_pMixControl(NULL),
      m_TimeFormat(TIME_FORMAT_MEDIA_TIME)
{
}

CMovie::~CMovie()
{
}


/******************************Public*Routine******************************\
* SetRenderingMode
*
\**************************************************************************/
static HRESULT SetRenderingMode( IBaseFilter* pBaseFilter, VMR9Mode mode )
{
    // Test VMRConfig, VMRMonitorConfig
    IVMRFilterConfig9* pConfig;

    HRESULT hr = pBaseFilter->QueryInterface(IID_IVMRFilterConfig9, (LPVOID *)&pConfig);
    if(SUCCEEDED(hr))
    {
        hr = pConfig->SetRenderingMode(mode);
        hr = pConfig->SetRenderingPrefs(RenderPrefs_ForceOverlays|RenderPrefs_AllowOverlays);
        pConfig->Release();
    }
    return hr;
}


/******************************Public*Routine******************************\
* AddVideoMixingRendererToFG
*
\**************************************************************************/
HRESULT
CMovie::AddVideoMixingRendererToFG()
{
    IBaseFilter* pBF = NULL;
    HRESULT hRes = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC,
                                    IID_IBaseFilter, (LPVOID *)&pBF);

    if(SUCCEEDED(hRes))
    {
        hRes = m_Fg->AddFilter(pBF, L"Video Mixing Renderer 9");

        if(SUCCEEDED(hRes))
        {
            // Test VMRConfig, VMRMonitorConfig
            IVMRFilterConfig9* pConfig;
            HRESULT hRes2 = pBF->QueryInterface(IID_IVMRFilterConfig9, (LPVOID *)&pConfig);
            if(SUCCEEDED(hRes2))
            {
                hRes2 = pConfig->SetNumberOfStreams(2);
                hRes2 = pConfig->SetRenderingMode(VMR9Mode_Windowless);
                //hRes2 = pConfig->SetRenderingPrefs(RenderPrefs_AllowOverlays);
                pConfig->Release();
            }

            IVMRMonitorConfig9* pMonitorConfig;
            HRESULT hRes3 = pBF->QueryInterface(IID_IVMRMonitorConfig9, (LPVOID *)&pMonitorConfig);
            if(SUCCEEDED(hRes3))
            {
                UINT iCurrentMonitor;
                HRESULT hr4 = pMonitorConfig->GetMonitor(&iCurrentMonitor);
                pMonitorConfig->Release();
            }

            hRes = pBF->QueryInterface(IID_IVMRWindowlessControl9, (LPVOID *)&m_Wc);
        }
    }

    if(pBF)
    {
        pBF->Release();
    }

    if(SUCCEEDED(hRes))
    {
        HRESULT hr = m_Wc->SetVideoClippingWindow(m_hwndApp);
        hr = m_Wc->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
    }
    else
    {
        if(m_Wc)
        {
            m_Wc->Release();
            m_Wc = NULL;
        }
    }

    return hRes;
}


/******************************Public*Routine******************************\
* RenderSecondFile
*
\**************************************************************************/
HRESULT
CMovie::RenderSecondFile(
    TCHAR* lpFileName
    )
{
    HRESULT hRes;
    WCHAR FileName[MAX_PATH];

    // Check input string
    if (lpFileName == NULL)
        return E_POINTER;

    // Since the user might have already rendered the second file,
    // free it if it has been rendered
    if (m_Gb)
    {
        m_Gb->Release();
        m_Gb = NULL;

        hRes = m_Fg->QueryInterface(IID_IGraphBuilder, (LPVOID *)&m_Gb);
        if(FAILED(hRes))
        {
            MessageBeep(0);
            return hRes;
        }
    }

    hRes = StringCchCopy(FileName, NUMELMS(FileName), lpFileName);
    if (SUCCEEDED(hRes))
    {
        hRes = RenderFileToVideoRenderer( m_Gb, FileName, FALSE );
    }

    if(SUCCEEDED(hRes) && m_pMixControl)
    {
        hRes = m_pMixControl->SetAlpha(1, 0.5f);
        g_bSecondFileLoaded = TRUE;
    }

    return hRes;
}


/******************************Public*Routine******************************\
* OpenMovie
*
\**************************************************************************/
HRESULT
CMovie::OpenMovie(
    TCHAR *lpFileName
    )
{
    IUnknown        *pUnk;
    HRESULT         hres;
    WCHAR           FileName[MAX_PATH];

    // Check input string
    if (lpFileName == NULL)
        return E_POINTER;

    hres = StringCchCopyW(FileName, NUMELMS(FileName), lpFileName);

    hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if(hres == S_FALSE)
        CoUninitialize();

    hres = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                            IID_IUnknown, (LPVOID *)&pUnk);

    if(SUCCEEDED(hres))
    {
        m_Mode = MOVIE_OPENED;
        hres = pUnk->QueryInterface(IID_IFilterGraph, (LPVOID *)&m_Fg);
        if(FAILED(hres))
        {
            pUnk->Release();
            return hres;
        }

        hres = AddVideoMixingRendererToFG();
        if(FAILED(hres))
        {
            m_Fg->Release(); m_Fg = NULL;
            return hres;
        }

        hres = pUnk->QueryInterface(IID_IGraphBuilder, (LPVOID *)&m_Gb);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            return hres;
        }

        hres = RenderFileToVideoRenderer( m_Gb, FileName, TRUE);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            m_Gb->Release(); m_Gb = NULL;
            return hres;
        }

        hres = m_Wc->QueryInterface(IID_IVMRMixerControl9, (LPVOID *) &m_pMixControl);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            m_Gb->Release(); m_Gb = NULL;
            m_pMixControl = NULL;
            return hres;
        }

        hres = pUnk->QueryInterface(IID_IMediaControl, (LPVOID *)&m_Mc);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            m_Gb->Release(); m_Gb = NULL;
            return hres;
        }

        //
        // Not being able to get the IMediaEvent interface doesn't
        // necessarly mean that we can't play the graph.
        //
        pUnk->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_Me);
        GetMovieEventHandle();

        pUnk->QueryInterface(IID_IMediaSeeking, (LPVOID *)&m_Ms);
        pUnk->Release();
        return S_OK;
    }
    else
    {
        m_Fg = NULL;
    }

    return hres;
}


/******************************Public*Routine******************************\
* CloseMovie
*
\**************************************************************************/
DWORD
CMovie::CloseMovie(
    )
{
    m_Mode = MOVIE_NOTOPENED;

    if(m_Mc)
    {
        if(m_Me)
        {
            m_MediaEvent = NULL;
            m_Me->Release();
            m_Me = NULL;
        }

        if(m_pMixControl)
        {
            m_pMixControl->Release();
            m_pMixControl = NULL;
        }

        if(m_Ms)
        {
            m_Ms->Release();
            m_Ms = NULL;
        }

        if(m_Wc)
        {
            m_Wc->Release();
            m_Wc = NULL;
        }

        m_Mc->Release();
        m_Mc = NULL;

        if(m_Gb)
        {
            m_Gb->Release();
            m_Gb = NULL;
        }

        if(m_Fg)
        {
            m_Fg->Release();
            m_Fg = NULL;
        }
    }

    CoUninitialize();
    return 0L;
}


/******************************Public*Routine******************************\
* CMovie::GetNativeMovieSize
*
\**************************************************************************/
BOOL
CMovie::GetNativeMovieSize(
    LONG *pcx,
    LONG *pcy
    )
{
    BOOL    bRet = FALSE;
    if(m_Wc)
    {
        bRet = (m_Wc->GetNativeVideoSize(pcx, pcy, NULL, NULL) == S_OK);
    }

    return bRet;
}


/******************************Public*Routine******************************\
* GetMoviePosition
*
\**************************************************************************/
BOOL
CMovie::GetMoviePosition(
    LONG *px,
    LONG *py,
    LONG *pcx,
    LONG *pcy
    )
{
    BOOL    bRet = FALSE;

    if(m_Wc)
    {
        RECT src={0}, dest={0};
        HRESULT hr = m_Wc->GetVideoPosition(&src, &dest);
        *px = dest.left;
        *py = dest.right;
        *pcx = dest.right - dest.left;
        *pcy = dest.bottom - dest.top;
    }

    return bRet;
}

/******************************Public*Routine******************************\
* PutMoviePosition
*
\**************************************************************************/
BOOL
CMovie::PutMoviePosition(
    LONG x,
    LONG y,
    LONG cx,
    LONG cy
    )
{
    BOOL    bRet = FALSE;

    RECT rc;
    SetRect(&rc, x, y, x + cx, y + cy);
    if(m_Wc)
    {
        bRet = (m_Wc->SetVideoPosition(NULL, &rc) == S_OK);
    }

    return bRet;
}


/******************************Public*Routine******************************\
* PlayMovie
*
\**************************************************************************/
BOOL
CMovie::PlayMovie(
    )
{
    REFTIME rt, rtAbs, rtDur;
    HRESULT hr=S_OK;

    rt = GetCurrentPosition();
    rtDur = GetDuration();

    //
    // If we are near the end of the movie seek to the start, otherwise
    // stay where we are.
    //
    rtAbs = rt - rtDur;
    if(rtAbs < (REFTIME)0)
    {
        rtAbs = -rtAbs;
    }

    if(rtAbs < (REFTIME)1)
    {
        SeekToPosition((REFTIME)0,FALSE);
    }

    //
    // Change mode after setting m_Mode but before starting the graph
    //
    m_Mode = MOVIE_PLAYING;
    hr = m_Mc->Run();
    return TRUE;
}


/******************************Public*Routine******************************\
* PauseMovie
*
\**************************************************************************/
BOOL
CMovie::PauseMovie(
    )
{
    m_Mode = MOVIE_PAUSED;

    HRESULT hr = m_Mc->Pause();
    return TRUE;
}


/******************************Public*Routine******************************\
* GetStateMovie
*
\**************************************************************************/

OAFilterState
CMovie::GetStateMovie(
    )
{
    OAFilterState State;

    HRESULT hr = m_Mc->GetState(INFINITE,&State);
    return State;
}


/******************************Public*Routine******************************\
* StopMovie
*
\**************************************************************************/
BOOL
CMovie::StopMovie(
    )
{
    m_Mode = MOVIE_STOPPED;
    HRESULT hr = m_Mc->Stop();
    return TRUE;
}


/******************************Public*Routine******************************\
* StatusMovie
*
\**************************************************************************/
EMovieMode
CMovie::StatusMovie(
    )
{
    if(m_Mc)
    {
        FILTER_STATE    fs;
        HRESULT         hr;

        hr = m_Mc->GetState(100, (OAFilterState *)&fs);

        // Don't know what the state is so just stay at old state.
        if(hr == VFW_S_STATE_INTERMEDIATE)
        {
            return m_Mode;
        }

        switch(fs)
        {
            case State_Stopped:
                m_Mode = MOVIE_STOPPED;
                break;

            case State_Paused:
                m_Mode = MOVIE_PAUSED;
                break;

            case State_Running:
                m_Mode = MOVIE_PLAYING;
                break;
        }
    }

    return m_Mode;
}


/******************************Public*Routine******************************\
* CanMovieFrameStep
*
\**************************************************************************/
BOOL
CMovie::CanMovieFrameStep()
{
    IVideoFrameStep* lpFS;
    HRESULT hr;

    hr = m_Fg->QueryInterface(__uuidof(IVideoFrameStep), (LPVOID *)&lpFS);
    if(SUCCEEDED(hr))
    {
        hr = lpFS->CanStep(0L, NULL);
        lpFS->Release();
    }

    return SUCCEEDED(hr);
}


/******************************Public*Routine******************************\
* FrameStepMovie
*
\**************************************************************************/
BOOL
CMovie::FrameStepMovie()
{
    IVideoFrameStep* lpFS;
    HRESULT hr;

    hr = m_Fg->QueryInterface(__uuidof(IVideoFrameStep), (LPVOID *)&lpFS);
    if(SUCCEEDED(hr))
    {
        FrameStepCount++;

        hr = lpFS->Step(1, NULL);
        lpFS->Release();
    }

    return SUCCEEDED(hr);
}


/******************************Public*Routine******************************\
* GetMediaEventHandle
*
* Returns the IMediaEvent event hamdle for the filter graph iff the
* filter graph exists.
*
\**************************************************************************/
HANDLE
CMovie::GetMovieEventHandle(
    )
{
    HRESULT     hr;

    if(m_Me != NULL)
    {
        if(m_MediaEvent == NULL)
        {
            hr = m_Me->GetEventHandle((OAEVENT *)&m_MediaEvent);
        }
    }
    else
    {
        m_MediaEvent = NULL;
    }

    return m_MediaEvent;
}


/******************************Public*Routine******************************\
* GetMovieEventCode
*
\**************************************************************************/
long
CMovie::GetMovieEventCode()
{
    HRESULT hr;
    long    lEventCode;
    LONG_PTR    lParam1, lParam2;

    if(m_Me != NULL)
    {
        hr = m_Me->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
        if(SUCCEEDED(hr))
        {
            hr = m_Me->FreeEventParams(lEventCode, lParam1, lParam2);
            return lEventCode;
        }
    }

    return 0L;
}


/******************************Public*Routine******************************\
* GetDuration
*
* Returns the duration of the current movie
*
\**************************************************************************/
REFTIME
CMovie::GetDuration()
{
    HRESULT hr;
    LONGLONG Duration;

    // Should we seek using IMediaSelection
    if(m_TimeFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = m_Ms->GetDuration(&Duration);
        if(SUCCEEDED(hr))
        {
            return double(Duration);
        }
    }
    else if(m_Ms != NULL)
    {
        hr = m_Ms->GetDuration(&Duration);
        if(SUCCEEDED(hr))
        {
            return double(Duration) / 10000000;
        }
    }

    return 0;
}


/******************************Public*Routine******************************\
* GetCurrentPosition
*
* Returns the position of the current movie
*
\**************************************************************************/
REFTIME
CMovie::GetCurrentPosition()
{
    REFTIME rt = (REFTIME) 0;
    HRESULT hr;
    LONGLONG Position;

    // Should we return a media position
    if(m_TimeFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = m_Ms->GetPositions(&Position, NULL);
        if(SUCCEEDED(hr))
        {
            return double(Position);
        }
    }
    else if(m_Ms != NULL)
    {
        hr = m_Ms->GetPositions(&Position, NULL);
        if(SUCCEEDED(hr))
        {
            return double(Position) / 10000000;
        }
    }

    return rt;
}


/*****************************Private*Routine******************************\
* SeekToPosition
*
\**************************************************************************/
BOOL
CMovie::SeekToPosition(
    REFTIME rt,
    BOOL bFlushData
    )
{
    HRESULT hr;
    LONGLONG llTime = (LONGLONG)(m_TimeFormat == TIME_FORMAT_MEDIA_TIME ? rt * double(10000000) : rt );

    if(m_Ms != NULL)
    {
        FILTER_STATE fs;
        hr = m_Mc->GetState(100, (OAFilterState *)&fs);

        hr = m_Ms->SetPositions(&llTime, AM_SEEKING_AbsolutePositioning, NULL, 0);

        // This gets new data through to the renderers
        if(fs == State_Stopped && bFlushData)
        {
            hr = m_Mc->Pause();
            hr = m_Mc->GetState(INFINITE, (OAFilterState *)&fs);
            hr = m_Mc->Stop();
        }

        if(SUCCEEDED(hr))
        {
            return TRUE;
        }
    }

    return FALSE;
}


/*****************************Public*Routine******************************\
* FindInterfaceFromFilterGraph
*
\**************************************************************************/
HRESULT
CMovie::FindInterfaceFromFilterGraph(
    REFIID iid, // interface to look for
    LPVOID *lp  // place to return interface pointer in
    )
{
    IEnumFilters* pEF;
    IBaseFilter*  pFilter;

    // Grab an enumerator for the filter graph.
    HRESULT hr = m_Fg->EnumFilters(&pEF);

    if(FAILED(hr))
    {
        return hr;
    }

    // Check out each filter.
    while(pEF->Next(1, &pFilter, NULL) == S_OK)
    {
        hr = pFilter->QueryInterface(iid, lp);
        pFilter->Release();

        if(SUCCEEDED(hr))
        {
            break;
        }
    }

    pEF->Release();

    return hr;
}


/*****************************Public*Routine******************************\
* IsTimeFormatSupported
*
\**************************************************************************/
BOOL
CMovie::IsTimeFormatSupported(GUID Format)
{
    return m_Ms != NULL && m_Ms->IsFormatSupported(&Format) == S_OK;
}


/*****************************Public*Routine******************************\
* IsTimeSupported
*
\**************************************************************************/
BOOL
CMovie::IsTimeSupported()
{
    return m_Ms != NULL && m_Ms->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK;
}


/*****************************Public*Routine******************************\
* GetTimeFormat
*
\**************************************************************************/
GUID
CMovie::GetTimeFormat()
{
    return m_TimeFormat;
}

/*****************************Public*Routine******************************\
* SetTimeFormat
*
\**************************************************************************/
BOOL
CMovie::SetTimeFormat(GUID Format)
{
    HRESULT hr = m_Ms->SetTimeFormat(&Format);
    if(SUCCEEDED(hr))
    {
        m_TimeFormat = Format;
    }
    return SUCCEEDED(hr);
}

/******************************Public*Routine******************************\
* SetFocus
*
\**************************************************************************/
void
CMovie::SetFocus()
{
    if(m_Fg)
    {
        // Tell the resource manager that we are being made active.  This
        // will then cause the sound to switch to us.  This is especially
        // important when playing audio only files as there is no other
        // playback window.
        IResourceManager* pResourceManager;

        HRESULT hr = m_Fg->QueryInterface(IID_IResourceManager, (void**)&pResourceManager);

        if(SUCCEEDED(hr))
        {
            IUnknown* pUnknown;

            hr = m_Fg->QueryInterface(IID_IUnknown, (void**)&pUnknown);

            if(SUCCEEDED(hr))
            {
                hr = pResourceManager->SetFocus(pUnknown);
                pUnknown->Release();
            }

            pResourceManager->Release();
        }
    }
}


/******************************Public*Routine******************************\
* RepaintVideo
*
\**************************************************************************/
BOOL
CMovie::RepaintVideo(
    HWND hwnd,
    HDC hdc
    )
{
    BOOL bRet = FALSE;

    if(m_Wc)
    {
        bRet = (m_Wc->RepaintVideo(hwnd, hdc) == S_OK);
    }

    return bRet;
}


/******************************Public*Routine******************************\
* SetAppImage
*
\**************************************************************************/
HRESULT
CMovie::SetAppImage(
    VMR9AlphaBitmap* lpBmpInfo
    )
{
    IVMRMixerBitmap9* pBmp;

    HRESULT hres = m_Wc->QueryInterface(IID_IVMRMixerBitmap9, (LPVOID *)&pBmp);
    if(SUCCEEDED(hres))
    {
        hres = pBmp->SetAlphaBitmap(lpBmpInfo);
        pBmp->Release();
    }

    return hres;
}


/******************************Public*Routine******************************\
* UpdateAppImage
*
\**************************************************************************/
HRESULT
CMovie::UpdateAppImage(VMR9AlphaBitmap* lpBmpInfo)
{
    IVMRMixerBitmap9* pBmp;

    if (!m_Wc)
        return S_FALSE;

    HRESULT hres = m_Wc->QueryInterface(IID_IVMRMixerBitmap9, (LPVOID *)&pBmp);
    if(SUCCEEDED(hres))
    {
        hres = pBmp->UpdateAlphaBitmapParameters(lpBmpInfo);
        pBmp->Release();
    }

    return hres;
}


/*****************************Private*Routine******************************\
* SetBorderClr
*
\**************************************************************************/
void
CMovie::SetBorderClr(COLORREF clr)
{
    m_Wc->SetBorderColor(clr);
}



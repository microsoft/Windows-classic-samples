//------------------------------------------------------------------------------
// File: Scope.cpp
//
// Desc: DirectShow sample code - illustration of an audio oscilloscope.  It
//       shows the waveform graphically as the audio is received by the filter.
//       The filter is a renderer that can be placed wherever the normal
//       runtime renderer goes.  It has a single input pin that accepts a 
//       number of different audio formats and renders the data appropriately.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Summary
//
// This is an audio oscilloscope renderer - we are basically an audio renderer.
// When we are created we also create a class to handle the scope window
// whose constructor creates a worker thread; when it is destroyed it will
// also terminate the worker thread. On that worker thread a window is handled
// that shows the audio waveform for data sent to us. The data is kept
// in a circular buffer that loops when sufficient data has been received.
// We support a number of different audio formats such as 8-bit mode and stereo.
//
//
// Demonstration Instructions
//
// (To really sure of this demonstration the machine must have a sound card)
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// an MPEG, AVI or MOV file into the tool and it will be rendered. Then go to
// the filters in the graph and find the filter (box) titled "Audio Renderer"
// This is the filter we will be replacing with this oscilloscope renderer.
// Then click on the box and hit DELETE. After that go to the Graph menu and
// select "Insert Filters", from the dialog box that pops up find and select
// "Oscilloscope", then dismiss the dialog. Back in the graph layout find the
// output pin of the filter that was connected to the input of the audio
// renderer you just deleted, right click and select "Render". You should
// see it being connected to the input pin of the oscilloscope you inserted
//
// Click Run on GraphEdit and you'll see a waveform for the audio soundtrack...
//
//
// Files
//
// resource.h           Microsoft Visual C++ generated file
// scope.cpp            The main filter and window implementations
// scope.def            What APIs the DLL imports and exports
// scope.h              Window and filter class definitions
// scope.mak            Visual C++ generated makefile
// scope.rc             Dialog box template for our window
//
//
// Base classes we use
//
// CBaseInputPin        A generic input pin we use for the filter
// CCritSec             A wrapper class around a critical section
// CBaseFilter          The generic DirectShow filter object
//
//

#include <streams.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <initguid.h>
#include <wxdebug.h>
#include "scope.h"
#include "resource.h"
#include <strsafe.h>

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Audio,           // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};


const AMOVIESETUP_PIN sudPins  =
{
    L"Input",                   // Pin string name
    FALSE,                      // Is it rendered
    FALSE,                      // Is it an output
    FALSE,                      // Allowed zero pins
    FALSE,                      // Allowed many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of pins types
    &sudPinTypes } ;            // Pin information


const AMOVIESETUP_FILTER sudScope =
{
    &CLSID_Scope,               // Filter CLSID
    L"Oscilloscope",            // String name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};


// List of class IDs and creator functions for class factory

CFactoryTemplate g_Templates []  = {
    { L"Oscilloscope"
    , &CLSID_Scope
    , CScopeFilter::CreateInstance
    , NULL
    , &sudScope }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



//
// CreateInstance
//
// This goes in the factory template table to create new instances
//
CUnknown * WINAPI CScopeFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CScopeFilter(pUnk, phr);

} // CreateInstance


//
// Constructor
//
// Create the filter, scope window, and input pin
//
#pragma warning(disable:4355 4127)

CScopeFilter::CScopeFilter(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseFilter(NAME("Oscilloscope"), pUnk, (CCritSec *) this, CLSID_Scope),
    m_Window(NAME("Oscilloscope"), this, phr)
{
    ASSERT(phr);

    // Create the single input pin
    m_pInputPin = new CScopeInputPin(this,phr,L"Scope Input Pin");
    if(m_pInputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

} // (Constructor)


//
// Destructor
//
CScopeFilter::~CScopeFilter()
{
    // Delete the contained interfaces

    ASSERT(m_pInputPin);
    delete m_pInputPin;
    m_pInputPin = NULL;

} // (Destructor)


//
// GetPinCount
//
// Return the number of input pins we support
//
int CScopeFilter::GetPinCount()
{
    return 1;

} // GetPinCount


//
// GetPin
//
// Return our single input pin - not addrefed
//
CBasePin *CScopeFilter::GetPin(int n)
{
    // We only support one input pin and it is numbered zero

    ASSERT(n == 0);
    if(n != 0)
    {
        return NULL;
    }

    return m_pInputPin;

} // GetPin


//
// JoinFilterGraph
//
// Show our window when we join a filter graph
//   - and hide it when we are annexed from it
//
STDMETHODIMP CScopeFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if(FAILED(hr))
    {
        return hr;
    }

    // Hide or show the scope as appropriate

    if(pGraph == NULL)
    {
        m_Window.InactivateWindow();
    }
    else
    {
        m_Window.ActivateWindow();
    }

    return hr;

} // JoinFilterGraph


//
// Stop
//
// Switch the filter into stopped mode.
//
STDMETHODIMP CScopeFilter::Stop()
{
    CAutoLock lock(this);

    if(m_State != State_Stopped)
    {
        // Pause the device if we were running
        if(m_State == State_Running)
        {
            HRESULT hr = Pause();
            if(FAILED(hr))
            {
                return hr;
            }
        }

        DbgLog((LOG_TRACE,1,TEXT("Stopping....")));

        // Base class changes state and tells pin to go to inactive
        // the pin Inactive method will decommit our allocator which
        // we need to do before closing the device

        HRESULT hr = CBaseFilter::Stop();
        if(FAILED(hr))
        {
            return hr;
        }
    }

    return NOERROR;

} // Stop


//
// Pause
//
// Override Pause to stop the window streaming
//
STDMETHODIMP CScopeFilter::Pause()
{
    CAutoLock lock(this);

    // Check we can PAUSE given our current state

    if(m_State == State_Running)
    {
        m_Window.StopStreaming();
    }

    // tell the pin to go inactive and change state
    return CBaseFilter::Pause();

} // Pause


//
// Run
//
// Overriden to start the window streaming
//
STDMETHODIMP CScopeFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(this);
    HRESULT hr = NOERROR;
    FILTER_STATE fsOld = m_State;

    // This will call Pause if currently stopped

    hr = CBaseFilter::Run(tStart);
    if(FAILED(hr))
    {
        return hr;
    }

    m_Window.ActivateWindow();

    if(fsOld != State_Running)
    {
        m_Window.StartStreaming();
    }

    return NOERROR;

} // Run


//
// Constructor
//
CScopeInputPin::CScopeInputPin(CScopeFilter *pFilter,
                               HRESULT *phr,
                               LPCWSTR pPinName) :
    CBaseInputPin(NAME("Scope Input Pin"), pFilter, pFilter, phr, pPinName)
{
    m_pFilter = pFilter;

} // (Constructor)


//
// Destructor does nothing
//
CScopeInputPin::~CScopeInputPin()
{
} // (Destructor)


//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and allows us to reset the connection media type to be invalid so that
// we can always use that to determine whether we are connected or not. We
// leave the format block alone as it will be reallocated if we get another
// connection or alternatively be deleted if the filter is finally released
//
HRESULT CScopeInputPin::BreakConnect()
{
    // Check we have a valid connection

    if(m_mt.IsValid() == FALSE)
    {
        // Don't return an error here, because it could lead to 
        // ASSERT failures when rendering media files in GraphEdit.
        return S_FALSE;
    }

    m_pFilter->Stop();

    // Reset the CLSIDs of the connected media type

    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);
    return CBaseInputPin::BreakConnect();

} // BreakConnect


//
// CheckMediaType
//
// Check that we can support a given proposed type
//
HRESULT CScopeInputPin::CheckMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->Format();

    if(pwfx == NULL)
        return E_INVALIDARG;

    // Reject non-PCM Audio type

    if(pmt->majortype != MEDIATYPE_Audio)
    {
        return E_INVALIDARG;
    }

    if(pmt->formattype != FORMAT_WaveFormatEx)
    {
        return E_INVALIDARG;
    }

    if(pwfx->wFormatTag != WAVE_FORMAT_PCM)
    {
        return E_INVALIDARG;
    }

    return NOERROR;

} // CheckMediaType


//
// SetMediaType
//
// Actually set the format of the input pin
//
HRESULT CScopeInputPin::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);
    CAutoLock lock(m_pFilter);

    // Pass the call up to my base class

    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    if(SUCCEEDED(hr))
    {
        WAVEFORMATEX *pwf = (WAVEFORMATEX *) pmt->Format();

        m_pFilter->m_Window.m_nChannels = pwf->nChannels;
        m_pFilter->m_Window.m_nSamplesPerSec = pwf->nSamplesPerSec;
        m_pFilter->m_Window.m_nBitsPerSample = pwf->wBitsPerSample;
        m_pFilter->m_Window.m_nBlockAlign = pwf->nBlockAlign;

        m_pFilter->m_Window.m_MaxValue = 128;
        m_pFilter->m_Window.m_nIndex = 0;

        if(!m_pFilter->m_Window.AllocWaveBuffers())
            return E_FAIL;

        // Reset the horizontal scroll bar
        m_pFilter->m_Window.SetHorizScrollRange(m_pFilter->m_Window.m_hwndDlg);
    }

    return hr;

} // SetMediaType


//
// Active
//
// Implements the remaining IMemInputPin virtual methods
//
HRESULT CScopeInputPin::Active(void)
{
    return NOERROR;

} // Active


//
// Inactive
//
// Called when the filter is stopped
//
HRESULT CScopeInputPin::Inactive(void)
{
    return NOERROR;

} // Inactive


//
// Receive
//
// Here's the next block of data from the stream
//
HRESULT CScopeInputPin::Receive(IMediaSample * pSample)
{
    // Lock this with the filter-wide lock
    CAutoLock lock(m_pFilter);

    // If we're stopped, then reject this call
    // (the filter graph may be in mid-change)
    if(m_pFilter->m_State == State_Stopped)
    {
        return E_FAIL;
    }

    // Check all is well with the base class
    HRESULT hr = CBaseInputPin::Receive(pSample);
    if(FAILED(hr))
    {
        return hr;
    }

    // Send the sample to the video window object for rendering
    return m_pFilter->m_Window.Receive(pSample);

} // Receive


//
// CScopeWindow Constructor
//
CScopeWindow::CScopeWindow(TCHAR *pName, CScopeFilter *pRenderer,HRESULT *phr) :
    m_hInstance(g_hInst),
    m_pRenderer(pRenderer),
    m_hThread(INVALID_HANDLE_VALUE),
    m_ThreadID(0),
    m_hwndDlg(NULL),
    m_hwnd(NULL),
    m_pPoints1(NULL),
    m_pPoints2(NULL),
    m_nPoints(0),
    m_bStreaming(FALSE),
    m_bActivated(FALSE),
    m_LastMediaSampleSize(0)
{
    // Create a thread to look after the window

    ASSERT(m_pRenderer);

    m_hThread = CreateThread(NULL,                  // Security attributes
                             (DWORD) 0,             // Initial stack size
                             WindowMessageLoop,     // Thread start address
                             (LPVOID) this,         // Thread parameter
                             (DWORD) 0,             // Creation flags
                             &m_ThreadID);          // Thread identifier

    // If we couldn't create a thread the whole thing's off

    ASSERT(m_hThread);
    if(m_hThread == NULL)
    {
        *phr = E_FAIL;
        return;
    }

    // Wait until the window has been initialised
    m_SyncWorker.Wait();

} // (Constructor)


//
// Destructor
//
CScopeWindow::~CScopeWindow()
{
    // Ensure we stop streaming and release any samples

    StopStreaming();
    InactivateWindow();

    // Tell the thread to destroy the window
    SendMessage(m_hwndDlg, WM_GOODBYE, (WPARAM)0, (LPARAM)0);

    // Make sure it has finished

    ASSERT(m_hThread != NULL);
    WaitForSingleObject(m_hThread,INFINITE);
    CloseHandle(m_hThread);

    if(m_pPoints1 != NULL) delete [] m_pPoints1;
    if(m_pPoints2 != NULL) delete [] m_pPoints2;

} // (Destructor)


//
// ResetStreamingTimes
//
// This resets the latest sample stream times
//
HRESULT CScopeWindow::ResetStreamingTimes()
{
    m_StartSample = 0;
    m_EndSample = 0;

    return NOERROR;

} // ResetStreamingTimes


//
// StartStreaming
//
// This is called when we start running state
//
HRESULT CScopeWindow::StartStreaming()
{
    CAutoLock cAutoLock(this);

    // Are we already streaming

    if(m_bStreaming == TRUE)
    {
        return NOERROR;
    }

    m_bStreaming = TRUE;
    return NOERROR;

} // StartStreaming


//
// StopStreaming
//
// This is called when we stop streaming
//
HRESULT CScopeWindow::StopStreaming()
{
    CAutoLock cAutoLock(this);

    // Have we been stopped already

    if(m_bStreaming == FALSE)
    {
        return NOERROR;
    }

    m_bStreaming = FALSE;
    return NOERROR;

} // StopStreaming


//
// InactivateWindow
//
// Called at the end to put the window in an inactive state
//
HRESULT CScopeWindow::InactivateWindow()
{
    // Has the window been activated
    if(m_bActivated == FALSE)
    {
        return S_FALSE;
    }

    // Now hide the scope window

    ShowWindow(m_hwndDlg,SW_HIDE);
    m_bActivated = FALSE;

    return NOERROR;

} // InactivateWindow


//
// ActivateWindow
//
// Show the scope window
//
HRESULT CScopeWindow::ActivateWindow()
{
    // Has the window been activated
    if(m_bActivated == TRUE)
    {
        return S_FALSE;
    }

    m_bActivated = TRUE;
    ASSERT(m_bStreaming == FALSE);

    ShowWindow(m_hwndDlg,SW_SHOWNORMAL);
    return NOERROR;

} // ActivateWindow


//
// OnClose
//
// This function handles the WM_CLOSE message
//
BOOL CScopeWindow::OnClose()
{
    InactivateWindow();
    return TRUE;

} // OnClose


typedef struct GainEntry_tag
{
    double GainValue;
    TCHAR GainText[8];
} GainEntry;

GainEntry GainEntries[] =
{
    128.,  TEXT("*128"),
    64.,   TEXT("*64"),
    32.,   TEXT("*32"),
    16.,   TEXT("*16"),
    8.,    TEXT("*8"),
    4.,    TEXT("*4"),
    2.,    TEXT("*2"),
    1.,    TEXT("*1"),
    1./2,  TEXT("/2"),
    1./4,  TEXT("/4"),
    1./8,  TEXT("/8"),
    1./16, TEXT("/16"),
    1./32, TEXT("/32"),
    1./64, TEXT("/64"),
    1./128,TEXT("/128"),
    1./256,TEXT("/256"),
};


#define N_GAINENTRIES (sizeof(GainEntries) / sizeof (GainEntries[0]))
#define GAIN_DEFAULT_INDEX 7

typedef struct TBEntry_tag
{
    int TBDivisor;
    TCHAR TBText[16];

} TBEntry;

TBEntry Timebases[] =
{
    10000,  TEXT("10 uS/Div"),
     5000,  TEXT("20 uS/Div"),
     2000,  TEXT("50 uS/Div"),
     1000,  TEXT("100 uS/Div"),
      500,  TEXT("200 uS/Div"),
      200,  TEXT("500 uS/Div"),
      100,  TEXT("1 mS/Div"),
       50,  TEXT("2 mS/Div"),
       20,  TEXT("5 mS/Div"),
       10,  TEXT("10 mS/Div"),
        5,  TEXT("20 mS/Div"),
        2,  TEXT("50 mS/Div"),
        1,  TEXT("100 mS/Div")
};

#define N_TIMEBASES (sizeof(Timebases) / sizeof (Timebases[0]))
#define TIMEBASE_DEFAULT_INDEX 9


//
// SetControlRanges
//
// Set the scroll ranges for all of the vertical trackbars
//
void CScopeWindow::SetControlRanges(HWND hDlg)
{
    SendMessage(m_hwndLGain, TBM_SETRANGE, TRUE, MAKELONG(0, N_GAINENTRIES - 1));
    SendMessage(m_hwndLGain, TBM_SETPOS, TRUE, (LPARAM) GAIN_DEFAULT_INDEX);
    SetDlgItemText(hDlg, IDC_L_GAIN_TEXT, GainEntries[m_LGain].GainText);

    SendMessage(m_hwndLOffset, TBM_SETRANGE, TRUE, MAKELONG(0, m_Height - 1));
    SendMessage(m_hwndLOffset, TBM_SETPOS, TRUE, (LPARAM) m_Height / 2);
    SetDlgItemInt(hDlg, IDC_L_OFFSET_TEXT, -m_LOffset, TRUE);

    SendMessage(m_hwndRGain, TBM_SETRANGE, TRUE, MAKELONG(0, N_GAINENTRIES - 1));
    SendMessage(m_hwndRGain, TBM_SETPOS, TRUE, (LPARAM) GAIN_DEFAULT_INDEX);
    SetDlgItemText(hDlg, IDC_R_GAIN_TEXT, GainEntries[m_RGain].GainText);

    SendMessage(m_hwndROffset, TBM_SETRANGE, TRUE, MAKELONG(0, m_Height - 1));
    SendMessage(m_hwndROffset, TBM_SETPOS, TRUE, (LPARAM) m_Height / 2);
    SetDlgItemInt(hDlg, IDC_R_OFFSET_TEXT, -m_ROffset, TRUE);

    SendMessage(m_hwndTimebase, TBM_SETRANGE, TRUE, MAKELONG(0, N_TIMEBASES - 1));
    SendMessage(m_hwndTimebase, TBM_SETPOS, TRUE, (LPARAM) m_nTimebase);
    SetDlgItemText(hDlg, IDC_TIMEBASE_TEXT, Timebases[m_nTimebase].TBText);

} // SetControlRanges


//
// SetHorizScrollRange
//
// The horizontal scrollbar handles scrolling through the 1 second circular buffer
//
void CScopeWindow::SetHorizScrollRange(HWND hDlg)
{
    SendMessage(m_hwndTBScroll, TBM_SETRANGE, TRUE, MAKELONG(0, (m_nPoints - 1) / 2));
    SendMessage(m_hwndTBScroll, TBM_SETPOS, TRUE, (LPARAM) (m_nPoints - 1) / 2);

    m_TBScroll = m_nPoints - 1;

    TCHAR szFormat[80];

    switch(m_nBitsPerSample + m_nChannels)
    {
        case 9:
            // Mono, 8-bit
            (void)StringCchCopy(szFormat, NUMELMS(szFormat), TEXT("M-8-\0"));
            break;

        case 10:
            // Stereo, 8-bit
            (void)StringCchCopy(szFormat, NUMELMS(szFormat), TEXT("S-8-\0"));
            break;

        case 17:
            // Mono, 16-bit
            (void)StringCchCopy(szFormat, NUMELMS(szFormat), TEXT("M-16-\0"));
            break;

        case 18:
            // Stereo, 16-bit
            (void)StringCchCopy(szFormat, NUMELMS(szFormat), TEXT("S-16-\0"));
            break;

        default:
            (void)StringCchCopy(szFormat, NUMELMS(szFormat),  TEXT(" \0"));
            SetDlgItemText(hDlg, IDC_FORMAT, szFormat);
            return;

    } // End of format switch

    TCHAR szSamplingFreq[80];
    (void)StringCchPrintf(szSamplingFreq, NUMELMS(szSamplingFreq), TEXT("%d\0"), m_nSamplesPerSec);
    (void)StringCchCat(szFormat, NUMELMS(szFormat), szSamplingFreq);
    SetDlgItemText(hDlg, IDC_FORMAT, szFormat);

} // SetHorizScrollRange


//
// ProcessHorizScrollCommands
//
// Called when we get a horizontal scroll bar message
//
void CScopeWindow::ProcessHorizScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int pos;
    int command = LOWORD(wParam);

    if (command != TB_ENDTRACK &&
        command != TB_THUMBTRACK &&
        command != TB_LINEDOWN &&
        command != TB_LINEUP &&
        command != TB_PAGEUP &&
        command != TB_PAGEDOWN)
    {
       return;
    }

    ASSERT(IsWindow((HWND) lParam));

    pos = (int) SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

    if((HWND) lParam == m_hwndTBScroll)
    {
        m_TBScroll = ((m_nPoints - 1) / 2 - pos) * 2;
    }

    OnPaint();

} // ProcessHorizScrollCommands


//
// ProcessVertScrollCommands
//
// Called when we get a vertical scroll bar message
//
void CScopeWindow::ProcessVertScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int pos;
    int command = LOWORD(wParam);

    if (command != TB_ENDTRACK &&
        command != TB_THUMBTRACK &&
        command != TB_LINEDOWN &&
        command != TB_LINEUP &&
        command != TB_PAGEUP &&
        command != TB_PAGEDOWN)
    {
        return;
    }

    ASSERT(IsWindow((HWND) lParam));

    pos = (int) SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

    if((HWND) lParam == m_hwndLGain)
    {
        m_LGain = pos;
        SetDlgItemText(hDlg, IDC_L_GAIN_TEXT, GainEntries[m_LGain].GainText);
    }
    else if((HWND) lParam == m_hwndLOffset)
    {
        m_LOffset = pos - m_Height / 2;
        SetDlgItemInt(hDlg, IDC_L_OFFSET_TEXT, -m_LOffset, TRUE);
    }
    else if((HWND) lParam == m_hwndRGain)
    {
        m_RGain = pos;
        SetDlgItemText(hDlg, IDC_R_GAIN_TEXT, GainEntries[m_RGain].GainText);
    }
    else if((HWND) lParam == m_hwndROffset)
    {
        m_ROffset = pos - m_Height / 2;
        SetDlgItemInt(hDlg, IDC_R_OFFSET_TEXT, -m_ROffset, TRUE);
    }
    else if((HWND) lParam == m_hwndTimebase)
    {
        m_nTimebase = pos ;

        if( m_nTimebase < 0 )
        {
            m_nTimebase = 0;
        } 
            
        if( m_nTimebase >= N_TIMEBASES ) 
        {
            m_nTimebase = N_TIMEBASES - 1;
        } 
        
        SetDlgItemText(hDlg, IDC_TIMEBASE_TEXT, Timebases[m_nTimebase].TBText);
    }
    OnPaint();

} // ProcessVertScrollCommands


//
// InitialiseWindow
//
// This is called by the worker window thread after it has created the main
// window and it wants to initialise the rest of the owner objects window
// variables such as the device contexts. We execute this function with the
// critical section still locked.
//
HRESULT CScopeWindow::InitialiseWindow(HWND hDlg)
{
    RECT rR;

    // Initialise the window variables
    m_hwnd = GetDlgItem(hDlg, IDC_SCOPEWINDOW);

    // Quick sanity check
    ASSERT(m_hwnd != NULL);

    m_nTimebase = TIMEBASE_DEFAULT_INDEX;
    m_fTriggerPosZeroCrossing = 1;
    m_fFreeze = 0;

    m_LGain = GAIN_DEFAULT_INDEX;
    m_RGain = GAIN_DEFAULT_INDEX;
    m_LOffset = 0;
    m_ROffset = 0;

    m_TBScroll = 0;

    GetWindowRect(m_hwnd, &rR);
    m_Width = rR.right - rR.left;
    m_Height = rR.bottom - rR.top;

    m_hwndLGain =       GetDlgItem(hDlg, IDC_L_GAIN);
    m_hwndLOffset =     GetDlgItem(hDlg, IDC_L_OFFSET);
    m_hwndLGainText =   GetDlgItem(hDlg, IDC_L_GAIN_TEXT);
    m_hwndLTitle =      GetDlgItem(hDlg, IDC_L_TITLE);

    m_hwndRGain =       GetDlgItem(hDlg, IDC_R_GAIN);
    m_hwndROffset =     GetDlgItem(hDlg, IDC_R_OFFSET);
    m_hwndRGainText =   GetDlgItem(hDlg, IDC_R_GAIN_TEXT);
    m_hwndRTitle =      GetDlgItem(hDlg, IDC_R_TITLE);

    m_hwndTimebase =    GetDlgItem(hDlg, IDC_TIMEBASE);
    m_hwndFreeze =      GetDlgItem(hDlg, IDC_FREEZE);
    m_hwndTBStart =     GetDlgItem(hDlg, IDC_TS_START);
    m_hwndTBEnd   =     GetDlgItem(hDlg, IDC_TS_LAST);
    m_hwndTBDelta =     GetDlgItem(hDlg, IDC_TS_DELTA);
    m_hwndTBScroll =    GetDlgItem(hDlg, IDC_TB_SCROLL);

    SetControlRanges(hDlg);
    SetHorizScrollRange(hDlg);

    CheckDlgButton(hDlg,                   // handle of dialog box
        IDC_FREEZE,                 // button-control identifier
        m_fFreeze);                 // check state

    CheckDlgButton(hDlg,                   // handle of dialog box
        IDC_TRIGGER,            // button-control identifier
        m_fTriggerPosZeroCrossing); // check state

    m_hPen1 = CreatePen(PS_SOLID, 0, RGB(0, 0xff, 0));
    m_hPen2 = CreatePen(PS_SOLID, 0, RGB(0x40, 0x40, 0xff));
    m_hPenTicks = CreatePen(PS_SOLID, 0, RGB(0x80, 0x80, 0x80));
    m_hBrushBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);

    HDC hdc = GetDC(NULL);
    m_hBitmap = CreateCompatibleBitmap(hdc, m_Width, m_Height);
    ReleaseDC(NULL, hdc);
    return NOERROR;

} // InitialiseWindow


//
// UninitialiseWindow
//
// This is called by the worker window thread when it receives a WM_GOODBYE
// message from the window object destructor to delete all the resources we
// allocated during initialisation
//
HRESULT CScopeWindow::UninitialiseWindow()
{
    // Reset the window variables
    DeleteObject(m_hPen1);
    DeleteObject(m_hPen2);
    DeleteObject(m_hPenTicks);
    DeleteObject(m_hBitmap);

    m_hwnd = NULL;
    return NOERROR;

} // UninitialiseWindow


//
// ScopeDlgProc
//
// The Scope window is actually a dialog box, and this is its window proc.
// The only thing tricky about this is that the "this" pointer to the
// CScopeWindow is passed during the WM_INITDIALOG message and is stored
// in the window user data. This lets us access methods in the class
// from within the dialog.
//
BOOL CALLBACK ScopeDlgProc(HWND hDlg,           // Handle of dialog box
                           UINT uMsg,           // Message identifier
                           WPARAM wParam,   // First message parameter
                           LPARAM lParam)   // Second message parameter
{
    CScopeWindow *pScopeWindow;      // Pointer to the owning object
    
    // Get the window long pointer that holds our owner pointer
    pScopeWindow = _GetWindowLongPtr<CScopeWindow*>(hDlg, GWLP_USERDATA);

    switch(uMsg)
    {
        case WM_INITDIALOG:
            pScopeWindow = (CScopeWindow *) lParam;
            _SetWindowLongPtr(hDlg, GWLP_USERDATA, pScopeWindow);
            return TRUE;

        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return TRUE;

                case IDC_FREEZE:
                    pScopeWindow->m_fFreeze =
                    (BOOL) IsDlgButtonChecked(hDlg,IDC_FREEZE);
                    pScopeWindow->DrawWaveform();
                    break;

                case IDC_TRIGGER:
                    pScopeWindow->m_fTriggerPosZeroCrossing =
                    (BOOL) IsDlgButtonChecked(hDlg,IDC_TRIGGER);
                    pScopeWindow->DrawWaveform();
                    break;

                default:
                    break;
            }

        case WM_VSCROLL:
            pScopeWindow->ProcessVertScrollCommands(hDlg, wParam, lParam);
            break;

        case WM_HSCROLL:
            pScopeWindow->ProcessHorizScrollCommands(hDlg, wParam, lParam);
            break;

        case WM_PAINT:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->OnPaint();
            break;

        // We stop WM_CLOSE messages going any further by intercepting them
        // and then setting an abort signal flag in the owning renderer so
        // that it knows the user wants to quit. The renderer can then
        // go about deleting it's interfaces and the window helper object
        // which will eventually cause a WM_DESTROY message to arrive. To
        // make it look as though the window has been immediately closed
        // we hide it and then wait for the renderer to catch us up

        case WM_CLOSE:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->OnClose();
            return (LRESULT) 0;

            // We receive a WM_GOODBYE window message (synchronously) from the
            // window object destructor in which case we do actually destroy
            // the window and complete the process in the WM_DESTROY message

        case WM_GOODBYE:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->UninitialiseWindow();
            PostQuitMessage(FALSE);
            EndDialog(hDlg, 0);
            return (LRESULT) 0;

        default:
            break;
    }

    return (LRESULT) 0;

} // ScopeDlgProc


//
// MessageLoop
//
// This is the standard windows message loop for our worker thread. It sits
// in a normal processing loop dispatching messages until it receives a quit
// message, which may be generated through the owning object's destructor
//
HRESULT CScopeWindow::MessageLoop()
{
    MSG Message;        // Windows message structure
    DWORD dwResult;     // Wait return code value

    HANDLE hWait[] = { (HANDLE) m_RenderEvent };

    // Enter the modified message loop

    while(TRUE)
    {
        // We use this to wait for two different kinds of events, the first
        // are the normal windows messages, the other is an event that will
        // be signaled when a sample is ready

        dwResult = MsgWaitForMultipleObjects((DWORD) 1,     // Number events
            hWait,         // Event handle
            FALSE,         // Wait for either
            INFINITE,      // No timeout
            QS_ALLINPUT);  // All messages

        // Has a sample become ready to render
        if(dwResult == WAIT_OBJECT_0)
        {
            DrawWaveform();
        }

        // Process the thread's window message

        while(PeekMessage(&Message,NULL,(UINT) 0,(UINT) 0,PM_REMOVE))
        {
            // Check for the WM_QUIT message

            if(Message.message == WM_QUIT)
            {
                return NOERROR;
            }

            // Send the message to the window procedure

            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

} // MessageLoop


#pragma warning(disable:4702)

//
// WindowMessageLoop
//
// This creates a window and processes it's messages on a separate thread
//
DWORD __stdcall CScopeWindow::WindowMessageLoop(LPVOID lpvThreadParm)
{
    CScopeWindow *pScopeWindow;     // The owner renderer object

    // Cast the thread parameter to be our owner object
    pScopeWindow = (CScopeWindow *) lpvThreadParm;

    pScopeWindow->m_hwndDlg =
        CreateDialogParam(pScopeWindow->m_hInstance,    // Handle of app instance
        MAKEINTRESOURCE(IDD_SCOPEDIALOG),               // Dialog box template
        NULL,                                           // Handle of owner window
        (DLGPROC) ScopeDlgProc,                         // Address of dialog procedure
        (LPARAM) pScopeWindow                           // Initialization value
        );

    if(pScopeWindow->m_hwndDlg != NULL)
    {
        // Initialise the window, then signal the constructor that it can
        // continue and then unlock the object's critical section and
        // process messages

        pScopeWindow->InitialiseWindow(pScopeWindow->m_hwndDlg);
    }

    pScopeWindow->m_SyncWorker.Set();

    if(pScopeWindow->m_hwndDlg != NULL)
    {
        pScopeWindow->MessageLoop();
    }

    ExitThread(TRUE);
    return TRUE;

} // WindowMessageLoop


#pragma warning(default:4702)


//
// OnPaint
//
// WM_PAINT message
//
BOOL CScopeWindow::OnPaint()
{
    DrawWaveform();
    return TRUE;

} // OnPaint


//
// ClearWindow
//
// Clear the scope to black and draw the center tickmarks
//
void CScopeWindow::ClearWindow(HDC hdc)
{
    int y = m_Height / 2;

    SetMapMode(hdc, MM_TEXT);
    SetWindowOrgEx(hdc, 0, 0, NULL);
    SetViewportOrgEx(hdc, 0, 0, NULL);

    // Paint the entire window black
    PatBlt(hdc,            // Handle of device context
           (INT) 0,        // x-coord of upper-left corner
           (INT) 0,        // y-coord of upper-left corner
           m_Width,        // Width of rectangle to be filled
           m_Height,       // Height of rectangle to be filled
           BLACKNESS);     // Raster operation code

    // Draw the horizontal line
    HPEN hPenOld = (HPEN) SelectObject(hdc, m_hPenTicks);
    MoveToEx(hdc, 0, y, NULL);
    LineTo(hdc, m_Width, y);

    // Draw the tickmarks
    float inc = (float) m_Width / 10;
    int pos, j;
    int TickPoint;

    for(j = 0; j <= 10; j++)
    {
        if(j == 0 || j == 5 || j == 10)
            TickPoint =  m_Height / 15;
        else
            TickPoint = m_Height / 30;

        pos = (int) (j * inc);
        MoveToEx(hdc, pos, y + TickPoint, NULL);
        LineTo(hdc, pos, y - TickPoint);
    }

    SelectObject(hdc, hPenOld);

} // ClearWindow


//
// DrawPartialWaveform
//
// Draw a part of the Oscilloscope waveform - IndexStart and IndexEnd
// are pointers into the m_pPoints array (in LOGICAL COORDINATES)
// while ViewpointStart and ViewpointEnd are in SCREEN COORDINATES
//
void CScopeWindow::DrawPartialWaveform(HDC hdc,
                                       int IndexStart,
                                       int IndexEnd,
                                       int ViewportStart,
                                       int ViewportEnd)
{
    int nPoints = IndexEnd - IndexStart;
    int nViewportWidth = ViewportEnd - ViewportStart;
    ASSERT(IndexStart + nPoints < m_nPoints);

    // Origin at lower left, x increases up, y increases to right
    SetMapMode(hdc, MM_ANISOTROPIC);

    SetWindowOrgEx(hdc, IndexStart, 0, NULL);
    SetWindowExtEx(hdc, nPoints, (int) (m_MaxValue / GainEntries[m_LGain].GainValue), NULL);
    SetViewportExtEx(hdc, nViewportWidth, -m_Height / 2, NULL);
    SetViewportOrgEx(hdc, ViewportStart, m_LOffset + m_Height / 2, NULL);

    HPEN OldPen = (HPEN) SelectObject(hdc, m_hPen1);
    Polyline(hdc, m_pPoints1 + IndexStart, nPoints + 1);
    SelectObject(hdc, OldPen);

    if(m_pPoints2)
    {
        SetWindowOrgEx(hdc, IndexStart, 0, NULL);
        SetWindowExtEx(hdc, nPoints, (int) (m_MaxValue / GainEntries[m_RGain].GainValue), NULL);
        SetViewportExtEx(hdc, nViewportWidth, -m_Height / 2, NULL);
        SetViewportOrgEx(hdc, ViewportStart, m_ROffset + m_Height / 2, NULL);

        HPEN OldPen2 = (HPEN) SelectObject(hdc, m_hPen2);
        Polyline(hdc, m_pPoints2 + IndexStart, nPoints + 1);
        SelectObject(hdc, OldPen2);
    }

} // DrawPartialWaveform


//
// DrawWaveform
//
// Draw the full Oscilloscope waveform
//
void CScopeWindow::DrawWaveform(void)
{
    CAutoLock lock(m_pRenderer);
    TCHAR szT[40];

    if(m_pPoints1 == NULL)
        return;

    HDC hdc = GetWindowDC(m_hwnd);  // WindowDC has clipping region
    HDC hdcT = CreateCompatibleDC(hdc);
    HBITMAP hBitmapOld = (HBITMAP) SelectObject(hdcT, m_hBitmap);

    ClearWindow(hdcT);

    int StartOffset;
    int IndexEdge;
    int IndexStart1=0, IndexEnd1=0;
    int IndexStart2, IndexEnd2;
    int PointsToDisplay, PointsToDisplay1=0, PointsToDisplay2;
    int ViewportBreak=0;
    int OffsetTimeMS;
    BOOL fWraps;                // If segment to display wraps around 0

    PointsToDisplay = m_nPoints / Timebases [m_nTimebase].TBDivisor;

    StartOffset = (m_nIndex - 1) - m_TBScroll;
    if(StartOffset < 0)
        StartOffset += m_nPoints;

    if(m_fTriggerPosZeroCrossing)
    {
        SearchForPosZeroCrossing(StartOffset, &IndexEdge);
        IndexEnd2 = IndexEdge;
    }
    else
    {
        IndexEnd2 = StartOffset;
    }
    IndexStart2 = IndexEnd2 - PointsToDisplay;    // can be negative

    if(IndexEnd2 > m_nIndex)
        OffsetTimeMS = (m_nIndex + (m_nPoints - IndexEnd2)) * 1000 / m_nSamplesPerSec;
    else
        OffsetTimeMS = (m_nIndex - IndexEnd2) * 1000 / m_nSamplesPerSec;

    fWraps = (IndexStart2 < 0);
    if(fWraps)
    {
        IndexStart1 = IndexStart2 + m_nPoints;
        IndexEnd1 = m_nPoints - 1;
        IndexStart2 = 0;

        PointsToDisplay1 = IndexEnd1 - IndexStart1;
    }

    PointsToDisplay2 = IndexEnd2 - IndexStart2;

    if(fWraps)
    {
        ViewportBreak = (int) (m_Width * (float) PointsToDisplay1 / PointsToDisplay);

        // Draw the first section (from the end of the POINT array)
        DrawPartialWaveform(hdcT,
            IndexStart1, IndexEnd1,     // Index start, Index end
            0, ViewportBreak);          // Window start, Window end

        // Draw the second section (from the beginning of the POINT array)
        DrawPartialWaveform(hdcT,
            IndexStart2, IndexEnd2,     // Index start, Index end
            ViewportBreak, m_Width);    // Window start, Window end
    }
    else
    {
        DrawPartialWaveform(hdcT,
            IndexStart2, IndexEnd2,     // Index start, Index end
            0, m_Width);                // Window start, Window end
    }

    SetMapMode(hdcT, MM_TEXT);
    SetWindowOrgEx(hdcT, 0, 0, NULL);
    SetViewportOrgEx(hdcT, 0, 0, NULL);

    BitBlt(hdc,        // Handle of destination device context
           0,          // x-coordinate of upper-left corner
           0,          // y-coordinate of upper-left corner
           m_Width,    // Wwidth of destination rectangle
           m_Height,   // Height of destination rectangle
           hdcT,       // Handle of source device context
           0,          // x-coordinate of source rectangle
           0,          // y-coordinate of source rectangle
           SRCCOPY);   // Raster operation code

    SelectObject(hdcT, hBitmapOld);
    DeleteDC(hdcT);
    GdiFlush();
    ReleaseDC(m_hwnd, hdc);

    // Show the size of the last buffer received
    (void)StringCchPrintf(szT, NUMELMS(szT), TEXT("%d\0"), m_LastMediaSampleSize);
    SetDlgItemText(m_hwndDlg, IDC_BUFSIZE, szT);

    // Show the timestamps
    LONG mSStart;
    LONG mSEnd = m_EndSample.Millisecs();

    CRefTime rt;
    m_pRenderer->StreamTime(rt);

    // Delta is the difference between the last sample received and
    // the current sample playing according to the StreamTime
    LONG mSDelta = mSEnd - rt.Millisecs();
    (void)StringCchPrintf(szT, NUMELMS(szT), TEXT("%d.%d\0"), mSDelta / 1000, abs(mSDelta) % 1000);
    SetDlgItemText(m_hwndDlg, IDC_TS_DELTA, szT);

    // Show the Delta point on the horizontal trackbar as the selection
    if(mSDelta < 1000)
    {
        int SelectStart = m_nPoints - (m_nPoints * mSDelta / 1000);
        SelectStart /= 2;
        int SelectEnd = SelectStart + m_nPoints / 100;
        SendMessage(m_hwndTBScroll, TBM_SETSEL, TRUE, MAKELONG(SelectStart, SelectEnd));
    }
    else
        SendMessage(m_hwndTBScroll, TBM_SETSEL, TRUE, 0L); // hide the selection

    // Display the begin and end times of the sweep
    mSEnd -= OffsetTimeMS;
    mSStart = mSEnd - PointsToDisplay * 1000 / m_nSamplesPerSec;

    (void)StringCchPrintf(szT, NUMELMS(szT), TEXT("%d.%d\0"), mSStart / 1000, abs(mSStart) % 1000);
    SetDlgItemText(m_hwndDlg, IDC_TS_START, szT);

    (void)StringCchPrintf(szT, NUMELMS(szT), TEXT("%d.%d\0"), mSEnd / 1000, abs(mSEnd) % 1000);
    SetDlgItemText(m_hwndDlg, IDC_TS_LAST, szT);

} // DrawWaveform


//
// AllocWaveBuffers
//
// Allocate a 1 second buffer for each channel
// This is only called when the format changes
// Return TRUE if allocations succeed
//
BOOL CScopeWindow::AllocWaveBuffers()
{
    int j;

    if(m_pPoints1) delete [] m_pPoints1;
    if(m_pPoints2) delete [] m_pPoints2;

    m_pPoints1 = NULL;
    m_pPoints2 = NULL;
    m_nPoints = 0;

    m_nPoints = m_nSamplesPerSec;

    m_pPoints1 = new POINT [m_nSamplesPerSec];
    if(m_pPoints1)
    {
        m_nPoints = m_nSamplesPerSec;
        for(j = 0; j < m_nSamplesPerSec; j++)
            m_pPoints1[j].x = j;
    }

    if(m_nChannels == 2)
    {
        m_pPoints2 = new POINT [m_nSamplesPerSec];
        if(m_pPoints2)
            for(j = 0; j < m_nSamplesPerSec; j++)
                m_pPoints2[j].x = j;
    }

    // Return TRUE if allocations succeeded
    ASSERT((m_pPoints1 != NULL) && ((m_nChannels == 2) ? (m_pPoints2 != NULL) : TRUE));
    return ((m_pPoints1 != NULL) && ((m_nChannels == 2) ? (m_pPoints2 != NULL) : TRUE));

} // AllocWaveBuffers


//
// SearchForPosZeroCrossing
//
// Searches backward for a positive going zero crossing in the waveform
//
void CScopeWindow::SearchForPosZeroCrossing(int StartPoint, int * IndexEdge)
{
    if(StartPoint < 0)
        StartPoint = 0;

    int cur, last, j;

    *IndexEdge = StartPoint;

    last = m_pPoints1[StartPoint].y;

    for(j = m_nPoints; j > 0; j--)
    {
        if(--StartPoint < 0)
            StartPoint = m_nPoints - 1;

        cur = m_pPoints1[StartPoint].y;
        if(cur < 0 && last >= 0)
        {
            *IndexEdge = StartPoint;
            break;
        }
        last = cur;
    }

} // SearchForPosZeroCrossing


//
// CopyWaveform
//
// Copy the current MediaSample into a POINT array so we can use GDI
// to paint the waveform.  The POINT array contains a 1 second history
// of the past waveform.  The "Y" values are normalized to a range of
// +128 to -127 within the POINT array.
//
void CScopeWindow::CopyWaveform(IMediaSample *pMediaSample)
{
    BYTE *pWave;                // Pointer to image data
    int  nBytes;
    int  nSamplesPerChan;

    ASSERT(pMediaSample);
    if (!pMediaSample)
        return;

    pMediaSample->GetPointer(&pWave);
    ASSERT(pWave != NULL);

    nBytes = pMediaSample->GetActualDataLength();
    nSamplesPerChan = nBytes / m_nBlockAlign;

    switch(m_nBitsPerSample + m_nChannels)
    {
        BYTE * pb;
        WORD * pw;

        case 9:
        {   // Mono, 8-bit
            pb = pWave;
            while(nSamplesPerChan--)
            {
                m_pPoints1[m_nIndex].y = (int)*pb++ - 127;  // Make zero centered
                if(++m_nIndex == m_nSamplesPerSec)
                    m_nIndex = 0;
            }
            break;
        }

        case 10:
        {   // Stereo, 8-bit
            pb = pWave;
            while(nSamplesPerChan--)
            {
                m_pPoints1[m_nIndex].y = (int)*pb++ - 127; // Make zero centered
                m_pPoints2[m_nIndex].y = (int)*pb++ - 127;
                if(++m_nIndex == m_nSamplesPerSec)
                    m_nIndex = 0;
            }
            break;
        }

        case 17:
        { // Mono, 16-bit
            pw = (WORD *) pWave;
            while(nSamplesPerChan--)
            {
                m_pPoints1[m_nIndex].y = (int) ((short) *pw++) / 256;
                if(++m_nIndex == m_nSamplesPerSec)
                    m_nIndex = 0;
            }
            break;
        }

        case 18:
        { // Stereo, 16-bit
            pw = (WORD *)pWave;
            while(nSamplesPerChan--)
            {
                m_pPoints1[m_nIndex].y = (int) ((short) *pw++) / 256;
                m_pPoints2[m_nIndex].y = (int) ((short) *pw++) / 256;
                if(++m_nIndex == m_nSamplesPerSec)
                    m_nIndex = 0;
            }
            break;
        }

        default:
            ASSERT(0);
            break;

    } // End of format switch

} // CopyWaveform


//
// Receive
//
// Called when the input pin receives another sample.
// Copy the waveform to our circular 1 second buffer
//
HRESULT CScopeWindow::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    CAutoLock cAutoLock(this);
    ASSERT(pSample != NULL);

    // Has our UI been frozen

    if(m_fFreeze)
    {
        return NOERROR;
    }

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime(&tStart,&tStop);

    m_StartSample = tStart;
    m_EndSample   = tStop;

    // Ignore zero-length samples
    if((m_LastMediaSampleSize = pSample->GetActualDataLength()) == 0)
        return NOERROR;

    if(m_bStreaming == TRUE)
    {
        CopyWaveform(pSample);     // Copy data to our circular buffer
        SetEvent(m_RenderEvent);    // Set an event to display the
        // new data on another thread
        return NOERROR;
    }

    return NOERROR;

} // Receive


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterServer
//
// Handles DLL registry
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


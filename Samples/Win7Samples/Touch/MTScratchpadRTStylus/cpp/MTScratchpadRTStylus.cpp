// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTScratchpadRTStylus application.
// Description:
//  Inside the application window, user can draw using multiple fingers
//  at the same time. The trace of each finger is drawn using different
//  color. The primary finger trace is always drawn in black, and the
//  remaining traces are drawn by rotating through the following colors:
//  red, blue, green, magenta, cyan and yellow.
//
// Purpose:
//  This sample demonstrates handling of the multi-touch input inside
//  a Win32 application using Real Time Stylus inking API:
//  - Setting up minimal RTS object configuration that allows the window
//    to be inking-enabled (this is not multi-touch specific);
//    Construction and initialization of RealTimeStylus object and
//    DynamicRenderer object.
//  - Using IRealTimeStylus3 to register an inking-enabled window for multi-touch.
//  - Deriving and initializing synchronous RTS plugin that receives
//    inking notifications that are multi-touch aware.
//  - Handling multi-touch aware pen-down/up/move notifications: changing
//    stroke color on pen-down.
//
// MTScratchpadRTStylus.cpp : Defines the entry point for the application.
//

// Windows header files
#include <windows.h>

// C RunTime header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#define ASSERT assert

// COM header files
#include <ole2.h>

// RealTimeStylus header files
#include <rtscom.h>     // RTS interface and GUID declarations
#include <rtscom_i.c>   // RTS GUID definitions

// Application header files
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                      // Current module instance
WCHAR g_wszTitle[MAX_LOADSTRING];                       // The title bar text
WCHAR g_wszWindowClass[MAX_LOADSTRING];                 // The main window class name
IRealTimeStylus* g_pRealTimeStylus = NULL;              // RTS object
IDynamicRenderer* g_pDynamicRenderer = NULL;            // DynamicRenderer object
IStylusSyncPlugin* g_pSyncEventHandlerRTS = NULL;       // EventHandler object

///////////////////////////////////////////////////////////////////////////////
// RealTimeStylus utilities

// Creates the RealTimeStylus object, attaches it to the window, and
// enables it for multi-touch.
// in:
//      hWnd        handle to device window
// returns:
//      RTS object through IRealTimeStylus interface, or NULL on failure
IRealTimeStylus* CreateRealTimeStylus(HWND hWnd)
{
    // Check input argument
    if (hWnd == NULL)
    {
        ASSERT(hWnd && L"CreateRealTimeStylus: invalid argument hWnd");
        return NULL;
    }

    // Create RTS object
    IRealTimeStylus* pRealTimeStylus = NULL;
    HRESULT hr = CoCreateInstance(CLSID_RealTimeStylus, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pRealTimeStylus));
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to CoCreateInstance of RealTimeStylus");
        return NULL;
    }

    // Attach RTS object to a window
    hr = pRealTimeStylus->put_HWND((HANDLE_PTR)hWnd);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to set window handle");
        pRealTimeStylus->Release();
        return NULL;
    }

    // Register RTS object for receiving multi-touch input.
    IRealTimeStylus3* pRealTimeStylus3 = NULL;
    hr = pRealTimeStylus->QueryInterface(&pRealTimeStylus3);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: cannot access IRealTimeStylus3");
        pRealTimeStylus->Release();
        return NULL;
    }
    hr = pRealTimeStylus3->put_MultiTouchEnabled(TRUE);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to enable multi-touch");
        pRealTimeStylus->Release();
        pRealTimeStylus3->Release();
        return NULL;
    }
    pRealTimeStylus3->Release();

    return pRealTimeStylus;
}

// Creates the DynamicRenderer object and adds it to the RTS object as a synchronous plugin.
// in:
//      pRealTimeStylus         RTS object
// returns:
//      DynamicRenderer object through IDynamicRenderer interface, or NULL on failure
IDynamicRenderer* CreateDynamicRenderer(IRealTimeStylus* pRealTimeStylus)
{
    // Check input argument
    if (pRealTimeStylus == NULL)
    {
        ASSERT(pRealTimeStylus && L"CreateDynamicRenderer: invalid argument RealTimeStylus");
        return NULL;
    }

    // Get window handle from RTS object
    HWND hWnd = NULL;
    HRESULT hr = pRealTimeStylus->get_HWND((HANDLE_PTR*)&hWnd);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateDynamicRenderer: failed to get window handle");
        return NULL;
    }

    // Create DynamicRenderer object
    IDynamicRenderer* pDynamicRenderer = NULL;
    hr = CoCreateInstance(CLSID_DynamicRenderer, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pDynamicRenderer));
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateDynamicRenderer: failed to CoCreateInstance of DynamicRenderer");
        return NULL;
    }

    // Add DynamicRenderer to the RTS object as a synchronous plugin
    IStylusSyncPlugin* pSyncDynamicRenderer = NULL;
    hr = pDynamicRenderer->QueryInterface(&pSyncDynamicRenderer);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateDynamicRenderer: failed to access IStylusSyncPlugin of DynamicRenderer");
        pDynamicRenderer->Release();
        return NULL;
    }

    hr = pRealTimeStylus->AddStylusSyncPlugin(
        0,                      // insert plugin at position 0 in the sync plugin list
        pSyncDynamicRenderer);  // plugin to be inserted - DynamicRenderer
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateDynamicRenderer: failed to add DynamicRenderer to the RealTimeStylus plugins");
        pDynamicRenderer->Release();
        pSyncDynamicRenderer->Release();
        return NULL;
    }

    // Attach DynamicRenderer to the same window RTS object is attached to
    hr = pDynamicRenderer->put_HWND((HANDLE_PTR)hWnd);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CreateDynamicRenderer: failed to set window handle");
        pDynamicRenderer->Release();
        pSyncDynamicRenderer->Release();
        return NULL;
    }

    pSyncDynamicRenderer->Release();

    return pDynamicRenderer;
}

// Turns on RTS and DynamicRenderer object.
// in:
//      pRealTimeStylus         RTS object
//      pDynamicRenderer        DynamicRenderer object
// returns:
//      flag, is initialization successful
bool EnableRealTimeStylus(IRealTimeStylus* pRealTimeStylus, IDynamicRenderer* pDynamicRenderer)
{
    // Check input arguments
    if (pRealTimeStylus == NULL)
    {
        ASSERT(pRealTimeStylus && L"EnableRealTimeStylus: invalid argument RealTimeStylus");
        return NULL;
    }
    if (pDynamicRenderer == NULL)
    {
        ASSERT(pDynamicRenderer && L"EnableRealTimeStylus: invalid argument DynamicRenderer");
        return false;
    }

    // Enable RTS object
    HRESULT hr = pRealTimeStylus->put_Enabled(TRUE);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"EnableRealTimeStylus: failed to enable RealTimeStylus");
        return false;
    }

    // Enable DynamicRenderer
    hr = pDynamicRenderer->put_Enabled(TRUE);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"EnableRealTimeStylus: failed to enable DynamicRenderer");
        return false;
    }

    // Enable DynamicRenderer's auto-redraw feature
    hr = pDynamicRenderer->put_DataCacheEnabled(TRUE);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"EnableRealTimeStylus: failed to enable DynamicRenderer data cache");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// RealTimeStylus notification handlers

// OnPaint notification handler.
// in:
//      pDynamicRenderer        DynamicRenderer object
// returns:
//      success status
bool OnPaintHandler(IDynamicRenderer* pDynamicRenderer)
{
    // Check input arguments
    if (pDynamicRenderer == NULL)
    {
        ASSERT(pDynamicRenderer && L"OnPaintHandler: invalid argument DynamicRenderer");
        return false;
    }

    // Ask DynamicRenderer to redraw itself
    HRESULT hr = pDynamicRenderer->Refresh();
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"OnPaintHandler: failed to refresh DynamicRenderer");
        return false;
    }    

    return true;
}

// Returns color for the newly started stroke.
// in:
//      bPrimaryContact     flag, whether the contact is the primary contact
// returns:
//      COLORREF, color of the stroke
COLORREF GetTouchColor(bool bPrimaryContact)
{
    static int g_iCurrColor = 0;    // Rotating secondary color index
    static COLORREF g_arrColor[] =  // Secondary colors array
    {
        RGB(255, 0, 0),             // Red
        RGB(0, 255, 0),             // Green
        RGB(0, 0, 255),             // Blue
        RGB(0, 255, 255),           // Cyan
        RGB(255, 0, 255),           // Magenta
        RGB(255, 255, 0)            // Yellow
    };

    COLORREF color;
    if (bPrimaryContact)
    {
        // The primary contact is drawn in black.
        color = RGB(0,0,0);         // Black
    }
    else
    {
        // Take current secondary color.
        color = g_arrColor[g_iCurrColor];

        // Move to the next color in the array.
        g_iCurrColor = (g_iCurrColor + 1) % (sizeof(g_arrColor)/sizeof(g_arrColor[0]));
    }

    return color;
}

///////////////////////////////////////////////////////////////////////////////
// Real Time Stylus sync event handler

// Synchronous plugin, notitification receiver that changes pen color.
class CSyncEventHandlerRTS : public IStylusSyncPlugin
{
    CSyncEventHandlerRTS();
    virtual ~CSyncEventHandlerRTS();

    public:
    // Factory method
    static IStylusSyncPlugin* Create(IRealTimeStylus* pRealTimeStylus);

    // IStylusSyncPlugin methods

    // Handled IStylusSyncPlugin methods, they require nontrivial implementation
    STDMETHOD(StylusDown)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPropCountPerPkt, LONG* pPacket, LONG** ppInOutPkt);
    STDMETHOD(StylusUp)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPropCountPerPkt, LONG* pPacket, LONG** ppInOutPkt);
    STDMETHOD(Packets)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPktCount, ULONG cPktBuffLength, LONG* pPackets, ULONG* pcInOutPkts, LONG** ppInOutPkts);
    STDMETHOD(DataInterest)(RealTimeStylusDataInterest* pEventInterest);

    // IStylusSyncPlugin methods with trivial inline implementation, they all return S_OK
    STDMETHOD(RealTimeStylusEnabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(RealTimeStylusDisabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(StylusInRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(StylusOutOfRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(InAirPackets)(IRealTimeStylus*, const StylusInfo*, ULONG, ULONG, LONG*, ULONG*, LONG**) { return S_OK; }
    STDMETHOD(StylusButtonUp)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(StylusButtonDown)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(SystemEvent)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID, SYSTEM_EVENT, SYSTEM_EVENT_DATA) { return S_OK; }
    STDMETHOD(TabletAdded)(IRealTimeStylus*, IInkTablet*) { return S_OK; }
    STDMETHOD(TabletRemoved)(IRealTimeStylus*, LONG) { return S_OK; }
    STDMETHOD(CustomStylusDataAdded)(IRealTimeStylus*, const GUID*, ULONG, const BYTE*) { return S_OK; }
    STDMETHOD(Error)(IRealTimeStylus*, IStylusPlugin*, RealTimeStylusDataInterest, HRESULT, LONG_PTR*) { return S_OK; }
    STDMETHOD(UpdateMapping)(IRealTimeStylus*) { return S_OK; }

    // IUnknown methods
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

private:
    LONG m_cRefCount;                   // COM object reference count
    IUnknown* m_punkFTMarshaller;       // free-threaded marshaller
    int m_nContacts;                    // number of fingers currently in the contact with the touch digitizer
};

// CSyncEventHandlerRTS constructor.
CSyncEventHandlerRTS::CSyncEventHandlerRTS()
:   m_cRefCount(1),
    m_punkFTMarshaller(NULL),
    m_nContacts(0)
{
}

// CSyncEventHandlerRTS destructor.
CSyncEventHandlerRTS::~CSyncEventHandlerRTS()
{
    if (m_punkFTMarshaller != NULL)
    {
        m_punkFTMarshaller->Release();
    }
}

// CSyncEventHandlerRTS factory method: creates new CSyncEventHandlerRTS and adds it to the synchronous
// plugin list of the RTS object.
// in:
//      pRealTimeStylus         RTS object
// returns:
//      CSyncEventHandlerRTS object through IStylusSyncPlugin interface, or NULL on failure
IStylusSyncPlugin* CSyncEventHandlerRTS::Create(IRealTimeStylus* pRealTimeStylus)
{
    // Check input argument
    if (pRealTimeStylus == NULL)
    {
        ASSERT(pRealTimeStylus != NULL && L"CSyncEventHandlerRTS::Create: invalid argument RealTimeStylus");
        return NULL;
    }

    // Instantiate CSyncEventHandlerRTS object
    CSyncEventHandlerRTS* pSyncEventHandlerRTS = new CSyncEventHandlerRTS();
    if (pSyncEventHandlerRTS == NULL)
    {
        ASSERT(pSyncEventHandlerRTS != NULL && L"CSyncEventHandlerRTS::Create: cannot create instance of CSyncEventHandlerRTS");
        return NULL;
    }

    // Create free-threaded marshaller for this object and aggregate it.
    HRESULT hr = CoCreateFreeThreadedMarshaler(pSyncEventHandlerRTS, &pSyncEventHandlerRTS->m_punkFTMarshaller);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CSyncEventHandlerRTS::Create: cannot create free-threaded marshaller");
        pSyncEventHandlerRTS->Release();
        return NULL;
    }

    // Add CSyncEventHandlerRTS object to the list of synchronous plugins in the RTS object.
    hr = pRealTimeStylus->AddStylusSyncPlugin(
        0,                      // insert plugin at position 0 in the sync plugin list
        pSyncEventHandlerRTS);  // plugin to be inserted - event handler CSyncEventHandlerRTS
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CEventHandlerRTS::Create: failed to add CSyncEventHandlerRTS to the RealTimeStylus plugins");
        pSyncEventHandlerRTS->Release();
        return NULL;
    }

    return pSyncEventHandlerRTS;
}

// Pen-down notification.
// Sets the color for the newly started stroke and increments finger-down counter.
// in:
//      piRtsSrc            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPropCountPerPkt    number of properties per packet
//      pPacket             packet data (layout depends on packet description set)
// in/out:
//      ppInOutPkt          modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::StylusDown(
    IRealTimeStylus* /* piRtsSrc */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPropCountPerPkt */,
    LONG* /* pPacket */,
    LONG** /* ppInOutPkt */)
{
    // Get DrawingAttributes of DynamicRenderer
    IInkDrawingAttributes* pDrawingAttributesDynamicRenderer;
    HRESULT hr = g_pDynamicRenderer->get_DrawingAttributes(&pDrawingAttributesDynamicRenderer);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CSyncEventHandlerRTS::StylusDown: failed to get RTS's drawing attributes");        
        return hr;
    }

    // Set new stroke color to the DrawingAttributes of the DynamicRenderer
    // If there are no fingers down, this is a primary contact
    hr = pDrawingAttributesDynamicRenderer->put_Color(GetTouchColor(m_nContacts == 0));
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CSyncEventHandlerRTS::StylusDown: failed to set color");
        pDrawingAttributesDynamicRenderer->Release();
        return hr;
    }

    pDrawingAttributesDynamicRenderer->Release();

    ++m_nContacts;  // Increment finger-down counter

    return S_OK;
}

// Pen-up notification.
// Decrements finger-down counter.
// in:
//      piRtsSrc            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPropCountPerPkt    number of properties per packet
//      pPacket             packet data (layout depends on packet description set)
// in/out:
//      ppInOutPkt          modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::StylusUp(
    IRealTimeStylus* /* piRtsSrc */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPropCountPerPkt */,
    LONG* /* pPacket */,
    LONG** /* ppInOutPkt */)
{
    --m_nContacts;  // Decrement finger-down counter

    return S_OK;
}

// Pen-move notification.
// In this case, does nothing, but likely to be used in a more complex application.
// RTS framework does stroke collection and rendering for us.
// in:
//      piRtsRtp            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPktCount           number of packets
//      cPktBuffLength      pPacket buffer size, in elements, equal to number of packets times number of properties per packet
//      pPackets            packet data (layout depends on packet description set)
// in/out:
//      pcInOutPkts         modified number of packets
//      ppInOutPkts         modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::Packets(
    IRealTimeStylus* /* piSrcRtp */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPktCount */,
    ULONG /* cPktBuffLength */,
    LONG* /* pPackets */,
    ULONG* /* pcInOutPkts */,
    LONG** /* ppInOutPkts */)
{
    return S_OK;    
}

// Defines which handlers are called by the framework. We set the flags for pen-down, pen-up and pen-move.
// in/out:
//      pDataInterest       flags that enable/disable notification handlers
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::DataInterest(RealTimeStylusDataInterest *pDataInterest)
{
    *pDataInterest = (RealTimeStylusDataInterest)(RTSDI_StylusDown | RTSDI_Packets | RTSDI_StylusUp);

    return S_OK;
}

// Increments reference count of the COM object.
// returns:
//      reference count
ULONG CSyncEventHandlerRTS::AddRef()
{
    return InterlockedIncrement(&m_cRefCount);
}

// Decrements reference count of the COM object, and deletes it
// if there are no more references left.
// returns:
//      reference count
ULONG CSyncEventHandlerRTS::Release()
{
    ULONG cNewRefCount = InterlockedDecrement(&m_cRefCount);
    if (cNewRefCount == 0)
    {
        delete this;
    }
    return cNewRefCount;
}

// Returns a pointer to any interface supported by this object.
// If IID_IMarshal interface is requested, delegate the call to the aggregated
// free-threaded marshaller.
// If a valid pointer is returned, COM object reference count is increased.
// returns:
//      pointer to the interface requested, or NULL if the interface is not supported by this object
HRESULT CSyncEventHandlerRTS::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if ((riid == IID_IStylusSyncPlugin) || (riid == IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    else if ((riid == IID_IMarshal) && (m_punkFTMarshaller != NULL))
    {
        return m_punkFTMarshaller->QueryInterface(riid, ppvObj);
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////
// Application framework

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Win32 application main entry point function.
// This function is generated by Visual Studio app wizard; added COM
// initialization and uninitialization.
// in:
//      hInstance       handle of the application instance
//      hPrevInstance   not used, always NULL
//      lpCmdLine       command line for the application, null-terminated string
//      nCmdShow        how to show the window
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPWSTR    /* lpCmdLine */,
                     int       nCmdShow)
{
    MSG msg;

    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return FALSE;
    }

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_wszTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MTSCRATCHPADRTSTYLUS, g_wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization
    if (!InitInstance (hInstance, nCmdShow))
    {
        // Uninitialize COM
        CoUninitialize();
        return FALSE;
    }

    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Uninitialize COM
    CoUninitialize();

    return (int) msg.wParam;
}

// Registers the window class of the application.
// This function is generated by Visual Studio app wizard.
// This function and its usage are only necessary if you want this code
// to be compatible with Win32 systems prior to the 'RegisterClassEx'
// function that was added to Windows 95. It is important to call this function
// so that the application will get 'well formed' small icons associated
// with it.
// in:
//      hInstance       handle to the instance of the application
// returns:
//      class atom that uniquely identifies the window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = g_wszWindowClass;
    wcex.hIconSm        = 0;

    return RegisterClassEx(&wcex);
}

// Saves instance handle and creates main window
// This function is generated by Visual Studio app wizard; added RTS object
// construction and initialization.
// In this function, we save the instance handle in a global variable and
// create and display the main program window.
// in:
//      hInstance       handle to the instance of the application
//      nCmdShow        how to show the window
// returns:
//      flag, succeeded or failed to create the window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    g_hInst = hInstance; // Store instance handle in our global variable

    // Create the application window
    hWnd = CreateWindow(g_wszWindowClass, g_wszTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // Create RTS object
    g_pRealTimeStylus = CreateRealTimeStylus(hWnd);
    if (g_pRealTimeStylus == NULL)
    {
        return FALSE;
    }

    // Create DynamicRenderer object
    g_pDynamicRenderer = CreateDynamicRenderer(g_pRealTimeStylus);
    if (g_pDynamicRenderer == NULL)
    {
        g_pRealTimeStylus->Release();
        g_pRealTimeStylus = NULL;

        return FALSE;
    }

    // Create EventHandler object
    g_pSyncEventHandlerRTS = CSyncEventHandlerRTS::Create(g_pRealTimeStylus);
    if (g_pSyncEventHandlerRTS == NULL)
    {
        g_pRealTimeStylus->Release();
        g_pRealTimeStylus = NULL;

        g_pDynamicRenderer->Release();
        g_pDynamicRenderer = NULL;

        return FALSE;
    }

    // Enable RTS and DynamicRenderer
    if (!EnableRealTimeStylus(g_pRealTimeStylus, g_pDynamicRenderer))
    {
        g_pSyncEventHandlerRTS->Release();
        g_pSyncEventHandlerRTS = NULL;

        g_pRealTimeStylus->Release();
        g_pRealTimeStylus = NULL;

        g_pDynamicRenderer->Release();
        g_pDynamicRenderer = NULL;

        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Processes messages for the main window:
//      WM_COMMAND  - process the application menu
//      WM_PAINT    - Paint the main window
//      WM_DESTROY  - post a quit message and return
// This function is generated by Visual Studio app wizard; added WM_PAINT
// and WM_DESTROY code.
// in:
//      hWnd        window handle
//      message     message code
//      wParam      message parameter (message-specific)
//      lParam      message parameter (message-specific)
// returns:
//      the result of the message processing and depends on the message sent
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_COMMAND:
            wmId = LOWORD(wParam);
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
            OnPaintHandler(g_pDynamicRenderer);
            UNREFERENCED_PARAMETER(hdc);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            // Release COM objects before CoUninitialize
            g_pDynamicRenderer->Release();
            g_pDynamicRenderer = NULL;

            g_pRealTimeStylus->Release();
            g_pRealTimeStylus = NULL;

            g_pSyncEventHandlerRTS->Release();
            g_pSyncEventHandlerRTS = NULL;

            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

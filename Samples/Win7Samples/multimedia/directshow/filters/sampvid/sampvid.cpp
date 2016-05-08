//------------------------------------------------------------------------------
// File: SampVid.cpp
//
// Desc: DirectShow sample code - illustrates a simple video renderer that
//       draws video into a text-shaped window on Windows NT or a simple
//       popup windows on Windows 95.  It shows hwo to use the base video 
//       renderer classes from the DirectX SDK.  A property page is 
//       implemented to allow users to find out quality management details.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Summary
//
// This is a sample DirectShow video renderer - the filter is based on the
// CBaseVideoRenderer base class. The base class handles all the seeking,
// synchronization and quality management necessary for video renderers.
// In particular, we override the DoRenderSample and PrepareRender methods so
// that we can draw images and realize palettes as necessary in Windows.
//
//
// Implementation
//
// The original idea was that the renderer would create a custom window that
// spelled ActiveX, and in the letters the video would be displayed. To create
// a window with a non rectangular clip region like this meant using paths.
// Unfortunately these are only truly supported on WindowsNT, so for Win95 we
// create a simple popup window (ie no system menu nor accellerator boxes).
//
// The renderer supports both IBasicVideo and IVideoWindow, which is achieved
// fairly simply by inheriting our renderer from the CBaseControlVideo and
// CBaseControlWindow base classes. To fully implement these interfaces we
// must then override and implement some more PURE virtual methods such as
// GetVideoFormat and Get/SetSourceRect (which all live in VIDEOTXT.CPP).
//
// Because we are either a simple popup window or a text shaped window we may
// not have a title bar for the user to grab to move the window around. So we
// handle WM_NCHITTEST messages (with HTCLIENT) to effectively enable window
// dragging by clicking on the video client area.
//
// We make heavy use of other base classes, notably the CImageAllocator which
// provides buffers that are really DIBSECTIONs. This enables faster drawing
// of video (and is the same method used by the real runtime renderer). We
// also use CImageDisplay to match up optimal drawing formats and for video
// type checking, CImagePalette for general palette creation and handling, and
// CDrawImage that can be used for general video drawing.
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. 
// Drag and drop an MPEG, AVI or MOV file into the tool and it will be rendered.
// Then go to the filters in the graph and find the filter (box) titled 
// "Video Renderer"
//
// This is the filter we will be replacing with the sample video renderer.
// Then click on the box and hit DELETE. After that go to the Graph menu and
// select "Insert Filters", from the dialog box find and select the "Sample
// Renderer" and then dismiss the dialog. Back in the graph layout find the
// output pin of the filter that was connected to the input of the video
// renderer you just deleted, right click and select "Render". You should
// see it being connected to the input pin of the renderer you just inserted
//
// Click Pause and Run on the GraphEdit frame and you will see the video...
//
//
// Files
//
// sampvid.cpp          Main implementation of the video renderer
// sampvid.def          What APIs the DLL will import and export
// sampvid.h            Class definition of the derived renderer
// sampvid.rc           Dialog box template for our property page
// videotxt.cpp         The code to look after a video window
// vidprop.cpp          The implementation of the property page
// vidprop.h            The class definition for the property page
//
//
// Base classes used
//
// CImageAllocator      A DIBSECTION video image allocator
// CVideoInputPin       IPin and IMemInputPin interfaces
// CImageDisplay        Manages the video display type
// CMediaType           Source connection media type
// CVideoText           Does the actual video rendering
// CImagePalette        Looks after managing a palette
// CDrawImage           Does the actual image drawing
//
//

#include "sampvid.h"

#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

// GUIDs

DEFINE_GUID(CLSID_SampleRenderer,
0x4d4b1600, 0x33ac, 0x11cf, 0xbf, 0x30, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);

DEFINE_GUID(CLSID_SampleQuality,
0xdb76d7f0, 0x97cc, 0x11cf, 0xa0, 0x96, 0x00, 0x80, 0x5f, 0x6c, 0xab, 0x82);


// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,           // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
    L"Input",                   // Name of the pin
    FALSE,                      // Is pin rendered
    FALSE,                      // Is an output pin
    FALSE,                      // Ok for no pins
    FALSE,                      // Allowed many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of pin types
    &sudPinTypes                // Details for pins
};

const AMOVIESETUP_FILTER sudSampVid =
{
    &CLSID_SampleRenderer,      // Filter CLSID
    L"Sample Video Renderer",   // Filter name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
    { L"Sample Video Renderer"
    , &CLSID_SampleRenderer
    , CVideoRenderer::CreateInstance
    , NULL
    , &sudSampVid }
  ,
    { L"Quality Property Page"
    , &CLSID_SampleQuality
    , CQualityProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// CreateInstance
//
// This goes in the factory template table to create new filter instances
//
CUnknown * WINAPI CVideoRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CVideoRenderer(NAME("Sample Video Renderer"),pUnk,phr);

} // CreateInstance


#pragma warning(disable:4355)

//
// Constructor
//
CVideoRenderer::CVideoRenderer(TCHAR *pName,
                               LPUNKNOWN pUnk,
                               HRESULT *phr) :

    CBaseVideoRenderer(CLSID_SampleRenderer,pName,pUnk,phr),
    m_InputPin(NAME("Video Pin"),this,&m_InterfaceLock,phr,L"Input"),
    m_ImageAllocator(this,NAME("Sample video allocator"),phr),
    m_VideoText(NAME("Video properties"),GetOwner(),phr,&m_InterfaceLock,this),
    m_ImagePalette(this,&m_VideoText,&m_DrawImage),
    m_DrawImage(&m_VideoText)
{
    // Store the video input pin
    m_pInputPin = &m_InputPin;

    // Reset the current video size

    m_VideoSize.cx = 0;
    m_VideoSize.cy = 0;

    // Initialise the window and control interfaces

    m_VideoText.SetControlVideoPin(&m_InputPin);
    m_VideoText.SetControlWindowPin(&m_InputPin);

} // (Constructor)


//
// Destructor
//
CVideoRenderer::~CVideoRenderer()
{
    m_pInputPin = NULL;

} // (Destructor)


//
// CheckMediaType
//
// Check the proposed video media type
//
HRESULT CVideoRenderer::CheckMediaType(const CMediaType *pmtIn)
{
    return m_Display.CheckMediaType(pmtIn);

} // CheckMediaType


//
// GetPin
//
// We only support one input pin and it is numbered zero
//
CBasePin *CVideoRenderer::GetPin(int n)
{
    ASSERT(n == 0);
    if (n != 0) {
        return NULL;
    }

    // Assign the input pin if not already done so

    if (m_pInputPin == NULL) {
        m_pInputPin = &m_InputPin;
    }

    return m_pInputPin;

} // GetPin


//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP CVideoRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);

    } else if (riid == IID_IVideoWindow) {
        return m_VideoText.NonDelegatingQueryInterface(riid,ppv);

    } else if (riid == IID_IBasicVideo) {
        return m_VideoText.NonDelegatingQueryInterface(riid,ppv);
    }

    return CBaseVideoRenderer::NonDelegatingQueryInterface(riid,ppv);

} // NonDelegatingQueryInterface


//
// GetPages
//
// Return the CLSIDs for the property pages we support
//
STDMETHODIMP CVideoRenderer::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    pPages->pElems[0] = CLSID_SampleQuality;
    return NOERROR;

} // GetPages


//
// DoRenderSample
//
// Have the drawing object render the current image
//
HRESULT CVideoRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
    return m_DrawImage.DrawImage(pMediaSample);

} // DoRenderSample


//
// PrepareRender
//
// Overriden to realise the palette before drawing. We used to have to realise
// the palette on every frame because we could never be sure of receiving top
// level messages like WM_PALETTECHANGED. However the plug in distributor will
// now make sure we get these but we still have to do this because otherwise
// we may not find the palette being realised before the thread comes to draw

void CVideoRenderer::PrepareRender()
{
    // Realise the palette on this thread
    m_VideoText.DoRealisePalette();

} // PrepareRender


//
// Active
//
// The auto show flag is used to have the window shown automatically when we
// change state. We do this only when moving to paused or running, when there
// is no outstanding EC_USERABORT set and when the window is not already up
// This can be changed through the IVideoWindow interface AutoShow property.
// If the window is not currently visible then we are showing it because of
// a state change to paused or running, in which case there is no point in
// the video window sending an EC_REPAINT as we're getting an image anyway
//
HRESULT CVideoRenderer::Active()
{
    HWND hwnd = m_VideoText.GetWindowHWND();
    NOTE("AutoShowWindow");

    if(m_VideoText.IsAutoShowEnabled() == TRUE)
    {
        if(m_bAbort == FALSE)
        {
            if(IsWindowVisible(hwnd) == FALSE)
            {
                NOTE("Executing AutoShowWindow");
                SetRepaintStatus(FALSE);

                m_VideoText.PerformanceAlignWindow();
                m_VideoText.DoShowWindow(SW_SHOWNORMAL);
                m_VideoText.DoSetWindowForeground(TRUE);
            }
        }
    }

    return CBaseVideoRenderer::Active();

} // Active


//
// SetMediaType
//
// We store a copy of the media type used for the connection in the renderer
// because it is required by many different parts of the running renderer
// This can be called when we come to draw a media sample that has a format
// change with it. We normally delay type changes until they are really due
// for rendering otherwise we will change types too early if the source has
// allocated a queue of samples. In our case this isn't a problem because we
// only ever receive one sample at a time so it's safe to change immediately
//
HRESULT CVideoRenderer::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    HRESULT hr = NOERROR;
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    CMediaType StoreFormat(m_mtIn);

    // Fill out the optional fields in the VIDEOINFOHEADER

    m_mtIn = *pmt;
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_mtIn.Format();
    m_Display.UpdateFormat(pVideoInfo);

    // We set the new palette before completing so that the method can look
    // at the old RGB colours we used and compare them with the new set, if
    // they're all identical colours we don't need to change the palette

    hr = m_ImagePalette.PreparePalette(&m_mtIn,&StoreFormat,NULL);
    if (FAILED(hr)) {
        return hr;
    }

    // Complete the initialisation

    m_DrawImage.NotifyMediaType(&m_mtIn);
    m_ImageAllocator.NotifyMediaType(&m_mtIn);
    return NOERROR;

} // SetMediaType


//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and lets us to reset the connection flag held by the base class renderer
// The filter object may be hanging onto an image to use for refreshing the
// video window so that must be freed (the allocator decommit may be waiting
// for that image to return before completing) then we must also uninstall
// any palette we were using, reset anything set with the control interfaces
// then set our overall state back to disconnected ready for the next time

HRESULT CVideoRenderer::BreakConnect()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    // Check we are in a valid state

    HRESULT hr = CBaseVideoRenderer::BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }

    // The window is not used when disconnected
    IPin *pPin = m_InputPin.GetConnected();
    if (pPin) 
        SendNotifyWindow(pPin,NULL);

    // The base class break connect disables us from sending any EC_REPAINT
    // events which is important otherwise when we come down here to remove
    // our palette we end up painting the window again - which in turn sees
    // there is no image to draw and would otherwise send a redundant event

    m_ImagePalette.RemovePalette();
    m_mtIn.ResetFormatBuffer();

    return NOERROR;

} // BreakConnect


//
// CompleteConnect
//
// When we complete connection we need to see if the video has changed sizes
// If it has then we activate the window and reset the source and destination
// rectangles. If the video is the same size then we bomb out early. By doing
// this we make sure that temporary disconnections such as when we go into a
// fullscreen mode do not cause unnecessary property changes. The basic ethos
// is that all properties should be persistent across connections if possible
//
HRESULT CVideoRenderer::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    CBaseVideoRenderer::CompleteConnect(pReceivePin);
    m_DrawImage.ResetPaletteVersion();

    // Has the video size changed between connections

    VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_mtIn.Format();
    if (pVideoInfo->bmiHeader.biWidth == m_VideoSize.cx) 
    {
        if (pVideoInfo->bmiHeader.biHeight == m_VideoSize.cy) 
        {
            return NOERROR;
        }
    }

    // Pass the video window handle upstream
    HWND hwnd = m_VideoText.GetWindowHWND();
    NOTE1("Sending EC_NOTIFY_WINDOW %x",hwnd);

    SendNotifyWindow(pReceivePin,hwnd);

    // Set them for the current video dimensions

    m_DrawImage.SetDrawContext();
    m_VideoSize.cx = pVideoInfo->bmiHeader.biWidth;
    m_VideoSize.cy = pVideoInfo->bmiHeader.biHeight;

    m_VideoText.SetDefaultSourceRect();
    m_VideoText.SetDefaultTargetRect();
    m_VideoText.OnVideoSizeChange();
    m_VideoText.ActivateWindow();

    return NOERROR;

} // CompleteConnect


//
// OnReceiveFirstSample
//
// Use the image just delivered to display a poster frame
//
void CVideoRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);
    
    DoRenderSample(pMediaSample);

} // OnReceiveFirstSample


// Constructor

CVideoInputPin::CVideoInputPin(TCHAR *pObjectName,
                               CVideoRenderer *pRenderer,
                               CCritSec *pInterfaceLock,
                               HRESULT *phr,
                               LPCWSTR pPinName) :

    CRendererInputPin(pRenderer,phr,pPinName),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pInterfaceLock)
{
    ASSERT(m_pRenderer);
    ASSERT(pInterfaceLock);

} // (Constructor)


//
// GetAllocator
//
// This overrides the CBaseInputPin virtual method to return our allocator
// we create to pass shared memory DIB buffers that GDI can directly access
// When NotifyAllocator is called it sets the current allocator in the base
// input pin class (m_pAllocator), this is what GetAllocator should return
// unless it is NULL in which case we return the allocator we would like
//
STDMETHODIMP CVideoInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    // Has an allocator been set yet in the base class

    if (m_pAllocator == NULL) 
    {
        m_pAllocator = &m_pRenderer->m_ImageAllocator;
        m_pAllocator->AddRef();
    }

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;

    return NOERROR;

} // GetAllocator


//
// NotifyAllocator
//
// The COM specification says any two IUnknown pointers to the same object
// should always match which provides a way for us to see if they are using
// our DIB allocator or not. Since we are only really interested in equality
// and our object always hands out the same IMemAllocator interface we can
// just see if the pointers match. If they are we set a flag in the main
// renderer as the window needs to know whether it can do fast rendering
//
STDMETHODIMP
CVideoInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    // Make sure the base class gets a look

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
    if (FAILED(hr))
        return hr;

    // Whose allocator is the source going to use?

    m_pRenderer->m_DrawImage.NotifyAllocator(FALSE);
    if (pAllocator == &m_pRenderer->m_ImageAllocator)
        m_pRenderer->m_DrawImage.NotifyAllocator(TRUE);

    return NOERROR;

} // NotifyAllocator


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterSever
//
// Handle the registration of this filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

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


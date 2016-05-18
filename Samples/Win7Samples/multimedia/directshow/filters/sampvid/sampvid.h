//------------------------------------------------------------------------------
// File: SampVid.h
//
// Desc: DirectShow sample code - header file for simple video renderer
//       application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include "vidprop.h"


#ifndef __SAMPVID__
#define __SAMPVID__

// Forward declarations

class CVideoRenderer;
class CVideoInputPin;
class CControlVideo;
class CVideoText;


// This is the video renderer window it supports IBasicVideo and IVideoWindow
// by inheriting from the CBaseControlWindow and CbaseControlVideo classes.
// Those classes leave a few PURE virtual methods that we have to override to
// complete their implementation such as the handling of source and target
// rectangles. The class also looks after creating the window with a custom
// clip region in the shape of the word ActiveX (only applies to Windows/NT)

class CVideoText : public CBaseControlWindow, public CBaseControlVideo
{
protected:

    CVideoRenderer *m_pRenderer;        // Owning sample renderer object
    SIZE m_Size;                        // Size of the masking bitmap

public:

    CVideoText(TCHAR *pName,                 // Object description
               LPUNKNOWN pUnk,               // Normal COM ownership
               HRESULT *phr,                 // OLE failure code
               CCritSec *pInterfaceLock,     // Main critical section
               CVideoRenderer *pRenderer);   // Delegates locking to

    virtual ~CVideoText();
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    // Pure virtual methods for the IBasicVideo interface

    HRESULT IsDefaultTargetRect();
    HRESULT SetDefaultTargetRect();
    HRESULT SetTargetRect(RECT *pTargetRect);
    HRESULT GetTargetRect(RECT *pTargetRect);
    HRESULT IsDefaultSourceRect();
    HRESULT SetDefaultSourceRect();
    HRESULT SetSourceRect(RECT *pSourceRect);
    HRESULT GetSourceRect(RECT *pSourceRect);
    HRESULT GetStaticImage(long *pBufferSize,long *pDIBImage);

    // Prepare the window with a text region

    void InitRenderer(TCHAR *pStringName);
    HRESULT InitWindowRegion(TCHAR *pStringName);
    HFONT CreateVideoFont();
    RECT GetDefaultRect();
    VIDEOINFOHEADER *GetVideoFormat();

    // Overriden from CBaseWindow return our window and class styles

    LPTSTR GetClassWindowStyles(DWORD *pClassStyles,
                                DWORD *pWindowStyles,
                                DWORD *pWindowStylesEx);

    // Method that gets all the window messages

    LRESULT OnReceiveMessage(HWND hwnd,          // Window handle
                             UINT uMsg,          // Message ID
                             WPARAM wParam,      // First parameter
                             LPARAM lParam);     // Other parameter
}; // CVideoText


// This class supports the renderer input pin. We have to override the base
// class input pin because we provide our own special allocator which hands
// out buffers based on GDI DIBSECTIONs. We have an extra limitation which
// is that we only connect to filters that agree to use our allocator. This
// stops us from connecting to the tee for example. The extra work required
// to use someone elses allocator and select the buffer into a bitmap and
// that into the HDC is not great but would only really confuse this sample

class CVideoInputPin : public CRendererInputPin
{
    CVideoRenderer *m_pRenderer;        // The renderer that owns us
    CCritSec *m_pInterfaceLock;         // Main filter critical section

public:

    // Constructor

    CVideoInputPin(
        TCHAR *pObjectName,             // Object string description
        CVideoRenderer *pRenderer,      // Used to delegate locking
        CCritSec *pInterfaceLock,       // Main critical section
        HRESULT *phr,                   // OLE failure return code
        LPCWSTR pPinName);              // This pins identification

    // Manage our DIBSECTION video allocator
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);

}; // CVideoInputPin


// This is the COM object that represents a simple rendering filter. It
// supports IBaseFilter and IMediaFilter and a single input stream (pin)
// The classes that support these interfaces have nested scope NOTE the
// nested class objects are passed a pointer to their owning renderer
// when they are created but they should not use it during construction

class CVideoRenderer : public ISpecifyPropertyPages, public CBaseVideoRenderer
{
public:

    // Constructor and destructor

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);
    CVideoRenderer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    ~CVideoRenderer();

    // Implement the ISpecifyPropertyPages interface

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
    STDMETHODIMP GetPages(CAUUID *pPages);

    CBasePin *GetPin(int n);

    // Override these from the filter and renderer classes

    void PrepareRender();
    HRESULT Active();
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmtIn);
    HRESULT CheckVideoType(const VIDEOINFO *pDisplay,const VIDEOINFO *pInput);
    HRESULT UpdateFormat(VIDEOINFO *pVideoInfo);
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    void OnReceiveFirstSample(IMediaSample *pMediaSample);

public:

    CImageAllocator m_ImageAllocator;  // Our DIBSECTION allocator
    CVideoInputPin  m_InputPin;        // IPin based interfaces
    CImageDisplay   m_Display;         // Manages the video display type
    CMediaType      m_mtIn;            // Source connection media type
    CVideoText      m_VideoText;       // Does the actual video rendering
    CImagePalette   m_ImagePalette;    // Looks after managing a palette
    CDrawImage      m_DrawImage;       // Does the actual image drawing
    SIZE            m_VideoSize;       // Size of the current video stream

}; // CVideoRenderer

#endif // __SAMPVID__


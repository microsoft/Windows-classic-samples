/////////////////////////////////////////////////////////////////////////////
//
// [!output root].h : Declaration of C[!output Safe_root]
//
// Note: Requires DirectX 8 SDK or later.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////
  
#ifndef __C[!output SAFE_ROOT]_H_
#define __C[!output SAFE_ROOT]_H_

#include "resource.h"
#include <mediaobj.h>       // The IMediaObject header from the DirectX SDK.
#include "wmpservices.h"    // The header containing the WMP interface definitions.
#include "wmprealestate.h"
#include <atlwin.h>
#include <stdio.h>

DEFINE_GUID(MEDIATYPE_[!output SAFE_ROOT], [!output MEDIATYPEID]);

//{[!output CLASSID]}
DEFINE_GUID(CLSID_[!output Safe_root], [!output DEFINEGUID]);

interface __declspec(uuid("{[!output INTERFACEID]}")) I[!output Safe_root] : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE get_color(DWORD *pVal) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_color(DWORD newVal) = 0;
};

// registry location for preferences
const WCHAR kwszPrefsRegKey[] = L"Software\\[!output root]\\Rendering Plugin";
const WCHAR kwszPrefsTextColor[] = L"TextColor";

// Color constants.
const COLORREF rgbRed   =  0x000000FF;
const COLORREF rgbGreen =  0x0000FF00;
const COLORREF rgbBlue  =  0x00FF0000;
const COLORREF rgbBlack =  0x00000000;
const COLORREF rgbWhite =  0x00FFFFFF;

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE C[!output Safe_root] : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<C[!output Safe_root], &CLSID_[!output Safe_root]>,
    public CWindowImpl<C[!output Safe_root]>,
    public ISpecifyPropertyPages,
    public IPropertyBag,
    public IMediaObject,
    public IWMPPluginEnable,
    public IWMPPlugin,
    public IWMPNodeRealEstate,
    public IWMPNodeWindowed,
    public IWMPNodeWindowless,
    public I[!output Safe_root]
{
public:
    C[!output Safe_root]();
    virtual ~C[!output Safe_root]();

DECLARE_REGISTRY_RESOURCEID(IDR_[!output SAFE_ROOT])

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(C[!output Safe_root])
    COM_INTERFACE_ENTRY(I[!output Safe_root])
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IPropertyBag)
    COM_INTERFACE_ENTRY(IMediaObject)
    COM_INTERFACE_ENTRY(IWMPPlugin)
    COM_INTERFACE_ENTRY(IWMPPluginEnable)
    COM_INTERFACE_ENTRY(IWMPNodeRealEstate)
    COM_INTERFACE_ENTRY(IWMPNodeWindowed)
    COM_INTERFACE_ENTRY(IWMPNodeWindowless)
    COM_INTERFACE_ENTRY(IWMPWindowMessageSink)
END_COM_MAP()

BEGIN_MSG_MAP(C[!output Safe_root])
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_RANGE_HANDLER(WM_KEYDOWN, WM_KEYUP, OnPluginWindowMessage)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnPluginWindowMessage)
    MESSAGE_RANGE_HANDLER(WM_MOUSEMOVE, WM_MBUTTONDBLCLK, OnPluginWindowMessage)
    MESSAGE_RANGE_HANDLER(WM_NCMOUSEMOVE, WM_NCMBUTTONDBLCLK, OnPluginWindowMessage)
END_MSG_MAP()

    // CComCoClass Overrides
    HRESULT FinalConstruct();
    void    FinalRelease();

    // I[!output Safe_root] methods
    STDMETHOD(get_color)(DWORD *pVal);
    STDMETHOD(put_color)(DWORD newVal);
    STDMETHOD(LoadResourceImage)(IStream **ppStream);

    // IMediaObject methods
    STDMETHOD( GetStreamCount )( 
                   DWORD *pcInputStreams,
                   DWORD *pcOutputStreams
                   );
    
    STDMETHOD( GetInputStreamInfo )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( GetOutputStreamInfo )( 
                   DWORD dwOutputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( GetInputType )( 
                   DWORD dwInputStreamIndex,
                   DWORD dwTypeIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetOutputType )( 
                   DWORD dwOutputStreamIndex,
                   DWORD dwTypeIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( SetInputType )( 
                   DWORD dwInputStreamIndex,
                   const DMO_MEDIA_TYPE *pmt,
                   DWORD dwFlags
                   );
    
    STDMETHOD( SetOutputType )( 
                   DWORD dwOutputStreamIndex,
                   const DMO_MEDIA_TYPE *pmt,
                   DWORD dwFlags
                   );
    
    STDMETHOD( GetInputCurrentType )( 
                   DWORD dwInputStreamIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetOutputCurrentType )( 
                   DWORD dwOutputStreamIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetInputSizeInfo )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pcbSize,
                   DWORD *pcbMaxLookahead,
                   DWORD *pcbAlignment
                   );
    
    STDMETHOD( GetOutputSizeInfo )( 
                   DWORD dwOutputStreamIndex,
                   DWORD *pcbSize,
                   DWORD *pcbAlignment
                   );
    
    STDMETHOD( GetInputMaxLatency )( 
                   DWORD dwInputStreamIndex,
                   REFERENCE_TIME *prtMaxLatency
                   );
    
    STDMETHOD( SetInputMaxLatency )( 
                   DWORD dwInputStreamIndex,
                   REFERENCE_TIME rtMaxLatency
                   );
    
    STDMETHOD( Flush )( void );
    
    STDMETHOD( Discontinuity )( 
                   DWORD dwInputStreamIndex
                   );
    
    STDMETHOD( AllocateStreamingResources )( void );
    
    STDMETHOD( FreeStreamingResources )( void );
    
    STDMETHOD( GetInputStatus )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( ProcessInput )( 
                   DWORD dwInputStreamIndex,
                   IMediaBuffer *pBuffer,
                   DWORD dwFlags,
                   REFERENCE_TIME rtTimestamp,
                   REFERENCE_TIME rtTimelength
                   );
    
    STDMETHOD( ProcessOutput )( 
                   DWORD dwFlags,
                   DWORD cOutputBufferCount,
                   DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                   DWORD *pdwStatus
                   );

    STDMETHOD( Lock )( LONG bLock );

    // Note: need to override CComObjectRootEx::Lock to avoid
    // ambiguity with IMediaObject::Lock. The override just
    // calls through to the base class implementation.

    // CComObjectRootEx overrides
    void Lock()
    {
        CComObjectRootEx<CComMultiThreadModel>::Lock();
    }

    // IWMPPluginEnable methods
    STDMETHOD( SetEnable )( BOOL fEnable );
    STDMETHOD( GetEnable )( BOOL *pfEnable );

    // ISpecifyPropertyPages methods
    STDMETHOD( GetPages )(CAUUID *pPages);

    // IWMPPlugin methods
    STDMETHOD( Init )( DWORD dwPlaybackContext );
    STDMETHOD( Shutdown )();
    STDMETHOD( GetID )( GUID *pGUID );
    STDMETHOD( GetCaps )( DWORD* pdwFlags );
    STDMETHOD( AdviseWMPServices )( IWMPServices* pWMPServices );
    STDMETHOD( UnAdviseWMPServices )();

    //IWMPNodeRealEstate methods
    STDMETHOD ( GetDesiredSize )(
                    LPSIZE  pSize
                    );

    STDMETHOD ( GetFullScreen )(
                    BOOL*  pfFullScreen
                    );

    STDMETHOD ( GetRects )(
                    RECT*  pSrc,
                    RECT*  pDest,
                    RECT*  pClip
                    );

    STDMETHOD ( GetWindowless )(
                    BOOL*  pfWindowless
                    );

    STDMETHOD ( SetFullScreen )(
                    BOOL  fFullScreen
                    );

    STDMETHOD ( SetRects )(
                    const RECT*  pSrc,
                    const RECT*  pDest,
                    const RECT*  pClip
                    );

    STDMETHOD ( SetWindowless )(
                    BOOL  fWindowless
                    );

    // IWMPNodeWindowed methods
    STDMETHOD ( SetOwnerWindow )(OLE_HWND hwnd);
    STDMETHOD ( GetOwnerWindow )(OLE_HWND *phwnd);

    // IWMPNodeWindowless methods
    STDMETHOD ( OnWindowMessage )(
                    UINT uMsg,
                    WPARAM wparam,
                    LPARAM lparam,
                    LRESULT *plRet,
                    BOOL *pfHandled
                    );

    STDMETHOD ( OnDraw )(
                    OLE_HDC hdc,
                    const RECT *prcDraw
                    );

    // IPropertyBag methods
    STDMETHOD ( Read )(
                    LPCWSTR pwszPropName,
                    VARIANT *pVar,
                    IErrorLog *pErrorLog
                    );

    STDMETHOD ( Write )(
                    LPCWSTR pwszPropName,
                    VARIANT *pVar
                    );
    
private:

    // Render plug-in window methods
    LRESULT OnEraseBackground(
                    UINT nMsg,
                    WPARAM wParam, 
                    LPARAM lParam,
                    BOOL& bHandled
                    );
 
    LRESULT OnPaint(
                    UINT nMsg,
                    WPARAM wParam, 
                    LPARAM lParam,
                    BOOL& bHandled
                    );

    STDMETHOD ( DoRendering )(
                    HDC hDC,
                    const RECT *rc
                    );
    
    STDMETHOD ( CreateRenderWin )();
    STDMETHOD ( DestroyRenderWin )();

    STDMETHOD ( Repaint )( void );

    LRESULT ( OnPluginWindowMessage )(
                    UINT uMsg,
                    WPARAM wparam,
                    LPARAM lparam, 
                    BOOL& bHandled
                    );
    
    STDMETHOD ( MakeBitmapFromData )( HDC hDC );

    CComPtr<IWMPServices>               m_spWMPServices;             // Smart Pointer to WMPServices
    CComPtr<IWMPNodeRealEstateHost>     m_spWMPNodeRealEstatehost;   // Smart Pointer to WMPNodeRealEstateHost
    CComPtr<IWMPNodeWindowedHost>       m_spWMPNodeWindowedhost;     // Smart Pointer to WMPNodeWindowedHost
    CComPtr<IWMPNodeWindowlessHost>     m_spWMPNodeWindowlesshost;   // Smart Pointer to the WMP NodeWindowlesshost

    CComPtr<IMediaBuffer>   m_spInputBuffer;    // Smart pointer to the input buffer object
    DMO_MEDIA_TYPE          m_mtInput;          // Stores the input format structure

    BOOL                    m_bFullScreen;      // Is the Player in full screen mode?

    // Window handles
    HWND                    m_hwndParent;

    // DC handles
    HDC                     m_hdcMem;

    // Rendering RECTs
    RECT                    m_rctSrc;           // The supplied source RECT.
    RECT                    m_rctDest;          // The supplied destination RECT.
    RECT                    m_rctClip;          // The supplied clipping RECT.
    RECT                    m_rctDisplay;       // Calculated video display RECT.
    RECT                    m_rctWindowPos;     // Calculated window position RECT.

    int                     m_widthSrc;         // Calculated source width.
    int                     m_heightSrc;        // Calculated source height.

    COLORREF                m_TextColor;        // Text color.
    BOOL                    m_bEnabled;         // true if enabled
    BOOL                    m_bWindowless;      // Draw using hdc if FALSE
    BOOL                    m_bHosted;          // TRUE if the plug-in window is hosted in Now Playing..
};

#endif //__C[!output Safe_root]_H_

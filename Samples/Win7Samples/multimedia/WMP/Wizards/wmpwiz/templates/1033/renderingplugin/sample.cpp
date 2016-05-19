/////////////////////////////////////////////////////////////////////////////
//
// [!output root].cpp : Implementation of C[!output Safe_root]
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <windowsx.h>

#include "[!output root].h"
#include "[!output Root]PropPage.h"

#include <mediaerr.h>   // DirectX SDK media errors
#include <dmort.h>      // DirectX SDK DMO runtime support
#include <uuids.h>      // DirectX SDK media types and subtyes

[!if VSNET]
#pragma warning( disable : 4312; disable: 4311 )
[!endif]

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::C[!output Safe_root]
//
// Constructor
/////////////////////////////////////////////////////////////////////////////

C[!output Safe_root]::C[!output Safe_root]()
{
    m_bFullScreen       = false;
    m_bEnabled          = true;
    m_hwndParent        = NULL;
    m_bWindowless       = false;
    m_bHosted           = false;
    m_widthSrc          = 0;
    m_heightSrc         = 0;
    m_hdcMem            = NULL;
    m_TextColor         = rgbBlue;
  
    ZeroMemory( &m_rctSrc, sizeof( m_rctSrc ) );
    ZeroMemory( &m_rctDest, sizeof( m_rctDest) );
    ZeroMemory( &m_rctClip, sizeof( m_rctClip ) );
    ZeroMemory( &m_rctDisplay, sizeof( m_rctDisplay ) );
    ZeroMemory( &m_rctWindowPos, sizeof( m_rctWindowPos ) );
    ZeroMemory( &m_mtInput, sizeof( m_mtInput ) );

    SetRect( &m_rctSrc, 0, 0, 300, 300 );
    CopyRect( &m_rctDisplay, &m_rctSrc );
    CopyRect( &m_rctWindowPos, &m_rctSrc );
    m_widthSrc = m_rctSrc.right - m_rctSrc.left;
    m_heightSrc = m_rctSrc.bottom - m_rctSrc.top;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::~C[!output Safe_root]
//
// Destructor
/////////////////////////////////////////////////////////////////////////////

C[!output Safe_root]::~C[!output Safe_root]()
{
    ::MoFreeMediaType(&m_mtInput);
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::FinalConstruct
//
// Called when an plug-in is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::FinalConstruct()
{
    CRegKey key;
    LONG    lResult;
    DWORD   dwValue = 0;
    HRESULT hr = S_OK;

    // Read text color from registry.
    lResult = key.Open(HKEY_CURRENT_USER, kwszPrefsRegKey, KEY_READ);
    if (ERROR_SUCCESS == lResult)
    {
[!if VSNET]
        DWORD dwType = 0;
        ULONG uLength = sizeof(dwValue);
        lResult = key.QueryValue(kwszPrefsTextColor, &dwType, &dwValue, &uLength);
[!else]
        lResult = key.QueryValue(dwValue, kwszPrefsTextColor);
[!endif]
        if (ERROR_SUCCESS == lResult)
        {
            // Assign the value to the member variable.
            m_TextColor = (COLORREF) dwValue;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]:::FinalRelease
//
// Called when an plug-in is unloaded. Use this function to free any
// resources allocated.
/////////////////////////////////////////////////////////////////////////////

void C[!output Safe_root]::FinalRelease()
{
    FreeStreamingResources();  // In case client does not call this.

    if( m_hdcMem )
    {
        DeleteDC(m_hdcMem);
        m_hdcMem = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::CreateRenderWin
// Create window for plug-in
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::CreateRenderWin()
{
    // Create the plug-in window.
    HWND hWnd = Create( GetDesktopWindow(), m_rctSrc, NULL, WS_CHILD );

    if ( NULL == hWnd )
    {
        return E_HANDLE;
    }

    ShowWindow(SW_HIDE);

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::DestroyRenderWin
// Destroy window for plug-in
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::DestroyRenderWin()
{
    if( NULL != m_hWnd )
    {
        DestroyWindow();
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetStreamCount
//
// Implementation of IMediaObject::GetStreamCount
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetStreamCount( 
               DWORD *pcInputStreams,
               DWORD *pcOutputStreams)
{
    HRESULT hr = S_OK;

    if ( NULL == pcInputStreams )
    {
        return E_POINTER;
    }

    if ( NULL == pcOutputStreams )
    {
        return E_POINTER;
    }

    // The plug-in uses one stream in input direction.
    *pcInputStreams = 1;
    *pcOutputStreams = 0;

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputStreamInfo
//
// Implementation of IMediaObject::GetInputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputStreamInfo( 
               DWORD dwInputStreamIndex,
               DWORD *pdwFlags)
{    
    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    // The stream index must be zero.
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    // Use the default input stream configuration (a single stream). 
    *pdwFlags = 0;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputStreamInfo
//
// Implementation of IMediaObject::GetOutputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputStreamInfo( 
               DWORD dwOutputStreamIndex,
               DWORD *pdwFlags)
{
    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    // The stream index must be zero.
    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    // Use the default output stream configuration (a single stream).
    *pdwFlags = 0;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputType
//
// Implementation of IMediaObject::GetInputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputType ( 
               DWORD dwInputStreamIndex,
               DWORD dwTypeIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX ;
    }

    // only support one preferred type
    if ( 0 != dwTypeIndex )
    {
        return DMO_E_NO_MORE_ITEMS;
    }
    else if ( NULL == pmt )
    {
       return S_OK;    
    }

    ::ZeroMemory( pmt, sizeof( DMO_MEDIA_TYPE ) );
    pmt->majortype = MEDIATYPE_[!output SAFE_ROOT];
    pmt->subtype = MEDIATYPE_NULL;

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputType
//
// Implementation of IMediaObject::GetOutputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputType( 
               DWORD dwOutputStreamIndex,
               DWORD dwTypeIndex,
               DMO_MEDIA_TYPE *pmt)
{
    return DMO_E_NO_MORE_ITEMS;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetInputType
//
// Implementation of IMediaObject::SetInputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetInputType( 
               DWORD dwInputStreamIndex,
               const DMO_MEDIA_TYPE *pmt,
               DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( DMO_SET_TYPEF_CLEAR & dwFlags ) 
    {
        ::MoFreeMediaType(&m_mtInput);
        ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));

        return S_OK;
    }

    if ( NULL == pmt )
    {
       return E_POINTER;
    }

    // Validate that the input media type is correct.
    // Note: Your media type may require additional validation.
    if(  MEDIATYPE_[!output SAFE_ROOT] != pmt->majortype ) 
    {
        hr = DMO_E_TYPE_NOT_ACCEPTED;
    }

    if (FAILED(hr))
    {
        if( DMO_SET_TYPEF_TEST_ONLY & dwFlags )
        {
            hr = S_FALSE;
        }
    }
    else if ( 0 == dwFlags )
    {
        // free existing media type
        ::MoFreeMediaType( &m_mtInput );
        ::ZeroMemory( &m_mtInput, sizeof( m_mtInput ) );

        // copy new media type
        hr = ::MoCopyMediaType( &m_mtInput, pmt );
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetOutputType
//
// Implementation of IMediaObject::SetOutputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetOutputType( 
               DWORD dwOutputStreamIndex,
               const DMO_MEDIA_TYPE *pmt,
               DWORD dwFlags)
{ 
    // Rendering plug-ins don't output data.
    return DMO_E_INVALIDSTREAMINDEX;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputCurrentType
//
// Implementation of IMediaObject::GetInputCurrentType
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetInputCurrentType( 
               DWORD dwInputStreamIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pmt )
    {
        return E_POINTER;
    }

    if ( GUID_NULL == m_mtInput.majortype )
    {
        return DMO_E_TYPE_NOT_SET;
    }

    hr = ::MoCopyMediaType( pmt, &m_mtInput );

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputCurrentType
//
// Implementation of IMediaObject::GetOutputCurrentType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputCurrentType( 
               DWORD dwOutputStreamIndex,
               DMO_MEDIA_TYPE *pmt)
{
    // Rendering plug-ins don't output data.
    return DMO_E_INVALIDSTREAMINDEX;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputSizeInfo
//
// Implementation of IMediaObject::GetInputSizeInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputSizeInfo( 
               DWORD dwInputStreamIndex,
               DWORD *pcbSize,
               DWORD *pcbMaxLookahead,
               DWORD *pcbAlignment)
{
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pcbSize )
    {
       return E_POINTER;
    }

    if ( NULL == pcbMaxLookahead )
    {
        return E_POINTER;
    }

    if ( NULL == pcbAlignment )
    {
       return E_POINTER;
    }

    if ( GUID_NULL == m_mtInput.majortype )
    {
        return DMO_E_TYPE_NOT_SET;
    }

    // Return the input sample size, in bytes.
    *pcbSize = m_mtInput.lSampleSize;

    // This plug-in doesn't perform lookahead. Return zero.
    *pcbMaxLookahead = 0;

    // The input stream has no alignment requirement.
    *pcbAlignment = 1;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputSizeInfo
//
// Implementation of IMediaObject::GetOutputSizeInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputSizeInfo( 
               DWORD dwOutputStreamIndex,
               DWORD *pcbSize,
               DWORD *pcbAlignment)
{
    return E_NOTIMPL; // Not dealing with output pin.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputMaxLatency
//
// Implementation of IMediaObject::GetInputMaxLatency
/////////////////////////////////////////////////////////////////////////////
   
STDMETHODIMP C[!output Safe_root]::GetInputMaxLatency( 
               DWORD dwInputStreamIndex,
               REFERENCE_TIME *prtMaxLatency)
{
    return E_NOTIMPL; // Not dealing with latency in this plug-in.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetInputMaxLatency
//
// Implementation of IMediaObject::SetInputMaxLatency
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetInputMaxLatency( 
               DWORD dwInputStreamIndex,
               REFERENCE_TIME rtMaxLatency)
{
    return E_NOTIMPL; // Not dealing with latency in this plug-in.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Flush
//
// Implementation of IMediaObject::Flush
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::Flush( void )
{
    Lock();

    m_spInputBuffer.Release();

    Unlock();

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Discontinuity
//
// Implementation of IMediaObject::Discontinuity
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::Discontinuity( 
               DWORD dwInputStreamIndex)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::AllocateStreamingResources
//
// Implementation of IMediaObject::AllocateStreamingResources
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::AllocateStreamingResources ( void )
{
    // Allocate any buffers need to process the stream. This plug-in does
    // all processing in-place, so it requires no extra buffers.

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::FreeStreamingResources
//
// Implementation of IMediaObject::FreeStreamingResources
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::FreeStreamingResources( void )
{
    Lock();

    m_spInputBuffer.Release();

    Unlock();

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputStatus
//
// Implementation of IMediaObject::GetInputStatus
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputStatus( 
           DWORD dwInputStreamIndex,
           DWORD *pdwFlags)
{ 
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    if ( m_spInputBuffer )
    {
        *pdwFlags = 0; //The buffer still contains data; return zero.
    }
    else
    {
        *pdwFlags = DMO_INPUT_STATUSF_ACCEPT_DATA; // OK to call ProcessInput.
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessInput
//
// Implementation of IMediaObject::ProcessInput
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::ProcessInput( 
               DWORD dwInputStreamIndex,
               IMediaBuffer *pBuffer,
               DWORD dwFlags,
               REFERENCE_TIME rtTimestamp,
               REFERENCE_TIME rtTimelength)
{ 
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pBuffer )
    {
        return E_POINTER;
    }

    if ( GUID_NULL == m_mtInput.majortype )
    {
        return DMO_E_TYPE_NOT_SET;
    }

    if ( m_hWnd &&
         !IsWindowVisible() && 
         m_bHosted  && 
         !m_bWindowless &&
         !m_bFullScreen )
    {
        // Under these circumstances, be sure 
        // that the plug-in window is visible.
        ShowWindow( SW_SHOW );
    }

    // Begin critical section.
    Lock();

    m_spInputBuffer.Release();

    // Hold on to the buffer using a smart pointer.
    m_spInputBuffer = pBuffer;

    //End critical section.
    Unlock();

    if( m_hWnd )
    {
        // Invalidate the client area to render the data in the buffer.
        hr = Repaint();

        if ( FAILED( hr ) )
        {
            return hr;
        }
    }

    return hr;
}
    
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessOutput
//
// Implementation of IMediaObject::ProcessOutput
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::ProcessOutput( 
               DWORD dwFlags,
               DWORD cOutputBufferCount,
               DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
               DWORD *pdwStatus)
{
    // Rendering plug-ins don't wait for ProcessOutput
    // before rendering.
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Lock
//
// Implementation of IMediaObject::Lock
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Lock( LONG bLock )
{
    if( bLock )
    {
        Lock();
    }
    else
    {
        Unlock();
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetEnable
//
// Implementation of IWMPPluginEnable::SetEnable
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::SetEnable( BOOL fEnable )
{
    // This function allows any state or UI associated with the plug-in to reflect the
    // enabled/disable state of the plug-in

    m_bEnabled = fEnable;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetEnable
//
// Implementation of IWMPPluginEnable::GetEnable
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetEnable( BOOL *pfEnable )
{
    if ( NULL == pfEnable )
    {
        return E_POINTER;
    }

    *pfEnable = m_bEnabled;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetPages
//
// Implementation of ISpecifyPropertyPages::GetPages
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetPages( CAUUID *pPages )
{
    if( NULL == pPages )
    {
        return E_POINTER;
    }

    // Only one property page is required for the plug-in.
    pPages->cElems = 1;
    pPages->pElems = ( GUID * ) ( CoTaskMemAlloc( sizeof( GUID ) ) );

    // Make sure memory is allocated for pPages->pElems
    if ( NULL == pPages->pElems )
    {
        return E_OUTOFMEMORY;
    }

    // Return the property page's class ID
    *( pPages->pElems ) = CLSID_[!output Safe_root]PropPage;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Init
//
// Implementation of IWMPPlugin::Init
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Init( DWORD dwPlaybackContext )
{
    // Create the plug-in window using the desktop as parent.
    // If Windows Media Player calls SetOwnerWindow(), the parent
    // will change.
 
    HRESULT hr = CreateRenderWin();
 
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Shutdown
//
// Implementation of IWMPPlugin::Shutdown
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Shutdown()
{
    DestroyRenderWin();

    return S_OK;
}
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetID
//
// Implementation of IWMPPlugin::GetID
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetID( GUID *pGUID )
{
    if( NULL == pGUID)
    {
        return E_POINTER;
    }

    *pGUID = CLSID_[!output Safe_root];

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetCaps
//
// Implementation of IWMPPlugin::GetCaps
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetCaps( DWORD *pdwFlags )
{
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::AdviseWMPServices
//
// Implementation of IWMPPlugin::AdviseWMPServices
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::AdviseWMPServices( IWMPServices *pWMPServices )
{
    HRESULT hr = S_OK;

    if( NULL == pWMPServices )
    {
        return E_POINTER;
    }

    m_spWMPServices = pWMPServices;

    if( m_spWMPServices.p )
    {
        hr = m_spWMPServices->QueryInterface( __uuidof(IWMPNodeRealEstateHost ),( void** )&m_spWMPNodeRealEstatehost );

        if(SUCCEEDED ( hr ) )
        {
            hr = m_spWMPServices->QueryInterface( __uuidof( IWMPNodeWindowedHost ),( void** )&m_spWMPNodeWindowedhost );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = m_spWMPServices->QueryInterface( __uuidof( IWMPNodeWindowlessHost ),( void** )&m_spWMPNodeWindowlesshost );
        }

        if( FAILED ( hr ) )
        {
            m_spWMPServices.Release();

            if( m_spWMPNodeRealEstatehost )
            {
                m_spWMPNodeRealEstatehost.Release();
            }

            if( m_spWMPNodeWindowedhost )
            {
                m_spWMPNodeWindowedhost.Release();
            }

            if( m_spWMPNodeWindowlesshost )
            {
                m_spWMPNodeWindowlesshost.Release();
            }
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::UnAdviseWMPServices
//
// Implementation of IWMPPlugin::UnAdviseWMPServices
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::UnAdviseWMPServices( )
{ 
    // Release the smart pointers.
    m_spWMPServices.Release();
    m_spWMPNodeRealEstatehost.Release();
    m_spWMPNodeWindowedhost.Release();
    m_spWMPNodeWindowlesshost.Release();
  
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetDesiredSize
//
// Implementation of IWMPNodeRealEstate::GetDesiredSize
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetDesiredSize( LPSIZE pSize )
{
    if( NULL == pSize )
    {
        return E_POINTER;
    }

    // Request a 300 x 300 pixels rendering region.
    pSize->cx = 300;
    pSize->cy = 300;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetFullScreen
//
// Implementation of IWMPNodeRealEstate::GetFullScreen
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetFullScreen( BOOL *pfFullScreen )
{
    if( NULL == pfFullScreen )
    {
        return E_POINTER;
    }
    
    *pfFullScreen = m_bFullScreen;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetRects
//
// Implementation of IWMPNodeRealEstate::GetRects
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetRects(  RECT *pSrc,
                                   RECT *pDest,
                                   RECT *pClip)
{
    if( pSrc )
    {
        *pSrc = m_rctSrc;
    }

    if( pDest )
    {
        *pDest = m_rctDest;
    }

    if( pClip )
    {
        *pClip = m_rctClip;
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetWindowless
//
// Implementation of IWMPNodeRealEstate::GetWindowless
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetWindowless(  BOOL*  pfWindowless)
{
    if( NULL == pfWindowless )
    {
        return E_POINTER;
    }

    *pfWindowless = m_bWindowless;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetFullScreen
//
// Implementation of IWMPNodeRealEstate::SetFullScreen
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::SetFullScreen( BOOL fFullScreen )
{
    m_bFullScreen = fFullScreen;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetRects
//
// Implementation of IWMPNodeRealEstate::SetRects
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::SetRects( const RECT *pSrc,
                                  const RECT *pDest,
                                  const RECT *pClip )
{
    if( pSrc )
    {
        m_rctSrc = *pSrc;

        // Cache the source RECT dimensions.
        m_widthSrc = m_rctSrc.right - m_rctSrc.left;
        m_heightSrc = m_rctSrc.bottom - m_rctSrc.top;
    }

    if( pDest )
    {
        m_rctDest = *pDest;
    }

    if( pClip )
    {
        m_rctClip = *pClip;
    }

    if ( IsRectEmpty( pClip ) )
    {
        // Set the clipping RECT equal to the destination RECT.
        m_rctClip = m_rctDest;
    }

    // Calculate the window position RECT.
    IntersectRect( &m_rctWindowPos, &m_rctClip, &m_rctDest );

    // Calculate the display area RECT and move it to the origin.
    m_rctDisplay = m_rctDest;
    OffsetRect( &m_rctDisplay, -m_rctDest.left, -m_rctDest.top );

    // Now move the display area RECT by the offset
    // between the destination RECT and the window position RECT.
    // This puts the display RECT into the correct position.
    long lShiftRight = m_rctDest.left - m_rctWindowPos.left;
    long lShiftDown  = m_rctDest.top - m_rctWindowPos.top;
    OffsetRect( &m_rctDisplay, lShiftRight, lShiftDown );
 
    if( m_hWnd )
    {
        if( !m_bHosted ||
            ( m_bWindowless && !m_bFullScreen ) )
        {
            // Make the plug-in window invisible.
            SetWindowPos( HWND_TOP, &m_rctWindowPos, SWP_HIDEWINDOW );
        }
        else
        {
            // Move the plug-in window to the window position RECT.
            SetWindowPos( HWND_TOP, &m_rctWindowPos, SWP_SHOWWINDOW );
        }
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetWindowless
//
// Implementation of IWMPNodeRealEstate::SetWindowless
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::SetWindowless( BOOL fWindowless )
{
    m_bWindowless   =   fWindowless;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetOwnerWindow
//
// Implementation of IWMPNodeWindowed::SetOwnerWindow
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::SetOwnerWindow( OLE_HWND hwnd )
{
    HRESULT hr = S_OK;

    if( NULL == m_hWnd )
    {
        return S_FALSE;
    }
 
    if( NULL == hwnd ) 
    { 
        // Hide the plug-in window.
        ShowWindow( SW_HIDE );
        
        // Set the Desktop as parent
        SetParent( NULL );
        m_bHosted = false; // Now Playing isn't visible
    }
    else 
    {
        // Set Windows Media Player as the parent
        SetParent( ( HWND ) hwnd );

        m_hwndParent = ( HWND ) hwnd;
        m_bHosted = true; // Now Playing is visible
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOwnerWindow
//
// Implementation of IWMPNodeWindowed::GetOwnerWindow
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetOwnerWindow( OLE_HWND *phwnd )
{
    if( NULL == phwnd )
    {
        return E_POINTER;
    }

    *phwnd = ( OLE_HWND ) m_hwndParent;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::OnWindowMessage
//
// Implementation of IWMPWindowMessageSink::OnWindowMessage
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::OnWindowMessage( UINT uMsg, WPARAM wparam, LPARAM lparam, 
                             LRESULT *plRet, BOOL *pfHandled )
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::OnDraw
//
// Implementation of IWMPNodeWindowless::OnDraw
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::OnDraw( OLE_HDC hdc, const RECT *prcDraw )
{
    HRESULT hr = S_OK;

    if( NULL == hdc )
    {
        return E_HANDLE;
    }

    if ( prcDraw )
    {
        HDC hDC = ( HDC )hdc;
    
        // Set the text color.
        COLORREF oldTextColor = ::SetTextColor( hDC, m_TextColor );
        // Set the text background color.
        COLORREF oldBkColor = ::SetBkColor( hDC, rgbWhite );

        hr = DoRendering( hDC, prcDraw );

        // Restore the original colors.
        ::SetBkColor( hDC, oldBkColor );
        ::SetTextColor( hDC, oldTextColor );
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::OnEraseBackground
//
// Handles WM_ERASEBKGND window message.
/////////////////////////////////////////////////////////////////////////////

LRESULT C[!output Safe_root]::OnEraseBackground( UINT nMsg, WPARAM wParam, 
                   LPARAM lParam, BOOL& bHandled )
{   
    // avoid erasing background to reduce flicker on resize
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::OnPaint
//
// Handles WM_PAINT window message.
/////////////////////////////////////////////////////////////////////////////

LRESULT C[!output Safe_root]::OnPaint( UINT nMsg, WPARAM wParam, 
                   LPARAM lParam, BOOL& bHandled )
{
    HRESULT hr = S_OK;

    PAINTSTRUCT ps;
    HDC hDC;
    
    hDC = BeginPaint( &ps );

    if ( NULL == hDC )
    {
        return 1;
    }

    // Set the text background to white.
    COLORREF oldBkColor = ::SetBkColor( hDC, rgbWhite );
    // Set the text color to the selection.
    COLORREF oldTextColor = ::SetTextColor( hDC, m_TextColor );

    hr = DoRendering(hDC, &m_rctDisplay);

    // Restore the colors.
    ::SetBkColor( hDC, oldBkColor );
    ::SetTextColor( hDC, oldTextColor );

    EndPaint( &ps );

    if ( FAILED( hr ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::DoRendering
//
// Renders the data.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::DoRendering (HDC hDC, const RECT *prc)
{
    if (NULL == prc)
    {
        return E_POINTER;
    }

    int widthDest = prc->right - prc->left;
    int heightDest = prc->bottom - prc->top;

    HRESULT hr = MakeBitmapFromData( hDC );
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( !StretchBlt( hDC, prc->left, prc->top, widthDest, heightDest,
        m_hdcMem, m_rctSrc.left, m_rctSrc.top, m_widthSrc, m_heightSrc,
        SRCCOPY ) )
    {
        return E_HANDLE;
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::get_color
//
// Property get to retrieve the text color value via the public interface.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::get_color( LPCOLORREF pVal )
{ 
    if ( NULL == pVal )
    {
        return E_POINTER;
    }

    *pVal = m_TextColor;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::put_color
//
// Property put to store the text color value via the public interface.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::put_color( COLORREF newVal )
{
    m_TextColor = newVal;

    if( m_hWnd )
    {
        // Repaint to ensure that all text is the same color.
       Repaint();
    }

    // Even if the repaint fails...
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Repaint
//
// Force a redraw of the painting surface.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Repaint(void)
{
    HRESULT hr = S_OK;

    if( m_bWindowless )
    {
        if( m_spWMPNodeWindowlesshost.p )
        {
            // Invalidate the RECT to which we moved the window.
            // That's the same as the region to redraw.
            hr = m_spWMPNodeWindowlesshost->InvalidateRect( &m_rctWindowPos, FALSE );
        }
    }
    else
    {               
        InvalidateRect( NULL, FALSE );
        UpdateWindow();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::OnPluginWindowMessage
//
// Handle or forward certain mouse and keyboard window messages.
/////////////////////////////////////////////////////////////////////////////

LRESULT C[!output Safe_root]::OnPluginWindowMessage( UINT uMsg, WPARAM wparam, LPARAM lparam, 
                             BOOL& bHandled )
{
    int xPos;
    int yPos;
    long lRet;

    bHandled = false;

    if( NULL == m_spWMPNodeWindowedhost.p )
    {
        return E_POINTER;
    }

    switch ( uMsg )
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_MOUSEACTIVATE:

            // OK to forward these without change.
            m_spWMPNodeWindowedhost->OnWindowMessageFromRenderer( uMsg, wparam, lparam, &lRet, &bHandled );

            break;

        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMOUSEMOVE:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:

            // Cache the cursor position.
            xPos = GET_X_LPARAM( lparam );
            yPos = GET_Y_LPARAM( lparam );

            // Translate the cursor position to the plug-in window RECT coordinates.
            xPos += m_rctWindowPos.left;
            yPos += m_rctWindowPos.top;

            // Rebuild the lparam.
            lparam = MAKELPARAM( xPos, yPos );

            // Forward the message.
            m_spWMPNodeWindowedhost->OnWindowMessageFromRenderer( uMsg, wparam, lparam, &lRet, &bHandled );

            break;

        default:
            break;

    }

    return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MakeBitmapFromData
//
// Create a bitmap that displays the text from the arbitrary data stream.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MakeBitmapFromData ( HDC hDC )
{
    HRESULT hr = S_OK;
    HBITMAP hBitmap = NULL;
    HBRUSH  hBrush = NULL;

    if(m_hdcMem)
    {
        DeleteDC(m_hdcMem);
        m_hdcMem = NULL;
    }

    if(!(GetDeviceCaps(hDC, RASTERCAPS) & ( RC_STRETCHBLT )))
    {
        return E_HANDLE;
    }

    // Create a DC in memory for the bitmap.
    m_hdcMem = CreateCompatibleDC(hDC);
    if(!m_hdcMem)
    {
        return E_HANDLE;
    }

    // Create the empty bitmap.
    hBitmap = CreateCompatibleBitmap(m_hdcMem, m_widthSrc, m_heightSrc);
    if(!hBitmap)
    {
        return E_HANDLE;
    }

    SelectObject(m_hdcMem, (HGDIOBJ) hBitmap);

    // Paint the backround white.
    hBrush = (HBRUSH) GetStockObject( WHITE_BRUSH );       
    HGDIOBJ oldMemObj = SelectObject( m_hdcMem, hBrush  );

    if(!FillRect( m_hdcMem, &m_rctSrc, hBrush ))
    {
       hr = E_HANDLE;
    }

    SelectObject( m_hdcMem, oldMemObj );

    // Begin critical section.
    Lock();

    BYTE *pbInputData = NULL;
    DWORD cbInputLength = 0;

    // Get the data pointer and the buffer length.
    if( SUCCEEDED( hr ) && m_spInputBuffer.p )
    {
        hr = m_spInputBuffer->GetBufferAndLength( &pbInputData, &cbInputLength );
    }

    if ( SUCCEEDED( hr ) )
    {    
        // Display the data from the stream.
        ::DrawText( m_hdcMem, ( WCHAR* ) pbInputData, cbInputLength/sizeof( WCHAR ) - 1, &m_rctSrc, DT_VCENTER | DT_CENTER | DT_SINGLELINE );
    }

    // End critical section.
    Unlock();

    // Clean up the drawing stuff.
    DeleteObject( ( HGDIOBJ ) hBitmap );
    DeleteObject( hBrush );

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Read
//
// Implementation of IPropertyBag::Read.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Read( LPCWSTR pwszPropName, VARIANT *pVar, IErrorLog *pErrorLog )
{
    HRESULT hr = S_OK;
    CComPtr<IStream> pStream;

    if( NULL == pVar )
    {
        return E_POINTER;
    }

    if( NULL == pwszPropName )
    {
        return E_POINTER;
    }

    if ( 0 == _wcsicmp( pwszPropName, L"IconStreams" ) )
    {
        // Get the image and load into an IStream.
        hr = LoadResourceImage(&pStream);

        if ( FAILED( hr ) )
        {
            return hr;
        }
        
        // There is only one image. Otherwise, set to VT_ARRAY.
        pVar->vt = VT_UNKNOWN;
        // Return the IStream pointer.
        pVar->punkVal = pStream;
        pVar->punkVal->AddRef();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Write
//
// Implementation of IPropertyBag::Write
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Write( LPCWSTR pwszPropName, VARIANT *pVar )
{
    // E_NOTIMPL is not a valid return code for this method.
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::LoadResourceImage
//
// Load the plug-in image into an IStream and return a pointer.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::LoadResourceImage( IStream **ppStream )
{
    LPVOID pvSrcData   = NULL;
    LPVOID pvDestData   = NULL;
    HRSRC hRsrc = NULL;
    HGLOBAL hGlobal = NULL;
    HGLOBAL hgRawBytes = NULL;
    DWORD dwSize    = 0;
    CComPtr<IStream> spStream;
    HINSTANCE hInst = NULL;
    HRESULT hr = S_OK;

    if( NULL == ppStream )
    {
        return E_POINTER;
    }

    *ppStream = NULL;

    // Get a handle to the plug-in module.
    hInst = _Module.GetResourceInstance();
    if ( NULL == hInst )
    {
        hr = E_HANDLE;
    }

    if( SUCCEEDED( hr ) )
    {
        // Find the resource.
        hRsrc = ::FindResource( hInst, MAKEINTRESOURCE( IDR_ICON ), L"JPG" );
        if ( NULL == hRsrc )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Determine the size of the resource.
        dwSize = ::SizeofResource( hInst, hRsrc );
        if ( 0 == dwSize )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Load the resource.
        hGlobal = ::LoadResource( hInst, hRsrc );
        if ( NULL == hGlobal )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Get the pointer to the image data
        pvSrcData = ::LockResource( hGlobal );
        if ( NULL == pvSrcData )
        {
            hr = E_POINTER;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Allocate enough memory before creating stream
        hgRawBytes = ::GlobalAlloc( GMEM_MOVEABLE |GMEM_NODISCARD | GMEM_ZEROINIT, dwSize );
        if ( NULL == hgRawBytes )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Get a pointer to the first byte in the buffer.
        pvDestData = ::GlobalLock( hgRawBytes );
        if ( NULL == pvDestData )
        {
            hr = E_POINTER;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Copy the data to the buffer.
        memcpy( pvDestData, pvSrcData, dwSize );

        // Create a stream for the data.
        hr = CreateStreamOnHGlobal( hgRawBytes, TRUE, &spStream );
        ::GlobalUnlock( hgRawBytes );
    }

    if ( SUCCEEDED( hr ) &&
         NULL != spStream.p )
    {
        // Set the size for the stream.
        ULARGE_INTEGER ulSize;
        ulSize.QuadPart = dwSize;
        spStream.p->SetSize( ulSize );

        // Return the stream.
        spStream.p->AddRef();
        *ppStream = spStream.p;
    }
    else
    {
        // Free up memory if the stream can't be created.
        // Otherwise IStream owns this memory.
        // It is not necessary to unlock resources because
        // the system automatically deletes them when the process
        // that created them terminates.
        if ( hgRawBytes )
        {
            ::GlobalFree( hgRawBytes );
            hgRawBytes = NULL;
        }
    }

    return hr;
}


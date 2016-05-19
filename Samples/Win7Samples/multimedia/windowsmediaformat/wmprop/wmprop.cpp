//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            wmprop.cpp
//
// Abstract:             Implementation of CWMProp class.
//
//*****************************************************************************


#include "wmprop.h"
#include <strsafe.h>

// OUTOFMEMORY
///////////////////////////////////////////////////////////////////////////////
CWMProp::CWMProp( HRESULT *phr )
{
        m_pReader = NULL;
        m_pIWMEditor = NULL;
        m_pProfile = NULL;
        m_pHeaderInfo = NULL;

        *phr = S_OK;

        m_hOpenEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( NULL == m_hOpenEvent )
        {
                *phr = E_OUTOFMEMORY;
        }

        m_hCloseEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( NULL == m_hCloseEvent )
        {
                *phr = E_OUTOFMEMORY;
        }

        m_bIsDRM = FALSE;
}
// Safe_Release
///////////////////////////////////////////////////////////////////////////////
CWMProp::~CWMProp()
{
        SAFE_RELEASE( m_pReader );
        SAFE_RELEASE( m_pIWMEditor );
        SAFE_RELEASE( m_pProfile );
        SAFE_RELEASE( m_pHeaderInfo );

        SAFECLOSE_HANDLE( m_hOpenEvent );
        SAFECLOSE_HANDLE( m_hCloseEvent );
}

// IUnknown methods
///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWMProp::QueryInterface( REFIID riid,
                                                                                                    void **ppvObject )
{
    *ppvObject = NULL;
    return( E_NOINTERFACE );
}

///////////////////////////////////////////////////////////////////////////////
//
// Don't implement AddRef and Release. They are only used by the callback
// in the reader, and the CWMProp object owns the reader, so there's no need
// to pay attention to AddRef/Release calls.
///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CWMProp::AddRef()
{
    return( 1 );
}

///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CWMProp::Release()
{
    return( 1 );
}

///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWMProp::OnSample( /* [in] */ DWORD dwOutputNum,
                                                                                          /* [in] */ QWORD cnsSampleTime,
                                                                                          /* [in] */ QWORD cnsSampleDuration,
                                                                                          /* [in] */ DWORD dwFlags,
                                                                                          /* [in] */ INSSBuffer __RPC_FAR *pSample,
                                                                                          /* [in] */ void __RPC_FAR *pvContext)
{
        return( S_OK );
}

///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWMProp:: OnStatus( /* [in] */ WMT_STATUS Status,
                                                                                           /* [in] */ HRESULT hr,
                                                                                           /* [in] */ WMT_ATTR_DATATYPE dwType,
                                                                                           /* [in] */ BYTE __RPC_FAR *pValue,
                                                                                           /* [in] */ void __RPC_FAR *pvContext)
{
        switch ( Status )
        {
                case WMT_OPENED:
                        {
                                m_hr = hr;
                                SetEvent( m_hOpenEvent );
                                break;
                        }

                case WMT_CLOSED:
                        {
                                m_hr = hr;
                                SetEvent( m_hCloseEvent );
                                break;
                        }
                default:
                        break;
        }
        return( S_OK );
}

// Use IWMMetadataEditor interface to access 
// Protected, Seekable, and Stridable attributes
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::Open( LPCWSTR pwszFileName )
{
        ZeroMemory( m_wszFileName, sizeof( m_wszFileName ) );
        HRESULT hr = StringCchCopyW( m_wszFileName, ARRAYSIZE( m_wszFileName ), pwszFileName );
        if (FAILED(hr)) {
            return hr;
        }

        hr = OpenFileWithEditor();
        if ( FAILED( hr ) )
        {
                return( hr );
				}
				
        if ( !m_bIsDRM )
        {
                hr = OpenFileWithReader();
                if ( FAILED( hr ) )
                {
                        return( hr );
                }
        }
        else
        {
                _tprintf( _T( "These are the properties available from this app for a DRM file\n" ) );
                return( S_OK );
        }
        return( hr );
}
// Use IWMMetadataEditor anf IWMHeaderInfo interfaces to access
// Protected, Seekable, and Stridable attributes
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::OpenFileWithEditor()
{
        HRESULT           hr = S_OK;
        BOOL              fProp = 0;

        hr = WMCreateEditor( &m_pIWMEditor );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "WMCreateEditor failed: (hr=0x%08x)\n" ), hr ) ;
                return( hr );
        }

        hr = m_pIWMEditor->Open( m_wszFileName );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Editor could not open %ws error: (hr=0x%08x)\n" ), m_wszFileName, hr ) ;
                return( hr );
        }

        _tprintf( _T( "Properties for %ws are:\n" ), m_wszFileName );
        _tprintf( _T( "\n" ) );

        hr = m_pIWMEditor->QueryInterface( IID_IWMHeaderInfo, ( void ** ) &m_pHeaderInfo );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "QI for IWMHeaderInfo failed: (hr=0x%08x)\n" ), hr );
                return( hr );
        }

		//
        //	Check whether file is DRM
		//

        hr = GetBoolAttribsFromEditor( g_wszWMProtected, &fProp );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Could not get g_wszWMProtected from the Header: (hr=0x%08x)\n" ), hr );
        }
        else
        {
                m_bIsDRM = fProp;
                if ( fProp )
                {
                        _tprintf( _T( "Is Protected (DRM): true\n" ) );
                }
                else
                {
                        _tprintf( _T( "Is Protected (DRM): false\n" ) );
                }
        }

		//
		// Get seekable attribute from content and return true/false
        //
		
		fProp = 0;
        hr = GetBoolAttribsFromEditor( g_wszWMSeekable, &fProp );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Could not get g_wszWMSeekable from the Header: (hr=0x%08x)\n" ), hr );
        }
        else
        {
                if ( fProp )
                {
                        _tprintf( _T( "Is Seekable: true\n" ) );
                }
                else
                {
                        _tprintf( _T( "Is Seekable: false\n" ) );
                }
        }

		//
		// Get stridable attribute from content and return true/false
        //
		
		fProp = 0;
        hr = GetBoolAttribsFromEditor( g_wszWMStridable, &fProp );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Could not get g_wszWMStridable from the Header: (hr=0x%08x)\n" ), hr );
        }
        else
        {
                if ( fProp )
                {
                        _tprintf( _T( "Is Stridable: true\n" ) );
                }
                else
                {
                        _tprintf( _T( "Is Stridable: false\n" ) );
                }
        }

        _tprintf( _T( "\n" ) );

        hr = m_pIWMEditor->Close();
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Could not close the Editor: (hr=0x%08x)\n" ), hr );
        }
        return( hr );
}
// Use Reader interface to access profile properties
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::OpenFileWithReader()
{
        HRESULT hr = S_OK;

        do
        {
                hr = WMCreateReader( NULL, 0, &m_pReader );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "Could not create Reader: (hr=0x%08x)\n" ), hr );
                        break;
                }

                hr = m_pReader->Open( m_wszFileName, this, NULL );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "Could not open %ws with the Reader. error: (hr=0x%08x)\n" ), m_wszFileName, hr );
                        break;
                }

                WaitForSingleObject( m_hOpenEvent, INFINITE );
                if ( FAILED( m_hr ) )
                {
                        _tprintf( _T( "Could not open %ws with the Reader. error: (hr=0x%08x)\n" ), m_wszFileName, hr );
                        break;
                }

                hr = m_pReader->QueryInterface( IID_IWMProfile, ( void **) &m_pProfile );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "QI for IWMProfile failed: (hr=0x%08x)\n" ), hr );
                        break;
                }

                hr = GetPropertiesFromProfile();
                if ( FAILED( hr ) )
                {
                        break;
                }

                hr = m_pReader->Close();
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "Close failed: (hr=0x%08x)\n" ), hr );
                        break;
                }

                WaitForSingleObject( m_hCloseEvent, INFINITE );
                if ( FAILED( m_hr ) )
                {
                        _tprintf( _T( "Close failed: (hr=0x%08x)\n" ), hr );
                        hr = m_hr;
                        break;
                }
        } while (FALSE );
        return( hr );
}
// Use IWMMetadataEditor interface to access 
// StreamNumber attribute
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::GetBoolAttribsFromEditor( LPCWSTR pwszName, BOOL *pResult )
{
        HRESULT           hr = S_OK;
        WMT_ATTR_DATATYPE wmType;
    WORD              wStreamNum = 0;
        USHORT            cbLen = 0;

        hr = m_pHeaderInfo->GetAttributeByName( &wStreamNum, pwszName, &wmType, NULL, &cbLen );

        if ( FAILED( hr ) )
        {
                return( hr );
        }

        BYTE *pData = new BYTE[ cbLen ];
        if ( NULL == pData )
        {
                return( E_OUTOFMEMORY );
				}
        hr = m_pHeaderInfo->GetAttributeByName( &wStreamNum, pwszName, &wmType, pData, &cbLen );

        if ( FAILED( hr ) )
        {
                delete [] pData;
                pData = NULL;
                return( hr );
        }

        *pResult = *( int* )pData;

        delete [] pData;
        pData = NULL;

        return( hr );
}
// Use IWMStreamConfig interface to access
// number of streams, each stream number, and bitrate
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::GetPropertiesFromProfile()
{
        HRESULT hr = S_OK;
        if ( NULL == m_pProfile )
        {
                return( E_FAIL );
        }

        DWORD dwStreamCount = 0;
        hr = m_pProfile->GetStreamCount( &dwStreamCount );
        if ( FAILED( hr ) )
        {
                _tprintf( _T( "Could not get stream count: (hr=0x%08x)\n" ), hr );
                return( hr );
        }

        _tprintf( _T( "This Windows Media file has %d stream(s)\n" ), dwStreamCount );
        _tprintf( _T( "\n" ) );

        for ( DWORD dwIndex = 0; dwIndex < dwStreamCount; dwIndex++ )
        {
                IWMStreamConfig *pConfig = NULL;
                hr = m_pProfile->GetStream( dwIndex, &pConfig );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "Could not get the stream: (hr=0x%08x)\n" ), hr );
                        return( hr );
                }

                GUID guid = GUID_NULL;
                hr = pConfig->GetStreamType( &guid );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( "Could not get the stream type: (hr=0x%08x)\n" ), hr );
                        return( hr );
                }
                else
                {
                        if ( WMMEDIATYPE_Video == guid )
                        {
                                WORD wStreamNum = 0;
                                hr = pConfig->GetStreamNumber( &wStreamNum );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get stream number: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                DWORD dwBitrate = 0;
                                hr = pConfig->GetBitrate( &dwBitrate );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get bit rate: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                _tprintf( _T( "Video Stream properties:\n" ) );
                                _tprintf( _T( "Stream number: %d\n" ), wStreamNum );
                                _tprintf( _T( "Bitrate: %d bps \n" ), dwBitrate );
                                hr = PrintCodecName( pConfig );
                                _tprintf( _T( "\n" ) );
                        }
                        else if ( WMMEDIATYPE_Audio == guid )
                        {
                                WORD wStreamNum = 0;
                                hr = pConfig->GetStreamNumber( &wStreamNum );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get stream number: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                DWORD dwBitrate = 0;
                                hr = pConfig->GetBitrate( &dwBitrate );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get bit rate: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                _tprintf( _T( "Audio Stream properties:\n" ) );
                                _tprintf( _T( "Stream number: %d\n" ), wStreamNum );
                                _tprintf( _T( "Bitrate: %d bps \n" ), dwBitrate );
                                hr = PrintCodecName( pConfig );
                                _tprintf( _T( "\n" ) );
                        }
                        else if ( WMMEDIATYPE_Script == guid )
                        {
                                WORD wStreamNum = 0;
                                hr = pConfig->GetStreamNumber( &wStreamNum );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get stream number: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                DWORD dwBitrate = 0;
                                hr = pConfig->GetBitrate( &dwBitrate );
                                if ( FAILED( hr ) )
                                {
                                        _tprintf( _T( "Could not get bit rate: (hr=0x%08x)\n" ), hr );
                                        return( hr );
                                }
                                _tprintf( _T( "Script Stream properties:\n" ) );
                                _tprintf( _T( "Stream number: %d\n" ), wStreamNum );
                                _tprintf( _T( "Bitrate: %d bps \n" ), dwBitrate );
                                _tprintf( _T( "\n" ) );
                        }
                }
                pConfig->Release();
        }
        return( S_OK );
}
// Use IWMStreamConfig interface to access codec names
///////////////////////////////////////////////////////////////////////////////
HRESULT CWMProp::PrintCodecName( IWMStreamConfig *pConfig )
{
        if ( NULL == pConfig )
        {
                return( E_FAIL);
        }

        HRESULT hr = S_OK;
        IWMMediaProps* pMediaProps = NULL;
        do
        {
                hr = pConfig->QueryInterface( IID_IWMMediaProps, ( void ** )&pMediaProps );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( " QI for IWMMediaProps failed: (hr=0x%08x)\n" ), hr );
                        break;
                }

                DWORD cbType = 0;
                hr = pMediaProps->GetMediaType( NULL, &cbType );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( " Get Mediatype failed: (hr=0x%08x)\n" ), hr );
                        break;
                }

                BYTE *pData = new BYTE[ cbType ];
                
                if ( NULL == pData )
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf( _T( " Out of memory: (hr=0x%08x)\n" ), hr );
                        break;
                }

                ZeroMemory( pData, cbType );
                hr = pMediaProps->GetMediaType( ( WM_MEDIA_TYPE *) pData, &cbType );
                if ( FAILED( hr ) )
                {
                        _tprintf( _T( " Get Mediatype failed: (hr=0x%08x)\n" ), hr );
                        break;
                }

				//
                // Audio Codec Names
				//
				
                if ( WMMEDIASUBTYPE_WMAudioV9 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Audio V9\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_WMAudio_Lossless == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Audio V9 (Lossless Mode)\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_WMAudioV7 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Audio V7/V8\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
				 				else if ( WMMEDIASUBTYPE_WMSP1 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Speech Codec V9\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_WMAudioV2 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Audio V2\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_ACELPnet == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: ACELP.net\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
				
				//
                // Video Codec Names
                //
				
				else if ( WMMEDIASUBTYPE_WMV1 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Video V7\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                
                if ( WMMEDIASUBTYPE_MSS1 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Screen V7\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                
								if ( WMMEDIASUBTYPE_MSS2 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Screen V9\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_WMV2 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Video V8\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_WMV3 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Windows Media Video V9\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_MP43 == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: Microsoft MPEG-4 Video Codec V3 \n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
                else if ( WMMEDIASUBTYPE_MP4S == ( ( WM_MEDIA_TYPE *) pData )->subtype )
                {
                        _tprintf( _T( "Codec Name: ISO MPEG-4 Video V1\n" ) );
                        delete []pData;
                        pData = NULL;
                        break;
                }
				
        } while ( FALSE );
        SAFE_RELEASE( pMediaProps );
        return( hr );
}

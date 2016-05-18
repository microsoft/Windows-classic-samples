//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            reader.cpp
//
// Abstract:            Implementation of the CReader class which handles 
//						opening and reading files with the Format SDK's 
//						IWMSyncReader interface.
//
//*****************************************************************************


#include <stdio.h>
#include "Reader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReader::CReader( )
{
    m_pReader               = NULL;
    m_wAudioStreamNum       = 0;
    m_wVideoStreamNum       = 0;
    m_cnsStart              = 0;
    m_cnsEnd                = 0;
    m_fCompressed           = FALSE;
    m_fAudioStream          = TRUE;
    m_fVideoStream          = TRUE;
    m_fRangeInFrames        = FALSE;
    m_pStream               = NULL;
}

///////////////////////////////////////////////////////////////
//////
CReader::~CReader()
{
    if( NULL != m_pStream )
    {
        delete m_pStream;
    }

    SAFE_RELEASE( m_pReader );
}

///////////////////////////////////////////////////////////////
/////
HRESULT CReader::Open( const TCHAR *ptszFile )
{

    HRESULT hr = S_OK;


    if ( NULL == ptszFile || NULL == _tcslen(ptszFile))
    {
        return( E_INVALIDARG );
    }

    //
    //  Currently we can play only files. Streams are not supported.
    //
    if( 0 == _tcsnicmp( ptszFile, TEXT( "http" ), 4 ) )
    {
        _tprintf( _T( "Wrong input file - streams are not supported : (hr=0x%08x).\n" ) ,hr );
        return( E_INVALIDARG );
    }

    if ( NULL == m_pReader )
    {
        hr = WMCreateSyncReader(  NULL, 0, &m_pReader );
    }

    if ( FAILED( hr ) )
    {
        _tprintf( _T( "Could not create reader (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    //
    // Open the requested file using IStream just to show how to use IStream with the synchronous reader 
    //
	m_pStream = new CROStream;
	if( NULL == m_pStream )
	{
		hr = E_OUTOFMEMORY;
        _tprintf( _T( "Could not open file (hr=0x%08x).\n" ), hr );
		return( hr );
	}

    hr = m_pStream->Open( ptszFile );
    
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not open file (hr=0x%08x).\n" ) ,hr );
        return( hr );
    }

    hr = m_pReader->OpenStream( m_pStream );
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "Could not open file (hr=0x%08x).\n" ), hr );
        return( hr );
    }
    
    //
    // Get the profile interface
    //
    IWMProfile*    pProfile = NULL;

    hr = m_pReader->QueryInterface( IID_IWMProfile, ( VOID ** )&pProfile );
    if ( FAILED( hr ) ) 
    {
        _tprintf( _T(  "Could not QI for IWMProfile (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    //
    // Find out stream numbers for video and audio using the profile
    //
    hr = GetStreamNumbers( pProfile );
    SAFE_RELEASE( pProfile );
    if ( FAILED( hr ) ) 
    {
        _tprintf( _T(  "Could not stream numbers (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    return( hr );
}

///////////////////////////////////////////////////////////////
////
HRESULT CReader::SetParams(  DWORD cnsStart = 0,
                        DWORD cnsEnd = 0,
                        BOOL fCompressed = FALSE,
                        BOOL fAudioPresent = TRUE,
                        BOOL fVideoPresent = TRUE,
                        BOOL fRangeInFrames = FALSE )
{
    HRESULT hr = S_OK;

    if( 0 != cnsEnd && cnsEnd <= cnsStart )
    {
        return( E_INVALIDARG );
    }

    m_cnsStart = cnsStart;
    m_cnsEnd = cnsEnd;
    m_fCompressed = fCompressed;
    m_fAudioStream = fAudioPresent;
    m_fVideoStream = fVideoPresent;
    m_fRangeInFrames = fRangeInFrames;

    return( hr );
}

///////////////////////////////////////////////////////////////
////
HRESULT CReader::Close()
{
    HRESULT hr = S_OK ;

    if( NULL != m_pReader )
    {
        hr = m_pReader->Close();
    }
    
    if( FAILED ( hr ) )
    {
        return hr;
    }

    SAFE_RELEASE( m_pReader );

    return hr;
}


///////////////////////////////////////////////////////////////
//// Retrieve video and audio stream numbers from profile
////
HRESULT CReader::GetStreamNumbers(IWMProfile* pProfile)
{
    HRESULT             hr = S_OK;
    IWMStreamConfig*    pStream = NULL;
    DWORD               dwStreams = 0;
    GUID                pguidStreamType;

    if ( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    hr = pProfile->GetStreamCount( &dwStreams );
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "GetStreamCount on IWMProfile failed (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    m_wAudioStreamNum = 0;
    m_wVideoStreamNum = 0;

    for ( DWORD i = 0; i < dwStreams; i++ )
    {
        hr = pProfile->GetStream( i, &pStream );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not get Stream %d of %d from IWMProfile (hr=0x%08x).\n" ),
                    i, dwStreams, hr );
            break;
        }

        WORD wStreamNumber = 0 ;

        //
        //  Get the stream number of the current stream
        //

        hr = pStream->GetStreamNumber( &wStreamNumber );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not get stream number from IWMStreamConfig %d of %d (hr=0x%08x).\n" ),
                    i, dwStreams, hr );
            break;
        }

        hr = pStream->GetStreamType( &pguidStreamType );
        if ( FAILED( hr ) )
        {
            _tprintf( _T("Could not get stream type of stream %d of %d from IWMStreamConfig (hr=0x%08x).\n" ),
                i, dwStreams, hr ) ;
            break ;
        }
        
        if( WMMEDIATYPE_Audio == pguidStreamType )
        {
            m_wAudioStreamNum = wStreamNumber;
        }
        else if( WMMEDIATYPE_Video == pguidStreamType )
        {
            m_wVideoStreamNum = wStreamNumber;
        }

        SAFE_RELEASE( pStream );
    }

    return( hr );
}


///////////////////////////////////////////////////////////////
////
HRESULT CReader::ReadSamples()
{
    HRESULT hr = S_OK;
    INSSBuffer* pSample = NULL;

    WORD	wStream = 1;
    WMT_STREAM_SELECTION	wmtSS = WMT_ON;
    QWORD cnsSampleTime = 0, cnsPrevSampleTime = 0;
    QWORD cnsDuration = 0;
    DWORD dwFlags = 0;
    DWORD dwOutputNum = 0;
    WORD wStreamNum = 0;
    static DWORD dwVideoSamplesCnt = 0;
    static DWORD dwAudioSamplesCnt = 0;


    if( 0 != m_wAudioStreamNum )
    {
        if( m_fAudioStream )
        {
            wmtSS = WMT_ON;
        }
        else
        {
            wmtSS = WMT_OFF;
        }

        hr = m_pReader->SetStreamsSelected( 1, &m_wAudioStreamNum, &wmtSS );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "SetStreamsSelected (hr=0x%08x).\n" ), hr );
            return( hr );
        }

        hr = m_pReader->SetReadStreamSamples( m_wAudioStreamNum, m_fCompressed );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "SetReadStreamSamples (hr=0x%08x).\n" ), hr );
            return( hr );
        }
    }

    if( 0 != m_wVideoStreamNum )
    {
        if( m_fVideoStream )
        {
            wmtSS = WMT_ON;
        }
        else
        {
            wmtSS = WMT_OFF;
        }

        hr = m_pReader->SetStreamsSelected( 1, &m_wVideoStreamNum, &wmtSS );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "SetStreamsSelected (hr=0x%08x).\n" ), hr );
            return( hr );
        }

        hr = m_pReader->SetReadStreamSamples( m_wVideoStreamNum, m_fCompressed );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "SetReadStreamSamples (hr=0x%08x).\n" ), hr );
            return( hr );
        }

        if( m_fRangeInFrames )
        {
            QWORD qwDuration = 0;

            if( 0 != m_cnsEnd )
            {
                qwDuration = m_cnsEnd - m_cnsStart;
            }

            hr = m_pReader->SetRangeByFrame( m_wVideoStreamNum, m_cnsStart, qwDuration );
            if ( FAILED( hr ) )
            {
                _tprintf( _T(  "SetRangeByFrame (hr=0x%08x).\n" ), hr );
                return( hr );
            }
        }
    }

    if( 0 == m_wVideoStreamNum || ( !m_fRangeInFrames && 0 != m_wVideoStreamNum ) )
    {
        QWORD qwDuration = 0;

        if( 0 != m_cnsEnd )
        {
            qwDuration = ( m_cnsEnd - m_cnsStart ) * 10000L;
        }

        hr = m_pReader->SetRange( m_cnsStart * 10000L, qwDuration );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "SetRange (hr=0x%08x).\n" ), hr );
            return( hr );
        }
    }

    _tprintf( _T( "\nGetting samples ...\n" ) );

    while( SUCCEEDED( hr ) )
    {


	    hr = m_pReader->GetNextSample( 0, &pSample,
                                            &cnsSampleTime,
                                            &cnsDuration,
                                            &dwFlags,
                                            &dwOutputNum,
                                            &wStreamNum );

        if( FAILED( hr ) )
        {
            if( NS_E_NO_MORE_SAMPLES == hr )
            {
                hr = S_OK;
                _tprintf( _T( "\nLast sample reached.\n" ) );
                _tprintf( _T( "\nLast sample time : %lu ms\n" ), cnsPrevSampleTime/10000 );
                break;
            }
            else
            {
                _tprintf( _T( "GetNextSample() failed : (hr=0x%08x).\n" ), hr );
                return( hr );
            }
        }

        cnsPrevSampleTime = cnsSampleTime;

        if( 0 == dwVideoSamplesCnt && 0 == dwAudioSamplesCnt )
        {
            _tprintf( _T( "\nFirst sample time : %lu ms\n" ), cnsSampleTime/10000 );
        }

        if( m_wVideoStreamNum == wStreamNum )
        {
            dwVideoSamplesCnt++;
            if ( 0 == dwVideoSamplesCnt % 4 )
            {
                _tprintf( _T( "v" ) );
            }
        }
        else if( m_wAudioStreamNum == wStreamNum )
        {
            dwAudioSamplesCnt++;

            if ( 0 == dwAudioSamplesCnt % 4 )
            {
                _tprintf( _T( "a" ) );
            }
        }


        pSample->Release();
    }
    return( hr );
}


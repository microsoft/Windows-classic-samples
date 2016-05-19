//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            reader.cpp
//
// Abstract:            Implementation of the CReader class
//
//*****************************************************************************

#include "Reader.h"
//////////////////////////////////////////////////////////////////////
//    The CReader object reads stream data from an input (file or URL) 
//    specified by the pwszFile parameter of the Configure method and 
//    sends it the output, which is a CWriter or CWaveOut object,
//    depending which one of them has been attached. Data sent to CWriter
//    is compressed, while data sent to CWaveOut is decompressed by CReader. 
//    The GetStats method enables you to get statistics at any time during
//    data transmission.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CReader::CReader()
{
    m_pReaderHeaderInfo     = NULL ;
    m_pReaderAdvanced       = NULL ;
    m_pReader               = NULL ;
    m_pReaderStreamClock    = NULL ;
    m_hEvent                = NULL ;
    m_fEOF                  = false ;
    m_qwTime                = 0 ;
    m_hrAsync               = S_OK ;
    m_fReaderStarted        = false;
    m_fNetReading           = false;
    m_pWriter               = NULL;
    m_dwAudioStreamNum      = 0;
    m_dwTimerId             = 0;

}
///////////////////////////////////////////////////////////////

CReader::~CReader()
{
    SAFE_RELEASE( m_pReaderStreamClock );
    SAFE_RELEASE( m_pReaderHeaderInfo );
    SAFE_RELEASE( m_pReaderAdvanced ) ;
    SAFE_RELEASE( m_pReader ) ;

    if( NULL != m_hEvent )
    {
        CloseHandle( m_hEvent ) ;
        m_hEvent = NULL ;
    }
}
///////////////////////////////////////////////////////////////
// Initialize Reader object:
//        create all necessary interfaces and an event
///////////////////////////////////////////////////////////////
HRESULT CReader::Init()
{
    HRESULT hr = S_OK;

    if ( m_fReaderStarted || m_pReaderHeaderInfo )
        return E_FAIL;

    do
    {
        m_hrAsync = S_OK ;
    
        m_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ;

        if ( NULL ==  m_hEvent )
        {
            _tprintf( _T( "Could not Create Event.\n" ));
            break;
        }
        if ( FAILED( hr = WMCreateReader( NULL, WMT_RIGHT_PLAYBACK, &m_pReader )))
        {
            _tprintf( _T( "Could not create reader (hr=0x%08x).\n" ), hr );
            break;
        }
        
        if ( FAILED( hr = m_pReader->QueryInterface( IID_IWMReaderAdvanced, ( void** )&m_pReaderAdvanced )))
        {
            _tprintf( _T( "Could not QI for IWMReaderAdvanced (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pReader->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pReaderHeaderInfo );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pReader->QueryInterface( IID_IWMReaderStreamClock, ( VOID ** )&m_pReaderStreamClock );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMReaderStreamClock (hr=0x%08x).\n" ), hr );
            break;
        }


    } while( FALSE );

    if (FAILED( hr ) )
    {
        SAFE_RELEASE( m_pReaderStreamClock );
        SAFE_RELEASE( m_pReaderHeaderInfo );
        SAFE_RELEASE( m_pReaderAdvanced );
        SAFE_RELEASE( m_pReader );
        CloseHandle( m_hEvent );
    }

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Configure previously intialized reader object
//        pwszFile - URL of input file
//
HRESULT CReader::Configure( const WCHAR *pwszFile )
{

    HRESULT hr = S_OK;
    const WCHAR * wszAttributes[] = { g_wszWMTitle,
                                g_wszWMAuthor,
                                g_wszWMDescription,
                                g_wszWMRating,
                                g_wszWMCopyright,
                                g_wszWMAlbumTitle,
                                g_wszWMTrack,
                                g_wszWMPromotionURL,
                                g_wszWMAlbumCoverURL,
                                g_wszWMGenre,
                                g_wszWMYear,
                                g_wszWMGenreID,
                                g_wszWMMCDI,
                                g_wszWMBannerImageType,
                                g_wszWMBannerImageData,
                                g_wszWMBannerImageURL,
                                g_wszWMCopyrightURL };

    //
    // Check, if it's a net location.
    //

    if ( NULL == pwszFile || NULL == wcslen(pwszFile))
        return E_INVALIDARG;

    m_fNetReading = _wcsnicmp( pwszFile, L"http", 4 )? FALSE : TRUE ;

    //
    // Open the requested file 
    //

    hr = m_pReader->Open( pwszFile, this, NULL );
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "Could not open file %ws (hr=0x%08x).\n" ), pwszFile ,hr );
        return( hr );
    }
    
    //
    // Wait for the open to finish
    //

    WaitForEvent();
    
    if ( FAILED( m_hrAsync ) )
    {
        _tprintf( _T( "Open failed (hr=0x%08x).\n" ), m_hrAsync );
        return( m_hrAsync );
    }

    //
    // Turn on manual stream selection, so we get all streams.
    //

    hr = m_pReaderAdvanced->SetManualStreamSelection( TRUE );
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Failed to set manual stream selection (hr=0x%08x).\n" ), hr );
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

    m_dwAudioStreamNum = 0;

    do 
    {
		//
        // Create the list of data unit extensions defined for all streams
        // in this profile
        //
        hr = m_ExtDataList.Create( pProfile );
        if ( FAILED( hr ) ) 
        {
            _tprintf( _T( "Creating data unit extension list failed (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // If output is writer, do not decompress samples
        //

        if ( NULL != m_pWriter )
        {
            hr = SetCodecOff( pProfile );
            if ( FAILED( hr ) )
                break;
        }

        //
        // Turn on the user clock if reading from file
        //

        if ( !m_fNetReading )
        {
            hr = m_pReaderAdvanced->SetUserProvidedClock( TRUE );
            if ( FAILED( hr ) ) 
            {
                _tprintf( _T( "SetUserProvidedClock failed (hr=0x%08x).\n" ), hr );
                break;
            }
        }

        //
        // Configure writer - if attached
        //

        if ( NULL != m_pWriter )
        {
            hr = m_pWriter->Configure( pProfile );
            if (FAILED ( hr ) )
                break;

            int attrs = sizeof( wszAttributes ) / sizeof( wszAttributes[ 0 ] );
            for ( int i = 0; i < attrs; i++ )
            {
                hr = CopyAttribToWriter( wszAttributes[ i ] );
                if (FAILED ( hr ) )
                    break;
            }
            if ( FAILED( hr ) )
                break;

            hr = CopyScriptsToWriter();
            if (FAILED ( hr ) )
                break;
        }


    }while( FALSE );

    SAFE_RELEASE( pProfile );

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Attach writer object used to output stream data
//    This function stops an already attached writer object or 
//    closes an already attached WaveOut object
//
HRESULT CReader::AttachWriter( CWriter *pWriter )
{
    HRESULT hr = S_OK;
    
    if ( NULL == pWriter )
        return E_INVALIDARG;

    if ( NULL != m_pWriter )
        m_pWriter->Stop();

    m_pWriter = pWriter;

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Start sending data to attached output. If a writer object has been
//    attached, it is initialized. This function is synchronous and waits until all
//    data is sent. Since the current writer object implementation has only a 
//    network sink, all data sent to this object is left compressed; that is, the
//    codec is off and data is passed thru OnStreamSample. If a WaveOut object 
//    is attached, data is decompressed and passed through theOnSample method.
//
HRESULT CReader::Start()
{

    HRESULT hr = S_OK;

    if ( m_fReaderStarted || NULL == m_pReader )
    {
        return( E_FAIL );
    }

    if ( NULL != m_pWriter )
    {
        hr = m_pWriter->Start();
        if ( FAILED( hr ) ) 
        {
            return( hr );
        }
    }

    hr = m_pReader->Start( 0, 0, 1.0, 0 );
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "Could not start IWMReader (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    m_fReaderStarted = true;

    //
    // wait until all data sent to output
    //

    WaitForEvent();

    if ( FAILED( m_hrAsync ) )
    {
        _tprintf( _T(  "IWMReader failed (hr=0x%08x).\n" ), m_hrAsync );
        return( m_hrAsync );
    }


    return( hr );
}
///////////////////////////////////////////////////////////////
// Stop both reader and writer object (if attached). Synchronous method
//
HRESULT CReader::Stop( HANDLE *hThread, int cHandles )
{
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;

    if ( !m_fReaderStarted || NULL == m_pReader )
    {
        return( E_FAIL );
    }

    if( NULL != hThread && cHandles > 0 )
    {
        dwStatus = WaitForMultipleObjects( cHandles, hThread, TRUE, 20000 );
        if( WAIT_TIMEOUT == dwStatus )
        {
            return E_FAIL ;
        }
    }

    if ( NULL != m_pWriter )
    {
        hr = m_pWriter->Stop();
        if ( FAILED( hr ) ) 
        {
            _tprintf( _T(  "Could not Stop IWMWriter (hr=0x%08x).\n" ), hr );
            return( hr );
        }
    }

    hr = m_pReader->Stop();
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not Stop IWMReader (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    WaitForEvent();

    m_fReaderStarted = false;

    return hr ;

}
///////////////////////////////////////////////////////////////
//    Stop and close reader object. If WaveOut is attached, wait
//    until it finishes with all samples and close it before closing 
//    the reader. Asynchronous method
//
HRESULT CReader::Close( HANDLE *hThread, int cHandles )
{
    HRESULT hr = S_OK ;

    if ( m_fReaderStarted )
    {
        hr = Stop( hThread, cHandles );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Error while stopping reader (hr=0x%08x).\n" ), hr);
        }
    }

    if( NULL != m_pReader )
    {
        hr = m_pReader->Close();
    }
    
    if( FAILED ( hr ) )
    {
        return hr;
    }

    WaitForEvent();
    return hr;
}


///////////////////////////////////////////////////////////////
// Get current reader statistics
//
HRESULT CReader::GetStats( WM_READER_STATISTICS *pStats )
{
    HRESULT hr = S_OK;

    if ( NULL == pStats )
    {
        return E_INVALIDARG;
    }

    if ( NULL != m_pReaderAdvanced )
    {
        ZeroMemory( pStats, sizeof( WM_READER_STATISTICS ));
        pStats->cbSize = sizeof( WM_READER_STATISTICS );

        hr = m_pReaderAdvanced->GetStatistics( pStats );
    }
    return( hr );
}


///////////////////////////////////////////////////////////////
//    Used for synchronization
//
void CReader::WaitForEvent()
{
    WaitForSingleObject( m_hEvent, INFINITE );
    return;
}

///////////////////////////////////////////////////////////////
//    Callback used to send decompressed samples to attached
//    WaveOut object
//
HRESULT CReader::OnSample( DWORD dwOutputNum, 
                             QWORD cnsSampleTime,
                             QWORD cnsSampleDuration, 
                             DWORD dwFlags,
                             INSSBuffer __RPC_FAR *pSample, void __RPC_FAR *pvContext )
{
    HRESULT hr = S_OK;

    if ( NULL == pSample )
    {
        return E_INVALIDARG;
    }

    if ( NULL != m_hEvent )
    {
        //
        // TODO: Insert per-sample code here
        //

    }
    return S_OK;
}
///////////////////////////////////////////////////////////////
//    Callback function used to send compressed samples to attached writer object
//
HRESULT CReader::OnStreamSample( WORD wStreamNum, 
                                   QWORD cnsSampleTime, 
                                   QWORD cnsSampleDuration, 
                                   DWORD dwFlags, 
                                   INSSBuffer __RPC_FAR *pSample, 
                                   void __RPC_FAR *pvContext )
{
    HRESULT hr = S_OK;

    if ( NULL == m_pWriter || NULL == pSample)
    {
        return E_INVALIDARG;
    }

    if ( cnsSampleTime % cnsSampleDuration )
    {
        _tprintf( _T(  "*" ) );
    }

	//
    // Getting extension data:
    // find what extensions are defined for this stream and then
    // iterate thru them, get their values and display them
    // 
    CExtensionData *pExtData = NULL;
    INSSBuffer3 *pMS3 = NULL;
    DWORD pvBuffer = 0;

    bool ret = m_ExtDataList.Find( ( WORD )wStreamNum, &pExtData );
    if( ret )
    {
        hr = pSample->QueryInterface( IID_INSSBuffer3, (void**)&pMS3 );
    }

    while( ret )
    {        
        DWORD dwTemp = pExtData->m_cbExtensionDataSize;
        hr = pMS3->GetProperty( pExtData->m_guidDUExt, pExtData->m_pValue, &dwTemp );
        pExtData->DisplayData();
        ret = m_ExtDataList.Find( 0, &pExtData );
    }

    SAFE_RELEASE( pMS3 );


    //
    // Pass uncompressed samples to writer
    //

    hr = m_pWriter->WriteStreamSample( wStreamNum,
                                          cnsSampleTime,
                                          0,
                                          cnsSampleDuration,
                                          dwFlags,
                                          pSample );
    return( hr );
}
///////////////////////////////////////////////////////////////
//    Callback used for setting synchronization event and getting 
//    current status of data transfer
//
HRESULT CReader::OnStatus( WMT_STATUS Status, 
                             HRESULT hr, 
                             WMT_ATTR_DATATYPE dwType, 
                             BYTE __RPC_FAR *pValue, 
                             void __RPC_FAR *pvContext )
{
    switch( Status )
    {
    
    case WMT_OPENED:
        
        m_hrAsync = hr;
        SetEvent( m_hEvent );        
        break;
    
    case WMT_EOF:

        m_fEOF = true;
        _tprintf( _T(  "EndOfStream detected in reader.\n" ) );
        
        SetEvent( m_hEvent );
        break;

    case WMT_CLOSED:

        m_hrAsync = hr;
        SetEvent( m_hEvent );
        break ;

    case WMT_STOPPED:
        
        m_hrAsync = hr;
        SetEvent( m_hEvent );
        break;
    
    case WMT_STARTED:

        //
        //Ask for the specific duration of the stream to be delivered
        //

        if (!m_fNetReading)
        {
            m_qwTime = 0;
            m_qwTime += 1000 * 10000;

            hr = m_pReaderAdvanced->DeliverTime( m_qwTime );
        }
        break;

    case WMT_LOCATING:
        //
        // The stream is being read from a network; set the variable.
        //
        m_fNetReading = TRUE;
        break;

    case WMT_TIMER :
        _tprintf( _T(  "Timer ...........................\n" ) );
        break;

    }

    return S_OK;
}
///////////////////////////////////////////////////////////////
// Callback : set user defined clock if stream data is read from file

HRESULT CReader::OnTime( QWORD cnsCurrentTime, void __RPC_FAR *pvContext)
{
    //
    //  Keep asking for the specific duration of the stream till EOF
    //
    HRESULT hr = S_OK;

    if( NULL == m_fEOF )
    {
        m_qwTime += 10000000 ; // 1 second
        hr = m_pReaderAdvanced->DeliverTime( m_qwTime );
    }

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Switch codec off so compressed samples are passed through OnStreamSample
//
HRESULT CReader::SetCodecOff(IWMProfile*    pProfile)
{
    HRESULT             hr = S_OK;
    IWMStreamConfig*    pStream = NULL;
    DWORD               dwStreams = 0;
    GUID                pguidStreamType;

    if (!pProfile)
        return E_INVALIDARG;


    hr = pProfile->GetStreamCount( &dwStreams );
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "GetStreamCount on IWMProfile failed (hr=0x%08x).\n" ), hr );
        return hr;
    }

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

        //
        //  Set the stream to be received in compressed mode
        //

        hr = m_pReaderAdvanced->SetReceiveStreamSamples( wStreamNumber, TRUE );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not SetReceivedStreamSamples for stream number %d (hr=0x%08x).\n" ),
                wStreamNumber, hr );
            break;
        }

        SAFE_RELEASE( pStream );
    }

    return( hr );
}
///////////////////////////////////////////////////////////////
//    Copy attribute value from reader to attached writer. 
//    pwszName - attribute name
//
HRESULT CReader::CopyAttribToWriter( const WCHAR *pwszName )
{
    WORD                nstreamNum = 0;
    WORD                cbLength = 0;
    WMT_ATTR_DATATYPE   type;
    HRESULT             hr = S_OK;
    BYTE*               pValue = NULL;
    

    if ( NULL == pwszName || 0 == wcslen(pwszName))
    {
        return E_INVALIDARG;
    }

    if ( NULL == m_pWriter)
    {
        return E_FAIL;
    }

    //
    // Get the number of bytes to be allocated for pValue
    //

    hr = m_pReaderHeaderInfo->GetAttributeByName( &nstreamNum,
                                                  pwszName,
                                                  &type,
                                                  NULL,
                                                  &cbLength );
    if ( FAILED( hr ) && hr != ASF_E_NOTFOUND )
    {
        _tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr);
        return hr;
    }

    if( 0 == cbLength && hr == ASF_E_NOTFOUND )
    {
        return S_OK;
    }

    pValue = new BYTE[ cbLength ];
    if ( NULL == pValue )
    {
        _tprintf( _T( "Unable to allocate memory for the Attribute name %ws" ), pwszName);
        return E_OUTOFMEMORY;
    }
    //
    //  Dummy do-while loop
    //
    do
    {
        //
        // Get the value
        //

        hr = m_pReaderHeaderInfo->GetAttributeByName( &nstreamNum,
                                                    pwszName,
                                                    &type,
                                                    pValue,
                                                    &cbLength );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ),
                      pwszName, hr);
            break;
        }

        //
        // Set the attribute
        //

        hr = m_pWriter->SetAttribute( nstreamNum,
                                                pwszName,
                                                type,
                                                pValue,
                                                cbLength );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "SetAttribute failed for Attribute name %ws (hr=0x%08x).\n" ),
                      pwszName, hr);
            break;
        }
    }
    while( FALSE );
        
	if (NULL != pValue)
	{
		delete[] pValue;
		pValue = NULL;
	}

    return( hr );
}
///////////////////////////////////////////////////////////////
// Copy all scripts from reader to attached writer
//
HRESULT CReader::CopyScriptsToWriter()
{
    HRESULT     hr = S_OK;
    WCHAR*      pwszCommand = NULL;
    WCHAR*      pwszType = NULL;
    QWORD       cnsScriptTime = 0;
    WORD        cScript = 0;
    WORD        cchTypeLen = 0;
    WORD        cchCommandLen = 0;

    if ( NULL == m_pWriter || NULL == m_pReaderHeaderInfo)
        return E_FAIL;

    hr = m_pReaderHeaderInfo->GetScriptCount( &cScript );
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "GetScriptCount failed (hr=0x%08x).\n" ), hr);
        return hr;
    }


    for( WORD i = 0 ; i < cScript ; i++)
    {
        //
        // Get the memory required for this script
        //

        hr = m_pReaderHeaderInfo->GetScript( i ,
                                             NULL ,
                                             &cchTypeLen ,
                                             NULL ,
                                             &cchCommandLen ,
                                             &cnsScriptTime );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr ) ;
            break ;
        }
        
        pwszType    = new WCHAR[ cchTypeLen ];
        pwszCommand = new WCHAR[ cchCommandLen ];

        if( NULL == pwszType || NULL == pwszCommand )
        {
            _tprintf( _T( "Insufficient Memory" ) );
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Get the script
        //

        hr = m_pReaderHeaderInfo->GetScript( i ,
                                             pwszType ,
                                             &cchTypeLen ,
                                             pwszCommand ,
                                             &cchCommandLen ,
                                             &cnsScriptTime );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr );
            break;
        }

        //
        // Add the script to the writer
        //

        hr = m_pWriter->AddScript( pwszType ,
                                             pwszCommand ,
                                             cnsScriptTime );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "AddScript failed for Script no %d (hr=0x%08x).\n" ), i, hr);
            break;
        }
        
        delete[] pwszType;
        delete[] pwszCommand;

        pwszType = pwszCommand = NULL;
        
        cchTypeLen       = 0;
        cchCommandLen    = 0;
    }
    
	if(NULL != pwszType)
	{
		delete[] pwszType;
		pwszType = NULL;
	}
	if(NULL != pwszCommand)
	{
		delete[] pwszCommand;
		pwszCommand = NULL;
	}
    
    pwszType = pwszCommand = NULL;
    
    return( hr );    

}

///////////////////////////////////////////////////////////////
////////
HRESULT STDMETHODCALLTYPE CReader::QueryInterface( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) 
{
    if ( riid == IID_IWMReaderCallback )
    {
        *ppvObject = ( IWMReaderCallback* )this;
    }
    else if( riid == IID_IWMReaderCallbackAdvanced )
    {
        *ppvObject = ( IWMReaderCallbackAdvanced* )this;
    }
    else if( riid == IID_IUnknown )
    {
        *ppvObject = this;
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

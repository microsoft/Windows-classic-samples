//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            writer.cpp
//
// Abstract:            Implementation of the CWriter class
//
//*****************************************************************************

#include "Writer.h"
//////////////////////////////////////////////////////////////////////
//    The CWriter object is fed with stream data samples by the CReader object
//    by calls to its WriteStreamSample method. It sends data to the output
//    defined by a file or network sink. Data has to be compressed. All attributes 
//    have to be set up by CReader according to its input stream attributes
//    The GetStats method enables you to get current output stream
//    statistics at any time during data transmission.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWriter::CWriter()
{
    m_pWriter = NULL;
    m_pWriterAdvanced = NULL;
    m_pWriterAdvanced3 = NULL;
    m_pWriterHeaderInfo = NULL;
    m_pFileSink = NULL;
	m_pNetSink = NULL;
    m_fWriterStarted = FALSE;

    m_pStreamNumList = NULL;
    m_nStreamNumListCount = 0;


}
///////////////////////////////////////////////////////////////

CWriter::~CWriter()
{
    if( NULL != m_pFileSink )
    {
        RemoveFileSink();
    }
	if( NULL != m_pNetSink )
    {
        RemoveNetSink();
    }

    SAFE_RELEASE( m_pWriterHeaderInfo ) ;
    SAFE_RELEASE( m_pWriterAdvanced ) ;
    SAFE_RELEASE( m_pWriterAdvanced3 ) ;
    SAFE_RELEASE( m_pFileSink );
	SAFE_RELEASE( m_pNetSink );
    SAFE_RELEASE( m_pWriter ) ;

    if ( NULL != m_pStreamNumList )
    {
        delete [] m_pStreamNumList;
    }

}

///////////////////////////////////////////////////////////////
// Initialize Writer object:
//        create all necessary interfaces and an event
///////////////////////////////////////////////////////////////

HRESULT CWriter::Init()
{
    HRESULT hr = S_OK;

    if ( m_fWriterStarted || NULL != m_pWriterHeaderInfo )
        return E_FAIL;

    do
    {
        hr = WMCreateWriter( NULL, &m_pWriter );
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not create Writer (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pWriter->QueryInterface( IID_IWMWriterAdvanced, ( void** ) &m_pWriterAdvanced );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMWriterAdvanced (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pWriter->QueryInterface( IID_IWMWriterAdvanced3, ( void** ) &m_pWriterAdvanced3 );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMWriterAdvanced3 (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pWriter->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pWriterHeaderInfo );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
            break;
        }
    }while ( FALSE );

    if (FAILED( hr ) )
    {
        SAFE_RELEASE( m_pWriterHeaderInfo );
        SAFE_RELEASE( m_pWriterAdvanced );
        SAFE_RELEASE( m_pWriterAdvanced3 );
        SAFE_RELEASE( m_pWriter );
    }
    return( hr );
}
///////////////////////////////////////////////////////////////
//    Configure writer output according to profile
//
HRESULT CWriter::Configure( IWMProfile *pProfile )
{
    HRESULT hr = S_OK;

    if ( m_fWriterStarted || NULL == m_pWriterHeaderInfo || NULL == pProfile)
    {
        return( E_FAIL );
    }

    //
    // Set the writer's properties
    //

    hr = m_pWriter->SetProfile( pProfile );
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not set profile on IWMWriter (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    //
    // Set compression off - samples are already compressed
    //

    hr = SetCodecOff();
    if ( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Create array of stream numbers for statistics reporting
    //

    CreateStreamNumList( pProfile );

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Starts writing to output
//
HRESULT CWriter::Start()
{
    HRESULT hr = S_OK ;

    if ( NULL == m_pWriter || m_fWriterStarted)
        return( E_FAIL );

    //
    // Start writing
    //

    hr = m_pWriter->BeginWriting();
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "BeginWriting on IWMWriter failed (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    m_fWriterStarted = TRUE;
    return( hr );
}

///////////////////////////////////////////////////////////////
// Stop writing to output. All data is flushed out
//
HRESULT CWriter::Stop()
{
    HRESULT hr = S_OK ;

    if ( NULL == m_pWriter || !m_fWriterStarted)
    {
        return( E_FAIL );
    }

    hr = m_pWriter->Flush();
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not Flush on IWMWriter (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    hr = m_pWriter->EndWriting();
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not EndWriting on IWMWriter (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    m_fWriterStarted = FALSE;

    if( NULL != m_pFileSink )
    {
        hr = RemoveFileSink();
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not remove file sink (hr=0x%08x).\n" ), hr );
            return( hr );
        }
    }
	
	if( NULL != m_pNetSink )
    {
        hr = RemoveNetSink();
        if ( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not remove net sink (hr=0x%08x).\n" ), hr );
            return( hr );
        }
    }


    return( hr );
}

///////////////////////////////////////////////////////////////
//    Create and attach a file sink
//        pwszFileName    - output file name
//
HRESULT CWriter::CreateFileSink( __in LPWSTR pwszFileName )
{
    HRESULT hr = S_OK ;

    if ( NULL == m_pWriterAdvanced || m_fWriterStarted || NULL != m_pFileSink)
    {
        return( E_FAIL );
    }

    do
    {
        hr = WMCreateWriterFileSink( &m_pFileSink );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create Writer File Sink (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Configure the File Sink
        //
        
        hr = m_pFileSink->Open( pwszFileName );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "File sink failed to open file %ws (hr=0x%08x).\n" ),
                      pwszFileName, hr ) ;
            break;
        }

        //
        // Add the network sink to the Writer
        //
        hr = m_pWriterAdvanced->AddSink(m_pFileSink);
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Add Sink (hr=0x%08x).\n" ), hr );
            break;
        }

    } while( FALSE );

    if (FAILED( hr ) )
    {
        SAFE_RELEASE( m_pFileSink );
    }

    return( hr );

}

///////////////////////////////////////////////////////////////
//    Remove file sink
//
HRESULT CWriter::RemoveFileSink()
{
    HRESULT hr = S_OK ;
    
    if ( NULL == m_pWriterAdvanced || NULL == m_pFileSink || m_fWriterStarted)
    {
        return( E_FAIL );
    }

    hr = m_pWriterAdvanced->RemoveSink( m_pFileSink );
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not remove the File Sink (hr=0x%08x).\n" ),  hr );
        return( hr );
    }

    SAFE_RELEASE( m_pFileSink );
    SAFE_RELEASE( m_pWriterHeaderInfo );
    SAFE_RELEASE( m_pWriterAdvanced );
    SAFE_RELEASE( m_pWriter );

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Create and attach a network sink
//        dwPortNumber      - port number
//        nMaxClient        - maximum number of connected clients    
//
HRESULT CWriter::CreateNetSink( DWORD dwPortNumber, UINT nMaxClient )
{
    HRESULT hr = S_OK ;

    if ( NULL == m_pWriterAdvanced || m_fWriterStarted || NULL != m_pNetSink)
    {
        return( E_FAIL );
    }

    do
    {
        hr = WMCreateWriterNetworkSink( &m_pNetSink );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create Writer Network Sink (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Configure the Net Sink
        //

        hr = m_pNetSink->SetNetworkProtocol( WMT_PROTOCOL_HTTP );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Set Network protocol (hr=0x%08x).\n" ), hr );
            break;
        }
        
        hr = m_pNetSink->Open( &dwPortNumber);
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Network sink failed to open port no %d (hr=0x%08x).\n" ),
                      dwPortNumber, hr ) ;
            break;
        }

        //
        // Set the maximum number of clients that can connect to the port
        //

        hr = m_pNetSink->SetMaximumClients( nMaxClient );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Set maximum clients (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Add the network sink to the Writer
        //
        hr = m_pWriterAdvanced->AddSink(m_pNetSink);
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Add Sink (hr=0x%08x).\n" ), hr );
            break;
        }

    } while( FALSE );

    if (FAILED( hr ) )
    {
        SAFE_RELEASE( m_pNetSink );
    }

    return( hr );

}

///////////////////////////////////////////////////////////////
//    Remove network sink
//
HRESULT CWriter::RemoveNetSink()
{
    HRESULT hr = S_OK ;
    
    if ( NULL == m_pWriterAdvanced || NULL == m_pNetSink || m_fWriterStarted)
    {
        return( E_FAIL );
    }

    hr = m_pWriterAdvanced->RemoveSink( m_pNetSink );
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not remove the Network Sink (hr=0x%08x).\n" ),  hr );
        return( hr );
    }

    hr = m_pNetSink->Close();
    if ( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not close on IWMWriterNetworkSink (hr=0x%08x).\n" ), hr );
        return( hr );
    }

    SAFE_RELEASE( m_pNetSink );

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Return output statistics and number of connected users.
//    A statistic with stream number 0 means summary statistics of all streams
//        dwClientNum        - number of currently connected clients
//        nStatsNum          - number of statistics stored in the buffer
//        pStats             - buffer containing summary statistics and statistics for
//                             each stream.
//
HRESULT CWriter::GetStats( DWORD &dwClientNum, int &nStatsNum, CWriterStats **pStats, CWriterStatsEx **pStatsEx )
{
    HRESULT                 hr = S_OK;
    WM_WRITER_STATISTICS    Stats; 
    WM_WRITER_STATISTICS_EX StatsEx;
    CWriterStats*           pLocalStats = NULL;
	CWriterStatsEx*         pLocalStatsEx = NULL;

    if ( NULL == pStats )
        return E_INVALIDARG;

    nStatsNum = 0;

    if ( NULL != m_pNetSink )
    {
        IWMClientConnections *pIWMClientConnections = NULL;
        hr = m_pNetSink->QueryInterface(IID_IWMClientConnections, (void**) &pIWMClientConnections);
        if ( SUCCEEDED( hr ) )
        {
            hr = pIWMClientConnections->GetClientCount(&dwClientNum);
            if (FAILED ( hr ) )
            {
                _tprintf( _T(  "Could not get client number from IWMWriter (hr=0x%08x).\n" ), hr );
            }

            SAFE_RELEASE( pIWMClientConnections );
        }
    }

    if ( NULL != m_pWriterAdvanced && NULL != m_pWriter )
    {
        //
        // Get summary statistics
        //

        hr = m_pWriterAdvanced->GetStatistics( 0, &Stats );
        if (FAILED ( hr ) )
        {
            _tprintf( _T(  "Could not get statistics IWMWriter (hr=0x%08x).\n" ), hr );
        }
    }

    if ( NULL != m_pWriterAdvanced3 && NULL != m_pWriter )
    {
        //
        // Get summary statistics
        //

        hr = m_pWriterAdvanced3->GetStatisticsEx( 0, &StatsEx );
        if (FAILED ( hr ) )
        {
            _tprintf( _T(  "Could not get statistics IWMWriter (hr=0x%08x).\n" ), hr );
        }
    }

    pLocalStats = new CWriterStats[ m_nStreamNumListCount + 1 ];
    if ( NULL == pLocalStats )
    {
        return E_OUTOFMEMORY;
    }

    *pStats = pLocalStats;

	pLocalStatsEx = new CWriterStatsEx;
    if ( NULL == pLocalStatsEx )
    {
        return E_OUTOFMEMORY;
    }

    *pStatsEx = pLocalStatsEx;
	pLocalStatsEx->m_nStreamNum = 0;
	*pLocalStatsEx = StatsEx;
	

    // Copy summary statistics
    pLocalStats->m_nStreamNum = 0; // all streams;
    *pLocalStats++ = Stats;
    nStatsNum++;

    DWORD nStreamsCount = m_nStreamNumListCount;
    WORD *pStreamNumList = m_pStreamNumList;

    while( 0 != nStreamsCount-- 
        && SUCCEEDED( hr = m_pWriterAdvanced->GetStatistics( *pStreamNumList, &Stats ) ) )
    {
        pLocalStats->m_nStreamNum = *pStreamNumList++;
        *pLocalStats++ = Stats;
        nStatsNum++;
    }

    return( hr );
}

///////////////////////////////////////////////////////////////
//    Write stream sample to output
//
HRESULT CWriter::WriteStreamSample(WORD wStreamNum, QWORD cnsSampleTime, DWORD msSampleSendTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample)
{
    HRESULT hr = E_FAIL;

    if (m_pWriterAdvanced && m_fWriterStarted)
        hr = m_pWriterAdvanced->WriteStreamSample( wStreamNum,
                                          cnsSampleTime,
                                          msSampleSendTime,
                                          cnsSampleDuration,
                                          dwFlags,
                                          pSample ) ;
    return hr ;

}

///////////////////////////////////////////////////////////////
//    Add a script to output profile
//
HRESULT CWriter::AddScript(__in LPWSTR pwszType, __in LPWSTR pwszCommand, QWORD cnsScriptTime)
{
    HRESULT hr = S_OK;

    if (!m_pWriterHeaderInfo)
        return E_FAIL;

    hr = m_pWriterHeaderInfo->AddScript( pwszType ,
                                         pwszCommand ,
                                         cnsScriptTime ) ;
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "AddScript failed (hr=0x%08x).\n" ), hr) ;
    }
    return hr;

}
///////////////////////////////////////////////////////////////
//    Set attribute value for an output stream
//
HRESULT CWriter::SetAttribute(WORD nstreamNum, LPCWSTR pwszName, WMT_ATTR_DATATYPE type, const BYTE *pValue, WORD cbLength)
{

    HRESULT hr = S_OK;

    if (!m_pWriterHeaderInfo)
        return E_FAIL;

    hr = m_pWriterHeaderInfo->SetAttribute( nstreamNum,
                                            pwszName,
                                            type,
                                            pValue,
                                            cbLength );
    if ( FAILED( hr ) )
    {
        _tprintf( _T( "SetAttribute failed for Attribute name %ws (hr=0x%08x).\n" ),
                  pwszName, hr) ;
    }

    return hr;
}

///////////////////////////////////////////////////////////////
//    Switch codec off for all streams - samples are sent without change
//
HRESULT CWriter::SetCodecOff()
{
    HRESULT hr = S_OK;
    DWORD   cInputs = 0 ;

    if (!m_pWriter)
        return E_FAIL;

    if( FAILED( hr = m_pWriter->GetInputCount( &cInputs )))
    {
        _tprintf( _T(  "Could not get input count from IWMWriter (hr=0x%08x).\n" ), hr );
        return hr ;
    }

    for( DWORD i = 0; i < cInputs; i++ )
    {
        //
        // Set the input props to NULL to indicate that we don't need a codec
        // because we are writing compressed samples to the port
        //
        if( FAILED( hr = m_pWriter->SetInputProps( i, NULL )))
            break;
    }

    return hr;
}

///////////////////////////////////////////////////////////////
// Create a global array of stream numbers for a given profile
//
HRESULT CWriter::CreateStreamNumList(IWMProfile*    pProfile)
{
    HRESULT             hr = S_OK;
    IWMStreamConfig*    pStream = NULL ;
    WORD*               pStreamNumList = NULL;

    if (!pProfile)
        return E_INVALIDARG;


    if ( FAILED( hr = pProfile->GetStreamCount( &m_nStreamNumListCount )))
    {
        _tprintf( _T(  "GetStreamCount on IWMProfile failed (hr=0x%08x).\n" ), hr );
        return hr ;
    }

    if (m_nStreamNumListCount)
        m_pStreamNumList = new WORD[m_nStreamNumListCount];
    else
        return S_OK;

    if (!m_pStreamNumList)
        return E_FAIL;

    pStreamNumList = m_pStreamNumList;

    for ( DWORD i = 0; i < m_nStreamNumListCount; i++ )
    {
        if ( FAILED( hr = pProfile->GetStream( i, &pStream )))
        {
            _tprintf( _T(  "Could not get Stream %d of %d from IWMProfile (hr=0x%08x).\n" ),
                    i, m_nStreamNumListCount, hr );
            break;
        }

        //
        //Get the stream number of the current stream
        //
        if ( FAILED( hr = pStream->GetStreamNumber( pStreamNumList )))
        {
            _tprintf( _T(  "Could not get stream number from IWMStreamConfig %d of %d (hr=0x%08x).\n" ),
                    i, m_nStreamNumListCount, hr );
            break;
        }

        pStreamNumList++;

        SAFE_RELEASE( pStream );
    }

    return hr;
}

///////////////////////////////////////////////////////////////

const CWriterStats &CWriterStats::operator= (const WM_WRITER_STATISTICS &right)
{

    qwSampleCount = right.qwSampleCount;
    qwByteCount = right.qwByteCount;
    qwDroppedSampleCount = right.qwDroppedSampleCount;
    qwDroppedByteCount = right.qwDroppedByteCount;
    dwCurrentBitrate = right.dwCurrentBitrate;
    dwAverageBitrate = right.dwAverageBitrate;
    dwExpectedBitrate = right.dwExpectedBitrate;
    dwCurrentSampleRate = right.dwCurrentSampleRate;
    dwAverageSampleRate = right.dwAverageSampleRate;
    dwExpectedSampleRate = right.dwExpectedSampleRate;
 
    return *this;
}

const CWriterStatsEx &CWriterStatsEx::operator= (const WM_WRITER_STATISTICS_EX &right)
{
	dwBitratePlusOverhead = right.dwBitratePlusOverhead ;
    dwCurrentSampleDropRateInQueue = right.dwCurrentSampleDropRateInQueue ;
    dwCurrentSampleDropRateInCodec = right.dwCurrentSampleDropRateInCodec ;
    dwCurrentSampleDropRateInMultiplexer = right.dwCurrentSampleDropRateInMultiplexer ;
    dwTotalSampleDropsInQueue = right.dwTotalSampleDropsInQueue ;
    dwTotalSampleDropsInCodec = right.dwTotalSampleDropsInCodec ;
    dwTotalSampleDropsInMultiplexer = right.dwTotalSampleDropsInMultiplexer ;

 
    return *this;
}


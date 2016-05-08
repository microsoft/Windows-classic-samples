//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Append.cpp
//
// Abstract:            Implementation of CAppend class
//
//*****************************************************************************

#include "stdafx.h"
#include <assert.h>
#include "Append.h"
#include "StreamData.h"

// User-defined attribute
static const WCHAR * pwszUserAttribute1 = L"_MyAttribute";

//------------------------------------------------------------------------------
// Name: CAppend::CAppend()
// Desc: Constructor.
//------------------------------------------------------------------------------
CAppend::CAppend()
{
    m_hrAsync         = S_OK;
    m_hAsyncEvent     = NULL;
    m_pReader1        = NULL;
    m_pReader2        = NULL;
    m_pReaderAdv1     = NULL;
    m_pReaderAdv2     = NULL;
    m_pWriter         = NULL;
    m_pWriterAdv      = NULL;
    m_pFirstProfile   = NULL;
    m_pwszOutFile     = NULL;
    m_pRdrHdrInfo1    = NULL;
    m_pRdrHdrInfo2    = NULL;
    m_pwStreamNumMap  = NULL;
    m_pSecondProfile  = NULL;
	m_bEOF            = FALSE;
    m_qwFirstTime     = 0;
    m_qwSecondTime    = 0;
    m_nCurrentFile    = 0;
    m_dwStreamCount   = 0;
	m_cRef			  = 1;
    //
    // The variable WORD* m_pwStreamNumMap contains the mapping of stream numbers from the
    // second file to the output file. It contains 2 * m_dwStreamCount number of WORDs.
    // The first m_dwStreamCount number of WORDs contains the output file stream numbers, 
    // which are in the same order as the stream numbers of first input file.
    // The second m_dwStreamCount number of WORDs contains the stream numbers from the
    // second file in the same order as the first half. The corresponding streams in the 
    // first half and the second half are always of the same type.
    //
    InitializeCriticalSection( &m_crisecFile );
}

//------------------------------------------------------------------------------
// Name: CAppend::~CAppend()
// Desc: Destructor.
//------------------------------------------------------------------------------
CAppend::~CAppend()
{
    Exit();
    
    DeleteCriticalSection( &m_crisecFile );
}

//------------------------------------------------------------------------------
// Name: CAppend::OnSample()
// Desc: Implementation of IWMReaderCallback::OnSample.
//------------------------------------------------------------------------------
HRESULT CAppend::OnSample( /* [in] */ DWORD dwOutputNum,
                           /* [in] */ QWORD qwSampleTime,
                           /* [in] */ QWORD qwSampleDuration,
                           /* [in] */ DWORD dwFlags,
                           /* [in] */ INSSBuffer __RPC_FAR * pSample,
                           /* [in] */ void __RPC_FAR * pvContext )        
{
    if( m_hAsyncEvent != NULL )
    {
        //
        // The samples are expected in OnStreamSample
        //
        _tprintf( _T( "Error: Received a decompressed sample from the reader.\n" ) );
        
        m_hrAsync = E_UNEXPECTED;
        SetEvent( m_hAsyncEvent );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAppend::OnStreamSample()
// Desc: Implementation of IWMReaderCallbackAdvanced::OnStreamSample.
//------------------------------------------------------------------------------
HRESULT CAppend::OnStreamSample( /* [in] */ WORD wStreamNum,
                                 /* [in] */ QWORD cnsSampleTime,
                                 /* [in] */ QWORD cnsSampleDuration,
                                 /* [in] */ DWORD dwFlags,
                                 /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                 /* [in] */ void __RPC_FAR * pvContext )
{
    short   nFileNum = 0;
    HRESULT  hr      = S_OK;

    EnterCriticalSection( &m_crisecFile );

        nFileNum = m_nCurrentFile;

    LeaveCriticalSection( &m_crisecFile );

        TCHAR tszFlags[ 200 ];

    (void)StringCchCopy( tszFlags, ARRAYSIZE(tszFlags), _T("") );

    if( dwFlags & WM_SF_CLEANPOINT )
    {
        (void)StringCchCat( tszFlags, ARRAYSIZE(tszFlags), _T(" WM_SF_CLEANPOINT ") );
    }
    if( dwFlags & WM_SF_DISCONTINUITY )
    {
        (void)StringCchCat( tszFlags, ARRAYSIZE(tszFlags), _T(" WM_SF_DISCONTINUITY ") );
    }
    if( dwFlags & WM_SF_DATALOSS )
    {
        (void)StringCchCat( tszFlags, ARRAYSIZE(tszFlags), _T(" WM_SF_DATALOSS ") );
    }

	//
	// We've got a stream. Let's write it in the output file
	//
    if( nFileNum == 1 )
    {
        _tprintf ( _T( "FirstFile StreamSample: num=%d, time=%I64u, duration=%I64u, flags=%s.\n" ),
                   wStreamNum, cnsSampleTime, cnsSampleDuration, tszFlags );
	    
        hr = m_pWriterAdv->WriteStreamSample( wStreamNum,
                                              cnsSampleTime,
                                              0,
                                              cnsSampleDuration,
                                              dwFlags,
                                              pSample );
    }
    else if( nFileNum == 2 )
    {
        _tprintf ( _T( "SecondFile StreamSample: num=%d, time=%I64u, duration=%I64u, flags=%s.\n" ),
                   wStreamNum, (cnsSampleTime + m_qwFirstTime), cnsSampleDuration, tszFlags );
	    
        WORD dwNum = MapStreamNum( wStreamNum );
        
        if( dwNum )
        {
            hr = m_pWriterAdv->WriteStreamSample( dwNum,
                                                  cnsSampleTime + m_qwFirstTime,
                                                  0,
                                                  cnsSampleDuration,
                                                  dwFlags,
                                                  pSample );
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Error in WriteStreamSample (hr=0x%08x).\n" ), hr );
        
        m_hrAsync = hr;
        SetEvent( m_hAsyncEvent );
    }

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CAppend::OnTime()
// Desc: Implementation of IWMReaderCallbackAdvanced::OnTime.
//------------------------------------------------------------------------------
HRESULT CAppend::OnTime( /* [in] */ QWORD cnsCurrentTime,
                         /* [in] */ void __RPC_FAR * pvContext)
{
	if( m_bEOF || m_nCurrentFile > 2 )
	{
		return( S_OK );
	}

    short   nFileNum = 0;
    HRESULT hr       = S_OK;

    EnterCriticalSection( &m_crisecFile );

        nFileNum = m_nCurrentFile;

    LeaveCriticalSection( &m_crisecFile );

    if( nFileNum == 1 )
    {
        m_qwFirstTime += 1000 * 10000;

        hr = m_pReaderAdv1->DeliverTime( m_qwFirstTime );
    }
    else if( nFileNum == 2 )
    {
        m_qwSecondTime += 1000 * 10000;

        hr = m_pReaderAdv2->DeliverTime( m_qwSecondTime );
    }
    
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Error in DeliverTime (hr=0x%08x).\n" ), hr );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAppend::OnStatus()
// Desc: Implementation of IWMStatusCallback::OnStatus.
//------------------------------------------------------------------------------
HRESULT CAppend::OnStatus( /* [in] */ WMT_STATUS Status,
						   /* [in] */ HRESULT hr,
						   /* [in] */ WMT_ATTR_DATATYPE dwType,
						   /* [in] */ BYTE __RPC_FAR * pValue,
						   /* [in] */ void __RPC_FAR * pvContext ) 
{
    short nFileNum = 0;

    switch(Status)
    {

    case WMT_OPENED:

        _tprintf( _T( "Status WMT_OPENED detected.\n" ) );

        EnterCriticalSection( &m_crisecFile );

            nFileNum = m_nCurrentFile;

        LeaveCriticalSection( &m_crisecFile );
        //
        // Set the event only for the first two open calls
        //
		if( nFileNum < 3 )
		{
			m_hrAsync = hr;
			SetEvent( m_hAsyncEvent );
		}

        break;

    case WMT_STARTED:
        
        _tprintf( _T( "Status WMT_STARTED detected.\n" ) );

        EnterCriticalSection( &m_crisecFile );

            nFileNum = ++m_nCurrentFile;

        LeaveCriticalSection( &m_crisecFile );
        //
        // Ask for the specific duration of the stream to be delivered
        //
        if( nFileNum == 1 )
        {
            m_qwFirstTime = 0;
            m_qwFirstTime += 1000 * 10000;
            hr = m_pReaderAdv1->DeliverTime( m_qwFirstTime );
        }
        else if( nFileNum == 2 )
        {
            m_qwSecondTime = 0;
            m_qwSecondTime += 1000 * 10000;
            hr = m_pReaderAdv2->DeliverTime( m_qwSecondTime );
        }
        
        assert( SUCCEEDED( hr ) );
        m_bEOF = FALSE;

        break;
    
    case WMT_STOPPED:
        
        _tprintf( _T( "Status WMT_STOPPED detected.\n" ) );

        EnterCriticalSection( &m_crisecFile );
            
            nFileNum = m_nCurrentFile;

            if( m_nCurrentFile == 2 )
            {
			    m_nCurrentFile++;
            }

        LeaveCriticalSection( &m_crisecFile );

        m_hrAsync = S_OK;
        SetEvent( m_hAsyncEvent );
        
        break;

    case WMT_ERROR:
    case WMT_EOF:

        _tprintf( _T( "Status WMT_EOF detected.\n" ) );
        
        if( m_bEOF )
        {
            break;
        }
		m_bEOF = TRUE;
        
        EnterCriticalSection( &m_crisecFile );

            nFileNum = m_nCurrentFile;

        LeaveCriticalSection( &m_crisecFile );
        //
        // Set the event only for the first two EOFs
        //
        if( nFileNum < 3 )
		{
			m_hrAsync = hr;
			SetEvent( m_hAsyncEvent );
		}
        
		
        break;
    
    case WMT_CLOSED:
        
        _tprintf( _T( "Status WMT_CLOSED detected.\n" ) );

        m_hrAsync = hr;
		SetEvent( m_hAsyncEvent);
		
        break;	
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAppend::Init()
// Desc: Creates an event, initializes COM, and creates readers.
//------------------------------------------------------------------------------
HRESULT CAppend::Init()
{
    HRESULT hr = S_OK;

	do
	{    
		//
		// Create an event for handling asynchronous calls
		//
		m_hAsyncEvent = CreateEvent( NULL, FALSE, FALSE, WMVAPPEND_ASYNC_EVENT );	    
		if( NULL == m_hAsyncEvent )
		{
			_tprintf( _T( "Could not create asynchronous event.\n" ) );
			hr = E_FAIL;
			break;	
		}
	    
		hr = CoInitialize( NULL );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "CoInitialize failed (hr=0x%08x).\n" ), hr );
			break;
		}
		//
		// Create two readers and two readeradvanced
		//
		hr = WMCreateReader( NULL, 0, &m_pReader1 );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not create first reader (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = WMCreateReader( NULL, 0, &m_pReader2 );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not create second reader (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = m_pReader1->QueryInterface( IID_IWMReaderAdvanced, ( VOID ** )&m_pReaderAdv1 );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not QI for first IWMReaderAdvanced (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = m_pReader2->QueryInterface( IID_IWMReaderAdvanced, ( VOID ** )&m_pReaderAdv2 );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not QI for second IWMReaderAdvanced (hr=0x%08x).\n" ), hr );
			break;
		}

		//
		// Create a writer and a writeradvanced
		//
		hr = WMCreateWriter( NULL, &m_pWriter );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not create writer (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = m_pWriter->QueryInterface( IID_IWMWriterAdvanced, ( VOID ** )&m_pWriterAdv );	    
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not QI for IWMWriterAdvanced (hr=0x%08x).\n" ), hr );			
		}

	}while( FALSE );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CAppend::Exit()
// Desc: Closes readers and cleans up.
//------------------------------------------------------------------------------
HRESULT CAppend::Exit()
{
    SAFE_ARRAYDELETE( m_pwszOutFile );
    SAFE_ARRAYDELETE( m_pwStreamNumMap );
    
    if( m_pReader1 )
    {
        m_pReader1->Close();
    }

    if( m_pReader2 )
    {
        m_pReader2->Close();
    }

    SAFE_RELEASE( m_pFirstProfile );
    SAFE_RELEASE( m_pSecondProfile );
    SAFE_RELEASE( m_pRdrHdrInfo1 );
    SAFE_RELEASE( m_pRdrHdrInfo2 );
    SAFE_RELEASE( m_pWriterAdv );
    SAFE_RELEASE( m_pWriter );
    SAFE_RELEASE( m_pReaderAdv2 );
    SAFE_RELEASE( m_pReaderAdv1 );
    SAFE_RELEASE( m_pReader2 );
    SAFE_RELEASE( m_pReader1 );

    CoUninitialize();

	SAFE_CLOSEHANDLE( m_hAsyncEvent )
    
    return( S_OK );
}


//------------------------------------------------------------------------------
// Name: CAppend::CompareProfiles()
// Desc: Ascertains whether two files contain the same stream numbers and
//       media types.
//------------------------------------------------------------------------------
HRESULT CAppend::CompareProfiles(__in LPWSTR pwszFirstInfile, __in LPWSTR pwszSecondInfile, BOOL* pIsEqual)
{
    if( NULL == m_pReader1  || NULL == m_pReader2 )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
	
    DWORD dwFirstStreams  = 0;
    DWORD dwSecondStreams = 0;

    *pIsEqual  = FALSE;

    do
    {
        //
        // Check if any of the files are protected.
        //
        hr = IsFileProtected( pwszFirstInfile );
        if( FAILED ( hr ) )
        {
            _tprintf( _T( "First file is protected, cannot open, aborting...\n" ) );
            break;
        }

        hr = IsFileProtected( pwszSecondInfile );
        if( FAILED ( hr ) )
        {
            _tprintf( _T( "Second file is protected, cannot open, aborting...\n" ) );
            break;
        }
        //
        // Open both the files
        //
        hr = m_pReader1->Open( pwszFirstInfile, this, NULL );
        if( FAILED ( hr ) )
        {
            _tprintf( _T( "Could not open first input file %ws (hr=0x%08x).\n" ), pwszFirstInfile, hr );
            break;
        }

        //
        // Wait for the open to finish
        //
        WaitForSingleObject( m_hAsyncEvent, INFINITE );
        if( FAILED( m_hrAsync ) )
        {
            hr = m_hrAsync;
            _tprintf( _T( "Open failed for first file %ws (hr=0x%08x).\n"), pwszFirstInfile, m_hrAsync );
            break;
        }
        //
        // Same thing for second file
        //
        hr = m_pReader2->Open( pwszSecondInfile, this, NULL );
        if( FAILED ( hr ) )
        {
            _tprintf( _T( "Could not open second input file %ws (hr=0x%08x).\n" ), pwszSecondInfile, hr );
            break;
        }

        WaitForSingleObject( m_hAsyncEvent, INFINITE );
        if( FAILED( m_hrAsync ) )
        {
            hr = m_hrAsync;
            _tprintf( _T( "Open failed for second file %ws (hr=0x%08x).\n"), pwszSecondInfile, m_hrAsync );
            break;
        }

        //
        // Get the profile interfaces
        //
        hr = m_pReader1->QueryInterface( IID_IWMProfile, ( VOID ** )&m_pFirstProfile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMProfile of first file (hr=0x%08x).\n" ), hr );
            break;
        }
        
        hr = m_pReader2->QueryInterface( IID_IWMProfile, ( VOID ** )&m_pSecondProfile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMProfile of second file (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pFirstProfile->GetStreamCount( &dwFirstStreams );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetStreamCount on IWMProfile failed for first file (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pSecondProfile->GetStreamCount( &dwSecondStreams );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetStreamCount on IWMProfile failed for second file (hr=0x%08x).\n" ), hr );
            break;
        }

		if( dwFirstStreams != dwSecondStreams )
		{
            _tprintf( _T( "Different Stream counts in both the files.\n" ) );
			break;
        }
        
        m_dwStreamCount = dwFirstStreams;
        //
        // Create objects of CStreamData for each input file.
        //
        CStreamData streamdata1( dwFirstStreams );
        CStreamData streamdata2( dwSecondStreams );
        //
		// Set all stream-related data from the profile
		//
        hr = streamdata1.SetAllStreamData( m_pFirstProfile );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = streamdata2.SetAllStreamData( m_pSecondProfile );
        if( FAILED( hr ) )
        {
            break;
        }
		//
		// Map the stream numbers of both the profiles
		//
        *pIsEqual = streamdata1.MapStreamNums( streamdata2, &m_pwStreamNumMap );

        //
        // This function assumes that streamdata1.m_ptrStreamBufferWindow[] 
        // already contains the sum of input BufferWindow values for each output stream
        // which was calculated by the MapStreamNums function call
        //
        hr = streamdata1.SetAllStreamsBufferWindow( m_pFirstProfile );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );
    
    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::Configure()
// Desc: Configures the output.
//------------------------------------------------------------------------------
HRESULT CAppend::Configure( __in LPWSTR pwszOutFile )
{
	
    if( NULL == m_pFirstProfile || NULL == m_pSecondProfile  )
    {
        return( E_INVALIDARG );
    }
    
    IWMHeaderInfo*   pWHeaderInfo  = NULL;
    HRESULT hr = S_OK;
    DWORD cInputs = 0;
    
    do
    {
        //
        // Turn on manual stream selections, so we get all streams.
        //
        hr = m_pReaderAdv1->SetManualStreamSelection( TRUE );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set manual stream selection for first file (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pReaderAdv2->SetManualStreamSelection( TRUE );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set manual stream selection for second file (hr=0x%08x).\n" ), hr );
            break;
        }

        UINT i;
        for( i = 0; i < m_dwStreamCount; i++ )
        {
			//
			// Receive all the streams as compressed streams
			//
            hr = SetReceiveStreamSample( m_pReaderAdv1, m_pFirstProfile, i );
            if( FAILED( hr ) )
            {
                break;
            }
            //
            // Same thing for the second file
            //
            hr = SetReceiveStreamSample( m_pReaderAdv2, m_pSecondProfile, i );
            if( FAILED( hr ) )
            {
                break;
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Turn on the user clocks, so we get the streams faster than the real life speed.
        //
        hr = m_pReaderAdv1->SetUserProvidedClock( TRUE );
        if( FAILED( hr ) )
        {
	        _tprintf( _T("SetUserProvidedClock failed for first file (hr=0x%08x).\n" ), hr );
	        break;
        }

        hr = m_pReaderAdv2->SetUserProvidedClock( TRUE );
        if( FAILED( hr ) )
        {
	        _tprintf( _T("SetUserProvidedClock failed for second file (hr=0x%08x).\n" ), hr );
	        break;
        }

        hr = m_pWriter->SetProfile( m_pFirstProfile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not set profile on IWMWriter (hr=0x%08x).\n" ), hr );
            break;
        }
        
        //
        // Keep a copy of the output file name
        //
        m_pwszOutFile = new WCHAR[ wcslen( pwszOutFile ) + 1 ];
        if( NULL == m_pwszOutFile)
        {
            _tprintf( _T( "Internal Error: Out of memory\n" ) );
            hr = E_OUTOFMEMORY;
            break;
        }
        (void)StringCchCopyW( m_pwszOutFile, wcslen( pwszOutFile ) + 1, pwszOutFile );

        hr = m_pWriter->SetOutputFilename( m_pwszOutFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not set output file %ws on IWMWriter (hr=0x%08x).\n" ), pwszOutFile, hr );
            break;
        }

        hr = m_pWriter->GetInputCount( &cInputs );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not get input count from IWMWriter (hr=0x%08x).\n" ), hr );
            break;
        }

        for( i = 0; i < cInputs; i++ )
        {
            //
            // Set the input props to NULL to indicate that we don't need a codec.
            //
            m_pWriter->SetInputProps( i, NULL );
        }

        //
		// QI for IWMHeaderInfo from readers and writer
		//
        hr = m_pReader1->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pRdrHdrInfo1 );
		if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMHeaderInfo from the reader (hr=0x%08x).\n" ), hr );
            break;
        }

		hr = m_pWriter->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&pWHeaderInfo );
		if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMHeaderInfo from the writer (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = m_pReader2->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pRdrHdrInfo2 );
		if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMHeaderInfo from the reader (hr=0x%08x).\n" ), hr );
            break;
        }
        
        hr = CopyAllAttributes( pWHeaderInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Copy codec info
        //
        hr = CopyCodecInfo( pWHeaderInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Header has been copied. Let's copy the script
        //
        hr = CopyScript( pWHeaderInfo , m_pRdrHdrInfo1, 0);
        if( FAILED( hr ) )
        {
	        break;
        }

		//
		// Get the offset time at which the script from
		// the second file is to be copied to the output file
		//
        WORD wStreamNum = 0;
		WMT_ATTR_DATATYPE type;
		WORD cbLength = sizeof( m_qwFirstTime );

		hr = m_pRdrHdrInfo1->GetAttributeByName( &wStreamNum,  g_wszWMDuration, &type, (BYTE *)&m_qwFirstTime, &cbLength );
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Error in getting Duration attribute for first file (hr=0x%08x).\n" ), hr );
			break;
		}

        hr = CopyScript( pWHeaderInfo , m_pRdrHdrInfo2, m_qwFirstTime);
        if( FAILED( hr ) )
        {
	        break;
        }

    }
    while( FALSE );
    
    SAFE_RELEASE( pWHeaderInfo );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::CopyAllAttributes()
// Desc: Copies all the header attributes which can be set, not related to a 
//       particular stream. It's illegal to read DRM attributes..
//------------------------------------------------------------------------------
HRESULT CAppend::CopyAllAttributes( IWMHeaderInfo * pWriterHeaderInfo )
{
    HRESULT hr = S_OK;

	do
	{

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMTitle );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMAuthor );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMDescription );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMRating );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMCopyright );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMAlbumTitle );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMTrack );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMPromotionURL );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMAlbumCoverURL );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMGenre );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMYear );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMGenreID );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMMCDI );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMBannerImageType );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMBannerImageData );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMBannerImageURL );
		if( FAILED( hr ) )
		{
			break;
		}

		hr = CopyAttribute( 0, 0, pWriterHeaderInfo, g_wszWMCopyrightURL );
		if( FAILED( hr ) )
		{
			break;
		}


		//
		// Shows how to copy stream-based attribute
		//

		for( WORD cnt = 0; cnt < m_dwStreamCount; cnt++ )
		{

			hr = CopyAttribute( m_pwStreamNumMap[ cnt ], 
                                m_pwStreamNumMap[ cnt + m_dwStreamCount ], 
                                pWriterHeaderInfo, 
                                pwszUserAttribute1 );
			if( FAILED( hr ) )
			{
				break;
			}
		}
		
	}while( FALSE );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::CopyAttribute()
// Desc: Copies an attribute from the reader to the writer.
//------------------------------------------------------------------------------
HRESULT CAppend::CopyAttribute( WORD nInStreamNum,  
                                WORD nOutStreamNum, 
                                IWMHeaderInfo * pWriterHeaderInfo, 
                                LPCWSTR pwszName )
{
    WORD    cbLength   = 0;
	HRESULT hr         = S_OK;
	BYTE*   pValue     = NULL;

    WMT_ATTR_DATATYPE type;
	//
	// Get the number of bytes to be allocated for pValue
	//
	hr = m_pRdrHdrInfo1->GetAttributeByName( &nInStreamNum,
											 pwszName,
											 &type,
											 NULL,
											 &cbLength );
	if( FAILED( hr ) && hr != ASF_E_NOTFOUND )
	{
		_tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ) , pwszName, hr );
		return( hr );
	}
	
	if( cbLength == 0 && hr == ASF_E_NOTFOUND )
	{
		hr = S_OK;
		return( hr );
	}

	pValue = new BYTE[ cbLength ];
	if( NULL == pValue )
	{
        _tprintf( _T( "Internal Error: Out of memory\n" ) );
		hr = E_OUTOFMEMORY;
		return( hr );
	}

	do
    {
	    
        //
	    // Get the value
	    //
	    hr = m_pRdrHdrInfo1->GetAttributeByName( &nInStreamNum,
												 pwszName,
												 &type,
												 pValue,
												 &cbLength );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr );
		    break;
	    }
	    //
	    // Set the attribute
	    //
	    hr = pWriterHeaderInfo->SetAttribute( nOutStreamNum,
										      pwszName,
										      type,
										      pValue,
										      cbLength );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T("SetAttribute failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr );
		    break;
	    }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pValue ); 

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::CopyCodecInfo()
// Desc: Copies codec information from the reader header to the writer header.
//------------------------------------------------------------------------------
HRESULT CAppend::CopyCodecInfo( IWMHeaderInfo * pWHdrInfo )
{
    HRESULT     hr = S_OK;
    DWORD       i, cCodecInfo;
    WORD        cchName, cchDescription, cbCodecInfo;
    LPWSTR      pwszName = NULL, pwszDescription = NULL;
    BYTE *      pbCodecInfo = NULL;

    WMT_CODEC_INFO_TYPE codecType;
    IWMHeaderInfo3      * pWHdrInfo3 = NULL,
                        * pRHdrInfo3 = NULL;

    hr = m_pRdrHdrInfo1->QueryInterface( IID_IWMHeaderInfo3,
            ( void ** )&pRHdrInfo3 );
    if( SUCCEEDED( hr ) )
    {
        hr = pWHdrInfo->QueryInterface( IID_IWMHeaderInfo3,
            ( void ** )&pWHdrInfo3 );
    }
    if( FAILED( hr ) )
    {
        _tprintf( _T( "QI for IWMHeaderInfo3 failed (hr=0x%08x).\n" ), hr );
        goto Exit;
    }

    hr = pRHdrInfo3->GetCodecInfoCount( &cCodecInfo );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "GetCodecInfoCount failed (hr=0x%08x).\n" ), hr );
        goto Exit;
    }

    for( i = 0; i < cCodecInfo; i++ )
    {
        //
        // Get codec info from the source
        //
        cchName = cchDescription = cbCodecInfo = 0;
        hr = pRHdrInfo3->GetCodecInfo( i, &cchName, NULL,
            &cchDescription, NULL, &codecType, &cbCodecInfo, NULL );
        if( SUCCEEDED( hr ) )
        {
            SAFE_ARRAYDELETE( pwszName );
            SAFE_ARRAYDELETE( pwszDescription );
            SAFE_ARRAYDELETE( pbCodecInfo );

            pwszName = new WCHAR [cchName+1];
            pwszDescription = new WCHAR [cchDescription+1];
            pbCodecInfo = new BYTE [cbCodecInfo];
            if( NULL == pwszName || NULL == pwszDescription || NULL == pbCodecInfo )
            {
                _tprintf( _T( "Internal Error: Out of memory\n" ) );
    	        hr = E_OUTOFMEMORY;
    	        goto Exit;
            }

            hr = pRHdrInfo3->GetCodecInfo( i, &cchName, pwszName,
                &cchDescription, pwszDescription, &codecType, &cbCodecInfo,
                pbCodecInfo );
        }

        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetCodecInfo failed (hr=0x%08x).\n" ), hr );
            goto Exit;
	    }

        pwszName[cchName] = pwszDescription[cchDescription] = L'\0';

        //
        // Add the codec info to the writer
        //
        hr = pWHdrInfo3->AddCodecInfo( pwszName, pwszDescription, codecType,
            cbCodecInfo, pbCodecInfo );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "AddCodecInfo failed (hr=0x%08x).\n" ), hr );
            goto Exit;
        }
    }

Exit:
    SAFE_ARRAYDELETE( pwszName );
    SAFE_ARRAYDELETE( pwszDescription );
    SAFE_ARRAYDELETE( pbCodecInfo );
    SAFE_RELEASE( pRHdrInfo3 );
    SAFE_RELEASE( pWHdrInfo3 );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CAppend::CopyScript()
// Desc: Copies script from the reader header to the writer header.
//------------------------------------------------------------------------------
HRESULT CAppend::CopyScript( IWMHeaderInfo * pWHdrInfo, IWMHeaderInfo * pRHdrInfo, QWORD qwTimeOffset )
{
    HRESULT hr              = S_OK;
    LPWSTR  pwszType        = NULL;
    LPWSTR  pwszCommand     = NULL;
    WORD    cchTypeLen      = 0;
    WORD    cScript         = 0;
    WORD    cchCommandLen   = 0;
    QWORD   cnsScriptTime   = 0;

    hr = pRHdrInfo->GetScriptCount( &cScript );
    if( FAILED( hr ) )
    {
	    _tprintf( _T( "GetScriptCount failed (hr=0x%08x).\n" ), hr );
	    return( hr );
    }


    for( WORD i = 0; i < cScript; i++)
    {
	    //
	    // Get the memory required for this script
	    //
	    hr = pRHdrInfo->GetScript( i,
								   NULL,
								   &cchTypeLen,
								   NULL,
								   &cchCommandLen,
								   &cnsScriptTime );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr );
		    break;
	    }
	    
	    pwszType = new WCHAR[cchTypeLen];
	    pwszCommand = new WCHAR[cchCommandLen];
	    if( NULL == pwszType || NULL == pwszCommand )
	    {
            _tprintf( _T( "Internal Error: Out of memory\n" ) );
		    hr = E_OUTOFMEMORY;
		    break;
	    }
	    //
	    // Get the script
	    //
	    hr = pRHdrInfo->GetScript( i,
								   pwszType,
								   &cchTypeLen,
								   pwszCommand,
								   &cchCommandLen,
								   &cnsScriptTime );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr );
		    break;
	    }
	    
        //
	    // Add the script to the writer
	    //
	    hr = pWHdrInfo->AddScript( pwszType, pwszCommand, cnsScriptTime + qwTimeOffset );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T("AddScript failed for Script no %d (hr=0x%08x).\n" ), i, hr );
		    break;
	    }

	    SAFE_ARRAYDELETE( pwszType );
	    SAFE_ARRAYDELETE( pwszCommand );	    
	    cchTypeLen = cchCommandLen = 0;
    }

    SAFE_ARRAYDELETE( pwszType );
    SAFE_ARRAYDELETE( pwszCommand );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::StartAppending()
// Desc: Copies the input files to the output file.
//------------------------------------------------------------------------------
HRESULT CAppend::StartAppending()
{
    HRESULT hr = S_OK;
    
    do
	{
		hr = m_pWriter->BeginWriting();
		if( FAILED( hr ) )
		{
			_tprintf( _T( "BeginWriting on IWMWriter failed (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = m_pReader1->Start( 0, 0, 1.0, 0 );
		if( FAILED( hr ) )
		{
			_tprintf( _T("Could not start IWMReader for first file (hr=0x%08x).\n" ), hr );
			break;
		}

		//
		// Wait for it to finish
		//
		WaitForSingleObject( m_hAsyncEvent, INFINITE );

		if( FAILED( m_hrAsync ) )
		{
			_tprintf( _T( "Stream copying failed for the first file (hr=0x%08x).\n" ), m_hrAsync );
			hr = m_hrAsync;
			break;
		}
	    
		//
		// Stop stuff
		//
		hr = m_pReader1->Stop( );
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not Stop IWMReader for the first file  (hr=0x%08x).\n" ), hr );
			break;
		}
		//
		// Wait for it to finish
		//
		WaitForSingleObject( m_hAsyncEvent, INFINITE );

		if( FAILED( m_hrAsync ) )
		{
			_tprintf( _T( "Could not Stop reader of the first file (hr=0x%08x).\n" ), m_hrAsync );
			hr = m_hrAsync;
			break;
		}

		//
		// Append the second file
		//
		hr = m_pReader2->Start( 0, 0, 1.0, 0 );
		if( FAILED( hr ) )
		{
			_tprintf( _T("Could not start IWMReader for second file (hr=0x%08x).\n" ), hr );
			break;
		}

		WaitForSingleObject( m_hAsyncEvent, INFINITE );
	    
		if( FAILED( m_hrAsync ) )
		{
			_tprintf( _T( "Stream copying failed for the second file (hr=0x%08x).\n" ), m_hrAsync );
			hr = m_hrAsync;
			break;
		}

		hr = m_pReader2->Stop();
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not Stop IWMReader for the second file (hr=0x%08x).\n" ), hr );
			break;
		}
	    
		WaitForSingleObject( m_hAsyncEvent, INFINITE );
	    
		if( FAILED( m_hrAsync ) )
		{
			_tprintf( _T( "Could not Stop IWMReader for the second file (hr=0x%08x).\n" ), m_hrAsync );
			hr = m_hrAsync;
			break;
		}

		hr = m_pWriter->EndWriting();
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Could not EndWriting on IWMWriter (hr=0x%08x).\n" ), hr );
			break;
		}

		hr = CopyAllMarkers();
		if( FAILED( hr ) )
		{
			break;
		}

	}while( FALSE );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::SetReceiveStreamSample()
// Desc: Sets the specified stream to be received in compressed mode.
//------------------------------------------------------------------------------
HRESULT CAppend::SetReceiveStreamSample( IWMReaderAdvanced* pReaderAdv, IWMProfile* pProfile, DWORD nStreamIndex )
{
    IWMStreamConfig * pStream  = NULL;
    
    WORD wStreamNumber = 0; 
    
    HRESULT hr = S_OK;
    
    do
    {
        hr = pProfile->GetStream( nStreamIndex, &pStream );
	    if( FAILED( hr ) )
	    {
            _tprintf( _T( "Could not get Stream %d from IWMProfile (hr=0x%08x).\n" ), nStreamIndex, hr );
            break;
	    }

	    //
	    // Get the stream number
	    //
	    hr = pStream->GetStreamNumber( &wStreamNumber );
	    if( FAILED( hr ) )
	    {
            _tprintf( _T( "Could not get stream number from IWMStreamConfig for Stream Index %d (hr=0x%08x).\n" ), nStreamIndex, hr );
            break;
	    }

	    SAFE_RELEASE( pStream );

	    //
	    // Set the stream to be received in compressed mode
	    //
	    hr = pReaderAdv->SetReceiveStreamSamples( wStreamNumber, TRUE );
	    if( FAILED( hr ) )
	    {
            _tprintf( _T( "Could not SetReceivedStreamSamples for Stream Index %d (hr=0x%08x).\n" ), nStreamIndex, hr );
            break;
	    }
    }
    while( FALSE );
    
    SAFE_RELEASE( pStream );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::CopyAllMarkers()
// Desc: Copies markers from the input headers to the output header.
//------------------------------------------------------------------------------
HRESULT CAppend::CopyAllMarkers()
{
    HRESULT hr      = S_OK;

    IWMMetadataEditor *     pEditor           = NULL;
    IWMHeaderInfo *         pWriterHeaderInfo = NULL;

	do
    {
	    //
	    // Markers can be copied only by the Metadata Editor.
	    // Let's create one.
	    //
	    hr = WMCreateEditor( &pEditor );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
		    break;
	    }

	    hr = pEditor->Open( m_pwszOutFile );
	    if( FAILED ( hr ) )
	    {
		    _tprintf( _T( "Could not open outfile %ws (hr=0x%08x).\n" ), m_pwszOutFile ,hr );
		    break;
	    }

	    hr = pEditor->QueryInterface( IID_IWMHeaderInfo, ( void ** ) &pWriterHeaderInfo );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
		    break;
	    }

        hr = CopyMarkersFromHdr( m_pRdrHdrInfo1, pWriterHeaderInfo, 0 );
        if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
		    break;
	    }
	    
        hr = CopyMarkersFromHdr( m_pRdrHdrInfo2, pWriterHeaderInfo, m_qwFirstTime );
        if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
		    break;
	    }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
	        _tprintf( _T( "Could not close the Editor (hr=0x%08x).\n" ), hr);
            break;
        }
    }
    while( FALSE);

    SAFE_RELEASE( pWriterHeaderInfo);
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::CopyMarkersFromHdr()
// Desc: Called by CopyAllMarkers to copy markers from one input file to the
//       output file.
//------------------------------------------------------------------------------
HRESULT CAppend::CopyMarkersFromHdr(IWMHeaderInfo * pRHdrInfo, IWMHeaderInfo * pWHdrInfo, QWORD qwTimeOffset )
{
    HRESULT hr = S_OK;
    WORD    cMarker = 0;

    hr = pRHdrInfo->GetMarkerCount( &cMarker );
	if( FAILED( hr ) )
	{
		_tprintf( _T("GetMarkerCount failed (hr=0x%08x).\n" ), hr);
		return(  hr );
	}

    if( cMarker == 0 )
	{
		return( S_OK );
	}

	LPWSTR  pwszMarkerName   = NULL;
	WORD    cchMarkerNameLen = 0;
	QWORD   cnsMarkerTime    = 0;

	for( WORD i = 0; i < cMarker; i++)
	{
		//
		// Get the memory required for this marker
		//
		hr = pRHdrInfo->GetMarker( i,
                                   NULL,
                                   &cchMarkerNameLen,
                                   &cnsMarkerTime );
		if( FAILED( hr ) )
		{
			_tprintf( _T("GetMarker failed for Marker no %d (hr=0x%08x).\n" ), i, hr );
			break;
		}
		
		pwszMarkerName = new WCHAR[cchMarkerNameLen];
		if( pwszMarkerName == NULL )
		{
			hr = E_OUTOFMEMORY;
            _tprintf( _T( "Internal Error: Out of memory\n" ) );
			break;
		}

		hr = pRHdrInfo->GetMarker( i,
                                   pwszMarkerName,
                                   &cchMarkerNameLen,
                                   &cnsMarkerTime );
		if( FAILED( hr ) )
		{
			_tprintf( _T( "GetMarker failed for Marker no %d (hr=0x%08x).\n" ), i, hr );
			break;
		}

		hr = pWHdrInfo->AddMarker( pwszMarkerName, cnsMarkerTime + qwTimeOffset );
		if( FAILED( hr ) )
		{
			_tprintf( _T( "AddMarker failed for Marker no %d (hr=0x%08x).\n" ), i, hr );
			break;
		}
		
		SAFE_ARRAYDELETE( pwszMarkerName );
		pwszMarkerName = NULL;
		cchMarkerNameLen = 0;
	}

	SAFE_ARRAYDELETE( pwszMarkerName );

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::MapStreamNum()
// Desc: Maps the stream numbers of the second file to those of the output file.
//------------------------------------------------------------------------------
WORD CAppend::MapStreamNum( WORD dwNum )
{
    for( WORD i =0; i < m_dwStreamCount; i++ )
    {
        if( m_pwStreamNumMap[i + m_dwStreamCount] == dwNum )
        {
            return( (WORD)m_pwStreamNumMap[i] );
        }
    }

    return( 0 );
}

//------------------------------------------------------------------------------
// Name: CAppend::IsFileProtected()
// Desc: Ascertains whether the specified file is protected by DRM.
//------------------------------------------------------------------------------
HRESULT CAppend::IsFileProtected( __in LPWSTR pswzFileName )
{
    IWMMetadataEditor *     pEditor     = NULL;
    IWMHeaderInfo *         pHeaderInfo = NULL;

    HRESULT hr = S_OK;
    //
    // Open the files using metadata editor.
    // Opening it using IWMReader would decrement the usage count,
    // if it's a protected file.
    //
    do
    {
        hr = WMCreateEditor( &pEditor );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
		    break;
	    }

	    hr = pEditor->Open( pswzFileName );
	    if( FAILED ( hr ) )
	    {
		    _tprintf( _T( "Could not open file %ws (hr=0x%08x).\n" ), pswzFileName ,hr );
		    break;
	    }

	    hr = pEditor->QueryInterface( IID_IWMHeaderInfo, ( void ** ) &pHeaderInfo );
	    if( FAILED( hr ) )
	    {
		    _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
		    break;
	    }

        WORD wStreamNum = 0;
		WMT_ATTR_DATATYPE type;
		BYTE value[4];
		WORD cbLength = 4;
        //
        // Check the protected attribute of the header
        //
		hr = pHeaderInfo->GetAttributeByName( &wStreamNum,  g_wszWMProtected, &type, value, &cbLength );
		if( FAILED( hr ) )
		{
			_tprintf( _T( "Error in getting Protected attribute for file %ws (hr=0x%08x).\n" ), pswzFileName, hr );
			break;
		}

        if( value[0] )
		{
			_tprintf( _T( "Protected attributes set for input file %ws .\n" ), pswzFileName );
            hr = E_INVALIDARG;
			break;
		}
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo );
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CAppend::QueryInterface()
// Desc: Implementation of the IUnknown method.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CAppend::QueryInterface( REFIID riid, void ** ppvObject )
{
	if ( riid == IID_IWMReaderCallback )
    {
		*ppvObject = ( IWMReaderCallback* )this;
		AddRef();
    }
	else if( riid == IID_IWMReaderCallbackAdvanced )
    {
		*ppvObject = ( IWMReaderCallbackAdvanced * )this;
		AddRef();	
    }
    else
	{
		*ppvObject = NULL;
        return( E_NOINTERFACE );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAppend::AddRef()
// Desc: Implementation of the IUnknown method.
//------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CAppend::AddRef( void )
{
    return( InterlockedIncrement( &m_cRef ) );
}

//------------------------------------------------------------------------------
// Name: CAppend::Release()
// Desc: Implementation of the IUnknown method.
//------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CAppend::Release( void )
{
    if ( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return 0;
    }
    
    return( m_cRef );
}


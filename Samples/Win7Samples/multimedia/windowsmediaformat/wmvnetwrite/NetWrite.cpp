//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            NetWrite.cpp
//
// Abstract:            CNetWrite class implementation. 
//                      
//
//*****************************************************************************

#include "wmsdk.h"
#include "NetWrite.h"
#include <TCHAR.H>
#include <stdio.h>
#include <assert.h>
#include <conio.h>


//----------------------------------------------------------------------------
// Name: CNetWrite::CNetWrite()
// Desc: Constructor.
//----------------------------------------------------------------------------

CNetWrite::CNetWrite()
{
    m_pReaderHeaderInfo	= NULL;
    m_pWriterHeaderInfo	= NULL;
    m_pWriterAdvanced	= NULL;
    m_pReaderAdvanced	= NULL;
    m_pReader		= NULL;
    m_pWriter		= NULL;
    m_pNetSink		= NULL;
    m_pPushSink         = NULL;
    m_pPushSinkCallbackCtrl = NULL;
    m_hEvent		= NULL;
    m_bEOF		= false;
    m_qwTime		= 0;
    m_hrAsync		= S_OK;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::~CNetWrite()
// Desc: Destructor.
//----------------------------------------------------------------------------

CNetWrite::~CNetWrite()
{
    CloseHandle( m_hEvent );
    
    SAFE_RELEASE( m_pWriterHeaderInfo );
    SAFE_RELEASE( m_pWriterAdvanced );
    SAFE_RELEASE( m_pWriter );
    SAFE_RELEASE( m_pNetSink );
    SAFE_RELEASE( m_pPushSinkCallbackCtrl );
    SAFE_RELEASE( m_pPushSink );
    SAFE_RELEASE( m_pReaderHeaderInfo );
    SAFE_RELEASE( m_pReaderAdvanced );
    SAFE_RELEASE( m_pReader );
    
    CoUninitialize();
}


//----------------------------------------------------------------------------
// Name: CNetWrite::OnSample()
// Desc: Called when the reader object delivers an uncompressed sample. 

// Note: This method implements the IWMReaderCallback::OnSample method. See the 
//       SDK documentation for a description of the parameters.
//
//       The CNetWrite object configures the reader to deliver compressed
//       samples, which happens via the OnStreamSample callback method.
//       Therefore, if this method is called, it is an error.
//----------------------------------------------------------------------------

HRESULT CNetWrite::OnSample( DWORD dwOutputNum, 
                            QWORD cnsSampleTime,
                            QWORD cnsSampleDuration, 
                            DWORD dwFlags,
                            INSSBuffer __RPC_FAR *pSample, void __RPC_FAR *pvContext )
{
    if( m_hEvent != NULL )
    {
        //
        //The samples are expected in OnStreamSample.
        //
        _tprintf( _T( "Error: Received a decompressed sample from the reader.\n" ) );
        
        m_hrAsync = E_UNEXPECTED;
        SetEvent( m_hEvent );
    }
    
    return S_OK;
}


//----------------------------------------------------------------------------
// Name: CNetWrite::OnStatus()
// Desc: Receives a status message from the reader object or the push sink. 
//
// Note: This method implements the IWMStatusCallback::OnStatus method. See the 
//       SDK documentation for a description of the parameters.
//----------------------------------------------------------------------------


HRESULT CNetWrite::OnStatus( WMT_STATUS Status, 
                            HRESULT hr, 
                            WMT_ATTR_DATATYPE dwType, 
                            BYTE __RPC_FAR *pValue, 
                            void __RPC_FAR *pvContext )
{
    // Check whether the push sink object is the caller.
    BOOL fFromPushSink = ( m_pPushSink != NULL && pvContext == ( void * ) m_pPushSink );

    // For some status messages, we expect the CNetWrite object is waiting
    // to receive the message. Signal to the waiting thread by calling SetEvent.
    // We also signal to the thread if there is an error message. 

    // Handle status messages from the push sink. 
    if( fFromPushSink )
    {
        switch(Status)
        {
        case WMT_LOCATING:
            _tprintf( _T(  "Locating server.\n" ) );
            break;

        case WMT_CONNECTING:
            _tprintf( _T(  "Connecting to server.\n" ) );
            break;

        case WMT_OPENED:

			_tprintf( _T(  "Connected to server.\n" ) );

            m_hrAsync = hr;
            SetEvent( m_hEvent );  // Unblock the thread that is waiting.

            break;

        case WMT_ERROR:
            _tprintf( _T(  "Error 0x%x during push distribution\n" ), hr );

            m_bEOF = true;
            m_hrAsync = hr;
            SetEvent( m_hEvent ); 

            break;
        }
        
        return S_OK;
    }

    // Handle status messages from the reader. 

    switch(Status)
    {
        
    case WMT_OPENED:
      
		if( SUCCEEDED( hr ))
		{
			_tprintf( _T(  "The reader completed opening the input file.\n" ) );
		}

		m_hrAsync = hr;
        SetEvent( m_hEvent );  // Unblock the thread that is waiting.
        
        break;
        
    case WMT_EOF:
        
        m_bEOF = true;
        _tprintf( _T(  "EndOfStream detected in reader.\n" ) );
        
        m_hrAsync = hr;
        SetEvent( m_hEvent );
        
        break;
        
    case WMT_STARTED:
        // Ask the reader object to deliver another block of time.
        m_qwTime = 0;
        m_qwTime += 1000 * 10000;
        
        hr = m_pReaderAdvanced->DeliverTime( m_qwTime );
        m_hrAsync = hr;

        break;

    case WMT_CLOSED:
       
        m_hrAsync = hr;
        SetEvent( m_hEvent );  // Unblock the thread that is waiting.
        
        break;

    case WMT_ERROR:
        _tprintf( _T(  "Error 0x%x reported by the reader\n" ), hr );

        m_bEOF = true;
        m_hrAsync = hr;
        SetEvent( m_hEvent );

        break;
    }    

    return S_OK;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::OnStreamSample()
// Desc: Called when the reader object delivers a compressed sample. 
//
// Note: This method implements the IWMReaderCallbackAdvanced::OnStreamSample 
//       method. See the SDK documentation for a description of the parameters.
//----------------------------------------------------------------------------

HRESULT CNetWrite::OnStreamSample( WORD wStreamNum, 
                                  QWORD cnsSampleTime, 
                                  QWORD cnsSampleDuration, 
                                  DWORD dwFlags, 
                                  INSSBuffer __RPC_FAR *pSample, 
                                  void __RPC_FAR *pvContext )
{
    _tprintf( _T( "StreamSample: num=%d, time=%d, duration=%d, flags=%d.\n" ),
        wStreamNum, ( DWORD )cnsSampleTime, cnsSampleDuration, dwFlags );

    if( m_pWriterAdvanced != NULL )
    {
        // Give the sample to the writer object.

        HRESULT hr = m_pWriterAdvanced->WriteStreamSample( wStreamNum,
                                                           cnsSampleTime,
                                                           0,
                                                           cnsSampleDuration,
                                                           dwFlags,
                                                           pSample );

        if( FAILED( hr ) )
        {
            _tprintf( _T(  "Error 0x%x reported by the writer\n" ), hr );

            m_bEOF = true;
            m_hrAsync = hr;
            SetEvent( m_hEvent );
        }
    }

    return S_OK;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::OnTime()
// Desc: Called when the reader object has delivered all of the data that this
//       object requested.
//
// Note: This method implements the IWMReaderCallbackAdvanced::OnTime method. 
//       See the SDK documentation for a description of the parameters.
//
//       This method gets called because the CNetWrite object is driving the
//       clock on the reader object.
//----------------------------------------------------------------------------

HRESULT CNetWrite::OnTime( QWORD cnsCurrentTime, void __RPC_FAR *pvContext)
{
    // Until the end of the file is reached, ask for another block of time.
    if( !m_bEOF )
    {
        m_qwTime += 10000000;
        HRESULT hr=m_pReaderAdvanced->DeliverTime( m_qwTime );
		if(FAILED(hr))
			return hr;
    }
    
    return S_OK;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::Init()
// Desc: Initializes the CNetWrite object.
//----------------------------------------------------------------------------

HRESULT CNetWrite::Init()
{
    HRESULT hr = S_OK;
    
    // Initialize the COM library.
    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: hr = 0x%08x\n" ), hr );
        return hr;
    }
    
    //
    // Create the reader and writer. Query for additional interfaces.
    //
	hr = WMCreateReader( NULL, 0, &m_pReader );

	if( FAILED( hr ) )
	{
		_tprintf( _T( "Could not create reader (hr=0x%08x).\n" ), hr );
		return hr;
	}
    
    hr = m_pReader->QueryInterface( IID_IWMReaderAdvanced, ( void** )&m_pReaderAdvanced );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not QI for IWMReaderAdvanced (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
	hr = WMCreateWriter( NULL, &m_pWriter );
	if( FAILED( hr ) )
	{
		_tprintf( _T(  "Could not create Writer (hr=0x%08x).\n" ), hr );
		return hr;
	}
    
    hr = m_pWriter->QueryInterface( IID_IWMWriterAdvanced, ( void** ) &m_pWriterAdvanced );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not QI for IWMWriterAdvanced (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pReader->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pReaderHeaderInfo );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pWriter->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pWriterHeaderInfo );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    return hr;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::WritetoNet()
// Desc: Writes a file to the network. Call the Configure method first, to set
//       the file name and other parameters.
//----------------------------------------------------------------------------
HRESULT CNetWrite::WritetoNet()
{
    if(   m_hEvent			== NULL ||
          m_pWriterAdvanced	== NULL ||
          m_pReaderAdvanced	== NULL || 
        ( m_pNetSink == NULL && m_pPushSink == NULL ) )
    {
        return E_UNEXPECTED;
    }
    
    HRESULT hr = S_OK;

    //
    // Start writing. 
    //

    // First, prepare the writer object for writing. Then start the reader.

    hr = m_pWriter->BeginWriting( );
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "BeginWriting on IWMWriter failed (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pReader->Start( 0, 0, 1.0, 0 );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not start IWMReader (hr=0x%08x).\n" ), hr );
        return hr;
    }

    // Wait for all of the read and write operations to finish. 
    WaitForSingleObject( m_hEvent, INFINITE );

    // Check the status of the operation. 
    if( FAILED( m_hrAsync ) )
    {
        _tprintf( _T(  "Net writing failed (hr=0x%08x).\n" ), m_hrAsync );
        return hr;
    }

    //
    // Stop the reader and the writer, and close everything.
    //

    hr = m_pReader->Stop();
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not Stop IWMReader (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pWriter->Flush();
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not Flush on IWMWriter (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pWriter->EndWriting( );
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not EndWriting on IWMWriter (hr=0x%08x).\n" ), hr );
        return hr;
    }
    
    hr = m_pReader->Close();
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not close the file (hr=0x%08x).\n" ),  hr );
        return hr;
    }

    // Remove the network sink from the writer.
    if( m_pNetSink != NULL )
    {   
        hr = m_pWriterAdvanced->RemoveSink( m_pNetSink );
        if( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not remove the Network Sink (hr=0x%08x).\n" ),  hr );
            return hr;
        }
    
        hr = m_pNetSink->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not close on IWMWriterNetworkSink (hr=0x%08x).\n" ), hr );
            return hr;
        }
    }
    
    // Remove the push sink from the writer.
    if( m_pPushSink != NULL )
    {   
        hr = m_pWriterAdvanced->RemoveSink( m_pPushSink );
        if( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not remove the Push Sink (hr=0x%08x).\n" ),  hr );
            return hr;
        }
    
        // End the push session and cancel the advise connection with the push sink.

        hr = m_pPushSink->EndSession();
        if( FAILED( hr ) )
        {
            _tprintf( _T(  "Could not close on IWMWriterPushSink (hr=0x%08x).\n" ), hr );
            return hr;
        }

        if( m_pPushSinkCallbackCtrl != NULL )
        {
            hr = m_pPushSinkCallbackCtrl->Unadvise( this, ( void * ) m_pPushSink );
			if(FAILED(hr))
				return hr;
        }
    }

    //
    // Wait for the reader to tell us that it is closed.
    //
    WaitForSingleObject( m_hEvent, 10000 );
    
    return hr;
}


//----------------------------------------------------------------------------
// Name: CNetWrite::Configure()
// Desc: Configures the object to broadcast a file.
//
// dwPortNum:     Port number for the network sink. Use -1 for no network sink.
// pwszFile:      The name of a local ASF file to broadcast. Cannot be NULL.
// nMaxClient:    Maximum number of client connections.
// pwszServerURL: URL of a publishing point, for the push sink. Use NULL for no push sink.
//
// Note:  If dwPortNum is -1, pwszServerURL cannot be NULL, and vice-versa.
//----------------------------------------------------------------------------

HRESULT CNetWrite::Configure( DWORD dwPortNum, const WCHAR *pwszFile, UINT nMaxClient, const WCHAR *pwszServerURL )
{
    if( pwszFile == NULL || ( dwPortNum == (DWORD)-1 && pwszServerURL == NULL ) )
    {
        return E_INVALIDARG;
    }

	if( dwPortNum > 0xFFFF )
	{
		_tprintf( _T( "Invalid port number.\n" ) );
		return E_INVALIDARG;
	}

	if( m_pWriterAdvanced == NULL || m_pReaderAdvanced == NULL )
    {
        return E_UNEXPECTED;
    }

    HRESULT		hr = S_OK;

    // If a port number was specified, create the network sink object.

    if( dwPortNum != (DWORD)-1 )
    {
        hr = WMCreateWriterNetworkSink( &m_pNetSink );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create Writer Network Sink (hr=0x%08x).\n" ), hr );
            return hr;
        }
    }

    // If a publishing point URL was specified, create the push sink object.

    if( pwszServerURL != NULL )
    {
        hr = WMCreateWriterPushSink( &m_pPushSink );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create Writer Push Sink (hr=0x%08x).\n" ), hr );
            return hr;
        }

        // Set up a callback for event notifications.

        hr = m_pPushSink->QueryInterface( IID_IWMRegisterCallback, 
                                          ( void ** ) &m_pPushSinkCallbackCtrl );

        if( FAILED( hr ) )
        {
			return hr;
		}
         
		hr = m_pPushSinkCallbackCtrl->Advise( this, ( void * ) m_pPushSink );
		if( FAILED( hr ) )
		{
			return hr;
        }
    }
    
    //
    // Create an event for handling asynchronous calls.
    //

    m_hrAsync = S_OK;
 
	if( m_hEvent != NULL)
	{
		CloseHandle(m_hEvent);
	}

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL );

	if( NULL ==  m_hEvent )
    {
        DWORD err = GetLastError();
        _tprintf( _T( "Could not Create Event: (hr=0x%08x).\n" ), err );
        return err;
    }

    if( m_pNetSink != NULL )
    {
        //
        // Configure the network sink.
        //
        hr = m_pNetSink->SetNetworkProtocol( WMT_PROTOCOL_HTTP );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Set Network protocol (hr=0x%08x).\n" ), hr );
            return hr;
        }
    
        hr = m_pNetSink->Open( &dwPortNum);
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Network sink failed to open port no %d (hr=0x%08x).\n" ),
                dwPortNum, hr );
            return hr;
        }
    
        // Find the host URL, to display to the user. 
        DWORD cchURL = 0;
    
        hr = m_pNetSink->GetHostURL( NULL, &cchURL );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not get the host URL from IWMWriterNEtworkSink (hr=0x%08x).\n" ),
                hr );
            return hr;
        }
    
        WCHAR *pwszURL = new WCHAR[cchURL];
        if( pwszURL == NULL )
        {
            _tprintf( _T( "Insufficient Memory" ) );
            return E_OUTOFMEMORY;
        }
    
        hr = m_pNetSink->GetHostURL( pwszURL, &cchURL );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not get the host URL from IWMWriterNetworkSink (hr=0x%08x).\n" ),
                hr );
			SAFE_ARRAYDELETE (pwszURL);
            return hr;
        }
    
        _tprintf( _T( "Connect to %ws\n" ), pwszURL );
    
        Sleep( 1000 );
    
        SAFE_ARRAYDELETE (pwszURL);
    
        //
        // Set the maximum number of clients that can connect to the port.
        //
        hr = m_pNetSink->SetMaximumClients( nMaxClient );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Set maximum clients (hr=0x%08x).\n" ), hr );
            return hr;
        }
    
        //
        // Add the network sink to the writer.
        //
        hr = m_pWriterAdvanced->AddSink(m_pNetSink);
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Add Sink (hr=0x%08x).\n" ), hr );
            return hr;
        }
    }

    if( m_pPushSink != NULL )
    {
        //
        // Configure the push sink.
        //
        _tprintf( _T( "Connecting to %ws\n" ), pwszServerURL );

        hr = m_pPushSink->Connect( pwszServerURL, NULL, TRUE );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not connect to server (hr=0x%08x).\n" ), hr );
            return hr;
        }

        //
        // Wait for the connection to the server to be established.
        //

        if( WaitForSingleObject( m_hEvent, 30000 ) == WAIT_TIMEOUT )
        {
            _tprintf( _T( "Timed out whiletrying to connect to the server\n" ) );
            return E_FAIL;
        }

        if( FAILED( m_hrAsync ) )
        {
            return m_hrAsync;
        }

        //
        // Add the push sink to the writer object.
        //
        hr = m_pWriterAdvanced->AddSink(m_pPushSink);
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not Add Sink (hr=0x%08x).\n" ), hr );
            return hr;
        }
    }
    
    //
    // Open the requested file.
    //
    hr = m_pReader->Open(pwszFile, this, NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not open file %ws (hr=0x%08x).\n" ), pwszFile ,hr );
        return hr;
    }
    
    //
    // Wait for the open to finish.
    //
    if( WaitForSingleObject( m_hEvent, 30000 ) == WAIT_TIMEOUT )
    {
        _tprintf( _T( "Timed out whiletrying to open the input file\n" ) );
        return E_FAIL;
    }
    
    if( FAILED( m_hrAsync ) )
    {
		hr = m_hrAsync;
        _tprintf( _T( "Open failed (hr=0x%08x).\n" ), m_hrAsync );
        return hr;
    }
    //
    // Turn on manual stream selection, so we get all streams. This prevents the reader
    // from disabling any streams if there are mutual exclusions on some streams.
    //	
    hr = m_pReaderAdvanced->SetManualStreamSelection( TRUE );
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Failed to set manual stream selection (hr=0x%08x).\n" ), hr );
        return hr;
    }
    

    IWMProfile			*pProfile = NULL;
    IWMStreamConfig		*pStream  = NULL;

    // Declare a dummy 'do' loop. On failure, we can break from the loop.
	do
	{
			hr = m_pReader->QueryInterface( IID_IWMProfile, ( VOID ** )&pProfile );
			if( FAILED( hr ) )
			{
				_tprintf( _T(  "Could not QI for IWMProfile (hr=0x%08x).\n" ), hr );
				break;
			}
		    
            // Loop through all the output streams on the reader. For each stream, configure 
            // the reader to deliver compressed samples. This prevents the reader from decoding
            // each sample. (We want to give the writer compressed samples.)

			DWORD dwStreams = 0;
			hr = pProfile->GetStreamCount( &dwStreams );
			if( FAILED( hr ) )
			{
				_tprintf( _T(  "GetStreamCount on IWMProfile failed (hr=0x%08x).\n" ), hr );
				break;
			}
		    
			for ( DWORD i = 0; i < dwStreams; i++ )
			{
				hr = pProfile->GetStream( i, &pStream );
				if( FAILED( hr ) )
				{
					_tprintf( _T(  "Could not get Stream %d of %d from IWMProfile (hr=0x%08x).\n" ),
						i, dwStreams, hr );
					break;
				}
		        
				WORD wStreamNumber = 0;
				//
				//Get the stream number of the current stream.
				//
				hr = pStream->GetStreamNumber( &wStreamNumber );
				if( FAILED( hr ) )
				{
					_tprintf( _T(  "Could not get stream number from IWMStreamConfig %d of %d (hr=0x%08x).\n" ),
						i, dwStreams, hr );
					break;
				}
		        
				SAFE_RELEASE( pStream );
				//
				//Set the stream to be received in compressed mode.
				//
				hr = m_pReaderAdvanced->SetReceiveStreamSamples( wStreamNumber, TRUE );
				if( FAILED( hr ) )
				{
					_tprintf( _T(  "Could not SetReceivedStreamSamples for stream number %d (hr=0x%08x).\n" ),
						wStreamNumber, hr );
					break;
				}
			}
		    
			if( FAILED( hr ) )
				break;

			//
			// Turn on the user clock. The CNetWrite object will drive the clock, in order
            // to make the reader run as quickly as possible.
			//
			hr = m_pReaderAdvanced->SetUserProvidedClock( TRUE );
			if( FAILED( hr ) )
			{
				_tprintf( _T( "SetUserProvidedClock failed (hr=0x%08x).\n" ), hr );
				break;
			}
		    
			//
			// Set same profile on the writer object that we got from the reader.
			//
			hr = m_pWriter->SetProfile( pProfile );
			if( FAILED( hr ) )
			{
				_tprintf( _T(  "Could not set profile on IWMWriter (hr=0x%08x).\n" ), hr );
				break;
			}
	} while(FALSE);

	SAFE_RELEASE( pStream );
    SAFE_RELEASE( pProfile );

	if(FAILED(hr))
	{
		return hr;		// Something has failed within the do-while loop. 
	}
    
    DWORD   cInputs = 0;
    
    hr = m_pWriter->GetInputCount( &cInputs );
    if( FAILED( hr ) )
    {
        _tprintf( _T(  "Could not get input count from IWMWriter (hr=0x%08x).\n" ), hr );
        return hr;
    }

    // Loop through all of the inputs on the writer. For each input, set the 
    // input properties to NULL. This tells the writer not to encode the
    // samples it receives. We do this because we are getting encoded samples
    // from the reader.

    for( DWORD i = 0; i < cInputs; i++ )
    {
        hr = m_pWriter->SetInputProps( i, NULL );
		if(FAILED(hr))
		{
			return hr;
		}
    }
    
    //
    // Copy header attributes from the reader to the writer. 
    //
    hr = WriteHeader( g_wszWMTitle);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMAuthor);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMDescription);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMRating);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMCopyright);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMAlbumTitle);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMTrack);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMPromotionURL);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMAlbumCoverURL);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMGenre);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMYear);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMGenreID);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMMCDI);
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMBannerImageType );
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMBannerImageData );
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMBannerImageURL );
    if( FAILED( hr ) )
        return hr;
    
    hr = WriteHeader( g_wszWMCopyrightURL );
    if( FAILED( hr ) )
        return hr;
    
    //
    // Header has been written. Write the script (if any).
    //
    hr = WriteScript();
    
    return hr;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::WriteHeader()
// Desc: Copies specified metadata from the reader to the writer.
//
// pwszName: Specifies the name of the metadata attribute to copy.
//----------------------------------------------------------------------------

HRESULT CNetWrite::WriteHeader(const WCHAR *pwszName)
{
    WORD				nstreamNum = 0;
    WORD				cbLength = 0;
    WMT_ATTR_DATATYPE	type;
    HRESULT				hr = S_OK;
    BYTE*				pValue = NULL;
    
    //
    // Get the number of bytes to allocate for the attribute data.
    //
    hr = m_pReaderHeaderInfo->GetAttributeByName( &nstreamNum,
        pwszName,
        &type,
        NULL,
        &cbLength );
    if( FAILED( hr ) && hr != ASF_E_NOTFOUND )
    {
        _tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr);
        return hr;
    }
    
    if( cbLength == 0 && hr == ASF_E_NOTFOUND )
    {
        return S_OK;
    }
    
    // Allocate a buffer.
    pValue = new BYTE[ cbLength ];
    if( NULL == pValue )
    {
        _tprintf( _T( "Unable to allocate memory for the Attribute name %ws" ), pwszName);
        return E_OUTOFMEMORY;
    }

    // Declare a dummy 'do' loop. On failure, we can break from the loop.
    do
    {
        // Get the value of the attribute from the reader.
        hr = m_pReaderHeaderInfo->GetAttributeByName( &nstreamNum,
            pwszName,
            &type,
            pValue,
            &cbLength );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ),
                pwszName, hr);
            break;
        }

        // Set the attribute on the writer.
        hr = m_pWriterHeaderInfo->SetAttribute( nstreamNum,
						pwszName,
                                                type,
                                                pValue,
                                                cbLength );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "SetAttribute failed for Attribute name %ws (hr=0x%08x).\n" ),
                pwszName, hr);
            break;
        }
    }
    while(FALSE);
    
    SAFE_ARRAYDELETE( pValue );
    return hr;
}

//----------------------------------------------------------------------------
// Name: CNetWrite::WriteScript()
// Desc: Copies all scripts from the reader's header to the writer's header.
//----------------------------------------------------------------------------
HRESULT CNetWrite::WriteScript()
{
    
    HRESULT	hr = S_OK;
    WCHAR*	pwszCommand = NULL;
    WCHAR*	pwszType = NULL;
    QWORD	cnsScriptTime = 0;
    WORD	cScript = 0;
    WORD  	cchTypeLen = 0;
    WORD	cchCommandLen = 0;
    

    // Get the number of scripts in the header. This might be zero.
    hr = m_pReaderHeaderInfo->GetScriptCount( &cScript );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "GetScriptCount failed (hr=0x%08x).\n" ), hr);
        return hr;
    }
    
    // Loop through the scripts. For each script, copy it to the writer's header.
    for( int i = 0; i < cScript; i++)
    {
        //
        // Get the memory required for this script.
        //
        hr = m_pReaderHeaderInfo->GetScript( (WORD)i ,
					                         NULL ,
                                             &cchTypeLen ,
                                             NULL ,
                                             &cchCommandLen ,
                                             &cnsScriptTime );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr );
            break;
        }
        
        // Allocate buffers.
        pwszType	= new WCHAR[cchTypeLen];
        pwszCommand = new WCHAR[cchCommandLen];
        
        if( pwszType == NULL || pwszCommand == NULL)
        {
            _tprintf( _T( "Insufficient Memory" ) );
            hr = E_OUTOFMEMORY;
            break;
        }
        //
        // Get the script.
        //
        hr = m_pReaderHeaderInfo->GetScript( (WORD)i ,
					                         pwszType ,
                                             &cchTypeLen ,
                                             pwszCommand ,
                                             &cchCommandLen ,
                                             &cnsScriptTime );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "GetScript failed for Script no %d (hr=0x%08x).\n" ), i, hr);
            break;
        }
        //
        // Add the script to the writer.
        //
        hr = m_pWriterHeaderInfo->AddScript( pwszType ,
					     pwszCommand ,
                                             cnsScriptTime );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "AddScript failed for Script no %d (hr=0x%08x).\n" ), i, hr);
            break;
        }
        
        SAFE_ARRAYDELETE( pwszType );
        SAFE_ARRAYDELETE( pwszCommand );
        
        pwszType = pwszCommand = NULL;
        
        cchTypeLen		= 0;
        cchCommandLen	= 0;
    }
    
    SAFE_ARRAYDELETE( pwszType );
    SAFE_ARRAYDELETE( pwszCommand );
    
    return hr;	
}

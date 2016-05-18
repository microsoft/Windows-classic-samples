//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            AudioPlay.cpp
//
// Abstract:            Implementation of the CAudioPlay class.
//
//*****************************************************************************

#include "stdafx.h"
#include <stdio.h> 
#include <string.h>
#include "AudioPlay.h"
#include "asferr.h"
#include "nserror.h"
#include "AudioDlg.h"

#ifdef SUPPORT_DRM

#include "DRM.h"

CDRM objDRM;    // to handle protected content

#endif

//------------------------------------------------------------------------------
// Name: CAudioPlay::CAudioPlay()
// Desc: Constructor.
//------------------------------------------------------------------------------
CAudioPlay::CAudioPlay()
{	
    m_cRef              = 1;

#ifdef SUPPORT_DRM
	m_bProcessingDRMOps = FALSE;
#endif

    m_bClosed           = TRUE;
    m_bIsSeekable       = FALSE;
    m_bIsBroadcast      = FALSE;
	m_bEOF				= FALSE;
    m_dwAudioOutputNum  = 0xFFFFFFFF;
	m_dwThreadID		= 0;
	m_hAsyncEvent		= INVALID_HANDLE_VALUE;
	m_hrAsync			= S_OK;
	m_hWaveOut			= NULL;
	m_pReader			= NULL;
    m_pHeaderInfo       = NULL;
	m_cHeadersLeft		= 0;
	m_pwszURL			= NULL;
	m_cnsFileDuration	= 0;
	m_pWfx              = NULL;
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::~CAudioPlay()
// Desc: Destructor.
//------------------------------------------------------------------------------
CAudioPlay::~CAudioPlay()
{
	Exit();
	CoUninitialize();
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::QueryInterface()
// Desc: IUnknown method.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CAudioPlay::QueryInterface( /* [in] */ REFIID riid,
                                                      /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject ) 
{
    if( ( IID_IWMReaderCallback == riid ) ||
        ( IID_IUnknown == riid ) )
    {
        *ppvObject = static_cast< IWMReaderCallback* >( this );
        AddRef();
        return( S_OK );
    }

    *ppvObject = NULL;
    return( E_NOINTERFACE );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::AddRef()
// Desc: IUnknown method.
//------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CAudioPlay::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Release()
// Desc: IUnknown method.
//------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CAudioPlay::Release()
{
    if( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return( 0 );
    }

    return( m_cRef );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::OnStatus()
// Desc: IWMReaderCallback method to process status notifications.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::OnStatus( /* [in] */ WMT_STATUS Status,
							  /* [in] */ HRESULT hr,
							  /* [in] */ WMT_ATTR_DATATYPE dwType,
							  /* [in] */ BYTE __RPC_FAR *pValue,
							  /* [in] */ void __RPC_FAR *pvContext ) 
{
	// This switch checks for the important messages sent by the reader object.
	switch( Status )
	{
	// The reader is finished opening a file.
	case WMT_OPENED:

#ifdef SUPPORT_DRM
		//
		// Set the event if DRM operation is not going on
		//
		if( !m_bProcessingDRMOps )
#endif	
        {
            SetAsyncEvent( hr );
        }
			
		break;

	// Playback of the opened file has begun.
	case WMT_STARTED:

		m_bEOF = FALSE;
		SetCurrentStatus( PLAY );
		break;

	// The reader is finished closing a file.
	case WMT_CLOSED:

		SetAsyncEvent( hr );
		break;

	// The previously playing reader has stopped.
	case WMT_STOPPED:
		
		SetAsyncEvent( hr );
		SetCurrentStatus( STOP );
		break;
	
	// This class reacts to any errors by changing its state to stopped.
	case WMT_ERROR:
	case WMT_EOF:
	case WMT_MISSING_CODEC:
		
		m_bEOF = TRUE;
		
        //
        // Set to STOP when no buffer is left for playback
        //
		if( 0 == m_cHeadersLeft )
		{
			SetCurrentStatus( STOP );
		}

		break;
	
	// The reader has begun buffering.
	case WMT_BUFFERING_START:
		
		SetCurrentStatus( BUFFERING );
		break;

	// the reader has completed buffering.
	case WMT_BUFFERING_STOP:

		SetCurrentStatus( PLAY );
		break;

	case WMT_LOCATING:
		break;

    //
    // License and DRM related messages
    //
	case WMT_NO_RIGHTS:
	case WMT_NO_RIGHTS_EX:
	case WMT_NEEDS_INDIVIDUALIZATION:
	case WMT_LICENSEURL_SIGNATURE_STATE:

#ifdef SUPPORT_DRM
		m_bProcessingDRMOps = TRUE;
#endif
    
    case WMT_ACQUIRE_LICENSE:
	case WMT_INDIVIDUALIZE:

#ifdef SUPPORT_DRM
		objDRM.OnDRMStatus( Status, hr, dwType, pValue, pvContext );
#endif
		break;

	default:
		break;
	}
	
	return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::OnSample()
// Desc: IWMReaderCallback method to process samples.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::OnSample( /* [in] */ DWORD dwOutputNum,
							  /* [in] */ QWORD cnsSampleTime,
							  /* [in] */ QWORD cnsSampleDuration,
							  /* [in] */ DWORD dwFlags,
							  /* [in] */ INSSBuffer __RPC_FAR *pSample,
							  /* [in] */ void __RPC_FAR *pvContext )
{
	// Check the output number of the sample against the stored output number.
	// Because only the first audio output is stored, all other outputs,
	//  regardless of type, will be ignored.
    if( dwOutputNum != m_dwAudioOutputNum )
    {
        return( S_OK );
    }

	BYTE        *pData = NULL;
	DWORD       cbData = 0;
	HRESULT     hr = S_OK;
    MMRESULT    mmr;

    // Get the sample from the buffer object.
    hr = pSample->GetBufferAndLength( &pData, &cbData );
    if( FAILED( hr ) )
	{
		return( hr );
	}

	// The rest of the code in this method uses Windows Multimedia wave
	//  handling functions to play the content. 
    //
    // Allocate memory for header and data.
    //
	LPWAVEHDR pwh = ( LPWAVEHDR ) new BYTE[ sizeof( WAVEHDR ) + cbData ];
    if( NULL == pwh )
    {
		return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
	
    pwh->lpData = ( LPSTR )&pwh[1];
    pwh->dwBufferLength = cbData; 
    pwh->dwBytesRecorded = cbData;
    pwh->dwUser = ( DWORD )cnsSampleTime;
    pwh->dwLoops = 0;
    pwh->dwFlags = 0;
	
	CopyMemory( pwh->lpData, pData, cbData );
	
    do
    {
	    //
	    // Prepare the header for playing.
	    //
	    mmr = waveOutPrepareHeader( m_hWaveOut, pwh, sizeof( WAVEHDR ) );
	    if( MMSYSERR_NOERROR != mmr )
	    {
            break;
        }

        //
		// Send the sample to the wave output device.
		//
		hr = waveOutWrite( m_hWaveOut, pwh, sizeof( WAVEHDR ) );
	    if( MMSYSERR_NOERROR != hr )
	    {
            break;
        }

		InterlockedIncrement( &m_cHeadersLeft );

		if( m_bIsBroadcast )
		{
			SetTime( cnsSampleTime, 0 );
		}
        else
		{
			SetTime( cnsSampleTime, m_cnsFileDuration );
		}
    }
    while( FALSE );

	if( MMSYSERR_NOERROR != mmr )
	{
        delete [] pwh;

        //
        // Stop the player.
        //
		MessageBox( g_hwndDialog, _T( "Wave function failed" ), ERROR_DIALOG_TITLE, MB_OK );
		Stop();
	}

	return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Init()
// Desc: Initializes the audio player.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Init()
{
	HRESULT hr = S_OK;
	TCHAR   tszErrMsg[ 256 ];
	
	do
	{
        hr = CoInitialize( NULL );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "CoInitialize failed" ) );
			break;
		}
		
		
		// Create an event for asynchronous calls.
		// When code in this class makes a call to an asynchronous
		//  method, it will wait for the event to be set to resume
		//  processing. 
		m_hAsyncEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if( NULL == m_hAsyncEvent )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not create the event" ) );
			hr = E_FAIL;
			break;
		}

        
        // Create a reader object, requesting only playback rights.
        hr = WMCreateReader( NULL, WMT_RIGHT_PLAYBACK, &m_pReader );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not create Reader" ) );
			break;
		}
	}
	while( FALSE );

	if( FAILED( hr ) )
	{
        (void)StringCchPrintf( tszErrMsg, ARRAYSIZE(tszErrMsg), _T("%s (hr=%#X)"), tszErrMsg, hr );
		MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
		Exit();
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Exit()
// Desc: Cleanup.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Exit()
{
    SAFE_RELEASE( m_pHeaderInfo );
	SAFE_RELEASE( m_pReader );

	if( NULL != m_hWaveOut )
    {
        waveOutClose( m_hWaveOut );
		m_hWaveOut = NULL;
    }

	if( NULL != m_hAsyncEvent )
	{
		CloseHandle( m_hAsyncEvent );
		m_hAsyncEvent = NULL;
	}

	SAFE_ARRAYDELETE( m_pwszURL );

    if( NULL != m_pWfx )
    {
        delete [] ( BYTE* )m_pWfx;
        m_pWfx = NULL;
    }

	return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Open()
// Desc: Opens a file.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Open( LPCWSTR pwszUrl )
{
    const INT MAX_ERROR_LENGTH = 256;

    // Check that the parameter is not NULL and that the reader is initialized.
	if( NULL == pwszUrl )
    {
		return( E_INVALIDARG );
    }

	if( NULL == m_pReader )
    {
		return( E_UNEXPECTED );
    }

	HRESULT hr = S_OK;
	TCHAR   tszErrMsg[ MAX_ERROR_LENGTH ];

	do
	{
        ZeroMemory( tszErrMsg, MAX_ERROR_LENGTH );
		ResetEvent( m_hAsyncEvent );

		
		// Close previously opened file, if any.
		Close();

		// Delete the old file name.
		SAFE_ARRAYDELETE( m_pwszURL );
		
		// Set the new file name.
		m_pwszURL = new WCHAR[ wcslen( pwszUrl ) + 1 ];
		if( NULL == m_pwszURL )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Insufficient memory" ) );
			break;
		}

		(void)StringCchCopyW( m_pwszURL, wcslen( pwszUrl ) + 1, pwszUrl );

#ifdef SUPPORT_DRM

		hr = objDRM.Init( this, m_pReader, m_pwszURL );
		if( FAILED( hr ) )
        {
			break;
		}
		
		m_bProcessingDRMOps = FALSE;
#endif

        // Open the file with the reader object. This method call also sets
		//  the status callback that the reader will use.
        hr = m_pReader->Open( pwszUrl, this, NULL );
		
		if( FAILED( hr ) )
        {
            (void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not open the specified file" ) );
			break;
		}
		

		// Wait for the Open call to complete. The event is set in the OnStatus 
		//  callback when the reader reports completion.
		WaitForEvent( m_hAsyncEvent );
		
#ifdef SUPPORT_DRM

		if( ( NS_S_DRM_ACQUIRE_CANCELLED == m_hrAsync ) || 
			( NS_E_DRM_INDIVIDUALIZATION_INCOMPLETE == m_hrAsync ) )
		{
			//
			// DRM operation has been canceled. So the file has not been opened yet.
			//
			hr = m_hrAsync;
			return( hr );
		}

        //
        // Check to see if license acquisition failed for some reason
        //
        BOOL fAcquiringLicenseNonSilently = FALSE;
        if( 7 == objDRM.GetLastDRMVersion() &&
            ( NS_E_DRM_LICENSE_NOTACQUIRED == m_hrAsync || 
              NS_E_DRM_LICENSE_STORE_ERROR == m_hrAsync ) )
        {
            //
            // Try to acquire the license non-silently.
            //
            fAcquiringLicenseNonSilently = TRUE;
            hr = objDRM.AcquireLastV7LicenseNonSilently();
            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

        //
        // Check to see if the license is being acquired non-silently
        //
		if( NS_E_DRM_NO_RIGHTS == m_hrAsync || // All DRMv1 licenses are gotten non-silently
            fAcquiringLicenseNonSilently )
		{
			//
			// The license for this file is being acquired non-silently,
			// and it cannot be played until the license is acquired.
			//
			MessageBox( g_hwndDialog, _T( "After acquiring the license, re-open the file." ), ERROR_DIALOG_TITLE, MB_OK );

			SetCurrentStatus( CLOSED );
			SetCurrentStatus( READY );
			hr = m_hrAsync;
			return( hr );
		}
#endif

        
        // Check the HRESULT reported by the reader object to the OnStatus 
		//  callback. Most errors in opening files will be reported this way.
        if( FAILED( m_hrAsync ) )
		{
			hr = m_hrAsync;

			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not open the specified file" ) );
			break;
		}
		
        
        SAFE_RELEASE( m_pHeaderInfo );
		hr = m_pReader->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pHeaderInfo );
		if( FAILED (hr) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not QI for IWMHeaderInfo" ) );
			break;
		}

		hr = RetrieveAndDisplayAttributes();
		if( FAILED( hr ) )
		{
			break;
		}
		
		// Set the audio ouput number for the current file. 
        // Only the first audio output is retrieved, regardless of the 
		//  number of audio outputs in the file.
		hr = GetAudioOutput();
		if( FAILED( hr ) )
		{
			break;
		}
	}
	while( FALSE );

	if( FAILED( hr ) )
	{
		Close();

		if( _tcslen( tszErrMsg ) > 0 )
		{
            (void)StringCchPrintf( tszErrMsg, ARRAYSIZE(tszErrMsg), _T("%s (hr=%#X)"), tszErrMsg, hr );
			MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
		}
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Close()
// Desc: Closes the file.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Close()
{
	HRESULT hr = S_OK;

	if( NULL != m_pReader )
	{
		hr = m_pReader->Close();
	    if( FAILED ( hr ) )
	    {
		    return( hr );
	    }
	}
	
    
    // Wait for the reader to deliver a WMT_CLOSED event to the OnStatus
	//  callback.
    WaitForEvent( m_hAsyncEvent );
	
    //
    // Close the wave output device.
    //
	if( m_hWaveOut )
	{
		if( MMSYSERR_NOERROR != waveOutReset( m_hWaveOut ) )
		{
			return( E_FAIL );
		}

		if( MMSYSERR_NOERROR != waveOutClose( m_hWaveOut ) )
		{
			return( E_FAIL );
		}

		m_hWaveOut = NULL;
	}
	
	return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Start()
// Desc: Configures output device and starts reader.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Start( QWORD cnsStart )
{
	HRESULT hr = S_OK;
	
    //
    // Ensure that a reader object has been instantiated.
    //
    if( NULL == m_pReader )
    {
        return( E_UNEXPECTED );
    }

    //
    // Configure the wave output device.
    //
	if( NULL != m_hWaveOut )
	{
		if( MMSYSERR_NOERROR != waveOutReset( m_hWaveOut ) )
		{
			return( E_FAIL );
		}
	}
	else
	{
		MMRESULT mmr = waveOutOpen( &m_hWaveOut,
						            WAVE_MAPPER,
						            m_pWfx,
						            ( DWORD_PTR )WaveProc,
						            ( DWORD_PTR )this,
						            CALLBACK_FUNCTION );

		if( MMSYSERR_NOERROR != mmr )
		{
			return( E_FAIL );
		}

        //
        // Use another thread for wave output.
        //
		HANDLE hThread = CreateThread( NULL, 0, OnWaveOutThread,
									   ( LPVOID )this, NULL, &m_dwThreadID );

		if( NULL == hThread )
		{
			return( HRESULT_FROM_WIN32( GetLastError() ) );
		}
		else
		{
			//
			// Close the thread handle, as it's no longer required in this thread.
			//
			CloseHandle( hThread );
		}
	}

	return( m_pReader->Start( cnsStart, 0, 1.0, NULL ) );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Stop()
// Desc: Stops the reader.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Stop()
{
	HRESULT hr = S_OK;

    //
    // Ensure that a reader object has been instantiated.
    //
    if( NULL == m_pReader )
    {
        return( E_UNEXPECTED );
    }

#ifdef SUPPORT_DRM
	//
	// Cancel current DRM operation, if any.
	//
	hr = objDRM.Cancel();
	if( FAILED( hr ) )
	{
		//
		// The cancellation of the DRM operation has failed.
		//
		return( hr );
    }
    
    if( S_OK == hr )
    {
	    //
	    // Since the DRM operation has been successfully canceled, the file is not yet opened.
	    // So no need to stop.
	    //
	    SetCurrentStatus( CLOSED );
	    SetCurrentStatus( READY );
	    return( hr );
    }
#endif

	hr = m_pReader->Stop();
	if( FAILED( hr ) )
	{
		return( hr );
	}
		
	//
	// Reset the wave output device
	//
	if( NULL != m_hWaveOut )
	{
		if( MMSYSERR_NOERROR != waveOutReset( m_hWaveOut ) )
		{
			return( E_FAIL );
		}
		
        //
		// Wait for all audio headers to be unprepared.
        //
		WaitForEvent( m_hAsyncEvent );
	}
	
    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Pause()
// Desc: Pauses the reader and the output device.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Pause()
{
    //
    // Sanity check
    //
    if( NULL == m_pReader )
    {
        return( E_UNEXPECTED );
    }

	if( NULL != m_hWaveOut )
	{
		//
		// Pause the wave output device
		//
		if( MMSYSERR_NOERROR != waveOutPause( m_hWaveOut ) )
		{
			return( E_FAIL );
		}
	}

	return( m_pReader->Pause() );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::Resume()
// Desc: Resumes after pause.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::Resume()
{
    //
    // Sanity check
    //
    if( NULL == m_pReader )
    {
        return( E_UNEXPECTED );
    }

	if( NULL != m_hWaveOut )
	{
		//
		// Resume the wave output device
		//
		if( MMSYSERR_NOERROR != waveOutRestart( m_hWaveOut ) )
		{
			return( E_FAIL );
		}
	}

	return( m_pReader->Resume() );
}


//------------------------------------------------------------------------------
// Name: CAudioPlay::ReopenReader()
// Desc: Opens reader after DRM operations.
//------------------------------------------------------------------------------

#ifdef SUPPORT_DRM

HRESULT CAudioPlay::ReopenReader( void *pvContext )
{
	if( NULL == m_pReader )
	{
        return( E_UNEXPECTED );
    }

	m_bProcessingDRMOps = FALSE;
	return m_pReader->Open( m_pwszURL, this, pvContext );
}
#endif

//------------------------------------------------------------------------------
// Name: CAudioPlay::SetAsyncEvent()
// Desc: This method sets the asynchronous event. When an asynchronous method
//  is called in the other methods of this class, execution of that thread waits
//  for the asynchronous event to be set.
//------------------------------------------------------------------------------
void CAudioPlay::SetAsyncEvent( HRESULT hrAsync )
{
	m_hrAsync = hrAsync;
	SetEvent( m_hAsyncEvent );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::GetFileDuration()
// Desc: Get the value of m_cnsFileDuration.
//------------------------------------------------------------------------------
QWORD CAudioPlay::GetFileDuration()
{
	return( m_cnsFileDuration );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::IsSeekable()
// Desc: Get the value of m_bIsSeekable.
//------------------------------------------------------------------------------
BOOL CAudioPlay::IsSeekable()
{
	return( m_bIsSeekable );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::IsBroadcast()
// Desc: Get the value of m_bIsBroadcast.
//------------------------------------------------------------------------------
BOOL CAudioPlay::IsBroadcast()
{
	return( m_bIsBroadcast );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::OnWaveOutThread()
// Desc: Start thread for processing WaveOut messages.
//------------------------------------------------------------------------------
DWORD WINAPI CAudioPlay::OnWaveOutThread( LPVOID lpParameter )
{
	CAudioPlay* pThis = ( CAudioPlay* )lpParameter;

	//
	// Redirect the processing to a non-static member
	// function of the CAudioPlay class
	//
	pThis->OnWaveOutMsg();

	return( 0 );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::WaveProc()
// Desc: Callback passed to waveOutOpen.
//------------------------------------------------------------------------------
void CALLBACK CAudioPlay::WaveProc( HWAVEOUT hwo,
									UINT uMsg, 
									DWORD_PTR dwInstance,
									DWORD dwParam1,
									DWORD dwParam2 )
{
	CAudioPlay* pThis = ( CAudioPlay* )dwInstance;
	
	//
	// Redirect the processing to a different thread
	//
	PostThreadMessage( pThis->m_dwThreadID, uMsg, dwParam1, dwParam2 );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::OnWaveOutMsg()
// Desc: Message pump for WaveOut messages.
//------------------------------------------------------------------------------
void CAudioPlay::OnWaveOutMsg()
{
	HRESULT     hr = S_OK;
    LPWAVEHDR   pwh = NULL;
    MMRESULT    mmr = MMSYSERR_NOERROR;
	MSG         uMsg;

	//
    // Create the message queue
	//
	PeekMessage( &uMsg, NULL, WM_USER, WM_USER, PM_NOREMOVE );

	//
	// Message queue has been created. Let's get the messages
	//
    while( 0 != GetMessage( &uMsg, NULL, 0, 0 ) )
    {
        switch( uMsg.message )
        {
		case WOM_DONE:
			//
			// Unprepare the wave header, as it has already been played
			//
			pwh = ( LPWAVEHDR )uMsg.wParam;

			mmr = waveOutUnprepareHeader( m_hWaveOut, pwh, sizeof( WAVEHDR ) );

			if( MMSYSERR_NOERROR == mmr )
			{
				InterlockedDecrement( &m_cHeadersLeft );
			}
			else if( WHDR_ENDLOOP == mmr )
            {
                SetEvent( m_hAsyncEvent );
            }
			else
			{
                //
                // Stop playing as error occurs
                //
				Stop();

				MessageBox( g_hwndDialog, _T("Wave function (waveOutUnprepareHeader) failed"), ERROR_DIALOG_TITLE, MB_OK );
			}

            //
            // Set to STOP if no buffer is left
            //
			if( m_bEOF && ( 0 == m_cHeadersLeft ) )
			{
				SetCurrentStatus( STOP );
			}

            break;

		case WOM_CLOSE:

			PostQuitMessage( 0 );
			break;
		}
	}

    return;
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::WaitForEvent()
// Desc: Waits for the specified event to be signaled. 
//       The maximum wait time is msMaxWaitTime.
//------------------------------------------------------------------------------
void CAudioPlay::WaitForEvent( HANDLE hEvent, DWORD msMaxWaitTime )
{
    DWORD   i;
	MSG     msg;

    for( i = 0; i < msMaxWaitTime; i += 10 )
    {
        if( PeekMessage( &msg, ( HWND ) NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        if( WAIT_TIMEOUT != WaitForSingleObject( hEvent, 10 ) )
        {
            break;
        }
    }
    
	return;
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::GetAudioOutput()
// Desc: Gets the wave format for the first audio output.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::GetAudioOutput()
{
	DWORD                   cOutputs = 0;
    DWORD                   i;
	TCHAR                   tszErrMsg[256];
	HRESULT                 hr = S_OK;
    IWMOutputMediaProps*    pProps = NULL;
	WM_MEDIA_TYPE*          pMediaType = NULL;
	ULONG                   cbType = 0;

    //
    // Sanity check
    //
    if( NULL == m_pReader )
    {
        return( E_UNEXPECTED );
    }

	do
	{
        //
        // Find out the output count
        //
        hr = m_pReader->GetOutputCount( &cOutputs );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get output count" ) );
			break;
		}

		//
		// Find out one audio output.
        // Note: This sample only shows how to handle one audio output.
        //       If there is more than one audio output, the first one will be picked.
		//
		for( i = 0; i < cOutputs; i++ )
		{
			SAFE_RELEASE( pProps );
			SAFE_ARRAYDELETE( pMediaType );

			hr = m_pReader->GetOutputProps( i, &pProps );
			if( FAILED( hr ) )
			{
				(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get output props" ) );
				break;
			}

            //
            // Find out the space needed for pMediaType
            //
			hr = pProps->GetMediaType( NULL, &cbType );
			if( FAILED( hr ) )
			{
				(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not query for the space needed for media type" ) );
				break;
			}

			pMediaType = ( WM_MEDIA_TYPE* ) new BYTE[ cbType ];
			if( NULL == pMediaType )
			{
				hr = HRESULT_FROM_WIN32( GetLastError() ) ;
    			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Insufficient memory" ) );
				break;
			}

            //
            // Get the value for MediaType
            //
			hr = pProps->GetMediaType( pMediaType, &cbType );
			if( FAILED( hr ) )
			{
				(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get media type" ) );
				break;
			}
			
			if( WMMEDIATYPE_Audio == pMediaType->majortype )
            {
                break;
            }
		}
	
		if( FAILED( hr ) )
		{
			break;
		}

		if( i == cOutputs )
		{
			//
			// Couldn't find any audio output number in the file
			//
			hr = E_UNEXPECTED;

			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not find an audio stream in the specified file" ) );
			break;
		}

        //
        // Store the wave format for this output
        //
        m_dwAudioOutputNum = i;

        if( NULL != m_pWfx )
        {
            delete [] ( BYTE* )m_pWfx;
            m_pWfx = NULL;
        }

        m_pWfx = ( WAVEFORMATEX * )new BYTE[ pMediaType->cbFormat ];
        if( NULL == m_pWfx )
        {
			hr = HRESULT_FROM_WIN32( GetLastError() ) ;
    		(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Insufficient memory" ) );
			break;
        }

		CopyMemory( m_pWfx, pMediaType->pbFormat, pMediaType->cbFormat );
	}
	while( FALSE );
	
	SAFE_ARRAYDELETE( pMediaType );
	SAFE_RELEASE( pProps );

	if( FAILED( hr ) )
	{
        (void)StringCchPrintf( tszErrMsg, ARRAYSIZE(tszErrMsg), _T("%s (hr=%#X)"), tszErrMsg, hr );
		MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::RetrieveAndDisplayAttributes()
// Desc: Retrieves and displays title, author, copyright and duration info.
//       Retrieves Seekable and Broadcast attributes
//------------------------------------------------------------------------------
HRESULT CAudioPlay::RetrieveAndDisplayAttributes()
{
	BYTE*   pbValue = NULL;
	HRESULT hr = S_OK;
	TCHAR   tszErrMsg[256];
	WCHAR   wszNoData[] = L"No Data";

	do
	{
		//
		// Get attribute "Title"
		//
		hr = GetHeaderAttribute( g_wszWMTitle, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Title attribute" ) );
			break;
		}
		
		if( NULL != pbValue )
		{
			SetItemText( IDC_CLIP, ( LPWSTR )pbValue );
			SAFE_ARRAYDELETE( pbValue );
        }
		else
		{
			SetItemText( IDC_CLIP, wszNoData );
		}
		
		//
		// Get attribute "Author"
		//
		hr = GetHeaderAttribute( g_wszWMAuthor, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Author attribute" ) );
			break;
		}

		if( NULL != pbValue )
		{
			SetItemText( IDC_AUTHOR, ( LPWSTR )pbValue  );
			SAFE_ARRAYDELETE( pbValue );
		}
		else
		{
			SetItemText( IDC_AUTHOR, wszNoData );
		}

		//
		// Get attribute "Copyright"
		//
		hr = GetHeaderAttribute( g_wszWMCopyright, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Copyright attribute" ) );
			break;
		}

		if( NULL != pbValue )
		{
			SetItemText( IDC_COPYRIGHT, ( LPWSTR )pbValue  );
			SAFE_ARRAYDELETE( pbValue );
		}
		else
		{
			SetItemText( IDC_COPYRIGHT, wszNoData );
		}
		
		//
		// Get attribute "Duration"
		//
		hr = GetHeaderAttribute( g_wszWMDuration, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Duration attribute" ) );
			break;
		}

        if( NULL != pbValue )
        {
            m_cnsFileDuration = *( QWORD* )pbValue;
			SAFE_ARRAYDELETE( pbValue );
        }
        else
        {
            m_cnsFileDuration = 0;
        }

		SetTime( 0, m_cnsFileDuration );

        //
        // Retrieve Seekable attribute
        //
		hr = GetHeaderAttribute( g_wszWMSeekable, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Seekable attribute" ) );
			break;
		}

		if( NULL != pbValue )
		{
            m_bIsSeekable = *( BOOL* )pbValue;
        }
        else
        {
            m_bIsSeekable = FALSE;
        }
        
        //
        // Retrieve Broadcast attribute
        //
		hr = GetHeaderAttribute( g_wszWMBroadcast, &pbValue );
		if( FAILED( hr ) )
		{
			(void)StringCchCopy( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not get the Broadcast attribute" ) );
			break;
		}

		if( NULL != pbValue )
		{
            m_bIsBroadcast = *( BOOL* )pbValue;
        }
        else
        {
            m_bIsBroadcast = FALSE;
        }
   	}
	while (FALSE );
		
	if( FAILED( hr ) )
	{
        (void)StringCchPrintf( tszErrMsg, ARRAYSIZE(tszErrMsg), _T("%s (hr=%#X)"), tszErrMsg, hr );
		MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CAudioPlay::GetHeaderAttribute()
// Desc: Retrieves the specified header attribute from the reader.
//------------------------------------------------------------------------------
HRESULT CAudioPlay::GetHeaderAttribute( LPCWSTR pwszName, BYTE** ppbValue )
{
	BYTE                *pbValue = NULL;
	HRESULT             hr = S_OK;
	WMT_ATTR_DATATYPE   wmtType;
	WORD                wStreamNum = 0;
    WORD                cbLength = 0;

    //
    // Sanity check
    //
    if( NULL == m_pHeaderInfo )
    {
        return( E_UNEXPECTED );
    }

    if( NULL == ppbValue )
    {
        return( E_INVALIDARG );
    }

	//
	// Get the count of bytes to be allocated for pbValue
	//
	hr = m_pHeaderInfo->GetAttributeByName( &wStreamNum,
										    pwszName,
										    &wmtType,
										    NULL,
										    &cbLength );
	if( FAILED( hr ) && ( ASF_E_NOTFOUND != hr ) )
	{
		return( hr );
	}

    //
    // No such an attribute, so return
    //
	if( ASF_E_NOTFOUND == hr )
	{
        *ppbValue = NULL;
		return( S_OK );
	}

	pbValue = new BYTE[ cbLength ];
	if( NULL == pbValue )
	{
		return( HRESULT_FROM_WIN32( GetLastError() ) );
	}

	//
	// Get the actual value
	//
	hr = m_pHeaderInfo->GetAttributeByName( &wStreamNum,
										    pwszName,
										    &wmtType,
										    pbValue,
										    &cbLength );
	*ppbValue = pbValue;

	return( S_OK );
}

//////////////////////////////////////////////////////////////////////
//
//
//  Copyright (c) 1998-2001  Microsoft Corporation
//
//
// callnot.cpp
//
// Implementation of the ITTAPIEventNotification interface.
//
// This is an outgoing interface that is defined by TAPI 3.0.  This
// is basically a callback function that TAPI 3.0 calls to inform
// the application of events related to calls (on a specific address)
//
// Please refer to COM documentation for information on outgoing
// interfaces.
// 
// An application must implement and register this interface in order
// to receive calls and events related to calls
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <tapi3.h>

#include "callnot.h"
#include "FileTerm.h"
#include "resource.h"

extern ITBasicCallControl * g_pCall;
extern HWND g_hDlg;

extern ITTerminal *g_pPlayFileTerm;
extern ITTerminal *g_pRecordFileTerm;

//
//to keep track of number of messages
//
DWORD g_dwMessages = 0;

//////////////////////////////////////////////////////////////////
//
// GetTerminalFromStreamEvent - get the stream from the event and 
// after this get the terminal from stream - if any
//
//////////////////////////////////////////////////////////////////
HRESULT GetTerminalFromStreamEvent( 
			IN ITCallMediaEvent * pCallMediaEvent,
            OUT ITTerminal ** ppTerminal )
{
	//
	//do not return bogus value
	//
    *ppTerminal = NULL;

    //
    // Get the stream for this event.
    //
    ITStream * pStream = NULL;
    HRESULT hr = pCallMediaEvent->get_Stream( &pStream );
    if (FAILED(hr))
	{
        DoMessage( _T("GetTerminalFromStreamEvent: get_Stream failed"));
		return E_FAIL;
	}

    //
    // Enumerate the terminals on this stream.
    //
    IEnumTerminal * pEnumTerminal = NULL;
    hr = pStream->EnumerateTerminals( &pEnumTerminal );

	//
	//not need this anymore
	//
    pStream->Release();

    if ( FAILED(hr) ) 
	{
        DoMessage(_T("GetTerminalFromStreamEvent: EnumerateTerminals failed"));
		return hr;
	}

    //
    // Get the first terminal -- if there aren't any, return E_FAIL so that
    // we skip this event (this happens when the terminal is on the other
    // stream).
    //
    hr = pEnumTerminal->Next(1, ppTerminal, NULL);

	//
	//clean up
	//
    pEnumTerminal->Release();

	//
	//map anything but S_OK to failure - even S_FALSE
	//
    if ( hr != S_OK )
	{
        return E_FAIL;
	}

    return S_OK;

}


///////////////////////////////////////////////////////////////////
// CallEventNotification
//
// The only method in the ITCallEventNotification interface.  This gets
// called by TAPI 3.0 when there is a call event to report. This just
// posts the message to our UI thread, so that we do as little as
// possible on TAPI's callback thread.
//
///////////////////////////////////////////////////////////////////

HRESULT
STDMETHODCALLTYPE
CTAPIEventNotification::Event(
					IN TAPI_EVENT TapiEvent,
					IN IDispatch * pEvent
					)
{

	//
	//sanity check
	//
	if(NULL == pEvent)
	{
		return S_OK;
	}

    //
    // Addref the event so it doesn't go away.
    //
    pEvent->AddRef();

    //
    // Post a message to our own UI thread.
    //

    PostMessage(
                g_hDlg,
                WM_PRIVATETAPIEVENT,
                (WPARAM) TapiEvent,
                (LPARAM) pEvent
               );

    return S_OK;
}

///////////////////////////////////////////////////////////////////
// OnTapiEvent
//
// This is the real event handler, called on our UI thread when
// the WM_PRIVATETAPIEVENT message is received
//
///////////////////////////////////////////////////////////////////

HRESULT
OnTapiEvent(
            IN TAPI_EVENT TapiEvent,
            IN IDispatch * pEvent
           )
{
    HRESULT hr = E_FAIL;

    switch ( TapiEvent )
    {
        case TE_CALLNOTIFICATION:
        {
            // TE_CALLNOTIFICATION means that the application is being notified
            // of a new call.
            //
			hr = DoCallNotification(pEvent);
            break;
        }//TE_CALLNOTIFICATION
        
        case TE_CALLSTATE:
        {
            //
			// TE_CALLSTATE is a call state event.  pEvent is
            // an ITCallStateEvent	
			//
			hr = DoCallState(pEvent);
            break;
        }//TE_CALLSTATE

        case TE_CALLMEDIA:
        {
			//
			// TE_CALLMEDIA is a media event.  pEvent is an ITCallMediaEvent
			//
			hr = DoCallMedia(pEvent);
            break;    
        }//TE_CALLMEDIA  
		

		case TE_FILETERMINAL:
		{
            //
			// TE_FILETERMINAL is a file terminal event.  pEvent is an ITFileTerminalEvent
			//
			hr = DoFileEvent(pEvent);
			break;
		}//TE_FILETERMINAL

		default:
			break;
    }    

   
    pEvent->Release(); // we addrefed CTAPIEventNotification::Event()

    return S_OK;
}

///////////////////////////////////////////////////////////////////
// CheckStreamMT
//
// Helper method - check if pITStream has the requested media type
//
///////////////////////////////////////////////////////////////////
HRESULT CheckStreamMT(
			IN ITStream* pITStream,
			IN long mt)
{
	long mtStream = TAPIMEDIATYPE_AUDIO;
	HRESULT hr = E_FAIL;
	if(FAILED(hr=pITStream->get_MediaType(&mtStream)))
	{
		return hr;
	}
	if(!(mt&mtStream))
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}

///////////////////////////////////////////////////////////////////
// CheckStreamDir
//
// Helper method - check if pITStream has the requested direction
//
///////////////////////////////////////////////////////////////////
HRESULT CheckStreamDir(
			IN ITStream* pITStream,
			IN TERMINAL_DIRECTION td)
{
	TERMINAL_DIRECTION tdStream = TD_CAPTURE ;
	HRESULT hr =E_FAIL;
	if(FAILED(hr=pITStream->get_Direction(&tdStream)))
	{
		return hr;
	}
	if(td!=tdStream)
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}


///////////////////////////////////////////////////////////////////
// CreateAndSelectFileRecordTerminal
//
// Creates FileRecording terminal, add the audio track, put_FileName
// select track on the stream
//
///////////////////////////////////////////////////////////////////
HRESULT CreateAndSelectFileRecordTerminal()
{

	//
    // we should already have the call 
    //
    if (NULL == g_pCall) 
    {
        DoMessage( _T("CreateAndSelectFileRecordTerminal: we do not have a call"));
        return E_UNEXPECTED;
    }

    //
    // get the ITStreamControl interface for this call
    //
	ITStreamControl* pStreamControl = NULL;
    HRESULT hr = g_pCall->QueryInterface( IID_ITStreamControl, (void**)&pStreamControl );
    if (FAILED(hr))
    {
        DoMessage( _T("CreateAndSelectFileRecordTerminal: QI for ITStreamControl failed"));
        return E_FAIL;
    }

	//
	// make sure the record terminal was released
	//
	if(NULL != g_pRecordFileTerm)
	{
		g_pRecordFileTerm->Release();
		g_pRecordFileTerm = NULL;
	}

    //
	// QI for ITBasicCallControl2 - prepare to request the terminal
	//
	ITBasicCallControl2* pITBCC2 = NULL;
    hr = g_pCall->QueryInterface( IID_ITBasicCallControl2, (void**)&pITBCC2 );
	if(FAILED(hr))
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: QI for ITBasicCallControl2 failed"));
		return hr;
	}

	//
	// get the BSTR CLSID used for ReqestTerminal
	//
	BSTR bstrCLSID = NULL;
	hr = StringFromCLSID(CLSID_FileRecordingTerminal, &bstrCLSID);
	if(FAILED(hr))
	{
		pITBCC2->Release();
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: StringFromCLSID failed"));
		return hr;
	}
	//
	// request the record terminal - use the right direction and media type
	//
	hr = pITBCC2->RequestTerminal(bstrCLSID, 
									TAPIMEDIATYPE_AUDIO, 
									TD_RENDER, 
									&g_pRecordFileTerm);
	//
	//clean up
	//
	::CoTaskMemFree(bstrCLSID);
	pITBCC2->Release();
	if(FAILED(hr))
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: RequestTerminal failed"));
		return hr;
	}

    //
    // get to the ITMediaRecord interface so we can put_FileName
	// we can NOT select before put_FileName 
    //
	ITMediaRecord* pITMediaRec = NULL;
    hr = g_pRecordFileTerm->QueryInterface( IID_ITMediaRecord, (void**)&pITMediaRec );
	if(FAILED(hr))
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: QI ITMediaRecord failed"));
		return hr;
	}

	//
	// we will have a file for each message
	//
	WCHAR wchFileName[MAX_PATH+1] = {0};
	_snwprintf_s(wchFileName, MAX_PATH, 
								_T("%s%ld%s"), REC_FILENAME, 
									g_dwMessages, REC_FILEEXT);

	
	wchFileName[MAX_PATH] = 0;
	BSTR bstrFileName = ::SysAllocString(wchFileName);
	if(NULL == bstrFileName)
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: SysAllocString failed"));
		return E_OUTOFMEMORY;
	}

	//
	// put_FileName
	//
	hr = pITMediaRec->put_FileName(bstrFileName);
	
	//
	//clean up
	//
	::SysFreeString(bstrFileName);
	pITMediaRec->Release();
	
	if(FAILED(hr))
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: put_FileName failed"));
		return hr;
	}

    //
    // get to the ITMultiTrackTerminal interface so we can create tracks 
    //
	ITMultiTrackTerminal* pMTRecTerminal = NULL;
    hr = g_pRecordFileTerm->QueryInterface( IID_ITMultiTrackTerminal, (void**)&pMTRecTerminal );
	if(FAILED(hr))
	{
		pStreamControl->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: QI IID_ITMultiTrackTerminal failed"));
		return hr;
	}

    //
    // enumerate the streams on the call
    //
    IEnumStream* pEnumStreams = NULL;
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);
    pStreamControl->Release();

    if (FAILED(hr))
    {
		pMTRecTerminal->Release();
		DoMessage( _T("CreateAndSelectFileRecordTerminal: EnumerateStreams failed"));
		return hr;
    }

	//
	// only do audio for now
	//
	long lMediaTypes = TAPIMEDIATYPE_AUDIO;

    //
    // walk through the list of streams on the call
    // create what we need to
    //
	ITStream* pStream = NULL;
    while (S_OK == pEnumStreams->Next(1, &pStream, NULL))
    {
        
        if ( (S_OK==CheckStreamMT(pStream,lMediaTypes))
				&& (S_OK==CheckStreamDir(pStream,TD_RENDER))) 
        {
            //
            // if we have a recording terminal, attempt to create a track terminal
            //
            ITTerminal* pRecordingTrack = NULL;

			//
			//check stream for media type - when we check we have an &
			//
			long lStreamMediaType=0;
			hr = pStream->get_MediaType(&lStreamMediaType);
            if (FAILED(hr))
            {
				pStream->Release();
				pStream=NULL;
                continue;

            }

			//
			// create track terminal
			//
            hr = pMTRecTerminal->CreateTrackTerminal(lStreamMediaType, TD_RENDER, &pRecordingTrack);
			if (FAILED(hr))
            {
				pStream->Release();
				pStream=NULL;
				//
				//will fail at the end anyway - lMediaTypes!=0 - so break here
				//
                break;

            }

            //
			//select it on stream
			//
			hr = pStream->SelectTerminal(pRecordingTrack);

			//
			//clean up
			//
			pStream->Release();
			pStream=NULL;
            pRecordingTrack->Release();
			pRecordingTrack=NULL;

            if (FAILED(hr))
            {
				//
				//will fail at the end anyway - lMediaTypes!=0 - so break here
				//
                break;
            }

			lMediaTypes^=lStreamMediaType;

			if(lMediaTypes==0)
			{
	            // 
				//we selected terminals for all media types we needed, we're done
				//
				break;
			}


        } 
		//
		// clean up
		//
		if(pStream!=NULL)
		{
			pStream->Release();
			pStream = NULL;
		}

    } // while (enumerating streams on the call)


	//
	//clean up
	//
    pMTRecTerminal->Release();
	pEnumStreams->Release();
    
	//
	//check if we were able to select for all media types
	//
    if (lMediaTypes==0)
    {
        return S_OK;
    }
    else
    {
		DoMessage( _T("CreateAndSelectFileRecordTerminal: no streams available"));
        return E_FAIL;
    }
}


///////////////////////////////////////////////////////////////////
// SameCall
//
// check if same call object as g_pCall
//
///////////////////////////////////////////////////////////////////
bool SameCall(ITCallStateEvent* pCallStateEvent)
{

	//
	// sanity check
	//
	if(NULL == pCallStateEvent)
	{
		return false;
	}

	//
	//get call object
	//
	ITCallInfo* pCallInfo = NULL;
	HRESULT hr = pCallStateEvent->get_Call(&pCallInfo);
	//
	//check if we have a call and get_Call succeded
	//
	if(NULL != pCallInfo &&  ( SUCCEEDED(hr) && NULL != g_pCall) )
	{
		bool bIsEqual = true;
		//
		//get and compare IUnknown for both objects
		//
		IUnknown* pIUnkCallInfo = NULL;
		IUnknown* pIUnkCallControl = NULL;
		pCallInfo->QueryInterface(IID_IUnknown, (void**)&pIUnkCallInfo);
		g_pCall->QueryInterface(IID_IUnknown, (void**)&pIUnkCallControl);
		//
		//compare
		//
		if(pIUnkCallInfo != pIUnkCallControl)
		{
			bIsEqual = false;
		}

		//
		//clean up
		//
		pCallInfo->Release();
		if(NULL != pIUnkCallInfo)
		{
			pIUnkCallInfo->Release();
		}
		if(NULL != pIUnkCallControl)
		{
			pIUnkCallControl->Release();
		}
		return bIsEqual;
	}
	else
	{
		return false;
	}

}


///////////////////////////////////////////////////////////////////
// DoCallNotification
//
// TE_CALLNOTIFICATION means that the application is being notified
// of a new call.
//
// Note that we don't answer to call at this point.  The application
// should wait for a CS_OFFERING CallState message before answering
// the call.
//
///////////////////////////////////////////////////////////////////
HRESULT DoCallNotification(
				IN IDispatch * pEvent)
{

    ITCallNotificationEvent         * pNotify = NULL;

    HRESULT hr = pEvent->QueryInterface( IID_ITCallNotificationEvent, (void **)&pNotify );

    if (FAILED(hr))
    {
        DoMessage( _T("Incoming call, but failed to get the interface"));
		return hr;
    }

    CALL_PRIVILEGE          cp = CP_MONITOR;
    ITCallInfo *            pCall = NULL;

    //
    // get the call
    //
    hr = pNotify->get_Call( &pCall );
    
    //
    // release the event object
    //
    pNotify->Release();

	if(FAILED(hr))
	{
		DoMessage( _T("Incoming call, but failed to get the call"));
		return hr;
	}

    //
    // check to see if we own the call
    //

    hr = pCall->get_Privilege( &cp );

	if(FAILED(hr))
	{
		pCall->Release();
		DoMessage( _T("Incoming call, but failed to get_Privilege"));
		return hr;
	}

    if ( CP_OWNER != cp )
    {
        //
		// just ignore it if we don't own it
		//
        pCall->Release();
		return hr;
    }

	//
	//check if we already have a call - if yes reject it
	//
	if(NULL != g_pCall)
	{
		ITBasicCallControl* pITBCC = NULL;

	    hr = pCall->QueryInterface( IID_ITBasicCallControl, (void**)&pITBCC );
        pCall->Release();
		//
		//sanity check
		//
		if(SUCCEEDED(hr))
		{
			//
			//disconnect - we'll handle the other events from this call later
			//
			pITBCC->Disconnect(DC_REJECTED);
	        pITBCC->Release();
		}
		return hr;
	}

	//
	//get the call if do not have already one
	//
    hr = pCall->QueryInterface( IID_ITBasicCallControl, (void**)&g_pCall );
	pCall->Release();
	if(FAILED(hr))
	{
		DoMessage( _T("Incoming call, but failed to QI ITBasicCallControl"));
	}
	else
	{
		//
		// update UI
		//
		SetStatusMessage(_T("Incoming Owner Call"));
	}

	//
	//clean up
	//

	return hr;
}

///////////////////////////////////////////////////////////////////
// DoCallState
//
// TE_CALLSTATE is a call state event.  pEvent is
// an ITCallStateEvent	
//
///////////////////////////////////////////////////////////////////
HRESULT DoCallState(
				IN IDispatch * pEvent)
{

    CALL_STATE         cs = CS_IDLE;
    ITCallStateEvent * pCallStateEvent = NULL;

    //
	// Get the interface
	//
    HRESULT hr = pEvent->QueryInterface( IID_ITCallStateEvent, (void **)&pCallStateEvent );
	if(FAILED(hr))
	{
        DoMessage( _T("CallStateEvent, but failed to get the interface"));
		return hr;
	}

	//
	//check if same call - if it is not do not process the event
	//
	if(false == SameCall(pCallStateEvent))
	{
		pCallStateEvent->Release();
		return hr;
	}

	//
    // get the CallState that we are being notified of.
	//
    hr = pCallStateEvent->get_State( &cs );

    pCallStateEvent->Release();

	if(FAILED(hr))
	{
        DoMessage( _T("ITCallStateEvent, but failed to get_State"));
		return hr;
	}

    //
	// send messages to main dialog procedure
	//
    if (CS_OFFERING == cs)
    {
		PostMessage(g_hDlg, WM_COMMAND, IDC_ANSWER, 0);
    }
    else if (CS_DISCONNECTED == cs)
    {
        PostMessage(g_hDlg, WM_COMMAND, IDC_DISCONNECTED, 0);
    }
    else if (CS_CONNECTED == cs)
    {
        PostMessage(g_hDlg, WM_COMMAND, IDC_CONNECTED, 0);
    }

	return hr;
}


///////////////////////////////////////////////////////////////////
// DoCallMedia
//
//
// TE_CALLMEDIA is a media event.  pEvent is an ITCallMediaEvent
//
//
///////////////////////////////////////////////////////////////////
HRESULT DoCallMedia(
				IN IDispatch * pEvent)
{

    CALL_MEDIA_EVENT    cme = CME_STREAM_INACTIVE;
    ITCallMediaEvent  * pCallMediaEvent = NULL;

	//
	// check if we still have a call
	//
	if(NULL == g_pCall)
	{
		return E_UNEXPECTED;
	}

    //
	// Get the interface
	//
    HRESULT hr = pEvent->QueryInterface( IID_ITCallMediaEvent, (void **)&pCallMediaEvent );
	if(FAILED(hr))
	{
        DoMessage( _T("ITCallMediaEvent, but failed to get the interface"));
		return hr;
	}

    //
	// get the CALL_MEDIA_EVENT that we are being notified of.
	//
    hr = pCallMediaEvent->get_Event( &cme );
	if(FAILED(hr))
	{
		pCallMediaEvent->Release();
		DoMessage( _T("ITCallMediaEvent, but failed to get_Event"));
		return hr;
	}

    switch ( cme ) 
    {
		// 
		// the only event we process
		//
        case CME_STREAM_ACTIVE:    
        {
            //
            // Get the terminal that's now active. 
            //    
            ITTerminal * pTerminal = NULL;
            hr = GetTerminalFromStreamEvent(pCallMediaEvent, &pTerminal);

            if ( FAILED(hr) )  
			{
				DoMessage(_T("ITCallMediaEvent: GetTerminalFromStreamEvent failed"));
				g_pCall->Disconnect(DC_NORMAL);
				break; 
			}

            //
            // Process this terminal based on the direction.
            //
            TERMINAL_DIRECTION td;
            hr = pTerminal->get_Direction( &td);
			//
			//clean up
			//
            pTerminal->Release(); 

            if ( FAILED(hr) ) 
            { 
	            DoMessage(_T("ITCallMediaEvent: get_Direction failed"));
				g_pCall->Disconnect(DC_NORMAL);
                break; 
            }

			//
			// if TD_CAPTURE and we have playback terminal start streaming
			//
            if ( TD_CAPTURE == td && NULL != g_pPlayFileTerm) 
            {
				ITMediaControl* pITMC = NULL;
				
				hr = g_pPlayFileTerm->QueryInterface(IID_ITMediaControl, (void**)&pITMC);
				//
				// get ITMediaControl so we can start streaming
				//
				if(FAILED(hr))
				{
					DoMessage(_T("ITCallMediaEvent: g_pPlayFileTerm QI for ITMediaControl failed"));
					g_pCall->Disconnect(DC_NORMAL);
					break;
				}
				//
				// Start streaming
				//
				hr = pITMC->Start();
				pITMC->Release();
				if(SUCCEEDED(hr))
				{
					SetStatusMessage(_T("File Playback Terminal started "));
				}
				else
				{
		            DoMessage(_T("ITCallMediaEvent: ITMediaControl::Start() failed"));
					g_pCall->Disconnect(DC_NORMAL);
				}
			}
        
            break;
        }
    
        default:
            break;
    }

    //
	// clean up
	//
    pCallMediaEvent->Release();

	return hr;
}

///////////////////////////////////////////////////////////////////
// DoFileEvent
//
//
// TE_FILETERMINAL is a file terminal event.  pEvent is an ITFileTerminalEvent
//
//
///////////////////////////////////////////////////////////////////
HRESULT DoFileEvent(
				IN IDispatch * pEvent)
{
			
	//
	// check if we still have a call
	//
	if(NULL == g_pCall)
	{
		return E_UNEXPECTED;
	}

	ITFileTerminalEvent* pITFTEvent = NULL;
	HRESULT hr = pEvent->QueryInterface( IID_ITFileTerminalEvent,
									 reinterpret_cast<void **>(&pITFTEvent) );

	if (FAILED(hr))
	{
		//
		// fatal error - can not continue - disconnect the call
		//
        DoMessage( _T("ITFileTerminalEvent, but failed to get the interface"));
		g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	// get the state - we'll make some decision based on this
	//
	TERMINAL_MEDIA_STATE ftState=TMS_ACTIVE;
	hr = pITFTEvent->get_State(&ftState);
	if(FAILED(hr))
	{
		//
		// fatal error - can not continue - disconnect the call
		//
		pITFTEvent->Release();
        DoMessage( _T("ITFileTerminalEvent, but failed to get_State"));
		g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	// we are interesred in TMS_IDLE because we will unselect playback and 
	// select recording
	//
	if(ftState != TMS_IDLE)
	{
		pITFTEvent->Release();
		return hr;
	}
	//
	// get the terminal
	//
	ITTerminal *pTerminal = NULL;
	hr = pITFTEvent->get_Terminal(&pTerminal);
	//
	// do not need this anymore
	//
	pITFTEvent->Release();

	if(FAILED(hr))
	{
		//
		// fatal error - can not continue - disconnect the call
		//
        DoMessage( _T("ITFileTerminalEvent, but failed to get_Terminal"));
		g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

    TERMINAL_DIRECTION td;
    hr = pTerminal->get_Direction( &td);
    pTerminal->Release(); 
	if(FAILED(hr))
	{
		//
		// fatal error - can not continue - disconnect the call
		//
        DoMessage( _T("ITFileTerminalEvent, but failed to get_Direction"));
		g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}
	if((td == TD_CAPTURE) && (NULL != g_pPlayFileTerm))
	{
		//
		// unselect playback - it is done - we reached the end or an error
		//
		ITBasicCallControl2 *pITBCC2 = NULL;
		hr = g_pCall->QueryInterface(IID_ITBasicCallControl2, (void**)&pITBCC2);
		if(FAILED(hr))
		{
			//
			// fatal error - can not continue - disconnect the call
			//
	        DoMessage( _T("ITFileTerminalEvent, but failed to QI for ITBasicCallControl2"));
			g_pCall->Disconnect(DC_NORMAL);
			return hr;
		}
		//
		// use ITBasicCallControl2 methods - much easier than 
		// enumerate stream, terminals, etc
		//
		hr = pITBCC2->UnselectTerminalOnCall(g_pPlayFileTerm);
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
		pITBCC2->Release();
		if(FAILED(hr))
		{
			//
			// fatal error - can not continue - disconnect the call
			//
	        DoMessage( _T("ITFileTerminalEvent, but failed to ITBasicCallControl2::UnselectTerminalOnCall"));
			g_pCall->Disconnect(DC_NORMAL);
			return hr;
		}
		//
		// select record - do not use automatic selection
		// we'll see how to use ITMultiTrackTerminal methods
		//
		hr = CreateAndSelectFileRecordTerminal();
		if(FAILED(hr))
		{
			//
			// fatal error - can not continue - disconnect the call
			//
			g_pCall->Disconnect(DC_NORMAL);
			SetStatusMessage(_T("CreateAndSelectFileRecordTerminal failed"));
			return hr;
		}

		//
		// get ITMediaControl interface - we need to call Start
		//
		ITMediaControl* pITMC = NULL;
		hr = g_pRecordFileTerm->QueryInterface(IID_ITMediaControl, (void**)&pITMC);
		if(FAILED(hr))
		{
			//
			// fatal error - can not continue - disconnect the call
			//
	        DoMessage( _T("ITFileTerminalEvent, but failed to QI ITMediaControl"));
			g_pCall->Disconnect(DC_NORMAL);
			return hr;
		}

		hr = pITMC->Start();
		pITMC->Release();
		if(FAILED(hr))
		{
			//
			// fatal error - can not continue - disconnect the call
			//
	        DoMessage( _T("ITFileTerminalEvent, but ITMediaControl::Start"));
			g_pCall->Disconnect(DC_NORMAL);
			return hr;
		}

		SetStatusMessage(_T("File Record Terminal started "));

		//
		// will stop recording after one minute
		//
		SetTimer(g_hDlg, TIMER_ID, MAX_REC_TIME, NULL);
		g_dwMessages++;

	}

	return hr;
}

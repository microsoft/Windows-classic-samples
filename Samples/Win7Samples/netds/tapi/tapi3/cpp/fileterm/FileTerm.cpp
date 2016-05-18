//////////////////////////////////////////////////////////////////////
//
//
//  Copyright (c) 1998-2001  Microsoft Corporation
//
//
//	Fileterm.cpp
//
// Sample application that handles incoming TAPI calls
// and uses the file terminals to perform the
// functions of a simple answering machine.
//
// This sample was adapted from the ANSMACH sample.
//
// In order to receive incoming calls, the application must
// implement and register the outgoing ITCallNotification
// interface.
//
// This application will register to receive calls on
// all addresses that support at least the audio media type.
//
// NOTES:
// 1. This application is limited to work with one
//    call at a time - all other calls will be rejected
// 2. This works for half-duplex modems and voice boards with the
//    wave MSP, as well as IP telephony with the H.323 MSP.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
#include <tapi3.h>

#include "FileTerm.h"
#include "callnot.h"
#include "resource.h"

//////////////////////////////////////////////////////////
//		GLOBALS
//////////////////////////////////////////////////////////

//
// Application dialog window handle
//
HWND                    g_hDlg = NULL;

//
// TAPI object, Call Object
//
ITTAPI *                g_pTapi = NULL;
ITBasicCallControl *    g_pCall = NULL;

//
// File Terminals
//
ITTerminal              *g_pPlayFileTerm = NULL;
ITTerminal				*g_pRecordFileTerm = NULL;

//
// Connection point cookie
//
ULONG                   g_ulAdvise = 0;

//////////////////////////////////////////////////////////
//
//		FUNCTIONS
//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// wWinMain - UNICODE application
//////////////////////////////////////////////////////////
int
WINAPI
WinMain(
        HINSTANCE hInst,
        HINSTANCE hPrevInst,
        LPSTR lpCmdLine,
        int nCmdShow
       )
{
	//
	// need to coinitialize
	//
    if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        return 0;
    }

	//
	// do all tapi initialization
	//
    if (S_OK == InitializeTapi())
    {
		//
		// everything is initialized, so start the main dialog box
		//
		DialogBox(
				  hInst,
				  MAKEINTRESOURCE(IDD_MAINDLG),
				  NULL,
				  MainDialogProc
				 );
	}

    //
    // When EndDialog is called, we get here; clean up and exit.
    //

    CoUninitialize();

    return 1;
}


//////////////////////////////////////////////////////////////
// InitializeTapi
//
// Create TAPI object, register callback object, 
// register addresses for call notifications
//
///////////////////////////////////////////////////////////////
HRESULT
InitializeTapi()
{
    HRESULT         hr;

    //
	// cocreate the TAPI object
	//
    hr = CoCreateInstance(
                          CLSID_TAPI,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTAPI,
                          (LPVOID *)&g_pTapi
                         );

    if (FAILED(hr))
    {
        DoMessage(_T("InitializeTapi: CoCreateInstance on TAPI failed"));
        return hr;
    }

    //
	// call initialize.  this must be called before any other tapi functions are called.
	//
    hr = g_pTapi->Initialize();

    if (FAILED(hr))
    {
        g_pTapi->Release();
        g_pTapi = NULL;
        DoMessage(_T("InitializeTapi: TAPI failed to initialize"));
        return hr;
    }

	//
	//register event callback object
	//
    hr = RegisterTapiEventInterface();
    if (FAILED(hr))
	{
        //
        // shutdown and release TAPI
        //		
        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;
        DoMessage(_T("InitializeTapi: RegisterTapiEventInterface failed"));
        return hr;	
	}

    //
	// put the Event filter to only give us only the events we process
	//
    hr = g_pTapi->put_EventFilter(TE_CALLNOTIFICATION | TE_CALLSTATE | TE_CALLMEDIA | TE_FILETERMINAL);
    if (FAILED(hr))
	{
        //
        // unregister callback
        //
		UnRegisterTapiEventInterface();
        //
        // shutdown and release TAPI
        //		
        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;
        DoMessage(_T("InitializeTapi: put_EventFilter failed"));
        return hr;	
	}


    // find all address objects that
    // we will use to listen for calls on
    hr = ListenOnAddresses();

    if (FAILED(hr))
    {
        //
        // unregister callback
        //
		UnRegisterTapiEventInterface();
        //
        // shutdown and release TAPI
        //		
        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;
        DoMessage(_T("InitializeTapi: ListenOnAddresses failed"));
    }

    return hr;

}


///////////////////////////////////////////////////////////////
// ShutdownTapi
//
// Called when application exits
// Disconnects the call and releases all internal objects
//
///////////////////////////////////////////////////////////////
void
ShutdownTapi()
{
    //
    // unregister callback
    //
	UnRegisterTapiEventInterface();
    //
    // if there is still a call, disconnect and release it
    //
    DisconnectTheCall();
	//
	//
	//
    ReleaseTheCall();
	//
	//wait for TAPI to cool off (2 sec. may be too short)
	//maybe there was an event sent by PostMessage but not processed yet
    //
	{
		//
		//we need an event for MsgWaitForMultipleObjectsEx
		//
		HANDLE hDumbEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		while(NULL != hDumbEvent)
		{
			
			DWORD dwStatus;
			MSG msg;

			dwStatus=::MsgWaitForMultipleObjectsEx(1,
										&hDumbEvent,
										TAPI_TIMEOUT,
										QS_ALLPOSTMESSAGE,
										MWMO_ALERTABLE);
			if(dwStatus != WAIT_TIMEOUT)
			{
				while(TRUE == ::PeekMessage(&msg,
						g_hDlg,//only from post thread
						NULL,
						NULL,
						PM_REMOVE))
				{
					//
					//if not TAPI event translate and dispatch it
					//
					if(msg.message != WM_PRIVATETAPIEVENT)
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
					//
					//TAPI event
					//
					else 
					{
						if((IDispatch *) msg.lParam != NULL)
						{
							//release the event
							((IDispatch *) msg.lParam)->Release();
							msg.lParam = NULL;
						}
					}
				}
			}
			else
			{
				::CloseHandle(hDumbEvent);
				hDumbEvent = NULL;
			}
		}
	}
    //
    // release main TAPI object.
    //

    if (NULL != g_pTapi)
    {
        g_pTapi->Shutdown();
        g_pTapi->Release();
		g_pTapi = NULL;

    }
}


///////////////////////////////////////////////////////////////////////////
// MainDlgProc
//
// Process dialog messages
//
///////////////////////////////////////////////////////////////////////////
INT_PTR
CALLBACK
MainDialogProc(
               HWND hDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
              )
{
    switch (uMsg)
    {
        case WM_PRIVATETAPIEVENT:
        {
			//
			//Process TAPI event
			//
	        OnTapiEvent(
                        (TAPI_EVENT) wParam,
                        (IDispatch *) lParam
                       );

            return 0;
        }

        case WM_INITDIALOG:
        {
            //
			// store dialog window handle
			//
            g_hDlg = hDlg;

            SetStatusMessage( _T("Waiting for a call..."));

            return 0;
        }

        case WM_COMMAND:
        {
            if ( LOWORD(wParam) == IDCANCEL )
            {

				ShutdownTapi();
                //
                // quit
                //
                EndDialog( hDlg, 0 );

                return 1;
            }

            switch ( LOWORD(wParam) )
            {
                //
				// offering notification
				//
                case IDC_ANSWER:
                {
                    SetStatusMessage(_T("Answering..."));
                    // answer the call
                    if ( S_OK == AnswerTheCall() )
                    {
                        SetStatusMessage(_T("Connecting..."));
                    }
                    else
                    {
                        DoMessage(_T("Answer failed"));
                        SetStatusMessage(_T("Waiting for a call..."));
                    }

                    return 1;
                }

                //
				// connect notification
				//
                case IDC_CONNECTED:
                {
                    SetStatusMessage(_T("Connected; waiting for file playback terminal to run..."));

                    return 1;
                }

                //
				// disconnected notification
				//
                case IDC_DISCONNECTED:
                {
                    //
					// release internal call objetcs
					//
                    ReleaseTheCall();

                    SetStatusMessage(_T("Waiting for a call..."));

                    return 1;
                }
                default:

                    return 0;
            }
        }
        case WM_TIMER:
        {
			//
			//disconnect the call when maximum record time is reached
			//it will unselect the terminal too
			//
            if ( g_pCall != NULL )
            {
                SetStatusMessage(_T("Disconnecting..."));

                //
				// disconnect the call
				//
                if (S_OK != DisconnectTheCall())
                {
                    DoMessage(_T("Disconnect failed"));
                }
            }

            return 1;
        }
        default:

            return 0;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// RegisterTapiEventInterface
//
// register call notification object used to receive events 
// fired by TAPI object
//
// The event notification interface is registered with TAPI 3.1 
// through the IConnectionPoint mechanism.  
// For more information on IConnectionPoint, and IConnectiontPointContainer
// please refer to the COM documentation.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT
RegisterTapiEventInterface()
{
    HRESULT                       hr = S_OK;
    IConnectionPointContainer   * pCPC = NULL;
    IConnectionPoint            * pCP = NULL;



    hr = g_pTapi->QueryInterface(
                                IID_IConnectionPointContainer,
                                (void **)&pCPC
                               );

    if (!SUCCEEDED(hr))
    {
        DoMessage(_T("RegisterTapiEventInterface: ")
                 _T("Failed to get IConnectionPointContainer on TAPI"));
        return hr;
    }

    hr = pCPC->FindConnectionPoint(
                                   IID_ITTAPIEventNotification,
                                   &pCP
                                  );
    pCPC->Release();

    if (!SUCCEEDED(hr))
    {
        DoMessage(_T("RegisterTapiEventInterface: Failed to get a connection point for ")
                 _T("ITTAPIEventNotification"));
        return hr;
    }


	//
	//create event callback object
	//
    CTAPIEventNotification* pTAPIEventNotification = new CTAPIEventNotification;
	if(NULL == pTAPIEventNotification)
	{
        DoMessage(_T("RegisterTapiEventInterface: Could not create CTAPIEventNotification"));
        return E_OUTOFMEMORY;
	}
	//
	//advise the callback object - it will be released by Unadvise
	//
    hr = pCP->Advise(
                      pTAPIEventNotification,
                      &g_ulAdvise
                     );

    pCP->Release();
    //
    // whether Advise failed or succeeded - Advise already addrefed the object, 
	// we no longer need a reference to the callback object
    //
    pTAPIEventNotification->Release();
    pTAPIEventNotification = NULL;	
 
    if (FAILED(hr))
	{
        DoMessage(_T("RegisterTapiEventInterface: Advise failed"));
	}


    return hr;

}



///////////////////////////////////////////////////////////////////////////////
//
// UnRegisterTapiEventInterface
//
// Unadvise call notification object.
//
// The event notification interface was registered with TAPI 3.1 
// through the IConnectionPoint mechanism.  
// For more information on IConnectionPoint, and IConnectiontPointContainer
// please refer to the COM documentation.
//
///////////////////////////////////////////////////////////////////////////////

HRESULT UnRegisterTapiEventInterface()
{
    HRESULT hr = S_OK;
    IConnectionPointContainer *pCPC = NULL;
    IConnectionPoint *pCP = NULL;

    //
    // get connection point container on the tapi object
    // 

    hr = g_pTapi->QueryInterface(IID_IConnectionPointContainer,
                                (void **)&pCPC);

    if (FAILED(hr))
    {
        DoMessage(_T("UnRegisterTapiEventInterface: ")
                 _T("Failed to get IConnectionPointContainer on TAPI"));
        return hr;
    }


    //
    // find connection point for our interface
    //


    hr = pCPC->FindConnectionPoint(IID_ITTAPIEventNotification,
                                           &pCP);

    pCPC->Release();
    pCPC = NULL;

    if (FAILED(hr))
    {
        DoMessage(_T("UnRegisterTapiEventInterface: Failed to get a connection point for ")
                 _T("ITTAPIEventNotification"));

        return hr;
    }


    // 
    // unregister the callback
    //
    
    hr = pCP->Unadvise(g_ulAdvise);

    pCP->Release();
    pCP = NULL;

    g_ulAdvise = 0;

    if (FAILED(hr))
	{
        DoMessage(_T("UnRegisterTapiEventInterface: Unadvise failed"));
	}

    return hr;
}


////////////////////////////////////////////////////////////////////////
// ListenOnAddresses
//
// This function will find all addresses that support audio in and audio out
// and will call ListenOnThisAddress to start listening on it.
////////////////////////////////////////////////////////////////////////

HRESULT
ListenOnAddresses()
{
    HRESULT             hr = S_OK;
    IEnumAddress *      pEnumAddress = NULL;
    ITAddress *         pAddress = NULL;
    ITMediaSupport *    pMediaSupport = NULL;
    VARIANT_BOOL        bSupport = VARIANT_FALSE;
	BOOL				bAddressFound = FALSE;

    // enumerate the addresses
    hr = g_pTapi->EnumerateAddresses( &pEnumAddress );

    if (FAILED(hr))
    {
        DoMessage(_T("ListenOnAddresses: EnumerateAddresses failed"));
        return hr;
    }

	//
	// loop for all addresses
	//

    while ( S_OK == pEnumAddress->Next( 1, &pAddress, NULL ) )
    {

        hr = pAddress->QueryInterface( IID_ITMediaSupport, (void **)&pMediaSupport );
		if(FAILED(hr))
		{
	        pAddress->Release();
			continue;
		}

		//
        // does it support Audio
		//
        hr = pMediaSupport->QueryMediaType(
                                      TAPIMEDIATYPE_AUDIO,
                                      &bSupport
                                     );

        pMediaSupport->Release();

		if(FAILED(hr))
		{
	        pAddress->Release();
			continue;
		}

        if (bSupport)
        {
            //
			// If it does support then we'll listen.
			//
            hr = ListenOnThisAddress( pAddress );
            if (SUCCEEDED(hr))
            {
                bAddressFound = TRUE;
            }
        }
        pAddress->Release();
    }

	//
	//clean up
	//

    pEnumAddress->Release();

	if(!bAddressFound)
	{
        DoMessage(_T("ListenOnAddresses: not address found"));
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}

}


///////////////////////////////////////////////////////////////////
// ListenOnThisAddress
//
// We call RegisterCallNotifications to inform TAPI that we want
// notifications of calls on this address. We already resistered
// our notification interface with TAPI, so now we are just telling
// TAPI that we want calls from this address to trigger events on
// our existing notification interface.
//
///////////////////////////////////////////////////////////////////

HRESULT
ListenOnThisAddress(
					IN ITAddress * pAddress)
{

    //
    // RegisterCallNotifications takes a media type bitmap indicating
    // the set of media types we are interested in. We know the
    // address supports audio.
    //


    long     lRegister = 0;

    return g_pTapi->RegisterCallNotifications(
                                           pAddress,
                                           VARIANT_FALSE,
                                           VARIANT_TRUE,
                                           TAPIMEDIATYPE_AUDIO,
                                           0,
                                           &lRegister
                                          );

}


/////////////////////////////////////////////////////////////////////
//
// Answers the call
// It will create and select the Playback terminal
//
/////////////////////////////////////////////////////////////////////

HRESULT
AnswerTheCall()
{

	//
	// check if call available
	//
    if (NULL == g_pCall)
    {
        DoMessage( _T("AnswerTheCall: call not available"));
        return E_UNEXPECTED;
    }

	//
	// it should not happen, but if you have already the terminal release it
	//
    if(NULL != g_pPlayFileTerm)
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
	}

    //
	// prepare to request the terminal - need ITBasicCallControl2 interface
	//
    ITBasicCallControl2*    pITBCC2 = NULL;
    HRESULT hr = g_pCall->QueryInterface( IID_ITBasicCallControl2, (void**)&pITBCC2 );
	if(FAILED(hr))
	{
        DoMessage( _T("AnswerTheCall: QI ITBasicCallControl2 failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

    //
	// prepare to request the terminal - need CLSID BSTR 
	//
	BSTR bstrCLSID = NULL;
	hr = StringFromCLSID(CLSID_FilePlaybackTerminal, &bstrCLSID);
	if(FAILED(hr))
	{
		pITBCC2->Release();
        DoMessage( _T("AnswerTheCall: StringFromCLSID failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	//request the terminal using right media type and direction
	//
	hr = pITBCC2->RequestTerminal(bstrCLSID, 
									TAPIMEDIATYPE_AUDIO, 
									TD_CAPTURE, 
									&g_pPlayFileTerm);
	//
	//clean up
	//
	::CoTaskMemFree(bstrCLSID);

	if(FAILED(hr))
	{
		pITBCC2->Release();
        DoMessage( _T("AnswerTheCall: RequestTerminal failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	// prepare to put the file name
	//
	BSTR bstrFileName = ::SysAllocString(PLAY_FILENAME);

	if(NULL == bstrFileName)
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
		pITBCC2->Release();
        DoMessage( _T("AnswerTheCall: SysAllocString for play list failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return E_OUTOFMEMORY;
	}

	//
	// call helper method for put_PlayList
	//
	hr = PutPlayList(g_pPlayFileTerm, bstrFileName);

	//
	// free the file name
	//
	::SysFreeString(bstrFileName);

	if(FAILED(hr))
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
		pITBCC2->Release();
		DoMessage( _T("AnswerTheCall: PutPlayList failed"));
		g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	//select the terminal using ITBasicCallControl2 method
	//
	hr = pITBCC2->SelectTerminalOnCall(g_pPlayFileTerm);
	if(FAILED(hr))
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
		pITBCC2->Release();
        DoMessage( _T("AnswerTheCall: SelectTerminalOnCall failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	//finally answer the call
	//
    hr = g_pCall->Answer();
	if(FAILED(hr))
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
		pITBCC2->Release();
        DoMessage( _T("AnswerTheCall: Answer failed"));
        g_pCall->Disconnect(DC_NORMAL);
		return hr;
	}

	//
	//if we are here we are OK
	//
	pITBCC2->Release();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// DisconnectTheCall
//
// Disconnects the call - the objects will be released when
// processing disconnect notification
//
//////////////////////////////////////////////////////////////////////

HRESULT
DisconnectTheCall()
{
	//
	//disconnect the call
	//
    if (NULL != g_pCall)
    {
		//
		// we will release the call and terminals on disconnected notification.
		//
        return g_pCall->Disconnect( DC_NORMAL );

    }

    return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
// ReleaseTheCall
//
// Releases the objects used for call processing
//
//////////////////////////////////////////////////////////////////////

void
ReleaseTheCall()
{
	//
	//do not the timer anymore anymore
	//
	KillTimer(g_hDlg, TIMER_ID);

	//
	//release the terminals
	//
	if(NULL != g_pPlayFileTerm)
	{
		g_pPlayFileTerm->Release();
		g_pPlayFileTerm = NULL;
	}
	if(NULL != g_pRecordFileTerm)
	{
		g_pRecordFileTerm->Release();
		g_pRecordFileTerm = NULL;
	}
	//
	//release the call object - this will unselect all selected terminals
	//
    if (NULL != g_pCall)
    {
        g_pCall->Release();
        g_pCall = NULL;
    }
}


///////////////////////////////////////////////////////////////////
//
// HELPER FUNCTIONS
//
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
// DoMessage - pops up a message box
//
///////////////////////////////////////////////////////////////////

void
DoMessage(
			IN LPWSTR pszMessage)
{
    ::MessageBox(
               g_hDlg,
               pszMessage,
               MSGBOX_NAME,
               MB_OK
              );
}


//////////////////////////////////////////////////////////////////
//
// SetStatusMessage - change the status 
//
//////////////////////////////////////////////////////////////////

void
SetStatusMessage(
			IN LPWSTR pszMessage)
{
    ::SetDlgItemText(
                   g_hDlg,
                   IDC_STATUS,
                   pszMessage
                  );
}

//////////////////////////////////////////////////////////////////
//
// PutPlayList - prepare the variant for put_PlayList method
// and invoke the method 
//
//////////////////////////////////////////////////////////////////

HRESULT PutPlayList(
			IN ITTerminal *pITTerminal, 
			IN BSTR bstrFileName)
{
	//
	//check if really have a terminal
	//
	if(NULL == pITTerminal)
	{
        DoMessage( _T("PutPlayList: playback terminal NULL"));
		return E_UNEXPECTED;
	}

	//
    // Get ITMediaPlayback interface - only playback terminal object 
	// exposes this interface
	//
    ITMediaPlayback*    pMediaPlayback = NULL;

    HRESULT hr = pITTerminal->QueryInterface(
										IID_ITMediaPlayback,
										(void**)&pMediaPlayback);


	if(FAILED(hr))
	{
        DoMessage( _T("PutPlayList: QI ITMediaPlayback failed"));
		return hr;
	}

	//
	//VARIANT to be passed to put_PlayList
	//
	VARIANT varPlaylist;
	VariantInit(&varPlaylist);

	//
	//Prepare SAFEARRAYBOUND for SAFEARRAY
	//Put file name into array with one element
	//
    SAFEARRAYBOUND DimensionBounds;
    DimensionBounds.lLbound = 1;

	//
	//number of files in play list - modify for more files
	//
    DimensionBounds.cElements = 1;

    //
	// Put file name into array at index 1 - see lLbound
	//
    long lArrayPos = 1;

	//
	//variant that will hold the BSTR - it will be added to SAFEARRAY
	//
    VARIANT* pvarArrayEntry = new VARIANT;
    if( pvarArrayEntry == NULL)
    {
        DoMessage( _T("PutPlayList: new VARIANT failed"));
        return E_OUTOFMEMORY;
    }
	VariantInit(pvarArrayEntry);

    //
	// Create SAFEARRAY
	//
    SAFEARRAY *pPlayListArray = NULL;
    pPlayListArray = SafeArrayCreate( VT_VARIANT, 1, &DimensionBounds);
    if( pPlayListArray == NULL)
    {
        DoMessage( _T("PutPlayList: SafeArrayCreate failed"));
		delete pvarArrayEntry;
        return E_OUTOFMEMORY;
    }

	//
	//repeat this for each file you want to add
	//you need to increment lArrayPos
	//
    pvarArrayEntry->vt = VT_BSTR;
    pvarArrayEntry->bstrVal = ::SysAllocString(bstrFileName);
    SafeArrayPutElement( pPlayListArray, &lArrayPos, pvarArrayEntry);
	VariantClear(pvarArrayEntry);

	//
	//prepare the variant for put_PlayList
	//
    V_VT(&varPlaylist) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(&varPlaylist) = pPlayListArray;

	//
	//finally put play list
	//
    hr = pMediaPlayback->put_PlayList(varPlaylist);
    if(FAILED(hr))
    {
        DoMessage( _T("PutPlayList: put_PlayList failed"));
		delete pvarArrayEntry;
        return E_OUTOFMEMORY;
    }

	//
	//clean up
	//
	delete pvarArrayEntry;
    pMediaPlayback->Release();
	VariantClear(&varPlaylist);

    return hr;
}
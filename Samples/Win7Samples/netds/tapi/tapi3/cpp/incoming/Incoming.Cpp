/*++

Copyright (c) 2000 - 2000 Microsoft Corporation.  All Rights Reserved.

--*/

#include <windows.h>
#include <tapi3.h>
#include <control.h>
#include "callnot.h"
#include "resource.h"

//////////////////////////////////////////////////////////
// Incoming.cpp
//
// Sample application that handles incoming TAPI calls.
// In order to receive incoming calls, the application must
// implement and register the outgoing ITCallNotification
// interface.
//
// This application will register to receive calls on
// all addresses that support at least the audio media type.
//
// NOTE:  This application is limited to working with one
// call at at time, and will not work correctly if multiple
// calls are present at the same time.
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////

const DWORD MAXTERMINALS    = 5;

//////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////

HINSTANCE               ghInst;
ITTAPI *                gpTapi;
ITBasicCallControl *    gpCall;
HWND                    ghDlg = NULL;

BOOL                    gfAutoAnswer = FALSE;

WCHAR gszTapi30[] = L"TAPI 3.0 Incoming Call Sample";


ULONG                         gulAdvise;

//////////////////////////////////////////////////////////
// PROTOTYPES
//////////////////////////////////////////////////////////

INT_PTR
CALLBACK
MainDialogProc(
               HWND hDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
              );

HRESULT
RegisterTapiEventInterface(CTAPIEventNotification *pTAPIEventNotification);

HRESULT
ListenOnAddresses();

HRESULT
ListenOnThisAddress(
                    ITAddress * pAddress
                   );

HRESULT
AnswerTheCall();

HRESULT
DisconnectTheCall();

void
ReleaseTheCall();

void
DoMessage(
          LPWSTR pszMessage
         );

void
SetStatusMessage(
                 LPWSTR pszMessage
                );

HRESULT
InitializeTapi();

void
ShutdownTapi();

void
EnableButton(
             int ID
            );
void
DisableButton(
              int ID
             );

BOOL
AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        );

//////////////////////////////////////////////////////////
//
//              FUNCTIONS
//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// WinMain
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
    ghInst = hInst;

    //
    // Initialize COM.
    //

    if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        return 0;
    }

    //
    // do all tapi initialization
    //

    if (S_OK != InitializeTapi())
    {
        return 0;
    }

    //
    // everything is initialized, so
    // start the main dialog box
    //

    DialogBox(
              ghInst,
              MAKEINTRESOURCE(IDD_MAINDLG),
              NULL,
              MainDialogProc
             );


    //
    // clean up tapi
    //

    ShutdownTapi();

    //
    // Shut down COM.
    //

    CoUninitialize();

    return 1;
}


//////////////////////////////////////////////////////////////
// InitializeTapi
//
// Various tapi initializations
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
                          (LPVOID *)&gpTapi
                         );

    if ( FAILED(hr) )
    {
        DoMessage(L"CoCreateInstance on TAPI failed");
        return hr;
    }

    //
    // call initialize.  this must be called before
    // any other tapi functions are called.
    //

    hr = gpTapi->Initialize();

    if ( FAILED(hr) )
    {
        DoMessage(L"TAPI failed to initialize");

        gpTapi->Release();
        gpTapi = NULL;

        return hr;
    }

    //
    // Create our own event notification object and register it
    // see callnot.h and callnot.cpp
    //

    CTAPIEventNotification *pTAPIEventNotification = new CTAPIEventNotification;

    hr = RegisterTapiEventInterface(pTAPIEventNotification);


    //
    // no longer need a reference to the callback object. tapi will keep a 
    // reference to it for as long as it is using it.
    //

    pTAPIEventNotification->Release();



    // Set the Event filter to only give us only the events we process
    gpTapi->put_EventFilter(TE_CALLNOTIFICATION | TE_CALLSTATE | TE_CALLMEDIA);


    //
    // find all address objects that
    // we will use to listen for calls on
    //

    hr = ListenOnAddresses();

    if ( FAILED(hr) )
    {
        DoMessage(L"Could not find any addresses to listen on");

        gpTapi->Release();
        gpTapi = NULL;

        return hr;
    }

    return S_OK;
}


///////////////////////////////////////////////////////////////
// ShutdownTapi
///////////////////////////////////////////////////////////////

void
ShutdownTapi()
{
    //
    // if there is still a call, release it
    //

    if (NULL != gpCall)
    {
        gpCall->Release();
        gpCall = NULL;
    }

    //
    // release main object.
    //

    if (NULL != gpTapi)
    {
        gpTapi->Shutdown();
        gpTapi->Release();
    }

}


///////////////////////////////////////////////////////////////////////////
// MainDlgProc
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
            OnTapiEvent(
                        (TAPI_EVENT) wParam,
                        (IDispatch *) lParam
                       );

            return 0;
        }

        case WM_INITDIALOG:
        {
            //
            // set up dialog
            //

            ghDlg = hDlg;

            DisableButton( IDC_ANSWER );
            DisableButton( IDC_DISCONNECT );

            SetStatusMessage( L"Waiting for a call..." );

            return 0;
        }

        case WM_COMMAND:
        {
            if ( LOWORD(wParam) == IDCANCEL )
            {
                //
                // quit
                //

                EndDialog( hDlg, 0 );

                return 1;
            }

            switch ( LOWORD(wParam) )
            {
                case IDC_AUTOANSWER:
                {
                    //
                    // auto answer check box was fiddled around with
                    //

                    gfAutoAnswer = !gfAutoAnswer;
                    return 1;
                }

                case IDC_ANSWER:
                {
                    //
                    // answer the call
                    //

                    SetStatusMessage(L"Answering...");

                    if ( S_OK == AnswerTheCall() )
                    {
                        SetStatusMessage(L"Connected");

                        EnableButton( IDC_DISCONNECT );
                        DisableButton( IDC_ANSWER );
                    }
                    else
                    {
                        DisableButton( IDC_ANSWER );
                        DoMessage(L"Answer failed");
                        SetStatusMessage(L"Waiting for a call...");
                    }

                    return 1;
                }

                case IDC_DISCONNECT:
                {
                    //
                    // disconnect request
                    //

                    SetStatusMessage(L"Disconnecting...");
                    if (S_OK != DisconnectTheCall())
                    {
                        DoMessage(L"Disconnect failed");
                    }

                    return 1;
                }

                case IDC_DISCONNECTED:
                {
                    //
                    // disconnected notification -- release the call
                    //

                    ReleaseTheCall();

                    EnableButton( IDOK );
                    DisableButton( IDC_DISCONNECT );

                    SetStatusMessage(L"Waiting for a call...");

                    return 1;
                }
                default:

                    return 0;
            }
        }
        default:

            return 0;
    }
}


///////////////////////////////////////////////////////////////////////////
// RegisterTapiEventInterface(CTAPIEventNotification *pTAPIEventNotification)
///////////////////////////////////////////////////////////////////////////

HRESULT
RegisterTapiEventInterface(CTAPIEventNotification *pTAPIEventNotification)
{
    HRESULT                       hr = S_OK;
    IConnectionPointContainer   * pCPC;
    IConnectionPoint            * pCP;


    hr = gpTapi->QueryInterface(
                                IID_IConnectionPointContainer,
                                (void **)&pCPC
                               );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCPC->FindConnectionPoint(
                                   IID_ITTAPIEventNotification,
                                   &pCP
                                  );

    pCPC->Release();

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCP->Advise(
                      pTAPIEventNotification,
                      &gulAdvise
                     );

    pCP->Release();


    return hr;

}


////////////////////////////////////////////////////////////////////////
// ListenOnAddresses
//
// This procedure will find all addresses that support audioin and audioout
// and will call ListenOnThisAddress to start listening on it.
////////////////////////////////////////////////////////////////////////

HRESULT
ListenOnAddresses()
{
    HRESULT             hr = S_OK;
    IEnumAddress *      pEnumAddress;
    ITAddress *         pAddress;

    //
    // enumerate the addresses
    //

    hr = gpTapi->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        return hr;
    }

    while ( TRUE )
    {
        //
        // get the next address
        //

        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {
            break;
        }

        //
        // does the address support audio?
        //

        if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_AUDIO) )
        {
            //
            // If it does then we'll listen.
            //

            hr = ListenOnThisAddress( pAddress );

            if ( FAILED(hr) )
            {
                DoMessage(L"Listen failed on an address");
            }
        }

        pAddress->Release();
    }

    pEnumAddress->Release();

    return S_OK;
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
                    ITAddress * pAddress
                   )
{

    //
    // RegisterCallNotifications takes a media type bitmap indicating
    // the set of media types we are interested in. We know the
    // address supports audio, but let's add in video as well
    // if the address supports it.
    //

    long lMediaTypes = TAPIMEDIATYPE_AUDIO;

    if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_VIDEO) )
    {
        lMediaTypes |= TAPIMEDIATYPE_VIDEO;
    }


    HRESULT  hr;
    long     lRegister;

    hr = gpTapi->RegisterCallNotifications(
                                           pAddress,
                                           VARIANT_TRUE,
                                           VARIANT_TRUE,
                                           lMediaTypes,
                                           0,
                                           &lRegister
                                          );

    return hr;
}

/////////////////////////////////////////////////////////////////
// IsVideoCaptureStream
//
// Returns true if the stream is for video capture
/////////////////////////////////////////////////////////////////

BOOL
IsVideoCaptureStream(
                     ITStream * pStream
                    )
{
    TERMINAL_DIRECTION tdStreamDirection;
    long               lStreamMediaType;

    if ( FAILED( pStream  ->get_Direction(&tdStreamDirection)   ) ) { return FALSE; }
    if ( FAILED( pStream  ->get_MediaType(&lStreamMediaType)    ) ) { return FALSE; }

    return (tdStreamDirection == TD_CAPTURE) &&
           (lStreamMediaType  == TAPIMEDIATYPE_VIDEO);
}

/////////////////////////////////////////////////////////
// GetVideoRenderTerminal
//
// Creates a dynamic terminal for the Video Render mediatype / direction
//
/////////////////////////////////////////////////////////
HRESULT
GetVideoRenderTerminal(
                   ITAddress   * pAddress,
                   ITTerminal ** ppTerminal
                  )
{
    //
    // Construct a BSTR for the correct IID.
    //

    LPOLESTR            lpTerminalClass;

    HRESULT             hr;

    hr = StringFromIID(CLSID_VideoWindowTerm,
                       &lpTerminalClass);

    if ( SUCCEEDED(hr) )
    {
        BSTR                bstrTerminalClass;

        bstrTerminalClass = SysAllocString ( lpTerminalClass );

        CoTaskMemFree( lpTerminalClass );

        if ( bstrTerminalClass == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {

            //
            // Get the terminal support interface
            //

            ITTerminalSupport * pTerminalSupport;

            hr = pAddress->QueryInterface(IID_ITTerminalSupport,
                                          (void **)&pTerminalSupport);

            if ( SUCCEEDED(hr) )
            {
                //
                // Create the video render terminal.
                //

                hr = pTerminalSupport->CreateTerminal(bstrTerminalClass,
                                                      TAPIMEDIATYPE_VIDEO,
                                                      TD_RENDER,
                                                      ppTerminal);

                pTerminalSupport->Release();

                //
                // We do not make the window visible now -- we will do that
                // when the CME_STREAM_ACTIVE event is received for the stream
                // (and we have placed the window within our frame).
                //
            }

            SysFreeString( bstrTerminalClass );
        }
    }

    return hr;
}


/////////////////////////////////////////////////////////////////
// EnablePreview
//
// Selects a video render terminal on a video capture stream,
// thereby enabling video preview.
/////////////////////////////////////////////////////////////////

HRESULT
EnablePreview(
              ITAddress * pAddress,
              ITStream * pStream
             )
{
    ITTerminal * pTerminal;

    HRESULT hr = GetVideoRenderTerminal(pAddress, &pTerminal);

    if ( SUCCEEDED(hr) )
    {
        hr = pStream->SelectTerminal(pTerminal);

        pTerminal->Release();
    }

    return hr;
}


/////////////////////////////////////////////////////////
// GetTerminal
//
// Creates the default terminal for the passed-in stream.
//
/////////////////////////////////////////////////////////
HRESULT
GetTerminal(
            ITAddress   * pAddress,
            ITStream    * pStream,
            ITTerminal ** ppTerminal
           )
{
    //
    // Determine the media type and direction of this stream.
    //

    HRESULT            hr;
    long               lMediaType;
    TERMINAL_DIRECTION dir;

    hr = pStream->get_MediaType( &lMediaType );
    if ( FAILED(hr) ) return hr;

    hr = pStream->get_Direction( &dir );
    if ( FAILED(hr) ) return hr;

    //
    // Since video render is a dynamic terminal, the procedure for creating
    // it is different.
    //

    if ( ( lMediaType == TAPIMEDIATYPE_VIDEO ) &&
         ( dir        == TD_RENDER ) )
    {
        return GetVideoRenderTerminal(pAddress, ppTerminal);
    }

    //
    // For all other terminals we use GetDefaultStaticTerminal.
    // First, get the terminal support interface.
    //

    ITTerminalSupport * pTerminalSupport;

    hr = pAddress->QueryInterface( IID_ITTerminalSupport,
                                   (void **)&pTerminalSupport);

    if ( SUCCEEDED(hr) )
    {
        //
        // get the default terminal for this MediaType and direction
        //

        hr = pTerminalSupport->GetDefaultStaticTerminal(lMediaType,
                                                        dir,
                                                        ppTerminal);

        pTerminalSupport->Release();
    }

    return hr;

}

/////////////////////////////////////////////////////////////////
// SelectTerminalOnCall
//
// Selects a given terminal on the first compatible stream that
// exists on the given call.
/////////////////////////////////////////////////////////////////

HRESULT
SelectTerminalsOnCall(
                     ITAddress * pAddress,
                     ITBasicCallControl * pCall
                     )
{
    HRESULT hr;

    //
    // get the ITStreamControl interface for this call
    //

    ITStreamControl * pStreamControl;

    hr = pCall->QueryInterface(IID_ITStreamControl,
                               (void **) &pStreamControl);

    if ( SUCCEEDED(hr) )
    {
        //
        // enumerate the streams
        //

        IEnumStream * pEnumStreams;

        hr = pStreamControl->EnumerateStreams(&pEnumStreams);

        pStreamControl->Release();

        if ( SUCCEEDED(hr) )
        {
            //
            // for each stream
            //

            ITStream * pStream;

            while ( S_OK == pEnumStreams->Next(1, &pStream, NULL) )
            {
                ITTerminal * pTerminal;

                //
                // Find out the media type and direction of this stream,
                // and create the default terminal for this media type and
                // direction.
                //

                hr = GetTerminal(pAddress,
                                 pStream,
                                 &pTerminal);

                if ( SUCCEEDED(hr) )
                {
                    //
                    // Select the terminal on the stream.
                    //

                    hr = pStream->SelectTerminal(pTerminal);

                    if ( SUCCEEDED(hr) )
                    {
                        //
                        // Also enable preview on the video capture stream.
                        //

                        if ( IsVideoCaptureStream( pStream ) )
                        {
                            EnablePreview( pAddress, pStream );
                        }
                    }

                    pTerminal->Release();
                }

                pStream->Release();
            }

            pEnumStreams->Release();
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Answer the call
/////////////////////////////////////////////////////////////////////

HRESULT
AnswerTheCall()
{
    HRESULT                 hr;
    ITCallInfo *            pCallInfo;
    ITAddress *             pAddress;



    if (NULL == gpCall)
    {
        return E_UNEXPECTED;
    }


    //
    // get the address object of this call
    //

    hr = gpCall->QueryInterface( IID_ITCallInfo, (void**)&pCallInfo );

    if ( FAILED(hr) )
    {
        gpCall->Release();
        gpCall = NULL;

        return hr;
    }

    hr = pCallInfo->get_Address( &pAddress );

    pCallInfo->Release();

    if ( FAILED(hr) )
    {
        gpCall->Release();
        gpCall = NULL;

        return hr;
    }

    //
    // Select our terminals on the call; if any of the selections fail
    // we proceed without that terminal
    //

    hr = SelectTerminalsOnCall( pAddress, gpCall );

    pAddress->Release();

    //
    // Now we can actually answer the call
    //

    hr = gpCall->Answer();

    return hr;
}

//////////////////////////////////////////////////////////////////////
// DisconnectTheCall
//
// Disconnects the call
//////////////////////////////////////////////////////////////////////

HRESULT
DisconnectTheCall()
{
    HRESULT         hr = S_OK;

    if (NULL != gpCall)
    {
        hr = gpCall->Disconnect( DC_NORMAL );

        // Do not release the call yet, as that would prevent
        // us from receiving the disconnected notification.

        return hr;
    }

    return S_FALSE;
}

//////////////////////////////////////////////////////////////////////
// ReleaseTheCall
//
// Releases the call
//////////////////////////////////////////////////////////////////////
void
ReleaseTheCall()
{
    if (NULL != gpCall)
    {
        gpCall->Release();
        gpCall = NULL;
    }
}


///////////////////////////////////////////////////////////////////
//
// HELPER FUNCTIONS
//
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// DoMessage
///////////////////////////////////////////////////////////////////

void
DoMessage(
          LPWSTR pszMessage
         )
{
    MessageBox(
               ghDlg,
               pszMessage,
               gszTapi30,
               MB_OK
              );
}


//////////////////////////////////////////////////////////////////
// SetStatusMessage
//////////////////////////////////////////////////////////////////

void
SetStatusMessage(
                 LPWSTR pszMessage
                )
{
    SetDlgItemText(
                   ghDlg,
                   IDC_STATUS,
                   pszMessage
                  );
}

///////////////////////////////////////////////////////////////
// EnableButton
//
// Enable, make default, and setfocus to a button
///////////////////////////////////////////////////////////////

void
EnableButton(
             int ID
            )
{
    SendDlgItemMessage(
                       ghDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_DEFPUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( ghDlg, ID ),
                 TRUE
                );
    SetFocus(
             GetDlgItem( ghDlg, ID )
            );
}

//////////////////////////////////////////////////////////////
// DisableButton
//
// Disable a button
//////////////////////////////////////////////////////////////

void
DisableButton(
              int ID
             )
{
    SendDlgItemMessage(
                       ghDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_PUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( ghDlg, ID ),
                 FALSE
                );
}

//////////////////////////////////////////////////////////////
// AddressSupportsMediaType
//
// Finds out if the given address supports the given media
// type, and returns TRUE if it does.
//////////////////////////////////////////////////////////////

BOOL
AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        )
{
    VARIANT_BOOL     bSupport = VARIANT_FALSE;
    ITMediaSupport * pMediaSupport;

    if ( SUCCEEDED( pAddress->QueryInterface( IID_ITMediaSupport,
                                              (void **)&pMediaSupport ) ) )
    {
        //
        // does it support this media type?
        //

        pMediaSupport->QueryMediaType(
                                      lMediaType,
                                      &bSupport
                                     );

        pMediaSupport->Release();
    }

    return (bSupport == VARIANT_TRUE);
}

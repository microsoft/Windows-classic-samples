/*++

Copyright (c) 1999 - 2000 Microsoft Corporation.  All Rights Reserved.

--*/

#define UNICODE
#include <windows.h>
#include <tapi3.h>
#include <strmif.h>
#include <control.h>
#include <stdio.h>
#include "resource.h"

//////////////////////////////////////////////////////////
// T3OUT.EXE
//
// Example of making an outgoing call with TAPI 3.0
//
// This application will allow a user to make a call
// by using TAPI 3.0.  The application will simply look
// for the first TAPI line that support Audio, and can
// dial a phone number.  It will then use that line to
// make calls.
//
// This application does not handle incoming calls, and
// does not process incoming messages.
//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////

const DWORD ADDRESSLENGTH   = 128;
const DWORD MAXTERMINALS    = 5;

const WCHAR * const gszTapi30           = L"TAPI 3.0 Outgoing Call Sample";

const WCHAR * const gszConferenceName   = L"Conference Name";
const WCHAR * const gszEmailName        = L"Email Name";
const WCHAR * const gszMachineName      = L"Machine Name";
const WCHAR * const gszPhoneNumber      = L"Phone Number";
const WCHAR * const gszIPAddress        = L"IP Address";

//////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////
HINSTANCE               ghInst;
HWND                    ghDlg = NULL;
ITTAPI *                gpTapi;
ITAddress *             gpAddress = NULL;
ITBasicCallControl *    gpCall;

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
FindAnAddress(
              DWORD dwAddressType,
              BSTR  * ppName
             );

HRESULT
GetTerminal(
            ITStream * pStream,
            ITTerminal ** ppTerminal
           );

HRESULT
GetVideoRenderTerminal(
                   ITTerminal ** ppTerminal
                  );

HRESULT
MakeTheCall(
            DWORD dwAddressType,
            PWCHAR szAddressToCall
           );

HRESULT
DisconnectTheCall();

void
DoMessage(
          LPWSTR pszMessage
         );

HRESULT
InitializeTapi();

void
ShutdownTapi();

void
EnableButton(
             HWND hDlg,
             int ID
            );
void
DisableButton(
              HWND hDlg,
              int ID
             );

BOOL
AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        );

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


    // need to coinit
    if ( FAILED( CoInitialize(NULL) ) )
    {
        return 0;
    }

    if ( FAILED( InitializeTapi() ) )
    {
        return 0;
    }

    // everything is initialized, so
    // start the main dialog box
    DialogBox(
              ghInst,
              MAKEINTRESOURCE(IDD_MAINDLG),
              NULL,
              MainDialogProc
             );


    ShutdownTapi();

    CoUninitialize();

    return 1;
}


//////////////////////////////////////////////////////////////
// InitializeTapi
//
// Various initializations
///////////////////////////////////////////////////////////////
HRESULT
InitializeTapi()
{
    HRESULT         hr;


    // cocreate the TAPI object
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

    // call initialize.  this must be called before
    // any other tapi functions are called.
    hr = gpTapi->Initialize();

    if (S_OK != hr)
    {
        DoMessage(L"TAPI failed to initialize");

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
    // if there is still a call,
    // release it
    if (NULL != gpCall)
    {
        gpCall->Release();
        gpCall = NULL;
    }

    // if we have an address object
    // release it
    if (NULL != gpAddress)
    {
        gpAddress->Release();
        gpAddress = NULL;
    }

    // release main object.
    if (NULL != gpTapi)
    {
        gpTapi->Shutdown();
        gpTapi->Release();
        gpTapi = NULL;
    }

}

///////////////////////////////////////////////////////////////////////////
// InitAddressTypeComboBox
//
// Put address type string in the combo box
// and save the addresstype with the string
//
///////////////////////////////////////////////////////////////////////////
void
InitAddressTypeComboBox(
    HWND hComboBox
    )
{
    int i;

    i = (int)SendMessage( hComboBox, CB_ADDSTRING, 0, (LONG_PTR)gszConferenceName );

    SendMessage(
                hComboBox,
                CB_SETITEMDATA ,
                i,
                (LONG_PTR)LINEADDRESSTYPE_SDP
               );


    i = (int)SendMessage( hComboBox, CB_ADDSTRING, 0, (LONG_PTR)gszEmailName );

    SendMessage(
                hComboBox,
                CB_SETITEMDATA ,
                i,
                (LONG_PTR)LINEADDRESSTYPE_EMAILNAME
               );


    i = (int)SendMessage( hComboBox, CB_ADDSTRING, 0, (LONG_PTR)gszMachineName );

    SendMessage(
                hComboBox,
                CB_SETITEMDATA ,
                i,
                (LONG_PTR)LINEADDRESSTYPE_DOMAINNAME
               );


    i = (int)SendMessage( hComboBox, CB_ADDSTRING, 0, (LONG_PTR)gszPhoneNumber );

    SendMessage(
                hComboBox,
                CB_SETITEMDATA ,
                i,
                (LONG_PTR)LINEADDRESSTYPE_PHONENUMBER
               );


    SendMessage( hComboBox, CB_SETCURSEL, i, 0 );

    i = (int)SendMessage( hComboBox, CB_ADDSTRING, 0, (LONG_PTR)gszIPAddress );

    SendMessage(
                hComboBox,
                CB_SETITEMDATA ,
                i,
                (LONG_PTR)LINEADDRESSTYPE_IPADDRESS
               );

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
        case WM_INITDIALOG:
        {
            HWND hComboBox;


            // set up dialog
            ghDlg = hDlg;

            EnableButton( hDlg, IDOK );
            DisableButton( hDlg, IDC_DISCONNECT );

            hComboBox = GetDlgItem( hDlg, IDC_ADDRESSTYPE );

            InitAddressTypeComboBox(hComboBox);

            SetFocus( hComboBox );

            return 0;
        }

        case WM_COMMAND:
        {
            if ( LOWORD(wParam) == IDCANCEL )
            {
                // quit
                EndDialog( hDlg, 0 );

                return 1;
            }

            // dial request
            if ( LOWORD(wParam) == IDOK )
            {
                HWND hComboBox;
                DWORD dwIndex;
                DWORD dwAddressType;
                WCHAR szAddressToCall[ADDRESSLENGTH];


                // get the address type the user selected.
                hComboBox = GetDlgItem( hDlg, IDC_ADDRESSTYPE );
                dwIndex = (DWORD)SendMessage( hComboBox, CB_GETCURSEL, 0, 0 );

                dwAddressType = (DWORD)SendMessage(
                                             hComboBox,
                                             CB_GETITEMDATA,
                                             dwIndex,
                                             0
                                           );

                // get the address the user wants to call
                GetDlgItemText(
                               hDlg,
                               IDC_ADDRESS,
                               szAddressToCall,
                               ADDRESSLENGTH
                              );

                // make the call
                if ( S_OK == MakeTheCall(dwAddressType, szAddressToCall) )
                {
                    EnableButton( hDlg, IDC_DISCONNECT );
                    DisableButton( hDlg, IDOK );
                }
                else
                {
                    DoMessage(L"The call failed to connect");
                }

                return 1;
            }

            // disconnect request
            if ( LOWORD( wParam ) == IDC_DISCONNECT )
            {
                // disconnect
                if (S_OK == DisconnectTheCall())
                {
                    EnableButton( hDlg, IDOK );
                    DisableButton( hDlg, IDC_DISCONNECT );
                }
                else
                {
                    DoMessage(L"The call failed to disconnect");
                }

                return 1;
            }

            return 0;
        }
        default:

            return 0;
    }
}


////////////////////////////////////////////////////////////////////////
// FindAnAddress
//
// Finds an address object that this application will use to make calls on.
//
// This function finds an address that supports the addresstype passed
// in, as well as the audioin and audioout media types.
//
// Return Value
//          S_OK if it finds an address
//          E_FAIL if it does not find an address
////////////////////////////////////////////////////////////////////////
HRESULT
FindAnAddress(
              DWORD dwAddressType,
              BSTR  * ppName
             )
{
    HRESULT                 hr = S_OK;
    BOOL                    bFoundAddress = FALSE;
    IEnumAddress          * pEnumAddress;
    ITAddress             * pAddress;
    ITAddressCapabilities * pAddressCaps;
    long                    lType = 0;

    // if we have an address object
    // release it
    if (NULL != gpAddress)
    {
        gpAddress->Release();
        gpAddress = NULL;
    }

    // enumerate the addresses
    hr = gpTapi->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        return hr;
    }

    while ( !bFoundAddress )
    {
        // get the next address
        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {
            break;
        }


        hr = pAddress->QueryInterface(IID_ITAddressCapabilities, (void**)&pAddressCaps);

        if ( SUCCEEDED(hr) )
        {

            hr = pAddressCaps->get_AddressCapability( AC_ADDRESSTYPES, &lType );

            pAddressCaps->Release();

            if ( SUCCEEDED(hr) )
            {
                // is the type we are looking for?
                if ( dwAddressType & lType )
                {
                    // does it support audio?
                    if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_AUDIO) )
                    {
                        // does it have a name?
                        if ( SUCCEEDED( pAddress->get_AddressName(ppName) ) )
                        {
                            // save it in the global variable
                            // since we break out of the loop, this one won't
                            // get released

                            gpAddress = pAddress;

                            bFoundAddress = TRUE;

                            break;
                        }
                    }
                }
            }
        }

        pAddress->Release();

    } // end while loop

    pEnumAddress->Release();

    if (!bFoundAddress)
    {
        return E_FAIL;
    }

    return S_OK;
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

/////////////////////////////////////////////////////////////////
// EnablePreview
//
// Selects a video render terminal on a video capture stream,
// thereby enabling video preview.
/////////////////////////////////////////////////////////////////

HRESULT
EnablePreview(
              ITStream * pStream
             )
{
    ITTerminal * pTerminal;

    HRESULT hr = GetVideoRenderTerminal(&pTerminal);

    if ( SUCCEEDED(hr) )
    {
        hr = pStream->SelectTerminal(pTerminal);

        pTerminal->Release();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////
// SelectTerminalsOnCall
//
// Creates and selects terminals for all streams on the given
// call.
/////////////////////////////////////////////////////////////////

HRESULT
SelectTerminalsOnCall(
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

                hr = GetTerminal(pStream,
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
                            EnablePreview( pStream );
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


/////////////////////////////////////////////////////////////////
// MakeTheCall
//
// Sets up and makes a call
/////////////////////////////////////////////////////////////////

HRESULT
MakeTheCall(
            DWORD dwAddressType,
            PWCHAR szAddressToCall
           )
{
    HRESULT                 hr = S_OK;
    BSTR                    bstrAddressToCall;
    BSTR                    pAddressName;


    // find an address object that
    // we will use to make calls on

    hr = FindAnAddress(dwAddressType, &pAddressName);

    if ( FAILED(hr) )
    {
        DoMessage(L"Could not find a TAPI address for making calls.");

        return hr;
    }

    WCHAR pText[200];

    swprintf_s(pText,200, L"Using address: %s", pAddressName);

    SysFreeString(pAddressName);

    DoMessage(pText);

    //
    // find out which media types this address supports
    //

    long lMediaTypes = 0;

    if ( AddressSupportsMediaType(gpAddress, TAPIMEDIATYPE_AUDIO) )
    {
        lMediaTypes |= TAPIMEDIATYPE_AUDIO; // we will use audio
    }

    if ( AddressSupportsMediaType(gpAddress, TAPIMEDIATYPE_VIDEO) )
    {
        lMediaTypes |= TAPIMEDIATYPE_VIDEO; // we will use video
    }

    //
    // Create the call.
    //

    bstrAddressToCall = SysAllocString( szAddressToCall );

    hr = gpAddress->CreateCall( bstrAddressToCall,
                                dwAddressType,
                                lMediaTypes,
                                &gpCall);

    SysFreeString ( bstrAddressToCall );

    if ( FAILED(hr) )
    {
        DoMessage(L"Could not create a call.");

        return hr;
    }

    //
    // Select our terminals on the call; if any of the selections fail we
    // proceed without that terminal.
    //

    hr = SelectTerminalsOnCall( gpCall );

    //
    // We're now ready to call connect.
    //
    // the VARIANT_TRUE parameter indicates that this
    // call is sychronous - that is, it won't
    // return until the call is in the connected
    // state (or fails to connect)
    // Since this is called in the UI thread,
    // this means that the app will appear
    // to hang until this function returns.
    // Some TAPI service providers may take a long
    // time for a call to reach the connected state.
    //

    hr = gpCall->Connect( VARIANT_TRUE );

    if ( FAILED(hr) )
    {
        gpCall->Release();
        gpCall = NULL;

        DoMessage(L"Could not connect the call.");

        return hr;
    }

    return S_OK;
}


/////////////////////////////////////////////////////////
// GetTerminal
//
// Creates the default terminal for the passed-in stream.
//
/////////////////////////////////////////////////////////
HRESULT
GetTerminal(
            ITStream * pStream,
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
        return GetVideoRenderTerminal(ppTerminal);
    }

    //
    // For all other terminals we use GetDefaultStaticTerminal.
    // First, get the terminal support interface.
    //

    ITTerminalSupport * pTerminalSupport;

    hr = gpAddress->QueryInterface( IID_ITTerminalSupport,
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

/////////////////////////////////////////////////////////
// GetVideoRenderTerminal
//
// Creates a dynamic terminal for the Video Render mediatype / direction
//
/////////////////////////////////////////////////////////
HRESULT
GetVideoRenderTerminal(
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

            hr = gpAddress->QueryInterface(IID_ITTerminalSupport,
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

                if ( SUCCEEDED(hr) )
                {
                    // Get the video window interface for the terminal
                    IVideoWindow *pVideoWindow = NULL;

                    hr = (*ppTerminal)->QueryInterface(IID_IVideoWindow,
                                                       (void**)&pVideoWindow);

                    if ( SUCCEEDED(hr) )
                    {
                        //
                        // Set the AutoShow member to true
                        //
                        // Note that the AutoShow property is the only one
                        // we can use on this terminal's IVideoWindow and
                        // IBasicVideo interfaces before the CME_STREAM_ACTIVE
                        // event is received for the stream. All other methods
                        // will fail until CME_STREAM_ACTIVE has been sent.
                        // Applications that need to control more about a video
                        // window than just its visibility must listen for the
                        // CME_STREAM_ACTIVE event. See the "t3in.exe" sample
                        // for how to do this.
                        //

                        hr = pVideoWindow->put_AutoShow( VARIANT_TRUE );

                        pVideoWindow->Release();
                    }
                }
            }

            SysFreeString( bstrTerminalClass );
        }
    }

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

        gpCall->Release();
        gpCall = NULL;

        return hr;
    }

    return S_FALSE;
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

///////////////////////////////////////////////////////////////
// EnableButton
//
// Enable, make default, and setfocus to a button
///////////////////////////////////////////////////////////////
void
EnableButton(
             HWND hDlg,
             int ID
            )
{
    SendDlgItemMessage(
                       hDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_DEFPUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( hDlg, ID ),
                 TRUE
                );
    SetFocus(
             GetDlgItem( hDlg, ID )
            );
}

//////////////////////////////////////////////////////////////
// DisableButton
//
// Disable a button
//////////////////////////////////////////////////////////////
void
DisableButton(
              HWND hDlg,
              int ID
             )
{
    SendDlgItemMessage(
                       hDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_PUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( hDlg, ID ),
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
        // does it support this media type?
        pMediaSupport->QueryMediaType(
                                      lMediaType,
                                      &bSupport
                                     );

        pMediaSupport->Release();
    }

    return (bSupport == VARIANT_TRUE);
}

/*++

Copyright (c) 1999 - 2000 Microsoft Corporation.  All Rights Reserved.

Module Name:
    callnot.cpp

Abstract:

    Implementation of the ITTAPIEventNotification interface.

    This is an outgoing interface that is defined by TAPI 3.0.  This
    is basically a callback function that TAPI 3.0 calls to inform
    the application of events related to calls (on a specific address)

    Please refer to COM documentation for information on outgoing
    interfaces.

    An application must implement and register this interface in order
    to receive calls and events related to calls

--*/


#include <windows.h>
#include <tapi3.h>
#include <control.h>
#include <strmif.h>

#include "callnot.h"
#include "resource.h"

extern ITBasicCallControl * gpCall;
extern HWND ghDlg;
extern BSTR gbstrVideo;
extern BOOL gfAutoAnswer;

void
DoMessage(
          LPWSTR pszMessage
         );

void
SetStatusMessage(
                 LPWSTR pszMessage
                );

void
EnableButton(
             int ID
            );
void
DisableButton(
              int ID
             );

enum {
    VWP_LEFT = 20,
    VWP_TOP = 100
};

///////////////////////////////////////////////////////////////////
// HostWindow
//
// This method takes a pVideoWindow interface pointer and puts
//  the window in our dialog.
//
// The location depends on the iWhere parameter.
//
///////////////////////////////////////////////////////////////////

void
HostWindow(
           IVideoWindow *pVideoWindow,
           int iWhere
          )
{
    
    pVideoWindow->put_Owner( (LONG_PTR)ghDlg );
    pVideoWindow->put_WindowStyle( WS_CHILDWINDOW | WS_BORDER );

    long lWidth, lHeight;
    pVideoWindow->get_Width( &lWidth );
    pVideoWindow->get_Height( &lHeight );

    pVideoWindow->SetWindowPosition( VWP_LEFT + (iWhere * (VWP_LEFT + lWidth)),
                                     VWP_TOP,
                                     lWidth,
                                     lHeight );

    pVideoWindow->put_Visible( VARIANT_TRUE );
}

///////////////////////////////////////////////////////////////////
// GetVideoRenderTerminalFromStreamEvent
//
// Find out if this stream has a video render terminal. If not,
// we don't need to do anything with this stream. Also note
// if this is the video capture stream or the video render
// stream.
//
///////////////////////////////////////////////////////////////////

HRESULT
GetVideoRenderTerminalFromStreamEvent(
                                      ITCallMediaEvent  * pCallMediaEvent,
                                      ITTerminal       ** ppTerminal,
                                      BOOL              * pfRenderStream
                                     )
{
    HRESULT hr;

    //
    // Get the stream for this event.
    //

    ITStream * pStream;

    hr = pCallMediaEvent->get_Stream( &pStream );
    
    if ( FAILED(hr) || (pStream == NULL) )
    {
        return E_FAIL;
    }

    //
    // Find out if this is a video stream. If not, fail --
    // we don't care about this stream.
    //

    long lMediaType;

    hr = pStream->get_MediaType(&lMediaType);
    
    if ( FAILED(hr) )
    {
        pStream->Release();

        return hr;
    }

    if ( lMediaType != TAPIMEDIATYPE_VIDEO )
    {
        pStream->Release();

        return E_FAIL;
    }

    //
    // Find out the direction of this stream. We will return this info to the
    // caller.
    //

    TERMINAL_DIRECTION td;

    hr = pStream->get_Direction(&td);
    
    if ( FAILED(hr) )
    {
        pStream->Release();

        return hr;
    }

    *pfRenderStream = (td == TD_RENDER);

    //
    // Enumerate the terminals on this stream.
    //

    IEnumTerminal * pEnumTerminal;

    hr = pStream->EnumerateTerminals( &pEnumTerminal );
    
    pStream->Release();
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    //
    // Search for the first video render terminal and return it. If there are
    // none, fail -- we don't care about this stream.
    //

    while ( S_OK == pEnumTerminal->Next(1, ppTerminal, NULL) )
    {
        if ( SUCCEEDED( (*ppTerminal)->get_Direction(&td) ) )
        {
            if ( td == TD_RENDER )
            {
                // found it; done with enum, but keep ref on terminal
                pEnumTerminal->Release();

                return S_OK;
            }
        }

        // this is not the right one
        (*ppTerminal)->Release();
    }

    pEnumTerminal->Release();

    return E_FAIL; // no terminal found
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
                              TAPI_EVENT TapiEvent,
                              IDispatch * pEvent
                             )
{
    //
    // Addref the event so it doesn't go away.
    //

    pEvent->AddRef();

    //
    // Post a message to our own UI thread.
    //

    PostMessage(
                ghDlg,
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
            TAPI_EVENT TapiEvent,
            IDispatch * pEvent
           )
{
    HRESULT hr;

    switch ( TapiEvent )
    {
        case TE_CALLNOTIFICATION:
        {
            //
            // TE_CALLNOTIFICATION means that the application is being notified
            // of a new call.
            //
            // Note that we don't answer to call at this point.  The application
            // should wait for a CS_OFFERING CallState message before answering
            // the call.
            //

            ITCallNotificationEvent         * pNotify;
            
            hr = pEvent->QueryInterface( IID_ITCallNotificationEvent, (void **)&pNotify );

            if (S_OK != hr)
            {
                DoMessage( L"Incoming call, but failed to get the interface");
            }
            else
            {
                CALL_PRIVILEGE          cp;
                ITCallInfo *            pCall;

                //
                // get the call
                //

                hr = pNotify->get_Call( &pCall );

                pNotify->Release();

                if ( SUCCEEDED(hr) )
                {
                    //
                    // check to see if we own the call
                    //

                    hr = pCall->get_Privilege( &cp );

                    if ( FAILED(hr) || (CP_OWNER != cp) )
                    {
                        //
                        // just ignore it if we don't own it
                        //

                        pCall->Release();

                        pEvent->Release(); // we addrefed it CTAPIEventNotification::Event()

                        return S_OK;
                    }

                    //
                    // Get the ITBasicCallControl interface 
                    //

                    //
                    // If we're already in a call, disconnect the new call.  Otherwise,
                    // save it in our global variable.
                    //

                    ITBasicCallControl * pCallControl;

                    hr = pCall->QueryInterface( IID_ITBasicCallControl,
                                                (void**)&pCallControl );

                    pCall->Release();

                  
                    if ( SUCCEEDED(hr) )
                    {
                        if (gpCall == NULL)
                        {
                            gpCall = pCallControl;
                
                            //
                            // update UI
                            //
                
                            EnableButton( IDC_ANSWER );
                            DisableButton( IDC_DISCONNECT );
                            SetStatusMessage(L"Incoming Owner Call");
                        }
                        else
                        {
                            //
                            // Reject this call since we're already in a call
                            //

                            hr = pCallControl->Disconnect(DC_REJECTED);

                            pCallControl->Release();

                            if (FAILED(hr))
                            {
                                break;
                            }
                        }
                    }

                }
            }
            
            break;
        }
        
        case TE_CALLSTATE:
        {
            // TE_CALLSTATE is a call state event.  pEvent is
            // an ITCallStateEvent

            CALL_STATE           cs;
            ITCallStateEvent   * pCallStateEvent;
            ITCallInfo *         pCall;
            ITBasicCallControl * pCallControl;

            // Get the interface
            hr = pEvent->QueryInterface( IID_ITCallStateEvent, (void **)&pCallStateEvent );

            if ( FAILED(hr) )
            {
                break;
            }

            // get the CallInfo interface
            hr = pCallStateEvent->get_Call( &pCall );

            if ( FAILED(hr) )
            {
                pCallStateEvent->Release();
                break;
            }

            //get the ITBasicCallControl interface and compare it to our existing call
            hr = pCall->QueryInterface( IID_ITBasicCallControl,(void**)&pCallControl );

            pCall->Release();

            if (FAILED(hr))
            {
                pCallStateEvent->Release();
                break;
            }

            //ignore call state events for other calls
            if (pCallControl != gpCall)
            {
                pCallControl->Release();
                pCallStateEvent->Release();
                break;
            }
            pCallControl->Release();

            //
            // This is a call state event for our call
            //

            // get the CallState that we are being notified of.
            hr = pCallStateEvent->get_State( &cs );

            // Release the interface
            pCallStateEvent->Release();

            if ( FAILED(hr) )
            {
                break;
            }

            // if it's offering, update our UI
            if (CS_OFFERING == cs)
            {
                if (gfAutoAnswer)
                {
                    PostMessage(ghDlg, WM_COMMAND, IDC_ANSWER, 0); 
                }
                else
                {
                    SetStatusMessage(L"Click the Answer button");
                }
            }
            else if (CS_DISCONNECTED == cs)
            {
                PostMessage(ghDlg, WM_COMMAND, IDC_DISCONNECTED, 0);
            }
            else if (CS_CONNECTED == cs)
            {
                // nothing to do -- we handle connection synchronously
            }
                
            break;
        }

        case TE_CALLMEDIA:
        {
            // TE_CALLMEDIA is a media event.  pEvent is
            // an ITCallMediaEvent

            CALL_MEDIA_EVENT    cme;
            ITCallMediaEvent  * pCallMediaEvent;

            // Get the interface
            hr = pEvent->QueryInterface( IID_ITCallMediaEvent, (void **)&pCallMediaEvent );

            if ( FAILED(hr) )
            {
                break;
            }

            // get the CALL_MEDIA_EVENT that we are being notified of.
            hr = pCallMediaEvent->get_Event( &cme );

            if ( SUCCEEDED(hr) )
            {
                switch ( cme )
                {
                case CME_STREAM_NOT_USED:
                case CME_STREAM_INACTIVE:
                case CME_NEW_STREAM:
                    break;

                case CME_STREAM_FAIL:
                    DoMessage( L"Call media event: stream failed");
                    break;
    
                case CME_TERMINAL_FAIL:
                    DoMessage( L"Call media event: terminal failed");
                    break;

                case CME_STREAM_ACTIVE:
                {
                    //
                    // Find out if this stream has a video render terminal. If not,
                    // we don't need to do anything with this stream. Also note
                    // if this is the video capture stream or the video render
                    // stream.
                    //

                    ITTerminal * pTerminal;
                    BOOL         fRenderStream;

                    hr = GetVideoRenderTerminalFromStreamEvent(
                                                               pCallMediaEvent,
                                                               &pTerminal,
                                                               &fRenderStream
                                                              );
                
                    if ( SUCCEEDED(hr) )
                    {
                        // Get the video window interface for the terminal
                        IVideoWindow *pVideoWindow = NULL;

                        hr = pTerminal->QueryInterface( IID_IVideoWindow, (void**)&pVideoWindow );
                
                        pTerminal->Release();
            
                        if ( SUCCEEDED(hr) )
                        {
                            // Put this window in our dialog
                            HostWindow(pVideoWindow, fRenderStream);
            
                            pVideoWindow->Release();
                        }
                    }

                    break;
                }
            
                default:
                    break;
                }
            }

            // We no longer need this interface.
            pCallMediaEvent->Release();

            break;
        }

        default:
            break;
    }

    pEvent->Release(); // we addrefed it CTAPIEventNotification::Event()
    
    return S_OK;
}


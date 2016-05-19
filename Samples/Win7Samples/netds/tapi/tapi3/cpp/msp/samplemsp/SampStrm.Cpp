/*++

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    sampstrm.cpp 

Abstract:

    This module contains the implementation for a sample MSP stream class.

--*/

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSPStream::CSampleMSPStream() : CMSPStream()
{
    LOG((MSP_TRACE, "CSampleMSPStream::CSampleMSPStream entered."));

    m_fTransportConfigured = FALSE;
    m_fTerminalConnected   = FALSE;
    m_DesiredGraphState    = State_Stopped;
    m_ActualGraphState     = State_Stopped;

    //
    // INSERT HERE: initialize other data members
    //

    LOG((MSP_TRACE, "CSampleMSPStream::CSampleMSPStream exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSPStream::~CSampleMSPStream()
{
    LOG((MSP_TRACE, "CSampleMSPStream::~CSampleMSPStream entered."));
    LOG((MSP_TRACE, "CSampleMSPStream::~CSampleMSPStream exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

void CSampleMSPStream::FinalRelease()
{
    LOG((MSP_TRACE, "CSampleMSPStream::FinalRelease entered."));

    //
    // At this point we should have no terminals selected, since
    // Shutdown is supposed to be called before we are destructed.
    //

    _ASSERTE( 0 == m_Terminals.GetSize() );

    if ( m_fTransportConfigured )
    {
        //
        // INSERT HERE: Remove our transport filters from the graph and
        // release them.
        //
    }

    //
    // Call the base class method to clean up everything else.
    //

    CMSPStream::FinalRelease();

    LOG((MSP_TRACE, "CSampleMSPStream::FinalRelease exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSPStream::get_Name (
    OUT     BSTR *                  ppName
    )
{
    LOG((MSP_TRACE, "CSampleMSPStream::get_Name - enter"));

    //
    // Check argument.
    //

    if (!ppName)
    {
        LOG((MSP_TRACE, "CSampleMSPStream::get_Name - "
            "bad return pointer - returning E_POINTER"));

        return E_POINTER;
    }

    //
    // Decide what string to return based on which stream this is.
    //

    ULONG ulID;
    
    if ( m_Direction == TD_CAPTURE )
    {
        ulID = IDS_CAPTURE_STREAM;
    }
    else
    {
        ulID = IDS_RENDER_STREAM;
    }

    //
    // Get the string from the string table.
    //

    const int   ciAllocSize = 2048;
    WCHAR       wszName[ciAllocSize];

    int iReturn = LoadString( _Module.GetModuleInstance(),
                              ulID,
                              wszName,
                              ciAllocSize - 1 );

    if ( iReturn == 0 )
    {
        _ASSERTE( FALSE );
        
        *ppName = NULL;

        LOG((MSP_ERROR, "CSampleMSPStream::get_Name - "
            "LoadString failed - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // Convert to a BSTR and return the BSTR.
    //

    *ppName = SysAllocString(wszName);

    if ( *ppName == NULL )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::get_Name - "
            "SysAllocString failed - returning E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CSampleMSPStream::get_Name - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// This implementation, typical of the simplest MSPs, limits each stream to
// having at most one terminal selected at a time.
//

STDMETHODIMP CSampleMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
{
    LOG((MSP_TRACE, "CSampleMSPStream::SelectTerminal - enter"));

    //
    // We are going to access the terminal list -- grab the lock
    //

    CLock lock(m_lock);

    //
    // Reject if we already have a terminal selected.
    //

    if ( 0 != m_Terminals.GetSize() )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::SelectTerminal - "
            "exit TAPI_E_MAXTERMINALS"));

        return TAPI_E_MAXTERMINALS;
    }

    //
    // Use base class method to add it to our list of terminals.
    //

    HRESULT hr = CMSPStream::SelectTerminal(pTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::SelectTerminal - "
            "base class method failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Re-pause or re-start the stream if needed.
    //

    if ( m_DesiredGraphState == State_Paused )
    {
        hr = PauseStream();
    }
    else if ( m_DesiredGraphState == State_Running )
    {
        hr = StartStream();
    }
    else
    {
        _ASSERTE( m_DesiredGraphState == State_Stopped );

        hr = S_OK;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CSampleMSPStream::SelectTerminal - "
            "can't regain old graph state - unselecting terminal - "
            "exit 0x%08x", hr));

		//
		// Unselect it to undo all of the above.
		//

	    UnselectTerminal(pTerminal);

        return hr;
    }

    LOG((MSP_TRACE, "CSampleMSPStream::SelectTerminal - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSPStream::UnselectTerminal (
        IN     ITTerminal *             pTerminal
        )
{
    LOG((MSP_TRACE, "CSampleMSPStream::UnselectTerminal - enter"));

    CLock lock(m_lock);

    //
    // Use base class method to remove terminal from our list of terminals.
    //
    // Note: the failure cases are messy here. In this sample implementation
    // we choose to return an error code but release our reference to the
    // terminal when disconnection fails. This is typically better than
    // keeping our reference to the terminal, which would cause us to leak
    // the terminal whenever disconnection fails.
    //

    HRESULT hr = CMSPStream::UnselectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSampleMSPStream::UnselectTerminal - "
            "base class method failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Stop the graph and disconnect the terminal if this call had it
    // connected. (We need a stopped graph to disconnect properly, and
    // couldn't have started the graph if the terminal isn't connected.)
    //

    if ( m_fTerminalConnected )
    {
        //
        // At this point we need to make sure the stream is stopped.
        // We can't use our own StopStream method because it
        // (1) changes the desired graph state to Stopped and 
        // (2) does nothing if no terminal has been selected (which it now
        //     thinks is the case)
        //

        _ASSERTE( m_fTransportConfigured );

        //
        // Stop the stream via the base class method.
        //

        hr = CMSPStream::StopStream();

        m_ActualGraphState = State_Stopped;

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CSampleMSPStream::UnselectTerminal - "
                "Stop failed - 0x%08x", hr));

            // don't return hr -- we really want to continue and
            // disconnect if we can!
        }

        //
        // Get the ITTerminalControl interface.
        //

        ITTerminalControl * pTerminalControl;

        hr = pTerminal->QueryInterface(IID_ITTerminalControl,
                                       (void **) &pTerminalControl);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CSampleMSPStream::UnselectTerminal - "
                "QI for ITTerminalControl failed - exit 0x%08x", hr));

            return hr;
        }

        //
        // Disconnect the terminal.
        //

        hr = pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

        pTerminalControl->Release();

        m_fTerminalConnected = FALSE;

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CSampleMSPStream::UnselectTerminal - "
                "DisconnectTerminal failed - exit 0x%08x", hr));

            return hr;
        }

        //
        // INSERT HERE: Additional disconnection may be needed, for example
        // for transport fitlers or transform filters that need to be
        // used for some terminals but not for others.
        //
    }

    LOG((MSP_TRACE, "CSampleMSPStream::UnselectTerminal - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSPStream::StartStream (void)
{
    LOG((MSP_TRACE, "CSampleMSPStream::StartStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Running;

    //
    // Can't start the stream if our transport filters are not configured.
    //

    if ( ! m_fTransportConfigured )
    {
        LOG((MSP_WARN, "CSampleMSPStream::PauseStream - "
            "transport not configured so nothing to do yet - exit S_OK"));

        return S_OK;
    }

    //
    // Can't start the stream if no terminal has been selected.
    //

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CSampleMSPStream::StartStream - "
            "no Terminal so nothing to do yet - exit S_OK"));

        return S_OK;
    }

    //
    // Connect the terminal. This does nothing if this call already
    // connected the terminal and fails if another call has the
    // terminal connected.
    //

    HRESULT hr;

    hr = ConnectTerminal(m_Terminals[0]);

    if ( FAILED(hr) )
    {
        FireEvent(CALL_TERMINAL_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);

        LOG((MSP_ERROR, "CSampleMSPStream::StartStream - "
            "our ConnectTerminal failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Run the stream via the base class method.
    //

    hr = CMSPStream::StartStream();

    if ( FAILED(hr) )
    {
        //
        // Failed to run -- tell the app.
        //

        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, "CSampleMSPStream::StartStream - "
            "Run failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Fire event if this just made us active.
    //

    if ( m_ActualGraphState != State_Running )
    {
        m_ActualGraphState = State_Running;

        HRESULT hr2 = FireEvent(CALL_STREAM_ACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            LOG((MSP_ERROR, "CSampleMSPStream::StartStream - "
                "FireEvent failed - exit 0x%08x", hr2));

            return hr2;
        }
    }

    LOG((MSP_TRACE, "CSampleMSPStream::StartStream - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSPStream::PauseStream (void)
{
    LOG((MSP_TRACE, "CSampleMSPStream::PauseStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Paused;

    //
    // Can't pause the stream if our transport filters are not configured.
    //

    if ( ! m_fTransportConfigured )
    {
        LOG((MSP_WARN, "CSampleMSPStream::PauseStream - "
            "transport not configured so nothing to do yet - exit S_OK"));

        return S_OK;
    }

    //
    // Can't pause the stream if no terminal has been selected.
    //

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CSampleMSPStream::PauseStream - "
            "no Terminal so nothing to do yet - exit S_OK"));

        return S_OK;
    }

    //
    // Connect the terminal. This does nothing if this call already
    // connected the terminal and fails if another call has the
    // terminal connected.
    //

    HRESULT hr;

    hr = ConnectTerminal(m_Terminals[0]);

    if ( FAILED(hr) )
    {
        FireEvent(CALL_TERMINAL_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);

        LOG((MSP_ERROR, "CSampleMSPStream::StartStream - "
            "our ConnectTerminal failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Pause the stream via the base class method.
    //

    hr = CMSPStream::PauseStream();

    if ( FAILED(hr) )
    {
        //
        // Failed to pause -- tell the app.
        //

        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, "CSampleMSPStream::PauseStream - "
            "Pause failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Fire event if this just made us inactive.
    //

    if ( m_ActualGraphState == State_Running )
    {
        HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            m_ActualGraphState = State_Paused;

            LOG((MSP_ERROR, "CSampleMSPStream::PauseStream - "
                "FireEvent failed - exit 0x%08x", hr2));

            return hr2;
        }
    }

    m_ActualGraphState = State_Paused;

    LOG((MSP_TRACE, "CSampleMSPStream::PauseStream - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSPStream::StopStream (void)
{
    LOG((MSP_TRACE, "CSampleMSPStream::StopStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Stopped;

    //
    // Nothing to do if our transport is not configured.
    //

    if ( ! m_fTransportConfigured )
    {
        LOG((MSP_WARN, "CSampleMSPStream::StopStream - "
            "transport not configured - exit S_OK"));

        return S_OK;
    }

    //
    // Nothing to do if no terminal has been selected.
    //

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CSampleMSPStream::StopStream - "
            "no Terminal - exit S_OK"));

        return S_OK;
    }

    //
    // Stop the stream via the base class method.
    //

    HRESULT hr;

    hr = CMSPStream::StopStream();

    if ( FAILED(hr) )
    {
        //
        // Failed to stop -- tell the app.
        //

        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        m_DesiredGraphState = m_ActualGraphState;

        LOG((MSP_ERROR, "CSampleMSPStream::StopStream - "
            "Stop failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Fire event if this just made us inactive.
    //

    if ( m_ActualGraphState == State_Running )
    {
        HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            m_ActualGraphState = State_Stopped;

            LOG((MSP_ERROR, "CSampleMSPStream::StopStream - "
                "FireEvent failed - exit 0x%08x", hr2));

            return hr2;
        }
    }

    m_ActualGraphState = State_Stopped;

    LOG((MSP_TRACE, "CSampleMSPStream::StopStream - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// A real MSP would have some arguments to this method, which would then be
// used to configure transport filters. The types of information passed here
// vary widely. For example, some MSPs may need to configure their transport
// filters with a GUID that indicates which device to use for streaming. This
// info would be passed to the MSP from the TSP via TAPI 3.0 (see sampcall.cpp).
//

HRESULT CSampleMSPStream::ConfigureTransport(void)
{
    LOG((MSP_TRACE, "CSampleMSPStream::ConfigureTransport - enter"));

    CLock lock(m_lock);

    //
    // INSERT HERE:
    // * create the transport filter(s)
    // * configure the transport filter(s) based on passe-in info
    // * add filters to our graph
    // * if some transport filters always need to be connected together
    //   (irrespective of what terminals are used, etc.) then they can be
    //   connected now).
    //

    m_fTransportConfigured = TRUE;

    LOG((MSP_TRACE, "CSampleMSPStream::ConfigureTransport - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Add the terminal to the graph and connect it to our
// filters, if it is not already in use.
//

HRESULT CSampleMSPStream::ConnectTerminal(ITTerminal * pTerminal)
{
    LOG((MSP_TRACE, "CSampleMSPStream::ConnectTerminal - enter"));

    //
    // Find out the terminal's internal state.
    //

    TERMINAL_STATE state;
    HRESULT hr;

    hr = pTerminal->get_State( &state );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "get_State on terminal failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // If we've already connected the terminal on this stream, then
    // there is nothing for us to do. Just assert that the terminal
    // also thinks it's connected.
    //

    if ( m_fTerminalConnected )
    {
        _ASSERTE( state == TS_INUSE );

        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "terminal already connected on this stream - exit S_OK"));

        return S_OK;
    }

    //
    // Otherwise we need to connect the terminal on this call. If the
    // terminal is already connected on another call, we must fail. Note
    // that since we are making several calls on the terminal here, the
    // terminal could become connected on another call while we are
    // in the process of doing this. If this happens, the we will just fail
    // later.
    //

    if ( state == TS_INUSE )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "terminal in use - exit TAPI_E_TERMINALINUSE"));

        return TAPI_E_TERMINALINUSE;
    }

    //
    // Get the ITTerminalControl interface.
    //

    ITTerminalControl * pTerminalControl;

    hr = m_Terminals[0]->QueryInterface(IID_ITTerminalControl,
                                        (void **) &pTerminalControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "QI for ITTerminalControl failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Find out how many pins the terminal has. If not one then bail as
    // we have no idea what to do with multiple-pin terminals (we don't
    // expose any such terminals from this MSP).
    //

    DWORD dwNumPinsAvailable;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           0,
                                           &dwNumPinsAvailable,
                                           NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "query for number of terminal pins failed - exit 0x%08x", hr));
        
        pTerminalControl->Release();

        return hr;
    }

    if ( 1 != dwNumPinsAvailable )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "unsupported number of terminal pins - exit E_FAIL"));

        pTerminalControl->Release();

        return E_FAIL;
    }

    //
    // Actually connect the terminal.
    //

    IPin * pTerminalPin;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           0,
                                           &dwNumPinsAvailable,
                                           &pTerminalPin);
    
    if ( FAILED(hr) )
    {
        pTerminalControl->Release();

        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "ConnectTerminal on terminal failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Make the connection between our filters and the terminal's pin.
    //

    hr = ConnectToTerminalPin(pTerminalPin);
    
    pTerminalPin->Release();

    if ( FAILED(hr) )
    {
        pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

        pTerminalControl->Release();

        LOG((MSP_ERROR, "CSampleMSPStream::ConnectTerminal - "
            "ConnectToTerminalPin failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Now we are actually connected. Update our state and perform postconnection
    // (ignore postconnection error code).
    //

    m_fTerminalConnected  = TRUE;

    pTerminalControl->CompleteConnectTerminal();

    pTerminalControl->Release();

    LOG((MSP_TRACE, "CSampleMSPStream::ConnectTerminal - exit S_OK"));

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//

HRESULT CSampleMSPStream::ConnectToTerminalPin(IPin * pTerminalPin)
{
    LOG((MSP_TRACE, "CSampleMSPStream::ConnectToTerminalPin - enter"));

    //
    // INSERT HERE: Code to connect the terminal's pin to some pin on some
    // transport filter belonging to the stream. See the documentation on
    // DirectShow in the Windows SDK for help.
    //

    LOG((MSP_TRACE, "CSampleMSPStream::ConnectToTerminalPin - exit S_OK"));

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// ProcessGraphEvent
//
// Sends an event to the app when we get an event from the filter graph.
// Note: The DirectShow graph events that the MSP cares about will vary
// from MSP to MSP. Be sure to modify this code to correctly propagate the
// events you need, depending on what filters you are using.
//

HRESULT CSampleMSPStream::ProcessGraphEvent(
    IN  long lEventCode,
    IN  long lParam1,
    IN  long lParam2
    )
{
    LOG((MSP_EVENT, "CSampleMSPStream::ProcessGraphEvent - enter"));

    HRESULT        hr = S_OK;

    switch (lEventCode)
    {
    case EC_COMPLETE:
        
        hr = FireEvent(CALL_STREAM_INACTIVE, (HRESULT) lParam1, CALL_CAUSE_UNKNOWN);
        break;
    
    case EC_USERABORT:
        
        hr = FireEvent(CALL_STREAM_INACTIVE, S_OK, CALL_CAUSE_UNKNOWN);
        break;

    case EC_ERRORABORT:
    case EC_STREAM_ERROR_STOPPED:
    case EC_STREAM_ERROR_STILLPLAYING:
    case EC_ERROR_STILLPLAYING:

        hr = FireEvent(CALL_STREAM_FAIL, (HRESULT) lParam1, CALL_CAUSE_UNKNOWN);
        break;

    default:
        
        LOG((MSP_EVENT, "CSampleMSPStream::ProcessGraphEvent - "
            "ignoring event code %d", lEventCode));
        break;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPStream::ProcessGraphEvent - "
            "FireEvent failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_EVENT, "CSampleMSPStream::ProcessGraphEvent - exit S_OK"));

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// FireEvent
//
// Sends an event to the application.
//

HRESULT CSampleMSPStream::FireEvent(
    IN MSP_CALL_EVENT        type,
    IN HRESULT               hrError,
    IN MSP_CALL_EVENT_CAUSE  cause
    )                                          
{
    LOG((MSP_EVENT, "CSampleMSPStream::FireEvent - enter"));

    //
    // Create the event structure. 
    //

    //
    // Note: 
    //
    // MSPEVENTITEMs are allocated with AllocateEventItem and freed with 
    // FreeEventItem
    //
    // AllocateEventItem takes an optional argument that specifies how many 
    // extra bytes need to be allocated in addition to the size of MSPEVENTITEM
    // 

    MSPEVENTITEM * pEventItem = AllocateEventItem();

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, "CSampleMSPStream::FireEvent - "
            "can't create MSPEVENTITEM structure - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the necessary fields for the event structure.
    //

    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    pEventItem->MSPEventInfo.Event  = ME_CALL_EVENT;

    ITTerminal * pTerminal = NULL;

    if ( 0 != m_Terminals.GetSize() )
    {
        _ASSERTE( 1 == m_Terminals.GetSize() );
        pTerminal = m_Terminals[0];
        pTerminal->AddRef();
    }

    ITStream * pStream = (ITStream *) this;
    pStream->AddRef();

    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Type      = type;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Cause     = cause;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream   = pStream;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal = pTerminal;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.hrError   = hrError;


#ifdef MSPLOG
    //
    // Spew some debug output to indicate what this is.
    //

    char * pszType;
    DWORD dwLevel;

    switch (type)
    {
    case CALL_NEW_STREAM:
        pszType = "CALL_NEW_STREAM (unexpected)";
        dwLevel = MSP_ERROR;
        break;

    case CALL_STREAM_FAIL:
        pszType = "CALL_STREAM_FAIL";
        dwLevel = MSP_INFO;
        break;

    case CALL_TERMINAL_FAIL:
        pszType = "CALL_TERMINAL_FAIL";
        dwLevel = MSP_INFO;
        break;

    case CALL_STREAM_NOT_USED:
        pszType = "CALL_STREAM_NOT_USED (unexpected)";
        dwLevel = MSP_ERROR;
        break;

    case CALL_STREAM_ACTIVE:
        pszType = "CALL_STREAM_ACTIVE";
        dwLevel = MSP_INFO;
        break;

    case CALL_STREAM_INACTIVE:
        pszType = "CALL_STREAM_INACTIVE";
        dwLevel = MSP_INFO;
        break;

    default:
        pszType = "UNKNOWN EVENT TYPE";
        dwLevel = MSP_ERROR;
        break;
    }

    LOG((dwLevel, "CSampleMSPStream::FireEvent - "
                  "EVENT DUMP: type      = %s", pszType));
    LOG((dwLevel, "CSampleMSPStream::FireEvent - "
                  "EVENT DUMP: pStream   = %p", pStream));    
    LOG((dwLevel, "CSampleMSPStream::FireEvent - "
                  "EVENT DUMP: pTerminal = %p", pTerminal));    
    LOG((dwLevel, "CSampleMSPStream::FireEvent - "
                  "EVENT DUMP: hrError   = %08x", hrError));    

#endif // ifdef MSPLOG

    //
    // Send the event to the app.
    //

    HRESULT hr = m_pMSPCall->HandleStreamEvent(pEventItem);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSampleMSPStream::FireEvent - "
            "HandleStreamEvent failed - returning 0x%08x", hr));

        pStream->Release();
        pTerminal->Release();

        
        //
        // Note:
        //
        // MSPEVENTITEMs are allocated with AllocateEventItem and freed with 
        // FreeEventItem
        //

        FreeEventItem(pEventItem);
        pEventItem = NULL;

        return hr;
    }

    LOG((MSP_EVENT, "CSampleMSPStream::FireEvent - exit S_OK"));

    return S_OK;
}


// eof

/*++

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    sampcall.cpp 

Abstract:

    This module contains implementation of CSampleMSPCall.

--*/

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSPCall::CSampleMSPCall() : CMSPCallMultiGraph()
{
    LOG((MSP_TRACE, "CSampleMSPCall::CSampleMSPCall entered."));
    LOG((MSP_TRACE, "CSampleMSPCall::CSampleMSPCall exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSPCall::~CSampleMSPCall()
{
    LOG((MSP_TRACE, "CSampleMSPCall::~CSampleMSPCall entered."));
    LOG((MSP_TRACE, "CSampleMSPCall::~CSampleMSPCall exited."));
}

ULONG CSampleMSPCall::MSPCallAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CSampleMSPCall::MSPCallRelease(void)
{
    return MSPReleaseHelper(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// We override this to make sure the number of streams we have is constant --
// we create them here. Simple MSPs often do this to make life easier.
//

HRESULT CSampleMSPCall::Init(
    IN      CMSPAddress *       pMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType
    )
{
    // No need to acquire locks on this call because it is called only
    // once when the object is created. No other calls can be made on
    // this object at this point.

    LOG((MSP_TRACE, "CSampleMSPCall::Init - enter"));
    
    //
    // First do the base class method. We are adding to the functionality,
    // not replacing it.
    //

    HRESULT hr;

    hr = CMSPCallMultiGraph::Init(pMSPAddress,
                                   htCall,
                                   dwReserved,
                                   dwMediaType);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSampleMSPCall::Init - "
            "base class method failed: %x", hr));

        return hr;
    }

    //
    // Our calls always come with two streams. Create them now. Use the base class
    // methods, as our overriden methods (exposed to the user) purposely fail in order
    // to keep the user from creating or removing streams themselves.
    // These methods return a pointer to the ITStream. They get saved in our list of
    // ITStreams, and we also save them here as CSampleMSPStream pointers.
    //

    ITStream * pStream;

    //
    // Create the capture stream.
    //

    hr = InternalCreateStream (dwMediaType,
                               TD_CAPTURE,
                               &pStream);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSampleMSPCall::Init - "
            "couldn't create capture stream: %x", hr));

        return hr;
    }

    m_pCaptureStream = dynamic_cast<CSampleMSPStream *> (pStream);

    if ( m_pCaptureStream == NULL )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::Init - "
            "couldn't dynamic_cast capture stream - exit E_FAIL"));

        return E_FAIL;
    }

    pStream->Release();
 
    //
    // Create the render stream.
    //

    hr = InternalCreateStream (dwMediaType,
                               TD_RENDER,
                               &pStream);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSampleMSPCall::Init - "
            "couldn't create capture stream: %x", hr));

        return hr;
    }

    m_pRenderStream = dynamic_cast<CSampleMSPStream *> (pStream);

    if ( m_pRenderStream == NULL )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::Init - "
            "couldn't dynamic_cast render stream - exit E_FAIL"));

        return E_FAIL;
    }

    pStream->Release();

    LOG((MSP_TRACE, "CSampleMSPCall::Init - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// We override this to make sure the number of streams we have is constant --
// we disallow any creation of streams after the call is created. Simple MSPs
// often do this to make life easier.
//

STDMETHODIMP CSampleMSPCall::CreateStream (
    IN      DWORD               dwMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
{
    LOG((MSP_TRACE, "CSampleMSPCall::CreateStream entered."));
    LOG((MSP_TRACE, "CSampleMSPCall::CreateStream - "
        "we have a fixed set of streams - exit TAPI_E_MAXSTREAMS"));

    return TAPI_E_MAXSTREAMS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// We override this to make sure the number of streams we have is constant --
// we disallow any removal of streams after the call is created. Simple MSPs
// often do this to make life easier.
//

STDMETHODIMP CSampleMSPCall::RemoveStream (
    IN      ITStream *          pStream
    )
{
    LOG((MSP_TRACE, "CSampleMSPCall::RemoveStream entered."));
    LOG((MSP_TRACE, "CSampleMSPCall::RemoveStream - "
        "we have a fixed set of streams - exit TAPI_E_NOTSUPPORTED"));

    return TAPI_E_NOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// This is our override to create the right kind of stream on stream creation.
// The base class checks the arguments for us.
//

HRESULT CSampleMSPCall::CreateStreamObject(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN      IMediaEvent *       pGraph,
        IN      ITStream **         ppStream
        )
{
    LOG((MSP_TRACE, "CSampleMSPCall::CreateStreamObject - enter"));

    HRESULT hr;
    CMSPComObject<CSampleMSPStream> * pStream;
 
    hr = CMSPComObject<CSampleMSPStream>::CreateInstance( &pStream );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::CreateStreamObject - "
            "can't create stream object - 0x%08x", hr));

        return hr;
    }

    hr = pStream->_InternalQueryInterface( IID_ITStream,
                                           (void **) ppStream );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::CreateStreamObject - "
            "can't get ITStream interface - 0x%08x", hr));

        delete pStream;
        return hr;
    }

    hr = pStream->Init( (MSP_HANDLE) m_pMSPAddress,
                       this,
                       pGraph,
                       dwMediaType,
                       Direction);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::CreateStreamObject - "
            "can't Init stream object - 0x%08x", hr));

        (*ppStream)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CSampleMSPCall::CreateStreamObject - exit S_OK"));
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Here we implement our sample TSP-MSP communication. The key thing to note
// here is that the protocol is arbitrary; it simply must be agreed upon by
// the TSP and MSP involved, and TAPI3.dll enables it by allowing opaque
// buffers to be passed between the TSP and MSP. TAPI3.dll itself knows nothing
// about the contents of the buffers.
//
// To demonstrate we have chosen a very simple protocol, which simply allows
// the TSP to configure the transports used on the streams, and stop and start
// streaming at opportune times. All of these messages are sent by the TSP and
// received by the MSP, with TAPI3.dll implementing the channel across the
// processes. At a minimum, the TSP should send a start message when the call
// connects and a stop message when the call disconnects. This sample does not
// show any communication sent back from the MSP to the TSP, but there is a
// mechanism in place for this as well (see the code for
// CSampleMSPStream::FireEvent in this sample and the file msp.idl in the
// Windows SDK for more information).
//
// Here's what our sample protocol looks like:
//
// First DWORD =  Command            Second DWORD  Third DWORD
// -------------  -------            ------------  -----------
// 0              Config transport   render config capture config
// 1              Start streaming    <ignored>     <ignored>
// 2              Stop streaming     <ignored>     <ignored>
//
// The render config and capture config are not actually used in this
// example; they are just there as a placeholder for some meaningful info
// and to show how to check the length of the buffer, etc.
//
// The method returns S_OK even if an individual stream failed to
// start, stop, or initialize. This is because TAPI 3.0 doesn't need to
// know about streaming failures in this code path. Instead, events are
// generated to inform the application of failures.
//

HRESULT CSampleMSPCall::ReceiveTSPCallData(
    IN      PBYTE               pBuffer,
    IN      DWORD               dwSize
    )
{
    LOG((MSP_TRACE, "CSampleMSPCall::ReceiveTSPCallData - enter"));

    //
    // Check that the buffer is as big as advertised.
    //

    if (!pBuffer)
    {
        LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
            "bad buffer - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Check if we have a command DWORD.
    //

    if ( dwSize < sizeof(DWORD) )
    {
        LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
            "need a DWORD for command - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // We are going to access the streams lists -- grab the lock
    //

    CLock lock(m_lock);

    _ASSERTE( m_Streams.GetSize() == 2 );

    HRESULT hr;

    //
    // Based on the command, take action:
    //

    switch ( ((DWORD *) pBuffer) [0] )
    {
    case 0: // configure the transports
        {
            //
            // In a real MSP, this sort of command would have meaningful
            // arguments that are passed to the streams. For our
            // purposes, let's just define this to have two DWORD
            // arguments and print what they are when we receive
            // the command.
            //

            if ( dwSize < 3 * sizeof(DWORD) )
            {
                LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
                    "need 3 DWORDs for Config Transports command - "
                    "exit E_INVALIDARG"));

                return E_INVALIDARG;
            }
            LOG((MSP_INFO, "CSampleMSPCall::ReceiveTSPCallData - "
                "configuring transport - dword args %d, %d",
                ((DWORD *) pBuffer) [1],
                ((DWORD *) pBuffer) [2]));

            //
            // Use our saved class pointers to access the private method,
            // and also to conveniently differentiate between render and
            // capture. Note that the capture stream is the one with a
            // capture terminal, and the render stream is the one with the
            // render terminal, so the arguments would have to be matched
            // here based on the data from the buffer (if there were any).
            //

            hr = m_pCaptureStream->ConfigureTransport ( ); 

            if ( FAILED(hr) )
            {
                LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
                    "capture stream ConfigureTransport failed 0x%08x - "
                    "firing CALL_STREAM_FAIL", hr));

                m_pCaptureStream->FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
            }

            hr = m_pRenderStream ->ConfigureTransport( );

            if ( FAILED(hr) )
            {
                LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
                    "render stream ConfigureTransport failed 0x%08x - "
                    "firing CALL_STREAM_FAIL", hr));

                m_pRenderStream->FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
            }

        }
        break;

    case 1: // start streaming
        {
            hr = m_pCaptureStream ->StartStream();
            hr = m_pRenderStream  ->StartStream();
        }
        break;

    case 2: // stop streaming
        {
            hr = m_pCaptureStream ->StopStream();
            hr = m_pRenderStream  ->StopStream();
        }
        break;

    default:
        LOG((MSP_ERROR, "CSampleMSPCall::ReceiveTSPCallData - "
            "invalid command - exit E_INVALIDARG"));

        return E_INVALIDARG;

    }

    LOG((MSP_TRACE, "CSampleMSPCall::ReceiveTSPCallData - exit S_OK"));
    return S_OK;
}

// eof

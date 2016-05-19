//////////////////////////////////////////////////////////////////////////
// 
// player.cpp : Playback helper class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "ProtectedPlayback.h"
#include "ContentEnabler.h"


// Forward declarations

HRESULT CreateSourceStreamNode(
    IMFMediaSource *pSource,
    IMFPresentationDescriptor *pSourcePD, 
    IMFStreamDescriptor *pSourceSD,
    IMFTopologyNode **ppNode
    );

HRESULT CreateOutputNode(
    IMFStreamDescriptor *pSourceSD, 
    HWND hwndVideo,
    IMFTopologyNode **ppNode
    );


///////////////////////////////////////////////////////////////////////
//  Name: CreateInstance
//  Description:  Static class method to create the CPlayer object.
//  
//  hVideo:   Handle to the video window.
//  hEvent:   Handle to the window to receive notifications.
//  ppPlayer: Receives an AddRef's pointer to the CPlayer object. 
//            The caller must release the pointer.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::CreateInstance(HWND hVideo, HWND hEvent, CPlayer **ppPlayer)
{
    TRACE((L"CPlayer::Create\n"));

    assert(hVideo != NULL);
    assert(hEvent != NULL);

    if (ppPlayer == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    CPlayer *pPlayer = new CPlayer(hVideo, hEvent);

    if (pPlayer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = pPlayer->Initialize();

    if (SUCCEEDED(hr))
    {
        *ppPlayer = pPlayer;
        (*ppPlayer)->AddRef();
    }

    // The CPlayer constructor sets the ref count to 1.
    // If the method succeeds, the caller receives an AddRef'd pointer.
    // Whether the method succeeds or fails, we must release the pointer.

    SAFE_RELEASE(pPlayer);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  CPlayer constructor
/////////////////////////////////////////////////////////////////////////

CPlayer::CPlayer(HWND hVideo, HWND hEvent) : 
    m_pSession(NULL),
    m_pSource(NULL),
    m_pVideoDisplay(NULL),
    m_hwndVideo(hVideo),
    m_hwndEvent(hEvent),
    m_state(Ready),
    m_hCloseEvent(NULL),
    m_nRefCount(1),
    m_pContentProtectionManager(NULL)
{
}

///////////////////////////////////////////////////////////////////////
//  CPlayer destructor
/////////////////////////////////////////////////////////////////////////

CPlayer::~CPlayer()
{
    assert(m_pSession == NULL);  // If FALSE, the app did not call Shutdown().

    // Note: The application must call Shutdown() because the media 
    // session holds a reference count on the CPlayer object. (This happens
    // when CPlayer calls IMediaEventGenerator::BeginGetEvent on the
    // media session.) As a result, there is a circular reference count
    // between the CPlayer object and the media session. Calling Shutdown()
    // breaks the circular reference count.

    // Note: If CreateInstance failed, the application will not call 
    // Shutdown(). To handle that case, we must call Shutdown() in the 
    // destructor. The circular ref-count problem does not occcur if
    // CreateInstance has failed. Also, calling Shutdown() twice is
    // harmless.

    Shutdown();
}


//////////////////////////////////////////////////////////////////////
//  Name: Initialize
//  Initializes the CPlayer object. This method is called by the
//  CreateInstance method.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Initialize()
{
    HRESULT hr = S_OK;

    if (m_hCloseEvent)
    {
        return MF_E_ALREADY_INITIALIZED;
    }

    // Start up Media Foundation platform.
    CHECK_HR(hr = MFStartup(MF_VERSION));

    m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCloseEvent == NULL)
    {
        CHECK_HR(hr = HRESULT_FROM_WIN32(GetLastError()));
    }

done:
    return hr;
}




///////////////////////////////////////////////////////////////////////
//  AddRef
/////////////////////////////////////////////////////////////////////////

ULONG CPlayer::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


///////////////////////////////////////////////////////////////////////
//  Release
/////////////////////////////////////////////////////////////////////////

ULONG CPlayer::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}



///////////////////////////////////////////////////////////////////////
//  QueryInterface
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}



///////////////////////////////////////////////////////////////////////
//  Name: OpenURL
//  Description:  Opens a URL for playback.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::OpenURL(const WCHAR *sURL)
{
    TRACE((L"CPlayer::OpenURL\n"));
    TRACE((L"URL = %s\n", sURL));

    // 1. Create a new media session.
    // 2. Create the media source.
    // 3. Create the topology.
    // 4. Queue the topology [asynchronous]
    // 5. Start playback [asynchronous - does not happen in this method.]

    HRESULT hr = S_OK;
    IMFTopology *pTopology = NULL;

    // Create the media session.
    CHECK_HR(hr = CreateSession());

    // Create the media source.
    CHECK_HR(hr = CreateMediaSource(sURL));

    // Create a partial topology.
    CHECK_HR(hr = CreateTopologyFromSource(&pTopology));

    // Set the topology on the media session.
    CHECK_HR(hr = m_pSession->SetTopology(0, pTopology));

    // Set our state to "open pending"
    m_state = OpenPending;

    // If SetTopology succeeded, the media session will queue an 
    // MESessionTopologySet event.

done:
    if (FAILED(hr))
    {
        m_state = Closed;
    }

    SAFE_RELEASE(pTopology);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: Play
//  Description:  Starts playback from paused state.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Play()
{
    TRACE((L"CPlayer::Play\n"));

    if (m_state != Paused && m_state != Stopped)
    {
        return MF_E_INVALIDREQUEST;
    }
    if (m_pSession == NULL || m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = StartPlayback();

    return hr;
}

///////////////////////////////////////////////////////////////////////
//  Name: Pause
//  Description:  Pauses playback.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Pause()
{
    TRACE((L"CPlayer::Pause\n"));

    if (m_state != Started)
    {
        return MF_E_INVALIDREQUEST;
    }
    if (m_pSession == NULL || m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pSession->Pause();

    if (SUCCEEDED(hr))
    {
        m_state = Paused;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////
//  Name: Repaint
//  Description:  Repaint the video window.
//
//  Note: The application should call this method when it receives a
//        WM_PAINT message.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Repaint()
{
    HRESULT hr = S_OK;

    if (m_pVideoDisplay)
    {
        hr = m_pVideoDisplay->RepaintVideo();
    }
    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: ResizeVideo
//  Description:  Repaint the video window.
//
//  Note: The application should call this method when it receives a
//        WM_SIZE message.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::ResizeVideo(WORD width, WORD height)
{
    HRESULT hr = S_OK;

    if (m_pVideoDisplay)
    {
        // Set the destination rectangle.
        // Leave the default source rectangle (0,0,1,1).

        RECT rcDest = { 0, 0, width, height };

        hr = m_pVideoDisplay->SetVideoPosition(NULL, &rcDest);
    }
    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: Invoke
//  Description:  Callback for asynchronous BeginGetEvent method.
//  
//  pAsyncResult: Pointer to the result.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Invoke(IMFAsyncResult *pResult)
{
    HRESULT hr = S_OK;
    MediaEventType meType = MEUnknown;  // Event type

    IMFMediaEvent *pEvent = NULL;

    // Get the event from the event queue.
    CHECK_HR(hr = m_pSession->EndGetEvent(pResult, &pEvent));

    // Get the event type. 
    CHECK_HR(hr = pEvent->GetType(&meType));

    // If the session is closed, the application is waiting on the
    // m_hCloseEvent event handle. Also, do not get any more 
    // events from the session.

    if (meType == MESessionClosed)
    {
        SetEvent(m_hCloseEvent);
    }
    else
    {
        // For all other events, ask the media session for the
        // next event in the queue.
        CHECK_HR(hr = m_pSession->BeginGetEvent(this, NULL));
    }

    // For most events, we post the event as a private window message to the
    // application. This lets the application process the event on it's
    // main thread.

    // However, if call to IMFMediaSession::Close is pending, it means the 
    // application is waiting on the m_hCloseEvent event handle. (Blocking 
    // call.) In that case, we simply discard the event.

    // NOTE: When IMFMediaSession::Close is called, MESessionClosed is NOT
    // necessarily the next event that we will receive. We may receive
    // any number of other events before receiving MESessionClosed.
    if (m_state != Closing)
    {
        // Leave a reference count on the event.
        pEvent->AddRef();
        PostMessage(m_hwndEvent, WM_APP_PLAYER_EVENT, (WPARAM)pEvent, (LPARAM)0);
    }

done:
    SAFE_RELEASE(pEvent);
    return S_OK;
}


//-------------------------------------------------------------------
//  HandleEvent
//
//  Called by the application when it receives a WM_APP_PLAYER_EVENT
//  message. 
//
//  This method is used to process media session events on the
//  application's main thread.
//
//  pUnkPtr: Pointer to the IUnknown interface of a media session 
//  event (IMFMediaEvent).
//-------------------------------------------------------------------

HRESULT CPlayer::HandleEvent(UINT_PTR pUnkPtr)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus = S_OK;            // Event status
    MediaEventType meType = MEUnknown;  // Event type
    MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    

    IUnknown *pUnk = NULL;
    IMFMediaEvent *pEvent = NULL;

    // pUnkPtr is really an IUnknown pointer.
    pUnk = (IUnknown*)pUnkPtr;

    if (pUnk == NULL)
    {
        return E_POINTER;
    }

    CHECK_HR(hr = pUnk->QueryInterface(__uuidof(IMFMediaEvent), (void**)&pEvent));

    // Get the event type.
    CHECK_HR(hr = pEvent->GetType(&meType));

    // Get the event status. If the operation that triggered the event did
    // not succeed, the status is a failure code.
    CHECK_HR(hr = pEvent->GetStatus(&hrStatus));

    TRACE((L"Media event: %s\n", EventName(meType)));

    // Check if the async operation succeeded.
    if (SUCCEEDED(hrStatus))
    {
        // Switch on the event type. Update the internal state of the CPlayer as needed.
        switch(meType)
        {
        case MESessionTopologyStatus:
            // Get the status code.
            CHECK_HR(hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
            switch (TopoStatus)
            {
            case MF_TOPOSTATUS_READY: 
                hr = OnTopologyReady(pEvent);
                break;
            default: 
                // Nothing to do.
                break;  
            }
            break;

        case MEEndOfPresentation:
            CHECK_HR(hr = OnPresentationEnded(pEvent));
            break;
        }
    }
    else
    {
        hr = hrStatus;
    }

done:
    SAFE_RELEASE(pUnk);
    SAFE_RELEASE(pEvent);
    return hr;
}



///////////////////////////////////////////////////////////////////////
//  Name: ShutDown
//  Description:  Releases all resources held by this object.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::Shutdown()
{
    TRACE((L"CPlayer::ShutDown\n"));

    HRESULT hr = S_OK;

    // Close the session
    hr = CloseSession();

    // Shutdown the Media Foundation platform
    MFShutdown();

    if (m_hCloseEvent)
    {
        CloseHandle(m_hCloseEvent);
        m_hCloseEvent = NULL;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: GetContentProtectionManager
//  Description:  Returns the content protection manager object.
//
//  This is a helper object for handling IMFContentEnabler operations.
/////////////////////////////////////////////////////////////////////////

HRESULT  CPlayer::GetContentProtectionManager(ContentProtectionManager **ppManager)
{
    if (ppManager == NULL)
    {
        return E_INVALIDARG;
    }

    if (m_pContentProtectionManager == NULL)
    {
        return E_FAIL; // Session wasn't created yet. No helper object;
    }

    *ppManager = m_pContentProtectionManager;
    (*ppManager)->AddRef();

    return S_OK;

}


///
///
/// Protected methods
///
/// All methods that follow are private to the CPlayer class.
///
///


///////////////////////////////////////////////////////////////////////
//  Name: OnTopologyReady
//  Description:  Handler for MESessionTopologyReady event.
//
//  Note: 
//  - The MESessionTopologySet event means the session queued the 
//    topology, but the topology is not ready yet. Generally, the 
//    applicationno need to respond to this event unless there is an
//    error.
//  - The MESessionTopologyReady event means the new topology is
//    ready for playback. After this event is received, any calls to
//    IMFGetService will get service interfaces from the new topology.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::OnTopologyReady(IMFMediaEvent *pEvent)
{
    TRACE((L"CPlayer::OnTopologyReady\n"));

    // Ask for the IMFVideoDisplayControl interface.
    // This interface is implemented by the EVR and is
    // exposed by the media session as a service.

    // Note: This call is expected to fail if the source
    // does not have video.

    MFGetService(
        m_pSession,
        MR_VIDEO_RENDER_SERVICE,
        __uuidof(IMFVideoDisplayControl),
        (void**)&m_pVideoDisplay
        );

    HRESULT hr = StartPlayback();

    return S_OK;
}



HRESULT CPlayer::OnPresentationEnded(IMFMediaEvent *pEvent)
{
    TRACE((L"CPlayer::OnPresentationEnded\n"));

    // The session puts itself into the stopped state autmoatically.

    m_state = Stopped;

    return S_OK;
}


///////////////////////////////////////////////////////////////////////
//  Name: CreateSession
//  Description:  Creates a new instance of the media session.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::CreateSession()
{
    TRACE((L"CPlayer::CreateSession\n"));

    HRESULT hr = S_OK;

    IMFAttributes *pAttributes = NULL;
    IMFActivate   *pEnablerActivate = NULL;

    // Close the old session, if any.
    CHECK_HR(hr = CloseSession());

    assert(m_state == Closed);

    // Create a new attribute store.
    CHECK_HR(hr = MFCreateAttributes(&pAttributes, 1));

    // Create the content protection manager.
    assert(m_pContentProtectionManager == NULL); // Was released in CloseSession

    CHECK_HR(hr = ContentProtectionManager::CreateInstance(
            m_hwndEvent, 
            &m_pContentProtectionManager
            ));

    // Set the MF_SESSION_CONTENT_PROTECTION_MANAGER attribute with a pointer
    // to the content protection manager.
    CHECK_HR(hr = pAttributes->SetUnknown(
            MF_SESSION_CONTENT_PROTECTION_MANAGER, 
            (IMFContentProtectionManager*)m_pContentProtectionManager
            ));

    // Create the PMP media session.
    CHECK_HR(hr = MFCreatePMPMediaSession(
            0, // Can use this flag: MFPMPSESSION_UNPROTECTED_PROCESS
            pAttributes, 
            &m_pSession,
            &pEnablerActivate
            ));


    // TODO:

    // If MFCreatePMPMediaSession fails it might return an IMFActivate pointer.
    // This indicates that a trusted binary failed to load in the protected process.
    // An application can use the IMFActivate pointer to create an enabler object, which 
    // provides revocation and renewal information for the component that failed to
    // load. 

    // This sample does not demonstrate that feature. Instead, we simply treat this
    // case as a playback failure. 


    // Start pulling events from the media session
    CHECK_HR(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

done:
    SAFE_RELEASE(pAttributes);
    SAFE_RELEASE(pEnablerActivate);
    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: CloseSession
//  Description:  Closes the media session. 
//
//  Note: The IMFMediaSession::Close method is asynchronous, but the
//        CPlayer::CloseSession method waits on the MESessionClosed event.
//        The MESessionClosed event is guaranteed to be the last event 
//        that the media session fires.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::CloseSession()
{
    HRESULT hr = S_OK;

    SAFE_RELEASE(m_pVideoDisplay);

    if (m_pSession)
    {
        DWORD dwWaitResult = 0;

        m_state = Closing;
           
        CHECK_HR(hr = m_pSession->Close());

        // Wait for the close operation to complete
        
        dwWaitResult = WaitForSingleObject(m_hCloseEvent, 5000);

        if (dwWaitResult == WAIT_TIMEOUT)
        {
            TRACE((L"CloseSession timed out!\n"));
        }

        // Now there will be no more events from this session.
    }

    // Complete shutdown operations.

    // Shut down the media source. (Synchronous operation, no events.)
    if (m_pSource)
    {
        m_pSource->Shutdown();
    }

    // Shut down the media session. (Synchronous operation, no events.)
    if (m_pSession)
    {
        m_pSession->Shutdown();
    }

    SAFE_RELEASE(m_pSource);
    SAFE_RELEASE(m_pSession);
    SAFE_RELEASE(m_pContentProtectionManager);

    m_state = Closed;

done:
    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: StartPlayback
//  Description:  Starts playback from the current position. 
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::StartPlayback()
{
    TRACE((L"CPlayer::StartPlayback\n"));

    assert(m_pSession != NULL);

    HRESULT hr = S_OK;

    PROPVARIANT varStart;
    PropVariantInit(&varStart);

    varStart.vt = VT_EMPTY;

    hr = m_pSession->Start(&GUID_NULL, &varStart);

    if (SUCCEEDED(hr))
    {
        // Note: Start is an asynchronous operation. However, we
        // can treat our state as being already started. If Start
        // fails later, we'll get an MESessionStarted event with
        // an error code, and we will update our state then.
        m_state = Started;
    }

    PropVariantClear(&varStart);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: CreateMediaSource
//  Description:  Create a media source from a URL.
//
//  sURL: The URL to open.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::CreateMediaSource(const WCHAR *sURL)
{
    TRACE((L"CPlayer::CreateMediaSource\n"));

    HRESULT hr = S_OK;
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

    IMFSourceResolver* pSourceResolver = NULL;
    IUnknown* pSource = NULL;

    SAFE_RELEASE(m_pSource);

    // Create the source resolver.
    CHECK_HR(hr = MFCreateSourceResolver(&pSourceResolver));

    // Use the source resolver to create the media source.

    // Note: For simplicity this sample uses the synchronous method on
    // IMFSourceResolver to create the media source. However, creating a 
    // media source can take a noticeable amount of time, especially for
    // a network source. For a more responsive UI, use the asynchronous
    // BeginCreateObjectFromURL method.

    CHECK_HR(hr = pSourceResolver->CreateObjectFromURL(
                sURL,                       // URL of the source.
                MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
                NULL,                       // Optional property store.
                &ObjectType,                // Receives the created object type. 
                &pSource                    // Receives a pointer to the media source.
            ));

    // Get the IMFMediaSource interface from the media source.
    CHECK_HR(hr = pSource->QueryInterface(__uuidof(IMFMediaSource), (void**)&m_pSource));

done:
    SAFE_RELEASE(pSourceResolver);
    SAFE_RELEASE(pSource);
    return hr;
}



///////////////////////////////////////////////////////////////////////
//  CreateTopologyFromSource
//  Description:  Create a playback topology from the media source.
//
//  Pre-condition: The media source must be created already.
//                 Call CreateMediaSource() before calling this method.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::CreateTopologyFromSource(IMFTopology **ppTopology)
{
    TRACE((L"CPlayer::CreateTopologyFromSource\n"));

    assert(m_pSession != NULL);
    assert(m_pSource != NULL);

    HRESULT hr = S_OK;

    IMFTopology *pTopology = NULL;
    IMFPresentationDescriptor* pSourcePD = NULL;
    DWORD cSourceStreams = 0;

    // Create a new topology.
    CHECK_HR(hr = MFCreateTopology(&pTopology));

    // Create the presentation descriptor for the media source.
    CHECK_HR(hr = m_pSource->CreatePresentationDescriptor(&pSourcePD));

    // Get the number of streams in the media source.
    CHECK_HR(hr = pSourcePD->GetStreamDescriptorCount(&cSourceStreams));

    TRACE((L"Stream count: %d\n", cSourceStreams));

    // For each stream, create the topology nodes and add them to the topology.
    for (DWORD i = 0; i < cSourceStreams; i++)
    {
        CHECK_HR(hr = AddBranchToPartialTopology(pTopology, pSourcePD, i));
    }

    // Return the IMFTopology pointer to the caller.
    if (SUCCEEDED(hr))
    {
        *ppTopology = pTopology;
        (*ppTopology)->AddRef();
    }

done:
    SAFE_RELEASE(pTopology);
    SAFE_RELEASE(pSourcePD);
    return hr;
}





///////////////////////////////////////////////////////////////////////
//  Name:  AddBranchToPartialTopology 
//  Description:  Adds a topology branch for one stream.
//
//  pTopology: Pointer to the topology object.
//  pSourcePD: The source's presentation descriptor.
//  iStream: Index of the stream to render.
//
//  Pre-conditions: The topology must be created already.
//
//  Notes: For each stream, we must do the following:
//    1. Create a source node associated with the stream. 
//    2. Create an output node for the renderer. 
//    3. Connect the two nodes.
//  The media session will resolve the topology, so we do not have
//  to worry about decoders or other transforms.
/////////////////////////////////////////////////////////////////////////

HRESULT CPlayer::AddBranchToPartialTopology(IMFTopology *pTopology, IMFPresentationDescriptor *pSourcePD, DWORD iStream)
{
    TRACE((L"CPlayer::AddBranchToPartialTopology\n"));

    assert(pTopology != NULL);

    IMFStreamDescriptor* pSourceSD = NULL;
    IMFTopologyNode* pSourceNode = NULL;
    IMFTopologyNode* pOutputNode = NULL;
    BOOL fSelected = FALSE;

    HRESULT hr = S_OK;

    // Get the stream descriptor for this stream.
    CHECK_HR(hr = pSourcePD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSourceSD));

    // Create the topology branch only if the stream is selected.
    // Otherwise, do nothing.
    if (fSelected)
    {
        // Create a source node for this stream.
        CHECK_HR(hr = CreateSourceStreamNode(m_pSource, pSourcePD, pSourceSD, &pSourceNode));

        // Create the output node for the renderer.
        CHECK_HR(hr = CreateOutputNode(pSourceSD, m_hwndVideo, &pOutputNode));

        // Add both nodes to the topology.
        CHECK_HR(hr = pTopology->AddNode(pSourceNode));

        CHECK_HR(hr = pTopology->AddNode(pOutputNode));

        // Connect the source node to the output node.
        CHECK_HR(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
    }

done:
    // Clean up.
    SAFE_RELEASE(pSourceSD);
    SAFE_RELEASE(pSourceNode);
    SAFE_RELEASE(pOutputNode);
    return hr;
}


/// Static functions

//-------------------------------------------------------------------
//  Name: CreateSourceStreamNode
//  Description:  Creates a source-stream node for a stream.
// 
//  pSource: Pointer to the media source that contains the stream.
//  pSourcePD: Presentation descriptor for the media source.
//  pSourceSD: Stream descriptor for the stream.
//  ppNode: Receives a pointer to the new node.
//-------------------------------------------------------------------

HRESULT CreateSourceStreamNode(
    IMFMediaSource *pSource,
    IMFPresentationDescriptor *pSourcePD, 
    IMFStreamDescriptor *pSourceSD,
    IMFTopologyNode **ppNode
    )
{
    if (!pSource || !pSourcePD || !pSourceSD || !ppNode)
    {
        return E_POINTER;
    }

    IMFTopologyNode *pNode = NULL;
    HRESULT hr = S_OK;

    // Create the source-stream node. 
    CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));

    // Set attribute: Pointer to the media source.
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));

    // Set attribute: Pointer to the presentation descriptor.
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pSourcePD));

    // Set attribute: Pointer to the stream descriptor.
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSourceSD));

    // Return the IMFTopologyNode pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SAFE_RELEASE(pNode);
    return hr;
}




//-------------------------------------------------------------------
//  Name: CreateOutputNode
//  Description:  Create an output node for a stream.
//
//  pSourceSD: Stream descriptor for the stream.
//  ppNode: Receives a pointer to the new node.
//
//  Notes:
//  This function does the following:
//  1. Chooses a renderer based on the media type of the stream.
//  2. Creates an IActivate object for the renderer.
//  3. Creates an output topology node.
//  4. Sets the IActivate pointer on the node.
//-------------------------------------------------------------------

HRESULT CreateOutputNode(
    IMFStreamDescriptor *pSourceSD, 
    HWND hwndVideo,
    IMFTopologyNode **ppNode
    )
{   

    IMFTopologyNode *pNode = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFActivate *pRendererActivate = NULL;

    GUID guidMajorType = GUID_NULL;
    HRESULT hr = S_OK;

    // Get the stream ID.
    DWORD streamID = 0;
    pSourceSD->GetStreamIdentifier(&streamID); // Just for debugging, ignore any failures.

    // Get the media type handler for the stream.
    CHECK_HR(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
    
    // Get the major media type.
    CHECK_HR(hr = pHandler->GetMajorType(&guidMajorType));

    // Create a downstream node.
    CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));

    // Create an IMFActivate object for the renderer, based on the media type.
    if (MFMediaType_Audio == guidMajorType)
    {
        // Create the audio renderer.
        TRACE((L"Stream %d: audio stream\n", streamID));
        CHECK_HR(hr = MFCreateAudioRendererActivate(&pRendererActivate));
    }
    else if (MFMediaType_Video == guidMajorType)
    {
        // Create the video renderer.
        TRACE((L"Stream %d: video stream\n", streamID));
        CHECK_HR(hr = MFCreateVideoRendererActivate(hwndVideo, &pRendererActivate));
    }
    else
    {
        TRACE((L"Stream %d: Unknown format\n", streamID));
        CHECK_HR(hr = E_FAIL);
    }

    // Set the IActivate object on the output node.
    CHECK_HR(hr = pNode->SetObject(pRendererActivate));

    // Return the IMFTopologyNode pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SAFE_RELEASE(pNode);
    SAFE_RELEASE(pHandler);
    SAFE_RELEASE(pRendererActivate);
    return hr;
}

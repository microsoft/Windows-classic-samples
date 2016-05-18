// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedplayer.h"
#include <mferror.h>
#include "assert.h"
#include "intsafe.h"

#include <initguid.h>
EXTERN_GUID(CLSID_CResamplerMediaObject,   0xf447b69e, 0x1884, 0x4a7e, 0x80, 0x55, 0x34, 0x6f, 0x74, 0xd6, 0xed, 0xb3);

////////////////////////////////////////////////////////////////////////////////
//
CTedMediaFileRenderer::CTedMediaFileRenderer(LPCWSTR szFileName, ITedVideoWindowHandler* pVideoWindowHandler, HRESULT& hr)
    : m_szFileName(szFileName)
    , m_spVideoWindowHandler(pVideoWindowHandler)
{
    hr = MFCreateTopoLoader(&m_spTopoLoader);
}

HRESULT CTedMediaFileRenderer::Load(IMFTopology** ppOutputTopology)
{
    HRESULT hr;
    CComPtr<IMFTopology> spPartialTopology;
    
    IFC( CreatePartialTopology(&spPartialTopology) );
    IFC( m_spTopoLoader->Load(spPartialTopology, ppOutputTopology, NULL) );
    
Cleanup:
    return hr;
}

HRESULT CTedMediaFileRenderer::CreatePartialTopology(IMFTopology** ppPartialTopology)
{
    HRESULT hr;
    IMFTopology* pPartialTopology;
    CComPtr<IMFMediaSource> spSource;
    
    IFC( MFCreateTopology(&pPartialTopology) );
    
    IFC( CreateSource(&spSource) );
    IFC( BuildTopologyFromSource(pPartialTopology, spSource) );
    
    *ppPartialTopology = pPartialTopology;
    
Cleanup:
    return hr;
}

HRESULT CTedMediaFileRenderer::CreateSource(IMFMediaSource** ppSource)
{
    HRESULT hr;
    CComPtr<IMFSourceResolver> spSourceResolver;
    CComPtr<IUnknown> spSourceUnk;
    IMFMediaSource* pSource;
    
    IFC( MFCreateSourceResolver(&spSourceResolver) );
    
    MF_OBJECT_TYPE ObjectType;
    IFC( spSourceResolver->CreateObjectFromURL(m_szFileName, MF_RESOLUTION_MEDIASOURCE,
                                            NULL, &ObjectType, &spSourceUnk) );
    hr = spSourceUnk->QueryInterface(IID_IMFMediaSource, (void**) &pSource);
    if(E_NOINTERFACE == hr)
    {
        hr = MF_E_UNSUPPORTED_BYTESTREAM_TYPE;
    }
    IFC( hr );
    
    *ppSource = pSource;

Cleanup:
    return hr;
}

// Given a source, connect each stream to a renderer for its media type
HRESULT CTedMediaFileRenderer::BuildTopologyFromSource(IMFTopology* pTopology, IMFMediaSource* pSource)
{
    HRESULT hr;
    CComPtr<IMFPresentationDescriptor> spPD;
    
    IFC( pSource->CreatePresentationDescriptor(&spPD) );
    
    DWORD cSourceStreams = 0;
    IFC( spPD->GetStreamDescriptorCount(&cSourceStreams) );
    for(DWORD i = 0; i < cSourceStreams; i++)
    {
        CComPtr<IMFStreamDescriptor> spSD;
        CComPtr<IMFTopologyNode> spNode;
        CComPtr<IMFTopologyNode> spRendererNode;
        BOOL fSelected = FALSE;

        IFC( spPD->GetStreamDescriptorByIndex(i, &fSelected, &spSD) );

        IFC( MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &spNode) );
        IFC( spNode->SetUnknown(MF_TOPONODE_SOURCE, pSource) );
        IFC( spNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, spPD) );
        IFC( spNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, spSD) );
        IFC( pTopology->AddNode(spNode) );
        
        IFC( CreateRendererForStream(spSD, &spRendererNode) );
        IFC( spNode->ConnectOutput(0, spRendererNode, 0) );
        IFC( pTopology->AddNode(spRendererNode) );
    }
    
Cleanup:
    return hr;
}

// Create a renderer for the media type on the given stream descriptor
HRESULT CTedMediaFileRenderer::CreateRendererForStream(IMFStreamDescriptor* pSD, IMFTopologyNode** ppRendererNode)
{
    HRESULT hr;
    CComPtr<IMFMediaTypeHandler> spMediaTypeHandler;
    CComPtr<IMFActivate> spRendererActivate;
    CComPtr<IMFMediaSink> spRendererSink;
    CComPtr<IMFStreamSink> spRendererStreamSink;
    IMFTopologyNode* pRendererNode;
    GUID gidMajorType;

    IFC( MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pRendererNode) );
    
    IFC( pSD->GetMediaTypeHandler( &spMediaTypeHandler ) );
    IFC( spMediaTypeHandler->GetMajorType( &gidMajorType ) );

    if(MFMediaType_Audio == gidMajorType) 
    {
        IFC( MFCreateAudioRendererActivate(&spRendererActivate) );
        IFC( spRendererActivate->ActivateObject(IID_IMFMediaSink, (void**) &spRendererSink) );
        IFC( spRendererSink->GetStreamSinkById(0, &spRendererStreamSink) );
        IFC( pRendererNode->SetObject(spRendererStreamSink) );
    }
    else if(MFMediaType_Video == gidMajorType)
    {
        HWND hVideoWindow;
        IFC( m_spVideoWindowHandler->GetVideoWindow((LONG_PTR*) &hVideoWindow) );
        IFC( MFCreateVideoRendererActivate(hVideoWindow, &spRendererActivate) );
        IFC( spRendererActivate->ActivateObject(IID_IMFMediaSink, (void**) &spRendererSink) );
        IFC( spRendererSink->GetStreamSinkById(0, &spRendererStreamSink) );
        IFC( pRendererNode->SetObject(spRendererStreamSink) );
    }
    else
    {
        // Do not have renderers for any other major types
    }
    
    *ppRendererNode = pRendererNode;
Cleanup:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
CTedPlayer::CTedPlayer(CTedMediaEventHandler* pMediaEventHandler, IMFContentProtectionManager* pCPM)
    : m_cRef(1)
    , m_pMediaEventHandler(pMediaEventHandler)
    , m_pCPM(pCPM)
    , m_bIsPlaying(false)
    , m_bIsPaused(false)
    , m_fTopologySet(false)
    , m_LastSeqID(0)
    , m_hnsMediastartOffset(0)
    , m_fPendingClearCustomTopoloader(false)
    , m_fPendingProtectedCustomTopoloader(false)
{
    m_pCPM->AddRef();
}

CTedPlayer::~CTedPlayer()
{
    HRESULT hr;
   
    if(m_pCPM)
    {
        m_pCPM->Release();
    }

    for(size_t i = 0; i < m_aTopologies.GetCount(); i++)
    {
        CComPtr<IMFCollection> spSourceNodeCollection;
        IMFTopology* pTopology = m_aTopologies.GetAt(i);
        
        hr = pTopology->GetSourceNodeCollection(&spSourceNodeCollection);
        if(FAILED(hr))
        {
            pTopology->Release();
            continue;
        }
        
        DWORD cElements = 0;
        spSourceNodeCollection->GetElementCount(&cElements);
        for(DWORD j = 0; j < cElements; j++)
        {
            CComPtr<IUnknown> spSourceNodeUnk;
            CComPtr<IMFTopologyNode> spSourceNode;
            CComPtr<IMFMediaSource> spSource;
            
            hr = spSourceNodeCollection->GetElement(j, &spSourceNodeUnk);
            if(FAILED(hr)) continue;
            
            hr = spSourceNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSourceNode);
            if(FAILED(hr)) continue;
            
            hr = spSourceNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**) &spSource);
            if(FAILED(hr)) continue;
            
            spSource->Shutdown();
        }
        
        CComPtr<IMFCollection> spOutputNodeCollection;
        hr = pTopology->GetOutputNodeCollection(&spOutputNodeCollection);
        if(FAILED(hr))
        {
            pTopology->Release();
            continue;
        }
        
        cElements = 0;
        spOutputNodeCollection->GetElementCount(&cElements);
        for(DWORD j = 0; j < cElements; j++)
        {
            CComPtr<IUnknown> spSinkNodeUnk;
            CComPtr<IMFTopologyNode> spSinkNode;
            CComPtr<IUnknown> spStreamSinkUnk;
            CComPtr<IMFStreamSink> spStreamSink;
            CComPtr<IMFMediaSink> spSink;
            
            hr = spOutputNodeCollection->GetElement(j, &spSinkNodeUnk);
            if(FAILED(hr)) continue;
            
            hr = spSinkNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSinkNode);
            if(FAILED(hr)) continue;
            
            hr = spSinkNode->GetObject(&spStreamSinkUnk);
            if(FAILED(hr)) continue;
            
            hr = spStreamSinkUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink);
            if(FAILED(hr)) continue;
            
            hr = spStreamSink->GetMediaSink(&spSink);
            if(FAILED(hr)) continue;
            
            spSink->Shutdown();
        }
        
        pTopology->Release();
    }
    
    if(m_spClearSession)
    {
        m_spClearSession->Shutdown();
    }
    
    if(m_spProtectedSession)
    {
        m_spProtectedSession->Shutdown();
    }
}

HRESULT CTedPlayer::InitClear()
{
    HRESULT hr;
    
    if(m_spSession)
    {
        if(m_bIsPlaying)
        {
            m_bIsPlaying = false;
            m_spSession->Stop();
        }
        
        m_spSession.Release();
    }
    
	if(m_fPendingClearCustomTopoloader && m_spClearSession.p)
	{
        m_spClearSession->Shutdown();
        m_spClearSession.Release();
	}

    if(m_spClearSession.p == NULL)
    {
        CComPtr<IMFAttributes> spConfiguration = NULL;
        if(m_fPendingClearCustomTopoloader && GUID_NULL != m_gidCustomTopoloader)
        {
            IFC( MFCreateAttributes(&spConfiguration, 1) );
            IFC( spConfiguration->SetGUID(MF_SESSION_TOPOLOADER, m_gidCustomTopoloader) );
        }

        IFC( MFCreateMediaSession(spConfiguration, &m_spClearSession) );
        IFC( m_spClearSession->BeginGetEvent(&m_xOnClearSessionEvent, NULL) );

        m_fPendingClearCustomTopoloader = false;
    }
    
    m_spSession = m_spClearSession;
    
    IFC( InitFromSession() );
    
Cleanup:
    return hr;
}

HRESULT CTedPlayer::InitProtected()
{
    HRESULT hr;
    
    if(m_spSession)
    {
        if(m_bIsPlaying)
        {
            m_bIsPlaying = false;
            m_spSession->Stop();
        }
        
        m_spSession.Release();
    }

    if(m_fPendingProtectedCustomTopoloader && m_spProtectedSession.p)
    {
        m_spProtectedSession->Shutdown();
        m_spProtectedSession.Release();
    }

    if(m_spProtectedSession.p == NULL)
    {
        CComPtr<IMFAttributes> spAttr;
        IFC( MFCreateAttributes(&spAttr, 1) );          
        IFC( spAttr->SetUnknown(MF_SESSION_CONTENT_PROTECTION_MANAGER, m_pCPM) );

        if(m_fPendingProtectedCustomTopoloader && GUID_NULL != m_gidCustomTopoloader)
        {
            IFC( spAttr->SetGUID(MF_SESSION_TOPOLOADER, m_gidCustomTopoloader) );
        }

        IFC( MFCreatePMPMediaSession( 0, spAttr, &m_spProtectedSession, NULL ) );
        
        IFC( m_spProtectedSession->BeginGetEvent(&m_xOnProtectedSessionEvent, NULL) );

        m_fPendingProtectedCustomTopoloader = false;
    }
    
    m_spSession = m_spProtectedSession;
    
    IFC( InitFromSession() );
    
Cleanup:
    return hr;
}

// AddRef and Release for callbacks only; not functional
LONG CTedPlayer::AddRef()
{   
    return m_cRef;
}

LONG CTedPlayer::Release()
{
    return m_cRef;
}

HRESULT CTedPlayer::Reset() 
{
    HRESULT hr;
    
    IFC( m_spSession->ClearTopologies() );
     
Cleanup:
    return hr;
}

HRESULT CTedPlayer::SetTopology(CComPtr<IMFTopology> pPartialTopo, BOOL fTranscode)
{
    HRESULT hr;
    
    assert(pPartialTopo != NULL);
    assert(m_spSession != NULL);

    m_aTopologies.Add(pPartialTopo);
    m_aTopologies.GetAt(m_aTopologies.GetCount() - 1)->AddRef();

    // Due to a bug, the topoloader cannot correctly resolve a resampler already existing
    // in the topology, and will just insert a new one.  Work around this by removing
    // the resampler
    IFC( RemoveResamplerNode(pPartialTopo) );

    if(NULL != m_spSource.p)
    {
        m_spSource.Release();
    }
    
    m_hnsMediastartOffset = INT64_MAX;
    
    WORD cNodes;
    IFC( pPartialTopo->GetNodeCount(&cNodes) );
    for(WORD i = 0; i < cNodes; i++) 
    {
        CComPtr<IMFTopologyNode> spNode;
        MF_TOPOLOGY_TYPE tidType;

        IFC( pPartialTopo->GetNode(i, &spNode) );

        IFC( spNode->GetNodeType(&tidType) );

        // We need to find the source node so we can get the IMFMediaSource for this playback
        if(MF_TOPOLOGY_SOURCESTREAM_NODE == tidType) 
        {
            if(!m_spSource.p)
            {
                IFC( spNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**) &m_spSource) );
            }
         
            MFTIME hnsMediastart = 0;
            (void)spNode->GetUINT64(MF_TOPONODE_MEDIASTART, (UINT64*) &hnsMediastart);
            if(hnsMediastart < m_hnsMediastartOffset)
            {
                m_hnsMediastartOffset = hnsMediastart;
            }
        }
    }
    
    // Set topology attributes to enable new MF features.  HWMODE_USE_HARDWARE allows topoedit
    // to pick up hardware MFTs for decoding.  DXVA_FULL allows topoedit to enable full
    // DXVA resolution for the topology -- in MFv1, only decoders automatically inserted
    // by the topoloader and directly connected to the EVR received a D3D manager message.
    IFC( pPartialTopo->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE) );
    IFC( pPartialTopo->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL) );

    if(NULL == m_spSource.p)
    {
        IFC( MF_E_NOT_FOUND );
    }
    else
    {
        m_fIsTranscoding = fTranscode;
        IFC( m_spSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, pPartialTopo) );
    }
    
    m_fReceivedTime = false;
    
Cleanup:
    return hr;
}

HRESULT CTedPlayer::Start()
{
    HRESULT hr;
    PROPVARIANT var;

    PropVariantInit( &var );

    LONGLONG hnsSeek = (m_bIsPaused) ? PRESENTATION_CURRENT_POSITION : 0;
    if ( hnsSeek == PRESENTATION_CURRENT_POSITION )
    {
        var.vt = VT_EMPTY;
    }
    else
    {
        var.vt = VT_I8;
        var.hVal.QuadPart = hnsSeek;
    }

    hr = m_spSession->Start( NULL, &var );

    m_bIsPaused = false;

    PropVariantClear( &var );

    return hr;
}

HRESULT CTedPlayer::Stop()
{
    HRESULT hr;

    IFC( m_spSession->Stop() );

    m_bIsPlaying = false;
    
Cleanup:
    return hr;
}

HRESULT CTedPlayer::Pause()
{
    HRESULT hr;

    m_bIsPaused = true;
    IFC( m_spSession->Pause() );
    
Cleanup:
    return hr;
}

HRESULT CTedPlayer::PlayFrom(MFTIME time) 
{
    HRESULT hr = S_OK;
    PROPVARIANT var;
    PropVariantInit( &var );
    
    if ( PRESENTATION_CURRENT_POSITION == time )
    {
        var.vt = VT_EMPTY;
    }
    else
    {
        var.vt = VT_I8;
        var.hVal.QuadPart = time;
    }
        
    hr = m_spSession->Start( NULL, &var );

    PropVariantClear( &var );

    return( hr );
}

HRESULT CTedPlayer::GetDuration(MFTIME& hnsTime) 
{
    HRESULT hr = S_OK;
    IMFPresentationDescriptor *pPD = NULL;

    if(NULL != m_spSource.p)
    {
        IFC( m_spSource->CreatePresentationDescriptor( &pPD ) );
        IFC( pPD->GetUINT64( MF_PD_DURATION, (UINT64*) &hnsTime ) );
    }
    else
    {
        hr = E_POINTER;
    }

    
Cleanup:
    if(pPD) pPD->Release();
    
    return( hr );
}

HRESULT CTedPlayer::GetFullTopology(IMFTopology** ppFullTopo) 
{
    HRESULT hr = S_OK;

    CComPtr<IMFGetService> spGetService;

    if(m_spFullTopology)
    {
        *ppFullTopo = m_spFullTopology;
        (*ppFullTopo)->AddRef();
    }
    else
    {
        hr = m_spSession->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, ppFullTopo);
    }

    return hr;
}

void CTedPlayer::OnClearSessionEvent(IMFAsyncResult* pResult)
{
    HRESULT hr;
    CComPtr<IMFMediaEvent> spEvent;

    IFC( m_spClearSession->EndGetEvent(pResult, &spEvent) );
    
    if(m_spSession.p == m_spClearSession.p)
    {
        IFC( HandleEvent(spEvent) );
    }
    
    IFC( m_spClearSession->BeginGetEvent(&m_xOnClearSessionEvent, NULL) );

Cleanup:
    if(FAILED(hr)) 
    {
        m_pMediaEventHandler->NotifyEventError(hr);
    }
}

void CTedPlayer::OnProtectedSessionEvent(IMFAsyncResult* pResult)
{
    HRESULT hr;
    CComPtr<IMFMediaEvent> spEvent;

    IFC( m_spProtectedSession->EndGetEvent(pResult, &spEvent) );
    
    if(m_spSession.p == m_spProtectedSession.p)
    {
        IFC( HandleEvent(spEvent) );
    }
    
    IFC( m_spProtectedSession->BeginGetEvent(&m_xOnProtectedSessionEvent, NULL) );

Cleanup:
    if(FAILED(hr)) 
    {
        m_pMediaEventHandler->NotifyEventError(hr);
    }
}

HRESULT CTedPlayer::HandleEvent(IMFMediaEvent * pEvent)
{
    HRESULT hr = S_OK;
    HRESULT hrEvent = S_OK;
    MediaEventType met;
    PROPVARIANT var;

    PropVariantInit(&var);
    
    IFC(pEvent->GetType(&met));
    IFC(pEvent->GetStatus(&hrEvent));
    IFC(pEvent->GetValue(&var));

    switch(met)
    {
        case MESessionStarted:
            IFC( HandleSessionStarted( pEvent ) );

            m_bIsPlaying = true;
            break;
        case MESessionEnded:
            m_bIsPlaying = false;

            if(m_fIsTranscoding)
            {
                m_spSession->Close();
            }

            break;
        case MESessionTopologySet:
            if(SUCCEEDED(hrEvent))
            {
                m_fTopologySet = true;
                m_bIsPaused = false;
            }
            
            m_spFullTopology.Release();
            var.punkVal->QueryInterface(IID_IMFTopology, (void**) &m_spFullTopology);
            break;
        case MESessionNotifyPresentationTime:
            IFC( HandleNotifyPresentationTime( pEvent ) );
            break;
    }

    m_pMediaEventHandler->HandleMediaEvent(pEvent);
    
Cleanup:
    PropVariantClear(&var);
    return hr;
}

HRESULT CTedPlayer::HandleSessionStarted(IMFMediaEvent* pEvent)
{
    MFTIME hnsTopologyPresentationOffset;

    if(SUCCEEDED( pEvent->GetUINT64(MF_EVENT_PRESENTATION_TIME_OFFSET, 
        (UINT64*)&hnsTopologyPresentationOffset) ))
    {
        m_hnsOffsetTime = hnsTopologyPresentationOffset;
    }

    return S_OK;
}

HRESULT CTedPlayer::HandleNotifyPresentationTime(IMFMediaEvent* pEvent) 
{
    HRESULT hr = S_OK;

    IFC( pEvent->GetUINT64(MF_EVENT_START_PRESENTATION_TIME, (UINT64*) &m_hnsStartTime) );
    IFC( pEvent->GetUINT64(MF_EVENT_PRESENTATION_TIME_OFFSET, (UINT64*) &m_hnsOffsetTime) );
    IFC( pEvent->GetUINT64(MF_EVENT_START_PRESENTATION_TIME_AT_OUTPUT, (UINT64*) &m_hnsStartTimeAtOutput) );
    m_fReceivedTime = true;

Cleanup:
    
    return( hr );
}

bool CTedPlayer::IsPlaying() const 
{
    return m_bIsPlaying;
}

bool CTedPlayer::IsPaused() const 
{
	return m_bIsPaused;
}

bool CTedPlayer::IsTopologySet() const
{
    return m_fTopologySet;
}

void CTedPlayer::SetCustomTopoloader(GUID gidTopoloader)
{
    m_gidCustomTopoloader = gidTopoloader;
    m_fPendingClearCustomTopoloader = true;
    m_fPendingProtectedCustomTopoloader = true;
}

HRESULT CTedPlayer::GetTime( MFTIME *phnsTime )
{
    HRESULT hr = S_OK;
    
    IFC( m_spSessionClock->GetTime( phnsTime ) );

    //
    // To get UI time, we need to apply the appropriate adjustment
    // to Presentation Clock time.
    //
    if (m_fReceivedTime)
    {
        *phnsTime -= m_hnsOffsetTime;
    }
    
    *phnsTime += m_hnsMediastartOffset;
    
Cleanup:
    return( hr );
}

HRESULT CTedPlayer::GetRateBounds(MFRATE_DIRECTION eDirection, float* pflSlowest, float* pflFastest)
{
    HRESULT hr = S_OK;
    CComPtr<IMFRateSupport> spRateSupport;

    assert(pflSlowest != NULL);
    assert(pflFastest != NULL);
    
    IFC( MFGetService( m_spSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateSupport, (void**) &spRateSupport ) );

    IFC( spRateSupport->GetSlowestRate( eDirection, FALSE, pflSlowest ) );
    IFC( spRateSupport->GetFastestRate( eDirection, FALSE, pflFastest ) );

Cleanup:
    return( hr );
}
   
HRESULT CTedPlayer::SetRate(float flRate)
{
    HRESULT hr = S_OK;
    CComPtr<IMFRateControl> spRateControl = NULL;
    
    IFC( MFGetService( m_spSession,
                                MF_RATE_CONTROL_SERVICE,
                                IID_IMFRateControl,
                                (void**) &spRateControl ));

    IFC( spRateControl->SetRate( FALSE, flRate ) );

Cleanup:
    return( hr );
}

HRESULT CTedPlayer::GetCapabilities(DWORD* pdwCaps)
{
    return m_spSession->GetSessionCapabilities(pdwCaps);   
}

HRESULT CTedPlayer::InitFromSession()
{
    HRESULT hr = S_OK;
    CComPtr<IMFClock> spClock;

    if(m_spSessionClock.p)
    {
        m_spSessionClock.Release();
    }
    
    IFC( Reset() );    
    IFC( m_spSession->GetClock(&spClock) );
    IFC( spClock->QueryInterface(IID_IMFPresentationClock, (void**) &m_spSessionClock) );

    m_bIsPaused = false;

Cleanup:
    return hr;
}

HRESULT CTedPlayer::RemoveResamplerNode(IMFTopology* pTopology)
{
    HRESULT hr = S_OK;
    
    WORD cNodes;
    IFC( pTopology->GetNodeCount(&cNodes) );
    
    for(WORD i = 0; i < cNodes; i++)
    {
        CComPtr<IMFTopologyNode> spNode;
        IFC( pTopology->GetNode(i, &spNode) );
        
        GUID gidTransformID;
        hr = spNode->GetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, &gidTransformID);
        
        if(SUCCEEDED(hr) && CLSID_CResamplerMediaObject == gidTransformID)
        {
            CComPtr<IMFTopologyNode> spUpstreamNode;
            DWORD dwUpstreamIndex;
            CComPtr<IMFTopologyNode> spDownstreamNode;
            DWORD dwDownstreamIndex;
            
            IFC( spNode->GetInput(0, &spUpstreamNode, &dwUpstreamIndex) );
            IFC( spNode->GetOutput(0, &spDownstreamNode, &dwDownstreamIndex) );
            IFC( spUpstreamNode->ConnectOutput(dwUpstreamIndex, spDownstreamNode, dwDownstreamIndex) );
            
            IFC( pTopology->RemoveNode(spNode) );
            IFC( pTopology->GetNodeCount(&cNodes) );
            
            i--;
        }
        
        hr = S_OK;
    }
    
Cleanup:
    return hr;
}
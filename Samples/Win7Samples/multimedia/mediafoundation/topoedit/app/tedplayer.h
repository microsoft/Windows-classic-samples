// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TEDPLAYER__
#define __TEDPLAYER__

#include "tedobj.h"
#include "tedutil.h"

#define METHODASYNCCALLBACKEX(Callback, Parent, Flag, Queue) \
class Callback##AsyncCallback; \
friend class Callback##AsyncCallback; \
class Callback##AsyncCallback : public IMFAsyncCallback \
{ \
public: \
    STDMETHOD_( ULONG, AddRef )() \
    { \
        Parent * pThis = ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
        return pThis->AddRef(); \
    } \
    STDMETHOD_( ULONG, Release )() \
    { \
        Parent * pThis = ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
        return pThis->Release(); \
    } \
    STDMETHOD( QueryInterface )( REFIID riid, void **ppvObject ) \
    { \
        if(riid == IID_IMFAsyncCallback || riid == IID_IUnknown) \
        { \
            (*ppvObject) = this; \
            AddRef(); \
            return S_OK; \
        } \
        (*ppvObject) = NULL; \
        return E_NOINTERFACE; \
    } \
    STDMETHOD( GetParameters )( \
        DWORD *pdwFlags, \
        DWORD *pdwQueue) \
    { \
        *pdwFlags = Flag; \
        *pdwQueue = Queue; \
        return S_OK; \
    } \
    STDMETHOD( Invoke )( IMFAsyncResult * pResult ) \
    { \
        Parent * pThis = ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
        pThis->Callback( pResult ); \
        return S_OK; \
    } \
} m_x##Callback; 

////////////////////////////////////////////////////////
//
   
#define METHODASYNCCALLBACK(Callback, Parent) \
    METHODASYNCCALLBACKEX(Callback, Parent, 0, MFASYNC_CALLBACK_QUEUE_STANDARD)

    
///////////////////////////////////////////////////////////////////////////////
// window for video playback
class CTedVideoWindow
    : public CWindowImpl<CTedVideoWindow>
{
public:
    CTedVideoWindow();
    ~CTedVideoWindow();

    void Init(CTedApp * pApp);
    
protected:    
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    BEGIN_MSG_MAP(CTedVideoWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// Renders a media file to a full topology
class CTedMediaFileRenderer
{
public:
    CTedMediaFileRenderer(LPCWSTR szFileName, ITedVideoWindowHandler* pVideoWindowHandler, HRESULT& hr);
    
    HRESULT Load(IMFTopology** ppOutputTopology);
    
protected:
    HRESULT CreatePartialTopology(IMFTopology** ppPartialTopology);
    HRESULT CreateSource(IMFMediaSource** ppSource);
    HRESULT BuildTopologyFromSource(IMFTopology* pTopology, IMFMediaSource* pSource);
    HRESULT CreateRendererForStream(IMFStreamDescriptor* pSD, IMFTopologyNode** ppRendererNode);
    
private:
    LPCWSTR m_szFileName;
    CComPtr<ITedVideoWindowHandler> m_spVideoWindowHandler;
    CComPtr<IMFTopoLoader> m_spTopoLoader;
};

///////////////////////////////////////////////////////////////////////////////
// Manages playback of a topology

class CTedMediaEventHandler
{
public:
    virtual void NotifyEventError(HRESULT hr) = 0;
    virtual void HandleMediaEvent(IMFMediaEvent* pEvent) = 0;
};

class CTedPlayer
{
public:
    CTedPlayer(CTedMediaEventHandler* pMediaEventHandler, IMFContentProtectionManager* pCPM);
    ~CTedPlayer();

    HRESULT InitClear();
    HRESULT InitProtected();

    // for callbacks
    LONG AddRef();
    LONG Release();

    HRESULT Reset();    

    HRESULT GetFullTopology(IMFTopology ** ppFullTopo);
    HRESULT GetPartialTopology(IMFTopology ** ppPartialTopo);

    HRESULT SetTopology(CComPtr<IMFTopology> pPartialTopo, BOOL fTranscode);
    
    HRESULT Start();
    HRESULT Stop();
    HRESULT Pause();
    HRESULT PlayFrom(MFTIME time);

    HRESULT GetDuration(MFTIME& hnsTime);
    HRESULT GetTime( MFTIME *phnsTime );

    HRESULT GetRateBounds(MFRATE_DIRECTION eDirection, float* pflSlowest, float* pflFastest);
    HRESULT SetRate(float flRate);

    bool IsPlaying() const;
    bool IsPaused() const;
    bool IsTopologySet() const;
    
    void SetCustomTopoloader(GUID gidTopoloader);
    
    HRESULT GetCapabilities(DWORD* pdwCaps);

protected:
    METHODASYNCCALLBACK(OnClearSessionEvent, CTedPlayer);
    METHODASYNCCALLBACK(OnProtectedSessionEvent, CTedPlayer);
    void OnClearSessionEvent(IMFAsyncResult* pResult);
    void OnProtectedSessionEvent(IMFAsyncResult* pResult);

    HRESULT HandleEvent(IMFMediaEvent* pEvent);
    HRESULT HandleSessionStarted(IMFMediaEvent* pEvent);
    HRESULT HandleNotifyPresentationTime(IMFMediaEvent* pEvent);
    
    HRESULT InitFromSession();
    HRESULT RemoveResamplerNode(IMFTopology* pTopology);

private:
    LONG m_cRef;

    CTedApp * m_pApp;
    CTedMediaEventHandler* m_pMediaEventHandler;
    
    CComPtr<IMFMediaSession> m_spSession;
    CComPtr<IMFMediaSession> m_spClearSession;
    CComPtr<IMFMediaSession> m_spProtectedSession;
    CComPtr<IMFMediaSource> m_spSource;
    CAtlArray<IMFTopology*> m_aTopologies;
    IMFContentProtectionManager* m_pCPM;
    
    CComPtr<IMFTopology> m_spFullTopology;
    CComPtr<IMFPresentationClock> m_spSessionClock;

    // sequencer for multiple sources and using the same source in two consecutive topologies
    MFSequencerElementId m_LastSeqID;
    
    bool m_fReceivedTime;
    MFTIME m_hnsStartTime;
    MFTIME m_hnsOffsetTime;
    MFTIME m_hnsStartTimeAtOutput;
    MFTIME m_hnsDuration;
    MFTIME m_hnsMediastartOffset;
    
    bool m_bIsPlaying;
    bool m_bIsPaused;
    bool m_fTopologySet;
    BOOL m_fIsTranscoding;
    
    GUID m_gidCustomTopoloader;
    bool m_fPendingClearCustomTopoloader;
    bool m_fPendingProtectedCustomTopoloader;
};

#endif


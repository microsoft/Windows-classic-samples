#include "stdafx.h"

#include "tedtestsink.h"

#include "mferror.h"

#include "Logger.h"

///////////////////////////////////
// CTedTestSink

CTedTestSink::CTedTestSink()
    : m_fShutdown(false)
//    , m_cRef(0)
{
    InitAudioStream();
    InitVideoStream();
}

CTedTestSink::~CTedTestSink()
{
}

HRESULT CTedTestSink::AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType* pMediaType, IMFStreamSink** ppStreamSink)
{
    return MF_E_STREAMSINKS_FIXED;
}

HRESULT CTedTestSink::GetCharacteristics(DWORD* pdwCharacteristics)
{
    if(NULL == pdwCharacteristics)
    {
        return E_POINTER;
    }

    *pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;

    return S_OK;
}

HRESULT CTedTestSink::GetPresentationClock(IMFPresentationClock** ppPresentationClock)
{
    if(NULL == ppPresentationClock)
    {
        return E_POINTER;
    }

    if(m_fShutdown)
    {
        return MF_E_SHUTDOWN;
    }
    
    if(NULL == m_spPresentationClock.p)
    {
        return MF_E_NO_CLOCK;
    }

    *ppPresentationClock = m_spPresentationClock;
    (*ppPresentationClock)->AddRef();

    return S_OK;
}

HRESULT CTedTestSink::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink)
{
    if(NULL == ppStreamSink)
    {
        return E_POINTER;
    }

    if(m_fShutdown)
    {
        return MF_E_SHUTDOWN;
    }

    if(dwStreamSinkIdentifier > 1)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if(0 == dwStreamSinkIdentifier)
    {
        *ppStreamSink = m_spAudioStreamSink;
    }
    else
    {
        *ppStreamSink = m_spVideoStreamSink;
    }

    (*ppStreamSink)->AddRef();

    return S_OK;
}

HRESULT CTedTestSink::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink)
{
    if(NULL == ppStreamSink)
    {
        return E_POINTER;
    }

    if(m_fShutdown)
    {
        return MF_E_SHUTDOWN;
    }

    if(dwIndex > 1)
    {
        return MF_E_INVALIDINDEX;
    }

    if(0 == dwIndex)
    {
        *ppStreamSink = m_spAudioStreamSink;
    }
    else
    {
        *ppStreamSink = m_spVideoStreamSink;
    }

    (*ppStreamSink)->AddRef();

    return S_OK;
}

HRESULT CTedTestSink::GetStreamSinkCount(DWORD* pcStreamSinkCount)
{
    if(NULL == pcStreamSinkCount)
    {
        return E_POINTER;
    }

    if(m_fShutdown)
    {
        return MF_E_SHUTDOWN;
    }

    *pcStreamSinkCount = 2;

    return S_OK;
}

HRESULT CTedTestSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
    return MF_E_STREAMSINKS_FIXED;
}

HRESULT CTedTestSink::SetPresentationClock(IMFPresentationClock* pPresentationClock)
{
    if(m_fShutdown)
    {
        return MF_E_SHUTDOWN;
    }

    m_spPresentationClock = pPresentationClock;

    return S_OK;
}

HRESULT CTedTestSink::Shutdown()
{
    m_fShutdown = true;

    return S_OK;
}

HRESULT CTedTestSink::InitAudioStream()
{
    HRESULT hr = S_OK;

    InternalAddRef();
    /*IMFMediaSink* pMediaSink;
    InternalQueryInterface(this, CTedTestSink::_GetEntries(), IID_IMFMediaSink, (void**) &pMediaSink);*/
    
    CComPtr<IMFMediaType> spAudioType;
    IFC( MFCreateMediaType(&spAudioType) );
    IFC( spAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio) );
    IFC( spAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float) );

    CComObject<CTedTestMediaTypeHandler>* pAudioTypeHandler = NULL;
    IFC( CComObject<CTedTestMediaTypeHandler>::CreateInstance(&pAudioTypeHandler) );
    pAudioTypeHandler->AddRef();
    pAudioTypeHandler->SetMajorType(MFMediaType_Audio);
    pAudioTypeHandler->AddAvailableType(spAudioType);

    CComObject<CTedTestStreamSink>* pAudioStreamSink = NULL;
    IFC( CComObject<CTedTestStreamSink>::CreateInstance(&pAudioStreamSink) );
    pAudioStreamSink->AddRef();
    m_spAudioStreamSink = pAudioStreamSink;
    //IFC( pAudioStreamSink->Init(pMediaSink, pAudioTypeHandler, 0) );
    IFC( pAudioStreamSink->Init(this, pAudioTypeHandler, 0) );
    pAudioTypeHandler->Release();
    pAudioStreamSink->Release();

    //pMediaSink->Release();
    InternalRelease();
Cleanup:
    return hr;
}

HRESULT CTedTestSink::InitVideoStream()
{
    HRESULT hr = S_OK;

    InternalAddRef();
    /*IMFMediaSink* pMediaSink;
    InternalQueryInterface(this, CTedTestSink::_GetEntries(), IID_IMFMediaSink, (void**) &pMediaSink);*/
    
    CComPtr<IMFMediaType> spVideoType;
    IFC( MFCreateMediaType(&spVideoType) );
    IFC( spVideoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video) );
    IFC( spVideoType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24) );

    CComObject<CTedTestMediaTypeHandler>* pVideoTypeHandler = NULL;
    IFC( CComObject<CTedTestMediaTypeHandler>::CreateInstance(&pVideoTypeHandler) );
    pVideoTypeHandler->AddRef();
    pVideoTypeHandler->SetMajorType(MFMediaType_Video);
    pVideoTypeHandler->AddAvailableType(spVideoType);

    CComObject<CTedTestStreamSink>* pVideoStreamSink = NULL;
    IFC( CComObject<CTedTestStreamSink>::CreateInstance(&pVideoStreamSink) );
    pVideoStreamSink->AddRef();
    m_spVideoStreamSink = pVideoStreamSink;
    //IFC( pVideoStreamSink->Init(pMediaSink, pVideoTypeHandler, 1) );
    IFC( pVideoStreamSink->Init(this, pVideoTypeHandler, 1) );
    pVideoTypeHandler->Release();
    pVideoStreamSink->Release();

    //pMediaSink->Release();
Cleanup:
    return hr;
}
/*
HRESULT CTedTestSink::QueryInterface(REFIID riid, void** ppInterface)
{
    if(NULL == ppInterface)
    {
        return E_POINTER;
    }

    if(IID_IUnknown == riid || IID_IMFMediaSink == riid || IID_IMFMediaEventGenerator == riid)
    {
        *ppInterface = this;
        AddRef();
        return S_OK;
    }

    *ppInterface = NULL;
    return E_NOINTERFACE;
}

ULONG CTedTestSink::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    return cRef;
}

ULONG CTedTestSink::Release()
{
    LONG cRef = ::InterlockedDecrement(&m_cRef);

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}
*/
///////////////////////////////////////////////
// CTedTestStreamSink

CTedTestStreamSink::CTedTestStreamSink()
    : m_pSink(NULL)
    , m_pMTH(NULL)
{
    MFCreateEventQueue(&m_spMEQ);
}

CTedTestStreamSink::~CTedTestStreamSink()
{
    //if(m_pSink) m_pSink->Release();
    if(m_pMTH) m_pMTH->Release();
}

HRESULT CTedTestStreamSink::Init(CTedTestSink* pSink, IMFMediaTypeHandler* pMTH, DWORD dwIdentifier)
{
    m_pSink = pSink;
    m_pSink->InternalAddRef();

    m_pMTH = pMTH;
    m_pMTH->AddRef();
    
    m_dwIdentifier = dwIdentifier;

    return S_OK;
}

HRESULT CTedTestStreamSink::Flush()
{
    return S_OK;
}

HRESULT CTedTestStreamSink::GetIdentifier(DWORD* pdwIdentifier)
{
    if(NULL == pdwIdentifier)
    {
        return E_POINTER;
    }

    *pdwIdentifier = m_dwIdentifier;

    return S_OK;
}

HRESULT CTedTestStreamSink::GetMediaSink(IMFMediaSink** ppMediaSink)
{
    if(NULL == ppMediaSink)
    {
        return E_POINTER;
    }

    //m_pSink->QueryInterface(IID_IMFMediaSink, (void**) ppMediaSink);
    
    *ppMediaSink = m_pSink;
    (*ppMediaSink)->AddRef();

    return S_OK;
}

HRESULT CTedTestStreamSink::GetMediaTypeHandler(IMFMediaTypeHandler** ppMediaTypeHandler)
{
    if(NULL == ppMediaTypeHandler)
    {
        return E_POINTER;
    }

    *ppMediaTypeHandler= m_pMTH;
    (*ppMediaTypeHandler)->AddRef();

    return S_OK;
}

HRESULT CTedTestStreamSink::PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT* pvarMarkerValue, const PROPVARIANT* pvarContextValue)
{
    return m_spMEQ->QueueEventParamVar(MEStreamSinkMarker, GUID_NULL, S_OK, pvarContextValue);
}

HRESULT CTedTestStreamSink::ProcessSample(IMFSample* pSample)
{
    return S_OK;
}

STDMETHODIMP CTedTestStreamSink::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    return m_spMEQ->BeginGetEvent(pCallback, punkState);    
}

STDMETHODIMP CTedTestStreamSink::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    return m_spMEQ->EndGetEvent(pResult, ppEvent);
}

STDMETHODIMP CTedTestStreamSink::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    return m_spMEQ->GetEvent(dwFlags, ppEvent);
}

STDMETHODIMP CTedTestStreamSink::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    return m_spMEQ->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

//////////////////////////////////
// CTedTestMediaTypeHandler

CTedTestMediaTypeHandler::CTedTestMediaTypeHandler()
{
}

CTedTestMediaTypeHandler::~CTedTestMediaTypeHandler()
{
    for(size_t i = 0; i < m_arrAvailableTypes.GetCount(); i++)
    {
        m_arrAvailableTypes.GetAt(i)->Release();
    }
}

HRESULT CTedTestMediaTypeHandler::GetCurrentMediaType(IMFMediaType** ppType)
{
    if(NULL == ppType)
    {
        return E_POINTER;
    }

    if(NULL == m_spCurrentType.p)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    *ppType = m_spCurrentType;
    (*ppType)->AddRef();

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::GetMajorType(GUID* pguidMajorType)
{
    if(NULL == pguidMajorType)
    {
        return E_POINTER;
    }

    *pguidMajorType = m_gidMajorType;

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType** ppType)
{
    if(NULL == ppType)
    {
        return E_POINTER;
    }

    if(dwIndex >= m_arrAvailableTypes.GetCount())
    {
        return MF_E_NO_MORE_TYPES;
    }

    *ppType = m_arrAvailableTypes.GetAt(dwIndex);
    (*ppType)->AddRef();

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::GetMediaTypeCount(DWORD* pdwTypeCount)
{
    if(NULL == pdwTypeCount)
    {
        return E_POINTER;
    }    

    *pdwTypeCount = (DWORD) m_arrAvailableTypes.GetCount();

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::IsMediaTypeSupported(IMFMediaType* pMediaType, IMFMediaType** ppMediaType)
{
    if(NULL != ppMediaType)
    {
        *ppMediaType = NULL;
    }
    
    for(size_t i = 0; i < m_arrAvailableTypes.GetCount(); i++)
    {
        DWORD dwFlags;
        pMediaType->IsEqual(m_arrAvailableTypes.GetAt(i), &dwFlags);

        if(dwFlags & MF_MEDIATYPE_EQUAL_MAJOR_TYPES && dwFlags & MF_MEDIATYPE_EQUAL_FORMAT_TYPES)
        {
            return S_OK;
        }
    }

    return MF_E_INVALIDMEDIATYPE;
    
}

HRESULT CTedTestMediaTypeHandler::SetCurrentMediaType(IMFMediaType* pMediaType)
{
    if(NULL == pMediaType)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    if(FAILED(IsMediaTypeSupported(pMediaType, NULL)))
    {
        return hr;
    }

    m_spCurrentType = pMediaType;

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::SetMajorType(GUID gidMajorType)
{
    m_gidMajorType = gidMajorType;

    return S_OK;
}

HRESULT CTedTestMediaTypeHandler::AddAvailableType(IMFMediaType* pType)
{
    pType->AddRef();
    
    m_arrAvailableTypes.Add(pType);

    return S_OK;
}
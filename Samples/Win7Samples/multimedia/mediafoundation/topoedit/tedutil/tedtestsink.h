#include "tedutil.h"
#include "resource.h"

class CTedTestSink
    : public CComObjectRoot
    , public CComCoClass<CTedTestSink, &CLSID_CTedTestSink>
    , public IMFMediaSink
{
public:
    BEGIN_COM_MAP(CTedTestSink)
        COM_INTERFACE_ENTRY(IMFMediaSink)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_TEDTESTSINK);
    DECLARE_NOT_AGGREGATABLE(CTedTestSink);
    DECLARE_CLASSFACTORY();
    
    CTedTestSink();
    virtual ~CTedTestSink();
    
    STDMETHODIMP AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType* pMediaType, IMFStreamSink** ppStreamSink);
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacterisitics);
    STDMETHODIMP GetPresentationClock(IMFPresentationClock** ppPresentationClock);
    STDMETHODIMP GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink);
    STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink);
    STDMETHODIMP GetStreamSinkCount(DWORD* pcStreamSinkCount);
    STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
    STDMETHODIMP SetPresentationClock(IMFPresentationClock* pPresentationClock);
    STDMETHODIMP Shutdown();

    /*STDMETHODIMP QueryInterface(REFIID riid, void** ppInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();*/
    
protected:
    STDMETHODIMP InitAudioStream();
    STDMETHODIMP InitVideoStream();
        
private:
    CComPtr<IMFPresentationClock> m_spPresentationClock;
    CComPtr<IMFStreamSink> m_spAudioStreamSink;
    CComPtr<IMFStreamSink> m_spVideoStreamSink;
    bool m_fShutdown;
    //LONG m_cRef;
};

class CTedTestStreamSink
    : public CComObjectRoot
    , public IMFStreamSink
{
public:
    CTedTestStreamSink();
    virtual ~CTedTestStreamSink();

    DECLARE_NOT_AGGREGATABLE(CTedTestStreamSink);
    
    BEGIN_COM_MAP(CTedTestStreamSink)
        COM_INTERFACE_ENTRY(IMFStreamSink)
        COM_INTERFACE_ENTRY(IMFMediaEventGenerator)
    END_COM_MAP()

    STDMETHODIMP Init(CTedTestSink* pSink, IMFMediaTypeHandler* pMTH, DWORD dwIdentifier);
    
    STDMETHODIMP Flush();
    STDMETHODIMP GetIdentifier(DWORD* pdwIdentifier);
    STDMETHODIMP GetMediaSink(IMFMediaSink** ppMediaSink);
    STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler);
    STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT* pvarMarkerValue, const PROPVARIANT* pvarContextValue);
    STDMETHODIMP ProcessSample(IMFSample* pSample);

    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

private:
    CTedTestSink* m_pSink;
    CComPtr<IMFMediaEventQueue> m_spMEQ;
    IMFMediaTypeHandler* m_pMTH;
    DWORD m_dwIdentifier;
};

class CTedTestMediaTypeHandler
    : public CComObjectRoot
    , public IMFMediaTypeHandler
{
public:
    CTedTestMediaTypeHandler();
    virtual ~CTedTestMediaTypeHandler();
    
    BEGIN_COM_MAP(CTedTestMediaTypeHandler)
        COM_INTERFACE_ENTRY(IMFMediaTypeHandler)
    END_COM_MAP()
    
    STDMETHODIMP GetCurrentMediaType(IMFMediaType** ppType);
    STDMETHODIMP GetMajorType(GUID* pguidMajorType);
    STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType** ppType);
    STDMETHODIMP GetMediaTypeCount(DWORD* pdwTypeCount);
    STDMETHODIMP IsMediaTypeSupported(IMFMediaType* pMediaType, IMFMediaType** ppMediaType);
    STDMETHODIMP SetCurrentMediaType(IMFMediaType* pMediaType);

    STDMETHODIMP AddAvailableType(IMFMediaType* pType);
    STDMETHODIMP SetMajorType(GUID gidMajorType);
    
private:
    CComPtr<IMFMediaType> m_spCurrentType;
    CAtlArray<IMFMediaType*> m_arrAvailableTypes;

    GUID m_gidMajorType;
};

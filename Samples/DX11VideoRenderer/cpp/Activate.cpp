#include "Activate.h"

HRESULT DX11VideoRenderer::CActivate::CreateInstance(HWND hwnd, IMFActivate** ppActivate)
{
    if (ppActivate == NULL)
    {
        return E_POINTER;
    }

    CActivate* pActivate = new CActivate();
    if (pActivate == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pActivate->AddRef();

    HRESULT hr = S_OK;

    do
    {
        hr = pActivate->Initialize();
        if (FAILED(hr))
        {
            break;
        }

        hr = pActivate->QueryInterface(IID_PPV_ARGS(ppActivate));
        if (FAILED(hr))
        {
            break;
        }

        pActivate->m_hwnd = hwnd;
    }
    while (FALSE);

    SafeRelease(pActivate);

    return hr;
}

// IUnknown
ULONG DX11VideoRenderer::CActivate::AddRef(void)
{
    return InterlockedIncrement(&m_lRefCount);
}

// IUnknown
HRESULT DX11VideoRenderer::CActivate::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFActivate*>(this));
    }
    else if (iid == __uuidof(IMFActivate))
    {
        *ppv = static_cast<IMFActivate*>(this);
    }
    else if (iid == __uuidof(IPersistStream))
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == __uuidof(IPersist))
    {
        *ppv = static_cast<IPersist*>(this);
    }
    else if (iid == __uuidof(IMFAttributes))
    {
        *ppv = static_cast<IMFAttributes*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// IUnknown
ULONG DX11VideoRenderer::CActivate::Release(void)
{
    ULONG lRefCount = InterlockedDecrement(&m_lRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return lRefCount;
}

// IMFActivate
HRESULT DX11VideoRenderer::CActivate::ActivateObject(__RPC__in REFIID riid, __RPC__deref_out_opt void** ppvObject)
{
    HRESULT hr = S_OK;
    IMFGetService* pSinkGetService = NULL;
    IMFVideoDisplayControl* pSinkVideoDisplayControl = NULL;

    do
    {
        if (m_pMediaSink == NULL)
        {
            hr = CMediaSink::CreateInstance(IID_PPV_ARGS(&m_pMediaSink));
            if (FAILED(hr))
            {
                break;
            }

            hr = m_pMediaSink->QueryInterface(IID_PPV_ARGS(&pSinkGetService));
            if (FAILED(hr))
            {
                break;
            }

            hr = pSinkGetService->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pSinkVideoDisplayControl));
            if (FAILED(hr))
            {
                break;
            }

            hr = pSinkVideoDisplayControl->SetVideoWindow(m_hwnd);
            if (FAILED(hr))
            {
                break;
            }
        }

        hr = m_pMediaSink->QueryInterface(riid, ppvObject);
        if (FAILED(hr))
        {
            break;
        }
    }
    while (FALSE);

    SafeRelease(pSinkGetService);
    SafeRelease(pSinkVideoDisplayControl);

    return hr;
}

// IMFActivate
HRESULT DX11VideoRenderer::CActivate::DetachObject(void)
{
    SafeRelease(m_pMediaSink);

    return S_OK;
}

// IMFActivate
HRESULT DX11VideoRenderer::CActivate::ShutdownObject(void)
{
    if (m_pMediaSink != NULL)
    {
        m_pMediaSink->Shutdown();
        SafeRelease(m_pMediaSink);
    }

    return S_OK;
}

// IPersistStream
HRESULT DX11VideoRenderer::CActivate::GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize)
{
    return E_NOTIMPL;
}

// IPersistStream
HRESULT DX11VideoRenderer::CActivate::IsDirty(void)
{
    return E_NOTIMPL;
}

// IPersistStream
HRESULT DX11VideoRenderer::CActivate::Load(__RPC__in_opt IStream* pStream)
{
    return E_NOTIMPL;
}

// IPersistStream
HRESULT DX11VideoRenderer::CActivate::Save(__RPC__in_opt IStream* pStream, BOOL bClearDirty)
{
    return E_NOTIMPL;
}

// IPersist
HRESULT DX11VideoRenderer::CActivate::GetClassID(__RPC__out CLSID* pClassID)
{
    if (pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_DX11VideoRendererActivate;
    return S_OK;
}

// ctor
DX11VideoRenderer::CActivate::CActivate(void) :
    m_lRefCount(0),
    m_pMediaSink(NULL),
    m_hwnd(NULL)
{
}

// dtor
DX11VideoRenderer::CActivate::~CActivate(void)
{
    SafeRelease(m_pMediaSink);
}

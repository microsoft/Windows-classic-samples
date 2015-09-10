#include "ClassFactory.h"

BOOL DX11VideoRenderer::CClassFactory::IsLocked(void)
{
    return (s_lLockCount == 0) ? FALSE : TRUE;
}

DX11VideoRenderer::CClassFactory::CClassFactory(void) :
    m_lRefCount(0)
{
}

DX11VideoRenderer::CClassFactory::~CClassFactory(void)
{
}

// IUnknown
ULONG DX11VideoRenderer::CClassFactory::AddRef(void)
{
    return InterlockedIncrement(&m_lRefCount);
}

// IUnknown
HRESULT DX11VideoRenderer::CClassFactory::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IClassFactory*>(this));
    }
    else if (iid == __uuidof(IClassFactory))
    {
        *ppv = static_cast<IClassFactory*>(this);
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
ULONG DX11VideoRenderer::CClassFactory::Release(void)
{
    ULONG lRefCount = InterlockedDecrement(&m_lRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return lRefCount;
}

// IClassFactory
HRESULT DX11VideoRenderer::CClassFactory::CreateInstance(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObject)
{
    if (ppvObject == NULL)
    {
        return E_POINTER;
    }

    *ppvObject = NULL;

    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    return CMediaSink::CreateInstance(riid, ppvObject);
}

// IClassFactory
HRESULT DX11VideoRenderer::CClassFactory::LockServer(BOOL bLock)
{
    if (bLock == FALSE)
    {
        InterlockedDecrement(&s_lLockCount);
    }
    else
    {
        InterlockedIncrement(&s_lLockCount);
    }
    return S_OK;
}

#pragma once

#include "Common.h"
#include "MediaSink.h"

namespace DX11VideoRenderer
{
    class CClassFactory : public IClassFactory, private CBase
    {
    public:

        static BOOL IsLocked(void);

        CClassFactory(void);
        ~CClassFactory(void);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP QueryInterface(REFIID riid, __RPC__deref_out _Result_nullonfailure_ void** ppvObject);
        STDMETHODIMP_(ULONG) Release(void);

        // IClassFactory
        STDMETHODIMP CreateInstance(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ppvObject);
        STDMETHODIMP LockServer(BOOL bLock);

    private:

        static volatile long s_lLockCount;

        long m_lRefCount;
    };
}

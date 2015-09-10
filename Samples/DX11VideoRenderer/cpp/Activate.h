#pragma once

#include "Common.h"
#include "MFAttributesImpl.h"
#include "MediaSink.h"

namespace DX11VideoRenderer
{
    class CActivate : public CMFAttributesImpl<IMFActivate>, public IPersistStream, private CBase
    {
    public:

        static HRESULT CreateInstance(HWND hwnd, IMFActivate** ppActivate);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP QueryInterface(REFIID riid, __RPC__deref_out _Result_nullonfailure_ void** ppvObject);
        STDMETHODIMP_(ULONG) Release(void);

        // IMFActivate
        STDMETHODIMP ActivateObject(__RPC__in REFIID riid, __RPC__deref_out_opt void** ppvObject);
        STDMETHODIMP DetachObject(void);
        STDMETHODIMP ShutdownObject(void);

        // IPersistStream
        STDMETHODIMP GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize);
        STDMETHODIMP IsDirty(void);
        STDMETHODIMP Load(__RPC__in_opt IStream* pStream);
        STDMETHODIMP Save(__RPC__in_opt IStream* pStream, BOOL bClearDirty);

        // IPersist (from IPersistStream)
        STDMETHODIMP GetClassID(__RPC__out CLSID* pClassID);

    private:

        CActivate(void);
        ~CActivate(void);

        long m_lRefCount;
        IMFMediaSink* m_pMediaSink;
        HWND m_hwnd;
    };
}

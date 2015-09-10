#pragma once

#include "Common.h"
#include "Scheduler.h"
#include "display.h"

namespace DX11VideoRenderer
{
    // IMarker:
    // Custom interface for handling IMFStreamSink::PlaceMarker calls asynchronously.

    // A marker consists of a marker type, marker data, and context data.
    // By defining this interface, we can store the marker data inside an IUnknown object
    // and keep that object on the same queue that holds the media samples. This is
    // useful because samples and markers must be serialized. That is, we cannot respond
    // to a marker until we have processed all of the samples that came before it.

    // Note that IMarker is not a standard Media Foundation interface.
    MIDL_INTERFACE("3AC82233-933C-43a9-AF3D-ADC94EABF406")
    IMarker : public IUnknown
    {
        virtual STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE* pType) = 0;
        virtual STDMETHODIMP GetMarkerValue(PROPVARIANT* pvar) = 0;
        virtual STDMETHODIMP GetContext(PROPVARIANT* pvar) = 0;
    };

    // Holds marker information for IMFStreamSink::PlaceMarker

    class CMarker : public IMarker, private CBase
    {
    public:

        static HRESULT Create(
            MFSTREAMSINK_MARKER_TYPE eMarkerType,
            const PROPVARIANT* pvarMarkerValue,
            const PROPVARIANT* pvarContextValue,
            IMarker** ppMarker
            );

        // IUnknown methods.
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);

        STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE* pType);
        STDMETHODIMP GetMarkerValue(PROPVARIANT* pvar);
        STDMETHODIMP GetContext(PROPVARIANT* pvar);

    protected:

        MFSTREAMSINK_MARKER_TYPE m_eMarkerType;
        PROPVARIANT m_varMarkerValue;
        PROPVARIANT m_varContextValue;

    private:

        CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType);
        virtual ~CMarker(void);

        long m_nRefCount;
    };
}

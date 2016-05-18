//------------------------------------------------------------------------------
// File: EZRGB24.h
//
// Desc: DirectShow sample code - special effects filter header file.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


class CEZrgb24 : public CTransformFilter,
         public IIPEffect,
         public ISpecifyPropertyPages,
         public CPersistStream
{

public:

    DECLARE_IUNKNOWN;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Reveals IEZrgb24 and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // CPersistStream stuff
    HRESULT ScribbleToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);

    // Overrriden from CTransformFilter base class

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    // These implement the custom IIPEffect interface

    STDMETHODIMP get_IPEffect(int *IPEffect, REFTIME *StartTime, REFTIME *Length);
    STDMETHODIMP put_IPEffect(int IPEffect, REFTIME StartTime, REFTIME Length);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

    // CPersistStream override
    STDMETHODIMP GetClassID(CLSID *pClsid);

private:

    // Constructor
    CEZrgb24(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Look after doing the special effect
    BOOL CanPerformEZrgb24(const CMediaType *pMediaType) const;
    HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;
    HRESULT Transform(IMediaSample *pMediaSample);

    CCritSec    m_EZrgb24Lock;          // Private play critical section
    int         m_effect;               // Which effect are we processing
    CRefTime    m_effectStartTime;      // When the effect will begin
    CRefTime    m_effectTime;           // And how long it will last for
    const long m_lBufferRequest;        // The number of buffers to use

}; // EZrgb24


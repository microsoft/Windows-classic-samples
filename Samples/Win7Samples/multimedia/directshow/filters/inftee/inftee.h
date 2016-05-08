//------------------------------------------------------------------------------
// File: InfTee.h
//
// Desc: DirectShow sample code - header file for infinite tee filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __INFTEE__
#define __INFTEE__

// define a GUID for infinite tee filters

// { 022B8142-0946-11cf-BCB1-444553540000 }
DEFINE_GUID(CLSID_Tee,
0x22b8142, 0x946, 0x11cf, 0xbc, 0xb1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

class CTee;
class CTeeOutputPin;

// class for the Tee filter's Input pin

class CTeeInputPin : public CBaseInputPin
{
    friend class CTeeOutputPin;
    CTee *m_pTee;                  // Main filter object
    BOOL m_bInsideCheckMediaType;  // Re-entrancy control

public:

    // Constructor and destructor
    CTeeInputPin(TCHAR *pObjName,
                 CTee *pTee,
                 HRESULT *phr,
                 LPCWSTR pPinName);

#ifdef DEBUG
    ~CTeeInputPin();
#endif

    // Used to check the input pin connection
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();

    // Reconnect outputs if necessary at end of completion
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

    // Pass through calls downstream
    STDMETHODIMP EndOfStream();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

    // Handles the next block of data from the stream
    STDMETHODIMP Receive(IMediaSample *pSample);

};


// Class for the Tee filter's Output pins.

class CTeeOutputPin : public CBaseOutputPin
{
    friend class CTeeInputPin;
    friend class CTee;

    CTee *m_pTee;                  // Main filter object pointer
    IUnknown    *m_pPosition;      // Pass seek calls upstream
    BOOL m_bHoldsSeek;             // Is this the one seekable stream

    COutputQueue *m_pOutputQueue;  // Streams data to the peer pin
    BOOL m_bInsideCheckMediaType;  // Re-entrancy control
    LONG m_cOurRef;                // We maintain reference counting

public:

    // Constructor and destructor

    CTeeOutputPin(TCHAR *pObjName,
                   CTee *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName,
                   INT PinNumber);

#ifdef DEBUG
    ~CTeeOutputPin();
#endif

    // Override to expose IMediaPosition
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppvoid);

    // Override since the life time of pins and filters are not the same
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

    // Override to enumerate media types
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);

    // Check that we can support an output type
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);

    // Negotiation to use our input pins allocator
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
                             ALLOCATOR_PROPERTIES * ppropInputRequest);

    // Used to create output queue objects
    HRESULT Active();
    HRESULT Inactive();

    // Overriden to create and destroy output pins
    HRESULT CompleteConnect(IPin *pReceivePin);

    // Overriden to pass data to the output queues
    HRESULT Deliver(IMediaSample *pMediaSample);
    HRESULT DeliverEndOfStream();
    HRESULT DeliverBeginFlush();
    HRESULT DeliverEndFlush();
    HRESULT DeliverNewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);


    // Overriden to handle quality messages
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);
};


// Class for the Tee filter

class CTee: public CCritSec, public CBaseFilter
{
    // Let the pins access our internal state
    friend class CTeeInputPin;
    friend class CTeeOutputPin;
    typedef CGenericList <CTeeOutputPin> COutputList;

    // Declare an input pin.
    CTeeInputPin m_Input;

    INT m_NumOutputPins;            // Current output pin count
    COutputList m_OutputPinsList;   // List of the output pins
    INT m_NextOutputPinNumber;      // Increases monotonically.
    LONG m_lCanSeek;                // Seekable output pin
    IMemAllocator *m_pAllocator;    // Allocator from our input pin

public:

    CTee(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CTee();

    CBasePin *GetPin(int n);
    int GetPinCount();

    // Function needed for the class factory
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    // Send EndOfStream if no input connection
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

protected:

    // The following manage the list of output pins

    void InitOutputPinsList();
    CTeeOutputPin *GetPinNFromList(int n);
    CTeeOutputPin *CreateNextOutputPin(CTee *pTee);
    void DeleteOutputPin(CTeeOutputPin *pPin);
    int GetNumFreePins();
};

#endif // __INFTEE__


//------------------------------------------------------------------------------
// File: DynSrc.h
//
// Desc: DirectShow sample code - defines classes to simplify creation of
//       ActiveX source filters that support continuous generation of data.
//       No support is provided for IMediaControl or IMediaPosition.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Derive your source filter from CDynamicSource.
// During construction either:
//    Create some CDynamicSourceStream objects to manage your pins
//    Provide the user with a means of doing so eg, an IPersistFile interface.
//
// CDynamicSource provides:
//    IBaseFilter interface management
//    IMediaFilter interface management, via CBaseFilter
//    Pin counting for CBaseFilter
//
// Derive a class from CDynamicSourceStream to manage your output pin types
//  Implement GetMediaType/1 to return the type you support. If you support multiple
//   types then overide GetMediaType/3, CheckMediaType and GetMediaTypeCount.
//  Implement Fillbuffer() to put data into one buffer.
//
// CDynamicSourceStream provides:
//    IPin management via CDynamicOutputPin
//    Worker thread management

#ifndef __CDYNAMICSOURCE__
#define __CDYNAMICSOURCE__

class CDynamicSourceStream;  // The class that will handle each pin


//
// CDynamicSource
//
// Override construction to provide a means of creating
// CDynamicSourceStream derived objects - ie a way of creating pins.
class CDynamicSource: public CBaseFilter {
public:

    CDynamicSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr);
#ifdef UNICODE
    CDynamicSource(CHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr);
#endif
    ~CDynamicSource();

    int       GetPinCount(void);
    CBasePin *GetPin(int n);

    // -- Utilities --

    CCritSec*   pStateLock(void) { return &m_cStateLock; }  // provide our critical section

    HRESULT     AddPin(CDynamicSourceStream *);
    HRESULT     RemovePin(CDynamicSourceStream *);

    STDMETHODIMP FindPin(
        LPCWSTR Id,
        IPin ** ppPin
    );

    int FindPinNumber(IPin *iPin);
    
    STDMETHODIMP JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName);
    STDMETHODIMP Stop(void);
    STDMETHODIMP Pause(void);
    
protected:

    CAMEvent m_evFilterStoppingEvent;

    int m_iPins;    // The number of pins on this filter. Updated by 
                    // CDynamicSourceStream constructors & destructors.
                    
    CDynamicSourceStream **m_paStreams;   // the pins on this filter.

    // This lock must be held when m_paStreams or m_iPins
    // is being used.  The state lock (m_cStateLock) must 
    // also be held if the program wants to change the value
    // of m_paStreams or m_iPins.  Functions cannot acquire 
    // the state lock (m_cStateLock) after they acquire
    // m_csPinStateLock.  The program will deadlock if it
    // violates this rule.  Also the program may not acquire
    // m_csPinStateLock on the streaming thread.  If it does,
    // the program will deadlock.  The streaming thread calls
    // ThreadProc() and DoBufferProcessingLoop().
    CCritSec m_csPinStateLock;

    // This lock serializes accesses to the filter's state.
    // It also must be held when the program changes 
    // m_iPins's or m_paStreams's value.
    CCritSec m_cStateLock;

};


//
// CDynamicSourceStream
//
// Use this class to manage a stream of data that comes from a
// pin.
// Uses a worker thread to put data on the pin.
class CDynamicSourceStream : public CAMThread, public CDynamicOutputPin {
public:

    CDynamicSourceStream(TCHAR *pObjectName,
                         HRESULT *phr,
                         CDynamicSource*pms,
                         LPCWSTR pName);
#ifdef UNICODE
    CDynamicSourceStream(CHAR *pObjectName,
                         HRESULT *phr,
                         CDynamicSource*pms,
                         LPCWSTR pName);
#endif
    virtual ~CDynamicSourceStream(void);  // virtual destructor ensures derived 
                                          // class destructors are called too

    HRESULT DestroySourceThread(void);

protected:

    CDynamicSource*m_pFilter;             // The parent of this stream

    // *
    // * Data Source
    // *
    // * The following three functions: FillBuffer, OnThreadCreate/Destroy, are
    // * called from within the ThreadProc. They are used in the creation of
    // * the media samples this pin will provide
    // *

    // Override this to provide the worker thread a means
    // of processing a buffer
    virtual HRESULT FillBuffer(IMediaSample *pSamp) PURE;

    // Called as the thread is created/destroyed - use to perform
    // jobs such as start/stop streaming mode
    // If OnThreadCreate returns an error the thread will exit.
    virtual HRESULT OnThreadCreate(void) {return NOERROR;};
    virtual HRESULT OnThreadDestroy(void) {return NOERROR;};
    virtual HRESULT OnThreadStartPlay(void) {return NOERROR;};

    // *
    // * Worker Thread
    // *

    HRESULT Active(void);    // Starts up the worker thread

    HRESULT BreakConnect(void);

public:
    // thread commands
    enum Command {CMD_INIT, CMD_PAUSE, CMD_RUN, CMD_STOP, CMD_EXIT};

    HRESULT Init(void) { return CallWorker(CMD_INIT); }
    HRESULT Exit(void) { return CallWorker(CMD_EXIT); }
    HRESULT Run(void) { return CallWorker(CMD_RUN); }
    HRESULT Pause(void) { return CallWorker(CMD_PAUSE); }
    HRESULT Stop(void) { return CallWorker(CMD_STOP); }

    void OutputPinNeedsToBeReconnected(void);

protected:
    Command GetRequest(void) { return (Command) CAMThread::GetRequest(); }
    BOOL    CheckRequest(Command *pCom) { return CAMThread::CheckRequest( (DWORD *) pCom); }

    // override these if you want to add thread commands
    virtual DWORD ThreadProc(void);         // the thread function

    virtual HRESULT DoBufferProcessingLoop(void);    // the loop executed whilst running
    
    void FatalError(HRESULT hr);

    // *
    // * AM_MEDIA_TYPE support
    // *

    // If you support more than one media type then override these 2 functions
    virtual HRESULT CheckMediaType(const CMediaType *pMediaType);
    virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);  // List pos. 0-n

    // If you support only one type then override this fn.
    // This will only be called by the default implementations
    // of CheckMediaType and GetMediaType(int, CMediaType*)
    // You must override this fn. or the above 2!
    virtual HRESULT GetMediaType(CMediaType *pMediaType) {return E_UNEXPECTED;}

    STDMETHODIMP QueryId(
        LPWSTR * Id
    );

    bool m_fReconnectOutputPin;
};
    
#endif // __CDYNAMICSOURCE__


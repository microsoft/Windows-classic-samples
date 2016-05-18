//-----------------------------------------------------------------------------
// File: WaveSink.h
// Description: Archive sink for creating .wav files.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mferror.h>

#include <assert.h>

// utility functions

#define USE_LOGGING

#include "common.h"
using namespace MediaFoundationSamples;

#include "CreateWavSink.h"

// Forward declares
class CWavStream;
class CMarker;

enum FlushState
{
    DropSamples = 0,
    WriteSamples 
};


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
    virtual STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType) = 0;
    virtual STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar) = 0;
    virtual STDMETHODIMP GetContext(PROPVARIANT *pvar) = 0;
};



class CWavSink : public IMFFinalizableMediaSink,   // Note: IMFFinalizableMediaSink inherits IMFMediaSink
                 public IMFClockStateSink
{

    friend class CWavStream;

public:
    // Static method to create the object.
    static HRESULT CreateInstance(IMFByteStream *pStream, REFIID iid, void **ppSink);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaSink methods
    STDMETHODIMP GetCharacteristics(DWORD *pdwCharacteristics);

    STDMETHODIMP AddStreamSink( 
        /* [in] */ DWORD dwStreamSinkIdentifier,
        /* [in] */ IMFMediaType *pMediaType,
        /* [out] */ IMFStreamSink **ppStreamSink);

    STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
    STDMETHODIMP GetStreamSinkCount(DWORD *pcStreamSinkCount);
    STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink **ppStreamSink);
    STDMETHODIMP GetStreamSinkById(DWORD dwIdentifier, IMFStreamSink **ppStreamSink);
    STDMETHODIMP SetPresentationClock(IMFPresentationClock *pPresentationClock);
    STDMETHODIMP GetPresentationClock(IMFPresentationClock **ppPresentationClock);
    STDMETHODIMP Shutdown();

    // IMFFinalizableMediaSink methods
    STDMETHODIMP BeginFinalize(IMFAsyncCallback *pCallback, IUnknown *punkState);
    STDMETHODIMP EndFinalize(IMFAsyncResult *pResult);

    // IMFClockStateSink methods
    STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

private:

    CWavSink();
    virtual ~CWavSink();

    HRESULT Initialize(IMFByteStream *pByteStream);

    HRESULT CheckShutdown() const
    {
        if (m_IsShutdown)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }


    long                        m_nRefCount;                // reference count
    CritSec                     m_critSec;                  // critical section for thread safety

    BOOL                        m_IsShutdown;               // Flag to indicate if Shutdown() method was called.

    CWavStream                  *m_pStream;                 // Byte stream
    IMFPresentationClock        *m_pClock;                  // Presentation clock.
};


class CWavStream : public IMFStreamSink, public IMFMediaTypeHandler
{
    friend class CWavSink;

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFStreamSink
    STDMETHODIMP GetMediaSink(IMFMediaSink **ppMediaSink);
    STDMETHODIMP GetIdentifier(DWORD *pdwIdentifier);
    STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler);
    STDMETHODIMP ProcessSample(IMFSample *pSample);
    
    STDMETHODIMP PlaceMarker( 
        /* [in] */ MFSTREAMSINK_MARKER_TYPE eMarkerType,
        /* [in] */ const PROPVARIANT *pvarMarkerValue,
        /* [in] */ const PROPVARIANT *pvarContextValue);
    
    STDMETHODIMP Flush();

    // IMFMediaTypeHandler
    STDMETHODIMP IsMediaTypeSupported(IMFMediaType *pMediaType, IMFMediaType **ppMediaType);
    STDMETHODIMP GetMediaTypeCount(DWORD *pdwTypeCount);
    STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType **ppType);
    STDMETHODIMP SetCurrentMediaType(IMFMediaType *pMediaType);
    STDMETHODIMP GetCurrentMediaType(IMFMediaType **ppMediaType);
    STDMETHODIMP GetMajorType(GUID *pguidMajorType);


private:

    // State enum: Defines the current state of the stream.
    enum State
    {
        State_TypeNotSet = 0,    // No media type is set
        State_Ready,             // Media type is set, Start has never been called.
        State_Started,  
        State_Stopped,
        State_Paused,
        State_Finalized,

        State_Count = State_Finalized + 1    // Number of states
    };

    // StreamOperation: Defines various operations that can be performed on the stream.
    enum StreamOperation
    {
        OpSetMediaType = 0,
        OpStart,
        OpRestart,
        OpPause,
        OpStop,
        OpProcessSample,
        OpPlaceMarker,
        OpFinalize,

        Op_Count = OpFinalize + 1  // Number of operations
    };

    // CAsyncOperation:
    // Used to queue asynchronous operations. When we call MFPutWorkItem, we use this
    // object for the callback state (pState). Then, when the callback is invoked,
    // we can use the object to determine which asynchronous operation to perform.
 
    class CAsyncOperation : public IUnknown
    {
    public:
        CAsyncOperation(StreamOperation op);

        StreamOperation m_op;   // The operation to perform.  

        // IUnknown methods.
        STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

    private:
        long    m_nRefCount;
        virtual ~CAsyncOperation();
    };

    
    // ValidStateMatrix: Defines a look-up table that says which operations
    // are valid from which states.
    static BOOL ValidStateMatrix[State_Count][Op_Count];


    CWavStream();
    virtual ~CWavStream();

    HRESULT Initialize(CWavSink *pParent, IMFByteStream *pStream);

    HRESULT CheckShutdown() const
    {
        if (m_IsShutdown)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }


    HRESULT     Start(MFTIME start);
    HRESULT     Restart();
    HRESULT     Stop();
    HRESULT     Pause();
    HRESULT     Finalize(IMFAsyncCallback *pCallback, IUnknown *punkState);
    HRESULT     Shutdown();

    HRESULT     ValidateOperation(StreamOperation op);

    HRESULT     QueueAsyncOperation(StreamOperation op);
    
    HRESULT     OnDispatchWorkItem(IMFAsyncResult* pAsyncResult);
    HRESULT     DispatchProcessSample(CAsyncOperation* pOp);
    HRESULT     DispatchFinalize(CAsyncOperation* pOp);

    HRESULT     ProcessSamplesFromQueue(FlushState bFlushData);
    HRESULT     WriteSampleToFile(IMFSample *pSample);
    HRESULT     SendMarkerEvent(IMarker *pMarker, FlushState bFlushData);


    long                        m_nRefCount;                // reference count
    CritSec                     m_critSec;                  // critical section for thread safety

    State                       m_state;
    BOOL                        m_IsShutdown;               // Flag to indicate if Shutdown() method was called.
    
    DWORD                       m_WorkQueueId;              // ID of the work queue for asynchronous operations.
    AsyncCallback<CWavStream>   m_WorkQueueCB;              // Callback for the work queue.

    MFTIME                      m_StartTime;                // Presentation time when the clock started.
    DWORD                       m_cbDataWritten;            // How many bytes we have written so far.

    CWavSink                    *m_pSink;                   // Parent media sink

    IMFMediaEventQueue          *m_pEventQueue;             // Event queue
    IMFByteStream               *m_pByteStream;             // Bytestream where we write the data.
    IMFMediaType                *m_pCurrentType;

    ComPtrList<IUnknown>        m_SampleQueue;              // Queue to hold samples and markers.
                                                            // Applies to: ProcessSample, PlaceMarker, BeginFinalize

    IMFAsyncResult              *m_pFinalizeResult;         // Result object for Finalize operation.

};




// Holds marker information for IMFStreamSink::PlaceMarker 

class CMarker : public IMarker
{
public:
    static HRESULT Create(
        MFSTREAMSINK_MARKER_TYPE eMarkerType,
        const PROPVARIANT* pvarMarkerValue,
        const PROPVARIANT* pvarContextValue,
        IMarker **ppMarker
        );

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType);
    STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar);
    STDMETHODIMP GetContext(PROPVARIANT *pvar);

protected:
    MFSTREAMSINK_MARKER_TYPE m_eMarkerType;
    PROPVARIANT m_varMarkerValue;
    PROPVARIANT m_varContextValue;

private:
    long    m_nRefCount;

    CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType);
    virtual ~CMarker();
};



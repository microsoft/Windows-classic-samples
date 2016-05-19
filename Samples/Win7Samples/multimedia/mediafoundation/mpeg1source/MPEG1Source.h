//////////////////////////////////////////////////////////////////////////
//
// MPEG1Source.h
// Implements the MPEG-1 media source object.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <windows.h>
#include <assert.h>

#ifndef _ASSERTE 
#define _ASSERTE assert
#endif

#include <mftransform.h>

#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>
#include <uuids.h>      // MPEG-1 subtypes

#include <amvideo.h>    // VIDEOINFOHEADER definition
#include <dvdmedia.h>   // VIDEOINFOHEADER2
#include <mmreg.h>      // MPEG1WAVEFORMAT
#include <shlwapi.h>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")      // Media Foundation GUIDs
#pragma comment(lib, "strmiids")    // DirectShow GUIDs
#pragma comment(lib, "Ws2_32")      // htonl, etc
#pragma comment(lib, "shlwapi")

// Common sample files.
#define USE_LOGGING
#include "common.h"
using namespace MediaFoundationSamples;

#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)

#include "OpQueue.h"

// Forward declares
class MPEG1ByteStreamHandler;
class MPEG1Source;
class MPEG1Stream;
class SourceOp;

// Typedefs
typedef GrowableArray<MPEG1Stream*> StreamList;
typedef TinyMap<BYTE, DWORD>        StreamMap;    // Maps stream ID to index

typedef ComPtrList<IMFSample>       SampleList;   
typedef ComPtrList<IUnknown, true>  TokenList;    // List of tokens for IMFMediaStream::RequestSample


enum SourceState
{
    STATE_INVALID,      // Initial state. Have not started opening the stream.
    STATE_OPENING,      // BeginOpen is in progress.
    STATE_STOPPED,
    STATE_PAUSED,
    STATE_STARTED,
    STATE_SHUTDOWN
};


// Internal headers
#include "Parse.h"          // MPEG-1 parser
#include "MPEG1Stream.h"    // MPEG-1 stream


// Constants

const DWORD INITIAL_BUFFER_SIZE = 4 * 1024; // Initial size of the read buffer. (The buffer expands dynamically.)
const DWORD READ_SIZE = 4 * 1024;           // Size of each read request.
const DWORD SAMPLE_QUEUE = 2;               // How many samples does each stream try to hold in its queue?


// SourceOp: Represents a request for an asynchronous operation.
class SourceOp : RefCountedObject, public IUnknown
{
public:

    enum Operation
    {
        OP_START,
        OP_PAUSE,
        OP_STOP,
        OP_REQUEST_DATA,
        OP_END_OF_STREAM
    };

    static HRESULT CreateOp(Operation op, SourceOp **ppOp);
    static HRESULT CreateStartOp(IMFPresentationDescriptor *pPD, SourceOp **ppOp);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef() { return RefCountedObject::AddRef(); }
    STDMETHODIMP_(ULONG) Release() { return RefCountedObject::Release(); }

    SourceOp(Operation op);
    virtual ~SourceOp();

    HRESULT SetData(const PROPVARIANT& var);

    Operation Op() const { return m_op; }
    const PROPVARIANT& Data() { return m_data;}

protected:
    Operation                   m_op;
    PROPVARIANT                 m_data; // Data for the operation.
};

class StartOp : public SourceOp
{
public:
    StartOp(IMFPresentationDescriptor *pPD);
    ~StartOp();

    HRESULT GetPresentationDescriptor(IMFPresentationDescriptor **ppPD);

protected:
    IMFPresentationDescriptor   *m_pPD; // Presentation descriptor for Start operations.

};


// MPEG1Source: The media source object.
class MPEG1Source : BaseObject, RefCountedObject, public OpQueue<SourceOp>, public IMFMediaSource
{
public:
    static HRESULT CreateInstance(MPEG1Source **ppSource);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef() { return RefCountedObject::AddRef(); }
    STDMETHODIMP_(ULONG) Release() { return RefCountedObject::Release(); }

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaSource
    STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor);
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacteristics);
    STDMETHODIMP Pause();
    STDMETHODIMP Shutdown();
    STDMETHODIMP Start(
        IMFPresentationDescriptor* pPresentationDescriptor,
        const GUID* pguidTimeFormat,
        const PROPVARIANT* pvarStartPosition
    );
    STDMETHODIMP Stop();

    // Called by the byte stream handler.
    HRESULT BeginOpen(IMFByteStream *pStream, IMFAsyncCallback *pCB, IUnknown *pUnkState);
    HRESULT EndOpen(IMFAsyncResult *pResult);

    // Queues an asynchronous operation, specify by op-type.
    // (This method is public because the streams call it.)
    HRESULT QueueAsyncOperation(SourceOp::Operation OpType);

    // Lock/Unlock:
    // Holds and releases the source's critical section. Called by the streams.
    void    Lock() { m_critSec.Lock(); }
    void    Unlock() { m_critSec.Unlock(); }

    // Callbacks
    HRESULT OnByteStreamRead(IMFAsyncResult *pResult);  // Async callback for RequestData

private:

    MPEG1Source(HRESULT& hr);
    ~MPEG1Source();

    // CheckShutdown: Returns MF_E_SHUTDOWN if the source was shut down.
    HRESULT CheckShutdown() const
    {
        return ( m_state == STATE_SHUTDOWN ? MF_E_SHUTDOWN : S_OK );
    }

    HRESULT     CompleteOpen(HRESULT hrStatus);

    HRESULT     IsInitialized() const;
    BOOL        IsStreamTypeSupported(StreamType type) const;
    BOOL        IsStreamActive(const MPEG1PacketHeader& packetHdr);
    BOOL        StreamsNeedData() const;

    HRESULT     DoStart(StartOp *pOp);
    HRESULT     DoStop(SourceOp *pOp);
    HRESULT     DoPause(SourceOp *pOp);
    HRESULT     OnStreamRequestSample(SourceOp *pOp);
    HRESULT     OnEndOfStream(SourceOp *pOp);

    HRESULT     InitPresentationDescriptor();
    HRESULT     SelectStreams(IMFPresentationDescriptor *pPD, const PROPVARIANT varStart);

    HRESULT     RequestData(DWORD cbRequest);
    HRESULT     ParseData();
    HRESULT     ReadPayload(DWORD *pcbAte, DWORD *pcbNextRequest);
    HRESULT     DeliverPayload();
    HRESULT     EndOfMPEGStream();

    HRESULT     CreateStream(const MPEG1PacketHeader& packetHdr);

    HRESULT     ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
        
    // Handler for async errors.
    void        StreamingError(HRESULT hr);

    // GetStream: Looks up a stream by ID.
    MPEG1Stream*    GetStream(BYTE stream_id);    // Can return NULL

    HRESULT     BeginAsyncOp(SourceOp *pOp);
    HRESULT     CompleteAsyncOp(SourceOp *pOp);
    HRESULT     DispatchOperation(SourceOp *pOp);
    HRESULT     ValidateOperation(SourceOp *pOp);


private:
    CritSec                     m_critSec;                  // critical section for thread safety
    SourceState                 m_state;                    // Current state (running, stopped, paused)

    Buffer                      m_ReadBuffer;
    Parser                      *m_pParser;

    IMFMediaEventQueue          *m_pEventQueue;             // Event generator helper
    IMFPresentationDescriptor   *m_pPresentationDescriptor; // Presentation descriptor.
    IMFAsyncResult              *m_pBeginOpenResult;        // Result object for async BeginOpen operation.
    IMFByteStream               *m_pByteStream;

    MPEG1SystemHeader           *m_pHeader;                 // Release with CoTaskMemFree

    StreamList                  m_streams;                  // Array of streams.  
    StreamMap                   m_streamMap;                // Maps stream IDs to indexes into m_streams array.

    DWORD                       m_cPendingEOS;              // Pending EOS notifications.
    ULONG                       m_cRestartCounter;          // Counter for sample requests.

    SourceOp                    *m_pCurrentOp;
    SourceOp                    *m_pSampleRequest;

    // Async callback helper.
    AsyncCallback<MPEG1Source>  m_OnByteStreamRead;

};



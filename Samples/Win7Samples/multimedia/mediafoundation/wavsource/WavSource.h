//////////////////////////////////////////////////////////////////////
//
// WavSource.h : Sample audio media source for Media Foundation
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Notes:
// This sample implements a relatively simple audio source
// to parse .wav files. 
// 
// Design decisions:
//
// - For simplicity, the source performs all methods synchronously.
// - Also for simplicity, the source only accepts uncompressed PCM audio
//   formats.
// - It does not support rate control. 
//
//////////////////////////////////////////////////////////////////////


#pragma once

#include <new>
#include <windows.h>
#include <assert.h>

#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>
#include <shlwapi.h>

#include "RiffParser.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


void DllAddRef();
void DllRelease();

class WavStream;
class WavSource;

LONGLONG AudioDurationFromBufferSize(const WAVEFORMATEX *pWav, DWORD cbAudioDataSize);


//////////////////////////////////////////////////////////////////////////
//  WavSource
//  Description: Media source object.
//////////////////////////////////////////////////////////////////////////

class WavSource : public IMFMediaSource
{
    friend class WavStream;

public:
    static HRESULT CreateInstance(REFIID iid, void **ppSource);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

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

    // Other methods
    HRESULT Open(IMFByteStream *pStream);

private:

    enum State
    {
        STATE_STOPPED,
        STATE_PAUSED,
        STATE_STARTED
    };


    // Constructor is private - client should use static CreateInstance method.
    WavSource(HRESULT &hr);
    virtual ~WavSource();

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

    HRESULT     CreatePresentationDescriptor();
    HRESULT     QueueNewStreamEvent(IMFPresentationDescriptor *pPD);
    HRESULT     CreateWavStream(IMFStreamDescriptor *pSD);
    HRESULT     ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);

    LONGLONG    GetCurrentPosition() const;
    State       GetState() const { return m_state; }

    const WAVEFORMATEX*         WaveFormat() const;         // Returns a pointer to the format.
    DWORD                       WaveFormatSize() const;     // Returns the size of the format, in bytes.

    IMFMediaEventQueue          *m_pEventQueue;             // Event generator helper
    IMFPresentationDescriptor   *m_pPresentationDescriptor; // Default presentation

    WavStream                   *m_pStream;                 // Media stream. Can be NULL is no stream is selected.

    long                        m_nRefCount;                // reference count
    CRITICAL_SECTION            m_critSec;
    BOOL                        m_IsShutdown;               // Flag to indicate if Shutdown() method was called.
    State                       m_state;                    // Current state (running, stopped, paused)

    CWavRiffParser              *m_pRiff;
};


class SampleQueue
{
protected:

    // Nodes in the linked list
    struct Node
    {
        Node *prev;
        Node *next;
        IMFSample*  item;

        Node() : prev(NULL), next(NULL)
        {
        }

        Node(IMFSample* item) : prev(NULL), next(NULL)
        {
            this->item = item;
        }

        IMFSample* Item() const { return item; }
    };


protected:
    Node    m_anchor;  // Anchor node for the linked list.

public:

    SampleQueue()
    {
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;
    }

    virtual ~SampleQueue()
    {
        Clear();
    }

    HRESULT Queue(IMFSample* item)
    {
        if (item == NULL)
        {
            return E_POINTER;
        }

        Node *pNode = new (std::nothrow) Node(item);
        if (pNode == NULL)
        {
            return E_OUTOFMEMORY;
        }

        item->AddRef();

        Node *pBefore = m_anchor.prev;

        Node *pAfter = pBefore->next;
        
        pBefore->next = pNode;
        pAfter->prev = pNode;

        pNode->prev = pBefore;
        pNode->next = pAfter;

        return S_OK;

    }

    HRESULT Dequeue(IMFSample* *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        if (ppItem == NULL)
        {
            return E_POINTER;
        }

        Node *pNode = m_anchor.next;

        // The next node's previous is this node's previous.
        pNode->next->prev = m_anchor.next->prev;

        // The previous node's next is this node's next.
        pNode->prev->next = pNode->next;

        *ppItem = pNode->item;
        delete pNode;

        return S_OK;
    }

    BOOL IsEmpty() const { return m_anchor.next == &m_anchor; }

    void Clear()
    {
        Node *n = m_anchor.next;

        // Delete the nodes
        while (n != &m_anchor)
        {
            if (n->item)
            {
                n->item->Release();
            }

            Node *tmp = n->next;
            delete n;
            n = tmp;
        }

        // Reset the anchor to point at itself
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;
    }

};



//////////////////////////////////////////////////////////////////////////
//  WavStream
//  Description: Media stream object.
//////////////////////////////////////////////////////////////////////////


class WavStream : public IMFMediaStream 
{
    friend class WavSource;

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

    // IMFMediaStream
    STDMETHODIMP GetMediaSource(IMFMediaSource** ppMediaSource);
    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor);
    STDMETHODIMP RequestSample(IUnknown* pToken);
    
private:

    WavStream(WavSource *pSource, CWavRiffParser *pRiff, IMFStreamDescriptor *pSD, HRESULT& hr);
    ~WavStream();


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


    HRESULT     Shutdown();
    HRESULT     CreateAudioSample(IMFSample **pSample);
    HRESULT     DeliverSample(IMFSample *pSample);
    HRESULT     DeliverQueuedSamples();
    HRESULT     Flush();

    LONGLONG    GetCurrentPosition() const { return m_rtCurrentPosition; }
    HRESULT     SetPosition(LONGLONG rtNewPosition);
    HRESULT     CheckEndOfStream();


    long                        m_nRefCount;            // reference count
    CRITICAL_SECTION            m_critSec;
    BOOL                        m_IsShutdown;           // Flag to indicate if source's Shutdown() method was called.
    LONGLONG                    m_rtCurrentPosition;    // Current position in the stream, in 100-ns units 
    BOOL                        m_discontinuity;        // Is the next sample a discontinuity?
    BOOL                        m_EOS;                  // Did we reach the end of the stream?

    CWavRiffParser              *m_pRiff;  // Non-owning
    IMFMediaEventQueue          *m_pEventQueue;         // Event generator helper.
    WavSource                   *m_pSource;             // Parent media source
    IMFStreamDescriptor         *m_pStreamDescriptor;   // Stream descriptor for this stream.

    SampleQueue                  m_sampleQueue;          // Queue for samples while paused.

};



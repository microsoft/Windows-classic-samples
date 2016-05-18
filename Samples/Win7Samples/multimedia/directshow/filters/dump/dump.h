//------------------------------------------------------------------------------
// File: Dump.h
//
// Desc: DirectShow sample code - definitions for dump renderer.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


class CDumpInputPin;
class CDump;
class CDumpFilter;

#define BYTES_PER_LINE 20
#define FIRST_HALF_LINE TEXT  ("   %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x")
#define SECOND_HALF_LINE TEXT (" %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x")


// Main filter object

class CDumpFilter : public CBaseFilter
{
    CDump * const m_pDump;

public:

    // Constructor
    CDumpFilter(CDump *pDump,
                LPUNKNOWN pUnk,
                CCritSec *pLock,
                HRESULT *phr);

    // Pin enumeration
    CBasePin * GetPin(int n);
    int GetPinCount();

    // Open and close the file as necessary
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();
};


//  Pin object

class CDumpInputPin : public CRenderedInputPin
{
    CDump    * const m_pDump;           // Main renderer object
    CCritSec * const m_pReceiveLock;    // Sample critical section
    REFERENCE_TIME m_tLast;             // Last sample receive time

public:

    CDumpInputPin(CDump *pDump,
                  LPUNKNOWN pUnk,
                  CBaseFilter *pFilter,
                  CCritSec *pLock,
                  CCritSec *pReceiveLock,
                  HRESULT *phr);

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);
    STDMETHODIMP ReceiveCanBlock();

    // Write detailed information about this sample to a file
    HRESULT WriteStringInfo(IMediaSample *pSample);

    // Check if the pin can support this specific proposed type and format
    HRESULT CheckMediaType(const CMediaType *);

    // Break connection
    HRESULT BreakConnect();

    // Track NewSegment
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart,
                            REFERENCE_TIME tStop,
                            double dRate);
};


//  CDump object which has filter and pin members

class CDump : public CUnknown, public IFileSinkFilter
{
    friend class CDumpFilter;
    friend class CDumpInputPin;

    CDumpFilter   *m_pFilter;       // Methods for filter interfaces
    CDumpInputPin *m_pPin;          // A simple rendered input pin

    CCritSec m_Lock;                // Main renderer critical section
    CCritSec m_ReceiveLock;         // Sublock for received samples

    CPosPassThru *m_pPosition;      // Renderer position controls

    HANDLE   m_hFile;               // Handle to file for dumping
    LPOLESTR m_pFileName;           // The filename where we dump
    BOOL     m_fWriteError;

public:

    DECLARE_IUNKNOWN

    CDump(LPUNKNOWN pUnk, HRESULT *phr);
    ~CDump();

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Write string, followed by CR/LF, to a file
    void WriteString(TCHAR *pString);

    // Write raw data stream to a file
    HRESULT Write(PBYTE pbData, LONG lDataLength);

    // Implements the IFileSinkFilter interface
    STDMETHODIMP SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt);

private:

    // Overriden to say what interfaces we support where
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // Open and write to the file
    HRESULT OpenFile();
    HRESULT CloseFile();
    HRESULT HandleWriteFailure();
};


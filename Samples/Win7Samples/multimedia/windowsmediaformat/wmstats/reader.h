//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            reader.h
//
// Abstract:            Definition for CReader class
//
//*****************************************************************************

#if !defined(AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_)
#define AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_

#include "stdafx.h"
#include "Writer.h"
#include "ExtensionData.h"

#define MAX_STATS_DELAY 120    //seconds


class CReader : 
        public IWMReaderCallback, 
        public IWMReaderCallbackAdvanced  
{
public:
    CReader();
    virtual ~CReader();

public:
    HRESULT Init();
    HRESULT Configure( const WCHAR *pwszFile );
    HRESULT AttachWriter( CWriter *pWriter );
    HRESULT Start();
    HRESULT Stop(  HANDLE *hThread, int cHandles );
    HRESULT Close(  HANDLE *hThread, int cHandles );
    HRESULT GetStats ( WM_READER_STATISTICS *pStats );
    void WaitForEvent();





    //
    //Methods of IWMReaderCallback
    //
    HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
                                        /* [in] */ QWORD cnsSampleTime,
                                        /* [in] */ QWORD cnsSampleDuration,
                                        /* [in] */ DWORD dwFlags,
                                        /* [in] */ INSSBuffer __RPC_FAR *pSample,
                                        /* [in] */ void __RPC_FAR *pvContext );

    HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
                                        /* [in] */ HRESULT hr,
                                        /* [in] */ WMT_ATTR_DATATYPE dwType,
                                        /* [in] */ BYTE __RPC_FAR *pValue,
                                        /* [in] */ void __RPC_FAR *pvContext );
    //
    //Methhods of IWMReaderCallbackAdvanced
    //
    HRESULT STDMETHODCALLTYPE OnStreamSample( /* [in] */ WORD wStreamNum,
                                              /* [in] */ QWORD cnsSampleTime,
                                              /* [in] */ QWORD cnsSampleDuration,
                                              /* [in] */ DWORD dwFlags,
                                              /* [in] */ INSSBuffer __RPC_FAR *pSample,
                                              /* [in] */ void __RPC_FAR *pvContext );

    HRESULT STDMETHODCALLTYPE OnTime( /* [in] */ QWORD cnsCurrentTime,
                                      /* [in] */ void __RPC_FAR *pvContext );

    HRESULT STDMETHODCALLTYPE OnStreamSelection( /* [in] */ WORD wStreamCount,
                                                 /* [in] */ WORD __RPC_FAR *pStreamNumbers,
                                                 /* [in] */ WMT_STREAM_SELECTION __RPC_FAR *pSelections,
                                                 /* [in] */ void __RPC_FAR *pvContext )
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
                                                    /* [in] */ WM_MEDIA_TYPE __RPC_FAR *pMediaType,
                                                    /* [in] */ void __RPC_FAR *pvContext )
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE AllocateForStream( /* [in] */ WORD wStreamNum,
                                                 /* [in] */ DWORD cbBuffer,
                                                 /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
                                                 /* [in] */ void __RPC_FAR *pvContext )
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE AllocateForOutput( /* [in] */ DWORD dwOutputNum,
                                                 /* [in] */ DWORD cbBuffer,
                                                 /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
                                                 /* [in] */ void __RPC_FAR *pvContext )
    {
        return E_NOTIMPL;
    }

    //
    //Methods of IUnknown
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject); 
    ULONG STDMETHODCALLTYPE AddRef( void ) { return 1; }
    ULONG STDMETHODCALLTYPE Release( void ) { return 1; }

private :
    HRESULT SetCodecOff(IWMProfile*  pProfile);
    HRESULT CopyScriptsToWriter();
    HRESULT CopyAttribToWriter(const WCHAR *pwszName);


private :
    CWriter*                m_pWriter;

    DWORD                   m_dwTimerId;

    HANDLE                  m_hEvent ;
    HRESULT                 m_hrAsync ;
    QWORD                   m_qwTime;
    IWMReaderAdvanced*      m_pReaderAdvanced;
    IWMReader*              m_pReader;
    IWMHeaderInfo*          m_pReaderHeaderInfo ;
    IWMReaderStreamClock*   m_pReaderStreamClock ;
    bool                    m_fNetReading;
    bool                    m_fReaderStarted;
    bool                    m_fEOF;
    DWORD                   m_dwAudioStreamNum;
	CExtDataList            m_ExtDataList;


};

#endif // !defined(AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_)

//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            NetWrite.cpp
//
// Abstract:            CNetWrite class declaration
//                      
//
//*****************************************************************************

#pragma once

#include "wmsdk.h"

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if( NULL != x )       \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }

#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE( x )	\
    if( NULL != x )		\
    {						\
        delete x;			\
        x = NULL;			\
    }
#endif

#ifndef SAFE_ARRAYDELETE
#define SAFE_ARRAYDELETE( x )	\
    if( NULL != x )		\
    {						\
        delete[] x;			\
        x = NULL;			\
    }
#endif

#define NETWRITE_ASYNC_EVENT	_T( "6d12fe9b-d029-4d08-b2eb-92c8cab323c7" )

class CNetWrite :public IWMReaderCallback, public IWMReaderCallbackAdvanced
{
public:
    CNetWrite();
    ~CNetWrite();
    HRESULT Configure( DWORD dwPortNum, const WCHAR *pwszFile, UINT nMaxClient, const WCHAR *pwszServerURL );
    HRESULT WritetoNet();
    HRESULT Init();
    
    
    //
    //Methods of IWMReaderCallback
    //
    HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
        /* [in] */ QWORD cnsSampleTime,
        /* [in] */ QWORD cnsSampleDuration,
        /* [in] */ DWORD dwFlags,
        /* [in] */ INSSBuffer __RPC_FAR *pSample,
        /* [in] */ void __RPC_FAR *pvContext);
        
        HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
        /* [in] */ HRESULT hr,
        /* [in] */ WMT_ATTR_DATATYPE dwType,
        /* [in] */ BYTE __RPC_FAR *pValue,
        /* [in] */ void __RPC_FAR *pvContext);
        //
        //Methods of IWMReaderCallbackAdvanced
        //
        HRESULT STDMETHODCALLTYPE OnStreamSample( /* [in] */ WORD wStreamNum,
        /* [in] */ QWORD cnsSampleTime,
        /* [in] */ QWORD cnsSampleDuration,
        /* [in] */ DWORD dwFlags,
        /* [in] */ INSSBuffer __RPC_FAR *pSample,
        /* [in] */ void __RPC_FAR *pvContext);
        
        HRESULT STDMETHODCALLTYPE OnTime( /* [in] */ QWORD cnsCurrentTime,
        /* [in] */ void __RPC_FAR *pvContext);
        
        HRESULT STDMETHODCALLTYPE OnStreamSelection( /* [in] */ WORD wStreamCount,
        /* [in] */ WORD __RPC_FAR *pStreamNumbers,
        /* [in] */ WMT_STREAM_SELECTION __RPC_FAR *pSelections,
        /* [in] */ void __RPC_FAR *pvContext)
    {
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
        /* [in] */ WM_MEDIA_TYPE __RPC_FAR *pMediaType,
        /* [in] */ void __RPC_FAR *pvContext)
    {
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE AllocateForStream( /* [in] */ WORD wStreamNum,
        /* [in] */ DWORD cbBuffer,
        /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
        /* [in] */ void __RPC_FAR *pvContext)
    {
        return E_NOTIMPL;
    }
    
    HRESULT STDMETHODCALLTYPE AllocateForOutput( /* [in] */ DWORD dwOutputNum,
        /* [in] */ DWORD cbBuffer,
        /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
        /* [in] */ void __RPC_FAR *pvContext)
    {
        return E_NOTIMPL;
    }
    
    //
    //Methods of IUnknown
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid,
        void __RPC_FAR *__RPC_FAR *ppvObject) 
    {
        if( riid == IID_IWMReaderCallback )
        {
            *ppvObject = ( IWMReaderCallback* )this;
        }
        else if( riid == IID_IWMReaderCallbackAdvanced )
        {
            *ppvObject = ( IWMReaderCallbackAdvanced* )this;
        }
        else if( riid == IID_IWMStatusCallback )
        {
            *ppvObject = ( IWMStatusCallback* )this;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }
    
    ULONG STDMETHODCALLTYPE AddRef( void ) { return 1; }
    
    ULONG STDMETHODCALLTYPE Release( void ) { return 1; }
    
private:
    HRESULT WriteHeader( const WCHAR * pwszName );
    HRESULT WriteScript();
private:
    HANDLE			m_hEvent;    // Event for handling asynchronous calls
    HRESULT			m_hrAsync;   // Receives error or success codes for asynchronous operations.
    QWORD			m_qwTime;    // Specifies the time interval for the next batch of samples.

    // Miscellaneous interfaces that we need.
    IWMWriterAdvanced*		m_pWriterAdvanced;
    IWMReaderAdvanced*		m_pReaderAdvanced;
    IWMReader*			m_pReader;
    IWMWriter*			m_pWriter;
    IWMWriterNetworkSink*	m_pNetSink;
    IWMWriterPushSink*          m_pPushSink;
    IWMRegisterCallback*        m_pPushSinkCallbackCtrl;
    bool			m_bEOF;
    IWMHeaderInfo*		m_pReaderHeaderInfo;
    IWMHeaderInfo*		m_pWriterHeaderInfo;
};

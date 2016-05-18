//***************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Append.h
//
// Abstract:            Definition of class CAppend
//
//*****************************************************************************
#if !defined(AFX_APPEND_H__7A75BCE7_12D1_48BC_8259_B6DE4E3685DD__INCLUDED_)
#define AFX_APPEND_H__7A75BCE7_12D1_48BC_8259_B6DE4E3685DD__INCLUDED_

#define WMVAPPEND_ASYNC_EVENT	_T( "ef30d794-80b3-4cfa-b241-d7d10fcd4c91" )

//////////////////////////////////////////////////////////////////////////////
class CAppend :public IWMReaderCallback, public IWMReaderCallbackAdvanced  
{

public:
    
    CAppend();
	~CAppend();
    
    //
	// Methods of IWMReaderCallback
	//
    HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
                                        /* [in] */ QWORD cnsSampleTime,
                                        /* [in] */ QWORD cnsSampleDuration,
                                        /* [in] */ DWORD dwFlags,
                                        /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                        /* [in] */ void __RPC_FAR * pvContext);

    HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
                                        /* [in] */ HRESULT hr,
                                        /* [in] */ WMT_ATTR_DATATYPE dwType,
                                        /* [in] */ BYTE __RPC_FAR * pValue,
                                        /* [in] */ void __RPC_FAR * pvContext);
    
    //
    // Methhods of IWMReaderCallbackAdvanced
    //
    HRESULT STDMETHODCALLTYPE OnStreamSample( /* [in] */ WORD wStreamNum,
                                              /* [in] */ QWORD cnsSampleTime,
                                              /* [in] */ QWORD cnsSampleDuration,
                                              /* [in] */ DWORD dwFlags,
                                              /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                              /* [in] */ void __RPC_FAR * pvContext);

    HRESULT STDMETHODCALLTYPE OnTime( /* [in] */ QWORD cnsCurrentTime,
                                      /* [in] */ void __RPC_FAR * pvContext);

    HRESULT STDMETHODCALLTYPE OnStreamSelection( /* [in] */ WORD wStreamCount,
                                                 /* [in] */ WORD __RPC_FAR * pStreamNumbers,
												 /* [in] */ WMT_STREAM_SELECTION __RPC_FAR * pSelections,
												 /* [in] */ void __RPC_FAR * pvContext)
    {
		return( S_OK );
    }

    HRESULT STDMETHODCALLTYPE OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
													/* [in] */ WM_MEDIA_TYPE __RPC_FAR * pMediaType,
													/* [in] */ void __RPC_FAR * pvContext)
    {
		return( S_OK );
    }

    HRESULT STDMETHODCALLTYPE AllocateForStream( /* [in] */ WORD wStreamNum,
												 /* [in] */ DWORD cbBuffer,
												 /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
												 /* [in] */ void __RPC_FAR * pvContext)
    {
		return( E_NOTIMPL );
    }

    HRESULT STDMETHODCALLTYPE AllocateForOutput( /* [in] */ DWORD dwOutputNum,
												 /* [in] */ DWORD cbBuffer,
												 /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
												 /* [in] */ void __RPC_FAR * pvContext)
    {
		return( E_NOTIMPL );
    }

	//
	// Methods of IUnknown
	//
    HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */  REFIID riid,
                                              /* [out] */ void ** ppvObject );
    ULONG STDMETHODCALLTYPE AddRef( void );
	ULONG STDMETHODCALLTYPE Release( void ); 
	
    	
	HRESULT StartAppending();
	HRESULT Configure( __in LPWSTR pwszOutFile );
	HRESULT CompareProfiles( __in LPWSTR pwszFirstInfile, __in LPWSTR pwszSecondInfile, BOOL * pIsEqual );
	HRESULT Exit();
	HRESULT Init();
	
private:

    HRESULT IsFileProtected( __in LPWSTR pswzFileName);
    HRESULT CopyAllMarkers();
    HRESULT CopyAttribute( WORD nInStreamNum,  WORD nOutStreamNum, IWMHeaderInfo * pWriterHeaderInfo, LPCWSTR pwszName );
    HRESULT CopyAllAttributes( IWMHeaderInfo * pWriterHeaderInfo );
    HRESULT CopyCodecInfo( IWMHeaderInfo * pWriterHeaderInfo );
    HRESULT CopyScript( IWMHeaderInfo * pWHdrInfo, IWMHeaderInfo * pRHdrInfo, QWORD qwTimeOffset );
    HRESULT SetReceiveStreamSample( IWMReaderAdvanced * pReaderAdv, IWMProfile * pProfile, DWORD nStreamIndex );
    HRESULT CopyMarkersFromHdr( IWMHeaderInfo * pRHdrInfo, IWMHeaderInfo * pWHdrInfo, QWORD qwTimeOffset );
    WORD    MapStreamNum( WORD dwNum );

private:
	
	CRITICAL_SECTION    m_crisecFile;

    IWMHeaderInfo *     m_pRdrHdrInfo2;
    IWMHeaderInfo *     m_pRdrHdrInfo1;
    IWMProfile *        m_pFirstProfile;
    IWMProfile *        m_pSecondProfile;
	IWMWriterAdvanced * m_pWriterAdv;
	IWMWriter *         m_pWriter;
    IWMReaderAdvanced * m_pReaderAdv2;
	IWMReaderAdvanced * m_pReaderAdv1;
    IWMReader *         m_pReader1;
    IWMReader *         m_pReader2;

    WORD*   m_pwStreamNumMap;
    DWORD   m_dwStreamCount;
    BOOL    m_bEOF;
    QWORD   m_qwFirstTime;
    QWORD   m_qwSecondTime;
    HRESULT m_hrAsync;
    HANDLE  m_hAsyncEvent;
	short   m_nCurrentFile;
	LPWSTR  m_pwszOutFile;
	LONG    m_cRef;
};

#endif // !defined(AFX_APPEND_H__7A75BCE7_12D1_48BC_8259_B6DE4E3685DD__INCLUDED_)

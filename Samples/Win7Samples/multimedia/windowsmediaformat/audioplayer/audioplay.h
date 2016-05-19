//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            AudioPlay.h
//
// Abstract:            Definition for the CAudioPlay class.
//
//*****************************************************************************

#if !defined(AFX_AUDIOPLAY_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_)
#define AFX_AUDIOPLAY_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x )  \
    if( NULL != x )        \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }
#endif

#ifndef SAFE_ARRAYDELETE
#define SAFE_ARRAYDELETE( x )  \
    if( NULL != x )        \
    {                      \
        delete [] x;       \
        x = NULL;          \
    }
#endif

#ifndef SAFE_SYSFREESTRING
#define SAFE_SYSFREESTRING( x )  \
    if( NULL != x )        \
    {                      \
        SysFreeString( x );       \
        x = NULL;          \
    }
#endif
///////////////////////////////////////////////////////////////////////////////
class CAudioPlay : public IWMReaderCallback  
{
public:

	CAudioPlay();

    //
    // Methods of IUnknown
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid,
                                              /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject ); 
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

	//
	// Methods of IWMReaderCallback
	//
	HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
										/* [in] */ HRESULT hr,
										/* [in] */ WMT_ATTR_DATATYPE dwType,
										/* [in] */ BYTE __RPC_FAR *pValue,
										/* [in] */ void __RPC_FAR *pvContext );

	HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
										/* [in] */ QWORD cnsSampleTime,
										/* [in] */ QWORD cnsSampleDuration,
										/* [in] */ DWORD dwFlags,
										/* [in] */ INSSBuffer __RPC_FAR *pSample,
										/* [in] */ void __RPC_FAR *pvContext );
    //
    // CAudioPlay methods
    //
	HRESULT Init();
	HRESULT Exit();
	HRESULT	Open( LPCWSTR pwszUrl );
    HRESULT	Close();
	HRESULT	Start( QWORD cnsStart = 0 );
	HRESULT	Stop();
	HRESULT Pause();
	HRESULT Resume();
#ifdef SUPPORT_DRM
	HRESULT ReopenReader( void *pvContext );
#endif
	void	SetAsyncEvent( HRESULT hrAsync );
	QWORD	GetFileDuration();
	BOOL	IsSeekable();
    BOOL    IsBroadcast();
	static DWORD WINAPI  OnWaveOutThread( LPVOID lpParameter );
	static void CALLBACK WaveProc( HWAVEOUT hwo,
								   UINT uMsg, 
								   DWORD_PTR dwInstance,
								   DWORD dwParam1,
								   DWORD dwParam2 );

private:

	~CAudioPlay();

	HRESULT GetHeaderAttribute( LPCWSTR pwszName, BYTE** ppbValue );
	HRESULT RetrieveAndDisplayAttributes();
	HRESULT GetAudioOutput();
	void	WaitForEvent( HANDLE hEvent, DWORD msMaxWaitTime = INFINITE );
	void	OnWaveOutMsg();

#ifdef SUPPORT_DRM
	BOOL	            m_bProcessingDRMOps;    // TRUE if license is required to play the content
#endif
    BOOL                m_bClosed;              // TRUE if no content is opened.
    BOOL                m_bIsSeekable;          // TRUE if the content is seekable
    BOOL                m_bIsBroadcast;         // TRUE if playing a broadcast stream
	BOOL				m_bEOF;                 // TRUE if EOF is reached or we need to stop
	DWORD				m_dwThreadID;           // thread ID for wave output
	DWORD				m_dwAudioOutputNum;     // audio output number
	HANDLE				m_hAsyncEvent;          // event handle
	HRESULT				m_hrAsync;              // HRESULT during async operations
	HWAVEOUT			m_hWaveOut;             // handle to waveout device
	IWMReader*			m_pReader;              // IWMReader pointer
	IWMHeaderInfo*	    m_pHeaderInfo;          // IWMHeaderInfo pointer
    LONG                m_cRef;                 // reference count
	LONG				m_cHeadersLeft;         // buffers to be played back at waveout device
	LPWSTR				m_pwszURL;              // URL to the content
	QWORD				m_cnsFileDuration;      // content's playback duration
    WAVEFORMATEX*       m_pWfx;                 // waveformat struct

};

#endif // !defined(AFX_AUDIOPLAY_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_)

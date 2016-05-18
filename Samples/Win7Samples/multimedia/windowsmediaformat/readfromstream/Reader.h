//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Reader.h
//
// Abstract:            Definition of class CReader.
//
//*****************************************************************************

#if !defined(AFX_READER_H__1112A58E_5BDE_4911_BE02_2822731B8DFC__INCLUDED_)
#define AFX_READER_H__1112A58E_5BDE_4911_BE02_2822731B8DFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ROStream.h"

#define MAX_TIMEOUT_FOR_EVENT   60000       // default timeout value when waiting for an event

///////////////////////////////////////////////////////////////////////////////
class CReader : public IWMReaderCallback
{
public:

	CReader();

    //
    //Methods of IUnknown
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid,
                                              /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject ); 
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

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
    // CReader methods
    //
    // Open specified input file for read
    //
    HRESULT Open( /* [in] */ const TCHAR *ptszFile );

    //
    // Start to read from IStream
    //
    HRESULT Start();

    //
    // Close the file
    //
    HRESULT Close();

    //
    // Wait for m_hEvent within specified time in milliseconds. Default timeout value is MAX_TIMEOUT_FOR_EVENT.
    //
    HRESULT WaitForEvent( /* [in] */ DWORD msWait = MAX_TIMEOUT_FOR_EVENT );

protected:

	virtual ~CReader();

    CROStream*          m_pStream;
    HANDLE              m_hEvent;
    HRESULT             m_hrAsync;
    IWMReader*          m_pReader;
    IWMReaderAdvanced2* m_pReader2;
    LONG                m_cRef;
};

#endif // !defined(AFX_READER_H__1112A58E_5BDE_4911_BE02_2822731B8DFC__INCLUDED_)

//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            WMVCopy.h
//
// Abstract:            Interface of CWMVCopy class. 
//
//*****************************************************************************

#if !defined(AFX_WMVCopy_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_WMVCopy_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wmsdk.h>
#include <strsafe.h>
#include "ScriptList.h"

////////////////////////////////////////////////////////////////////////////////
// This class is used to copy a WMV file
////////////////////////////////////////////////////////////////////////////////
class CWMVCopy : public IWMReaderCallback, 
                 public IWMReaderCallbackAdvanced
{
public:
    CWMVCopy();
    ~CWMVCopy();

    //
    // Copy the input file to the output file                  
    //
    HRESULT Copy( const WCHAR * pwszInputFile, 
                  const WCHAR * pwszOutputFile, 
                  QWORD qwMaxDuration,
                  BOOL fMoveScriptStream );


    //
    // IWMReaderCallback interface
    //
    virtual HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
                                                /* [in] */ QWORD qwSampleTime,
                                                /* [in] */ QWORD qwSampleDuration,
                                                /* [in] */ DWORD dwFlags,
                                                /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                                /* [in] */ void __RPC_FAR * pvContext);

    //
    // IWMReaderAdvanced interface
    //
    virtual HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
                                                /* [in] */ HRESULT hr,
                                                /* [in] */ WMT_ATTR_DATATYPE dwType,
                                                /* [in] */ BYTE __RPC_FAR * pValue,
                                                /* [in] */ void __RPC_FAR * pvContext);

    virtual HRESULT STDMETHODCALLTYPE OnStreamSample( /* [in] */ WORD wStreamNum,
                                                      /* [in] */ QWORD cnsSampleTime,
                                                      /* [in] */ QWORD cnsSampleDuration,
                                                      /* [in] */ DWORD dwFlags,
                                                      /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                                      /* [in] */ void __RPC_FAR * pvContext);

    virtual HRESULT STDMETHODCALLTYPE OnTime( /* [in] */ QWORD qwCurrentTime,
                                              /* [in] */ void __RPC_FAR * pvContext);

    virtual HRESULT STDMETHODCALLTYPE OnStreamSelection( /* [in] */ WORD wStreamCount,
                                                         /* [in] */ WORD __RPC_FAR * pStreamNumbers,
                                                         /* [in] */ WMT_STREAM_SELECTION __RPC_FAR * pSelections,
                                                         /* [in] */ void __RPC_FAR * pvContext);

    virtual HRESULT STDMETHODCALLTYPE OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
                                                            /* [in] */ WM_MEDIA_TYPE __RPC_FAR * pMediaType,
                                                            /* [in] */ void __RPC_FAR * pvContext );

    virtual HRESULT STDMETHODCALLTYPE AllocateForOutput( /* [in] */ DWORD dwOutputNum,
                                                         /* [in] */ DWORD cbBuffer,
                                                         /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                                         /* [in] */ void __RPC_FAR * pvContext);

    virtual HRESULT STDMETHODCALLTYPE AllocateForStream( /* [in] */ WORD wStreamNum,
                                                         /* [in] */ DWORD cbBuffer,
                                                         /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                                         /* [in] */ void __RPC_FAR * pvContext);

    //
    // IUnknown interface
    //
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid,
                                                      /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR * ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

protected:
    
    HRESULT CreateReader( const WCHAR * pwszInputFile );

    HRESULT GetProfileInfo();

    HRESULT CreateWriter( const WCHAR * pwszOutputFile );

    HRESULT CopyAttribute();

    HRESULT CopyCodecInfo();

    HRESULT CopyScriptInHeader();

    HRESULT Process();

    HRESULT CopyMarker( const WCHAR * pwszOutputFile );

    HRESULT CopyScriptInList( const WCHAR * pwszOutputFile );

    HRESULT WaitForCompletion();

    IWMWriter               * m_pWriter;
    IWMWriterAdvanced       * m_pWriterAdvanced;
    IWMHeaderInfo           * m_pWriterHeaderInfo;
    IWMReader               * m_pReader;
    IWMReaderAdvanced       * m_pReaderAdvanced;
    IWMHeaderInfo           * m_pReaderHeaderInfo;
    IWMProfile              * m_pReaderProfile;

    DWORD                   m_dwStreamCount;
    GUID                    * m_pguidStreamType;
    WORD                    * m_pwStreamNumber;

    HANDLE                  m_hEvent;
    HRESULT                 m_hr;
    QWORD                   m_qwReaderTime;
    BOOL                    m_fEOF;
    LONG                    m_cRef;
    BOOL                    m_fMoveScriptStream;
    QWORD                   m_qwMaxDuration;

    QWORD                   m_qwDuration;
    QWORD                   m_dwProgress;

    CScriptList             m_ScriptList;
};

#endif // !defined(AFX_WMVCopy_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)

//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            WMVRecompress.h
//
// Abstract:            Interface of CWMVRecompress class
//
//*****************************************************************************

#if !defined(AFX_WMVRecompress_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_WMVRecompress_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wmsdk.h>

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE( x )           \
        if ( NULL != x )                \
        {                               \
            x->Release( );              \
            x = NULL;                   \
        }
#endif // SAFE_RELEASE

#ifndef SAFE_ARRAYDELETE
    #define SAFE_ARRAYDELETE( x )       \
       if( x )                          \
       {                                \
           delete [] x;                 \
           x = NULL;                    \
       }
#endif //SAFE_ARRAYDELETE

#ifndef SAFE_CLOSEHANDLE
    #define SAFE_CLOSEHANDLE( h )       \
        if( NULL != h )                 \
        {                               \
            CloseHandle( h );           \
            h = NULL;                   \
        }
#endif //SAFE_CLOSEHANDLE

#ifndef SAFE_CLOSEFILEHANDLE
    #define SAFE_CLOSEFILEHANDLE( h )   \
        if( INVALID_HANDLE_VALUE != h ) \
        {                               \
            CloseHandle( h );           \
            h = INVALID_HANDLE_VALUE;   \
        }
#endif //SAFE_CLOSEFILEHANDLE


////////////////////////////////////////////////////////////////////////////////
// This class is used to recompress WMV files. It implements the methods of the 
// interfaces IWMReaderCallback and IWMReaderCallbackAdvanced                                                    
////////////////////////////////////////////////////////////////////////////////
class CWMVRecompress : public IWMReaderCallback, 
                       public IWMReaderCallbackAdvanced
{
public:
    CWMVRecompress();
    ~CWMVRecompress();

    HRESULT Recompress( const WCHAR * pwszInputFile, 
                        const WCHAR * pwszOutputFile,
                        IWMProfile * pIWMProifle,
                        BOOL fMultiPass,
                        BOOL fMultiChannel,
                        BOOL fSmartRecompression );

    static HRESULT ListSystemProfile();

    static HRESULT LoadSystemProfile( DWORD dwProfileIndex, 
                                      IWMProfile ** ppIWMProfile );

    static HRESULT LoadCustomProfile( const WCHAR * pwszProfileFile, 
                                      IWMProfile ** ppIWMProfile );

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

    HRESULT CreateWriter( const WCHAR * pwszOutputFile, IWMProfile * pProfile );

    HRESULT GetOutputMap();

    HRESULT SetWriterInput( BOOL fMultiChannel, BOOL fSmartRecompression );

    HRESULT Preprocess();

    HRESULT Process();

    HRESULT WaitForCompletion();

    IWMWriter               * m_pWriter;
    IWMWriterPreprocess     * m_pWriterPreprocess;
    IWMHeaderInfo           * m_pWriterHeaderInfo;
    IWMReader               * m_pReader;
    IWMReaderAdvanced       * m_pReaderAdvanced;
    IWMHeaderInfo           * m_pReaderHeaderInfo;
    IWMProfile              * m_pReaderProfile;

    DWORD                   m_cWriterInput;
    DWORD                   m_cReaderOutput;
    DWORD                   * m_pdwPreprocessPass;
    DWORD                   * m_pdwOutputToInput;
    DWORD                   * m_pdwOutputToStream;

    BOOL                    m_bPreprocessing;
    HANDLE                  m_hEvent;
    HRESULT                 m_hr;
    QWORD                   m_qwReaderTime;
    BOOL                    m_fEOF;
    LONG                    m_cRef;

    QWORD                   m_qwDuration;
    QWORD                   m_dwProgress;
};

#endif // !defined(AFX_WMVRecompress_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)

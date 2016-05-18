/////////////////////////////////////////////////////////////////////////////
//
// [!output root].h : Declaration of C[!output Safe_root]
//
// Note: Requires DirectX 8 SDK or later.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////
  
#ifndef __C[!output SAFE_ROOT]_H_
#define __C[!output SAFE_ROOT]_H_

#include "resource.h"
#include <mediaobj.h>       // The IMediaObject header from the DirectX SDK.
[!if DUALMODE]
#define MFT_UNIQUE_METHOD_NAMES // Makes IMFTransform method names unique so that they can be                               // implemented on the same interface as IMediaObject.
                                // implemented on the same interface as IMediaObject.
#include <mftransform.h>    // The IMFTransform header from Windows SDK.
#include <mfidl.h>          // For IMFGetService definition.
[!endif]
#include "wmpservices.h"    // The header containing the WMP interface definitions.
#include "./[!output root]PS/[!output root]_h.h"  // Generated from the IDL file during proxy/stub compilation.

const DWORD UNITS = 10000000;  // 1 sec = 1 * UNITS
const DWORD MAXSTRING = 1024;

// registry location for preferences
const WCHAR kwszPrefsRegKey[] = L"Software\\[!output root]\\DSP Plugin";
const WCHAR kwszPrefsScaleFactor[] = L"ScaleFactor";

// {[!output CLASSID]}
DEFINE_GUID(CLSID_[!output Safe_root], [!output DEFINEGUID]);

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE C[!output Safe_root] : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<C[!output Safe_root], &CLSID_[!output Safe_root]>,
    public I[!output Safe_root],
    public IMediaObject,
[!if DUALMODE]
    public IMFTransform,
[!endif]
    public IWMPPluginEnable,
[!if DUALMODE]
    public IMFGetService,
[!endif]
    public ISpecifyPropertyPages
{
public:
    C[!output Safe_root]();
    virtual ~C[!output Safe_root]();

DECLARE_REGISTRY_RESOURCEID(IDR_[!output SAFE_ROOT])

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(C[!output Safe_root])
    COM_INTERFACE_ENTRY(I[!output Safe_root])
    COM_INTERFACE_ENTRY(IMediaObject)
[!if DUALMODE]
    COM_INTERFACE_ENTRY(IMFTransform)
[!endif]
    COM_INTERFACE_ENTRY(IWMPPluginEnable)
[!if DUALMODE]
    COM_INTERFACE_ENTRY(IMFGetService)
[!endif]
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
END_COM_MAP()

    // CComCoClass Overrides
    HRESULT FinalConstruct();
    void    FinalRelease();

    // I[!output Safe_root] methods
    STDMETHOD(get_scale)(double *pVal);
    STDMETHOD(put_scale)(double newVal);

    // IMediaObject methods
    STDMETHOD( GetStreamCount )( 
                   DWORD *pcInputStreams,
                   DWORD *pcOutputStreams
                   );
    
    STDMETHOD( GetInputStreamInfo )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( GetOutputStreamInfo )( 
                   DWORD dwOutputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( GetInputType )( 
                   DWORD dwInputStreamIndex,
                   DWORD dwTypeIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetOutputType )( 
                   DWORD dwOutputStreamIndex,
                   DWORD dwTypeIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( SetInputType )( 
                   DWORD dwInputStreamIndex,
                   const DMO_MEDIA_TYPE *pmt,
                   DWORD dwFlags
                   );
    
    STDMETHOD( SetOutputType )( 
                   DWORD dwOutputStreamIndex,
                   const DMO_MEDIA_TYPE *pmt,
                   DWORD dwFlags
                   );
    
    STDMETHOD( GetInputCurrentType )( 
                   DWORD dwInputStreamIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetOutputCurrentType )( 
                   DWORD dwOutputStreamIndex,
                   DMO_MEDIA_TYPE *pmt
                   );
    
    STDMETHOD( GetInputSizeInfo )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pcbSize,
                   DWORD *pcbMaxLookahead,
                   DWORD *pcbAlignment
                   );
    
    STDMETHOD( GetOutputSizeInfo )( 
                   DWORD dwOutputStreamIndex,
                   DWORD *pcbSize,
                   DWORD *pcbAlignment
                   );
    
    STDMETHOD( GetInputMaxLatency )( 
                   DWORD dwInputStreamIndex,
                   REFERENCE_TIME *prtMaxLatency
                   );
    
    STDMETHOD( SetInputMaxLatency )( 
                   DWORD dwInputStreamIndex,
                   REFERENCE_TIME rtMaxLatency
                   );
    
    STDMETHOD( Flush )( void );
    
    STDMETHOD( Discontinuity )( 
                   DWORD dwInputStreamIndex
                   );
    
    STDMETHOD( AllocateStreamingResources )( void );
    
    STDMETHOD( FreeStreamingResources )( void );
    
    STDMETHOD( GetInputStatus )( 
                   DWORD dwInputStreamIndex,
                   DWORD *pdwFlags
                   );
    
    STDMETHOD( ProcessInput )( 
                   DWORD dwInputStreamIndex,
                   IMediaBuffer *pBuffer,
                   DWORD dwFlags,
                   REFERENCE_TIME rtTimestamp,
                   REFERENCE_TIME rtTimelength
                   );
    
    STDMETHOD( ProcessOutput )( 
                   DWORD dwFlags,
                   DWORD cOutputBufferCount,
                   DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                   DWORD *pdwStatus
                   );

    STDMETHOD( Lock )( LONG bLock );

    // Note: need to override CComObjectRootEx::Lock to avoid
    // ambiguity with IMediaObject::Lock. The override just
    // calls through to the base class implementation.

    // CComObjectRootEx overrides
    void Lock()
    {
        CComObjectRootEx<CComMultiThreadModel>::Lock();
    }

[!if DUALMODE]
    // IMFTransform methods
    STDMETHOD( MFTAddInputStreams )( DWORD cStreams, DWORD* adwStreamIDs ) {return E_NOTIMPL;}
    STDMETHOD( MFTDeleteInputStream )( DWORD dwStreamID ) {return E_NOTIMPL;}
    STDMETHOD( GetAttributes )( IMFAttributes** pAttributes ) {return E_NOTIMPL;}
    STDMETHOD( MFTGetInputAvailableType )( DWORD dwInputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType );
    STDMETHOD( MFTGetInputCurrentType )( DWORD dwInputStreamID, IMFMediaType** ppType );
    STDMETHOD( MFTGetInputStatus )( DWORD dwInputStreamID, DWORD* pdwFlags );
    STDMETHOD( GetInputStreamAttributes )( DWORD  dwInputStreamID, IMFAttributes** ppAttributes ) {return E_NOTIMPL;}
    STDMETHOD( MFTGetInputStreamInfo )( DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo );
    STDMETHOD( MFTGetOutputAvailableType )( DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType );
    STDMETHOD( MFTGetOutputCurrentType )( DWORD dwOutputStreamID, IMFMediaType** ppType );
    STDMETHOD( MFTGetOutputStatus )( DWORD* pdwFlags ) {return E_NOTIMPL;}
    STDMETHOD( GetOutputStreamAttributes )( DWORD dwOutputStreamID, IMFAttributes** ppAttributes ) {return E_NOTIMPL;}
    STDMETHOD( MFTGetOutputStreamInfo )( DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo );
    STDMETHOD( MFTGetStreamCount )( DWORD* pcInputStreams, DWORD* pcOutputStreams );
    STDMETHOD( MFTGetStreamIDs )( DWORD dwInputIDArraySize, DWORD* pdwInputIDs, DWORD dwOutputIDArraySize, DWORD* pdwOutputIDs ) {return E_NOTIMPL;}
    STDMETHOD( MFTGetStreamLimits )( DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum );
    STDMETHOD( MFTProcessEvent )( DWORD  dwInputStreamID, IMFMediaEvent* pEvent ) {return E_NOTIMPL;}
    STDMETHOD( MFTProcessInput )( DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags );
    STDMETHOD( MFTProcessMessage )( MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam );
    STDMETHOD( MFTProcessOutput )( DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus );
    STDMETHOD( MFTSetInputType )( DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags );
    STDMETHOD( MFTSetOutputBounds )( LONGLONG hnsLowerBound, LONGLONG hnsUpperBound ) {return E_NOTIMPL;}
    STDMETHOD( MFTSetOutputType )( DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags );
[!endif]

    // IWMPPluginEnable methods
    STDMETHOD( SetEnable )( BOOL fEnable );
    STDMETHOD( GetEnable )( BOOL *pfEnable );

[!if DUALMODE]
    // IMFGetService methods
    STDMETHOD( GetService )( REFGUID guidService, REFIID riid, LPVOID* ppvObject );
[!endif]

    // ISpecifyPropertyPages methods
    STDMETHOD( GetPages )(CAUUID *pPages);
    
private:
    HRESULT DoProcessOutput(
                BYTE *pbOutputData,             // Pointer to the output buffer
                const BYTE *pbInputData,        // Pointer to the input buffer
                DWORD *cbBytesProcessed);       // Number of bytes processed
    HRESULT ValidateMediaType(
                const DMO_MEDIA_TYPE *pmtTarget,    // target media type to verify
                const DMO_MEDIA_TYPE *pmtPartner);  // partner media type to verify

[!if DUALMODE]
    HRESULT CreateMFMediaType( DMO_MEDIA_TYPE* pmtDMOType, IMFMediaType** ppMFType );
[!endif]

    DMO_MEDIA_TYPE          m_mtInput;          // Stores the input format structure
    DMO_MEDIA_TYPE          m_mtOutput;         // Stores the output format structure

    CComPtr<IMediaBuffer>   m_spInputBuffer;    // Smart pointer to the input buffer object

[!if DUALMODE] 
   CComPtr<IMFSample>      m_spMFSample;       // Smart pointer to the input MF sample
[!endif]

    BYTE*                   m_pbInputData;      // Pointer to the data in the input buffer
    DWORD                   m_cbInputLength;    // Length of the data in the input buffer
   
    bool                    m_bValidTime;       // Is timestamp valid?
    REFERENCE_TIME          m_rtTimestamp;      // Stores the input buffer timestamp

    double                  m_fScaleFactor;     // Scale factor
    BOOL                    m_bEnabled;         // TRUE if enabled
};

#endif //__C[!output SAFE_ROOT]_H_

#pragma once

#include "Common.h"
#include "display.h"

namespace DX11VideoRenderer
{
    class CPresenter :
        public IMFVideoDisplayControl,
        public IMFGetService,
        private CBase
    {
    public:
        CPresenter(void);
        virtual ~CPresenter(void);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);
        STDMETHODIMP_(ULONG) Release(void);

        // IMFVideoDisplayControl
        STDMETHODIMP GetAspectRatioMode(__RPC__out DWORD* pdwAspectRatioMode) { return E_NOTIMPL; }
        STDMETHODIMP GetBorderColor(__RPC__out COLORREF* pClr) { return E_NOTIMPL; }
        STDMETHODIMP GetCurrentImage(__RPC__inout BITMAPINFOHEADER* pBih, __RPC__deref_out_ecount_full_opt(*pcbDib) BYTE** pDib, __RPC__out DWORD* pcbDib, __RPC__inout_opt LONGLONG* pTimestamp) { return E_NOTIMPL; }
        STDMETHODIMP GetFullscreen(__RPC__out BOOL* pfFullscreen);
        STDMETHODIMP GetIdealVideoSize(__RPC__inout_opt SIZE* pszMin, __RPC__inout_opt SIZE* pszMax) { return E_NOTIMPL; }
        STDMETHODIMP GetNativeVideoSize(__RPC__inout_opt SIZE* pszVideo, __RPC__inout_opt SIZE* pszARVideo) { return E_NOTIMPL; }
        STDMETHODIMP GetRenderingPrefs(__RPC__out DWORD* pdwRenderFlags) { return E_NOTIMPL; }
        STDMETHODIMP GetVideoPosition(__RPC__out MFVideoNormalizedRect* pnrcSource, __RPC__out LPRECT prcDest) { return E_NOTIMPL; }
        STDMETHODIMP GetVideoWindow(__RPC__deref_out_opt HWND* phwndVideo) { return E_NOTIMPL; }
        STDMETHODIMP RepaintVideo(void) { return E_NOTIMPL; }
        STDMETHODIMP SetAspectRatioMode(DWORD dwAspectRatioMode) { return E_NOTIMPL; }
        STDMETHODIMP SetBorderColor(COLORREF Clr) { return E_NOTIMPL; }
        STDMETHODIMP SetFullscreen(BOOL fFullscreen);
        STDMETHODIMP SetRenderingPrefs(DWORD dwRenderingPrefs) { return E_NOTIMPL; }
        STDMETHODIMP SetVideoPosition(__RPC__in_opt const MFVideoNormalizedRect* pnrcSource, __RPC__in_opt const LPRECT prcDest) { return E_NOTIMPL; }
        STDMETHODIMP SetVideoWindow(__RPC__in HWND hwndVideo);

        // IMFGetService
        STDMETHODIMP GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject);

        BOOL    CanProcessNextSample(void);
        HRESULT Flush(void);
        HRESULT GetMonitorRefreshRate(DWORD* pdwMonitorRefreshRate);
        HRESULT IsMediaTypeSupported(IMFMediaType* pMediaType, DXGI_FORMAT dxgiFormat);
        HRESULT PresentFrame(void);
        HRESULT ProcessFrame(IMFMediaType* pCurrentType, IMFSample* pSample, UINT32* punInterlaceMode, BOOL* pbDeviceChanged, BOOL* pbProcessAgain, IMFSample** ppOutputSample = NULL);
        HRESULT SetCurrentMediaType(IMFMediaType* pMediaType);
        HRESULT Shutdown(void);

    private:

        void    AspectRatioCorrectSize(
                    LPSIZE lpSizeImage,     // size to be aspect ratio corrected
                    const SIZE& sizeAr,     // aspect ratio of image
                    const SIZE& sizeOrig,   // original image size
                    BOOL ScaleXorY          // axis to correct in
                    );
        void    CheckDecodeSwitchRegKey(void);
        HRESULT CheckDeviceState(BOOL* pbDeviceChanged);
        BOOL    CheckEmptyRect(RECT* pDst);
        HRESULT CheckShutdown(void) const;
        HRESULT CreateDCompDeviceAndVisual(void);
        HRESULT CreateDXGIManagerAndDevice(D3D_DRIVER_TYPE DriverType=D3D_DRIVER_TYPE_HARDWARE);
        HRESULT CreateXVP(void);
        HRESULT FindBOBProcessorIndex(DWORD* pIndex);
        HRESULT GetVideoDisplayArea(IMFMediaType* pType, MFVideoArea* pArea);
        void    LetterBoxDstRect(
                    LPRECT lprcLBDst,   // output letterboxed rectangle
                    const RECT& rcSrc,  // input source rectangle
                    const RECT& rcDst   // input destination rectangle
                    );
        void    PixelAspectToPictureAspect(
                    int Width,
                    int Height,
                    int PixelAspectX,
                    int PixelAspectY,
                    int* pPictureAspectX,
                    int* pPictureAspectY
                    );
        HRESULT ProcessFrameUsingD3D11( ID3D11Texture2D* pLeftTexture2D, ID3D11Texture2D* pRightTexture2D, UINT dwLeftViewIndex, UINT dwRightViewIndex, RECT rcDest, UINT32 unInterlaceMode, IMFSample** ppVideoOutFrame );
        HRESULT ProcessFrameUsingXVP( IMFMediaType* pCurrentType, IMFSample* pVideoFrame, ID3D11Texture2D* pTexture2D, RECT rcDest, IMFSample** ppVideoOutFrame, BOOL* pbInputFrameUsed );
        void    ReduceToLowestTerms(
                    int NumeratorIn,
                    int DenominatorIn,
                    int* pNumeratorOut,
                    int* pDenominatorOut
                    );
        HRESULT SetMonitor(UINT adapterID);
        void    SetVideoContextParameters(ID3D11VideoContext* pVideoContext, const RECT* pSRect, const RECT* pTRect, UINT32 unInterlaceMode);
        HRESULT SetVideoMonitor(HWND hwndVideo);
        HRESULT SetXVPOutputMediaType(IMFMediaType* pType, DXGI_FORMAT vpOutputFormat);
        _Post_satisfies_(this->m_pSwapChain1 != NULL)
        HRESULT UpdateDXGISwapChain(void);
        void    UpdateRectangles(RECT* pDst, RECT* pSrc);

        long                            m_nRefCount;                // reference count
        CCritSec                        m_critSec;                  // critical section for thread safety
        BOOL                            m_IsShutdown;               // Flag to indicate if Shutdown() method was called.
        IDXGIFactory2*                  m_pDXGIFactory2;
        ID3D11Device*                   m_pD3D11Device;
        ID3D11DeviceContext*            m_pD3DImmediateContext;
        IMFDXGIDeviceManager*           m_pDXGIManager;
        IDXGIOutput1*                   m_pDXGIOutput1;
        IMFVideoSampleAllocatorEx*      m_pSampleAllocatorEx;
        IDCompositionDevice*            m_pDCompDevice;
        IDCompositionTarget*            m_pHwndTarget;
        IDCompositionVisual*            m_pRootVisual;
        BOOL                            m_bSoftwareDXVADeviceInUse;
        HWND                            m_hwndVideo;
        CMonitorArray*                  m_pMonitors;
        CAMDDrawMonitorInfo*            m_lpCurrMon;
        UINT                            m_DeviceResetToken;
        UINT                            m_ConnectionGUID;
        UINT                            m_DXSWSwitch;
        UINT                            m_useXVP;
        UINT                            m_useDCompVisual;
        UINT                            m_useDebugLayer;
        ID3D11VideoDevice*              m_pDX11VideoDevice;
        ID3D11VideoProcessorEnumerator* m_pVideoProcessorEnum;
        ID3D11VideoProcessor*           m_pVideoProcessor;
        IDXGISwapChain1*                m_pSwapChain1;
        BOOL                            m_bDeviceChanged;
        BOOL                            m_bResize;
        BOOL                            m_b3DVideo;
        BOOL                            m_bStereoEnabled;
        MFVideo3DFormat                 m_vp3DOutput;
        BOOL                            m_bFullScreenState;
        BOOL                            m_bCanProcessNextSample;
        RECT                            m_displayRect;
        UINT32                          m_imageWidthInPixels;
        UINT32                          m_imageHeightInPixels;
        UINT32                          m_uiRealDisplayWidth;
        UINT32                          m_uiRealDisplayHeight;
        RECT                            m_rcSrcApp;
        RECT                            m_rcDstApp;
        IMFTransform*                   m_pXVP;
        IMFVideoProcessorControl*       m_pXVPControl;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Wrapper class for D3D11 Device and D3D11 Video device used for DXVA to Software decode switch
    class CPrivate_ID3D11VideoDevice : public ID3D11VideoDevice
    {
    private:

        ID3D11VideoDevice* m_pReal;
        ULONG m_cRef;

    public:

        CPrivate_ID3D11VideoDevice(ID3D11VideoDevice* pReal) :
            m_pReal(pReal),
            m_cRef(0)
        {
        }

        virtual ~CPrivate_ID3D11VideoDevice(void)
        {
        }

        STDMETHODIMP QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject)
        {
            if (__uuidof(ID3D11VideoDevice) == riid)
            {
                this->AddRef();
                *ppvObject = this;
                return S_OK;
            }
            else
            {
                return m_pReal->QueryInterface(riid,ppvObject);
            }
        }

        STDMETHODIMP_(ULONG) AddRef(void)
        {
            InterlockedIncrement(&m_cRef);
            return m_pReal->AddRef();
        }

        STDMETHODIMP_(ULONG) Release(void)
        {
            ULONG ulVal = m_pReal->Release();
            if (0 == InterlockedDecrement(&m_cRef))
            {
            }
            return ulVal;
        }

        STDMETHODIMP CreateVideoDecoder(
            _In_  const D3D11_VIDEO_DECODER_DESC* pVideoDesc,
            _In_  const D3D11_VIDEO_DECODER_CONFIG* pConfig,
            _Out_  ID3D11VideoDecoder** ppDecoder)
        {
            return E_FAIL;
        }

        STDMETHODIMP CreateVideoProcessor(
            _In_  ID3D11VideoProcessorEnumerator* pEnum,
            _In_  UINT RateConversionIndex,
            _Out_  ID3D11VideoProcessor** ppVideoProcessor)
        {
            return m_pReal->CreateVideoProcessor(pEnum,RateConversionIndex,ppVideoProcessor);
        }

        STDMETHODIMP CreateAuthenticatedChannel(
            _In_  D3D11_AUTHENTICATED_CHANNEL_TYPE ChannelType,
            _Out_  ID3D11AuthenticatedChannel** ppAuthenticatedChannel)
        {
            return m_pReal->CreateAuthenticatedChannel(ChannelType,ppAuthenticatedChannel);
        }

        STDMETHODIMP CreateCryptoSession(
            _In_  const GUID* pCryptoType,
            _In_opt_  const GUID* pDecoderProfile,
            _In_  const GUID* pKeyExchangeType,
            _Outptr_  ID3D11CryptoSession** ppCryptoSession)
        {
            return m_pReal->CreateCryptoSession(pCryptoType,pDecoderProfile,pKeyExchangeType,ppCryptoSession);
        }

        STDMETHODIMP CreateVideoDecoderOutputView(
            _In_  ID3D11Resource* pResource,
            _In_  const D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11VideoDecoderOutputView** ppVDOVView)
        {
            return m_pReal->CreateVideoDecoderOutputView(pResource,pDesc,ppVDOVView);
        }

        STDMETHODIMP CreateVideoProcessorInputView(
            _In_  ID3D11Resource* pResource,
            _In_  ID3D11VideoProcessorEnumerator* pEnum,
            _In_  const D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11VideoProcessorInputView** ppVPIView)
        {
            return m_pReal->CreateVideoProcessorInputView(pResource,pEnum,pDesc,ppVPIView);
        }

        STDMETHODIMP CreateVideoProcessorOutputView(
            _In_  ID3D11Resource* pResource,
            _In_  ID3D11VideoProcessorEnumerator* pEnum,
            _In_  const D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11VideoProcessorOutputView** ppVPOView)
        {
            return m_pReal->CreateVideoProcessorOutputView(pResource,pEnum,pDesc,ppVPOView);
        }

        STDMETHODIMP CreateVideoProcessorEnumerator(
            _In_  const D3D11_VIDEO_PROCESSOR_CONTENT_DESC* pDesc,
            _Out_  ID3D11VideoProcessorEnumerator** ppEnum)
        {
            return m_pReal->CreateVideoProcessorEnumerator(pDesc,ppEnum);
        }

        STDMETHODIMP_(UINT) GetVideoDecoderProfileCount(void)
        {
            return m_pReal->GetVideoDecoderProfileCount();
        }

        STDMETHODIMP GetVideoDecoderProfile(
            _In_  UINT Index,
            _Out_  GUID* pDecoderProfile)
        {
            return m_pReal->GetVideoDecoderProfile(Index,pDecoderProfile);
        }

        STDMETHODIMP CheckVideoDecoderFormat(
            _In_  const GUID* pDecoderProfile,
            _In_  DXGI_FORMAT Format,
            _Out_  BOOL* pSupported)
        {
            return m_pReal->CheckVideoDecoderFormat(pDecoderProfile,Format,pSupported);
        }

        STDMETHODIMP GetVideoDecoderConfigCount(
            _In_  const D3D11_VIDEO_DECODER_DESC* pDesc,
            _Out_  UINT* pCount)
        {
            return m_pReal->GetVideoDecoderConfigCount(pDesc,pCount);
        }

        STDMETHODIMP GetVideoDecoderConfig(
            _In_  const D3D11_VIDEO_DECODER_DESC* pDesc,
            _In_  UINT Index,
            _Out_  D3D11_VIDEO_DECODER_CONFIG* pConfig)
        {
            return m_pReal->GetVideoDecoderConfig(pDesc,Index,pConfig);
        }

        STDMETHODIMP GetContentProtectionCaps(
            _In_opt_  const GUID* pCryptoType,
            _In_opt_  const GUID* pDecoderProfile,
            _Out_  D3D11_VIDEO_CONTENT_PROTECTION_CAPS* pCaps)
        {
            return m_pReal->GetContentProtectionCaps(pCryptoType,pDecoderProfile,pCaps);
        }

        STDMETHODIMP CheckCryptoKeyExchange(
            _In_  const GUID* pCryptoType,
            _In_opt_  const GUID* pDecoderProfile,
            _In_  UINT Index,
            _Out_  GUID* pKeyExchangeType)
        {
            return m_pReal->CheckCryptoKeyExchange(pCryptoType,pDecoderProfile,Index,pKeyExchangeType);
        }

        STDMETHODIMP SetPrivateData(
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_(DataSize)  const void* pData)
        {
            return m_pReal->SetPrivateData(guid,DataSize,pData);
        }

        STDMETHODIMP SetPrivateDataInterface(
            _In_  REFGUID guid,
            _In_opt_  const IUnknown* pData)
        {
            return m_pReal->SetPrivateDataInterface(guid,pData);
        }
    };

    class CPrivate_ID3D11Device : public ID3D11Device
    {
    private:

        ID3D11Device* m_pReal;
        ULONG m_cRef;
        CPrivate_ID3D11VideoDevice* m_pVideoDevice;

    public:

        CPrivate_ID3D11Device(ID3D11Device* pReal) :
            m_pReal(pReal),
            m_cRef(1),
            m_pVideoDevice(NULL)
        {
            ID3D11VideoDevice* pDevice;
            m_pReal->QueryInterface(__uuidof(ID3D11VideoDevice),(void**)&pDevice);
            m_pVideoDevice = new CPrivate_ID3D11VideoDevice(pDevice);
            if (pDevice != NULL)
            {
                pDevice->Release();
            }
        }

        virtual ~CPrivate_ID3D11Device(void)
        {
            SafeDelete(m_pVideoDevice);
        }

        STDMETHODIMP QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject)
        {
            if (__uuidof(ID3D11VideoDevice) == riid)
            {
                m_pVideoDevice->AddRef();
                *ppvObject = m_pVideoDevice;
                return S_OK;
            }
            else if (__uuidof(ID3D11Device) == riid)
            {
                this->AddRef();
                *ppvObject = this;
                return S_OK;
            }
            else
            {
                return m_pReal->QueryInterface(riid,ppvObject);
            }
        }

        STDMETHODIMP_(ULONG) AddRef(void)
        {
            InterlockedIncrement(&m_cRef);
            return m_pReal->AddRef();
        }

        STDMETHODIMP_(ULONG) Release(void)
        {
            ULONG ulVal = m_pReal->Release();
            if (0 == InterlockedDecrement(&m_cRef))
            {
                delete this;
            }
            return ulVal;
        }
    
        STDMETHODIMP CreateBuffer(
            _In_  const D3D11_BUFFER_DESC* pDesc,
            _In_opt_  const D3D11_SUBRESOURCE_DATA* pInitialData,
            _Out_opt_  ID3D11Buffer** ppBuffer)
        {
            return m_pReal->CreateBuffer(pDesc,pInitialData,ppBuffer);
        }
    
        STDMETHODIMP CreateTexture1D(
            _In_  const D3D11_TEXTURE1D_DESC* pDesc,
            _In_reads_opt_(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA* pInitialData,
            _Out_opt_  ID3D11Texture1D** ppTexture1D)
        {
            return m_pReal->CreateTexture1D(pDesc,pInitialData,ppTexture1D);
        }

        STDMETHODIMP CreateTexture2D(
            _In_  const D3D11_TEXTURE2D_DESC* pDesc,
            _In_reads_opt_(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA* pInitialData,
            _Out_opt_  ID3D11Texture2D** ppTexture2D)
        {
            return m_pReal->CreateTexture2D(pDesc,pInitialData,ppTexture2D);
        }

        STDMETHODIMP CreateTexture3D(
            _In_  const D3D11_TEXTURE3D_DESC* pDesc,
            _In_reads_opt_(pDesc->MipLevels)  const D3D11_SUBRESOURCE_DATA* pInitialData,
            _Out_opt_  ID3D11Texture3D** ppTexture3D)
        {
            return m_pReal->CreateTexture3D(pDesc,pInitialData,ppTexture3D);
        }

        STDMETHODIMP CreateShaderResourceView(
            _In_  ID3D11Resource* pResource,
            _In_opt_  const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11ShaderResourceView** ppSRView)
        {
            return m_pReal->CreateShaderResourceView(pResource,pDesc,ppSRView);
        }

        STDMETHODIMP CreateUnorderedAccessView(
            _In_  ID3D11Resource* pResource,
            _In_opt_  const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11UnorderedAccessView** ppUAView)
        {
            return m_pReal->CreateUnorderedAccessView(pResource,pDesc,ppUAView);
        }

        STDMETHODIMP CreateRenderTargetView(
            _In_  ID3D11Resource* pResource,
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11RenderTargetView** ppRTView)
        {
            return m_pReal->CreateRenderTargetView(pResource,pDesc,ppRTView);
        }

        STDMETHODIMP CreateDepthStencilView(
            _In_  ID3D11Resource* pResource,
            _In_opt_  const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
            _Out_opt_  ID3D11DepthStencilView** ppDepthStencilView)
        {
            return m_pReal->CreateDepthStencilView(pResource,pDesc,ppDepthStencilView);
        }

        STDMETHODIMP CreateInputLayout(
            _In_reads_(NumElements)  const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
            _In_range_( 0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT )  UINT NumElements,
            _In_  const void* pShaderBytecodeWithInputSignature,
            _In_  SIZE_T BytecodeLength,
            _Out_opt_  ID3D11InputLayout** ppInputLayout)
        {
            return m_pReal->CreateInputLayout(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout);
        }

        STDMETHODIMP CreateVertexShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11VertexShader** ppVertexShader)
        {
            return m_pReal->CreateVertexShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader);
        }

        STDMETHODIMP CreateGeometryShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11GeometryShader** ppGeometryShader)
        {
            return m_pReal->CreateGeometryShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppGeometryShader);
        }

        STDMETHODIMP CreateGeometryShaderWithStreamOutput(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_reads_opt_(NumEntries)  const D3D11_SO_DECLARATION_ENTRY* pSODeclaration,
            _In_range_( 0, D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT )  UINT NumEntries,
            _In_reads_opt_(NumStrides)  const UINT* pBufferStrides,
            _In_range_( 0, D3D11_SO_BUFFER_SLOT_COUNT )  UINT NumStrides,
            _In_  UINT RasterizedStream,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11GeometryShader** ppGeometryShader)
        {
            return m_pReal->CreateGeometryShaderWithStreamOutput(pShaderBytecode,BytecodeLength,pSODeclaration,NumEntries,pBufferStrides,NumStrides,RasterizedStream,pClassLinkage,ppGeometryShader);
        }

        STDMETHODIMP CreatePixelShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11PixelShader** ppPixelShader)
        {
            return m_pReal->CreatePixelShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader);
        }

        STDMETHODIMP CreateHullShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11HullShader** ppHullShader)
        {
            return m_pReal->CreateHullShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppHullShader);
        }

        STDMETHODIMP CreateDomainShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11DomainShader** ppDomainShader)
        {
            return m_pReal->CreateDomainShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppDomainShader);
        }

        STDMETHODIMP CreateComputeShader(
            _In_  const void* pShaderBytecode,
            _In_  SIZE_T BytecodeLength,
            _In_opt_  ID3D11ClassLinkage* pClassLinkage,
            _Out_opt_  ID3D11ComputeShader** ppComputeShader)
        {
            return m_pReal->CreateComputeShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppComputeShader);
        }

        STDMETHODIMP CreateClassLinkage(
            _Out_  ID3D11ClassLinkage** ppLinkage)
        {
            return m_pReal->CreateClassLinkage(ppLinkage);
        }

        STDMETHODIMP CreateBlendState(
            _In_  const D3D11_BLEND_DESC* pBlendStateDesc,
            _Out_opt_  ID3D11BlendState** ppBlendState)
        {
            return m_pReal->CreateBlendState(pBlendStateDesc,ppBlendState);
        }

        STDMETHODIMP CreateDepthStencilState(
            _In_  const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc,
            _Out_opt_  ID3D11DepthStencilState** ppDepthStencilState)
        {
            return m_pReal->CreateDepthStencilState(pDepthStencilDesc,ppDepthStencilState);
        }

        STDMETHODIMP CreateRasterizerState(
            _In_  const D3D11_RASTERIZER_DESC* pRasterizerDesc,
            _Out_opt_  ID3D11RasterizerState** ppRasterizerState)
        {
            return m_pReal->CreateRasterizerState(pRasterizerDesc,ppRasterizerState);
        }

        STDMETHODIMP CreateSamplerState(
            _In_  const D3D11_SAMPLER_DESC* pSamplerDesc,
            _Out_opt_  ID3D11SamplerState** ppSamplerState)
        {
            return m_pReal->CreateSamplerState(pSamplerDesc,ppSamplerState);
        }

        STDMETHODIMP CreateQuery(
            _In_  const D3D11_QUERY_DESC* pQueryDesc,
            _Out_opt_  ID3D11Query** ppQuery)
        {
            return m_pReal->CreateQuery(pQueryDesc,ppQuery);
        }

        STDMETHODIMP CreatePredicate(
            _In_  const D3D11_QUERY_DESC* pPredicateDesc,
            _Out_opt_  ID3D11Predicate** ppPredicate)
        {
            return m_pReal->CreatePredicate(pPredicateDesc,ppPredicate);
        }

        STDMETHODIMP CreateCounter(
            _In_  const D3D11_COUNTER_DESC* pCounterDesc,
            _Out_opt_  ID3D11Counter** ppCounter)
        {
            return m_pReal->CreateCounter(pCounterDesc,ppCounter);
        }

        STDMETHODIMP CreateDeferredContext(
            UINT ContextFlags,
            _Out_opt_  ID3D11DeviceContext** ppDeferredContext)
        {
            return m_pReal->CreateDeferredContext(ContextFlags,ppDeferredContext);
        }

        STDMETHODIMP OpenSharedResource(
            _In_  HANDLE hResource,
            _In_  REFIID ReturnedInterface,
            _Out_opt_  void** ppResource)
        {
            return m_pReal->OpenSharedResource(hResource,ReturnedInterface,ppResource);
        }

        STDMETHODIMP CheckFormatSupport(
            _In_  DXGI_FORMAT Format,
            _Out_  UINT* pFormatSupport)
        {
            return m_pReal->CheckFormatSupport(Format,pFormatSupport);
        }

        STDMETHODIMP CheckMultisampleQualityLevels(
            _In_  DXGI_FORMAT Format,
            _In_  UINT SampleCount,
            _Out_  UINT* pNumQualityLevels)
        {
            return m_pReal->CheckMultisampleQualityLevels(Format,SampleCount,pNumQualityLevels);
        }

        STDMETHODIMP_(void) CheckCounterInfo(
            _Out_  D3D11_COUNTER_INFO* pCounterInfo)
        {
            return m_pReal->CheckCounterInfo(pCounterInfo);
        }

        STDMETHODIMP CheckCounter(
            _In_  const D3D11_COUNTER_DESC* pDesc,
            _Out_  D3D11_COUNTER_TYPE* pType,
            _Out_  UINT* pActiveCounters,
            _Out_writes_opt_(*pNameLength)  LPSTR szName,
            _Inout_opt_  UINT* pNameLength,
            _Out_writes_opt_(*pUnitsLength)  LPSTR szUnits,
            _Inout_opt_  UINT* pUnitsLength,
            _Out_writes_opt_(*pDescriptionLength)  LPSTR szDescription,
            _Inout_opt_  UINT* pDescriptionLength)
        {
            return m_pReal->CheckCounter(pDesc,pType,pActiveCounters,szName,pNameLength,szUnits,pUnitsLength,szDescription,pDescriptionLength);
        }

        STDMETHODIMP CheckFeatureSupport(
            D3D11_FEATURE Feature,
            _Out_writes_bytes_(FeatureSupportDataSize)  void* pFeatureSupportData,
            UINT FeatureSupportDataSize)
        {
            return m_pReal->CheckFeatureSupport(Feature,pFeatureSupportData,FeatureSupportDataSize);
        }

        STDMETHODIMP GetPrivateData(
            _In_  REFGUID guid,
            _Inout_  UINT* pDataSize,
            _Out_writes_bytes_opt_(*pDataSize)  void* pData)
        {
            return m_pReal->GetPrivateData(guid,pDataSize,pData);
        }

        STDMETHODIMP SetPrivateData(
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_(DataSize)  const void* pData)
        {
            return m_pReal->SetPrivateData(guid,DataSize,pData);
        }

        STDMETHODIMP SetPrivateDataInterface(
            _In_  REFGUID guid,
            _In_opt_  const IUnknown* pData)
        {
            return m_pReal->SetPrivateDataInterface(guid,pData);
        }

        STDMETHODIMP_(D3D_FEATURE_LEVEL) GetFeatureLevel(void)
        {
            return m_pReal->GetFeatureLevel();
        }

        STDMETHODIMP_(UINT) GetCreationFlags(void)
        {
            return m_pReal->GetCreationFlags();
        }

        STDMETHODIMP GetDeviceRemovedReason(void)
        {
            return m_pReal->GetDeviceRemovedReason();
        }

        STDMETHODIMP_(void) GetImmediateContext(
            _Out_  ID3D11DeviceContext** ppImmediateContext)
        {
            return m_pReal->GetImmediateContext(ppImmediateContext);
        }

        STDMETHODIMP SetExceptionMode(
            UINT RaiseFlags)
        {
            return m_pReal->SetExceptionMode(RaiseFlags);
        }

        STDMETHODIMP_(UINT) GetExceptionMode(void)
        {
            return m_pReal->GetExceptionMode();
        }
    };
}

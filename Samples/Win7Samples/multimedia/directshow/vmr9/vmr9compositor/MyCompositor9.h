//////////////////////////////////////////////////////////////////////////
// MyCompositor.h: Defines the custom compositor.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

const int NUM_VERTICES = 18;
const int MAX_STREAMS = 3;

class CMyCompositor9 : public IVMRImageCompositor9
{
public:
    CMyCompositor9(void);
    virtual ~CMyCompositor9(void);

public:
    // IVMRImageCompositor9
    virtual HRESULT STDMETHODCALLTYPE InitCompositionDevice( 
        /* [in] */ IUnknown *pD3DDevice);
    
    virtual HRESULT STDMETHODCALLTYPE TermCompositionDevice( 
        /* [in] */ IUnknown *pD3DDevice);
    
    virtual HRESULT STDMETHODCALLTYPE SetStreamMediaType( 
        /* [in] */ DWORD dwStrmID,
        /* [in] */ AM_MEDIA_TYPE *pmt,
        /* [in] */ BOOL fTexture);
    
    virtual HRESULT STDMETHODCALLTYPE CompositeImage( 
        /* [in] */ IUnknown *pD3DDevice,
        /* [in] */ IDirect3DSurface9 *pddsRenderTarget,
        /* [in] */ AM_MEDIA_TYPE *pmtRenderTarget,
        /* [in] */ REFERENCE_TIME rtStart,
        /* [in] */ REFERENCE_TIME rtEnd,
        /* [in] */ D3DCOLOR dwClrBkGnd,
        /* [in] */ VMR9VideoStreamInfo *pVideoStreamInfo,
        /* [in] */ UINT cStreams);
    
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void** ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    void ChangeX( int X );
    void ChangeY( int Y );

protected:
    HRESULT AdjustViewMatrix( IDirect3DDevice9* d3ddev );
    static HRESULT SetUpFog( IDirect3DDevice9* d3ddev );
    HRESULT CreateTexture( IDirect3DDevice9* d3ddev, DWORD x, DWORD y  );

	SmartPtr<IDirect3DTexture9> GetTexture( 
							IDirect3DDevice9* d3ddev,
							VMR9VideoStreamInfo *pVideoStreamInfo
							);

private:
    struct CUSTOMVERTEX
    {
        struct Position {
            Position() : 
                x(0.0f),y(0.0f),z(0.0f) {            
            };
            Position(float x_, float y_, float z_) :
                x(x_),y(y_),z(z_) {
            };
            float x,y,z;
        };

        Position    position; // The position
        FLOAT       tu, tv;   // The texture coordinates
    };

    CUSTOMVERTEX                    m_vertices[NUM_VERTICES];
    SmartPtr<IDirect3DTexture9>      m_texture;
    SmartPtr<IDirect3DSurface9>      m_zSurface;
    SmartPtr<IDirect3DVertexBuffer9> m_vertexBuffer;

    bool m_needTurn;
    bool m_createTexture;
    long m_refCount;
    int m_x;
    int m_y;
};

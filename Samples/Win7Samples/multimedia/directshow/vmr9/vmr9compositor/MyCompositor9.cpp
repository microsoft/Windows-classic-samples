//////////////////////////////////////////////////////////////////////////
// MyCompositor.cpp : Defines the custom compositor
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "vmr9compositor.h"
#include "MyCompositor9.h"
#include <strsafe.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_TEX1 )

// Matrix functions
D3DMATRIX* MatrixPerspectiveFovLH(
    D3DMATRIX * pOut,
    FLOAT fovy,
    FLOAT Aspect,
    FLOAT zn,
    FLOAT zf
    );


D3DMATRIX* MatrixLookAtLH( 
    D3DMATRIX *pOut, 
    const D3DVECTOR *pEye, 
    const D3DVECTOR *pAt,
    const D3DVECTOR *pUp 
    );

CMyCompositor9::CMyCompositor9(void)
: m_refCount( 1 )
, m_needTurn( true ) // first turn needs to happen
, m_x( 0 )
, m_y( 0 )
, m_createTexture(false)
{
    m_vertices[0].position =    CUSTOMVERTEX::Position( -1.0f,  0.0f, -1.0f); // - forward bottom left
    m_vertices[1].position =    CUSTOMVERTEX::Position(  0.0f,  2.0f,  0.0f); // - tip
    m_vertices[2].position =    CUSTOMVERTEX::Position(  1.0f,  0.0f, -1.0f); // - forward bottom right
    m_vertices[3].position =    CUSTOMVERTEX::Position(  1.0f,  0.0f,  1.0f); // - back bottom right 
    
    m_vertices[4].position =    CUSTOMVERTEX::Position( -1.0f,  0.0f, -1.0f ); // - forward bottom left
    m_vertices[5].position =    CUSTOMVERTEX::Position(  0.0f,  2.0f,  0.0f ); // - tip
    m_vertices[6].position =    CUSTOMVERTEX::Position( -1.0f,  0.0f,  1.0f);  // - back bottom left
    m_vertices[7].position =    CUSTOMVERTEX::Position(  1.0f,  0.0f,  1.0f);  // - back bottom right
    
    m_vertices[8].position =    CUSTOMVERTEX::Position( -1.0f,  0.0f, -1.0f ); // - forward  
    m_vertices[9].position =    CUSTOMVERTEX::Position( -1.0f,  0.0f,  1.0f ); // - back
    m_vertices[10].position =   CUSTOMVERTEX::Position(  1.0f,  0.0f, -1.0f ); // - forward 
    m_vertices[11].position =   CUSTOMVERTEX::Position(  1.0f,  0.0f,  1.0f ); // - back


    // set up texture coordinates
    m_vertices[0].tu = 0.0f; m_vertices[0].tv = 0.0f; // low left
    m_vertices[1].tu = 0.0f; m_vertices[1].tv = 1.0f; // high left
    m_vertices[2].tu = 1.0f; m_vertices[2].tv = 0.0f; // low right
    m_vertices[3].tu = 1.0f; m_vertices[3].tv = 1.0f; // high right
    
    m_vertices[4].tu = 0.0f; m_vertices[4].tv = 0.0f; 
    m_vertices[5].tu = 0.0f; m_vertices[5].tv = 1.0f; 
    m_vertices[6].tu = 1.0f; m_vertices[6].tv = 0.0f; 
    m_vertices[7].tu = 1.0f; m_vertices[7].tv = 1.0f; 
    
    m_vertices[8].tu =  0.0f; m_vertices[8].tv =  0.0f; 
    m_vertices[9].tu =  0.0f; m_vertices[9].tv =  1.0f; 
    m_vertices[10].tu = 1.0f; m_vertices[10].tv = 0.0f; 
    m_vertices[11].tu = 1.0f; m_vertices[11].tv = 1.0f; 

}

CMyCompositor9::~CMyCompositor9(void)
{
}

// IVMRImageCompositor9
HRESULT STDMETHODCALLTYPE CMyCompositor9::InitCompositionDevice( 
    /* [in] */ IUnknown *pD3DDevice)
{
    HRESULT hr;
    SmartPtr<IDirect3DDevice9> d3ddev;
    
    hr = pD3DDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void**)&d3ddev);
    ASSERT( d3ddev );

    //FAIL_RET( SetUpFog( d3ddev ) );
    //
    // Set the projection matrix
    //
    SmartPtr<IDirect3DSurface9> backBuffer;
    FAIL_RET( d3ddev->GetRenderTarget( 0, &backBuffer ) );

    D3DSURFACE_DESC backBufferDesc;
    backBuffer->GetDesc( & backBufferDesc );

    FAIL_RET( AdjustViewMatrix( d3ddev ) );

    D3DMATRIX matProj;
    FLOAT fAspect = backBufferDesc.Width / 
                    (float)backBufferDesc.Height;
    MatrixPerspectiveFovLH( &matProj, (float)M_PI_4, fAspect, 
                                1.0f, 100.0f );
    FAIL_RET( d3ddev->SetTransform( D3DTS_PROJECTION, &matProj ) );

    // 
    // vertex buffer
    // 
    m_vertexBuffer = NULL;
    FAIL_RET( d3ddev->CreateVertexBuffer(sizeof(m_vertices),D3DUSAGE_WRITEONLY,
         D3DFVF_CUSTOMVERTEX,D3DPOOL_MANAGED,& m_vertexBuffer, NULL ) );

    if( m_createTexture ) {
        FAIL_RET( CreateTexture( d3ddev, backBufferDesc.Width, backBufferDesc.Height ) );
    }

	m_zSurface = NULL;
	FAIL_RET(  d3ddev->CreateDepthStencilSurface( backBufferDesc.Width, backBufferDesc.Height,
        D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_zSurface, NULL )  ); 

    return S_OK;
}

HRESULT CMyCompositor9::CreateTexture( IDirect3DDevice9* d3ddev, DWORD x, DWORD y )
{
    ASSERT( d3ddev );
    HRESULT hr;
    D3DDISPLAYMODE dm; 
    FAIL_RET( d3ddev->GetDisplayMode(NULL,  & dm ) );

    m_texture = NULL;
    // create the private texture
    FAIL_RET( d3ddev->CreateTexture(x, y,
        1, 
        D3DUSAGE_RENDERTARGET, 
        dm.Format, 
        D3DPOOL_DEFAULT /* default pool - usually video memory */, 
        & m_texture, NULL ) );
    return hr;
}


HRESULT CMyCompositor9::SetUpFog( IDirect3DDevice9* d3ddev )
{
    HRESULT hr;
    float fogStart = 2.0;
    float fogEnd = 6.0;

    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGENABLE, TRUE ));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGCOLOR, 0x00001020));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGSTART, DWORD( fogStart ) ));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGEND, DWORD( fogEnd ) ));
    return hr;
}


HRESULT STDMETHODCALLTYPE CMyCompositor9::TermCompositionDevice( 
    /* [in] */ IUnknown *pD3DDevice)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMyCompositor9::SetStreamMediaType( 
    /* [in] */ DWORD dwStrmID,
    /* [in] */ AM_MEDIA_TYPE *pmt,
    /* [in] */ BOOL fTexture)
{
    m_createTexture = ! fTexture; 
    // ok it's not a texture.
    // we need to create one
    return S_OK;
}

SmartPtr<IDirect3DTexture9> CMyCompositor9::GetTexture( 
							IDirect3DDevice9* d3ddev,
							VMR9VideoStreamInfo *pVideoStreamInfo
							)
{

	SmartPtr<IDirect3DTexture9> texture;
	HRESULT hr = pVideoStreamInfo->pddsVideoSurface->GetContainer( __uuidof(IDirect3DTexture9), (LPVOID*) & texture );
    if( FAILED ( hr ) ) {
        SmartPtr<IDirect3DSurface9> surface;
		if( FAILED( m_texture->GetSurfaceLevel( 0 , & surface ) ) ) {
			return NULL;
		}

        // copy the full surface onto the texture's surface
        if( FAILED( d3ddev->StretchRect( pVideoStreamInfo->pddsVideoSurface, NULL,
                             surface, NULL,
							 D3DTEXF_NONE ) )) {
			return NULL;					 
		}

        texture = m_texture;
    }

	return texture;
}


HRESULT STDMETHODCALLTYPE CMyCompositor9::CompositeImage( 
    /* [in] */ IUnknown *pD3DDevice,
    /* [in] */ IDirect3DSurface9 *pddsRenderTarget,
    /* [in] */ AM_MEDIA_TYPE *pmtRenderTarget,
    /* [in] */ REFERENCE_TIME rtStart,
    /* [in] */ REFERENCE_TIME rtEnd,
    /* [in] */ D3DCOLOR dwClrBkGnd,
    /* [in] */ VMR9VideoStreamInfo *pVideoStreamInfo,
    /* [in] */ UINT cStreams)
{
    ASSERT( pD3DDevice );
    if( pD3DDevice == NULL ) {
        return E_POINTER;
    }

    HRESULT hr;
    SmartPtr<IDirect3DDevice9> d3ddev;
    
    pD3DDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void**)&d3ddev);
    ASSERT( d3ddev );

    FAIL_RET( d3ddev->SetRenderState( D3DRS_AMBIENT, 0x00202020 ) );
    FAIL_RET( d3ddev->SetRenderState( D3DRS_LIGHTING, FALSE ) );
    FAIL_RET( d3ddev->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE)); 
    FAIL_RET( d3ddev->SetRenderState( D3DRS_ZENABLE,TRUE)); 

    FAIL_RET( AdjustViewMatrix( d3ddev ) );

    // write the new vertex information into the buffer
    void* pData;
    FAIL_RET( m_vertexBuffer->Lock(0, sizeof m_vertices, &pData,0) );
    memcpy(pData,m_vertices,sizeof(m_vertices));                            
    FAIL_RET( m_vertexBuffer->Unlock() );  

	FAIL_RET( d3ddev->SetDepthStencilSurface( m_zSurface) );

    // clear the scene so we don't have any articats left
    d3ddev->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                   D3DCOLOR_XRGB(128,128,255), 1.0f, 0L );

    FAIL_RET( d3ddev->BeginScene() );
    FAIL_RET( d3ddev->SetStreamSource(0, m_vertexBuffer, 0, sizeof( CUSTOMVERTEX)  ) );
    FAIL_RET( d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX ) );
    FAIL_RET( d3ddev->SetTexture( 0, GetTexture( d3ddev, pVideoStreamInfo + 0 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2) );  
    FAIL_RET( d3ddev->SetTexture( 0, GetTexture( d3ddev, pVideoStreamInfo + 1 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,4,2) );  
    FAIL_RET( d3ddev->SetTexture( 0, GetTexture( d3ddev, pVideoStreamInfo + 2 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,8,2) );  
    FAIL_RET( d3ddev->EndScene());
    return S_OK;
}

// IUnknown
HRESULT STDMETHODCALLTYPE CMyCompositor9::QueryInterface( 
    REFIID riid,
    void** ppvObject)
{
    HRESULT hr = E_NOINTERFACE;

    if( ppvObject == NULL ) {
        hr = E_POINTER;
    } 
    else if( riid == IID_IVMRImageCompositor9 ) {
        *ppvObject = static_cast<IVMRImageCompositor9*>( this );
        AddRef();
        hr = S_OK;
    } 
    else if( riid == IID_IUnknown ) {
        *ppvObject = 
            static_cast<IUnknown*>( this );
        AddRef();
        hr = S_OK;    
    }

    return hr;
}

ULONG STDMETHODCALLTYPE CMyCompositor9::AddRef()
{
    return InterlockedIncrement(& m_refCount);
}

ULONG STDMETHODCALLTYPE CMyCompositor9::Release()
{
    ULONG ret = InterlockedDecrement(& m_refCount);
    if( ret == 0 )
    {
        delete this;
    }

    return ret;
}

void CMyCompositor9::ChangeX( int X )
{
    m_needTurn = true;
    m_x = X;
}

void CMyCompositor9::ChangeY( int Y )
{
    m_needTurn = true;
    m_y = Y;
}

HRESULT CMyCompositor9::AdjustViewMatrix( IDirect3DDevice9* d3ddev )
{
    ASSERT( d3ddev );
    HRESULT hr;

    if( m_needTurn == false ) {
        return S_FALSE;
    }

    //
    // view matrix
    //
    float x, y;
    x = float( m_x ) / 100 ;
    y = float( m_y ) / 100 ;
    D3DVECTOR from = { x, y, -4.0f };
    D3DVECTOR at = { 0.0f, 1.0f, 0.0f };
    D3DVECTOR up = { 0.0f, 1.0f, 0.0f };

    D3DMATRIX matView;
    MatrixLookAtLH(&matView, &from, &at, &up);
    FAIL_RET( d3ddev->SetTransform( D3DTS_VIEW, &matView ) );

    m_needTurn = false;
    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// Matrix functions
//
// The purpose of these functions is to remove any dependencies on
// the D3DX utility library from this sample. The functions are
// modeled after the equivalent D3DX functions. In a real application,
// you should use the D3DX library instead.
//
//////////////////////////////////////////////////////////////////////

template <class T>
inline T SQUARED(T x)
{
    return x * x;
}

D3DVECTOR* VecSubtract(D3DVECTOR *pOut, const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
}

D3DVECTOR* VecNormalize(D3DVECTOR *pOut, const D3DVECTOR *pV1)
{
    FLOAT norm_sq = SQUARED(pV1->x) + SQUARED(pV1->y) + SQUARED(pV1->z);

    if (norm_sq > FLT_MIN)
    {
        FLOAT f = sqrtf(norm_sq);
        pOut->x = pV1->x / f;
        pOut->y = pV1->y / f;
        pOut->z = pV1->z / f;
    }
    else
    {
        pOut->x = 0.0f;
        pOut->y = 0.0f;
        pOut->z = 0.0f;
    }
    return pOut;
}

D3DVECTOR* VecCross(D3DVECTOR *pOut, const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    pOut->x = pV1->y * pV2->z - pV1->z * pV2->y;
    pOut->y = pV1->z * pV2->x - pV1->x * pV2->z;
    pOut->z = pV1->x * pV2->y - pV1->y * pV2->x;

    return pOut;
}



FLOAT VecDot(const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

// MatrixLookAtLH: Approximately equivalent to D3DXMatrixLookAtLH.

D3DMATRIX* MatrixLookAtLH( 
    D3DMATRIX *pOut, 
    const D3DVECTOR *pEye, 
    const D3DVECTOR *pAt,
    const D3DVECTOR *pUp 
    )
{

    D3DVECTOR vecX, vecY, vecZ;

    // Compute direction of gaze. (+Z)

    VecSubtract(&vecZ, pAt, pEye);
    VecNormalize(&vecZ, &vecZ);

    // Compute orthogonal axes from cross product of gaze and pUp vector.
    VecCross(&vecX, pUp, &vecZ);
    VecNormalize(&vecX, &vecX);
    VecCross(&vecY, &vecZ, &vecX);

    // Set rotation and translate by pEye
    pOut->_11 = vecX.x;
    pOut->_21 = vecX.y;
    pOut->_31 = vecX.z;
    pOut->_41 = -VecDot(&vecX, pEye);

    pOut->_12 = vecY.x;
    pOut->_22 = vecY.y;
    pOut->_32 = vecY.z;
    pOut->_42 = -VecDot(&vecY, pEye);

    pOut->_13 = vecZ.x;
    pOut->_23 = vecZ.y;
    pOut->_33 = vecZ.z;
    pOut->_43 = -VecDot(&vecZ, pEye);

    pOut->_14 = 0.0f;
    pOut->_24 = 0.0f;
    pOut->_34 = 0.0f;
    pOut->_44 = 1.0f;

    return pOut;
}


// MatrixPerspectiveFovLH: Approximately equivalent to D3DXMatrixPerspectiveFovLH.

D3DMATRIX* MatrixPerspectiveFovLH(
    D3DMATRIX * pOut,
    FLOAT fovy,
    FLOAT Aspect,
    FLOAT zn,
    FLOAT zf
    )
{   
    // yScale = cot(fovy/2)

    FLOAT yScale = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);
    FLOAT xScale = yScale / Aspect;

    ZeroMemory(pOut, sizeof(D3DMATRIX));

    pOut->_11 = xScale;

    pOut->_22 = yScale;

    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;

    pOut->_43 = -pOut->_33 * zn;

    return pOut;
}



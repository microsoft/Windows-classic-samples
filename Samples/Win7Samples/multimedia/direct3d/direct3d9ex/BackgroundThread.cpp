//--------------------------------------------------------------------------------------
// File: BackgroundThread.cpp
//
// Sample showing how to use D3D9Ex advanced features.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }

//-----------------------------------------------------------------------------
// structures
//-----------------------------------------------------------------------------
struct BACKGROUNDVERTEX
{
    FLOAT x, y, z;	  // The untransformed position for the vertex
    DWORD dwColor;	  // Color
};
#define D3DFVF_BACKGROUNDVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
bool g_bEndThread = false;
IDirect3DTexture9*	g_pRenderTarget = NULL;
IDirect3DTexture9*	g_pSharedTexture = NULL;
IDirect3DVertexBuffer9* g_pCubeVB = NULL;
IDirect3DIndexBuffer9* g_pCubeIB = NULL;
HANDLE g_SharedHandle = NULL;
UINT g_RTWidth = 1024;
UINT g_RTHeight = 1024;
int g_CubeCubes = 3;
float g_BoxRad = 30.0f;
CRITICAL_SECTION g_CSCubes;

float g_fBackLastFrameTime = 0.0f;
LARGE_INTEGER g_liBackLastTimerUpdate = {0};
LARGE_INTEGER g_liBackTimerFrequency = {0};

//--------------------------------------------------------------------------------------
// Function Prototypes and Externs
//--------------------------------------------------------------------------------------
extern HRESULT CreateD3D9VDevice( IDirect3D9Ex* pD3D, IDirect3DDevice9Ex** ppDev9Ex, D3DPRESENT_PARAMETERS* pD3DPresentParameters, HWND hWnd );
DWORD WINAPI BackgroundThreadProc( LPVOID lpParam );
HRESULT CreateCube(IDirect3DDevice9Ex* pDev);
HRESULT CreateSharedRenderTexture(IDirect3DDevice9Ex* pDev);
void RenderBackground(IDirect3DDevice9Ex* pDev);
void CleanupBackground();


//--------------------------------------------------------------------------------------
// Exported
//--------------------------------------------------------------------------------------
HANDLE CreateBackgroundThread(IDirect3D9Ex* pD3D9);
void KillBackgroundThread() { g_bEndThread = true; }
HANDLE GetSharedTextureHandle() { return g_SharedHandle; }
int IncreaseCubeCount();
int DecreaseCubeCount();
float GetFPS();


// main background thread proc
DWORD WINAPI BackgroundThreadProc( LPVOID lpParam )
{
	// Create a critsec
	InitializeCriticalSection( &g_CSCubes );

	// Create a d3d9V device
	IDirect3DDevice9Ex*		pDevBackground = NULL;
	IDirect3D9Ex* pD3D9 = (IDirect3D9Ex*)(lpParam);

    D3DPRESENT_PARAMETERS d3dpp = {0};

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	if( FAILED( CreateD3D9VDevice( pD3D9, &pDevBackground, &d3dpp, NULL ) ) )
		return 1;
	if( FAILED( CreateCube( pDevBackground ) ) )
		return 2;
	if( FAILED( CreateSharedRenderTexture( pDevBackground ) ) )
		return 3;

	// Set the GPU thread priority
	pDevBackground->SetGPUThreadPriority( -7 );
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_LOWEST );

	// Get Timer Frequency
	QueryPerformanceFrequency( &g_liBackTimerFrequency );
	QueryPerformanceCounter( &g_liBackLastTimerUpdate );

	while(!g_bEndThread)
	{
		RenderBackground( pDevBackground );
		// Uncomment next line to throttle rendering to no more then the refresh rate of the monitor.
		// pDevBackground->WaitForVBlank(0);	
	}

	// Cleanup
	DeleteCriticalSection( &g_CSCubes );
	CleanupBackground();
	SAFE_RELEASE( pDevBackground );

	return 0;
}

HANDLE CreateBackgroundThread(IDirect3D9Ex* pD3D9)
{
	// create the thread
	DWORD dwThreadID = 0;
	HANDLE hBackgroundThread = NULL;
	hBackgroundThread = CreateThread(	NULL,
										0,
										BackgroundThreadProc,
										(LPVOID)pD3D9,
										CREATE_SUSPENDED,
										&dwThreadID );

	if( hBackgroundThread )
	{
		// set the priority

		// resume the thread
		ResumeThread( hBackgroundThread );
	}

	return hBackgroundThread;
}

HRESULT CreateCube( IDirect3DDevice9Ex* pDev )
{
	// create the vb
    if( FAILED( pDev->CreateVertexBuffer( 8*sizeof(BACKGROUNDVERTEX),
													0, 
													D3DFVF_BACKGROUNDVERTEX,
													D3DPOOL_DEFAULT, 
													&g_pCubeVB, 
													NULL ) ) )
    {
        return E_FAIL;
    }

	BACKGROUNDVERTEX vertices[] =
    {
        { -1.0f,  1.0f, -1.0f, 0x000066 },
        {  1.0f,  1.0f, -1.0f, 0x006600 },
        {  1.0f,  1.0f,  1.0f, 0x006666 },
        { -1.0f,  1.0f,  1.0f, 0x660000 },
        { -1.0f, -1.0f, -1.0f, 0x660066 },
        {  1.0f, -1.0f, -1.0f, 0x666600 },
        {  1.0f, -1.0f,  1.0f, 0x666666 },
        { -1.0f, -1.0f,  1.0f, 0x000000 },
    };

	VOID* pVertices = NULL;
	if( FAILED( g_pCubeVB->Lock( 0, sizeof(vertices), &pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, vertices, sizeof(vertices) );
	g_pCubeVB->Unlock();


	if( FAILED( pDev->CreateIndexBuffer( 36*sizeof(WORD), 
										 0,
										 D3DFMT_INDEX16,
										 D3DPOOL_DEFAULT,
										 &g_pCubeIB,
										 NULL ) ) )
	{
		return E_FAIL;
	}

	WORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };

	VOID* pIndices = NULL;
	if( FAILED( g_pCubeIB->Lock( 0, sizeof(indices), &pIndices, 0 ) ) )
		return E_FAIL;
	memcpy( pIndices, indices, sizeof(indices) );
	g_pCubeIB->Unlock();

	return S_OK;
}

HRESULT CreateSharedRenderTexture(IDirect3DDevice9Ex* pDev)
{
	if( FAILED( pDev->CreateTexture( g_RTWidth,
									g_RTHeight,
									1,
									D3DUSAGE_RENDERTARGET,
									D3DFMT_X8R8G8B8,
									D3DPOOL_DEFAULT,
									&g_pSharedTexture,
									&g_SharedHandle ) ) )
	{
		return E_FAIL;
	}

	if( !g_SharedHandle )
		return E_FAIL;

	// create a render target
	if( FAILED( pDev->CreateTexture( g_RTWidth,
									g_RTHeight,
									1,
									D3DUSAGE_RENDERTARGET,
									D3DFMT_X8R8G8B8,
									D3DPOOL_DEFAULT,
									&g_pRenderTarget,
									NULL ) ) )
	{
		return E_FAIL;
	}

	IDirect3DSurface9* pRTSurf = NULL;
	if( FAILED(g_pRenderTarget->GetSurfaceLevel( 0, &pRTSurf ) ) )
		return E_FAIL;

	if( FAILED(pDev->SetRenderTarget( 0, pRTSurf )) )
		return E_FAIL;

	pRTSurf->Release();

	// viewport
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = g_RTWidth;
	vp.Height = g_RTHeight;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	pDev->SetViewport( &vp );

	return S_OK;
}

void RenderBackground( IDirect3DDevice9Ex* pDev )
{
	static float fRot = 0.0f;
	fRot += g_fBackLastFrameTime*60.0f*(D3DX_PI/180.0f);

	// setup the matrices
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMatrixRotationY( &mWorld, fRot );

	D3DXVECTOR3 eye( 0, 2.0f, g_BoxRad*3.5f );
	D3DXVECTOR3 at( 0, 0, 0 );
	D3DXVECTOR3 up( 0, 1, 0 );
	D3DXMatrixLookAtLH( &mView, &eye, &at, &up );
	D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI/4.0f, 1.0f, 0.1f, 1000.0f );

	pDev->SetTransform( D3DTS_VIEW, &mView );
	pDev->SetTransform( D3DTS_PROJECTION, &mProj );

	// clear
	HRESULT hr = S_OK;
	pDev->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

	// Begin the scene
    if( SUCCEEDED( pDev->BeginScene() ) )
    {
		// set the texture stage states
		pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );

		// disable lighting
		pDev->SetRenderState( D3DRS_LIGHTING, FALSE );

		// stream sources
        pDev->SetStreamSource( 0, g_pCubeVB, 0, sizeof(BACKGROUNDVERTEX) );
		pDev->SetIndices( g_pCubeIB );
        pDev->SetFVF( D3DFVF_BACKGROUNDVERTEX );

		// draw
		float fStep = (g_BoxRad*2) / g_CubeCubes;
		float fStart = -g_BoxRad + fStep/2.0f;
		float fStop = fStart + fStep*g_CubeCubes;

		EnterCriticalSection( &g_CSCubes );
		for( float z=fStart; z<fStop; z+=fStep )
		{
			for( float y=fStart; y<fStop; y+=fStep )
			{
				for( float x=fStart; x<fStop; x+=fStep )
				{
					D3DXMATRIX mPos;
					D3DXMatrixTranslation( &mPos, x,y,z );
					mPos = mWorld*mPos;
					pDev->SetTransform( D3DTS_WORLD, &mPos );

					hr = pDev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 );
				}
			}
		}
		LeaveCriticalSection( &g_CSCubes );

        // End the scene
        pDev->EndScene();
    }

	// Stretch rect to our shared texture
	IDirect3DSurface9* pSurfSrc;
	IDirect3DSurface9* pSurfDest;
	g_pRenderTarget->GetSurfaceLevel( 0, &pSurfSrc );
	g_pSharedTexture->GetSurfaceLevel( 0, &pSurfDest );
	hr = pDev->StretchRect( pSurfSrc, NULL, pSurfDest, NULL, D3DTEXF_POINT );
	SAFE_RELEASE( pSurfSrc );
	SAFE_RELEASE( pSurfDest );

	// Get the time
	LARGE_INTEGER liCurrentTime;
	QueryPerformanceCounter( &liCurrentTime );
	g_fBackLastFrameTime = (float)(liCurrentTime.QuadPart - g_liBackLastTimerUpdate.QuadPart) / (float)g_liBackTimerFrequency.QuadPart;
	g_liBackLastTimerUpdate.QuadPart = liCurrentTime.QuadPart;

}

void CleanupBackground()
{
	SAFE_RELEASE(g_pRenderTarget);
	SAFE_RELEASE(g_pSharedTexture);
	SAFE_RELEASE(g_pCubeVB);
	SAFE_RELEASE(g_pCubeIB);
}

int IncreaseCubeCount()
{
	EnterCriticalSection( &g_CSCubes );

	g_CubeCubes ++;
	if( g_CubeCubes > 100 )
		g_CubeCubes = 100;

	LeaveCriticalSection( &g_CSCubes );

	return g_CubeCubes;
}

int DecreaseCubeCount()
{
	EnterCriticalSection( &g_CSCubes );

	g_CubeCubes --;
	if( g_CubeCubes < 0 )
		g_CubeCubes = 0;

	LeaveCriticalSection( &g_CSCubes );

	return g_CubeCubes;
}

float GetFPS()
{
	return 1.0f / g_fBackLastFrameTime;
}
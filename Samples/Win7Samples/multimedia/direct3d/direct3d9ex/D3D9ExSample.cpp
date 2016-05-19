//--------------------------------------------------------------------------------------
// File: D3D9ExSample.cpp
//
// Sample showing how to use D3D9Ex advanced features.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define INITGUID
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "TxtHelper.h"

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }

//--------------------------------------------------------------------------------------
// structures
//--------------------------------------------------------------------------------------
struct CURSORVERTEX
{
    FLOAT x, y, z, w; // The transformed position for the vertex
    DWORD dwColor;	  // Color
};
#define D3DFVF_CURSORVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

struct SCREENVERTEX
{
    FLOAT x, y, z, w; // The transformed position for the vertex
    FLOAT u, v;	  // texture coordinate
};
#define D3DFVF_SCREENVERTEX (D3DFVF_XYZRHW|D3DFVF_TEX1)

#define CURSOR_SIZE 60.0f

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
HWND					g_hWnd = NULL;			// Window Handle
IDirect3D9Ex*			g_pD3D9 = NULL;			// Direct3D9Ex object
IDirect3DDevice9Ex*		g_pDevRealTime = NULL;	// Device for rendering the real-time
                                                // objects like cursors

D3DPRESENT_PARAMETERS	g_D3DPresentParameters = {0}; // Current present parameters

HANDLE					g_hBackgroundThread = NULL; // Background thread

IDirect3DVertexBuffer9* g_pCursorVB = NULL;		// cursor vertex buffer
IDirect3DVertexBuffer9* g_pScreenVB = NULL;		// screen vertex buffer

IDirect3DTexture9*      g_pSharedBackgroundTexture = NULL;// Our shared texture

ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pSprite = NULL;       // Sprite for batching draw text calls

extern UINT g_RTWidth;
extern UINT g_RTHeight;

bool g_bSkipRendering = false;
int g_cubeCount = 3;
D3DPRESENTSTATS g_PresentStats;
float g_fLastFrameTime = 0.0f;
LARGE_INTEGER g_liLastTimerUpdate = {0};
LARGE_INTEGER g_liTimerFrequency = {0};

//--------------------------------------------------------------------------------------
// Function Prototypes
//--------------------------------------------------------------------------------------
BOOL InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT CreateD3D9VDevice( IDirect3D9Ex* pD3D, IDirect3DDevice9Ex** ppDev9Ex, D3DPRESENT_PARAMETERS* pD3DPresentParameters, HWND hWnd );
HRESULT InitD3D();
HRESULT CreateFont();
HRESULT CreateSprite();
void Cleanup();
HRESULT CreateD3DCursor();
HRESULT CreateScreenVB( float screenX, float screenY );
HRESULT UpdateScreenVB( float screenX, float screenY );
HRESULT CreateSharedTexture();
void MoveCursor( UINT posX, UINT posY );
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Render( IDirect3DDevice9Ex* pDev );
void RenderText( IDirect3DDevice9Ex* pDev );

extern HANDLE CreateBackgroundThread(IDirect3D9Ex* pD3D9);
extern void KillBackgroundThread();
extern HANDLE GetSharedTextureHandle();
extern int IncreaseCubeCount();
extern int DecreaseCubeCount();
extern float GetFPS();


int APIENTRY wWinMain( HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPWSTR    lpCmdLine,
                       int       nCmdShow )
{
    MSG msg = {0};
    msg.wParam = -1;

    if( !InitWindow( hInstance, nCmdShow ) )
        return 0;
    if( FAILED(InitD3D()) )
        return 1;

    g_D3DPresentParameters.hDeviceWindow = g_hWnd;
    g_D3DPresentParameters.Windowed = TRUE;
    g_D3DPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_D3DPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if( FAILED(CreateD3D9VDevice( g_pD3D9, &g_pDevRealTime, &g_D3DPresentParameters, g_hWnd ) ) )
        return 2;
    if( FAILED(CreateD3DCursor()) )
        return 3;
    if( FAILED( CreateScreenVB( 800, 600 ) ) )
        return 4;
    if( FAILED( CreateFont() ) )
        return 5;
    if( FAILED( CreateSprite() ) )
        return 6;

    // set the realtime GPU thread priority
    g_pDevRealTime->SetGPUThreadPriority( 7 );
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

    // Get Timer Frequency
    QueryPerformanceFrequency( &g_liTimerFrequency );
    QueryPerformanceCounter( &g_liLastTimerUpdate );

    // Create the background thread
    g_hBackgroundThread = CreateBackgroundThread(g_pD3D9);
    if(!g_hBackgroundThread)
        goto exit;

    if( FAILED( CreateSharedTexture() ) )
        return 5;

    // Hide the cursor
    ShowCursor( false );

    // Main loop
    msg.message = WM_NULL;
    PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render(g_pDevRealTime);  // Do some rendering
        }
    }

exit:
    if(g_hBackgroundThread)
    {
        KillBackgroundThread();
        WaitForSingleObject(g_hBackgroundThread,INFINITE);
    }

    Cleanup();
    return (int) msg.wParam;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
BOOL InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    //
    // Register class
    //
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style          = CS_CLASSDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = L"d3d9vwindowclass";
    wcex.hIconSm        = 0;

    if( !RegisterClassEx(&wcex) )
        return FALSE;

    //
    // Create window
    //
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRectEx( &rc, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE );
    g_hWnd = CreateWindowEx( WS_EX_CLIENTEDGE, L"d3d9vwindowclass", L"Direct3D9Ex Sample", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL,
                             hInstance, NULL);

    if( !g_hWnd )
        return FALSE;

    ShowWindow( g_hWnd, nCmdShow );

    return TRUE;
}

HRESULT CreateD3D9VDevice( IDirect3D9Ex* pD3D, IDirect3DDevice9Ex** ppDev9Ex, D3DPRESENT_PARAMETERS* pD3DPresentParameters, HWND hWnd )
{
    HRESULT hr = S_OK;

    // Create a d3d9ex device
    DWORD dwFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_ENABLE_PRESENTSTATS;
                             
    if( !hWnd )
    {
        hWnd = g_hWnd;
        dwFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    hr = pD3D->CreateDeviceEx( D3DADAPTER_DEFAULT, 
                               D3DDEVTYPE_HAL, 
                               hWnd,
                               dwFlags,
                               pD3DPresentParameters,
                               NULL,
                               ppDev9Ex );
    
    if( SUCCEEDED(hr) )
    {
        // Make sure our formats are OK
        D3DCAPS9 Caps;
        ZeroMemory(&Caps,sizeof(D3DCAPS9));
        pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps );

        // Skip backbuffer formats that don't support alpha blending
        hr = pD3D->CheckDeviceFormat( Caps.AdapterOrdinal, Caps.DeviceType,
                        pD3DPresentParameters->BackBufferFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                        D3DRTYPE_TEXTURE, pD3DPresentParameters->BackBufferFormat );
        if(FAILED(hr))
        {
            SAFE_RELEASE( *ppDev9Ex );
            return hr;
        }
    }

    return hr;
}

HRESULT InitD3D()
{
    // Create a d3d9 device
    HRESULT hr = Direct3DCreate9Ex( D3D_SDK_VERSION, &g_pD3D9 );
    return hr;
}

HRESULT Reset(IDirect3DDevice9Ex* pDev9Ex)
{
    // reset the d3d9ex device
    HRESULT hr = pDev9Ex->Reset( &g_D3DPresentParameters);
    if(FAILED(hr))
        return hr;

    return CreateSprite();
}

HRESULT CreateFont()
{
    if ( FAILED( D3DXCreateFont( g_pDevRealTime, 16, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                              L"Arial", &g_pFont ) ) )
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CreateSprite()
{
    // Create a sprite to help batch calls when drawing many lines of text
    return D3DXCreateSprite( g_pDevRealTime, &g_pSprite );
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void Cleanup()
{
    SAFE_RELEASE(g_pD3D9);
    SAFE_RELEASE(g_pDevRealTime);
    SAFE_RELEASE(g_pCursorVB);
    SAFE_RELEASE(g_pScreenVB);	
    SAFE_RELEASE(g_pSharedBackgroundTexture);
    SAFE_RELEASE(g_pFont);
    SAFE_RELEASE(g_pSprite);
}


HRESULT CreateD3DCursor()
{
    // create the vb
    if( FAILED( g_pDevRealTime->CreateVertexBuffer( 4*sizeof(CURSORVERTEX),
                                                    D3DUSAGE_DYNAMIC, 
                                                    D3DFVF_CURSORVERTEX,
                                                    D3DPOOL_DEFAULT, 
                                                    &g_pCursorVB, 
                                                    NULL ) ) )
    {
        return E_FAIL;
    }

    MoveCursor( 400, 300 );

    return S_OK;
}

HRESULT CreateScreenVB( float screenX, float screenY )
{
    // create the vb
    if( FAILED( g_pDevRealTime->CreateVertexBuffer( 4*sizeof(SCREENVERTEX),
                                                    D3DUSAGE_DYNAMIC, 
                                                    D3DFVF_SCREENVERTEX,
                                                    D3DPOOL_DEFAULT, 
                                                    &g_pScreenVB, 
                                                    NULL ) ) )
    {
        return E_FAIL;
    }

    UpdateScreenVB( screenX, screenY);

    return S_OK;
}

HRESULT UpdateScreenVB( float screenX, float screenY )
{
    SCREENVERTEX* vertices = NULL;
    if( FAILED( g_pScreenVB->Lock( 0, 4*sizeof(SCREENVERTEX), (void**)&vertices, 0 ) ) )
        return E_FAIL;

    vertices[0].x = 0;
    vertices[0].y = screenY;
    vertices[0].z = 0.5f;
    vertices[0].w = 1.0f;
    vertices[0].u = 0.0f;
    vertices[0].v = 1.0f;
    
    vertices[1].x = 0;
    vertices[1].y = 0;
    vertices[1].z = 0.5f;
    vertices[1].w = 1.0f;
    vertices[1].u = 0.0f;
    vertices[1].v = 0.0f;

    vertices[2].x = screenX;
    vertices[2].y = screenY;
    vertices[2].z = 0.5f;
    vertices[2].w = 1.0f;
    vertices[2].u = 1.0f;
    vertices[2].v = 1.0f;

    vertices[3].x = screenX;
    vertices[3].y = 0;
    vertices[3].z = 0.5f;
    vertices[3].w = 1.0f;
    vertices[3].u = 1.0f;
    vertices[3].v = 0.0f;

    g_pScreenVB->Unlock();

    return S_OK;
}

HRESULT CreateSharedTexture()
{
    // wait for a handle from the other thread
    HANDLE hHandle = NULL;
    while( !hHandle )
        hHandle = GetSharedTextureHandle();

    HRESULT hr = g_pDevRealTime->CreateTexture( g_RTWidth,
                                    g_RTHeight,
                                    1,
                                    D3DUSAGE_RENDERTARGET,
                                    D3DFMT_X8R8G8B8,
                                    D3DPOOL_DEFAULT,
                                    &g_pSharedBackgroundTexture,
                                    &hHandle );
    if( FAILED( hr ) )
        return E_FAIL;

    return hr;
}

void MoveCursor( UINT posX, UINT posY )
{
    float x = static_cast<float>(posX);
    float y = static_cast<float>(posY);

    // Initialize vertices for rendering a quad
    CURSORVERTEX vertices[4];

    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = 0.5f;
    vertices[0].w = 1.0f;

    vertices[1].x = x + CURSOR_SIZE;
    vertices[1].y = y + CURSOR_SIZE*0.6f;
    vertices[1].z = 0.5f;
    vertices[1].w = 1.0f;

    vertices[2].x = x + CURSOR_SIZE*0.6f;
    vertices[2].y = y + CURSOR_SIZE;
    vertices[2].z = 0.5f;
    vertices[2].w = 1.0f;

    vertices[3].x = x + CURSOR_SIZE;
    vertices[3].y = y + CURSOR_SIZE;
    vertices[3].z = 0.5f;
    vertices[3].w = 1.0f;

    for(int i=0; i<4; i++)
        vertices[i].dwColor = 0xff0000;

    VOID* pVertices;
    if( FAILED( g_pCursorVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
        return;
    memcpy( pVertices, vertices, sizeof(vertices) );
    g_pCursorVB->Unlock();
}


LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) 
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_MOUSEMOVE:
            {
                UINT x = LOWORD( lParam );
                UINT y = HIWORD( lParam );
                MoveCursor( x,y );
            }

            break;

        case WM_KEYUP:
            switch( wParam )
            {
            case VK_UP:
                g_cubeCount = IncreaseCubeCount();
                break;
            case VK_DOWN:
                g_cubeCount = DecreaseCubeCount();
                break;
            case VK_ESCAPE:
                PostQuitMessage(0);
                break;
            };
            break;

        case WM_SYSKEYUP:
            if( (wParam == VK_RETURN) && (g_pD3D9 != NULL) && (g_pDevRealTime != NULL) )
            {
                g_D3DPresentParameters.Windowed = !g_D3DPresentParameters.Windowed;
                
                DWORD dwStyle;
                DWORD dwExStyle;

                D3DDISPLAYMODE Mode;

                g_pD3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode);

                if (g_D3DPresentParameters.Windowed)
                {
                    //
                    // Default to using the window client area size
                    //
                    g_D3DPresentParameters.BackBufferWidth = 800;
                    g_D3DPresentParameters.BackBufferHeight = 600;

                    //
                    // Window styles for D3D windowed mode
                    //
                    dwStyle = WS_OVERLAPPEDWINDOW;
                    dwExStyle = WS_EX_CLIENTEDGE;
                }
                else
                {
                    g_D3DPresentParameters.BackBufferWidth=Mode.Width;
                    g_D3DPresentParameters.BackBufferHeight=Mode.Height;

                    dwStyle = WS_POPUP;
                    dwExStyle = WS_EX_TOPMOST;
                }

                SetWindowLong( g_hWnd, GWL_STYLE, dwStyle );
                SetWindowLong( g_hWnd, GWL_EXSTYLE, dwExStyle );

                if (g_D3DPresentParameters.Windowed)
                {

                    RECT rc = { 0, 0, g_D3DPresentParameters.BackBufferWidth, g_D3DPresentParameters.BackBufferHeight };
                    AdjustWindowRectEx( &rc, dwStyle, FALSE, dwExStyle);

                    SetWindowPos( g_hWnd, HWND_NOTOPMOST,
                                  0, 0,
                                  rc.right - rc.left,
                                  rc.bottom - rc.top,
                                  SWP_SHOWWINDOW | SWP_NOMOVE );
                }

                HRESULT hr;

                hr = g_pDevRealTime->Reset(&g_D3DPresentParameters);

                if (SUCCEEDED( hr ))
                {
                    hr = UpdateScreenVB(static_cast<float>(g_D3DPresentParameters.BackBufferWidth),
                                        static_cast<float>(g_D3DPresentParameters.BackBufferHeight));
                }

                if (FAILED( hr ))
                {
                    return 0;
                }
            }
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void Render( IDirect3DDevice9Ex* pDev )
{
    // Begin the scene
    if( !g_bSkipRendering && SUCCEEDED( pDev->BeginScene() ) )
    {
        // disable lighting
        pDev->SetRenderState( D3DRS_LIGHTING, FALSE );


        //
        // Render the background
        //

        // set the texture
        pDev->SetTexture( 0, g_pSharedBackgroundTexture );

        pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

        // stream sources
        pDev->SetStreamSource( 0, g_pScreenVB, 0, sizeof(SCREENVERTEX) );
        pDev->SetFVF( D3DFVF_SCREENVERTEX );

        // draw
        pDev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );


        //
        // Render the cursor
        //

        pDev->SetTexture( 0, NULL );

        // set the texture stage states
        pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );

        // stream sources
        pDev->SetStreamSource( 0, g_pCursorVB, 0, sizeof(CURSORVERTEX) );
        pDev->SetFVF( D3DFVF_CURSORVERTEX );

        // draw
        pDev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

        // render text
        RenderText( pDev );

        // End the scene
        pDev->EndScene();
    }
    else if( g_bSkipRendering )
        Sleep(20);	//don't hog the entire CPU

    // Present the backbuffer contents to the display
    HRESULT hr = pDev->Present( NULL, NULL, NULL, NULL );

    // Handle Occluded, DeviceReset, or Mode Changes
    if( S_PRESENT_OCCLUDED == hr )
    {
        g_bSkipRendering = true;
    }
    else if( D3DERR_DEVICELOST == hr )
    {
        if(FAILED( Reset( pDev ) ) )
            g_bSkipRendering = true;	
    }
    else if( S_PRESENT_MODE_CHANGED == hr )
    {
        //
        // Reenumerate modes by calling IDirect3D9::GetAdapterModeCountEx
        //
        D3DDISPLAYMODEFILTER DisplayModeFilter;

        ZeroMemory(&DisplayModeFilter, sizeof(DisplayModeFilter));

        DisplayModeFilter.Size			   = sizeof(DisplayModeFilter);
        DisplayModeFilter.Format		   = D3DFMT_UNKNOWN;
        DisplayModeFilter.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;

        UINT ModeCount = g_pD3D9->GetAdapterModeCountEx(D3DADAPTER_DEFAULT, &DisplayModeFilter);

        if(FAILED( Reset( pDev ) ) )
            g_bSkipRendering = true;	
    }
    else if( D3DERR_DEVICEHUNG == hr )
    {
        MessageBox( g_hWnd,
                    L"This application has caused the graphics adapter to hang, check with the hardware vendor for a new driver.",
                    L"Graphics Adapter Hang",
                    MB_OK );

        PostQuitMessage(0);
        g_bSkipRendering = true;
    }
    else
    {
        g_bSkipRendering = false;
    }

    // Get some presents stats
    IDirect3DSwapChain9* pSwapChain;
    if( SUCCEEDED( pDev->GetSwapChain( 0, &pSwapChain ) ) )
    {
        IDirect3DSwapChain9Ex* pSwapChainEx;
        if( SUCCEEDED( pSwapChain->QueryInterface( IID_IDirect3DSwapChain9Ex, (void**)&pSwapChainEx ) ) )
        {
            hr = pSwapChainEx->GetLastPresentCount( &g_PresentStats.PresentCount );
            if( SUCCEEDED( hr ) )
                hr = pSwapChainEx->GetPresentStats( &g_PresentStats );
            pSwapChainEx->Release();
        }
        pSwapChain->Release();
    }

    // Get the time
    LARGE_INTEGER liCurrentTime;
    QueryPerformanceCounter( &liCurrentTime );
    g_fLastFrameTime = (float)(liCurrentTime.QuadPart - g_liLastTimerUpdate.QuadPart) / (float)g_liTimerFrequency.QuadPart;
    g_liLastTimerUpdate.QuadPart = liCurrentTime.QuadPart;
}

void RenderText( IDirect3DDevice9Ex* pDev )
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    CTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"This sample demonstrates rendering a cursor indepedently of geometry." );
    txtHelper.DrawTextLine( L"The scene (cubes in this case) is drawn by a D3D9Ex device that runs" );
    txtHelper.DrawTextLine( L"in a lower priority background thread.  The image is copied to a shared" );
    txtHelper.DrawTextLine( L"surface.  The main application thread contains a D3D9Ex device as well." );
    txtHelper.DrawTextLine( L"This thread runs at a higher priority and composites the shared image" );
    txtHelper.DrawTextLine( L"with a D3D9Ex drawn cursor and text in real time.  This allows for" );
    txtHelper.DrawTextLine( L"fluid cursor and text updates even when the scene is too complex to be" );
    txtHelper.DrawTextLine( L"handled in real-time." );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"" );
    txtHelper.DrawTextLine( L"Press the UP arrow key to increase the scene complexity." );
    txtHelper.DrawTextLine( L"Press the DOWN arrow key to decrease the scene complexity." );
    txtHelper.DrawTextLine( L"" );
    txtHelper.DrawTextLine( L"Try increasing the Number of Cubes to a very high number.  The cubes will" );
    txtHelper.DrawTextLine( L"update slowly, but the cursor will still be responsive." );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"" );
    txtHelper.DrawFormattedTextLine( L"Number of Cubes: %d", g_cubeCount*g_cubeCount );

    // get stats from the background thread
    float FPS = GetFPS();
    txtHelper.SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"" );
    txtHelper.DrawTextLine( L"Background Thread:" );
    txtHelper.DrawFormattedTextLine( L"FPS: %0.2f", FPS );
    txtHelper.DrawTextLine( L"" );
    txtHelper.DrawTextLine( L"Foreground Thread:" );
    txtHelper.DrawFormattedTextLine( L"FPS: %0.2f", 1.0f / g_fLastFrameTime );
    txtHelper.DrawFormattedTextLine( L"Present Count: %d", g_PresentStats.PresentCount );
    txtHelper.DrawFormattedTextLine( L"Present Refresh Count: %d", g_PresentStats.PresentRefreshCount );
    txtHelper.DrawFormattedTextLine( L"Sync Refresh Count: %d", g_PresentStats.SyncRefreshCount );

    txtHelper.End();
}



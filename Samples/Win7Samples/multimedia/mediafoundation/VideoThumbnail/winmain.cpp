//////////////////////////////////////////////////////////////////////////
//
// Application entry-point
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "videothumbnail.h"
#include "clock.h"
#include "Thumbnail.h"

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

BOOL    InitializeApp();
BOOL    InitializeWindow(HWND *pHwnd);
HRESULT CreateDrawingResources(HWND hwnd);
void    CleanUp();
INT     MessageLoop(HWND hwnd);
void    ShowErrorMessage(PCWSTR format, HRESULT hr);

LRESULT CALLBACK    WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Window message handlers
BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void    OnClose(HWND hwnd);
void    OnPaint(HWND hwnd);
void    OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void    OnSize(HWND hwnd, UINT state, int cx, int cy);
void    OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);

void    OnFileOpen(HWND hwnd);
void    OnSaveBitmap(HWND hwnd);

HRESULT RenderFrame(HWND hwnd);
HRESULT OpenVideoFile(HWND hwnd, const WCHAR *sURL);
void    SelectSprite(int iSelection);
void    UnselectAllSprites();

// Constants 

const WCHAR CLASS_NAME[]  = L"Video Thumbnail Sample - Window Class";
const WCHAR WINDOW_NAME[] = L"Video Thumbnail Sample";

const D2D1_COLOR_F      BACKGROUND_COLOR = D2D1::ColorF(D2D1::ColorF::DarkSlateGray); 
const DWORD             MAX_SPRITES = 4;    
const float             ANIMATION_DURATION = 0.4f;


// Global variables

ThumbnailGenerator      g_ThumbnailGen;     
Timer                   g_Timer;

Sprite                  g_pSprites[ MAX_SPRITES ];
int                     g_Selection = -1;   // Which sprite is selected (-1 = no selection)

ID2D1HwndRenderTarget   *g_pRT = NULL;      // Render target for D2D animation

//
//  DPI Scaling logic - helper functions to allow us to respect DPI settings.
//
float g_fDPIScaleX = 1.0f;
float g_fDPIScaleY = 1.0f;



// The following rectangles define the small (unselected) and 
// big (selected) locations for the sprites.

const D2D1_RECT_F g_rcSmall[] = { 

    //    left   top    right  bottom
    D2D1::RectF(0.05f, 0.0f,  0.2f,  0.25f), 
    D2D1::RectF(0.05f, 0.25f, 0.2f,  0.50f), 
    D2D1::RectF(0.05f, 0.50f, 0.2f,  0.75f), 
    D2D1::RectF(0.05f, 0.75f, 0.2f,  1.00f) 
};

const D2D1_RECT_F g_rcBig = D2D1::RectF(0.25f, 0.05f, 0.95f, 0.95f);



void InitializeDPIScale(HWND hWnd)
{
    //
    //  Create the font to be used in the OSD, making sure that
    //  the font size is scaled system DPI setting.
    //
    HDC hdc = GetDC(hWnd);
    g_fDPIScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    g_fDPIScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    ReleaseDC(hWnd, hdc);
}

int DPIScaleX(int iValue)
{
    return static_cast<int>(iValue / g_fDPIScaleX);
}
int DPIScaleY(int iValue)
{
    return static_cast<int>(iValue / g_fDPIScaleY);
}


/////////////////////////////////////////////////////////////////////

INT WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,INT)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HWND hwnd = 0;

    if (InitializeApp() && InitializeWindow(&hwnd))
    {
        MessageLoop(hwnd);
    }

    CleanUp();

    return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,      OnCreate);
        HANDLE_MSG(hwnd, WM_CLOSE,       OnClose);
        HANDLE_MSG(hwnd, WM_PAINT,       OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND,     OnCommand);
        HANDLE_MSG(hwnd, WM_SIZE,        OnSize);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);

    case WM_ERASEBKGND:
        return 1;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//-------------------------------------------------------------------
// InitializeApp: Initializes the application.
//-------------------------------------------------------------------

BOOL InitializeApp()
{
    HRESULT hr = S_OK;

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        // Initialize Media Foundation.
        hr = MFStartup(MF_VERSION);
    }

    if (SUCCEEDED(hr))
    {
        // Start the clock
        g_Timer.InitializeTimer(30);
    }

    return (SUCCEEDED(hr));
}


//-------------------------------------------------------------------
// Releases resources
//-------------------------------------------------------------------

void CleanUp()
{
    for (DWORD i = 0; i < MAX_SPRITES; i++)
    {
        g_pSprites[i].Clear();
    }

    SafeRelease(&g_pRT);

    MFShutdown();
    CoUninitialize();
}


//-------------------------------------------------------------------
// InitializeWindow: Initializes the application window.
//-------------------------------------------------------------------

BOOL InitializeWindow(HWND *pHwnd)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
        );

    if (!hwnd)
    {
        return FALSE;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    *pHwnd = hwnd;

    return TRUE;
}


//-------------------------------------------------------------------
// Message loop for the application window.
//-------------------------------------------------------------------

INT MessageLoop(HWND hwnd)
{
    MSG msg = {0};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;

        }

        // Wait until the timer expires or any message is posted.
        if (WAIT_OBJECT_0 == MsgWaitForMultipleObjects(
                1,
                &g_Timer.Handle(),
                FALSE,
                INFINITE,
                QS_ALLINPUT
                ))
        {
            RenderFrame(hwnd);  
        }
    }

    DestroyWindow(hwnd);

    return INT(msg.wParam);
}



//-------------------------------------------------------------------
// WM_CREATE handler.
//-------------------------------------------------------------------

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
    // Initialize the DPI scalar.
    InitializeDPIScale(hwnd);


    LPWSTR lpCmdLineW = GetCommandLine();

    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(lpCmdLineW, &argc);

    if (argv == NULL)
    {
        return FALSE;
    }

    if (argc == 2)
    {
        OpenVideoFile(hwnd, argv[1]);
    }

    LocalFree(argv);

    return TRUE;
}

//-------------------------------------------------------------------
// WM_CLOSE handler.
//-------------------------------------------------------------------

void OnClose(HWND /*hwnd*/)
{
    PostQuitMessage(0);
}


//-------------------------------------------------------------------
// WM_PAINT handler.
//-------------------------------------------------------------------

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = 0;

    hdc = BeginPaint(hwnd, &ps);

    RenderFrame(hwnd);

    EndPaint(hwnd, &ps);
}


//-------------------------------------------------------------------
// WM_SIZE handler.
//-------------------------------------------------------------------

void OnSize(HWND hwnd, UINT /*state*/, int cx, int cy)
{
    if (g_pRT)
    {
        g_pRT->Resize( D2D1::SizeU( cx, cy ) );

        InvalidateRect(hwnd, NULL, FALSE);
    }
}



//-------------------------------------------------------------------
// WM_COMMAND handler.
//-------------------------------------------------------------------

void OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
    switch (id)
    {
    case ID_FILE_OPENFILE:
        OnFileOpen(hwnd);
        break;
    }
}

//-------------------------------------------------------------------
// WM_LBUTTONDOWN handler.
//-------------------------------------------------------------------

void OnLButtonDown(HWND /*hwnd*/, BOOL /*fDoubleClick*/, int x, int y, UINT /*keyFlags*/)
{
    // Scale for DPI
    x = static_cast<int>(x / g_fDPIScaleX);
    y = static_cast<int>(y / g_fDPIScaleY);

    for (DWORD i = 0; i < MAX_SPRITES; i++)
    {
        // Hit-test each sprite.

        Sprite *pSprite = &g_pSprites[i];
        if (pSprite && pSprite->HitTest(x, y))
        {
            SelectSprite(i);
            break;
        }
    }
}



//-------------------------------------------------------------------
// OnFileOpen: "File Open" menu handler.
//-------------------------------------------------------------------

void OnFileOpen(HWND hwnd)
{    
    HRESULT hr = S_OK;

    IFileDialog *pDialog = NULL;
    IShellItem *pItem = NULL;

    LPWSTR pwszFilePath = NULL;

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(
        __uuidof(FileOpenDialog), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&pDialog)
        );

    if (FAILED(hr)) { goto done; }

    hr = pDialog->SetTitle(L"Select a File to Play");
    if (FAILED(hr)) { goto done; }

    // Show the file-open dialog.
    hr = pDialog->Show(hwnd);

    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        // User cancelled.
        hr = S_OK;
        goto done;
    }

    if (FAILED(hr)) { goto done; }

    hr = pDialog->GetResult(&pItem);

    if (FAILED(hr)) { goto done; }

    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwszFilePath);

    if (FAILED(hr)) { goto done; }

    // Open the file and create bitmaps.
    hr = OpenVideoFile(hwnd, pwszFilePath);

done:
    if (FAILED(hr))
    {
        ShowErrorMessage(L"Cannot open file.", hr);
    }

    CoTaskMemFree(pwszFilePath);
    SafeRelease(&pItem);
    SafeRelease(&pDialog);
}


//-------------------------------------------------------------------
// OpenVideoFile: Opens a new video file and creates thumbnails.
//-------------------------------------------------------------------

HRESULT OpenVideoFile(HWND hwnd, const WCHAR *sURL)
{
    HRESULT hr = S_OK;

    hr = g_ThumbnailGen.OpenFile(sURL);

    if (FAILED(hr)) 
    { 
        return hr; 
    }

    // Clear all the sprites.
    for (DWORD i = 0; i < MAX_SPRITES; i++)
    {
        g_pSprites[i].Clear();
    }

    if (g_pRT == NULL)
    {
        // Create the Direct2D resources.
        hr = CreateDrawingResources(hwnd);
    }

    // Generate new sprites.
    if (SUCCEEDED(hr))
    {
        assert(g_pRT != NULL);

        hr = g_ThumbnailGen.CreateBitmaps(g_pRT, MAX_SPRITES, g_pSprites);
    }

    if (SUCCEEDED(hr))
    {
        UnselectAllSprites();

        // Select the first sprite.
        SelectSprite(0);
    }

    if (SUCCEEDED(hr))
    {
        hr = RenderFrame(hwnd);
    }

    return hr;
}



//-------------------------------------------------------------------
// RenderFrame: Draw all the sprites.
//-------------------------------------------------------------------

HRESULT RenderFrame(HWND hwnd)
{
    HRESULT hr = S_OK;

    if (g_pRT && g_pRT->CheckWindowState() == D2D1_WINDOW_STATE_OCCLUDED)
    {
        return S_OK; // The render target is occluded.
    }

    if (!g_pRT)
    {
        // Create the Direct2D resources.
        hr = CreateDrawingResources(hwnd);
    }

    if (SUCCEEDED(hr))
    {

        float fTime = float(g_Timer.GetFrameNumber()) / 30;

        g_pRT->BeginDraw();

        g_pRT->Clear(BACKGROUND_COLOR);

        // Update and draw each sprite.
        for (DWORD i = 0; i < MAX_SPRITES; i++)
        {
            Sprite *pSprite = &g_pSprites[i];
            if (pSprite)
            {
                pSprite->Update(g_pRT, fTime);

                pSprite->Draw(g_pRT);
            }
        }

        // Reset the transform to the identiy matrix.
        g_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        hr = g_pRT->EndDraw();
    }

    return hr;
}

//-------------------------------------------------------------------
// SelectSprite: Selects a sprite.
//-------------------------------------------------------------------

void SelectSprite(int iSelection)
{
    if (iSelection > MAX_SPRITES)
    {
        assert(FALSE);
        return;
    }

    float time = float(g_Timer.GetFrameNumber()) / 30;

    // Apply animation to the sprite.

    // NOTE: If the sprite is already selected, this call is still used
    // to apply "wobble" to the sprite.

    g_pSprites[iSelection].AnimateBoundingBox(g_rcBig, time, ANIMATION_DURATION ); 

    // Unselect the current selection.
    if ((g_Selection != -1) && (g_Selection != iSelection))
    {
        g_pSprites[ g_Selection ].AnimateBoundingBox(g_rcSmall[ g_Selection ], time, ANIMATION_DURATION );
    }

    g_Selection = iSelection;
}

//-------------------------------------------------------------------
// UnselectAllSprites: Unselects all sprites.
//-------------------------------------------------------------------

void UnselectAllSprites()
{
    for (DWORD i = 0; i < MAX_SPRITES; i++)
    {
        g_pSprites[i].AnimateBoundingBox( g_rcSmall[i], 0, 0 );
    }

    g_Selection = -1;
}


//-------------------------------------------------------------------
// CreateDrawingResources: Creates Direct2D resources.
//-------------------------------------------------------------------

HRESULT CreateDrawingResources(HWND hwnd)
{
    HRESULT hr = S_OK;
    RECT rcClient = { 0 };

    ID2D1Factory *pFactory = NULL;
    ID2D1HwndRenderTarget *pRenderTarget = NULL;

    GetClientRect(hwnd, &rcClient);

    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pFactory
        );


    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd,
                D2D1::SizeU(rcClient.right, rcClient.bottom)
                ),
            &pRenderTarget
            );
    }

    if (SUCCEEDED(hr))
    {
        g_pRT = pRenderTarget;
        g_pRT->AddRef();
    }

    SafeRelease(&pFactory);
    SafeRelease(&pRenderTarget);
    return hr;
}


void ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
    HRESULT hr = S_OK;
    WCHAR msg[MAX_PATH];

    hr = StringCbPrintf(msg, sizeof(msg), L"%s (hr=0x%X)", format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, msg, L"Error", MB_ICONERROR);
    }
    else
    {
        DebugBreak();
    }
}

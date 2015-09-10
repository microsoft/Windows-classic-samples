// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*------------------------------------------------------------------------
Sample name:    WAMManagingDComp

Description:
This sample demonstrates how to use the curve generation feature of 
Windows Animation Manager (WAM) with DirectComposition (DComp). 
Specifically, the application shows how to:

    - create DirectComposition device and visuals
    - set DirectComposition transforms
    - create WAM storyboard, variable, and transitions
    - generate animation curves and propagate the curves to DComp
    - synchronize WAM and DirectComposition time
    - Schedule animations

Usage:
    Left mousekey press - slides the tiles forward
    Right mousekey press - slides the tiles backward

--------------------------------------------------------------------------*/

#include <dwmapi.h>
#include <math.h>
#include <wincodec.h>
#include <tchar.h>
#include <strsafe.h>

#include "Resource.h"
#include "DirectComposition_WAM.h"

CApplication *CApplication::_application = nullptr;

#define WINDOW_SIZE 500
#define TILE_SPACING 170.0f

//------------------------------------------------------
// Processing Windows Commands
//------------------------------------------------------
LRESULT CALLBACK CApplication::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (msg)
    {
        case WM_CREATE:
            _application->OnCreate();
            break;

        case WM_PAINT:
            result = _application->OnPaint(hwnd);
            break;

        case WM_LBUTTONDOWN:
            result = _application->Move(forward);
            break;

        case WM_RBUTTONDOWN:
            result = _application->Move(backward);
            break;

        case WM_LBUTTONUP:
            result = _application->Move(stopForward);
            break;

        case WM_RBUTTONUP:
            result = _application->Move(stopBackward);
            break;

        case WM_CLOSE:
            result = _application->OnClose();
            break;

        case WM_DESTROY:
            result = _application->OnDestroy();
            break;

        default:
            result = DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return result;
}

CApplication::CApplication(HINSTANCE instance) : 
    _hinstance(instance),
    _hwnd(NULL),
    _hbrush(NULL)
{
    _application = this;
}

CApplication::~CApplication()
{
    _application = nullptr;
}

int CApplication::Run()
{
    int result = 0;

    if (SUCCEEDED(BeforeEnteringMessageLoop()))
    {
        result = EnterMessageLoop();
    }
    else
    {
        MessageBoxW(NULL, L"An error occuring when running the sample", NULL, MB_OK);
    }

    AfterLeavingMessageLoop();

    return result;
}

//-------------------------------------------------------
// Creates and initializes all the objects we need for the application
//--------------------------------------------------------

HRESULT CApplication::BeforeEnteringMessageLoop()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        hr = CreateApplicationWindow();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD3D11Device();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD2D1Factory();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD2D1Device();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateWICFactory();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateAnimationManager();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateAnimationTransitionLibrary();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateAnimationVariables();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionRenderTarget();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionVisualTree();
    }

    if (SUCCEEDED(hr))
    {
        hr = AttachDCompositionVisualTreeToRenderTarget();
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->Commit();
    }

    return hr;
}
//----------------------------------------------------------
// Create an instance of WAM Manager Object which manages storyboards, 
// transitions, and variables
//----------------------------------------------------------
HRESULT CApplication::CreateAnimationManager()
{
    return ::CoCreateInstance(
        CLSID_UIAnimationManager2, 
        nullptr, 
        CLSCTX_INPROC_SERVER, 
        IID_IUIAnimationManager2, 
        reinterpret_cast<LPVOID *>(&_manager));
}

//-----------------------------------------------------------
// Creates an WAM transition library which enables us to schedule 
// transitions
//-----------------------------------------------------------
HRESULT CApplication::CreateAnimationTransitionLibrary()
{
    return ::CoCreateInstance(
        CLSID_UIAnimationTransitionLibrary2, 
        nullptr, 
        CLSCTX_INPROC_SERVER, 
        IID_IUIAnimationTransitionLibrary2, 
        reinterpret_cast<LPVOID *>(&_transitionLibrary));
}

//-----------------------------------------------------------
// Creates an WAM animation variable  which we will use to animate 
// the tiles
//-----------------------------------------------------------
HRESULT CApplication::CreateAnimationVariables()
{
    HRESULT hr = (_manager == nullptr) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _manager->CreateAnimationVariable(0.0, &_animationVariable);
    }

    return hr;
}

HRESULT CApplication::CreateD3D11Device()
{
    HRESULT hr = S_OK;

    D3D_DRIVER_TYPE driverTypes[] = 
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };

    D3D_FEATURE_LEVEL featureLevelSupported;

    for (int i = 0; i < sizeof(driverTypes) / sizeof(driverTypes[0]); ++i)
    {
        CComPtr<ID3D11Device> d3d11Device;
        CComPtr<ID3D11DeviceContext> d3d11DeviceContext;

        hr = D3D11CreateDevice(
            nullptr,
            driverTypes[i], 
            NULL, 
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,  
            NULL, 
            0, 
            D3D11_SDK_VERSION,
            &d3d11Device,
            &featureLevelSupported,
            &d3d11DeviceContext);

        if (SUCCEEDED(hr))
        {
            _d3d11Device = d3d11Device.Detach();
            _d3d11DeviceContext = d3d11DeviceContext.Detach();

            break;
        }
    }

    return hr;
}

HRESULT CApplication::CreateD2D1Factory()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2d1Factory);
}

HRESULT CApplication::CreateD2D1Device()
{
    HRESULT hr = ((_d3d11Device == nullptr) || (_d2d1Factory == nullptr)) ? E_UNEXPECTED : S_OK;

    CComPtr<IDXGIDevice> dxgiDevice;

    if (SUCCEEDED(hr))
    {
        hr = _d3d11Device->QueryInterface(&dxgiDevice);
    }

    if (SUCCEEDED(hr))
    {
        hr = _d2d1Factory->CreateDevice(dxgiDevice, &_d2d1Device);
    }

    if (SUCCEEDED(hr))
    {
        hr = _d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_d2d1DeviceContext);
    }

    return hr;
}

HRESULT CApplication::CreateWICFactory()
{
    return CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&_wicFactory));
}

int CApplication::EnterMessageLoop()
{
    int result = 0;

    if (ShowApplicationWindow())
    {
        MSG msg = { 0 };

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        result = static_cast<int>(msg.wParam);
    }

    return result;
}

void CApplication::DestroyAnimationVariables()
{
    _animationVariable = nullptr;
}

void CApplication::DestroyAnimationTransitionLibrary()
{
    _transitionLibrary = nullptr;
}

void CApplication::DestroyAnimationManager()
{
    _manager = nullptr;
}

void CApplication::DestroyWICFactory()
{
    _wicFactory = nullptr;
}

void CApplication::DestroyD2D1Device()
{
    _d2d1DeviceContext = nullptr;
    _d2d1Device = nullptr;
}

void CApplication::DestroyD2D1Factory()
{
    _d2d1Factory = nullptr;
}

void CApplication::DestroyD3D11Device()
{
    _d3d11DeviceContext = nullptr;
    _d3d11Device = nullptr;
}


//-----------------------------------------------------------
// Cleanup when we close the application
//-----------------------------------------------------------
void CApplication::AfterLeavingMessageLoop()
{
    DestroyDCompositionVisualTree();
    DestroyDCompositionRenderTarget();
    DestroyDCompositionDevice();
    DestroyApplicationWindow();
    DestroyAnimationTransitionLibrary();
    DestroyAnimationVariables();
    DestroyAnimationManager();
    DestroyApplicationWindow();
    DestroyWICFactory();
    DestroyD2D1Device();
    DestroyD2D1Factory();
    DestroyD3D11Device();
    CoUninitialize();
}

HRESULT CApplication::CreateApplicationWindow()
{
    HRESULT hr = S_OK;
    WNDCLASSEX wcex = { 0 };

    wcex.cbSize = sizeof (wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = CApplication::WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = _hinstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "MainWindowClass";
    wcex.hIconSm = NULL;

    hr = !RegisterClassEx(&wcex) ? E_FAIL : S_OK;

    if (SUCCEEDED(hr))
    {
        RECT rect = { 0, 0, WINDOW_SIZE, WINDOW_SIZE};

        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        _hwnd = CreateWindowExW(
           0,
           L"MainWindowClass",
           L"Windows Animation Manager (WAM) Sample",
           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           rect.right - rect.left,
           rect.bottom - rect.top,
           NULL,
           NULL,
           _hinstance,
           nullptr);

        if (_hwnd == NULL)
        {
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontTypeface[32] = { 0 };

        hr = !LoadStringW(_hinstance, IDS_FONT_TYPEFACE, fontTypeface, ARRAYSIZE(fontTypeface)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopyW(_fontTypeface, ARRAYSIZE(_fontTypeface), fontTypeface);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightLogo[32] = { 0 };

        hr = !LoadStringW(_hinstance, IDS_FONT_HEIGHT_LOGO, fontHeightLogo, ARRAYSIZE(fontHeightLogo)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            _fontHeightLogo = _wtoi(fontHeightLogo);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightTitle[32] = { 0 };

        hr = !LoadStringW(_hinstance, IDS_FONT_HEIGHT_TITLE, fontHeightTitle, ARRAYSIZE(fontHeightTitle)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            _fontHeightTitle = _wtoi(fontHeightTitle);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightDescription[32] = { 0 };

        hr = !LoadStringW(_hinstance, IDS_FONT_HEIGHT_DESCRIPTION, fontHeightDescription, ARRAYSIZE(fontHeightDescription)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            _fontHeightDescription = _wtoi(fontHeightDescription);
        }
    }

    return hr;
}

bool CApplication::ShowApplicationWindow()
{
    if (_hwnd != NULL)
    {
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);
        return true;
    }
    else
    {
        return false;
    }
}

void CApplication::DestroyApplicationWindow()
{
    if (_hwnd != NULL)
    {
        DestroyWindow(_hwnd);
       _hwnd = NULL;
    }
}

//-----------------------------------------------------------
// Creates a DirectComposition device
//-----------------------------------------------------------
HRESULT CApplication::CreateDCompositionDevice()
{
    HRESULT hr = (_d3d11Device == nullptr) ? E_UNEXPECTED : S_OK;
    
    CComPtr<IDXGIDevice> dxgiDevice;

    if (SUCCEEDED(hr))
    {
        hr = _d3d11Device->QueryInterface(&dxgiDevice);
    }

    if (SUCCEEDED(hr))
    {
        hr = DCompositionCreateDevice(dxgiDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void **>(&_device));
    }

    return hr;
}

//--------------------------------------------------------
// Creates an render target for DirectComposition which
// is an hwnd in this case
//--------------------------------------------------------
HRESULT CApplication::CreateDCompositionRenderTarget()
{
    HRESULT hr = ((_device == nullptr) || (_hwnd == NULL)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTargetForHwnd(_hwnd, TRUE, &_target);
    }

    return hr;
}
//---------------------------------------------------------------------
// Creates a DirectComposition visual tree and places each visual 
// inside the application window
//---------------------------------------------------------------------
HRESULT CApplication::CreateDCompositionVisualTree()
{
    static const WCHAR *filename = L"220Strawberry.png";
    static const float tileSize = 0.3f * WINDOW_SIZE;
    static const int visualChildCount = sizeof(_visualChild) / sizeof(_visualChild[0]);
    static const float d = 2.0f * WINDOW_SIZE;

    HRESULT hr = ((_device == nullptr) || (_hwnd == NULL)) ? E_UNEXPECTED : S_OK;

    int bitmapWidth = 0;
    int bitmapHeight = 0;

    CComPtr<IDCompositionSurface> surface;

    // Create DirectComposition surface from the bitmap file
    if (SUCCEEDED(hr))
    {
        hr = CreateSurfaceFromFile(filename, &bitmapWidth, &bitmapHeight, &surface);
    }

    if (SUCCEEDED(hr))
    {
        _bitmapWidth = bitmapWidth;
        _bitmapHeight = bitmapHeight;

        hr = _device->CreateVisual(&_visual);
    }

    // Set the content of each visual to be the surface that was created from the bitmap
    if (SUCCEEDED(hr))
    {
        for (int i = 0; SUCCEEDED(hr) && i < visualChildCount; ++i)
        {
            hr = _device->CreateVisual(&_visualChild[i]);

            if (SUCCEEDED(hr))
            {
                hr = _visual->AddVisual(_visualChild[i], FALSE, nullptr);
            }

            if (SUCCEEDED(hr))
            {
                hr = _visualChild[i]->SetContent(surface);
            }
        }
    }

    // Using DirectComposition transforms to scale and place each visual such that the tiles 
    // are side by side within the application window
    if (SUCCEEDED(hr))
    {
        for (int i = 0; SUCCEEDED(hr) && i < visualChildCount; ++i)
        {
            //setting up scale transform on each visual
            CComPtr<IDCompositionScaleTransform> scaleTransform;

            if (SUCCEEDED(hr))
            {
                hr = _device->CreateScaleTransform(&scaleTransform);
            }
            
            float sx = tileSize / bitmapWidth;

            if (SUCCEEDED(hr))
            {
                hr = scaleTransform->SetScaleX(sx);
            }

            float sy = tileSize / bitmapHeight;

            if (SUCCEEDED(hr))
            {
                hr = scaleTransform->SetScaleY(sy);
            }

            //Setting up a translate transform on each visual
            CComPtr<IDCompositionTranslateTransform> translateTransform;

            if (SUCCEEDED(hr))
            {
                hr = _device->CreateTranslateTransform(&translateTransform);
            }

            float x = (visualChildCount - 1 - i) * TILE_SPACING;
            float y = TILE_SPACING + 30;

            if (SUCCEEDED(hr))
            {
                hr = translateTransform->SetOffsetX(x);
            }

            if (SUCCEEDED(hr))
            {
                hr = translateTransform->SetOffsetY(y);
            }

            // Creating a transform group to group the two transforms together such that 
            // they can be applied at once.
            IDCompositionTransform *transforms[] = 
            { 
                scaleTransform,
                translateTransform,
            };

            CComPtr<IDCompositionTransform> transformGroup;
            if (SUCCEEDED(hr))
            {
                _device->CreateTransformGroup(transforms, sizeof(transforms)/sizeof(transforms[0]), &transformGroup);
            }
            if (SUCCEEDED(hr))
            {
                _visualChild[i]->SetTransform(transformGroup);
            }
        }
    }

    return hr;
}

//-------------------------------------------------------------------------------
// Use WAM to generate and propagate the appropriate animation curves to DirectComposition when 
// keypress is detected
//-------------------------------------------------------------------------------
HRESULT CApplication::CreateSlideAnimation(DIRECTION dir, IDCompositionAnimation **slideAnimation)
{    
    HRESULT hr = (slideAnimation == nullptr) ? E_POINTER : S_OK;

    float rightMargin = 27 * TILE_SPACING * -1;  //where the tiles end. Note forward direction is represented by a negative value.
    float leftMargin = 0; // where the tiles begin

    if (SUCCEEDED(hr))
    {
        *slideAnimation = nullptr;
        hr = ((_device == nullptr) || (_animationVariable == nullptr)) ? E_UNEXPECTED : S_OK;
    }

    //WAM propagates curves to DirectComposition using the IDCompositionAnimation object
    CComPtr<IDCompositionAnimation> animation;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateAnimation(&animation);
    }

    //Create a storyboard for the slide animation
    CComPtr<IUIAnimationStoryboard2> storyboard;

    if (SUCCEEDED(hr))
    {
        hr = _manager->CreateStoryboard(&storyboard);
    }

    // Synchronizing WAM and DirectComposition time such that when WAM Update is called, 
    // the value reflects the DirectComposition value at the given time.
    DCOMPOSITION_FRAME_STATISTICS frameStatistics = { 0 };

    if (SUCCEEDED(hr))
    {
        hr = _device->GetFrameStatistics(&frameStatistics);
    }

    UI_ANIMATION_SECONDS nextEstimatedFrameTime = 0.0;

    if (SUCCEEDED(hr))
    {
        nextEstimatedFrameTime = static_cast<double>(frameStatistics.nextEstimatedFrameTime.QuadPart) / static_cast<double>(frameStatistics.timeFrequency.QuadPart);
    }

    //Upating the WAM time 
    if (SUCCEEDED(hr))
    {
        hr = _manager->Update(nextEstimatedFrameTime);
    }

    CComPtr<IUIAnimationTransition2> transition;
    double curValue = 0;    //current value of the animation variable
    int velocity = 500;     //arbitrary fix velocity for the slide animation
    
    if (SUCCEEDED(hr))
    {
        hr = _animationVariable->GetValue(&curValue);

        switch (dir)
        {
            case stopForward:
            case stopBackward:
                // Stopping the animation smoothly when key is let go
                if (curValue != leftMargin && curValue != rightMargin)
                    hr = _transitionLibrary->CreateSmoothStopTransition(0.5, curValue + dir * 50, &transition);
                break;
            case forward:
                // slide the tiles forward using a linear curve upon left button press
                hr = _transitionLibrary->CreateLinearTransition(-1 * (rightMargin - curValue)/velocity, rightMargin, &transition);
                break;
            case backward:
                // slide the tiles backward using a linear cruve upon right button press
                hr = _transitionLibrary->CreateLinearTransition(-1 * curValue/velocity, leftMargin, &transition);
                break;
         }
    }

    //Add above transition to storyboard
    if (SUCCEEDED(hr))
    {
        hr = storyboard->AddTransition(_animationVariable, transition);
    }

    //schedule the storyboard for play at the next estimate vblank
    if (SUCCEEDED(hr))
    {
        hr = storyboard->Schedule(nextEstimatedFrameTime);
    }

    //Giving WAM varialbe the IDCompositionAnimation object to recieve the animation curves
    if (SUCCEEDED(hr))
    {
        hr = _animationVariable->GetCurve(animation);
    }

    if (SUCCEEDED(hr))
    {
        *slideAnimation = animation.Detach();
    }

    return hr;
}

HRESULT CApplication::AttachDCompositionVisualTreeToRenderTarget()
{
    HRESULT hr = ((_target == nullptr) || (_visual == nullptr)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _target->SetRoot(_visual);
    }

    return hr;
}

HRESULT CApplication::DetachDCompositionVisualTreeToRenderTarget()
{
    HRESULT hr = (_target == nullptr) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _target->SetRoot(nullptr);
    }

    return hr;
}

void CApplication::DestroyDCompositionVisualTree()
{
    for (int i = 0; i < sizeof(_visualChild) / sizeof(_visualChild[0]); ++i)
    {
        _visualChild[i] = nullptr;
    }

    _visual = nullptr;
}

void CApplication::DestroyDCompositionRenderTarget()
{
    _target = nullptr;
}

void CApplication::DestroyDCompositionDevice()
{
    _device = nullptr;
}

void CApplication::OnCreate()
{
    _hbrush = CreateSolidBrush(RGB(255, 255, 255));
}

LRESULT CApplication::OnPaint(HWND hwnd)
{
    RECT rcClient; 
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, _hbrush);

    // get the dimensions of the main window.   
    GetClientRect(_hwnd, &rcClient);

    // Logo
    HFONT hlogo = CreateFontW(_fontHeightLogo, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, _fontTypeface);    // Logo Font and Size
    if (hlogo != NULL)
    {
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hlogo));

        SetBkMode(hdc, TRANSPARENT);

        rcClient.top = 10;
        rcClient.left = 30;

        DrawTextW(hdc, L"Windows samples", -1, &rcClient, DT_WORDBREAK);

        SelectObject(hdc, hOldFont);

        DeleteObject(hlogo);
    }

    // Title
    HFONT htitle = CreateFontW(_fontHeightTitle, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, _fontTypeface);    // Title Font and Size
    if (htitle != NULL)
    {
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, htitle));

        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        
        rcClient.top = 25;
        rcClient.left = 30;
        
        DrawTextW(hdc, L"WAM Sample", -1, &rcClient, DT_WORDBREAK);

        SelectObject(hdc, hOldFont);

        DeleteObject(htitle);
    }

    // Description
    HFONT hdescription = CreateFontW(_fontHeightDescription, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, _fontTypeface);    // Description Font and Size
    if (hdescription != NULL)
    {
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hdescription));

        rcClient.top = 90;
        rcClient.left = 30;
        
        DrawTextW(hdc, L"This sample shows how DirectComposition and Windows Animation Manager (WAM) can be used together as an independent animation platform.", -1, &rcClient, DT_WORDBREAK);

        rcClient.top = 400;
        rcClient.left = 220;
        
        DrawTextW(hdc, L"Left/Right click to control the animation.", -1, &rcClient, DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);

        DeleteObject(hdescription);
    }

    EndPaint(hwnd, &ps); 

    return 0;
}

LRESULT CApplication::OnClose()
{
    if (_hwnd != NULL)
    {
        DestroyWindow(_hwnd);
       _hwnd = NULL;
    }

    return 0;
}

LRESULT CApplication::OnDestroy()
{
    if (_hbrush != NULL)
    {
        DeleteObject(_hbrush);
        _hbrush = NULL;
    }

    PostQuitMessage(0);
    return 0;
}

HRESULT CApplication::CreateSurfaceFromFile(const WCHAR *filename, int *bitmapWidth, int *bitmapHeight, IDCompositionSurface **surface)
{
    HRESULT hr = ((bitmapWidth == nullptr) || (bitmapHeight == nullptr) || (surface == nullptr)) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *bitmapWidth = 0;
        *bitmapHeight = 0;
        *surface = NULL;

        hr = (filename == nullptr) ? E_INVALIDARG : S_OK;
    }

    CComPtr<ID2D1Bitmap> d2d1Bitmap;
    D2D1_SIZE_F bitmapSize = { 0 };

    if (SUCCEEDED(hr))
    {
        hr = CreateD2D1BitmapFromFile(filename, &d2d1Bitmap);
    }

    CComPtr<IDCompositionSurface> surfaceTile;

    if (SUCCEEDED(hr))
    {
        bitmapSize = d2d1Bitmap->GetSize();

        hr = _device->CreateSurface(
            static_cast<UINT>(bitmapSize.width), 
            static_cast<UINT>(bitmapSize.height), 
            DXGI_FORMAT_R8G8B8A8_UNORM, 
            DXGI_ALPHA_MODE_IGNORE, 
            &surfaceTile);
    }

    CComPtr<IDXGISurface> dxgiSurface;
    POINT offset;

    if (SUCCEEDED(hr))
    {
        RECT rect = { 0, 0, static_cast<LONG>(bitmapSize.width), static_cast<LONG>(bitmapSize.height) };
        
        hr = surfaceTile->BeginDraw(&rect, __uuidof(IDXGISurface), reinterpret_cast<void **>(&dxgiSurface), &offset);
    }

    CComPtr<ID2D1Bitmap1> d2d1Target;

    if (SUCCEEDED(hr))
    {
        FLOAT dpiX = 0.0f;
        FLOAT dpiY = 0.0f;

        _d2d1Factory->GetDesktopDpi(&dpiX, &dpiY);

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
            dpiX,
            dpiY);

        hr = _d2d1DeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bitmapProperties, &d2d1Target);

        if (SUCCEEDED(hr))
        {
            _d2d1DeviceContext->SetTarget(d2d1Target);

            _d2d1DeviceContext->BeginDraw();

            _d2d1DeviceContext->DrawBitmap(
                d2d1Bitmap,
                D2D1::RectF(
                    offset.x + 0.0f, 
                    offset.y + 0.0f, 
                    offset.x + bitmapSize.width, 
                    offset.y + bitmapSize.height));

            hr = _d2d1DeviceContext->EndDraw();
        }

        surfaceTile->EndDraw();
    }

    if (SUCCEEDED(hr))
    {
        *bitmapWidth = static_cast<int>(bitmapSize.width);
        *bitmapHeight = static_cast<int>(bitmapSize.height);
        *surface = surfaceTile.Detach();
    }

    return hr;
}

HRESULT CApplication::CreateD2D1BitmapFromFile(LPCWSTR filename, ID2D1Bitmap **bitmap)
{
    HRESULT hr = (bitmap == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *bitmap = nullptr;

        hr = (_wicFactory == nullptr) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IWICBitmapDecoder> wicBitmapDecoder;

    if (SUCCEEDED(hr))
    {
        hr = _wicFactory->CreateDecoderFromFilename(
            filename,
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &wicBitmapDecoder);
    }

    CComPtr<IWICBitmapFrameDecode> wicBitmapFrame;

    if (SUCCEEDED(hr))
    {
        hr = wicBitmapDecoder->GetFrame(0, &wicBitmapFrame);
    }

    CComPtr<IWICFormatConverter> wicFormatConverter;

    if (SUCCEEDED(hr))
    {
        hr = _wicFactory->CreateFormatConverter(&wicFormatConverter);
    }

    if (SUCCEEDED(hr))
    {
        hr = wicFormatConverter->Initialize(
            wicBitmapFrame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0f,
            WICBitmapPaletteTypeMedianCut);
    }

    CComPtr<IWICBitmap> wicBitmap;

    if (SUCCEEDED(hr))
    {
        hr = _wicFactory->CreateBitmapFromSource(wicFormatConverter, WICBitmapCacheOnLoad, &wicBitmap);
    }

    if (SUCCEEDED(hr))
    {
        hr = _d2d1DeviceContext->CreateBitmapFromWicBitmap(wicBitmap, bitmap); 
    }

    return hr;
}

//---------------------------------------------------------------------
// Slides the tiles in the direction of the button press. WAM is 
// used to generate the animation curves while DirectComposition translates the visuals
// using those curves
//--------------------------------------------------------------------
HRESULT CApplication::Move(DIRECTION dir)
{
    HRESULT hr = ((_device == nullptr) || (_hwnd == NULL)) ? E_UNEXPECTED : S_OK;

    CComPtr<IDCompositionAnimation> spAnimation;    
    CComPtr<IDCompositionAnimation> slideAnimation;
    CComPtr<IDCompositionTranslateTransform> translateTransform;

    if (SUCCEEDED(hr))
    {
        // Create the animation curves using WAM
        hr = CreateSlideAnimation(dir, &slideAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTranslateTransform(&translateTransform);
    }

    //Set DirectComposition translation animation using the curves propagated by WAM
    if (SUCCEEDED(hr))
    {
        hr = translateTransform->SetOffsetX(slideAnimation);
    }

    if (SUCCEEDED(hr))
    {
        _visual->SetTransform(translateTransform);
    }

    // Committing all changes to DirectComposition visuals in order for them to take effect visually
    if (SUCCEEDED(hr))
    {
        _device->Commit();
    }
    
    return hr;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR pszCmdLine, _In_ int iCmdShow)
{
    CApplication application(hInstance);
    return application.Run();
}
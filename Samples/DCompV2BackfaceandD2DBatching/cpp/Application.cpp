// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Application.h"
#include <windowsx.h>

Application *Application::_application = nullptr;

// Main window procedure
LRESULT CALLBACK Application::WindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch (msg)
    {
    case WM_POINTERUP:
        {
            result = _application->OnPointerUp(lparam);
            break;
        }

    case WM_CLOSE:
        {
            result = _application->OnClose();
            break;
        }

    case WM_DESTROY:
        {
            result = _application->OnDestroy();
            break;
        }

    default:
        {
            result = DefWindowProc(hWnd, msg, wparam, lparam);
            break;
        }
    }

    return result;
}

// Provides the entry point to the application and defines client size
Application::Application(HINSTANCE hInstance) :
    _hInstance(hInstance),
    _hWnd(NULL),
    _hWndClientWidth(512),
    _hWndClientHeight(512)
{
    _application = this;
}

Application::~Application()
{
    _application = nullptr;
}

// Runs the application
int Application::Run()
{
    int result = 0;

    if (SUCCEEDED(BeforeEnteringMessageLoop()))
    {
        result = EnterMessageLoop();
    }

    AfterLeavingMessageLoop();

    return result;
}

// Initialization for the application window, devices and
// DComp resources before entering the message loop
HRESULT Application::BeforeEnteringMessageLoop()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = CreateMainWindow();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD3DDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD2DDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDWriteFactory();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompVisualTree();
    }

    if (SUCCEEDED(hr))
    {
        hr = ShowMainWindow();
    }

    if (SUCCEEDED(hr))
    {
        hr = EnableMouseAsPointerDevice();
    }

    return hr;
}

// Message loop
int Application::EnterMessageLoop()
{
    MSG message = { 0 };

    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return static_cast<int>(message.wParam);
}

// Destroys the application window, devices and DComp resources
void Application::AfterLeavingMessageLoop()
{
    DestroyDCompVisualTree();

    DestroyDCompDevice();

    DestroyDWriteFactory();

    DestroyD2DDevice();

    DestroyD3DDevice();

    DestroyMainWindow();
}

// Creates the application window
HRESULT Application::CreateMainWindow()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex = { 0 };

        wcex.cbSize = sizeof (wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = Application::WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = _hInstance;
        wcex.hIcon = NULL;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"MainWindowClass";
        wcex.hIconSm = NULL;

        hr = (RegisterClassEx(&wcex) == 0) ? E_FAIL : S_OK;

        ShowMessageBoxIfFailed(hr, L"RegisterClassEx");
    }

    if (SUCCEEDED(hr))
    {
        RECT windowRect = { 0, 0, _hWndClientWidth, _hWndClientHeight };

        hr = (AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE) == FALSE) ? E_FAIL : S_OK;

        ShowMessageBoxIfFailed(hr, L"AdjustWindowRect");

        if (SUCCEEDED(hr))
        {
            _hWnd = CreateWindowExW(
                0,
                L"MainWindowClass",
                L"DirectComposition",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                0,
                0,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                NULL,
                NULL,
                _hInstance,
                nullptr);

            hr = (_hWnd == NULL) ? E_FAIL : S_OK;

            ShowMessageBoxIfFailed(hr, L"CreateWindowExW");
        }
    }

    return hr;
}

// Shows the application window 
HRESULT Application::ShowMainWindow()
{
    HRESULT hr = (_hWnd == NULL) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        ShowWindow(_hWnd, SW_SHOW);
        UpdateWindow(_hWnd);
    }

    return hr;
}

// Destroys the application window 
void Application::DestroyMainWindow()
{
    if (_hWnd != NULL)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
}

// Enables mouse pointer device for hit-testing
HRESULT Application::EnableMouseAsPointerDevice()
{
    HRESULT hr = S_OK;

    if (!IsMouseInPointerEnabled())
    {
        hr = (EnableMouseInPointer(TRUE) == FALSE) ? E_FAIL : S_OK;
    }

    return hr;
}

HRESULT Application::CreateD3DDevice()
{
    HRESULT hr = (_d3dDevice != nullptr) ? E_UNEXPECTED : S_OK;;

    ShowMessageBoxIfFailed(hr, L"(_d3dDevice != nullptr)");

    if (SUCCEEDED(hr))
    {
        D3D_DRIVER_TYPE driverTypes [] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP
        };

        D3D_FEATURE_LEVEL featureLevels [] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        D3D_FEATURE_LEVEL featureLevelSupported;

        for (int i = 0; i < sizeof(driverTypes) / sizeof(driverTypes[0]); ++i)
        {
            Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;

            hr = ::D3D11CreateDevice(
                nullptr,
                driverTypes[i],
                NULL,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                featureLevels,
                sizeof(featureLevels) / sizeof(featureLevels[0]),
                D3D11_SDK_VERSION,
                &d3dDevice,
                &featureLevelSupported,
                nullptr);

            ShowMessageBoxIfFailed(hr, L"::D3D11CreateDevice");

            if (SUCCEEDED(hr))
            {
                _d3dDevice = d3dDevice.Detach();

                break;
            }
        }
    }

    return hr;
}

void Application::DestroyD3DDevice()
{
    _d3dDevice = nullptr;
}

HRESULT Application::CreateD2DDevice()
{
    HRESULT hr = ((_d3dDevice == nullptr) || (_d2dDevice != nullptr)) ? E_UNEXPECTED : S_OK;

    ShowMessageBoxIfFailed(hr, L"((_d3dDevice == nullptr) || (_d2dDevice != nullptr))");

    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;

        hr = _d3dDevice.As(&dxgiDevice);

        ShowMessageBoxIfFailed(hr, L"_d3dDevice->QueryInterface");

        if (SUCCEEDED(hr))
        {
            hr = ::D2D1CreateDevice(dxgiDevice.Get(), nullptr, &_d2dDevice);

            ShowMessageBoxIfFailed(hr, L"::D2D1CreateDevice");
        }
    }

    return hr;
}

void Application::DestroyD2DDevice()
{
    _d2dDevice = nullptr;
}

/*----Begin calling DirectComposition APIs----*/
HRESULT Application::CreateDWriteFactory()
{
    HRESULT hr = (_dwriteFactory != nullptr) ? E_UNEXPECTED : S_OK;

    ShowMessageBoxIfFailed(hr, L"(_dwriteFactory != nullptr)");

    if (SUCCEEDED(hr))
    {
        hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory) , reinterpret_cast<IUnknown **>(_dwriteFactory.GetAddressOf()));

        ShowMessageBoxIfFailed(hr, L"::DWriteCreateFactory");
    }

    return hr;
}

void Application::DestroyDWriteFactory()
{
    _dwriteFactory = nullptr;
}

// Initializing DirectComposition device using a D2D device object
// Pointing to a D2D device object is required for D2D batching
HRESULT Application::CreateDCompDevice()
{
    HRESULT hr = ((_d2dDevice == nullptr) || (_dcompDevice != nullptr)) ? E_UNEXPECTED : S_OK;

    ShowMessageBoxIfFailed(hr, L"((_d2dDevice == nullptr) || (_dcompDevice != nullptr))");

    if (SUCCEEDED(hr))
    {
        hr = ::DCompositionCreateDevice2(_d2dDevice.Get(), __uuidof(IDCompositionDesktopDevice) , reinterpret_cast<void **>(_dcompDevice.GetAddressOf()));

        ShowMessageBoxIfFailed(hr, L"::DCompositionCreateDevice2");
    }

    return hr;
}

void Application::DestroyDCompDevice()
{
    _dcompDevice = nullptr;
}

// Calling functions to build DirectComposition visual tree, creating surfaces and setting visual properties 
HRESULT Application::CreateDCompVisualTree()
{
    HRESULT hr = ((_hWnd == NULL) || (_dcompDevice == nullptr)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompTarget();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompRootVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = BindDCompRootVisualToTarget();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompTextVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompTextSurface();
    }

    if (SUCCEEDED(hr))
    {
        hr = DrawDCompTextSurface();
    }

    if (SUCCEEDED(hr))
    {
        hr = BindDCompTextSurfaceToVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = SetOffsetOnDCompTextVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = AddDCompTextVisualToRootVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompTileVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompTileSurfaces();
    }

    if (SUCCEEDED(hr))
    {
        hr = DrawDCompTileSurfaces();
    }

    if (SUCCEEDED(hr))
    {
        hr = BindDCompTileSurfacesToVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = SetOffsetOnDCompTileVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = SetBackfaceVisibilityOnDCompTileVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = AddDCompTileVisualsToRootVisual();
    }

    if (SUCCEEDED(hr))
    {
        hr = CommitDCompDevice();
    }

    return hr;
}

void Application::DestroyDCompVisualTree()
{
    DestroyDCompTileSurfaces();

    DestroyDCompTileVisuals();

    DestroyDCompTextSurface();

    DestroyDCompTextVisual();

    DestroyDCompRootVisual();

    DestroyDCompTarget();
}

// Creating DirectComposition Target which is associated to an HWND
HRESULT Application::CreateDCompTarget()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateTargetForHwnd(_hWnd, TRUE, &_dcompTarget);

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->CreateTargetForHwnd");
    }

    return hr;
}

void Application::DestroyDCompTarget()
{
    _dcompTarget = nullptr;
}

/*----Creating visuals----*/
HRESULT Application::CreateDCompRootVisual()
{
    HRESULT hr = ((_dcompDevice == nullptr) || (_dcompRootVisual != nullptr)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateVisual(&_dcompRootVisual);

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->CreateVisual");
    }

    return hr;
}

void Application::DestroyDCompRootVisual()
{
    _dcompRootVisual = nullptr;
}

// Creating visual which contains sample description
HRESULT Application::CreateDCompTextVisual()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateVisual(&_dcompTextVisual);

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->CreateVisual");
    }

    return hr;
}

void Application::DestroyDCompTextVisual()
{
    _dcompTextVisual = nullptr;
}

// Creating surface for DCompTextVisual
HRESULT Application::CreateDCompTextSurface()
{
    HRESULT hr = S_OK;

    UINT dcompTextSurfaceWidth = 7 * _hWndClientWidth / 8;
    UINT dcompTextSurfaceHeight = 2 * _hWndClientHeight / 8;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateSurface(dcompTextSurfaceWidth, dcompTextSurfaceHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ALPHA_MODE_IGNORE, &_dcompTextSurface);

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->CreateSurface");
    }

    return hr;
}

void Application::DestroyDCompTextSurface()
{
    _dcompTextSurface = nullptr;
}

// Creating tile visuals
HRESULT Application::CreateDCompTileVisuals()
{
    HRESULT hr = S_OK;

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompDevice->CreateVisual(&_dcompTileVisuals[i]);

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->CreateVisual");
    }

    return hr;
}

void Application::DestroyDCompTileVisuals()
{
    for (int i = 0; i < 4; ++i)
    {
        _dcompTileVisuals[i] = nullptr;
    }
}

// Creating surfaces for the DComp tile visuals
HRESULT Application::CreateDCompTileSurfaces()
{
    HRESULT hr = S_OK;

    UINT dcompTileSurfaceWidth = 2 * _hWndClientWidth / 8;
    UINT dcompTileSurfaceHeight = 2 * _hWndClientHeight / 8;

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompDevice->CreateSurface(dcompTileSurfaceWidth, dcompTileSurfaceHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ALPHA_MODE_IGNORE, &_dcompTileSurfaces[i]);

        ShowMessageBoxIfFailed(hr, L"dcompTileSurfaceFactory->CreateSurface");
    }

    return hr;
}

void Application::DestroyDCompTileSurfaces()
{
    for (int i = 0; i < 4; ++i)
    {
        _dcompTileSurfaces[i] = nullptr;
    }
}
/*----End of creating visuals----*/

/*----Creating surfaces----*/
// Defining text properties and drawing text into the DComp text surface
HRESULT Application::DrawDCompTextSurface()
{
    HRESULT hr = S_OK;

    UINT dcompTextSurfaceWidth = 7 * _hWndClientWidth / 8;
    UINT dcompTextSurfaceHeight = 2 * _hWndClientHeight / 8;

    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dDeviceContext;
        POINT updateOffset = { 0 };

        hr = _dcompTextSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext) , reinterpret_cast<void **>(d2dDeviceContext.GetAddressOf()), &updateOffset);

        ShowMessageBoxIfFailed(hr, L"_dcompTextSurface->BeginDraw");

        if (SUCCEEDED(hr))
        {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dBackgroundBrush;

            if (SUCCEEDED(hr))
            {
                hr = d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &d2dBackgroundBrush);

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->CreateSolidColorBrush");
            }

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dForegroundBrush;

            if (SUCCEEDED(hr))
            {
                hr = d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &d2dForegroundBrush);

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->CreateSolidColorBrush");
            }

            Microsoft::WRL::ComPtr<IDWriteTextFormat> dwriteTextFormat;

            if (SUCCEEDED(hr))
            {
                hr = _dwriteFactory->CreateTextFormat(L"Verdana", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us", &dwriteTextFormat);

                ShowMessageBoxIfFailed(hr, L"dwriteFactory->CreateTextFormat");
            }

            if (SUCCEEDED(hr))
            {
                hr = dwriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);

                ShowMessageBoxIfFailed(hr, L"dwriteTextFormat->SetTextAlignment");
            }

            if (SUCCEEDED(hr))
            {
                hr = dwriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

                ShowMessageBoxIfFailed(hr, L"dwriteTextFormat->SetParagraphAlignment");
            }

            if (SUCCEEDED(hr))
            {
                const wchar_t dcompTileSurfaceText [] = L"This sample demonstrates how to use DirectComposition to apply backface visibility and utilize performance optmization feature known as D2D Batching. Tiles 2 & 3 show backface visible while 1 & 4 show the backface hidden. \n\n"
                    L"Step 1. Tap or click any tile below \n"
                    L"Step 2. Reset by selecting background \n";

                D2D1_RECT_F dcompTextSurfaceRect = D2D1::RectF(
                    updateOffset.x + 0.0f * dcompTextSurfaceWidth,
                    updateOffset.y + 0.0f * dcompTextSurfaceHeight,
                    updateOffset.x + 1.0f * dcompTextSurfaceWidth,
                    updateOffset.y + 1.0f * dcompTextSurfaceHeight
                    );

                d2dDeviceContext->FillRectangle(
                    dcompTextSurfaceRect,
                    d2dBackgroundBrush.Get());

                d2dDeviceContext->DrawText(
                    dcompTileSurfaceText,
                    wcslen(dcompTileSurfaceText),
                    dwriteTextFormat.Get(),
                    &dcompTextSurfaceRect,
                    d2dForegroundBrush.Get());

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->EndDraw");
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _dcompTextSurface->EndDraw();

            ShowMessageBoxIfFailed(hr, L"_dcompTextSurface->EndDraw");
        }
    }

    return hr;
}

// Drawing tile content into tile surfaces using D2D batching
HRESULT Application::DrawDCompTileSurfaces()
{
    HRESULT hr = S_OK;

    // Defining tile colors
    D2D1_COLOR_F dcompTileSurfaceBackgroundColors [] =
    {
        D2D1::ColorF(D2D1::ColorF::LightBlue),
        D2D1::ColorF(D2D1::ColorF::BlueViolet),
        D2D1::ColorF(D2D1::ColorF::Blue),
        D2D1::ColorF(D2D1::ColorF::Cyan)
    };

    D2D1_COLOR_F dcompTileSurfaceForegroundColors [] =
    {
        D2D1::ColorF(D2D1::ColorF::Black),
        D2D1::ColorF(D2D1::ColorF::Black),
        D2D1::ColorF(D2D1::ColorF::Black),
        D2D1::ColorF(D2D1::ColorF::Black),
    };

    // Defining tile content
    const wchar_t *dcompTileSurfaceTexts [] =
    {
        L"1",
        L"2",
        L"3",
        L"4"
    };

    UINT dcompTileSurfaceWidth = 2 * _hWndClientWidth / 8;
    UINT dcompTileSurfaceHeight = 2 * _hWndClientHeight / 8;

    // Drawing content for all tile surfaces
    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dDeviceContext;
        POINT updateOffset = { 0 };

        // Begin draw is using a D2D device context for D2D batching
        hr = _dcompTileSurfaces[i]->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext) , reinterpret_cast<void **>(d2dDeviceContext.GetAddressOf()), &updateOffset);

        ShowMessageBoxIfFailed(hr, L"pDCompSurface->BeginDraw");

        if (SUCCEEDED(hr))
        {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dBackgroundBrush;

            if (SUCCEEDED(hr))
            {
                hr = d2dDeviceContext->CreateSolidColorBrush(dcompTileSurfaceBackgroundColors[i], &d2dBackgroundBrush);

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->CreateSolidColorBrush");
            }

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dForegroundBrush;

            if (SUCCEEDED(hr))
            {
                hr = d2dDeviceContext->CreateSolidColorBrush(dcompTileSurfaceForegroundColors[i], &d2dForegroundBrush);

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->CreateSolidColorBrush");
            }

            Microsoft::WRL::ComPtr<IDWriteTextFormat> dwriteTextFormat;

            if (SUCCEEDED(hr))
            {
                hr = _dwriteFactory->CreateTextFormat(L"Verdana", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"en-us", &dwriteTextFormat);

                ShowMessageBoxIfFailed(hr, L"dwriteFactory->CreateTextFormat");
            }

            if (SUCCEEDED(hr))
            {
                hr = dwriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                ShowMessageBoxIfFailed(hr, L"dwriteTextFormat->SetTextAlignment");
            }

            if (SUCCEEDED(hr))
            {
                hr = dwriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

                ShowMessageBoxIfFailed(hr, L"dwriteTextFormat->SetParagraphAlignment");
            }

            if (SUCCEEDED(hr))
            {
                D2D1_RECT_F dcompTileSurfaceRect = D2D1::RectF(
                    updateOffset.x + 0.0f * dcompTileSurfaceWidth,
                    updateOffset.y + 0.0f * dcompTileSurfaceHeight,
                    updateOffset.x + 1.0f * dcompTileSurfaceWidth,
                    updateOffset.y + 1.0f * dcompTileSurfaceHeight
                    );

                d2dDeviceContext->FillRectangle(
                    dcompTileSurfaceRect,
                    d2dBackgroundBrush.Get());

                d2dDeviceContext->DrawText(
                    dcompTileSurfaceTexts[i],
                    wcslen(dcompTileSurfaceTexts[i]),
                    dwriteTextFormat.Get(),
                    &dcompTileSurfaceRect,
                    d2dForegroundBrush.Get());

                ShowMessageBoxIfFailed(hr, L"d2dDeviceContext->EndDraw");
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _dcompTileSurfaces[i]->EndDraw();

            ShowMessageBoxIfFailed(hr, L"pDCompSurface->EndDraw");
        }
    }

    return hr;
}
/*---- End of creating surfaces ----*/

/*---- Binding surfaces to visuals and root visual to target ----*/
HRESULT Application::BindDCompRootVisualToTarget()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompTarget->SetRoot(_dcompRootVisual.Get());

        ShowMessageBoxIfFailed(hr, L"_dcompTarget->SetRoot");
    }

    return hr;
}

HRESULT Application::BindDCompTextSurfaceToVisual()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompTextVisual->SetContent(_dcompTextSurface.Get());

        ShowMessageBoxIfFailed(hr, L"_dcompTextVisual->SetContent");
    }

    return hr;
}

HRESULT Application::BindDCompTileSurfacesToVisuals()
{
    HRESULT hr = S_OK;

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompTileVisuals[i]->SetContent(_dcompTileSurfaces[i].Get());

        ShowMessageBoxIfFailed(hr, L"_dcompTileVisuals[i]->SetContent");
    }

    return hr;
}
/*---- End of binding visuals ----*/

/*---- Setting offsets to visuals ----*/
HRESULT Application::SetOffsetOnDCompTextVisual()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompTextVisual->SetOffsetX(0.5f * _hWndClientWidth / 8.0f);

        ShowMessageBoxIfFailed(hr, L"_dcompTextVisual->SetOffsetX");
    }

    if (SUCCEEDED(hr))
    {
        hr = _dcompTextVisual->SetOffsetY(0.25f * _hWndClientHeight / 8.0f);

        ShowMessageBoxIfFailed(hr, L"_dcompTextVisual->SetOffsetY");
    }

    return hr;
}

HRESULT Application::SetOffsetOnDCompTileVisuals()
{
    HRESULT hr = S_OK;

    float dcompVisualOffsetX [] =
    {
        1.0f * _hWndClientWidth / 8.0f,
        5.0f * _hWndClientWidth / 8.0f,
        1.0f * _hWndClientWidth / 8.0f,
        5.0f * _hWndClientWidth / 8.0f
    };

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompTileVisuals[i]->SetOffsetX(dcompVisualOffsetX[i]);

        ShowMessageBoxIfFailed(hr, L"_dcompTileVisuals[i]->SetOffsetX");
    }

    float dcompVisualOffsetY [] =
    {
        2.0f * _hWndClientWidth / 8.0f,
        2.0f * _hWndClientWidth / 8.0f,
        5.0f * _hWndClientWidth / 8.0f,
        5.0f * _hWndClientWidth / 8.0f
    };

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompTileVisuals[i]->SetOffsetY(dcompVisualOffsetY[i]);

        ShowMessageBoxIfFailed(hr, L"_dcompTileVisuals[i]->SetOffsetY");
    }

    return hr;
}
/*---- End of setting offsets ----*/

// Setting backface visibility visible or hidden to visuals
HRESULT Application::SetBackfaceVisibilityOnDCompTileVisuals()
{
    HRESULT hr = S_OK;

    DCOMPOSITION_BACKFACE_VISIBILITY dcompVisualBackfaceVisibility [] =
    {
        DCOMPOSITION_BACKFACE_VISIBILITY_HIDDEN,
        DCOMPOSITION_BACKFACE_VISIBILITY_VISIBLE,
        DCOMPOSITION_BACKFACE_VISIBILITY_VISIBLE,
        DCOMPOSITION_BACKFACE_VISIBILITY_HIDDEN
    };

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompTileVisuals[i]->SetBackFaceVisibility(dcompVisualBackfaceVisibility[i]);
    }

    return hr;
}

/*---- Begin binding visuals to root visual to create visual tree ----*/
HRESULT Application::AddDCompTextVisualToRootVisual()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompRootVisual->AddVisual(_dcompTextVisual.Get(), TRUE, nullptr);

        ShowMessageBoxIfFailed(hr, L"_dcompRootVisual->AddVisual");
    }

    return hr;
}

HRESULT Application::AddDCompTileVisualsToRootVisual()
{
    HRESULT hr = S_OK;

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompRootVisual->AddVisual(_dcompTileVisuals[i].Get(), TRUE, (i == 0) ? _dcompTextVisual.Get() : _dcompTileVisuals[i - 1].Get());

        ShowMessageBoxIfFailed(hr, L"_dcompRootVisual->AddVisual");
    }

    return hr;
}
/*---- End of binding viusals ----*/

// Commiting the DComp device
HRESULT Application::CommitDCompDevice()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->Commit();

        ShowMessageBoxIfFailed(hr, L"_dcompDevice->Commit");
    }

    return hr;
}

// Hit-testing visual
LRESULT Application::OnPointerUp(LPARAM lparam)
{
    POINT p = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };

    ::ScreenToClient(_hWnd, &p);

    int visualIndex = -1;

    if (HasAnyVisualBeenHit(p.x, p.y, &visualIndex))
    {
        ShowMessageBoxIfFailed(FlipVisual(visualIndex), L"FlipVisual(visualIndex)");
    }
    else
    {
        ShowMessageBoxIfFailed(ResetVisuals(), L"ResetVisual");
    }

    return 0;
}

// Identify which tile visual has been hit-tested
bool Application::HasAnyVisualBeenHit(int x, int y, int *pVisualIndex)
{
    bool br = (pVisualIndex != nullptr);

    int visualIndex = -1;

    if (br)
    {
        int x0[4] =
        {
            1 * _hWndClientWidth / 8,
            5 * _hWndClientWidth / 8,
            1 * _hWndClientWidth / 8,
            5 * _hWndClientWidth / 8,
        };

        int x1[4] =
        {
            3 * _hWndClientWidth / 8,
            7 * _hWndClientWidth / 8,
            3 * _hWndClientWidth / 8,
            7 * _hWndClientWidth / 8,
        };

        int y0[4] =
        {
            2 * _hWndClientWidth / 8,
            2 * _hWndClientWidth / 8,
            5 * _hWndClientWidth / 8,
            5 * _hWndClientWidth / 8,
        };

        int y1[4] =
        {
            4 * _hWndClientWidth / 8,
            4 * _hWndClientWidth / 8,
            7 * _hWndClientWidth / 8,
            7 * _hWndClientWidth / 8,
        };

        for (int i = 0; i < 4; ++i)
        {
            if ((x0[i] <= x) && (x <= x1[i]) && (y0[i] <= y) && (y <= y1[i]))
            {
                visualIndex = i;
                break;
            }
        }

        br = (visualIndex != -1);
		*pVisualIndex = visualIndex;
    }



    return br;
}

// Apply a 3D flip for hit-tested tile visual
HRESULT Application::FlipVisual(int visualIndex)
{
    HRESULT hr = S_OK;

    Microsoft::WRL::ComPtr<IDCompositionAnimation> dcompAngleAnimation;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateAnimation(&dcompAngleAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompAngleAnimation->AddCubic(0.0, 0.0f, 180.0f, 0.0f, 0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompAngleAnimation->End(1.0, 180.0f);
    }

    Microsoft::WRL::ComPtr<IDCompositionRotateTransform3D> dcompFlipTransform;

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->CreateRotateTransform3D(&dcompFlipTransform);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetCenterX(1.0f * _hWndClientWidth / 8.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetCenterY(1.0f * _hWndClientHeight / 8.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetCenterZ(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetAxisX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetAxisY(1.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetAxisZ(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = dcompFlipTransform->SetAngle(dcompAngleAnimation.Get());
    }

    if (SUCCEEDED(hr))
    {
        hr = _dcompTileVisuals[visualIndex]->SetEffect(dcompFlipTransform.Get());
    }

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->Commit();
    }

    return hr;
}

// Reset all visuals when a hit-test is detected outside of tile visuals
HRESULT Application::ResetVisuals()
{
    HRESULT hr = S_OK;

    for (int i = 0; SUCCEEDED(hr) && i < 4; ++i)
    {
        hr = _dcompTileVisuals[i]->SetEffect(nullptr);
    }

    if (SUCCEEDED(hr))
    {
        hr = _dcompDevice->Commit();
    }

    return hr;
}

LRESULT Application::OnClose()
{
    if (_hWnd != NULL)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }

    return 0;
}

LRESULT Application::OnDestroy()
{
    PostQuitMessage(0);

    return 0;
}

void Application::ShowMessageBoxIfFailed(HRESULT hr, const wchar_t *pMessage)
{
    if (FAILED(hr))
    {
        ::MessageBox(NULL, pMessage, L"Failed", MB_OK);
    }
}
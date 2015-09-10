// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Application.h"
#include "resource.h"
#include <strsafe.h>

CApplication *CApplication::_application = nullptr;

const int CApplication::_gridSize = 100;

// Runs the application
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

// Creates the application window, the d3d device and DirectComposition device and visual tree
// before entering the message loop.
HRESULT CApplication::BeforeEnteringMessageLoop()
{
    HRESULT hr = CreateApplicationWindow();

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
        hr = CreateDCompositionDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionVisualTree();
    }

    return hr;
}

// Message loop
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

// Destroys the application window, DirectComposition device and visual tree.
VOID CApplication::AfterLeavingMessageLoop()
{
    DestroyDCompositionVisualTree();

    DestroyDCompositionDevice();

    DestroyD2D1Device();

    DestroyD2D1Factory();

    DestroyD3D11Device();

    DestroyApplicationWindow();
}

/*---Code calling DirectComposition APIs------------------------------------------------------------*/

// Creates D2D Factory
HRESULT CApplication::CreateD2D1Factory()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2d1Factory);
}

// Creates D2D Device
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

// Creates D3D device
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

// Initializes DirectComposition
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

// Creates DirectComposition visual tree
HRESULT CApplication::CreateDCompositionVisualTree()
{
    HRESULT hr = ((_device == nullptr) || (_hwnd == NULL)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateVisual(&_visual);		
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateVisual(&_visualLeft);
    }

    CComPtr<IDCompositionSurface> surfaceLeft;

    if (SUCCEEDED(hr))
    {
        hr = CreateSurface(CApplication::_tileSize, 1.0f, 0.0f, 0.0f, &surfaceLeft);
    }

    if (SUCCEEDED(hr))
    {
        hr = _visualLeft->SetContent(surfaceLeft);
    }

    for (int i = 0; i < 4 && SUCCEEDED(hr); ++i)
    {
        if (SUCCEEDED(hr))
        {
            hr = _device->CreateVisual(&_visualLeftChild[i]);
        }

        if (i == 0)
        {
            if (SUCCEEDED(hr))
            {
                hr = CreateSurface(CApplication::_tileSize, 0.0f, 1.0f, 0.0f, &_surfaceLeftChild[i]);
            }
        }

        else if (i == 1)
        {
            if (SUCCEEDED(hr))
            {
                hr = CreateSurface(CApplication::_tileSize, 0.5f, 0.0f, 0.5f, &_surfaceLeftChild[i]);
            }
        }

        else if (i == 2)
        {
            if (SUCCEEDED(hr))
            {
                hr = CreateSurface(CApplication::_tileSize, 0.5f, 0.5f, 0.0f, &_surfaceLeftChild[i]);
            }
        }

        else if (i == 3)
        {
            if (SUCCEEDED(hr))
            {
                hr = CreateSurface(CApplication::_tileSize, 0.0f, 0.0f, 1.0f, &_surfaceLeftChild[i]);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _visualLeftChild[i]->SetContent(_surfaceLeftChild[i]);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateVisual(&_visualRight);
    }

    if (SUCCEEDED(hr))
    {
        hr = _visualRight->SetContent(_surfaceLeftChild[_currentVisual]);
    }

    if (SUCCEEDED(hr))
    {
        hr = _visual->AddVisual(_visualLeft, TRUE, nullptr);
    }

    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < 4 && SUCCEEDED(hr); ++i)
        {
            hr = _visualLeft->AddVisual(_visualLeftChild[i], FALSE, nullptr);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = _visual->AddVisual(_visualRight, TRUE, _visualLeft);
    }

    if (SUCCEEDED(hr))
    {
        hr = SetEffectOnVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTargetForHwnd(_hwnd, TRUE, &_target);
    }

    if (SUCCEEDED(hr))
    {
        hr = _target->SetRoot(_visual);
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->Commit();
    }

    return hr;
}

// Creates surface
HRESULT CApplication::CreateSurface(int size, float red, float green, float blue, IDCompositionSurface **surface)
{
    HRESULT hr = (surface == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        hr = ((_device == nullptr) || (_d2d1Factory == nullptr) || (_d2d1DeviceContext == nullptr)) ? E_UNEXPECTED : S_OK;

        *surface = nullptr;
    }

    CComPtr<IDCompositionSurface> surfaceTile;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateSurface(size, size, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ALPHA_MODE_IGNORE, &surfaceTile);
    }

    CComPtr<IDXGISurface> dxgiSurface;
    POINT offset;

    if (SUCCEEDED(hr))
    {
        RECT rect = { 0, 0, size, size };

        hr = surfaceTile->BeginDraw(&rect, __uuidof(IDXGISurface), reinterpret_cast<void **>(&dxgiSurface), &offset);
    }

    CComPtr<ID2D1Bitmap1> d2d1Bitmap;

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

        hr = _d2d1DeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bitmapProperties, &d2d1Bitmap);

        if (SUCCEEDED(hr))
        {
            _d2d1DeviceContext->SetTarget(d2d1Bitmap);
        }

        CComPtr<ID2D1SolidColorBrush> d2d1Brush;

        if (SUCCEEDED(hr))
        {
            hr = _d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(red, green, blue), &d2d1Brush);
        }

        if (SUCCEEDED(hr))
        {
            _d2d1DeviceContext->BeginDraw();

            _d2d1DeviceContext->FillRectangle(
                D2D1::RectF(
                    offset.x + 0.0f,
                    offset.y + 0.0f,
                    offset.x + static_cast<float>(size),
                    offset.y + static_cast<float>(size)
                ),
                d2d1Brush);

            hr = _d2d1DeviceContext->EndDraw();
        }

        surfaceTile->EndDraw();
    }

    if (SUCCEEDED(hr))
    {
        *surface = surfaceTile.Detach();
    }

    return hr;
}

// Sets effects on both the left and the right visuals.
HRESULT CApplication::SetEffectOnVisuals()
{
    HRESULT hr = SetEffectOnVisualLeft();

    if (SUCCEEDED(hr))
    {
        hr = SetEffectOnVisualLeftChildren();
    }

    if (SUCCEEDED(hr))
    {
        hr = SetEffectOnVisualRight();
    }

    return hr;
}

// Sets effect on the left visual.
HRESULT CApplication::SetEffectOnVisualLeft()
{
    HRESULT hr = (_visualLeft == nullptr) ? E_UNEXPECTED : S_OK;

    float beginOffsetX = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 3.0f : 0.5f;
    float endOffsetX = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 0.5f : 3.0f;
    float offsetY = 1.5f;

    float beginAngle = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 0.0f : 30.0f;
    float endAngle = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 30.0f : 0.0f;

    CComPtr<IDCompositionTranslateTransform3D> translateTransform;

    if (SUCCEEDED(hr))
    {
        hr = CreateTranslateTransform(
            beginOffsetX * CApplication::_gridSize, offsetY * CApplication::_gridSize, 0.0f,
            endOffsetX * CApplication::_gridSize, offsetY * CApplication::_gridSize, 0.0f,
            0.25f, 1.25f,
            &translateTransform);
    }

    CComPtr<IDCompositionRotateTransform3D> rotateTransform;

    if (SUCCEEDED(hr))
    {
        hr = CreateRotateTransform(
            3.5f * CApplication::_gridSize, 1.5f * CApplication::_gridSize, 0.0f * CApplication::_gridSize,
            0.0f, 1.0f, 0.0f,
            beginAngle, endAngle,
            0.25f, 1.25f,
            &rotateTransform);
    }

    CComPtr<IDCompositionMatrixTransform3D> perspectiveTransform;

    if (SUCCEEDED(hr))
    {
        hr = CreatePerspectiveTransform(0.0f, 0.0f, -1.0f / (9.0f * CApplication::_gridSize), &perspectiveTransform);
    }

    IDCompositionTransform3D *transforms[] =
    {
        translateTransform,
        rotateTransform,
        perspectiveTransform,
    };

    CComPtr<IDCompositionTransform3D> transformGroup;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTransform3DGroup(transforms, sizeof(transforms) / sizeof(transforms[0]), &transformGroup);
    }

    if (SUCCEEDED(hr))
    {
        _effectGroupLeft = nullptr;

        hr = _device->CreateEffectGroup(&_effectGroupLeft);
    }

    if (SUCCEEDED(hr))
    {
        hr = _effectGroupLeft->SetTransform3D(transformGroup);
    }

    if (SUCCEEDED(hr))
    {
        hr = _visualLeft->SetEffect(_effectGroupLeft);
    }

    return hr;
}

HRESULT CApplication::SetEffectOnVisualLeftChildren()
{
    HRESULT hr = S_OK;

    for (int i = 0; i < 4 && SUCCEEDED(hr); ++i)
    {
        int r = i / 2;
        int c = i % 2;

        CComPtr<IDCompositionScaleTransform3D> scale;

        if (SUCCEEDED(hr))
        {
            hr = CreateScaleTransform(
                0.0f, 0.0f, 0.0f,
                1.0f / 3.0f, 1.0f / 3.0f, 1.0f,
                &scale);
        }

        CComPtr<IDCompositionTranslateTransform3D> translate;

        if (SUCCEEDED(hr))
        {
            hr = CreateTranslateTransform((0.25f + c * 1.5f) * CApplication::_gridSize, (0.25f + r * 1.5f) * CApplication::_gridSize, 0.0f, &translate);
        }

        IDCompositionTransform3D *transforms[] =
        {
            scale,
            translate,
        };

        CComPtr<IDCompositionTransform3D> transformGroup;

        if (SUCCEEDED(hr))
        {
            hr = _device->CreateTransform3DGroup(transforms, sizeof(transforms) / sizeof(transforms[0]), &transformGroup);
        }

        if (SUCCEEDED(hr))
        {
            _effectGroupLeftChild[i] = nullptr;

            hr = _device->CreateEffectGroup(&_effectGroupLeftChild[i]);
        }

        if (SUCCEEDED(hr))
        {
            hr = _effectGroupLeftChild[i]->SetTransform3D(transformGroup);
        }

        if (SUCCEEDED(hr) && i == _currentVisual)
        {
            CComPtr<IDCompositionAnimation> opacityAnimation;

            float beginOpacity = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 1.0f : 0.0f;
            float endOpacity = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 0.0f : 1.0f;

            hr = CreateLinearAnimation(beginOpacity, endOpacity, 0.25f, 1.25f, &opacityAnimation);

            if (SUCCEEDED(hr))
            {
                hr = _effectGroupLeftChild[i]->SetOpacity(opacityAnimation);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _visualLeftChild[i]->SetEffect(_effectGroupLeftChild[i]);
        }
    }

    return hr;
}

// Sets effect on the right visual
HRESULT CApplication::SetEffectOnVisualRight()
{
    HRESULT hr = (_visualRight == nullptr) ? E_UNEXPECTED : S_OK;

    float beginOffsetX = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 6.5f : 3.75f;
    float endOffsetX = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 3.75f : 6.5f;
    float offsetY = 1.5f;

    CComPtr<IDCompositionTranslateTransform3D> translateTransform;

    if (SUCCEEDED(hr))
    {
        hr = CreateTranslateTransform(
            beginOffsetX * CApplication::_gridSize, offsetY * CApplication::_gridSize, 0.0f,
            endOffsetX * CApplication::_gridSize, offsetY * CApplication::_gridSize, 0.0f,
            0.25f, 1.25f,
            &translateTransform);
    }

    IDCompositionTransform3D *transforms[] =
    {
        translateTransform,
    };

    CComPtr<IDCompositionTransform3D> transformGroup;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTransform3DGroup(transforms, sizeof(transforms) / sizeof(transforms[0]), &transformGroup);
    }

    if (SUCCEEDED(hr))
    {
        _effectGroupRight = nullptr;
        hr = _device->CreateEffectGroup(&_effectGroupRight);
    }

    if (SUCCEEDED(hr))
    {
        hr = _effectGroupRight->SetTransform3D(transformGroup);
    }

    if (SUCCEEDED(hr))
    {
        CComPtr<IDCompositionAnimation> opacityAnimation;

        float beginOpacity = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 0.0f : 1.0f;
        float endOpacity = (_actionType == CApplication::ACTION_TYPE::ZOOMOUT) ? 1.0f : 0.0f;

        hr = CreateLinearAnimation(beginOpacity, endOpacity, 0.25f, 1.25f, &opacityAnimation);

        if (SUCCEEDED(hr))
        {
            hr = _effectGroupRight->SetOpacity(opacityAnimation);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = _visualRight->SetEffect(_effectGroupRight);
    }

    return hr;
}

// Creates Translate transform without animation
HRESULT CApplication::CreateTranslateTransform(float offsetX, float offsetY, float offsetZ, IDCompositionTranslateTransform3D **translateTransform)
{
    HRESULT hr = (translateTransform == NULL) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *translateTransform = nullptr;

        hr = (_device == NULL) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionTranslateTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTranslateTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetX(offsetX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetY(offsetY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetZ(offsetZ);
    }

    if (SUCCEEDED(hr))
    {
        *translateTransform = transform.Detach();
    }

    return hr;
}

// Creates Translate transform with animation
HRESULT CApplication::CreateTranslateTransform(float beginOffsetX, float beginOffsetY, float beginOffsetZ, float endOffsetX, float endOffsetY, float endOffsetZ, float beginTime, float endTime, IDCompositionTranslateTransform3D **translateTransform)
{
    HRESULT hr = (translateTransform == NULL) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *translateTransform = nullptr;

        hr = (_device == NULL) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionTranslateTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateTranslateTransform3D(&transform);
    }

    CComPtr<IDCompositionAnimation> offsetXAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginOffsetX, endOffsetX, beginTime, endTime, &offsetXAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetX(offsetXAnimation);
    }

    CComPtr<IDCompositionAnimation> offsetYAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginOffsetY, endOffsetY, beginTime, endTime, &offsetYAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetY(offsetYAnimation);
    }

    CComPtr<IDCompositionAnimation> offsetZAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginOffsetZ, endOffsetZ, beginTime, endTime, &offsetZAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetOffsetZ(offsetZAnimation);
    }

    if (SUCCEEDED(hr))
    {
        *translateTransform = transform.Detach();
    }

    return hr;
}

// Creates scale transform without animation
HRESULT CApplication::CreateScaleTransform(float centerX, float centerY, float centerZ, float scaleX, float scaleY, float scaleZ, IDCompositionScaleTransform3D **scaleTransform)
{
    HRESULT hr = (scaleTransform == NULL) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *scaleTransform = nullptr;

        hr = (_device == NULL) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionScaleTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateScaleTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterX(centerX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterY(centerY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterZ(centerZ);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleX(scaleX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleY(scaleY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleZ(scaleZ);
    }

    if (SUCCEEDED(hr))
    {
        *scaleTransform = transform.Detach();
    }

    return hr;
}

// Creates scale transform with animation
HRESULT CApplication::CreateScaleTransform(float centerX, float centerY, float centerZ, float beginScaleX, float beginScaleY, float beginScaleZ, float endScaleX, float endScaleY, float endScaleZ, float beginTime, float endTime, IDCompositionScaleTransform3D **scaleTransform)
{
    HRESULT hr = (scaleTransform == NULL) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *scaleTransform = nullptr;

        hr = (_device == NULL) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionScaleTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateScaleTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterX(centerX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterY(centerY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterZ(centerZ);
    }

    CComPtr<IDCompositionAnimation> scaleXAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginScaleX, endScaleX, beginTime, endTime, &scaleXAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleX(scaleXAnimation);
    }

    CComPtr<IDCompositionAnimation> scaleYAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginScaleY, endScaleY, beginTime, endTime, &scaleYAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleY(scaleYAnimation);
    }

    CComPtr<IDCompositionAnimation> scaleZAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginScaleZ, endScaleZ, beginTime, endTime, &scaleZAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetScaleZ(scaleZAnimation);
    }

    if (SUCCEEDED(hr))
    {
        *scaleTransform = transform.Detach();
    }

    return hr;
}

// Creates rotate transform without animation
HRESULT CApplication::CreateRotateTransform(float centerX, float centerY, float centerZ, float axisX, float axisY, float axisZ, float angle, IDCompositionRotateTransform3D **rotateTransform)
{
    HRESULT hr = (rotateTransform == NULL) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *rotateTransform = nullptr;

        hr = (_device == NULL) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionRotateTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateRotateTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterX(centerX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterY(centerY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterZ(centerZ);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisX(axisX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisY(axisY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisZ(axisZ);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAngle(angle);
    }

    if (SUCCEEDED(hr))
    {
        *rotateTransform = transform.Detach();
    }

    return hr;
}

// Creates rotate transform with animation
HRESULT CApplication::CreateRotateTransform(float centerX, float centerY, float centerZ, float axisX, float axisY, float axisZ, float beginAngle, float endAngle, float beginTime, float endTime, IDCompositionRotateTransform3D **rotateTransform)
{
    HRESULT hr = (rotateTransform == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *rotateTransform = nullptr;

        hr = (_device == nullptr) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionRotateTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateRotateTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterX(centerX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterY(centerY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetCenterZ(centerZ);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisX(axisX);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisY(axisY);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAxisZ(axisZ);
    }

    CComPtr<IDCompositionAnimation> angleAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginAngle, endAngle, beginTime, endTime, &angleAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetAngle(angleAnimation);
    }

    if (SUCCEEDED(hr))
    {
        *rotateTransform = transform.Detach();
    }

    return hr;
}

HRESULT CApplication::CreateLinearAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **linearAnimation)
{
    HRESULT hr = (linearAnimation == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *linearAnimation = nullptr;

        hr = (_device == nullptr) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionAnimation> animation;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateAnimation(&animation);
    }

    // Ensures animation start value takes effect immediately
    if (SUCCEEDED(hr))
    {
        if (beginTime > 0.0)
        {
            hr = animation->AddCubic(0.0, beginValue, 0.0f, 0.0f, 0.0f);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->AddCubic(beginTime, beginValue, (endValue - beginValue) / (endTime - beginTime), 0.0f, 0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->End(endTime, endValue);
    }

    if (SUCCEEDED(hr))
    {
        *linearAnimation = animation.Detach();
    }

    return hr;
}

// Creates perspective transform
HRESULT CApplication::CreatePerspectiveTransform(float dx, float dy, float dz, IDCompositionMatrixTransform3D **perspectiveTransform)
{
    HRESULT hr = (perspectiveTransform == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *perspectiveTransform = nullptr;
    }

    D3DMATRIX matrix;
    matrix._11 = 1.0f; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = dx;
    matrix._21 = 0.0f; matrix._22 = 1.0f; matrix._23 = 0.0f; matrix._24 = dy;
    matrix._31 = 0.0f; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = dz;
    matrix._41 = 0.0f; matrix._42 = 0.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;

    CComPtr<IDCompositionMatrixTransform3D> transform;

    if (SUCCEEDED(hr))
    {
        hr = _device->CreateMatrixTransform3D(&transform);
    }

    if (SUCCEEDED(hr))
    {
        hr = transform->SetMatrix(matrix);
    }

    if (SUCCEEDED(hr))
    {
        *perspectiveTransform = transform.Detach();
    }

    return hr;
}

// The child visual associated with the pressed key disappears and the previously disappeared one appears again.
LRESULT CApplication::UpdateVisuals(int currentVisual, int nextVisual)
{
    HRESULT hr = _visualRight->SetContent(_surfaceLeftChild[nextVisual]);

    if (SUCCEEDED(hr))
    {
        hr = _effectGroupLeftChild[currentVisual]->SetOpacity(1.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = _effectGroupLeftChild[nextVisual]->SetOpacity(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->Commit();
    }

    return SUCCEEDED(hr) ? 0 : 1;
}

// Destroys D2D Device
VOID CApplication::DestroyD2D1Device()
{
    _d2d1DeviceContext = nullptr;
    _d2d1Device = nullptr;
}

// Destroys D3D device
VOID CApplication::DestroyD3D11Device()
{
    _d3d11DeviceContext = nullptr;
    _d3d11Device = nullptr;
}

// Destroy D2D Factory
VOID CApplication::DestroyD2D1Factory()
{
    _d2d1Factory = nullptr;
}

// Destroys DirectComposition Visual tree
VOID CApplication::DestroyDCompositionVisualTree()
{
    _effectGroupRight = nullptr;

    for (int i = 0; i < 4; ++i)
    {
        _effectGroupLeftChild[i] = nullptr;
    }

    _effectGroupLeft = nullptr;

    for (int i = 0; i < 4; ++i)
    {
        _surfaceLeftChild[i] = nullptr;
    }

    _visualRight = nullptr;

    for (int i = 0; i < 4; ++i)
    {
        _visualLeftChild[i] = nullptr;
    }

    _visualLeft = nullptr;

    _visual = nullptr;

    _target = nullptr;
}

// Destroys DirectComposition device
VOID CApplication::DestroyDCompositionDevice()
{
    _device = nullptr;
}
/*---End of code calling DirectComposition APIs-------------------------------------------------------*/

// Main Window procedure
LRESULT CALLBACK CApplication::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (msg)
    {
        case WM_LBUTTONUP:
            result = _application->OnLeftButton();
            break;

        case WM_KEYDOWN:
            result = _application->OnKeyDown(wParam);
            break;

        case WM_CLOSE:
            result = _application->OnClose();
            break;

        case WM_DESTROY:
            result = _application->OnDestroy();
            break;

        case WM_PAINT:
            result = _application->OnPaint();
            break;

        default:
            result = DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return result;
}

// Provides the entry point to the application
CApplication::CApplication(HINSTANCE instance) :
    _hinstance(instance),
    _hwnd(NULL),
    _tileSize(3 * CApplication::_gridSize),
    _windowWidth(9 * CApplication::_gridSize),
    _windowHeight(6 * CApplication::_gridSize),
    _state(CApplication::VIEW_STATE::ZOOMEDOUT),
    _actionType(CApplication::ACTION_TYPE::ZOOMOUT),
    _currentVisual(0)
{
    _application = this;
}

CApplication::~CApplication()
{
    _application = nullptr;
}

// Creates the application window
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
        RECT rect = { 0, 0, _windowWidth, _windowHeight };

        AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

        _hwnd = CreateWindowExW(
           0,
           L"MainWindowClass",
           L"DirectComposition Effects Sample",
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

// Shows the application window
BOOL CApplication::ShowApplicationWindow()
{
    BOOL bSucceeded = (_hwnd == NULL) ? FALSE : TRUE;

    if (bSucceeded)
    {
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);
    }

    return bSucceeded;
}

// Destroys the applicaiton window
VOID CApplication::DestroyApplicationWindow()
{
    if (_hwnd != NULL)
    {
        DestroyWindow(_hwnd);
       _hwnd = NULL;
    }
}

// Zoom out to have all the picture on sight.
HRESULT CApplication::ZoomOut()
{
    HRESULT hr = (_state == CApplication::VIEW_STATE::ZOOMEDOUT) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        _actionType = CApplication::ACTION_TYPE::ZOOMOUT;
        hr = SetEffectOnVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->Commit();
    }

    if (SUCCEEDED(hr))
    {
        _state = CApplication::VIEW_STATE::ZOOMEDOUT;
    }

    return hr;
}

// Zoom in to look more closely to the selected pictures
HRESULT CApplication::ZoomIn()
{
    HRESULT hr = (_state == CApplication::VIEW_STATE::ZOOMEDIN) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        _actionType = CApplication::ACTION_TYPE::ZOOMIN;
        hr = SetEffectOnVisuals();
    }

    if (SUCCEEDED(hr))
    {
        hr = _device->Commit();
    }

    if (SUCCEEDED(hr))
    {
        _state = CApplication::VIEW_STATE::ZOOMEDIN;
    }

    return hr;
}

// Handles the WM_LBUTTONUP message
LRESULT CApplication::OnLeftButton()
{
    HRESULT hr = (_state == CApplication::VIEW_STATE::ZOOMEDOUT) ? ZoomIn() : ZoomOut();

    return SUCCEEDED(hr) ? 0 : 1;
}

// Handles the WM_KEYDOWN message
LRESULT CApplication::OnKeyDown(WPARAM wParam)
{
    LRESULT lr = 0;

    if (_state == CApplication::VIEW_STATE::ZOOMEDOUT)
    {
        if (wParam == '1' && _currentVisual != 0)
        {
            lr = UpdateVisuals(_currentVisual, 0);
            _currentVisual = 0;
        }

        else if (wParam == '2' && _currentVisual != 1)
        {
            lr = UpdateVisuals(_currentVisual, 1);
            _currentVisual = 1;
        }

        else if (wParam == '3' && _currentVisual != 2)
        {
            lr = UpdateVisuals(_currentVisual, 2);
            _currentVisual = 2;
        }

        else if (wParam == '4' && _currentVisual != 3)
        {
            lr = UpdateVisuals(_currentVisual, 3);
            _currentVisual = 3;
        }
    }

    return lr;
}

// Handles the WM_CLOSE message
LRESULT CApplication::OnClose()
{
    if (_hwnd != NULL)
    {
        DestroyWindow(_hwnd);
       _hwnd = NULL;
    }

    return 0;
}

// Handles the WM_DESTROY message
LRESULT CApplication::OnDestroy()
{
    PostQuitMessage(0);

    return 0;
}

// Handles the WM_PAINT message
LRESULT CApplication::OnPaint()
{
    RECT rcClient;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(_hwnd, &ps);

    // get the dimensions of the main window.
    GetClientRect(_hwnd, &rcClient);

    // Logo
    HFONT hlogo = CreateFontW(_fontHeightLogo, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, _fontTypeface);    // Logo Font and Size
    if (hlogo != NULL)
    {
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hlogo));

        SetBkMode(hdc, TRANSPARENT);

        rcClient.top = 10;
        rcClient.left = 50;

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
        rcClient.left = 50;

        DrawTextW(hdc, L"DirectComposition Effects Sample", -1, &rcClient, DT_WORDBREAK);

        SelectObject(hdc, hOldFont);

        DeleteObject(htitle);
    }

    // Description
    HFONT hdescription = CreateFontW(_fontHeightDescription, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, _fontTypeface);    // Description Font and Size
    if (hdescription != NULL)
    {
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hdescription));

        rcClient.top = 90;
        rcClient.left = 50;

        DrawTextW(hdc, L"This sample explains how to use DirectComposition effects: rotation, scaling, perspective, translation and opacity.", -1, &rcClient, DT_WORDBREAK);

        rcClient.top = 500;
        rcClient.left = 450;

        DrawTextW(hdc, L"A) Left-click to toggle between single and multiple-panels view.\nB) Use keys 1-4 to switch the color of the right-panel.", -1, &rcClient, DT_WORDBREAK);

        SelectObject(hdc, hOldFont);

        DeleteObject(hdescription);
    }

    EndPaint(_hwnd, &ps);

    return 0;
}
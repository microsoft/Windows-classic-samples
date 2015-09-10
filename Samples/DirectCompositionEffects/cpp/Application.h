// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <windows.h>
#include <atlbase.h>
#include <d3d11_1.h>
#include <d2d1_1.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <math.h>

class CApplication
{
public:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    explicit CApplication(HINSTANCE hInstance);
    ~CApplication();

    int Run();

private:
    CApplication(const CApplication &); 
    CApplication &operator=(const CApplication &); 

private:
    HRESULT BeforeEnteringMessageLoop();
    INT EnterMessageLoop();
    VOID AfterLeavingMessageLoop();

    HRESULT CreateApplicationWindow();
    BOOL ShowApplicationWindow();
    VOID DestroyApplicationWindow();

    HRESULT CreateD2D1Factory();
    VOID DestroyD2D1Factory();

    HRESULT CreateD2D1Device();
    VOID DestroyD2D1Device();

    HRESULT CreateD3D11Device();
    VOID DestroyD3D11Device();

    HRESULT CreateDCompositionDevice();
    VOID DestroyDCompositionVisualTree();

    HRESULT CreateDCompositionVisualTree();
    VOID DestroyDCompositionDevice();

    HRESULT CreateSurface(int size, float red, float green, float blue, IDCompositionSurface **surface);

    HRESULT CreateTranslateTransform(float offsetX, float offsetY, float offsetZ, IDCompositionTranslateTransform3D **translateTransform);
    HRESULT CreateTranslateTransform(float beginOffsetX, float beginOffsetY, float beginOffsetZ, float endOffsetX, float endOffsetY, float endOffsetZ, float beginTime, float endTime, IDCompositionTranslateTransform3D **translateTransform);

    HRESULT CreateScaleTransform(float centerX, float centerY, float centerZ, float scaleX, float scaleY, float scaleZ, IDCompositionScaleTransform3D **scaleTransform);
    HRESULT CreateScaleTransform(float centerX, float centerY, float centerZ, float beginScaleX, float beginScaleY, float beginScaleZ, float endScaleX, float endScaleY, float endScaleZ, float beginTime, float endTime, IDCompositionScaleTransform3D **scaleTransform);

    HRESULT CreateRotateTransform(float centerX, float centerY, float centerZ, float axisX, float axisY, float axisZ, float angle, IDCompositionRotateTransform3D **rotateTransform);
    HRESULT CreateRotateTransform(float centerX, float centerY, float centerZ, float axisX, float axisY, float axisZ, float beginAngle, float endAngle, float beginTime, float endTime, IDCompositionRotateTransform3D **rotateTransform);
    
    HRESULT CreatePerspectiveTransform(float dx, float dy, float dz, IDCompositionMatrixTransform3D **perspectiveTransform);

    HRESULT CreateLinearAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **Animation);

    HRESULT SetEffectOnVisuals();
    HRESULT SetEffectOnVisualLeft();
    HRESULT SetEffectOnVisualLeftChildren();
    HRESULT SetEffectOnVisualRight();

    HRESULT ZoomOut();
    HRESULT ZoomIn();

    LRESULT OnKeyDown(WPARAM wParam);
    LRESULT OnLeftButton();
    LRESULT OnClose();
    LRESULT OnDestroy();
    LRESULT OnPaint();

    LRESULT UpdateVisuals(int currentVisual, int nextVisual);

private:
    static CApplication *_application;

    static const int _gridSize;

private:
    HINSTANCE _hinstance;

    WCHAR _fontTypeface[32];
    int _fontHeightLogo;
    int _fontHeightTitle;
    int _fontHeightDescription;

    HWND _hwnd;

    int _tileSize;

    int _windowWidth;
    int _windowHeight;

    CComPtr<ID3D11Device> _d3d11Device;
    CComPtr<ID3D11DeviceContext> _d3d11DeviceContext;

    CComPtr<ID2D1Factory1> _d2d1Factory;

    CComPtr<ID2D1Device> _d2d1Device;
    CComPtr<ID2D1DeviceContext> _d2d1DeviceContext;

    CComPtr<IDCompositionDevice> _device;
    CComPtr<IDCompositionTarget> _target;
    CComPtr<IDCompositionVisual> _visual;
    CComPtr<IDCompositionVisual> _visualLeft;
    CComPtr<IDCompositionVisual> _visualLeftChild[4];
    CComPtr<IDCompositionVisual> _visualRight;

    CComPtr<IDCompositionSurface> _surfaceLeftChild[4];
    
    CComPtr<IDCompositionEffectGroup> _effectGroupLeft;
    CComPtr<IDCompositionEffectGroup> _effectGroupLeftChild[4];
    CComPtr<IDCompositionEffectGroup> _effectGroupRight;

    int _currentVisual;

    enum VIEW_STATE
    {
        ZOOMEDOUT = 0,
        ZOOMEDIN,
    };

    VIEW_STATE _state;

    enum ACTION_TYPE
    {
        ZOOMOUT = 0,
        ZOOMIN,
    };

    ACTION_TYPE _actionType;
};
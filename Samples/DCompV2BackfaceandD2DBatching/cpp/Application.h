// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <windows.h>
#include <wrl.h>
#include <d2d1.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dwrite.h>

class Application
{
public:
    Application(HINSTANCE hInstance);
    ~Application();

    int Run();

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    Application(const Application &); // not implemented
    Application &operator=(const Application &); // not implemented

private:
    HRESULT BeforeEnteringMessageLoop();
    int EnterMessageLoop();
    void AfterLeavingMessageLoop();

    HRESULT CreateMainWindow();
    HRESULT ShowMainWindow();
    void DestroyMainWindow();

    HRESULT EnableMouseAsPointerDevice();

    HRESULT CreateD3DDevice();
    void DestroyD3DDevice();

    HRESULT CreateD2DDevice();
    void DestroyD2DDevice();

    HRESULT CreateDWriteFactory();
    void DestroyDWriteFactory();

    HRESULT CreateDCompDevice();
    void DestroyDCompDevice();

    HRESULT CreateDCompVisualTree();
    void DestroyDCompVisualTree();

    HRESULT CreateDCompTarget();
    void DestroyDCompTarget();

    HRESULT CreateDCompRootVisual();
    void DestroyDCompRootVisual();

    HRESULT CreateDCompTextVisual();
    void DestroyDCompTextVisual();

    HRESULT CreateDCompTextSurface();
    void DestroyDCompTextSurface();

    HRESULT CreateDCompTileVisuals();
    void DestroyDCompTileVisuals();

    HRESULT CreateDCompTileSurfaces();
    void DestroyDCompTileSurfaces();

    HRESULT DrawDCompTextSurface();
    HRESULT DrawDCompTileSurfaces();

    HRESULT BindDCompRootVisualToTarget();
    HRESULT BindDCompTextSurfaceToVisual();
    HRESULT BindDCompTileSurfacesToVisuals();

    HRESULT SetOffsetOnDCompTextVisual();
    HRESULT SetOffsetOnDCompTileVisuals();

    HRESULT SetBackfaceVisibilityOnDCompTileVisuals();

    HRESULT AddDCompTextVisualToRootVisual();
    HRESULT AddDCompTileVisualsToRootVisual();

    HRESULT CommitDCompDevice();

    LRESULT OnPointerUp(LPARAM lparam);
    LRESULT OnClose();
    LRESULT OnDestroy();

    bool HasAnyVisualBeenHit(int x, int y, int *pVisualIndex);
    HRESULT FlipVisual(int visualIndex);
    HRESULT ResetVisuals();

    void ShowMessageBoxIfFailed(HRESULT hr, const wchar_t *pMessage);

private:
    static Application *_application;

private:
    HINSTANCE _hInstance;

    HWND _hWnd;
    UINT _hWndClientWidth;
    UINT _hWndClientHeight;

    Microsoft::WRL::ComPtr<ID3D11Device> _d3dDevice;

    Microsoft::WRL::ComPtr<ID2D1Device> _d2dDevice;

    Microsoft::WRL::ComPtr<IDWriteFactory> _dwriteFactory;

    Microsoft::WRL::ComPtr<IDCompositionDesktopDevice> _dcompDevice;
    Microsoft::WRL::ComPtr<IDCompositionTarget> _dcompTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual2> _dcompRootVisual;
    Microsoft::WRL::ComPtr<IDCompositionVisual2> _dcompTextVisual;
    Microsoft::WRL::ComPtr<IDCompositionSurface> _dcompTextSurface;
    Microsoft::WRL::ComPtr<IDCompositionVisual2> _dcompTileVisuals[4];
    Microsoft::WRL::ComPtr<IDCompositionSurface> _dcompTileSurfaces[4];

};
//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DirectXHelper.h"                   // For ThrowIfFailed

// Setup Auto initializer for COM for the WIC Factory used in the DeviceResources.
struct AutoCoInitialize
{
    AutoCoInitialize()
    {
		DX::ThrowIfFailed(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
	}

    ~AutoCoInitialize()
    {
        CoUninitialize();
    }
};

	// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
	interface IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	// Controls all the DirectX device resources.
    class DeviceResources
    {
    public:
        typedef struct {
            float X;
            float Y;
            float Width;
            float Height;
        } Rect;

        typedef struct {
            float Width;
            float Height;
        } Size;
        DeviceResources();
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(HWND window, float dpi);
		void SetLogicalSize(Size& logicalSize);
        void SetDpi(float dpi);
        void ValidateDevice();
		void RegisterDeviceNotify(IDeviceNotify* handler);
		void HandleDeviceLost();
        void Trim();
        bool Present();

        // Private Font handling.
        void LoadFonts(std::wstring path);
        IDWriteFontCollection*  GetFontCollection();

        // Device Accessors.
		Size GetOutputSize() const                                      { return m_outputSize; }
		Size GetLogicalSize() const                                     { return m_logicalSize; }
		float GetDpi() const                                            { return m_dpi; }

        // D3D Accessors.
        ID3D11Device2*          GetD3DDevice() const                    { return m_d3dDevice.Get(); }
        ID3D11DeviceContext2*   GetD3DDeviceContext() const             { return m_d3dContext.Get(); }
        IDXGISwapChain1*        GetSwapChain() const                    { return m_swapChain.Get(); }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const           { return m_d3dFeatureLevel; }
        ID3D11RenderTargetView* GetBackBufferRenderTargetView() const   { return m_d3dRenderTargetView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const             { return m_d3dDepthStencilView.Get(); }
        D3D11_VIEWPORT          GetScreenViewport() const               { return m_screenViewport; }
		IDXGIFactory2*          GetDxgiFactory() const;

        // D2D Accessors.
        ID2D1Factory2*          GetD2DFactory() const                   { return m_d2dFactory.Get(); }
        ID2D1Device1*           GetD2DDevice() const                    { return m_d2dDevice.Get(); }
        ID2D1DeviceContext1*    GetD2DDeviceContext() const             { return m_d2dContext.Get(); }
        ID2D1Bitmap1*           GetD2DTargetBitmap() const              { return m_d2dTargetBitmap.Get(); }
        IDWriteFactory2*        GetDWriteFactory() const                { return m_dwriteFactory.Get(); }
        IWICImagingFactory2*    GetWicImagingFactory() const            { return m_wicFactory.Get(); }

    private:
        // Initializer for COM for WIC Factory.
        AutoCoInitialize m_comHelper;

        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D11Device2>        m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext2> m_d3dContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>      m_swapChain;

        // Direct3D rendering objects. Required for 3D.
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;
        D3D11_VIEWPORT                                 m_screenViewport;

        // Direct2D drawing components.
        Microsoft::WRL::ComPtr<ID2D1Factory2>       m_d2dFactory;
        Microsoft::WRL::ComPtr<ID2D1Device1>        m_d2dDevice;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext1> m_d2dContext;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1>        m_d2dTargetBitmap;

        // DirectWrite drawing components.
        Microsoft::WRL::ComPtr<IDWriteFactory2>     m_dwriteFactory;
        Microsoft::WRL::ComPtr<IWICImagingFactory2> m_wicFactory;

        // Cached HWND
        HWND                                            m_hWnd;
        // Cached device properties.
        D3D_FEATURE_LEVEL                               m_d3dFeatureLevel;
        Size                                            m_d3dRenderTargetSize;
		Size                                            m_outputSize;
		Size                                            m_logicalSize;
        float                                           m_dpi;

		IDeviceNotify* m_deviceNotify;
	};
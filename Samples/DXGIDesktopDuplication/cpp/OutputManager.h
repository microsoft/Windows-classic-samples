// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <stdio.h>

#include "CommonTypes.h"
#include "warning.h"

//
// Handles the task of drawing into a window.
// Has the functionality to draw the mouse given a mouse shape buffer and position
//
class OutputManager
{
public:
	OutputManager();
	~OutputManager();
	DUPL_RETURN InitOutput(HWND Window, INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds);
	DUPL_RETURN UpdateApplicationWindow(_In_ PointerInfo* PointerInfo, _Inout_ bool* Occluded);
	void CleanRefs();
	HANDLE GetSharedHandle();
	void WindowResize();

private:
	// Methods
	DUPL_RETURN ProcessMonoMask(bool IsMono, _Inout_ PointerInfo* PtrInfo, _Out_ INT* PtrWidth, _Out_ INT* PtrHeight, _Out_ INT* PtrLeft, _Out_ INT* PtrTop, _Inout_ std::vector<BYTE>& InitBuffer, _Out_ D3D11_BOX* Box);
	DUPL_RETURN MakeRTV();
	void SetViewPort(UINT Width, UINT Height);
	DUPL_RETURN InitShaders();
	DUPL_RETURN CreateSharedSurf(INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds);
	DUPL_RETURN DrawFrame();
	DUPL_RETURN DrawMouse(_In_ PointerInfo* PtrInfo);
	DUPL_RETURN ResizeSwapChain();

	// Vars
	winrt::com_ptr<IDXGISwapChain1> m_SwapChain;
	winrt::com_ptr<ID3D11Device> m_Device;
	winrt::com_ptr<IDXGIFactory2> m_Factory;
	winrt::com_ptr<ID3D11DeviceContext> m_DeviceContext;
	winrt::com_ptr<ID3D11RenderTargetView> m_RTV;
	winrt::com_ptr<ID3D11SamplerState> m_SamplerLinear;
	winrt::com_ptr<ID3D11BlendState> m_BlendState;
	winrt::com_ptr<ID3D11VertexShader> m_VertexShader;
	winrt::com_ptr<ID3D11PixelShader> m_PixelShader;
	winrt::com_ptr<ID3D11InputLayout> m_InputLayout;
	winrt::com_ptr<ID3D11Texture2D> m_SharedSurf;
	winrt::com_ptr<IDXGIKeyedMutex> m_KeyMutex;
	HWND m_WindowHandle;
	bool m_NeedsResize;
	DWORD m_OcclusionCookie;
};

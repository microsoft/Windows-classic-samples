// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <d2d1effects_2.h>
#include <sal.h>
#include <new>
#include <mutex>
#include <span>
#include <warning.h>
#include <DirectXMath.h>
#include <winrt\base.h>
#include <winrt\windows.foundation.h>
#include <winrt\windows.graphics.display.h>

#include "PixelShader.h"
#include "VertexShader.h"

DEFINE_ENUM_FLAG_OPERATORS(D3D11_CREATE_DEVICE_FLAG);

#define NUMVERTICES 6
#define BPP         4

#define OCCLUSION_STATUS_MSG WM_USER

extern HRESULT SystemTransitionsExpectedErrors[];
extern HRESULT CreateDuplicationExpectedErrors[];
extern HRESULT FrameInfoExpectedErrors[];
extern HRESULT AcquireFrameExpectedError[];
extern HRESULT EnumOutputsExpectedErrors[];

_Return_type_success_(return == DUPL_RETURN_SUCCESS)
enum DUPL_RETURN
{
	DUPL_RETURN_SUCCESS = 0,
	DUPL_RETURN_ERROR_EXPECTED = 1,
	DUPL_RETURN_ERROR_UNEXPECTED = 2
};

_Post_satisfies_(return != DUPL_RETURN_SUCCESS)
DUPL_RETURN ProcessFailure(_In_opt_ ID3D11Device * Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ HRESULT * ExpectedErrors = nullptr);

void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr);

//
// Holds info about the pointer/cursor
//
struct PointerInfo
{
	std::vector<BYTE> PtrShapeBuffer;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo = {};
    POINT Position = {};
	bool Visible = false;
	UINT WhoUpdatedPositionLast = 0;
    LARGE_INTEGER LastTimeStamp = {};
};

//
// Structure that holds D3D resources not directly tied to any one thread
//
struct DxResources
{
	winrt::com_ptr<ID3D11Device> Device;
	winrt::com_ptr<ID3D11DeviceContext> Context;
	winrt::com_ptr<ID3D11VertexShader> VertexShader;
	winrt::com_ptr<ID3D11PixelShader> PixelShader;
	winrt::com_ptr<ID3D11InputLayout> InputLayout;
	winrt::com_ptr<ID3D11SamplerState> SamplerLinear;
};

//
// Structure to pass to a new thread
//
struct DuplicationThreadData
{
	// Used to indicate abnormal error condition
	HANDLE UnexpectedErrorEvent;

	// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
	HANDLE ExpectedErrorEvent;

	// Used by WinProc to signal to threads to exit
	HANDLE TerminateThreadsEvent;

	HANDLE TexSharedHandle;
	UINT Output;
	INT OffsetX;
	INT OffsetY;
	PointerInfo* PtrInfo;
	DxResources DxRes;
};

//
// FrameData holds information about an acquired frame
//
struct FrameData
{
	ID3D11Texture2D* Frame;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	_Field_size_bytes_((MoveCount * sizeof(DXGI_OUTDUPL_MOVE_RECT)) + (DirtyCount * sizeof(RECT))) BYTE* MetaData;
	UINT DirtyCount;
	UINT MoveCount;
};

//
// A vertex with a position and texture coordinate
//
struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexCoord;
};

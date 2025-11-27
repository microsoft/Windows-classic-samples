// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "CommonTypes.h"

//
// Handles the task of duplicating an output.
//
class DuplicationManager
{
public:
	DuplicationManager();
	~DuplicationManager();
	_Success_(*Timeout == false && return == DUPL_RETURN_SUCCESS) DUPL_RETURN GetFrame(_Out_ FrameData* Data, _Out_ bool* Timeout);
	DUPL_RETURN DoneWithFrame();
	DUPL_RETURN InitDupl(_In_ ID3D11Device* Device, UINT Output);
	DUPL_RETURN GetMouse(_Inout_ PointerInfo* PtrInfo, _In_ DXGI_OUTDUPL_FRAME_INFO* FrameInfo, INT OffsetX, INT OffsetY);
	void GetOutputDesc(_Out_ DXGI_OUTPUT_DESC1* DescPtr);

private:

	// vars
	winrt::com_ptr<IDXGIOutputDuplication> m_DeskDupl;
	winrt::com_ptr<ID3D11Texture2D> m_AcquiredDesktopImage;
	std::vector<BYTE> m_MetaDataBuffer;
	UINT m_OutputNumber = 0;
    DXGI_OUTPUT_DESC1 m_OutputDesc = {};
	winrt::com_ptr<ID3D11Device> m_Device;
};

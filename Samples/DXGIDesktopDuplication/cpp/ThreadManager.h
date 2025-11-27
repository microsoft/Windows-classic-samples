// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "CommonTypes.h"

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();
	void Clean();
	DUPL_RETURN Initialize(INT SingleOutput, UINT OutputCount, HANDLE UnexpectedErrorEvent, HANDLE ExpectedErrorEvent, HANDLE TerminateThreadsEvent, HANDLE SharedHandle, _In_ RECT* DesktopDim);
	PointerInfo* GetPointerInfo();
	void WaitForThreadTermination();

private:
	DUPL_RETURN InitializeDx(_Inout_ DxResources* Data);

	PointerInfo m_PtrInfo;
	UINT m_ThreadCount = 0;
	std::vector<winrt::handle> m_ThreadHandles;
	std::vector<DuplicationThreadData> m_ThreadData;
};

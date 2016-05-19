// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// RibbonFrameword.h/cpp implements the initialization and tear down of ribbon framework.
//

#pragma once
#include <uiribbon.h>

extern IUIFramework* g_pFramework; // Reference to the ribbon framework.

// Methods to setup and tear down the framework.
bool InitializeFramework(HWND hWindowFrame);
void DestroyFramework();

UINT GetRibbonHeight();
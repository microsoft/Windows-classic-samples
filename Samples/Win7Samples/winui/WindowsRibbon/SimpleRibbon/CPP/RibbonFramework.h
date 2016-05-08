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

#include <UIRibbon.h>
extern IUIFramework *g_pFramework;  // Reference to the Ribbon framework.
extern IUIApplication *g_pApplication;  // Reference to the Application object.

// Methods to facilitate the initialization and destruction of the Ribbon framework.
bool InitializeFramework(HWND hWnd);
void DestroyFramework();

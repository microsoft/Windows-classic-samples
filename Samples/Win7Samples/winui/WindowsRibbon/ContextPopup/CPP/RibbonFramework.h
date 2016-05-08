// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// RibbonFramework.h/cpp implements the initialization and tear down of ribbon framework.
//

#pragma once

#include "Application.h"

extern IUIFramework *g_pFramework;  // Reference to the Ribbon framework.
extern CApplication* g_pApplication;  // Reference to the Application object.

// Methods to facilitate the initialization and destruction of the Ribbon framework.
__checkReturn bool InitializeFramework(HWND hWnd);
void DestroyFramework();

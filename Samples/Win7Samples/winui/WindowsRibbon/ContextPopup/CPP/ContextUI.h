// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <windows.h>

// PURPOSE: Show contextual UI in the location indicated.
// RETURNS: S_OK when actual UI is shown (including when both parts are turned off), E_FAIL otherwise.
HRESULT ShowContextualUI(POINT& ptLocation, HWND hWnd);
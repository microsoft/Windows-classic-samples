//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#define NOMINMAX
//
// pch.h
// Header for standard system include files.
//

#pragma once

// Windows Header Files:
#include <windows.h>
#include <wrl.h>
#include <wrl/client.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <sstream>
#include "resource.h"

#if WINAPI_FAMILY_DESKTOP_APP == WINAPI_FAMILY
#include <atlbase.h>
#include <atlwin.h>
#endif

#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "Dwrite.lib")

#define ASSERT(expression) _ASSERTE(expression)

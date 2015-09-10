// Copyright (c) Microsoft Corporation. All rights reserved 

#pragma once
#include <windows.h>
#include <initguid.h>
#include <d3d11.h>
#include <dcomp.h>
#include <directmanipulation.h>
#include <wrl.h>
#include <dwrite.h>
#include <strsafe.h>

#define SQUARES_PER_SIDE (8)

#define UWM_REDRAWSTATUS (WM_USER + 1)

namespace DManipSample
{
    class CAppWindow;
    class CViewportEventHandler;
}

using namespace Microsoft::WRL;


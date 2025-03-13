#pragma once

#include <windows.h>
#include <wil\com.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include <d3d11_4.h>
#include <d3dcompiler.h>

#include <dcomp.h>
#include "dcomp-preview.h"

#define EXIT_IF_FAILED(a) {HRESULT hr = (a); if (FAILED(hr)) { __debugbreak(); ExitProcess(0); }}

#include "App.h"

const UINT TEXTURE_WIDTH = 1200;
const UINT TEXTURE_HEIGHT = 900;

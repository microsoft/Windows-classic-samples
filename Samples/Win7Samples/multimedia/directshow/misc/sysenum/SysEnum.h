#pragma once

#define _WIN32_WINNT 0x0501

#include <tchar.h>
#include <dshow.h>
#include <strsafe.h>

#include "wincontrol.h"
#include "utils.h"
#include "resource.h"


#define SAFE_RELEASE(x) { if (x) { x->Release(); x = NULL; } }


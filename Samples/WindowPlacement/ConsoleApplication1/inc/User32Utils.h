#pragma once

#ifdef USE_WINDOW_ACTION_APIS
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include "windows.h"
#include "shellscalingapi.h"
#include "dwmapi.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <wil/stl.h>
#include <stdexcept>
#include <strsafe.h>
#include <optional>
#include <algorithm>

#include "MonitorData.h"
#include "MiscUser32.h"
#include "RegistryHelpers.h"
#include "CurrentMonitorTopology.h"

#ifdef USE_VIRTUAL_DESKTOP_APIS
#include "shobjidl.h"
#include "shobjidl_core.h"
#include <wrl.h>
#include "VirtualDesktopIds.h"
#endif

#ifdef USE_WINDOW_ACTION_APIS
#include <minappmodelp.h>
bool IsApplyWindowActionSupported();
bool ApplyWindowActionWrapper(HWND hwnd, WINDOW_ACTION* action);
#endif

#include "PlacementEx.h"

#ifdef USE_WINDOW_ACTION_APIS
#include "WindowActions.h"
#endif

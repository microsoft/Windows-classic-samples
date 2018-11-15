#pragma once

// prevent redefinition of NTSTATUS messages
#define UMDF_USING_NTSTATUS

#include <windows.h>
#include <objbase.h>    // For CoCreateGuid

#include <ntstatus.h>   // For STATUS_CANNOT_DELETE

// STL
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>

// Windows SDK
#include <projectedfslib.h>

// regfs headers
#include "dirInfo.h"
#include "virtualizationInstance.h"
#include "pathUtils.h"
#include "RegOps.h"
#include "regfsProvider.h"

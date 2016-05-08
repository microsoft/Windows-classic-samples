
#pragma once

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

// ATL
#include <atlbase.h>
#include <atlcom.h>
using namespace ATL;

#include <strsafe.h>

#include <ndhelper.h>
#include <wlanihv.h>

#include "Resource.h"
#include "WlExtHC.h"
#include "ExtLog.h"


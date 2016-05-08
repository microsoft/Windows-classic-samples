//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// stdafx.h : include file for standard system include files,

#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_LONGHORN
#endif

#include <atlbase.h>
extern CComModule _Module;
#include <atlwin.h>
#include <atlcom.h>
#include <strsafe.h>
#include <commctrl.h>
#include <atlhost.h>
#include "wmp.h"
#include "resource.h"
/*--

Copyright (C) Microsoft Corporation, 2006

--*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif


#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>

#include <ntddcdrm.h>

#pragma warning(push)
    #pragma warning(disable:4200)
    #include <ntddcdvd.h>
#pragma warning(pop)

#include <ntddmmc.h>
#pragma warning(push)
    #pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
    #define _NTSCSI_USER_MODE_ 1 // CDBs and such only
    #include <scsi.h>            // CDBs, opcodes, SRB_STATUS, etc.
#pragma warning(pop)

#include <assert.h>

#include <imapi2.h>
#include <imapi2error.h>

#include "eraseSample.h"
#include "utility.h"

// 
typedef ATL::CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, ATL::_Copy<VARIANT> > CComEnumVariant;

// Most of the events take two IDispatch* (object, args) as arguments
static ::ATL::_ATL_FUNC_INFO g_GenericDualIDispatchEventInfo = { CC_STDCALL, VT_EMPTY, 2, { VT_DISPATCH, VT_DISPATCH } };

using namespace ATL;

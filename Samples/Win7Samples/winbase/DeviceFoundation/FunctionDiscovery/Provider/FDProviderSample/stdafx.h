// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef WINVER				
#define WINVER 0x0600		// Allow use of features specific to Windows Vista or later.
#endif

#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0600	// Allow use of features specific to Windows Vista or later.
#endif						

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <new>
#include <windows.h>
#include <assert.h>
#include <specstrings.h>
#include <strsafe.h>
#include <functiondiscovery.h>
#include <functiondiscoverykeys.h>
#include <functiondiscoveryprovider.h>
#include "functiondiscoveryproviderHelper.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>
#include <Iphlpapi.h>
#include "..\\inc\\Messages.h"
#include "..\\inc\\IEXEHostControl.h"
#include "Locks.h"
#include "LinkedList.h"
#include "ComServer.h"
#include "Threadpool.h"
#include "FunctionInstanceInfo.h"
#include "ClientNotificationWork.h"
#include "FDProvider.h"
#include "DiscoveryProtocol.h"

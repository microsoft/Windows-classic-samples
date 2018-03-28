// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <TraceLoggingProvider.h>
#include <amsi.h>
#include <wrl/module.h>

TRACELOGGING_DECLARE_PROVIDER(g_traceLoggingProvider);

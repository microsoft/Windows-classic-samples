//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  main.h
//
//  Purpose:
//  Extern declaration of global variables that are used in more than
//  one module.
//
//----------------------------------------------------------------------------


#pragma once

#include "util.h"
#include "cdialog.h"
#include "cmonitor.h"

// -------------------------------------------------------------------------------------
// Global variables
// -------------------------------------------------------------------------------------

extern CSmartComPtr<IBackgroundCopyManager> g_JobManager;
extern CMonitor                             g_NotificationReceiver;
extern CSimpleDialog                       *g_pDialog;
extern HANDLE                               g_hSafeToExitEvent;


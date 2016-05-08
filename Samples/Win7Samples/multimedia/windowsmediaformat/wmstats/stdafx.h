//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            stdafx.h
//
// Abstract:            Include file for standard system include files,
//                      or project specific include files that are used
//                      frequently, but are changed infrequently
//
//*****************************************************************************

#ifndef  __AFX_STDAFX_STATS_
#define  __AFX_STDAFX_STATS_

#include "wmsdk.h"
#include <TCHAR.H>
#include <stdio.h>
#include <assert.h>
#include <conio.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if ( NULL != x )       \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }

#endif

#endif // __AFX_STDAFX_STATS_

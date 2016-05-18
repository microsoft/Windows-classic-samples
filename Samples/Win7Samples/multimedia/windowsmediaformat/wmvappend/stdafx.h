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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <TCHAR.h>
#include <stdio.h>
#include <string.h>
#include <wmsdk.h>
#include <vfw.h>
#include <strsafe.h>

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if ( NULL != x )       \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }

#endif

#ifndef SAFE_ARRAYDELETE

#define SAFE_ARRAYDELETE( x )   \
    if ( NULL != x )            \
    {                           \
        delete[] x;             \
        x = NULL;               \
    }

#endif

#ifndef SAFE_CLOSEHANDLE

#define SAFE_CLOSEHANDLE( x )   \
    if ( NULL != x )            \
    {                           \
        CloseHandle( x );       \
        x = NULL;               \
    }

#endif


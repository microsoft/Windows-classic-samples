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

#if !defined(AFX_STDAFX_H__BC124D7E_0953_435F_BECE_B00BEEE70671__INCLUDED_)
#define AFX_STDAFX_H__BC124D7E_0953_435F_BECE_B00BEEE70671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__BC124D7E_0953_435F_BECE_B00BEEE70671__INCLUDED_)

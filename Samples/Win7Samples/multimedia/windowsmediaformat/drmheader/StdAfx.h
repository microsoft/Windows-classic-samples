//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            stdafx.h
//
// Abstract:            common includes and defines for DRMHeader sample
//
//*****************************************************************************

#if !defined(AFX_STDAFX_H__9D6AC4E4_056E_4CA5_BF95_BF72CB6C1CEE__INCLUDED_)
#define AFX_STDAFX_H__9D6AC4E4_056E_4CA5_BF95_BF72CB6C1CEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wmsdk.h"
#include <TCHAR.H>
#include <stdio.h>


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


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9D6AC4E4_056E_4CA5_BF95_BF72CB6C1CEE__INCLUDED_)

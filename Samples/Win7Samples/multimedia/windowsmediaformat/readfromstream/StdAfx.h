//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            stdafx.h
//
// Abstract:            include file for standard system include files,
//                      or project specific include files that are used frequently,
//                      but are changed infrequently.
//
//*****************************************************************************

#if !defined(AFX_STDAFX_H__EDCEEACA_7E34_47A4_8970_2BE282DA6608__INCLUDED_)
#define AFX_STDAFX_H__EDCEEACA_7E34_47A4_8970_2BE282DA6608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wmsdk.h"
#include <TCHAR.H>
#include <stdio.h>
#include <assert.h>

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if( NULL != x )        \
    {                      \
        x->Release();      \
        x = NULL;          \
    }

#endif


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EDCEEACA_7E34_47A4_8970_2BE282DA6608__INCLUDED_)

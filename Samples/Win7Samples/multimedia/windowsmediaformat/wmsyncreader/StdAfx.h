//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            stdafx.h
//
// Abstract:            Include file for standard system include files and
//						project specific include files that are used frequently.
//
//*****************************************************************************


#if !defined(AFX_STDAFX_H__04FDDE3E_2CBB_425B_B7B7_54D268C8DD27__INCLUDED_)
#define AFX_STDAFX_H__04FDDE3E_2CBB_425B_B7B7_54D268C8DD27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wmsdk.h>
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


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__04FDDE3E_2CBB_425B_B7B7_54D268C8DD27__INCLUDED_)

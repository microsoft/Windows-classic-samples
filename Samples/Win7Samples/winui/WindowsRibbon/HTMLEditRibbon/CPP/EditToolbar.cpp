// EditToolbar.cpp : implementation file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "HTMLEdit.h"
#include "EditToolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditToolbar

CEditToolbar::CEditToolbar()
{
}

CEditToolbar::~CEditToolbar()
{
}


BEGIN_MESSAGE_MAP(CEditToolbar, CToolBar)
	//{{AFX_MSG_MAP(CEditToolbar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditToolbar message handlers

int CEditToolbar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
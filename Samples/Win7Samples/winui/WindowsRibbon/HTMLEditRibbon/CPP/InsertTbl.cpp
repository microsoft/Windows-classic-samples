// InsertTbl.cpp : implementation file
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
#include "InsertTbl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsertTableDlg dialog


CInsertTableDlg::CInsertTableDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertTableDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsertTableDlg)
	m_Caption = _T("");
	m_CellAttribs = _T("");
	m_NumCols = 1;
	m_NumRows = 1;
	m_TableAttribs = _T("border=1 cellPadding=1 cellSpacing=1 width=75%");
	//}}AFX_DATA_INIT
}


void CInsertTableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertTableDlg)
	DDX_Text(pDX, IDC_EDIT_CAPTION, m_Caption);
	DDX_Text(pDX, IDC_EDIT_CELLATTRIBS, m_CellAttribs);
	DDX_Text(pDX, IDC_EDIT_NUMCOLS, m_NumCols);
	DDV_MinMaxUInt(pDX, m_NumCols, 0, 50000);
	DDX_Text(pDX, IDC_EDIT_NUMROWS, m_NumRows);
	DDV_MinMaxUInt(pDX, m_NumRows, 1, 50000);
	DDX_Text(pDX, IDC_EDIT_TABLEATTRIBS, m_TableAttribs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsertTableDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertTableDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertTableDlg message handlers



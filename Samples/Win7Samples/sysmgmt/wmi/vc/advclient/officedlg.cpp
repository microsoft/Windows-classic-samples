// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OfficeDlg.cpp
//
// Description:
//	This file implements the OfficeDlg dialog class which 
//		collects information for the OnAddEquipment() routine.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClient.h"
#include "OfficeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COfficeDlg dialog


COfficeDlg::COfficeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COfficeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COfficeDlg)
	m_item = _T("");
	m_SKU = _T("");
	//}}AFX_DATA_INIT
}


void COfficeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COfficeDlg)
	DDX_Text(pDX, IDC_ITEM, m_item);
	DDX_Text(pDX, IDC_SKU, m_SKU);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COfficeDlg, CDialog)
	//{{AFX_MSG_MAP(COfficeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COfficeDlg message handlers

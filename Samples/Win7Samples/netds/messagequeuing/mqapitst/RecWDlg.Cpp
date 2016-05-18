// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------
//
// RecWDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "RecWDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReceiveWaitDialog dialog


CReceiveWaitDialog::CReceiveWaitDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CReceiveWaitDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReceiveWaitDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReceiveWaitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReceiveWaitDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReceiveWaitDialog, CDialog)
	//{{AFX_MSG_MAP(CReceiveWaitDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReceiveWaitDialog message handlers


void CReceiveWaitDialog::CenterWindow()
{
    CWnd * pParent = GetParent();
    RECT parentRect;
    RECT childRect;

    pParent->GetWindowRect(&parentRect);
    int dxP = parentRect.right-parentRect.left;
    int dyP = parentRect.bottom - parentRect.top;

    GetWindowRect(&childRect);
    int dxC = childRect.right-childRect.left;
    int dyC = childRect.bottom - childRect.top;

    childRect.left = dxP/2-dxC/2;
    childRect.right = childRect.left+dxC;

    childRect.top = dyP/2-dyC/2;
    childRect.bottom = childRect.top+dyC;

    MoveWindow(&childRect);
};

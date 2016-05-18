//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module PROGRESS.H
//
//-----------------------------------------------------------------------------

#ifndef _PROGRESS_H_
#define _PROGRESS_H_

//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#include "wizard.h"



//////////////////////////////////////////////////////////////////////
// CProgress
//
//////////////////////////////////////////////////////////////////////
class CProgress : public CDialogBase
{
public:

	//Constructors
	CProgress(HWND hWnd, HINSTANCE hInst);
	virtual ~CProgress();

	//members
	static  BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();
	virtual ULONG	Destroy();

	virtual BOOL SetHeading(WCHAR* pwszText);
	virtual BOOL SetText(WCHAR* pwszText);

	virtual BOOL Update(WCHAR* pwszText);
	virtual BOOL Cancel();

	//data
	BOOL		m_fCancel;
};

#endif //_PROGRESS_H_

//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module PROGRESS.CPP
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////
#include "progress.h"
#include "common.h"



////////////////////////////////////////////////////////////////////
// CProgress::CProgress
//
////////////////////////////////////////////////////////////////////
CProgress::CProgress(HWND hWnd, HINSTANCE hInst)
	: CDialogBase(hWnd, hInst)
{
}

////////////////////////////////////////////////////////////////////
// CProgress::~CProgress
//
////////////////////////////////////////////////////////////////////
CProgress::~CProgress()
{
	Destroy();
}


////////////////////////////////////////////////////////////////////
// BOOL WINAPI CProgress::DlgProc
//
////////////////////////////////////////////////////////////////////
BOOL WINAPI CProgress::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			//Save the this pointer, since this is a static method
			Busy();
			CProgress* pThis = (CProgress*)lParam;
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			//On INIT we know we have a valid hWnd to store
			pThis->m_hWnd = hWnd;

			CenterDialog(hWnd);
			return TRUE;
		}
		break;

		case WM_COMMAND:
		{
			// All buttons are handled the same way
			//Restore instance pointer, since this is a static function
			CProgress* pThis = (CProgress*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

			switch(GET_WM_COMMAND_ID(wParam, lParam)) 
			{
                case IDCANCEL:
					Busy();
					pThis->m_fCancel = TRUE;
                	return TRUE;
			}
			return FALSE;
		}
		break;

		default:
			return FALSE;
	}
}


////////////////////////////////////////////////////////////////////
// ULONG CProgress::Display
//
////////////////////////////////////////////////////////////////////
INT_PTR CProgress::Display()
{
	m_fCancel = FALSE;

	//Create a ModeLess Dialog Box
	m_hWnd = CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_PROGRESS), NULL, (DLGPROC)DlgProc, (LPARAM)this);
	return TRUE;
}


////////////////////////////////////////////////////////////////////
// ULONG CProgress::Destroy
//
////////////////////////////////////////////////////////////////////
ULONG CProgress::Destroy()
{
	if(m_hWnd) 
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////
// BOOL CProgress::SetHeading
//
////////////////////////////////////////////////////////////////////
BOOL CProgress::SetHeading(WCHAR* pwszText)
{
	ASSERT(pwszText);

	wSetDlgItemText(m_hWnd, IDT_STATUS, pwszText);
	return TRUE;
}


////////////////////////////////////////////////////////////////////
// BOOL CProgress::SetText
//
////////////////////////////////////////////////////////////////////
BOOL CProgress::SetText(WCHAR* pwszText)
{
	ASSERT(pwszText);
	
	wSetDlgItemText(m_hWnd, IDT_STAT_INFO, pwszText);
	return TRUE;
}


////////////////////////////////////////////////////////////////////
// BOOL CProgress::Cancel
//
////////////////////////////////////////////////////////////////////
BOOL CProgress::Cancel()
{
	MSG		msg;

	//Must have an existing window to cancel
	if(m_hWnd==NULL)
		return FALSE;

	while(PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) 
	{
		if(!IsDialogMessage(m_hWnd, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return m_fCancel;
}


////////////////////////////////////////////////////////////////////
// BOOL CProgress::Update
//
////////////////////////////////////////////////////////////////////
BOOL CProgress::Update(WCHAR* pwszText)
{
	ASSERT(pwszText);

	//Update the progress text
	SetText(pwszText);

	// Now check for a cancel from the user, by checking
	// the messages from the user
	if(Cancel()) 
	{
		if(IDYES == wMessageBox(m_hWnd, MB_TASKMODAL | MB_ICONEXCLAMATION | MB_YESNO, wsz_CANCEL, wsz_CANCEL_OP))
		{
			//Indicate the users wishes to stop 
			return FALSE;
		}
		else 
		{
			m_fCancel = FALSE;
		}	
	}

	//Indicate the user wishes to continue
	return TRUE;
}
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SELECTEVENTSDLG_H_
#define __SELECTEVENTSDLG_H_

#include "resource.h"       // main symbols

class CComSpy;


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CSelectEventsDlg
Summary: Dialog for selecting which events to subscribe to.  
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class CSelectEventsDlg : public CDialogImpl<CSelectEventsDlg>
{
private:
	MapStringToAppInfo * m_map;
	CAppInfo * m_pSysAppInfo;  
	CComSpy * m_pSpyObj;

	// Cannot call default constructor because it's Private.
	CSelectEventsDlg();	

public:
	CSelectEventsDlg(MapStringToAppInfo* spMap, CAppInfo * pSysAppInfo, CComSpy* pSpyObj);  

	void PopulateEventList();
	void PopulateSysEventList(); 
	bool GetRunningApps();
	bool GetCatalogApps();
	bool SelectApplication(int nItem);
	void MarkAllAppsForDelete(bool bDeleteUnused);
	void ShowSelectedEvents(CAppInfo* pInfo);
	void ShowSelectedSystemEvents();  
	bool SelectAllApps(bool bSelectAll);

	enum { IDD = IDD_CONFIGUREEVENTSDLG };


	BEGIN_MSG_MAP(CSelectEventsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_LIST_APPS, AppsListHandler)
		COMMAND_ID_HANDLER(IDC_CHECK_ALLAPPS, OnSelectAllApps)
		NOTIFY_HANDLER(IDC_LISTVIEWEVENTS, LVN_ITEMCHANGED, LVEventItemChanged)
		NOTIFY_HANDLER(IDC_LISTVIEWSYSEVENTS, LVN_ITEMCHANGED, LVSystemEventItemChanged) 
		MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
	END_MSG_MAP()

	//	Message Handles
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT AppsListHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectAllApps(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT LVEventItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT LVSystemEventItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT OnDestroyDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif //__SELECTEVENTSDLG_H_

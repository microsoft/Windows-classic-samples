//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module COBJECTTREE.H
//
//-----------------------------------------------------------------------------------

#ifndef _COBJECTTREE_H_
#define _COBJECTTREE_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////

						
/////////////////////////////////////////////////////////////////////
// CObjTree
//
/////////////////////////////////////////////////////////////////////
class CObjTree : public CTreeViewLite
{
public:
	//constructors
	CObjTree(CMainWindow* pCMainWindow);
	virtual ~CObjTree();

	//messages
	virtual BOOL	OnRButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnContextMenu(HWND hWnd, REFPOINTS pts);
	virtual BOOL	OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData);
	virtual BOOL	OnDblclk(WPARAM fwKeys, REFPOINTS pts);

	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	//Helpers
	virtual BOOL	AddObject(CBase* pCSource, CBase* pCBase);
	virtual BOOL	SelectObject(CBase* pCBase);
	virtual CBase*	GetSelectedObject();

	virtual BOOL	RemoveObject(CBase* pCBase);
	virtual BOOL	RemoveNode(HTREEITEM hTreeItem);
	virtual BOOL	GarbageCollectNode(HTREEITEM hTreeItem);

	//Data
	CMainWindow*	m_pCMainWindow;
};



/////////////////////////////////////////////////////////////////////
// CMDIObjects
//
/////////////////////////////////////////////////////////////////////
class CMDIObjects : public CMDIChildLite
{
public:
	//constructors
	CMDIObjects(CMainWindow* pCMainWindow);
	virtual ~CMDIObjects();

	virtual BOOL	PreCreateWindow(CREATESTRUCTW& cs);
	virtual BOOL	OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL	AutoPosition(BOOL fDefaultPosition = TRUE);
	
	virtual BOOL	OnDestroy();
	virtual BOOL	OnClose();
	virtual BOOL	OnInitialUpdate();
	virtual BOOL	OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate);

	//members
	virtual BOOL	UpdateControls();
	
	//Messages
	virtual BOOL	OnSize(WPARAM nType, REFPOINTS pts);
	virtual BOOL	OnSetFocus(HWND hWndPrevFocus);

	//Overloads
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	//Data
	CMainWindow*	m_pCMainWindow;
	CObjTree*		m_pCObjTree;
};


#endif //_OBJECTTREE_H_
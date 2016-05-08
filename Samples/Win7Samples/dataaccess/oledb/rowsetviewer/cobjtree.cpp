//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module COBJTREE.CPP
//
//-----------------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "CObjTree.h"


////////////////////////////////////////////////////////////////
// CObjTree::CObjTree
//
/////////////////////////////////////////////////////////////////
CObjTree::CObjTree(CMainWindow* pCMainWindow)
{
	//Data
	ASSERT(pCMainWindow);
	m_pCMainWindow = pCMainWindow;
}


////////////////////////////////////////////////////////////////
// CObjTree::~CObjTree
//
/////////////////////////////////////////////////////////////////
CObjTree::~CObjTree()
{
}


////////////////////////////////////////////////////////////////
// CObjTree::OnRButtonDown
//
/////////////////////////////////////////////////////////////////
BOOL CObjTree::OnRButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	//NOTE: The right mouse button doesn't automatically activate the MDI window...
	m_pCMainWindow->MDIActivate(m_hWndParent);

	//NOTE: The right mouse doesn't automatically select the tree item
	//so we have to find where the mouse clicked and determine the inteneded item...
	HTREEITEM hItem = HitTest(pts, NULL);
	if(hItem)
		SelectItem(hItem);

	//Display the Context Menu (for this object)
	CBase* pCBase = (CBase*)GetItemParam(hItem);
	if(pCBase)
	{
		UINT nIDMenu = pCBase->GetObjectMenu();
		if(nIDMenu)
		{
			//xPos, yPos are Relative to the Client Area...
			DisplayContextMenu( 
								m_hWnd,
								nIDMenu, 
								pts,
								m_pCMainWindow->GetWnd(),
								TRUE
								);
				
		}
	}
	
	return TRUE;
}

////////////////////////////////////////////////////////////////
// CObjTree::OnContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL CObjTree::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	CBase* pCBase = GetSelectedObject();
	if(pCBase)
	{
		UINT nIDMenu = pCBase->GetObjectMenu();

		if(nIDMenu)
		{
			DisplayContextMenu(
							hWnd,
							nIDMenu, 
							pts,
							m_pCMainWindow->GetWnd()
							);
		}
	}
	
	return  TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	//Is this command meant for the Child Window?
	CBase* pCBase = GetSelectedObject();
	if(pCBase && pCBase->m_pCMDIChild)
		if(pCBase->m_pCMDIChild->OnUpdateCommand(hMenu, nID, pdwFlags))
			return TRUE;

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CObjTree::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CObjTree::OnCommand(UINT iID, HWND hWndCtrl)
{
	//Is this command meant for the Child Window?
	CBase* pCBase = GetSelectedObject();
	if(pCBase && pCBase->m_pCMDIChild)
		if(pCBase->m_pCMDIChild->OnCommand(iID, hWndCtrl))
			return TRUE;

	return FALSE;
}
		

/////////////////////////////////////////////////////////////////////////////
// CObjTree::OnDblclk
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::OnDblclk(WPARAM fwKeys, REFPOINTS pts) 
{
	UINT uFlags = 0;
	HTREEITEM hItem = HitTest(pts, &uFlags);
	if(hItem)
	{
		CBase* pCBase = (CBase*)GetItemParam(hItem);
		if(pCBase)
		{
			//If there is a child assoicated with this object, then let it perform what it should
			//do on a double click operation (which by default activates the  child window)
			pCBase->OnDefOperation();
			return TRUE;
		}
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::OnKeyDown
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData) 
{
	switch(nVirtKey)
	{
		case VK_RETURN:
		{
			CBase* pCBase = GetSelectedObject();
			if(pCBase)
			{
				//NOTE: Not all objects in the tree will have an assiocate child Window
				if(pCBase->m_pCMDIChild)
				{
					//Activate the MDIChild window assoicated with this object
					m_pCMainWindow->MDIActivate(pCBase->m_pCMDIChild->m_hWnd);
				}
				return TRUE;
			}
			break;
		}
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::AddObject
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::AddObject(CBase* pCSource, CBase* pCBase)
{
	if(pCBase)
	{
		WCHAR* pwszDesc = pCBase->GetObjectDesc();
		WCHAR* pwszName = pCBase->GetObjectName();
		
		//Obtain the icon/image for the object
		INT iImage = pCBase->GetObjectImage();
		if(pCBase->m_pIAggregate)
			iImage = IMAGE_INTERFACE;

		if(pwszDesc)
		{
			static WCHAR wszBuffer[MAX_NAME_LEN] = {0};
			if(pCBase->m_dwCLSCTX == CLSCTX_INPROC_SERVER)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%s (%s)", pwszName, pwszDesc);
			else
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%s%s (%s)", L"Remote ", pwszName, pwszDesc);
			pwszName = wszBuffer;
		}
		
		//Insert the Item (if not already)
		if(pCBase->m_hTreeItem == NULL)
		{
			pCBase->m_hTreeItem = InsertItem(pCSource ? pCSource->m_hTreeItem : NULL, NULL, pwszName, (INT_PTR)pCBase, iImage, iImage);
		}
		else
		{
			//Otherwise it probably just needs to have the item updated...
			SetItemText(pCBase->m_hTreeItem, pwszName);
			SetItemParam(pCBase->m_hTreeItem, (INT_PTR)pCBase);
			SetItemImage(pCBase->m_hTreeItem, iImage, iImage);
		}

		SelectObject(pCBase);
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::RemoveObject
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::RemoveObject(CBase* pCBase)
{
	//No-op
	if(!m_hWnd || !pCBase || !pCBase->m_hTreeItem)
		return TRUE;
	
	//Make a copy, since RemoveNode will null the objects members...
	HTREEITEM hParentItem = GetParentItem(pCBase->m_hTreeItem);
	
	//First try and remove this node (and possibly any children)
	if(RemoveNode(pCBase->m_hTreeItem))
	{
		pCBase->m_hTreeItem = NULL;

		//If we were able to remove this node, then maybe we should 
		//try and "garbage-collect" any parent nodes now that this is gone...
		if(hParentItem)
			GarbageCollectNode(hParentItem);
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::GarbageCollectNode
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::GarbageCollectNode(HTREEITEM hTreeItem)
{
	//No-op
	if(!m_hWnd || !hTreeItem)
		return TRUE;

	HTREEITEM hHighestItem = NULL;

	//Try to find the "highest" un-used parent node.
	while(hTreeItem)
	{
		//As soon as one of the nodes has a (non-empty) object, its no longer "un-used"
		CBase* pCBase = (CBase*)GetItemParam(hTreeItem);
		if(pCBase && pCBase->m_pIUnknown)
			break;

		hHighestItem = hTreeItem;
		hTreeItem = GetParentItem(hTreeItem);
	}

	//Have we found "un-used" parents, that can be garbage-collected
	if(hHighestItem)
	{
		//Garbage Collect the usued nodes...
		return RemoveNode(hHighestItem);
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::RemoveNode
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::RemoveNode(HTREEITEM hTreeItem)
{
	//No-op
	if(!m_hWnd || !hTreeItem)
		return TRUE;

	BOOL bSafeToRemove = TRUE;
	CBase* pCBase = (CBase*)GetItemParam(hTreeItem);
	CMDIChild* pCMDIChild = pCBase ? pCBase->m_pCMDIChild : NULL;

	//First, make sure this node is safe to remove...
	if(pCBase && pCBase->m_pIUnknown)
		bSafeToRemove = FALSE;
	
	//If it is before deleting it from the tree, we need to make there are no child nodes 
	//that depend upon it
	if(bSafeToRemove)
	{
		HTREEITEM hChildItem = GetChildItem(hTreeItem);
		while(hChildItem)
		{
			//NOTE: Before deleting this node of the tree, obtain the next sibling (if there is one...)
			//Since we can't do this after the node has been deleted!
			HTREEITEM hNextSibling = GetNextItem(hChildItem);
			
			//Recurse
			if(!RemoveNode(hChildItem))
			{
				bSafeToRemove = FALSE;
				break;
			}

			//Go to the next sibling...
			hChildItem = hNextSibling;
		}
	}

	if(bSafeToRemove)
	{
		//Completely safe to delete this node (and all children)
		DeleteItem(hTreeItem);
		if(pCBase)
			pCBase->m_hTreeItem = NULL;
	}

	return bSafeToRemove;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::SelectObject
//
/////////////////////////////////////////////////////////////////////////////
BOOL CObjTree::SelectObject(CBase* pCBase)
{
	if(pCBase && pCBase->m_hTreeItem)
		SelectItem(pCBase->m_hTreeItem);
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CObjTree::GetSelectedObject
//
/////////////////////////////////////////////////////////////////////////////
CBase* CObjTree::GetSelectedObject()
{
	//Obtain the selected item
	return (CBase*)GetItemParam(GetSelectedItem());
}


////////////////////////////////////////////////////////////////
// CMDIObjects::CMDIObjects
//
/////////////////////////////////////////////////////////////////
CMDIObjects::CMDIObjects(CMainWindow* pCMainWindow)
{
	//Objects
	ASSERT(pCMainWindow);
	m_pCMainWindow = pCMainWindow;
	
	//Controls
	m_pCObjTree		= new CObjTree(pCMainWindow);
}


////////////////////////////////////////////////////////////////
// CMDIObjects::~CMDIObjects
//
/////////////////////////////////////////////////////////////////
CMDIObjects::~CMDIObjects()
{
	//Controls
	SAFE_DELETE(m_pCObjTree);
}


	
////////////////////////////////////////////////////////////////
// CMDIObjects::PreCreateWindow
//
/////////////////////////////////////////////////////////////////
BOOL CMDIObjects::PreCreateWindow(CREATESTRUCTW& cs)
{
	//Load Saved Window Positions
	memset(&m_wndPlacement,	0, sizeof(m_wndPlacement));
	GetRegEntry(HKEY_ROWSETVIEWER, wszOBJECTS_KEY, L"WinPosition",	&m_wndPlacement, sizeof(m_wndPlacement), NULL);

	//Window Hidden?
	if(m_wndPlacement.length)
	{
		if(m_wndPlacement.showCmd == SW_HIDE)
			cs.style &= ~WS_VISIBLE;
	}
	
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIObjects::AutoPosition
//
/////////////////////////////////////////////////////////////////
BOOL CMDIObjects::AutoPosition(BOOL fDefaultPosition)
{
	if(fDefaultPosition || m_wndPlacement.length == 0)
	{
		//Default setting, left corner (1/4 of the client area)...
		SIZE sizeClient = GetClientSize(m_pCMainWindow->m_hWndMDIClient);
		return MoveWindow(m_hWnd, 0, 0, (INT)((float)sizeClient.cx * 0.25), sizeClient.cy, TRUE);
	}
	else
	{
		return SetWindowPlacement();
	}
}


////////////////////////////////////////////////////////////////
// CMDIObjects::OnCreate
//
/////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//Create Object Tree
	SIZE size = GetClientSize(m_hWnd);
	m_pCObjTree->Create(m_hWnd, WC_TREEVIEWW, NULL, ID_OBJECTTREE, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP | /*TVS_EDITLABELS |*/ TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE,
		0, 0, size.cx, size.cy);
	TreeView_SetImageList(m_pCObjTree->m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), TVSIL_NORMAL);
	m_pCObjTree->OnCreate(NULL);

	//Window Position
	AutoPosition(m_wndPlacement.length == 0/*fDefaultPosition*/);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMDIObjects::OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnDestroy()
{
	//Save Window Positions
	if(GetWindowPlacement())
		SetRegEntry(HKEY_ROWSETVIEWER, wszOBJECTS_KEY, L"WinPosition",	&m_wndPlacement,	sizeof(m_wndPlacement));

	//Delegate
	return CMDIChildLite::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CMDIObjects::OnClose
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnClose()
{
	//NOTE:  OnClose only gets hit for MDI Child Windows when the user actually
	//closes the window.  If the mainframe window is closed directly then OnClose is never fired...
	
	//Due to this, if the user has closed the window before exiting the app, then all we really 
	//want to do is make the window not-visible, so if they bring the window up again it has all
	//the info saved.

	ShowWindow(SW_HIDE);

	//NOTE:  Don't delegate the call, since we don't want to "DestroyWindow"
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIObjects::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnInitialUpdate()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CMDIObjects::OnMDIActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate)
{
	//So see if this window is being activated or deactivated
	if(hWndActivate == m_hWnd)
	{
		//Refresh all Controls
		UpdateControls();
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIObjects::UpdateControls
//
/////////////////////////////////////////////////////////////////
BOOL CMDIObjects::UpdateControls()
{
	//ToolBar Buttons
	m_pCMainWindow->UpdateControls();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMDIObjects::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	return m_pCObjTree->OnUpdateCommand(hMenu, nID, pdwFlags);
}


/////////////////////////////////////////////////////////////////////
// CMDIObjects::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	return m_pCObjTree->OnNotify(idCtrl, pNMHDR);
}



/////////////////////////////////////////////////////////////////////
// CMDIObjects::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnCommand(UINT iID, HWND hWndCtrl)
{
	return m_pCObjTree->OnCommand(iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////
// CMDIObjects::OnSize
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnSize(WPARAM nType, REFPOINTS pts)
{
	ASSERT(m_pCObjTree->m_hWnd);

	switch(nType)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			if(pts.x && pts.y)
			{
				//Obtain window sizes...
				MoveWindow(m_pCObjTree->m_hWnd, 0, 0, pts.x, pts.y, TRUE);
			
				//Call default procedure first, 
				//to let MDI position the child & then move its children
				//We simply do this by returning false, to indicate we didn't handle it...
				return FALSE;
			}
			break;
		}
	};

	return FALSE;
}                


/////////////////////////////////////////////////////////////////////////////
// CMDIObjects::OnSetFocus
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIObjects::OnSetFocus(HWND hWndPrevFocus)
{
	m_pCObjTree->SetFocus();
	return TRUE;
}


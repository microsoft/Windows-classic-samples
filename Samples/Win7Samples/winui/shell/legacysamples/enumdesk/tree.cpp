/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Tree.cpp

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <tchar.h>
#include "EnumDesk.h"
#include "List.h"
#include "resource.h"

/**************************************************************************
   function prototypes
**************************************************************************/

BOOL Tree_GetRootItems(HWND);
BOOL Tree_GetChildItems(HWND, HTREEITEM);
void Tree_EnumObjects(HWND, HTREEITEM, LPSHELLFOLDER, LPITEMIDLIST);
int CALLBACK Tree_CompareProc(LPARAM, LPARAM, LPARAM);

/**************************************************************************
   global variables and definitions
**************************************************************************/

/******************************************************************************

   Tree_Create

******************************************************************************/

HWND Tree_Create(HINSTANCE hInstance, HWND hwndParent, HIMAGELIST hImageList)
{
DWORD dwStyle;
HWND  hwndTreeView;

dwStyle =   WS_TABSTOP | 
            WS_CHILD | 
            WS_VISIBLE | 
            TVS_HASLINES | 
            TVS_HASBUTTONS | 
            TVS_NOTOOLTIPS |
            0;
            
hwndTreeView = CreateWindowEx(   WS_EX_CLIENTEDGE,
                                 WC_TREEVIEW,
                                 NULL,
                                 dwStyle,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 (HMENU)IDC_TREEVIEW,
                                 hInstance,
                                 NULL);

if(!hwndTreeView)
   return NULL;

// attach ImageList to TreeView
if (hImageList)
   TreeView_SetImageList(hwndTreeView, hImageList, 0);

return hwndTreeView;
}

/******************************************************************************

   Tree_Init

******************************************************************************/

void Tree_Init(HWND hwndTreeView)
{
//empty the list
TreeView_DeleteAllItems(hwndTreeView);

Tree_GetRootItems(hwndTreeView);

TreeView_SetScrollTime(hwndTreeView, 100);
}

/**************************************************************************

   Tree_Notify()

**************************************************************************/

LRESULT Tree_Notify(HWND hWnd, LPARAM lParam)
{
LPNMHDR  lpnmh = (LPNMHDR) lParam;
HWND     hwndTreeView = GetDlgItem(hWnd, IDC_TREEVIEW);

switch(lpnmh->code)
   {
   case TVN_GETDISPINFO:
      {
      LPNMTVDISPINFO lpdi = (LPNMTVDISPINFO)lParam;
      LPITEMINFO     pItem = (LPITEMINFO)lpdi->item.lParam;

      //do we need to supply the text?
      if(lpdi->item.mask & TVIF_TEXT)
         {
         SHFILEINFO     sfi;
      
         //get the display name of the item
         if(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
            {
            _tcsncpy_s(lpdi->item.pszText,
					   lpdi->item.cchTextMax,
					   sfi.szDisplayName,
					   _TRUNCATE);
            }
         }

      //do we need to supply the unselected image?
      if(lpdi->item.mask & TVIF_IMAGE)
         {
         SHFILEINFO     sfi;
         
         //get the unselected image for this item
         if(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
            {
            lpdi->item.iImage = sfi.iIcon;
            }
         }

      //do we need to supply the selected image?
      if(lpdi->item.mask & TVIF_SELECTEDIMAGE)
         {
         SHFILEINFO     sfi;
         
         //get the selected image for this item
         if(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
            {
            lpdi->item.iSelectedImage = sfi.iIcon;
            }
         }
      }
      break;

   case TVN_ITEMEXPANDING:
      {
      LPNMTREEVIEW   pnmtv = (LPNMTREEVIEW)lParam;
      
      switch(pnmtv->action)
         {
         case TVE_EXPAND:
            Tree_GetChildItems(hwndTreeView, pnmtv->itemNew.hItem);
            break;
         
         case TVE_COLLAPSE:
            //remove all of the items from this node
            TreeView_Expand(hwndTreeView, pnmtv->itemNew.hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
            break;
         }
      }
      break;

   case TVN_DELETEITEM:
      {
      LPNMTREEVIEW   pnmtv = (LPNMTREEVIEW)lParam;
      LPITEMINFO     pItem = (LPITEMINFO)pnmtv->itemOld.lParam;
      LPMALLOC       pMalloc;

      if(SUCCEEDED(SHGetMalloc(&pMalloc)))
         {
         //free up the pidls that we allocated
         Pidl_Free(pItem->pidlFQ);
         Pidl_Free(pItem->pidlRel);

         pMalloc->Release();
         }
      
      //this may be NULL if this is the root item
      if(pItem->pParentFolder)
         pItem->pParentFolder->Release();
      
      GlobalFree((HGLOBAL)pItem);
      }
      break;
   
   case TVN_SELCHANGED:
      {
      LPNMTREEVIEW   pnmtv = (LPNMTREEVIEW)lParam;
      LPITEMINFO pItem = (LPITEMINFO)pnmtv->itemNew.lParam;

      List_DisplayFolder(GetDlgItem(hWnd, IDC_LISTVIEW), pItem);
      }
      break;
   }

return 0;
}

/**************************************************************************

   Tree_GetRootItems

**************************************************************************/

BOOL Tree_GetRootItems(HWND hwndTreeView)
{
LPITEMIDLIST      pidl;

if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl)))
   {
   TV_ITEM           tvItem;
   TV_INSERTSTRUCT   tvInsert;
   LPSHELLFOLDER     pDesktop;
   HTREEITEM         hParentItem;
   LPITEMINFO        pItem;

   //get the desktop's IShellFolder
   if(FAILED(SHGetDesktopFolder(&pDesktop)))
      return FALSE;
   
   //fill in the TVITEM structure for this item
   tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

   //put the private information in the lParam
   pItem = (LPITEMINFO)GlobalAlloc(GPTR, sizeof(ITEMINFO));
   pItem->pidlRel = pidl;
   pItem->pidlFQ = Pidl_Copy(pidl);
   
   //the desktop doesn't have a parent folder, so make this NULL
   pItem->pParentFolder = NULL;
   tvItem.lParam = (LPARAM)pItem;

   //text and images are done on a callback basis
   tvItem.pszText = LPSTR_TEXTCALLBACK;
   tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
   
   //assume the desktop has children
   tvItem.cChildren = TRUE;

   //fill in the TV_INSERTSTRUCT structure for this item
   tvInsert.item = tvItem;
   tvInsert.hInsertAfter = TVI_LAST;
   tvInsert.hParent = TVI_ROOT;

   //add the item
   hParentItem = TreeView_InsertItem(hwndTreeView, &tvInsert);

   //go ahead and expand this item
   TreeView_Expand(hwndTreeView, hParentItem, TVE_EXPAND);

   pDesktop->Release();

   return TRUE;
   }

return FALSE;
}

/**************************************************************************

   Tree_GetChildItems

**************************************************************************/

BOOL Tree_GetChildItems(HWND hwndTreeView, HTREEITEM hParentItem)
{
TVITEM         tvItem;
LPITEMINFO     pItem;
LPSHELLFOLDER  pParentFolder = NULL;
HRESULT        hr;
HCURSOR        hCursor;
TV_SORTCB      tvSort;


//get the parent item's pidl
ZeroMemory(&tvItem, sizeof(tvItem));
tvItem.mask = TVIF_PARAM;
tvItem.hItem = hParentItem;
if(!TreeView_GetItem(hwndTreeView, &tvItem))
   return FALSE;

//change the cursor
hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

//turn redawing off in the TreeView. This will speed things up as we add items
SendMessage(hwndTreeView, WM_SETREDRAW, FALSE, 0);

pItem = (LPITEMINFO)tvItem.lParam;

//if the parent folder is NULL, then we are at the root of the namespace, so the parent of this item is the desktop folder
if(!pItem->pParentFolder)
   {
   hr = SHGetDesktopFolder(&pParentFolder);
   }
//otherwise we need to get the IShellFolder for this item
else
   {
   hr = pItem->pParentFolder->BindToObject(pItem->pidlRel, NULL, IID_IShellFolder, (LPVOID*)&pParentFolder);
   }

if(FAILED(hr))
   return FALSE;

Tree_EnumObjects(hwndTreeView, hParentItem, pParentFolder, pItem->pidlFQ);

//sort the new items
tvSort.hParent = hParentItem;
tvSort.lpfnCompare = Tree_CompareProc;
tvSort.lParam = 0;
TreeView_SortChildrenCB(hwndTreeView, &tvSort, 0);

//turn redawing back on in the TreeView
SendMessage(hwndTreeView, WM_SETREDRAW, TRUE, 0);
//UpdateWindow(hwndTreeView);

pParentFolder->Release();

SetCursor(hCursor);

return TRUE;
}

/**************************************************************************

   Tree_EnumObjects

**************************************************************************/

void Tree_EnumObjects(  HWND hwndTreeView, 
                        HTREEITEM hParentItem, 
                        LPSHELLFOLDER pParentFolder, 
                        LPITEMIDLIST pidlParent)
{
LPENUMIDLIST   pEnum;
   
if(SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
   {
   LPITEMIDLIST   pidlTemp;
   DWORD          dwFetched = 1;
   LPITEMINFO     pItem;

   //enumerate the item's PIDLs
   while(S_OK == (pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
      {
      TVITEM         tvItem;
      TVINSERTSTRUCT tvInsert;
      DWORD          dwAttribs;

      ZeroMemory(&tvItem, sizeof(tvItem));
      
      //fill in the TV_ITEM structure for this item
      tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

      //AddRef the parent folder so it's pointer stays valid
      pParentFolder->AddRef();

      //put the private information in the lParam
      pItem = (LPITEMINFO)GlobalAlloc(GPTR, sizeof(ITEMINFO));
      
      pItem->pidlRel = pidlTemp;
      pItem->pidlFQ = Pidl_Concatenate(pidlParent, pidlTemp);
      
      pItem->pParentFolder = pParentFolder;
      tvItem.lParam = (LPARAM)pItem;

      //text and images are done on a callback basis
      tvItem.pszText = LPSTR_TEXTCALLBACK;
      tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;

      //determine if the item has children
      dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME;
      pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttribs);
      tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);

      //determine if the item is shared
      if(dwAttribs & SFGAO_SHARE)
         {
         tvItem.mask |= TVIF_STATE;
         tvItem.stateMask |= TVIS_OVERLAYMASK;
         tvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
         }
      
      //fill in the TV_INSERTSTRUCT structure for this item
      tvInsert.item = tvItem;
      tvInsert.hInsertAfter = TVI_LAST;
      tvInsert.hParent = hParentItem;

      TreeView_InsertItem(hwndTreeView, &tvInsert);

      dwFetched = 0;
      }

   pEnum->Release();
   }
}

/**************************************************************************

   Tree_CompareProc

**************************************************************************/

int CALLBACK Tree_CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
UNREFERENCED_PARAMETER(lParamSort);

LPITEMINFO  pItem1 = (LPITEMINFO)lParam1;
LPITEMINFO  pItem2 = (LPITEMINFO)lParam2;
HRESULT     hr;

hr = pItem1->pParentFolder->CompareIDs(   0,
                                          pItem1->pidlRel,
                                          pItem2->pidlRel);

if(FAILED(hr))
   return 0;

return (short)SCODE_CODE(GetScode(hr));
}

/**************************************************************************

   Tree_DoItemMenu

**************************************************************************/

void Tree_DoItemMenu(HWND hwndTreeView, HTREEITEM hItem, LPPOINT pptScreen)
{
TVITEM   tvItem;

ZeroMemory(&tvItem, sizeof(tvItem));
tvItem.mask = TVIF_PARAM;
tvItem.hItem = hItem;

if(TreeView_GetItem(hwndTreeView, &tvItem))
   {
   HWND           hwndParent = GetParent(hwndTreeView);
   HRESULT        hr;
   LPITEMINFO     pInfo = (LPITEMINFO)tvItem.lParam;
   IContextMenu   *pcm;
   IShellFolder   *psfFolder = pInfo->pParentFolder;

   if(!psfFolder)
      {
      SHGetDesktopFolder(&psfFolder);
      }
   else
      {
      psfFolder->AddRef();
      }

   if(psfFolder)
      {
      hr = psfFolder->GetUIObjectOf(   hwndParent, 
                                       1, 
                                       (LPCITEMIDLIST*)&pInfo->pidlRel, 
                                       IID_IContextMenu, 
                                       NULL, 
                                       (LPVOID*)&pcm);

      if(SUCCEEDED(hr))
         {
         HMENU hPopup;

         hPopup = CreatePopupMenu();
         if(hPopup)
            {
            hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_NORMAL | CMF_EXPLORE);

            if(SUCCEEDED(hr))
               {
               pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&g_pcm2);

               UINT  idCmd;

               idCmd = TrackPopupMenu( hPopup, 
                                       TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                       pptScreen->x, 
                                       pptScreen->y,
                                       0,
                                       hwndParent,
                                       NULL);
            
               if(g_pcm2)
                  {
                  g_pcm2->Release();
                  g_pcm2 = NULL;
                  }

               if(idCmd)
                  {
                  CMINVOKECOMMANDINFO  cmi;
                  cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
                  cmi.fMask = 0;
                  cmi.hwnd = hwndParent;
                  cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
                  cmi.lpParameters = NULL;
                  cmi.lpDirectory = NULL;
                  cmi.nShow = SW_SHOWNORMAL;
                  cmi.dwHotKey = 0;
                  cmi.hIcon = NULL;
                  hr = pcm->InvokeCommand(&cmi);
                  }
               }
            }
      
         pcm->Release();
         }

      psfFolder->Release();
      }
   }
}

/**************************************************************************

   Tree_DoContextMenu

**************************************************************************/

void Tree_DoContextMenu(HWND hwndTreeView, LPPOINT pptScreen)
{
TVHITTESTINFO  tvhti;

tvhti.pt = *pptScreen;
ScreenToClient(hwndTreeView, &tvhti.pt);

tvhti.flags = LVHT_NOWHERE;

TreeView_HitTest(hwndTreeView, &tvhti);

if(TVHT_ONITEM & tvhti.flags)
   {
   Tree_DoItemMenu(hwndTreeView, tvhti.hItem , pptScreen);
   }
else
   {
   //the tree doesn't support background menus
   }
}


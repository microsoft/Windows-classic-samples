/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          List.cpp

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
#include "resource.h"

/**************************************************************************
   function prototypes
**************************************************************************/

void List_EnumObjects(HWND, LPSHELLFOLDER, LPITEMIDLIST);
int CALLBACK List_CompareProc(LPARAM, LPARAM, LPARAM);
void List_DoDefault(HWND hwndListView, int iItem);

/**************************************************************************
   global variables and definitions
**************************************************************************/

IShellFolder   *g_psfCurFolder = NULL;
LPITEMIDLIST   g_pidlCurFQ = NULL;

/******************************************************************************

   List_Create

******************************************************************************/

HWND List_Create( HINSTANCE hInstance, 
                  HWND hwndParent, 
                  HIMAGELIST himlLarge, 
                  HIMAGELIST himlSmall)
{
DWORD       dwStyle;
HWND        hwndListView;

dwStyle =   WS_TABSTOP | 
            WS_CHILD | 
            WS_BORDER | 
            WS_VISIBLE |
            LVS_AUTOARRANGE |
            LVS_LIST | 
            LVS_SHAREIMAGELISTS |
            LVS_SHOWSELALWAYS |
            0;
            
hwndListView = CreateWindowEx(   WS_EX_CLIENTEDGE,
                                 WC_LISTVIEW,
                                 NULL,
                                 dwStyle,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 (HMENU)IDC_LISTVIEW,
                                 hInstance,
                                 NULL);

if(hwndListView)
   {
   //set the image lists
   ListView_SetImageList(hwndListView, himlLarge, LVSIL_NORMAL);
   ListView_SetImageList(hwndListView, himlSmall, LVSIL_SMALL);
   }

return hwndListView;
}

/**************************************************************************

   List_ReleaseCurrentFolder()

**************************************************************************/

void List_ReleaseCurrentFolder(void)
{
if(g_psfCurFolder)
   {
   g_psfCurFolder->Release();
   g_psfCurFolder = NULL;

   Pidl_Free(g_pidlCurFQ);
   g_pidlCurFQ = NULL;
   }
}

/**************************************************************************

   List_LockCurrentFolder()

**************************************************************************/

void List_LockCurrentFolder(LPITEMINFO pItemInfo)
{
if(pItemInfo && pItemInfo->pParentFolder)
   {
   pItemInfo->pParentFolder->BindToObject(pItemInfo->pidlRel, NULL, IID_IShellFolder, (LPVOID*)&g_psfCurFolder);

   g_pidlCurFQ = Pidl_Copy(pItemInfo->pidlFQ);
   }
else
   {
   SHGetDesktopFolder(&g_psfCurFolder);

   if(pItemInfo)
      g_pidlCurFQ = Pidl_Copy(pItemInfo->pidlFQ);
   }
}

/**************************************************************************

   List_DisplayFolder()

**************************************************************************/

HRESULT List_DisplayFolder(HWND hwndListView, LPITEMINFO pItemInfo)
{
if(pItemInfo)
   {
   List_ReleaseCurrentFolder();

   List_LockCurrentFolder(pItemInfo);
   }

HRESULT  hr = E_FAIL;

ListView_DeleteAllItems(hwndListView);

if(g_psfCurFolder)
   {
   HCURSOR  hCur = LoadCursor(NULL, IDC_WAIT);

   hCur = SetCursor(hCur);

   //turn redawing off in the ListView. This will speed things up as we add items
   SendMessage(hwndListView, WM_SETREDRAW, FALSE, 0);

   List_EnumObjects(hwndListView, g_psfCurFolder, g_pidlCurFQ);

   ListView_SortItems(hwndListView, List_CompareProc, 0);

   //turn redawing back on
   SendMessage(hwndListView, WM_SETREDRAW, TRUE, 0);

   SetCursor(hCur);
   }

return hr;
}

/**************************************************************************

   List_Refresh()

**************************************************************************/

HRESULT List_Refresh(HWND hwndListView)
{
return List_DisplayFolder(hwndListView, NULL);
}

/**************************************************************************

   List_Notify()

**************************************************************************/

LRESULT List_Notify(HWND hWnd, LPARAM lParam)
{
LPNMHDR  pnmh = (LPNMHDR) lParam;
HWND     hwndListView = GetDlgItem(hWnd, IDC_LISTVIEW);

switch(pnmh->code)
   {
   case LVN_GETDISPINFO:
      {
      NMLVDISPINFO   *pdi = (NMLVDISPINFO*)lParam;
      LPITEMINFO     pItem = (LPITEMINFO)pdi->item.lParam;

      //do we need to supply the text?
      if(pdi->item.mask & LVIF_TEXT)
         {
         switch(pdi->item.iSubItem)
            {
            case 0:  //name
               {
               SHFILEINFO     sfi;
      
               //get the display name of the item
               if(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
                  {
                  _tcsncpy_s(pdi->item.pszText,
							 pdi->item.cchTextMax,
							 sfi.szDisplayName,
							 _TRUNCATE);
                  }
               }
               break;

            case 1:  //size
               break;
            }
         }

      //do we need to supply the unselected image?
      if(pdi->item.mask & LVIF_IMAGE)
         {
         SHFILEINFO     sfi;
         
         //get the unselected image for this item
         if(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
            {
            pdi->item.iImage = sfi.iIcon;
            }
         }
      }
      break;

   case LVN_DELETEITEM:
      {
      LPNMLISTVIEW   pnmlv = (LPNMLISTVIEW)lParam;
      LPITEMINFO     pItem = (LPITEMINFO)pnmlv->lParam;

      //free up the pidls that we allocated
      Pidl_Free(pItem->pidlFQ);
      Pidl_Free(pItem->pidlRel);
      
      //this may be NULL if this is the root item
      if(pItem->pParentFolder)
         pItem->pParentFolder->Release();
      
      GlobalFree((HGLOBAL)pItem);
      }
      break;
   
#ifdef LVN_ITEMACTIVATE
   
   case LVN_ITEMACTIVATE:

#else    //LVN_ITEMACTIVATE

   case NM_DBLCLK:
   case NM_RETURN:

#endif   //LVN_ITEMACTIVATE
      {
      //get the item that has the focus
      int   nItem = ListView_GetNextItem(hwndListView, -1, LVNI_FOCUSED);

      if(-1 != nItem)
         List_DoDefault(hwndListView, nItem);
      }
      break;
   
   default:
      break;
   }

return 0;
}

/**************************************************************************

   List_EnumObjects

**************************************************************************/

void List_EnumObjects(  HWND hwndListView,
                        LPSHELLFOLDER pParentFolder, 
                        LPITEMIDLIST pidlParent)
{
LPENUMIDLIST   pEnum;
   
if(SUCCEEDED(pParentFolder->EnumObjects(  NULL, 
                                          SHCONTF_FOLDERS | 
                                             SHCONTF_NONFOLDERS | 
                                             SHCONTF_INCLUDEHIDDEN, 
                                          &pEnum)))
   {
   LPITEMIDLIST   pidlTemp;
   DWORD          dwFetched = 1;
   LPITEMINFO     pItem;

   //enumerate the item's PIDLs
   while(S_OK == (pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
      {
      LVITEM   lvItem;
      DWORD    dwAttr;

      ZeroMemory(&lvItem, sizeof(lvItem));
      
      //fill in the TV_ITEM structure for this item
      lvItem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;

      //AddRef the parent folder so it's pointer stays valid
      pParentFolder->AddRef();

      //put the private information in the lParam
      pItem = (LPITEMINFO)GlobalAlloc(GPTR, sizeof(ITEMINFO));
      
      pItem->pidlRel = pidlTemp;
      pItem->pidlFQ = Pidl_Concatenate(pidlParent, pidlTemp);
      
      pItem->pParentFolder = pParentFolder;
      lvItem.lParam = (LPARAM)pItem;

      //text and images are done on a callback basis
      lvItem.pszText = LPSTR_TEXTCALLBACK;
      lvItem.iImage = I_IMAGECALLBACK;

      //determine if the item is shared
      dwAttr = SFGAO_DISPLAYATTRMASK;
      pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttr);

      if(dwAttr & SFGAO_SHARE)
         {
         lvItem.mask |= LVIF_STATE;
         lvItem.stateMask |= LVIS_OVERLAYMASK;
         lvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
         }
      
      if(dwAttr & SFGAO_GHOSTED)
         {
         lvItem.mask |= LVIF_STATE;
         lvItem.stateMask |= LVIS_CUT;
         lvItem.state |= LVIS_CUT;
         }
      
      ListView_InsertItem(hwndListView, &lvItem);

      dwFetched = 0;
      }

   pEnum->Release();
   }
}

/**************************************************************************

   List_CompareProc

**************************************************************************/

int CALLBACK List_CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

   List_DoItemMenu

**************************************************************************/

void List_DoItemMenu(   HWND hwndListView, 
                        LPINT piItems, 
                        UINT cbItems, 
                        LPPOINT pptScreen)
{
LPITEMIDLIST   *pPidls;

pPidls = (LPITEMIDLIST*)g_pMalloc->Alloc(sizeof(LPITEMIDLIST) * cbItems);
if(!pPidls)
   return;

LVITEM   lvItem;
UINT     i;

for(i = 0; i < cbItems; i++)
   {
   ZeroMemory(&lvItem, sizeof(lvItem));
   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = piItems[i];
   if(ListView_GetItem(hwndListView, &lvItem))
      {
      LPITEMINFO  pInfo = (LPITEMINFO)lvItem.lParam;
      pPidls[i] = pInfo->pidlRel;
      }
   }

if(pPidls[0])
   {
   HRESULT        hr;
   IContextMenu   *pcm;

   if(g_psfCurFolder)
      {
      hr = g_psfCurFolder->GetUIObjectOf( hwndListView, 
                                          cbItems, 
                                          (LPCITEMIDLIST*)pPidls, 
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
                                       GetParent(hwndListView),
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
                  cmi.hwnd = (HWND)GetParent(hwndListView);
                  cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
                  cmi.lpParameters = NULL;
                  cmi.lpDirectory = NULL;
                  cmi.nShow = SW_SHOWNORMAL;
                  cmi.dwHotKey = 0;
                  cmi.hIcon = NULL;
                  hr = pcm->InvokeCommand(&cmi);

                  /*
                  The operation performed by the context menu may have changed 
                  the contents of the folder, so do a refresh.
                  */
                  List_Refresh(hwndListView);
                  }
               }
            }

         pcm->Release();
         }
      }
   }

g_pMalloc->Free(pPidls);
}

/**************************************************************************

   List_DoBackgroundMenu

**************************************************************************/

void List_DoBackgroundMenu(HWND hwndListView, LPPOINT pptScreen)
{
HWND           hwndParent = GetParent(hwndListView);
HRESULT        hr;

if(g_psfCurFolder)
   {
   IContextMenu   *pcm;

   hr = g_psfCurFolder->CreateViewObject( hwndParent, 
                                          IID_IContextMenu, 
                                          (LPVOID*)&pcm);

   if(SUCCEEDED(hr))
      {
      HMENU hMenu;

      hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_MAIN_MENU));
      if(hMenu)
         {
         HMENU hPopup;
         hPopup = GetSubMenu(hMenu, 1);

         if(hPopup)
            {
            int   i;
            UINT  idCmdFirst;
            UINT  idCmd;
         
            //find the largest ID in the menu
            i = 0;
            idCmdFirst = 0;

            while((idCmd = GetMenuItemID(hPopup, i)) != (UINT)-1)
               {
               if(idCmd > idCmdFirst)
                  idCmdFirst = idCmd;
            
               i++;
               }

            hr = pcm->QueryContextMenu(hPopup, 0, ++idCmdFirst, (UINT)-1, CMF_NORMAL | CMF_EXPLORE);

            if(SUCCEEDED(hr))
               {
               pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&g_pcm2);

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
                  if(idCmd < idCmdFirst)
                     {
                     //this is one of our commands
                     FORWARD_WM_COMMAND(hwndParent, idCmd, NULL, 0, SendMessage);
                     }
                  else
                     {
                     CMINVOKECOMMANDINFO  cmi;
                     cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
                     cmi.fMask = 0;
                     cmi.hwnd = hwndParent;
                     cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - idCmdFirst);
                     cmi.lpParameters = NULL;
                     cmi.lpDirectory = NULL;
                     cmi.nShow = SW_SHOWNORMAL;
                     cmi.dwHotKey = 0;
                     cmi.hIcon = NULL;
                     hr = pcm->InvokeCommand(&cmi);

                     /*
                     The operation performed by the context menu may have 
                     changed the contents of the folder, so do a refresh.
                     */
                     List_Refresh(hwndListView);
                     }
                  }
               }
            }
         DestroyMenu(hMenu);
         }

      pcm->Release();
      }
   }
}

/**************************************************************************

   List_DoContextMenu

**************************************************************************/

void List_DoContextMenu(HWND hwndListView, LPPOINT pptScreen)
{
LVHITTESTINFO  lvhti;

lvhti.pt = *pptScreen;
ScreenToClient(hwndListView, &lvhti.pt);

lvhti.flags = LVHT_NOWHERE;

ListView_HitTest(hwndListView, &lvhti);

if(LVHT_ONITEM & lvhti.flags)
   {
   //get the selected items
   UINT  nItems = ListView_GetSelectedCount(hwndListView);
   LPINT pItems = (LPINT)GlobalAlloc(GPTR, sizeof(int) * nItems);

   if(pItems)
      {
      UINT  i;
      int   nCurItem;

      //put the item clicked on first in the list
      pItems[0] = lvhti.iItem;

      for(i = 1, nCurItem = -1; i < nItems; i++)
         {
         nCurItem = ListView_GetNextItem(hwndListView, nCurItem, LVNI_SELECTED);
         if(nCurItem != lvhti.iItem)
            pItems[i] = nCurItem;
         else
            i--;
         }

      List_DoItemMenu(hwndListView, pItems, nItems, pptScreen);

      GlobalFree(pItems);
      }
   }
else
   {
   List_DoBackgroundMenu(hwndListView, pptScreen);
   }
}

/**************************************************************************

   List_DoDefault

**************************************************************************/

void List_DoDefault(HWND hwndListView, int iItem)
{
LVITEM   lvItem;

ZeroMemory(&lvItem, sizeof(lvItem));
lvItem.mask = LVIF_PARAM;
lvItem.iItem = iItem;

if(ListView_GetItem(hwndListView, &lvItem))
   {
   HRESULT        hr;
   LPITEMINFO     pInfo = (LPITEMINFO)lvItem.lParam;
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
      hr = psfFolder->GetUIObjectOf(   hwndListView, 
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
            hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_DEFAULTONLY | CMF_EXPLORE);

            if(SUCCEEDED(hr))
               {
               UINT  idCmd;

               idCmd = GetMenuItemID(hPopup, 0);
            
               if(idCmd && (idCmd != (UINT)-1))
                  {
                  CMINVOKECOMMANDINFO  cmi;
                  cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
                  cmi.fMask = 0;
                  cmi.hwnd = GetParent(hwndListView);
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


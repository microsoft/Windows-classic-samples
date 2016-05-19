/*****************************************************************************
 *
 *  SendTo.c
 *
 *  Copyright (c) 1997 Microsoft Corporation.  All rights reserved.
 *
 *       This source code is only intended as a supplement to
 *       Microsoft Development Tools, q.v. for detailed
 *       information regarding the Microsoft samples programs.
 *
 *  Abstract:
 *
 *      Sample program to demonstrate how a program can display a
 *      "Send To" menu and send documents to items thereon.
 *
 *****************************************************************************/

#include "SendTo.h"

/*****************************************************************************
 *
 *      Overview
 *
 *      This sample program illustrates the following points:
 *
 *      -   Enumerating the contents of the "Send To" folder.
 *
 *      -   Obtaining the IDataObject interface for a file.
 *
 *      -   Obtaining the IDropTarget for a shell extension.
 *
 *      -   Dropping a file onto an item thus enumerated.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *      Notes
 *
 *      Since this program is written in C, invoking OLE methods is
 *      done via
 *
 *          p->lpVtbl->MethodName(p, arg, arg)
 *
 *      To convert this program to C++, the code that invokes OLE
 *      methods should be changed to
 *
 *          p->MethodName(arg, arg)
 *
 *      just like any other C++ method.
 *
 *      For example,
 *
 *          peidl->lpVtbl->Release(peidl);
 *
 *      becomes
 *
 *          peidl->Release();
 *
 *      Also, REFIID parameters in C are passed as
 *
 *          &IID_IName
 *
 *      but they are passed in C++ as
 *
 *          IID_IName
 *
 *      Therefore,
 *
 *          hres = g_psfDesktop->lpVtbl->BindToObject(g_psfDesktop,
 *                      pidl, NULL, &IID_IShellFolder, (LPVOID *)&psf);
 *
 *      becomes
 *
 *          hres = g_psfDesktop->BindToObject(
 *                      pidl, NULL, IID_IShellFolder, (LPVOID *)&psf);
 *
 *****************************************************************************/

HINSTANCE   g_hinst;                /* My hinstance */
LPSHELLFOLDER g_psfDesktop;         /* The desktop folder */
HMENU       g_hmenuSendTo;          /* Our SendTo popup */
TCHAR       g_szFileName[MAX_PATH]; /* The name of the "open" file */

/*****************************************************************************
 *
 *  Our SendTo menu occupies the IDs IDM_SENDTOFIRST through IDM_SENDTOLAST.
 *  The text of the menu item is the name of the target, and the reference
 *  data points to the corresponding PIDL relative to the SENDTO folder.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *  GetSpecialFolder
 *
 *      Create an LPSHELLFOLDER for the specified special folder.
 *
 *      It is the responsibility of the caller to Release() the
 *      IShellFolder that is returned.
 *
 *****************************************************************************/

LPSHELLFOLDER
GetSpecialFolder(HWND hwnd, int idFolder)
{
    HRESULT hres;
    LPSHELLFOLDER psf;
    LPITEMIDLIST pidl;

    hres = SHGetSpecialFolderLocation(hwnd, CSIDL_SENDTO, &pidl);
    if (SUCCEEDED(hres)) {
		hres = g_psfDesktop->lpVtbl->BindToObject(g_psfDesktop,
                    pidl, NULL, &IID_IShellFolder, (LPVOID *)&psf);
        if (SUCCEEDED(hres)) {
            /* Woo-hoo, we're done */
        } else {
            psf = NULL;
        }
        CoTaskMemFree(pidl);
    } else {
        psf = NULL;
    }
    return psf;
}

/*****************************************************************************
 *
 *  PidlFromPath
 *
 *      Convert a path to an LPITEMIDLIST.
 *
 *****************************************************************************/

LPITEMIDLIST
PidlFromPath(HWND hwnd, LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;
    ULONG ulEaten;
    DWORD dwAttributes;
    HRESULT hres;
	WCHAR wszName[MAX_PATH];
#ifdef UNICODE
	if (FAILED(StringCchCopy(wszName, ARRAYSIZE(wszName), pszPath))) {
		return NULL;
	}
#else
    if (!MultiByteToWideChar(CP_ACP, 0, pszPath, -1, wszName, ARRAYSIZE(wszName))) {
        return NULL;
    }
#endif

    hres = g_psfDesktop->lpVtbl->ParseDisplayName(g_psfDesktop, hwnd,
                         NULL, wszName, &ulEaten, &pidl, &dwAttributes);
    if (FAILED(hres)) {
        return NULL;
    }

    return pidl;
}

/*****************************************************************************
 *
 *  GetUIObjectOfAbsPidl
 *
 *      Given an absolute (desktop-relative) LPITEMIDLIST, get the
 *      specified UI object.
 *
 *****************************************************************************/

HRESULT
GetUIObjectOfAbsPidl(HWND hwnd, LPITEMIDLIST pidl, REFIID riid, LPVOID *ppvOut)
{
    /*
     *  To get the UI object of an absolute pidl, we must first bind
     *  to its parent, and then call GetUIObjectOf on the last part.
     */

    LPITEMIDLIST pidlLast;
    LPSHELLFOLDER psf;
    HRESULT hres;

    /*
     *  Just for safety's sake.
     */
    *ppvOut = NULL;

	/*
	 *  Bind to the parent folder of the item we are interested in.
	 */
	hres = SHBindToParent(pidl, &IID_IShellFolder, (LPVOID *)&psf, &pidlLast);
    if (FAILED(hres)) {
        /*
         *  Couldn't even get to the parent; we have no chance of
         *  getting to the item itself.
         */
        return hres;
    }

    /*
     *  Now ask the parent for the the UI object of the child.
     */
    hres = psf->lpVtbl->GetUIObjectOf(psf, hwnd, 1, &pidlLast,
                                riid, NULL, ppvOut);

    /*
     *  Regardless of whether or not the GetUIObjectOf succeeded,
     *  we have no further use for the parent folder.
     */
    psf->lpVtbl->Release(psf);

    return hres;
}

/*****************************************************************************
 *
 *  GetUIObjectOfPath
 *
 *      Given an absolute path, get its specified UI object.
 *
 *****************************************************************************/

HRESULT
GetUIObjectOfPath(HWND hwnd, LPCTSTR pszPath, REFIID riid, LPVOID *ppvOut)
{
    LPITEMIDLIST pidl;
    HRESULT hres;

    /*
     *  Just for safety's sake.
     */
    *ppvOut = NULL;

    pidl = PidlFromPath(hwnd, pszPath);
    if (!pidl) {
        return E_FAIL;
    }

    hres = GetUIObjectOfAbsPidl(hwnd, pidl, riid, ppvOut);

    CoTaskMemFree(pidl);

    return hres;
}

/*****************************************************************************
 *
 *  DoDrop
 *
 *      Drop a data object on a drop target.
 *
 *****************************************************************************/

void
DoDrop(LPDATAOBJECT pdto, LPDROPTARGET pdt)
{
    POINTL pt = { 0, 0 };
    DWORD dwEffect;
    HRESULT hres;

    /*
     *  The data object enters the drop target via the left button
     *  with all drop effects permitted.
     */
    dwEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
    hres = pdt->lpVtbl->DragEnter(pdt, pdto, MK_LBUTTON, pt, &dwEffect);
    if (SUCCEEDED(hres) && dwEffect) {
        /*
         *  The drop target likes the data object and the effect.
         *  Go drop it.
         */
        hres = pdt->lpVtbl->Drop(pdt, pdto, MK_LBUTTON, pt, &dwEffect);

    } else {
        /*
         *  The drop target didn't like us.  Tell it we're leaving,
         *  sorry to bother you.
         */
        hres = pdt->lpVtbl->DragLeave(pdt);
    }
}

/*****************************************************************************
 *
 *  SendTo_OnCreate
 *
 *      When we are created, remember the handle of our SendTo menu
 *      so we can recognize it later.
 *
 *****************************************************************************/

BOOL
SendTo_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    MENUITEMINFO mii;
    HMENU hmenu;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;

    hmenu = GetMenu(hwnd);
    if (GetMenuItemInfo(hmenu, IDM_SENDTOPOPUP, FALSE, &mii)) {
        g_hmenuSendTo = mii.hSubMenu;
    }

    return TRUE;
}

/*****************************************************************************
 *
 *  SendTo_ResetSendToMenu
 *
 *      Wipe out all the items in the menu, freeing the associated memory.
 *
 *****************************************************************************/

void
SendTo_ResetSendToMenu(HMENU hmenu)
{
    MENUITEMINFO mii;

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA;

    while (GetMenuItemInfo(hmenu, 0, TRUE, &mii)) {
        if (mii.dwItemData) {
            CoTaskMemFree((LPVOID)mii.dwItemData);
        }
        DeleteMenu(hmenu, 0, MF_BYPOSITION);
    }

}

/*****************************************************************************
 *
 *  SendTo_FillSendToMenu
 *
 *      Enumerate the contents of the SendTo folder and fill the
 *      menu with the items therein.
 *
 *****************************************************************************/

void
SendTo_FillSendToMenu(HWND hwnd, HMENU hmenu)
{
    MENUITEMINFO mii;
    LPSHELLFOLDER psf;
    LPITEMIDLIST pidl;
    LPENUMIDLIST peidl;
    HRESULT hres;
    STRRET str;
    UINT idm = IDM_SENDTOFIRST;

    psf = GetSpecialFolder(hwnd, CSIDL_SENDTO);
    if (psf) {
        hres = psf->lpVtbl->EnumObjects(psf, hwnd,
                    SHCONTF_FOLDERS | SHCONTF_NONFOLDERS,
                    &peidl);
        if (SUCCEEDED(hres)) {
            while (peidl->lpVtbl->Next(peidl, 1, &pidl, NULL) == S_OK &&
                   idm < IDM_SENDTOLAST) {
                hres = psf->lpVtbl->GetDisplayNameOf(psf, pidl,
                                    SHGDN_NORMAL, &str);
                if (SUCCEEDED(hres)) {
					LPTSTR pszName;
					hres = StrRetToStr(&str, pidl, &pszName);
					if (SUCCEEDED(hres)) {
                        if (AppendMenu(hmenu, MF_ENABLED | MF_STRING,
                                       idm, pszName)) {
                            mii.cbSize = sizeof(mii);
                            mii.fMask = MIIM_DATA;
                            mii.dwItemData = (ULONG_PTR)pidl;
                           SetMenuItemInfo(hmenu, idm, FALSE, &mii);
                            idm++;
						}
						CoTaskMemFree(pszName);
                    }
                }
            }
            peidl->lpVtbl->Release(peidl);
        }
        psf->lpVtbl->Release(psf);

    }

    /*
     *  If the menu is still empty (the user has an empty SendTo folder),
     *  then add a disabled "None" item so we have at least something
     *  to display.
     */
    if (idm == IDM_SENDTOFIRST) {
        AppendMenu(hmenu, MF_GRAYED | MF_DISABLED | MF_STRING,
                   idm, TEXT("(none)"));
    }
}

/*****************************************************************************
 *
 *  SendTo_OnInitMenuPopup
 *
 *      When the SendTo menu pops up, enumerate the contents of the
 *      SendTo folder and populate the menu.
 *
 *****************************************************************************/

void
SendTo_OnInitMenuPopup(HWND hwnd, HMENU hmenu, UINT item, BOOL fSystemMenu)
{

    /*
     *  If it's the SendTo menu, then rebuild it.
     */
    if (hmenu == g_hmenuSendTo) {
        SendTo_ResetSendToMenu(hmenu);
        SendTo_FillSendToMenu(hwnd, hmenu);
    }
}

/*****************************************************************************
 *
 *  SendTo_OnOpen
 *
 *      "Open" a file.  Just get its name and save it in our global
 *      g_szFileName variable.  (And update the title too, just to
 *      look pretty.)
 *
 *****************************************************************************/

void
SendTo_OnOpen(HWND hwnd)
{
    OPENFILENAME ofn;
    TCHAR szFileName[MAX_PATH];
    TCHAR szTitle[64];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    StringCchCopy(szFileName, ARRAYSIZE(szFileName), g_szFileName);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.lpstrFileTitle = szTitle;
    ofn.nMaxFileTitle = ARRAYSIZE(szTitle);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn)) {
        StringCchCopy(g_szFileName, ARRAYSIZE(g_szFileName), szFileName);
        StringCchPrintf(szFileName, ARRAYSIZE(szFileName), TEXT("%s - SendTo Demo"), szTitle);
        SetWindowText(hwnd, szFileName);
    }
}

/*****************************************************************************
 *
 *  SendTo_SendToItem
 *
 *      The user selected an item from our SendTo menu.
 *
 *      hwnd - window
 *      idm - menu item id
 *
 *****************************************************************************/

void
SendTo_SendToItem(HWND hwnd, int idm)
{
    MENUITEMINFO mii;
    LPSHELLFOLDER psf;
    LPITEMIDLIST pidl;
    LPDATAOBJECT pdto;
    LPDROPTARGET pdt;
    HRESULT hres;

    /*
     *  First convert our filename to a data object.
     */
    hres = GetUIObjectOfPath(hwnd, g_szFileName, &IID_IDataObject,
                                    (LPVOID *)&pdto);

    if (SUCCEEDED(hres)) {

        /*
         *  Now go find the item we should send to.
         */
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_DATA;
        if (GetMenuItemInfo(g_hmenuSendTo, idm, FALSE, &mii) &&
            mii.dwItemData) {
            pidl = (LPITEMIDLIST)mii.dwItemData;

            /*
             *  Now convert the send to pidl to a drop target.
             */
            psf = GetSpecialFolder(hwnd, CSIDL_SENDTO);
            if (psf) {
                hres = psf->lpVtbl->GetUIObjectOf(psf, hwnd, 1, &pidl,
                                    &IID_IDropTarget, NULL, (LPVOID *)&pdt);
                if (SUCCEEDED(hres)) {

                    /*
                     *  Now drop the file on the drop target.
                     */

                    DoDrop(pdto, pdt);
                    pdt->lpVtbl->Release(pdt);
                }

                psf->lpVtbl->Release(psf);
            }
        }
        pdto->lpVtbl->Release(pdto);
    }

}

/*****************************************************************************
 *
 *  SendTo_OnCommand
 *
 *      Handle our menu messages.
 *
 *      Our "Send To" commands live in the range
 *      IDM_SENDTOFIRST through IDM_SENDTOLAST.
 *
 *****************************************************************************/

void
SendTo_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id >= IDM_SENDTOFIRST && id <= IDM_SENDTOLAST) {
        SendTo_SendToItem(hwnd, id);
    } else if (id == IDM_OPEN) {
        SendTo_OnOpen(hwnd);
    }
}

/*****************************************************************************
 *
 *  SendTo_OnDestroy
 *
 *      When our window is destroyed, clean up the memory associated
 *      with the SendTo submenu.
 *
 *      Also post a quit message because our application is over.
 *
 *****************************************************************************/

void
SendTo_OnDestroy(HWND hwnd)
{
    if (g_hmenuSendTo) {
        SendTo_ResetSendToMenu(g_hmenuSendTo);
    }

    PostQuitMessage(0);
}

/*****************************************************************************
 *
 *  SendTo_WndProc
 *
 *      Window procedure for the Send To demo.
 *
 *****************************************************************************/

LRESULT CALLBACK
SendTo_WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    HANDLE_MSG(hwnd, WM_CREATE, SendTo_OnCreate);

    HANDLE_MSG(hwnd, WM_COMMAND, SendTo_OnCommand);

    HANDLE_MSG(hwnd, WM_INITMENUPOPUP, SendTo_OnInitMenuPopup);

    HANDLE_MSG(hwnd, WM_DESTROY, SendTo_OnDestroy);

    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

/*****************************************************************************
 *
 *  InitApp
 *
 *      Register our window classes and otherwise prepare for action.
 *
 *****************************************************************************/

BOOL
InitApp(void)
{
    WNDCLASS wc;
    HRESULT hr;

    wc.style = 0;
    wc.lpfnWndProc = SendTo_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hinst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = TEXT("SendToMenu");
    wc.lpszClassName = TEXT("SendTo");

    RegisterClass(&wc);

    hr = SHGetDesktopFolder(&g_psfDesktop);
    if (FAILED(hr)) {
        return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
 *
 *  TermApp
 *
 *      Clean up.
 *
 *****************************************************************************/

void
TermApp(void)
{
    if (g_psfDesktop) {
        g_psfDesktop->lpVtbl->Release(g_psfDesktop);
        g_psfDesktop = NULL;
    }
}

/*****************************************************************************
 *
 *  WinMain
 *
 *      Program entry point.
 *
 *      Demonstrate Send To menu.
 *
 *****************************************************************************/

int WINAPI
WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR pszCmdLine, int nCmdShow)
{
    MSG msg;
    HWND hwnd;
    HRESULT hrInit;

    g_hinst = hinst;

    if (!InitApp()) return 0;

    hrInit = CoInitialize(NULL);

    hwnd = CreateWindow(
        TEXT("SendTo"),                 /* Class Name */
        TEXT("SendTo Demo"),            /* Title */
        WS_OVERLAPPEDWINDOW,            /* Style */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* Size */
        NULL,                           /* Parent */
        NULL,                           /* No menu */
        hinst,                          /* Instance */
        0);                             /* No special parameters */

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TermApp();

    if (SUCCEEDED(hrInit))
        CoUninitialize();

    return 0;
}

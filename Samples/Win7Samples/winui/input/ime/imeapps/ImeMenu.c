/**********************************************************************/
/*                                                                    */
/*      IMEMENU.C                                                     */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <commdlg.h>
#include <commctrl.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

LPDWORD lpdwIDTable = NULL;
int nIDTable = 0;

/**********************************************************************/
/*                                                                    */
/*    EndMenuItemIDTable()                                            */
/*                                                                    */
/**********************************************************************/
void EndMenuItemIDTable()
{
    if (lpdwIDTable) 
    {
        GlobalFree((HANDLE)lpdwIDTable);
        lpdwIDTable = NULL;
    }

    nIDTable = 0;
}

/**********************************************************************/
/*                                                                    */
/*    InitMenuItemIDTable()                                           */
/*                                                                    */
/**********************************************************************/
void InitMenuItemIDTable()
{
    EndMenuItemIDTable();
    lpdwIDTable = GlobalAlloc(GPTR, 100 * 2 * sizeof(DWORD));
}

/**********************************************************************/
/*                                                                    */
/*    AddIDTableEntry()                                               */
/*                                                                    */
/**********************************************************************/
void AddIDTableEntry(UINT wID, DWORD dwItemData)
{
    *(lpdwIDTable+(nIDTable*2)) = wID;
    *(lpdwIDTable+(nIDTable*2)+1) = dwItemData;
    nIDTable++;
}

/**********************************************************************/
/*                                                                    */
/*    FindItemData()                                                  */
/*                                                                    */
/**********************************************************************/
DWORD FindItemData(UINT wID)
{
    int i;
    for (i = 0; i < nIDTable ; i++)
    {
        if (*(lpdwIDTable+(i*2)) == wID)
        {
            return *(lpdwIDTable+(i*2)+1);
        }
    }
    return 0;
}


/**********************************************************************/
/*                                                                    */
/*    FillMenuItemInfo()                                              */
/*                                                                    */
/**********************************************************************/
void FillMenuItemInfo(HWND hWnd, HIMC hIMC, LPMENUITEMINFO lpmii, LPMYIMEMENUITEMINFO lpIme, BOOL fRight)
{
    FillMemory((PVOID)lpmii, sizeof(MENUITEMINFO), 0);
    lpmii->cbSize = sizeof(MENUITEMINFO);
    lpmii->fMask = 0;

    // Set fType;
    if (lpIme->fType)
    {
        lpmii->fMask |= MIIM_TYPE;
        lpmii->fType = 0;

        if (lpIme->fType & IMFT_RADIOCHECK)
        {
            lpmii->fType |= MFT_RADIOCHECK;
        }

        if (lpIme->fType & IMFT_SEPARATOR)
        {
            lpmii->fType |= MFT_SEPARATOR;
        }

    }

    lpmii->fMask |= MIIM_ID;
    lpmii->wID = lpIme->wID + IDM_STARTIMEMENU;

    if (lpIme->fType & IMFT_SUBMENU)
    {
        lpmii->fMask |= MIIM_SUBMENU;
        lpmii->hSubMenu = CreateImeMenu(hWnd, hIMC, lpIme, fRight);
    }

    lpmii->fMask |= MIIM_STATE;
    lpmii->fState = lpIme->fState;

    if (lpIme->hbmpChecked &&  lpIme->hbmpUnchecked)
    {
       lpmii->fMask |= MIIM_CHECKMARKS;
       lpmii->hbmpChecked = lpIme->hbmpChecked;
       lpmii->hbmpUnchecked = lpIme->hbmpUnchecked;
    }
    

    lpmii->fMask |= MIIM_DATA;
    lpmii->dwItemData = lpIme->dwItemData;

    if (lpIme->hbmpItem)
    {
       lpmii->fMask |= MIIM_BITMAP;
       lpmii->hbmpItem = lpIme->hbmpItem;
    }


#ifdef USEWAPI
    {
        BOOL bUDC;
        int i;
        char   szTemp[IMEMENUITEM_STRING_SIZE] = {0};

        i = WideCharToMultiByte(CP_ACP,
                                0,
                                lpIme->szString,
                                (int)wcslen(lpIme->szString),
                                szTemp,
                                IMEMENUITEM_STRING_SIZE,
                                (LPSTR)NULL,
                                &bUDC);
        szTemp[i] = '\0';

        if (lstrlen(szTemp))
        {
            lpmii->fMask |= MIIM_STRING;
            StringCchCopyNA((LPSTR)lpIme->szString, ARRAYSIZE(lpIme->szString), szTemp, IMEMENUITEM_STRING_SIZE-1);
            lpmii->dwTypeData = (LPSTR)lpIme->szString;
            lpmii->cch = lstrlen((LPSTR)lpIme->szString);
        }
    }
#else
    if (lstrlen(lpIme->szString))
    {
        lpmii->fMask |= MIIM_STRING;
        lpmii->dwTypeData = lpIme->szString;
        lpmii->cch = lstrlen(lpIme->szString);
    }
#endif

}

/**********************************************************************/
/*                                                                    */
/*    CreateImeMenu()                                                 */
/*                                                                    */
/**********************************************************************/
HMENU CreateImeMenu(HWND hWnd, HIMC hIMC, LPMYIMEMENUITEMINFO lpImeParentMenu, BOOL fRight)
{
    HMENU hMenu = NULL;
    DWORD dwSize, dwNum, dwI;
    MENUITEMINFO mii = {0};
    LPMYIMEMENUITEMINFO lpImeMenu = NULL;
    

    dwNum = MyImmGetImeMenuItems(hIMC, fRight ? IGIMIF_RIGHTMENU : 0, 
                                 IGIMII_CMODE |
                                 IGIMII_SMODE |
                                 IGIMII_CONFIGURE |
                                 IGIMII_TOOLS |
                                 IGIMII_HELP |
                                 IGIMII_OTHER,
                                 lpImeParentMenu, NULL, 0);

    if (!dwNum)
    {
        return 0;
    }



    dwSize = dwNum * sizeof(MYIMEMENUITEMINFO);

    lpImeMenu = (LPMYIMEMENUITEMINFO)GlobalAlloc(GPTR, dwSize);

    dwNum = MyImmGetImeMenuItems(hIMC, fRight ? IGIMIF_RIGHTMENU : 0, 
                                 IGIMII_CMODE |
                                 IGIMII_SMODE |
                                 IGIMII_CONFIGURE |
                                 IGIMII_TOOLS |
                                 IGIMII_HELP |
                                 IGIMII_OTHER,
                                 lpImeParentMenu, lpImeMenu, dwSize);

    
    hMenu = CreatePopupMenu();

    for (dwI = 0 ; dwI < dwNum; dwI++)
    {
        AddIDTableEntry(lpImeMenu->wID, lpImeMenu->dwItemData);
        FillMenuItemInfo(hWnd, hIMC, &mii, lpImeMenu + dwI, fRight);
        InsertMenuItem(hMenu, dwI, TRUE, &mii);
    }

    GlobalFree((HANDLE)lpImeMenu);


    return hMenu;
}





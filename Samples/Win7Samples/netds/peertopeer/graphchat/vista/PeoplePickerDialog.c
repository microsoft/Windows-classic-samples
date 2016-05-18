/**********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PeoplePickerDialog.c

Abstract:

    This C file includes a sample, simple People Picker dialog.

Feedback:
    If you have any questions or feedback, please contact us using 
    any of the mechanisms below:

    Email: peerfb@microsoft.com 
    Newsgroup: Microsoft.public.win32.programmer.networks 
    Website: http://www.microsoft.com/p2p 

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include "PeoplePickerDialog.h"

// Forward Declarations
//
HRESULT InitPeoplePickerDialog(HWND hDlg);
void AddPersonToListView(HWND hDlg, LPARAM lParam);
void RemovePersonFromListView(HWND hDlg, LPARAM lParam);
void ClearPeopleFromListView(HWND hDlg);
INT_PTR CALLBACK PeoplePicker(HWND, UINT, WPARAM, LPARAM);

//-----------------------------------------------------------------------------
// Function: ShowPeoplePicker
//
// Purpose:  Displays dialog and returns chosen user
//
// Returns:  HRESULT
//
INT_PTR ShowPeoplePicker(HINSTANCE hInst, HWND hWnd, PEER_PEOPLE_NEAR_ME ** ppPerson)
{
    if (ppPerson == NULL)
        return FALSE;

    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PEOPLEPICKERDIALOG), hWnd, PeoplePicker, (LPARAM) ppPerson);
}

//-----------------------------------------------------------------------------
// Function: PeoplePickerFreePerson
//
// Purpose:  Frees a PEER_PEOPLE_NEAR_ME
//
// Returns:  HRESULT
//
void PeoplePickerFreePerson(PEER_PEOPLE_NEAR_ME * pPerson)
{
    PeoplePickerModelFreePerson(pPerson);
}


//-----------------------------------------------------------------------------
// Function: InitPeoplePickerDialog
//
// Purpose:  Initializes the people picker dialog setting up the list view
//             and initializing the model
//
// Returns:  HRESULT
//
HRESULT InitPeoplePickerDialog(HWND hDlg)
{
    HRESULT hr = S_OK;
    LV_COLUMN lvCol = {0};
    HWND hList = NULL;

    // Setup the column
    //
    lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
    lvCol.pszText = L"Name";
    lvCol.cx = 0x190;

    // Add the column
    //
    hList = GetDlgItem(hDlg,IDC_PEOPLELIST);

    if (NULL != hList)
    {
        SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
        SendMessage(hList, LVM_INSERTCOLUMN, 0, (LPARAM) &lvCol);
    }
    else
    {
        MessageBox(NULL, L"Critical error", L"Critical error", MB_OK);
        return E_UNEXPECTED;
    }
    
    // Init the data model
    //
    hr = InitPeoplePickerModel(hDlg);
    
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: AddPersonToListView
//
// Purpose:  Creates the list view item associating the 
//             person added with the LPARAM of the list view item.
//
// Returns:  VOID
//
VOID AddPersonToListView(HWND hDlg, LPARAM lParam)
{
    HWND hList = NULL;
    LVITEM lvItem = {0};
    PEER_PEOPLE_NEAR_ME * pPerson = (PEER_PEOPLE_NEAR_ME *) lParam;

    // Setup listview item & unpack person near me info
    //
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;  
    lvItem.cchTextMax = 256;
    lvItem.iItem = 0; 
    lvItem.iSubItem = 0;
    lvItem.pszText = pPerson->pwzNickName;
    lvItem.lParam = (LPARAM) pPerson;

    // Get the list from the dialog to insert the item
    // 
    hList = GetDlgItem(hDlg, IDC_PEOPLELIST);

    if (ListView_InsertItem(hList, &lvItem) == -1)
    {
        MessageBox(hDlg, L"Unable to add person to the list", L"Error", MB_OK);
        PeoplePickerModelFreePerson(pPerson);
    }
}

//-----------------------------------------------------------------------------
// Function: RemovePersonFromListView
//
// Purpose:  Removes the person from the ListView
//
// Returns:  VOID
//
VOID RemovePersonFromListView(HWND hDlg, LPARAM lParam)
{
    HWND hList = NULL;
    LVFINDINFO lvItemToFind = {0};
    LVITEM lvItem = {0};
    int index = -1;
    PEER_PEOPLE_NEAR_ME * pPerson = (PEER_PEOPLE_NEAR_ME*) lParam;
    PEER_PEOPLE_NEAR_ME * pTempPerson = NULL;

    // Setup the search item
    //
    lvItemToFind.flags = LVFI_STRING;
    lvItemToFind.psz = pPerson->pwzNickName;
    
    // Setup the LVITEM info wanted
    //
    lvItem.mask = LVIF_PARAM;

    hList = GetDlgItem(hDlg, IDC_PEOPLELIST);
    
    // Get the list and find the item
    //
    index = ListView_FindItem(hList, index, &lvItemToFind);

    while (-1 != index)
    {
        lvItem.iItem = index;

        if (ListView_GetItem(hList, &lvItem))
        {
            pTempPerson = (PEER_PEOPLE_NEAR_ME *) lvItem.lParam;

            if (0 == memcmp(&(pPerson->id), &(pTempPerson->id), sizeof(GUID)))
            {            
                // Free the person we associated with the list view and delete it from the list view
                // 
                PeoplePickerModelFreePerson((PEER_PEOPLE_NEAR_ME *)lvItem.lParam);
                ListView_DeleteItem(hList, index);
                PeoplePickerModelFreePerson(pPerson);
                return;
            }
            else
            {
                index = ListView_FindItem(hList, index, &lvItemToFind);
            }
        }
    }
    PeoplePickerModelFreePerson(pPerson);
}

//-----------------------------------------------------------------------------
// Function: ClearPeopleFromListView
//
// Purpose:  Cleans up the list view and all LPARAMs containing people
//
// Returns:  VOID
//
VOID ClearPeopleFromListView(HWND hDlg)
{
    HWND hList = NULL;
    LVITEM lvItem = {0};
    int index = 0;
    int count = 0;

    // Get the list
    //
    hList = GetDlgItem(hDlg,IDC_PEOPLELIST);

    // Get the total # of items
    //
    count = ListView_GetItemCount(hList);

    for (index = 0; index < count; index++)
    {
        // Get the item to delete
        //
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = index;

        if (ListView_GetItem(hList,&lvItem))
        {
            // Free the person associated with the list view item
            //
            PeoplePickerModelFreePerson((PEER_PEOPLE_NEAR_ME *)lvItem.lParam);
        }
    }

    // Clear the list view
    //
    ListView_DeleteAllItems(hList);
}


//-----------------------------------------------------------------------------
// Function: AcceptSelectedItem
//
// Purpose:  Save the PNM contact represented by the index and close the dialog.
//
// Returns:  HRESULT
//
HRESULT AcceptSelectedItem(HWND hDlg, int index)
{
    HRESULT hr = E_FAIL;
    HWND hList = NULL;
    LVITEM lvItem = {0};
    PEER_PEOPLE_NEAR_ME * pSelectedPerson = NULL;
    PEER_PEOPLE_NEAR_ME ** ppPerson = NULL;

    if (index < 0) return E_INVALIDARG;

    hList = GetDlgItem(hDlg, IDC_PEOPLELIST);

    //Setup the lvItem to get the actual item at the index
    //
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = index;

    if (ListView_GetItem(hList, &lvItem))
    {
        //Get the relevant fields from the person stored in the item's LPARAM
        //
        pSelectedPerson = (PEER_PEOPLE_NEAR_ME *) lvItem.lParam;
        ppPerson = (PEER_PEOPLE_NEAR_ME **) (LONG_PTR) GetWindowLongPtr(hDlg, DWLP_USER);

        hr = DuplicatePeerPeopleNearMe(ppPerson, pSelectedPerson);
    }

    // Clean up the dialog and associated model
    //
    ClearPeopleFromListView(hDlg);
    PeoplePickerModelDestroy();

    if (SUCCEEDED(hr))
    {
        EndDialog(hDlg, IDOK);
    }
    else
    {
        EndDialog(hDlg, IDCANCEL);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: PeoplePicker
//
// Purpose:  People Picker Dialog Proc
//
// Returns:  HRESULT
//
INT_PTR CALLBACK PeoplePicker(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hList = NULL;
    int index = 0;

    switch (message)
    {
        case WM_INITDIALOG:
            // Save the PEER_PEOPLE_NEAR_ME output param received from ShowPeoplePicker
            //
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG) lParam);
            InitPeoplePickerDialog(hDlg);
            return (INT_PTR)TRUE;

        case WM_ADDPERSON:
            AddPersonToListView(hDlg, lParam);
            return (INT_PTR)TRUE;
        
        case WM_REMOVEPERSON:
            RemovePersonFromListView(hDlg, lParam);
            return (INT_PTR)TRUE;

        case WM_CLEARPEOPLE:
            ClearPeopleFromListView(hDlg);
            return (INT_PTR)TRUE;

        case WM_NOTIFY:
            // Check the list to see if something is selected
            // If so, enable the "OK" button, if not disable the "OK" button.
            //
            if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
            {
                hList = GetDlgItem(hDlg,IDC_PEOPLELIST);

                if (ListView_GetSelectedCount(hList))
                {
                    EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                }
            }

            if (((LPNMHDR)lParam)->code == NM_DBLCLK)
            {
                index = ((LPNMITEMACTIVATE)lParam)->iItem;

                if (index >= 0)
                {
                    AcceptSelectedItem(hDlg, index);
                }
            }

            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                hList = GetDlgItem(hDlg, IDC_PEOPLELIST);

                //Get the index of the first selected item
                //
                index = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

                if (index >= 0)
                {
                    AcceptSelectedItem(hDlg, index);
                }

                return (INT_PTR)TRUE;
            }

            if (LOWORD(wParam) == IDCANCEL)
            {
                // Clean up the dialog and associated model
                //
                ClearPeopleFromListView(hDlg);
                PeoplePickerModelDestroy();
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

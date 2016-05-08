//-----------------------------------------------------------------------------
// File: Listbox.cpp
// Desc: Listbox control class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "wincontrol.h"
#include "listbox.h"

//-----------------------------------------------------------------------------
// ListBox constructor
//-----------------------------------------------------------------------------

ListBox::ListBox()
: m_bMultiSelect(false)
{

}


//-----------------------------------------------------------------------------
// Name: SetWindow
// Desc: Override the base class to check if we are a multi-select list.
//-----------------------------------------------------------------------------

void ListBox::SetWindow(HWND hwnd)
{
    Control::SetWindow(hwnd);
    LONG style = GetWindowLong(m_hwnd, GWL_STYLE);
    m_bMultiSelect = HasStyle(LBS_MULTIPLESEL) || HasStyle(LBS_EXTENDEDSEL);
}


//-----------------------------------------------------------------------------
// Name: Count
// Desc: Returns the number of items in the listbox.
//-----------------------------------------------------------------------------

UINT ListBox::Count()
{
    LRESULT res = SendMessage(LB_GETCOUNT, 0, 0);
    return (res == LB_ERR ? 0 : (UINT)res);
}

//-----------------------------------------------------------------------------
// Name: AddString
// Desc: Adds an item to the listbox.
//
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------

BOOL ListBox::AddString(LPCTSTR sz)
{
    if (!sz)
    {
        return FALSE;
    }

    LRESULT res = SendMessage(LB_ADDSTRING, 0, (LPARAM)sz);
    if (res == LB_ERR || res == LB_ERRSPACE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//-----------------------------------------------------------------------------
// Name: GetString
// Desc: Returns a string at a specified index. 
//
// ppStr: Receives a pointer to the string. The method allocates the string.
//        The caller must free the string using CoTaskMemFree.
//
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------

BOOL ListBox::GetString(UINT index, TCHAR **ppStr)
{
    if (!ppStr)
    {
        return FALSE;
    }

    *ppStr = NULL;

    LRESULT cch = SendMessage(LB_GETTEXTLEN, (WPARAM)index, 0);
    if (cch == LB_ERR)
    {
        return FALSE;
    }

    TCHAR *str = (TCHAR*)CoTaskMemAlloc(sizeof(TCHAR) * (cch + 1));
    if (str == NULL)
    {
        return FALSE;
    }

    cch = SendMessage(LB_GETTEXT, (WPARAM)index, (LPARAM)str);
    if (cch == LB_ERR)
    {
        CoTaskMemFree(str);
        return FALSE;
    }

    *ppStr = str;
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: AddItem
// Desc: Adds a string plus user data to a list box.
//
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------

BOOL ListBox::AddItem(LPCTSTR szName, void* pItemData)
{
    if (!szName)
    {
        return FALSE;
    }

    LRESULT result = SendMessage(LB_ADDSTRING, 0, (LPARAM)szName);
    if (result == LB_ERR || result == LB_ERRSPACE)
    {
        return FALSE;
    }
    if (pItemData)
    {
        UINT index = (UINT)result;
        result = SendMessage(LB_SETITEMDATA, (WPARAM)index, (LPARAM)pItemData);

        if (result == LB_ERR)
        {
            // ??
            SendMessage(LB_DELETESTRING, index, 0);
            return FALSE;
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: GetItem
// Desc: Returns the user data associated with a listbox item (or NULL).
//
// index: Index of the listbox item.
//-----------------------------------------------------------------------------

void* ListBox::GetItem(UINT index)
{
    LRESULT result = SendMessage(LB_GETITEMDATA, index, 0);
    if (result == LB_ERR)
    {
        return NULL;
    }
    return (void*)result;
}


//-----------------------------------------------------------------------------
// Name: Select
// Desc: Sets the current selection.
//
// index: Zero-based index.
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------

BOOL ListBox::Select(UINT index)
{
    LRESULT res;
    
    if (IsMultiSelect())
    {   
        res = SendMessage(LB_SETSEL, (WPARAM)TRUE, (LPARAM)index);
    }
    else
    {
        res = SendMessage(LB_SETCURSEL, (WPARAM)index, 0);
    }

    if (res == LB_ERR)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//-----------------------------------------------------------------------------
// Name: GetCurrentSelection
// Desc: Returns the current selection. (Single-select list only)
//
// pindex: Receives the zero-based index of the current selection.
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------


BOOL ListBox::GetCurrentSelection(UINT *pindex)
{
    if (!pindex)
    {
        return FALSE;
    }

    if (IsMultiSelect())
    {
        return FALSE;
    }


    LRESULT res = SendMessage(LB_GETCURSEL, 0, 0);
    if (res == LB_ERR)
    {
        return FALSE;
    }
    else
    {
        *pindex = (int)res;
        return TRUE;
    }
}

//-----------------------------------------------------------------------------
// Name: GetMultiSelection
// Desc: Returns an array of selection indices. (Multi-select list only)
//
// ppIndexes: Receives a pointer to the array. The caller must free the
//            array, using CoTaskMemFree
// pCount:    Receives the size of the array.
//
// Returns TRUE if successful, or FALSE if an error occurred.
//-----------------------------------------------------------------------------

BOOL ListBox::GetMultiSelection(UINT **ppIndexes, UINT *pCount)
{
    if (!ppIndexes || !pCount)
    {
        return FALSE;
    }

    *ppIndexes = NULL;
    *pCount = 0;

    if (!IsMultiSelect())
    {
        return FALSE;
    }
    UINT selCount = (UINT)SendMessage(LB_GETSELCOUNT, 0, 0);
    if (selCount == LB_ERR)
    {
        return FALSE;
    }

    UINT *pIndexes = (UINT*)CoTaskMemAlloc(sizeof(UINT) * selCount);
    if (pIndexes == NULL)
    {
        return FALSE;
    }

    if (LB_ERR == SendMessage(LB_GETSELITEMS, (WPARAM)selCount, (LPARAM)pIndexes))
    {
        CoTaskMemFree(pIndexes);
        return FALSE;
    }

    *pCount = selCount;
    *ppIndexes = pIndexes;
    return TRUE;
}
    


//-----------------------------------------------------------------------------
// Name: ClearSelection
// Desc: Clears the current selection.
//-----------------------------------------------------------------------------

void ListBox::ClearSelection()
{
    if (m_bMultiSelect)
    {
        UINT c = Count();
        for (UINT i = 0; i < c; i++)
        {
            SendMessage(LB_SETSEL, (WPARAM)FALSE, (LPARAM)i);
        }
    }
    else
    {
        SendMessage(LB_SETCURSEL, -1, 0);

        // Per MSDN: In this case, SendMessage returns LB_ERR even though 
        // no error has occurred. Go figure.
    }
}

//-----------------------------------------------------------------------------
// Name: DeleteItem
// Desc: Deletes a listbox item. 
//
// Note: Does not release any user data associated with the item.
//-----------------------------------------------------------------------------

BOOL ListBox::DeleteItem(UINT index)
{
    LRESULT res = SendMessage(LB_DELETESTRING, index, 0);
    return (res == LB_ERR ? FALSE : TRUE);
}


//-----------------------------------------------------------------------------
// Name: ClearItems
// Desc: Removes all items.
//-----------------------------------------------------------------------------

void ListBox::ClearItems()
{
    SendMessage(LB_RESETCONTENT, 0, 0);
}
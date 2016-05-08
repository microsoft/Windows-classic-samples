//-----------------------------------------------------------------------------
// File: ComboBox.cpp
// Desc: ComboBox control class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "wincontrol.h"
#include "ComboBox.h"

//-----------------------------------------------------------------------------
// Name: AddString
// Description: Add a string to the combo box list.
//-----------------------------------------------------------------------------

BOOL ComboBox::AddString(LPCTSTR sItem)
{
    if (!sItem)
    {
        return FALSE;
    }

    LRESULT result = SendMessage(CB_ADDSTRING, 0, (LPARAM)sItem);
    if (result == CB_ERR || result == CB_ERRSPACE)
    {
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: AddString
// Description: Add a string to the combo box list, with an associated
//              data item.
//-----------------------------------------------------------------------------

BOOL ComboBox::AddString(LPCTSTR sItem, DWORD_PTR pData)
{
    if (!sItem)
    {
        return FALSE;
    }

    LRESULT result = SendMessage(CB_ADDSTRING, 0, (LPARAM)sItem);
    if (result == CB_ERR || result == CB_ERRSPACE)
    {
        return FALSE;
    }
    else
    {
        return SetItemData((int)result, pData);
    }
}
    

//-----------------------------------------------------------------------------
// Name: InsertString
// Description: Insert a string at a specified index.
//-----------------------------------------------------------------------------

BOOL ComboBox::InsertString(int index, LPCTSTR sItem)
{
    if (!sItem)
    {
        return FALSE;
    }

    LRESULT result = SendMessage(CB_INSERTSTRING, (WPARAM)index, (LPARAM)sItem);
    if (result == CB_ERR || result == CB_ERRSPACE)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: GetCurrentSelection
// Description: Get the index of the current selection.
//-----------------------------------------------------------------------------

BOOL ComboBox::GetCurrentSelection(int *pindex)
{
    if (!pindex)
    {
        return FALSE;
    }

    LRESULT res = SendMessage(CB_GETCURSEL, 0, 0);
    if (res == CB_ERR)
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
// Name: Select
// Description: Select an item in the list, by index.
//-----------------------------------------------------------------------------

BOOL ComboBox::Select(int index)
{
    LRESULT res = SendMessage(CB_SETCURSEL, (WPARAM)index, 0);
    if (res == CB_ERR)
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
// Description: Get the string at a specified index.
//
// The caller must call CoTaskMemFree to release the string.
//-----------------------------------------------------------------------------

BOOL ComboBox::GetString(int index, TCHAR **ppBuffer)
{
    if (ppBuffer == NULL)
    {
        return FALSE;
    }

    LRESULT res = SendMessage(CB_GETLBTEXTLEN, (WPARAM)index, 0);
    if (res == CB_ERR)
    {
        return FALSE;
    }

    DWORD count = (DWORD)(res + 1);

    TCHAR *pBuffer = (TCHAR*)CoTaskMemAlloc(sizeof(TCHAR) * count);
    if (pBuffer == NULL)
    {
        return FALSE;
    }

    res = SendMessage(CB_GETLBTEXT, (WPARAM)index, (LPARAM)pBuffer);

    if (res == CB_ERR)
    {
        CoTaskMemFree(pBuffer);
        return FALSE;
    }
    else
    {
        *ppBuffer = pBuffer;
        return TRUE;
    }
}

//-----------------------------------------------------------------------------
// Name: GetSelectedString
// Description: Get the string for the item that is currently selected.
//
// The caller must call CoTaskMemFree to release the string.
//-----------------------------------------------------------------------------

BOOL ComboBox::GetSelectedString(TCHAR **ppBuffer)
{
    int index = 0;
    if (GetCurrentSelection(&index))
    {
        return GetString(index, ppBuffer);
    }
    else
    {
        return FALSE;
    }
}


//-----------------------------------------------------------------------------
// Name: Count
// Description: Returns the number of items in the combo box.
//-----------------------------------------------------------------------------

UINT ComboBox::Count()
{
    LRESULT res = SendMessage(CB_GETCOUNT, 0, 0);
    return (res == CB_ERR ? 0 : (UINT)res);
}


//-----------------------------------------------------------------------------
// Name: ClearItems
// Description: Clears the combo box list.
//
// Note: If the combo box items have user data associated with them, the
//       application must release the free the memory for the user data before 
//       calling this method.
//-----------------------------------------------------------------------------

void ComboBox::ClearItems()
{
    SendMessage(CB_RESETCONTENT, 0, 0);
}


//-----------------------------------------------------------------------------
// Name: SetItemData
// Description: Sets the user data for an item in the combo box.
//
// nIndex: Index of the combo box item.
// pData: User data.
//-----------------------------------------------------------------------------

BOOL ComboBox::SetItemData(int nIndex, DWORD_PTR pData)
{
    LRESULT result = SendMessage(CB_SETITEMDATA, nIndex, (LPARAM) pData);
	if (result == CB_ERR)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//-----------------------------------------------------------------------------
// Name: GetItemData
// Description: Gets the user data for an item in the combo box.
// 
// nIndex: Index of the combo box item.
// pData: Receives a pointer to the data.
//-----------------------------------------------------------------------------

BOOL ComboBox::GetItemData(int nIndex, DWORD_PTR *ppData)
{

	LRESULT result = SendMessage(CB_GETITEMDATA, nIndex, 0);
	if (result == CB_ERR)
	{
		return FALSE;
	}
	else
	{
		*ppData = (DWORD_PTR)result;
		return TRUE;
	}
}

//-----------------------------------------------------------------------------
// Name: GetCurrentSelectionItemData
// Description: Gets the user data for the current selection.
//-----------------------------------------------------------------------------

BOOL ComboBox::GetCurrentSelectionItemData(DWORD_PTR *pData)
{
	int selection;
	BOOL bResult = GetCurrentSelection(&selection);
	if (bResult)
	{
		bResult = GetItemData(selection, pData);
	}
	return bResult;
}
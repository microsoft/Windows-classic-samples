//-----------------------------------------------------------------------------
// File: ComboBox.h
// Desc: ComboBox control class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once


class ComboBox : public Control
{

public:
    BOOL AddString(LPCTSTR sItem);
    BOOL AddString(LPCTSTR sItem, DWORD_PTR pData);
    BOOL InsertString(int index, LPCTSTR sItem);

    BOOL GetCurrentSelection(int *pindex);
    BOOL Select(int index);
    BOOL GetString(int index, TCHAR **ppBuffer);
    BOOL GetSelectedString(TCHAR **ppBuffer);

    void ClearItems();
    UINT Count();

	BOOL SetItemData(int nIndex, DWORD_PTR pData);
	BOOL GetItemData(int nIndex, DWORD_PTR *ppData);
	BOOL GetCurrentSelectionItemData(DWORD_PTR *pData);

};
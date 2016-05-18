//-----------------------------------------------------------------------------
// File: Listbox.h
// Desc: Listbox control class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

class ListBox : public Control
{
protected:
    bool m_bMultiSelect;

public:
    ListBox();
    void SetWindow(HWND hwnd);
    bool IsMultiSelect() const { return m_bMultiSelect; }

    BOOL AddString(LPCTSTR sz);
    BOOL AddItem(LPCTSTR szName, void* pItemData);
    BOOL GetString(UINT index, TCHAR **ppStr);
    void* GetItem(UINT index);
    BOOL DeleteItem(UINT  index);

    BOOL Select(UINT index);
    void ClearSelection();
    BOOL GetCurrentSelection(UINT *pindex);
    BOOL GetMultiSelection(UINT **ppIndexes, UINT *pCount);

    void ClearItems();
    UINT Count();
};

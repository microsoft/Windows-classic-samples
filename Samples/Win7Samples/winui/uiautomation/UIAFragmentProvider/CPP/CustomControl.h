/*************************************************************************************************
* Description: Declarations for the custom list control.
* 
* See EntryPoint.cpp for a full description of this sample.
*   
*
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
*************************************************************************************************/
#pragma once
#pragma warning (disable : 4244)  // Disable bogus warning for SetWindowLongPtr

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreAPI.h>
#include "resource.h"

#include <deque>
using namespace std;

#include <assert.h>

enum ContactStatus 
{
    Status_Offline,
    Status_Online
};


// Custom message type for adding new contacts.
#define CUSTOMLB_ADDITEM WM_USER + 1

// Iterator type for list of items.
#define LISTITERATOR std::deque<CustomListItem*>::iterator

void RegisterListControl(HINSTANCE hInstance);
LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HFONT GetFont(LONG height);

// Forward declarations.
class CustomListItem;
class ListProvider;
class ListItemProvider;

// CustomList control class -- the list box itself.
//
class CustomListControl
{
public:
    // For simplicity, declare some properties as constants.
    static const int MaxItems = 10;
    // Height of list item.
    static const int ItemHeight = 15;
    // Dimensions of image that signifies item status.
    static const int ImageWidth = 10;
    static const int ImageHeight = 10;

private:
    bool   m_hasFocus;
    int    m_selectedIndex; 
    HWND   m_controlHwnd;
    deque<CustomListItem*> m_itemCollection;
    ListProvider*   m_pListProvider;

public:
    CustomListControl(HWND hwnd);
    virtual ~CustomListControl();
    ListProvider* GetListProvider();
    int IndexFromY(int y);
    void SelectItem(int index);
    int GetSelectedIndex(); 
    bool GetIsFocused();
    void SetIsFocused(bool isFocused);
    bool AddItem(ContactStatus status, WCHAR* name);
    LISTITERATOR GetItemAt(int index);
    bool RemoveSelected();
    int GetCount();
    int CreateUniqueId();
    HWND GetHwnd();
};

// CustomListItem control class -- an item in the list.
//
class CustomListItem
{
private:
    int m_Id; 
    WCHAR* m_name;
    ContactStatus m_status;
    ListItemProvider* m_pListItemProvider;
    CustomListControl* m_pOwnerControl;

public:
    CustomListItem(CustomListControl* owner, int id, WCHAR* name);
    virtual ~CustomListItem();
    int GetItemIndex();
    CustomListControl* GetOwnerList();
    ListItemProvider* GetListItemProvider();
    ContactStatus GetStatus();
    void SetStatus(ContactStatus status);
    WCHAR* GetName();
    int GetId();
};
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
#pragma warning (disable : 4244)  // Disable bogus warning for SetWindowLongPtr
#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <oleacc.h>
#include "resource.h"
#include <deque>
using namespace std;
#define LISTITERATOR std::deque<CustomListControlItem*>::iterator

// Forward declarations.
class CustomListControlItem;
class AccServer;


// Values for status of contacts.
enum ContactStatus 
{
    Status_Offline,
    Status_Online
};

// Custom message types.
#define CUSTOMLB_ADDITEM            (WM_USER + 1)
#define CUSTOMLB_DEFERDOUBLECLICK   (WM_USER + 2)
#define CUSTOMLB_DELETEITEM         (WM_USER + 3)


void RegisterListControl(HINSTANCE hInstance);
LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HFONT GetFont(LONG height);


// CustomList control class -- the list box itself.
//
class CustomListControl
{
private:
    bool   m_hasFocus;
    int    m_selectedIndex; 
    HWND   m_controlHwnd;
    std::deque<CustomListControlItem*> m_itemCollection;
    AccServer* m_pAccServer;

public:
    // For simplicity, declare some properties as constants.
    static const int MaxItems = 10;
    // Height of list item.
    static const int ItemHeight = 15;
    // Dimensions of image that signifies item status.
    static const int ImageWidth = 10;
    static const int ImageHeight = 10;

    CustomListControl(HWND hwnd);
    virtual ~CustomListControl();
    AccServer* GetAccServer();
    void SetAccServer(AccServer* pAccServer);

    int IndexFromY(int y);
    void SelectItem(int index);
    int GetSelectedIndex(); 
    bool GetIsFocused();
    void SetIsFocused(bool isFocused);
    bool AddItem(ContactStatus status, WCHAR* name);
    LISTITERATOR GetItemAt(int index);
    bool RemoveSelected();
    int GetCount();
    bool GetItemScreenRect(int index, RECT* pRetVal);
    void OnDoubleClick();
};

// CustomListItem control class -- an item in the list.
//
class CustomListControlItem
{
private:
    WCHAR* m_name;
    ContactStatus m_status;

public:
    CustomListControlItem(WCHAR* name);
    virtual ~CustomListControlItem();
    ContactStatus GetStatus();
    void SetStatus(ContactStatus status);
    WCHAR* GetName();
};

// Helper function.
inline CustomListControl* GetControl(HWND hwnd)
{
    return reinterpret_cast<CustomListControl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

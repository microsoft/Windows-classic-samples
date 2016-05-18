/*************************************************************************************************
* Description: Implementation of the custom list control.
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
#include "CustomControl.h"
#include "AccServer.h"

// CustomListControl class.
//
CustomListControl::CustomListControl(HWND hwnd) :
    m_selectedIndex(-1), m_controlHwnd(hwnd), m_pAccServer(NULL)
{
}

// Destructor.
//
CustomListControl::~CustomListControl()
{
    // Free the items in the collection.
    for (int i = static_cast<int>(m_itemCollection.size()) - 1; i >= 0; i--)
    {
        CustomListControlItem* pItem = m_itemCollection.at(i);
        delete pItem;
    }

    // Destroy the accessible object.
    if (m_pAccServer!= NULL)
    {
        // Notify the accessibility object that the control no longer exists.
        m_pAccServer->SetControlIsAlive(FALSE);
        // Release the reference created in WM_GETOBJECT.
        m_pAccServer->Release(); 
    }   
}

void CustomListControl::SetAccServer(AccServer* pAccServer)
{
    m_pAccServer = pAccServer;
}

AccServer* CustomListControl::GetAccServer()
{
    return m_pAccServer;
}

// Adds an item to the end of the list.
//
bool CustomListControl::AddItem(ContactStatus status, WCHAR* name)
{
    if (GetCount() >= MaxItems)
    {
        return false;
    }
    CustomListControlItem* newItem = new (std::nothrow) CustomListControlItem(name);
    if (newItem)
    {
        newItem->SetStatus(status);

        // Add to collection.
        m_itemCollection.push_back(newItem);  

        // Send WinEvent.
        NotifyWinEvent(EVENT_OBJECT_CREATE, m_controlHwnd, OBJID_CLIENT, 
            (LONG)m_itemCollection.size());

        // Initialize selection when first item is added.
        if (GetSelectedIndex() < 0)
        {
            SelectItem(0);
        }
        // Force visual refresh.
        InvalidateRect(m_controlHwnd, NULL, TRUE);

        return true;
    }

    return false;
}

// Gets the item at the specified index.
//
LISTITERATOR CustomListControl::GetItemAt(int index)
{
    return m_itemCollection.begin() + index;
}


// Removes the specified item.
//
bool CustomListControl::RemoveSelected()
{
    int index = GetSelectedIndex();
    LISTITERATOR itemToDelete = GetItemAt(index);
    // Don't allow deletion of the last remaining item. This is just to
    // simplify the logic of the sample.
    if (GetCount() == 1)
    {
        return FALSE;
    }
    CustomListControlItem* pItem = static_cast<CustomListControlItem*>(*itemToDelete);
    // Remove from list.
    m_itemCollection.erase(itemToDelete);
    // Delete object.
    delete pItem;

    // Select at the same index; if we deleted the bottom item, 
    // the index will be decremented.
    SelectItem(GetSelectedIndex());   

    // Raise WinEvent.
    NotifyWinEvent(EVENT_OBJECT_DESTROY, m_controlHwnd, OBJID_CLIENT, static_cast<LONG>(index) + 1);
    return TRUE;
}

// Gets the index of the item at a point on the Y coordinate within the list.
//
int CustomListControl::IndexFromY(int y)
{
    int index = y / ItemHeight;
    if ((index < 0) || (GetCount() <= index))
    {
        index = -1;
    }
    return index;
}

// Sets the selected item.
//
void CustomListControl::SelectItem(int index)
{
    m_selectedIndex = index;
    if (m_selectedIndex >= static_cast<int>(m_itemCollection.size()))
    {
        m_selectedIndex = static_cast<int>(m_itemCollection.size()) - 1;  
    }

    // Raise WinEvents.
    NotifyWinEvent(EVENT_OBJECT_SELECTION, m_controlHwnd, OBJID_CLIENT, m_selectedIndex + 1);
    if (GetIsFocused())
    {
        NotifyWinEvent(EVENT_OBJECT_FOCUS, m_controlHwnd, OBJID_CLIENT, m_selectedIndex + 1);
    }

    // Force refresh.
    InvalidateRect(m_controlHwnd, NULL, TRUE);
}

// Gets the index of the selected item.
//
int CustomListControl::GetSelectedIndex()
{
    return m_selectedIndex;
}

// Gets the focused state.
//
bool CustomListControl::GetIsFocused()
{
    return m_hasFocus;
}

// Sets the focused state.
//
void CustomListControl::SetIsFocused(bool isFocused)
{
    m_hasFocus = isFocused;
}

// Gets the count of items in the list.
//
int CustomListControl::GetCount()
{
    return static_cast<int>(m_itemCollection.size());
}

// Gets the bounds of the specified item.
//
bool CustomListControl::GetItemScreenRect(int index, RECT* pRetVal)
{
    if ((pRetVal == NULL) || (index >= static_cast<int>(m_itemCollection.size())) || (index < 0))
    {
        return false;
    }
    // Get the container rectangle.
    RECT parentRect;
    GetClientRect(m_controlHwnd, &parentRect);
    // Align to size of contents.
    InflateRect(&parentRect, -4, -4);

    // Convert top left corner to screen coordinates.
    POINT upperLeft;
    upperLeft.x = parentRect.left;  
    upperLeft.y = parentRect.top;
    ClientToScreen(m_controlHwnd, &upperLeft);

    // Get coordinates of list item.
    pRetVal->left = upperLeft.x;
    pRetVal->right = pRetVal->left + parentRect.right - parentRect.left;
    pRetVal->top = upperLeft.y + (ItemHeight * index);
    pRetVal->bottom = pRetVal->top + ItemHeight;
    return true;
}


// Responds to double-click on an item. For simplicity, we simply show the name
// of the selected contact.
// This method is simply to demonstrate how a default action is invoked by 
// IAccessible::accDoDefaultAction. 
//
void CustomListControl::OnDoubleClick()
{
    LISTITERATOR item = GetItemAt(GetSelectedIndex());
    CustomListControlItem* pItem = static_cast<CustomListControlItem*>(*item);
    HWND h = GetParent(m_controlHwnd);
    WCHAR* name = pItem->GetName();
    MessageBox(h, name, TEXT("Contact"), MB_OK);
}


// Registers the control class.
//
void RegisterListControl(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc      = ControlWndProc;
    wc.hInstance        = hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName    = TEXT("CONTACTLIST");
    RegisterClass(&wc);
}


// Handles window messages for the HWND that contains the custom control.
//
LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_CREATE:
        {
            // Create the control object.
            CustomListControl* pCustomList = new (std::nothrow) CustomListControl(hwnd);

            // Save the class instance as window data so that its members 
            // can be accessed from within this function.
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCustomList);
            break;
        }

    case WM_DESTROY:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);
            // Destroy the control.
            delete pCustomList;

            break;
        }

    case WM_GETOBJECT:
        {
            // Return the IAccessible object.
            if (static_cast<LONG>(lParam) == OBJID_CLIENT)
            {
                // Retrieve the control. 
                CustomListControl* pCustomList = GetControl(hwnd);

                // Create the accessible object.
                AccServer* pAccServer = pCustomList->GetAccServer();
                if (pAccServer == NULL)
                {
                    pAccServer = new (std::nothrow) AccServer(hwnd, pCustomList);
                    pCustomList->SetAccServer(pAccServer);
                }
                if (pAccServer != NULL)  // NULL if out of memory.
                {
                    LRESULT Lresult = LresultFromObject(IID_IAccessible, wParam, 
                        static_cast<IAccessible*>(pAccServer));
                    return Lresult;
                }
                else return 0;
            }
            break;
        }

    case WM_PAINT:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            // Set up graphics context.
            PAINTSTRUCT paintStruct;
            HDC hdc = BeginPaint(hwnd, &paintStruct);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);


            // Save the context.
            HGDIOBJ oldHgdi = SelectObject(hdc, GetStockObject(BLACK_PEN)); 

            // Draw items.
            // Create and select a null pen so the rectangle isn't outlined.
            HPEN nullPen = CreatePen(PS_NULL, 1, RGB(0,0,0));
            SelectObject(hdc, nullPen);

            // Erase the whole window.
            Rectangle(hdc, clientRect.left, clientRect.top, clientRect.right, 
                clientRect.bottom);

            // Create and select the font.
            HFONT font = GetFont(8);
            HGDIOBJ oldFont = SelectObject(hdc, font);

            // Set transparency for text.
            SetBkMode(hdc, TRANSPARENT); 

            int itemHeight = pCustomList->ItemHeight;

            // Create brushes
            HBRUSH unfocusedFillBrush  = GetSysColorBrush(COLOR_BTNFACE);
            HBRUSH focusedFillBrush = GetSysColorBrush(COLOR_HIGHLIGHT);

            HBRUSH onlineFillBrush = CreateSolidBrush(RGB(0, 192, 0));  // Green.
            HBRUSH offlineFillBrush = CreateSolidBrush(RGB(255, 0, 0)); // Red.

            if (pCustomList->GetCount() > 0)
            {
                for (int i = 0; i < pCustomList->GetCount(); i++)              
                {
                    // Get the rectangle for the item.
                    RECT itemRect;
                    itemRect.left = clientRect.left + 2;
                    itemRect.top = clientRect.top + 2 + itemHeight * i;
                    itemRect.right = clientRect.right - 2;
                    itemRect.bottom = itemRect.top + itemHeight;

                    // Set the default text color.
                    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

                    // Set up the appearance of the focused item.
                    // It's different depending on whether the list control has focus.
                    if (i == pCustomList->GetSelectedIndex())
                    {
                        if (pCustomList->GetIsFocused())
                        {
                            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                            HGDIOBJ oldBrush = SelectObject(hdc, focusedFillBrush);
                            Rectangle(hdc, itemRect.left+1, itemRect.top+1, 
                                itemRect.right, itemRect.bottom);
                            SelectObject(hdc, oldBrush);
                        }
                        else
                        {
                            HGDIOBJ oldBrush = SelectObject(hdc, unfocusedFillBrush);
                            Rectangle(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom);
                            SelectObject(hdc, oldBrush);
                        }
                        DrawFocusRect(hdc, &itemRect); 
                    }
                    // Get the item.
                    LISTITERATOR item = pCustomList->GetItemAt(i);
                    CustomListControlItem* pItem = static_cast<CustomListControlItem*>(*item);

                    // Draw the text.
                    TextOut(hdc, itemRect.left + pCustomList->ImageWidth + 5, itemRect.top + 2, 
                        pItem->GetName(), static_cast<int>(wcslen(pItem->GetName())));

                    // Draw the status icon.
                    if (pItem->GetStatus() == Status_Online)
                    {
                        SelectObject(hdc, onlineFillBrush);
                        Rectangle(hdc, itemRect.left + 2, itemRect.top + 3,
                            itemRect.left + pCustomList->ImageWidth + 2, itemRect.top + 3 + pCustomList->ImageHeight);
                    }
                    else
                    {
                        SelectObject(hdc, offlineFillBrush);
                        Ellipse(hdc, itemRect.left + 2, itemRect.top + 3,
                            itemRect.left + pCustomList->ImageWidth + 2, itemRect.top + 3 + pCustomList->ImageHeight);
                    }
                }  // for each item.
            }

            EndPaint(hwnd, &paintStruct);
            // Restore context.
            SelectObject(hdc, oldFont);
            SelectObject(hdc, oldHgdi);
            // Clean brushes.
            DeleteObject(font);
            DeleteObject(nullPen);
            DeleteObject(focusedFillBrush);
            DeleteObject(unfocusedFillBrush);
            DeleteObject(onlineFillBrush);
            DeleteObject(offlineFillBrush);
            break;
        }

    case WM_SETFOCUS:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);
            if (pCustomList != NULL)
            {
                pCustomList->SetIsFocused(TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;        
        }
    case WM_KILLFOCUS:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            pCustomList->SetIsFocused(FALSE); 
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

    case CUSTOMLB_DELETEITEM:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);
            pCustomList->RemoveSelected();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

    case CUSTOMLB_ADDITEM:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            if (pCustomList->GetCount() < pCustomList->MaxItems)
            {
                pCustomList->AddItem(static_cast<ContactStatus>(wParam), (WCHAR*)lParam);
            }
            break;
        }

    case WM_GETDLGCODE:
        {
            // Trap arrow keys.
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
            break;
        }

    case WM_LBUTTONDBLCLK:
    case CUSTOMLB_DEFERDOUBLECLICK:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            // Check that the click was on an item. If CUSTOMLB_DEFERDOUBLECLICK,
            // lParam is 0. 
            int itemClicked = pCustomList->IndexFromY(HIWORD(lParam));
            if (itemClicked >= 0)
            {
                pCustomList->OnDoubleClick();
            }
            break;
        }

    case WM_LBUTTONDOWN:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            // Get the item under the cursor. This is -1 if the user clicked on a blank space.
            int y = HIWORD(lParam);
            int item = pCustomList->IndexFromY(y);

            // Set the focus to the control regardless of whether the selection is valid.
            SetFocus(hwnd);
            if (item >= 0)
            {
                pCustomList->SelectItem(item);
            }

            break;
        }


    case WM_KEYDOWN:
        // Move the selection with up/down arrows.
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            switch (wParam)
            {
            case VK_UP:
                if (pCustomList->GetSelectedIndex() > 0) 
                {
                    pCustomList->SelectItem(pCustomList->GetSelectedIndex() - 1);
                }
                return 0;
                break;

            case VK_DOWN:
                if (pCustomList->GetSelectedIndex() < pCustomList->GetCount() - 1)          
                {
                    pCustomList->SelectItem(pCustomList->GetSelectedIndex() + 1);  
                }
                return 0;
                break;
            }
            break; // WM_KEYDOWN
        }
    }  // switch (message)

    return DefWindowProc(hwnd, message, wParam, lParam);
}


// CustomListControlItem class 
//
CustomListControlItem::CustomListControlItem(WCHAR* name)
{
    // In case of failure, name will be set to NULL, which is acceptable.
    m_name = _wcsdup(name);
}

CustomListControlItem::~CustomListControlItem()
{
    free(m_name);
}

// Gets the status (online/offline) of this contact.
//
ContactStatus CustomListControlItem::GetStatus()
{
    return m_status;
}

// Sets the status (online/offline) of this contact.
//
void CustomListControlItem::SetStatus(ContactStatus status)
{
    m_status = status;
}

// Gets the name of the contact.
//
WCHAR* CustomListControlItem::GetName()
{
    return m_name;
}


// Helper functions. 
//
// Retrieves a font for list items.
//
HFONT GetFont(LONG height) 
{ 
    // Get a handle to the ANSI fixed-pitch font, and copy 
    // information about the font to a LOGFONT structure. 
    static LOGFONT lf;
    GetObject(GetStockObject(ANSI_VAR_FONT), sizeof(LOGFONT), &lf); 

    // Change the font size.
    lf.lfHeight = height;

    // Create the font and return its handle.  
    return CreateFont(lf.lfHeight, lf.lfWidth, 
        lf.lfEscapement, lf.lfOrientation, lf.lfWeight, 
        lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, 
        lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, 
        lf.lfPitchAndFamily, lf.lfFaceName); 
} 


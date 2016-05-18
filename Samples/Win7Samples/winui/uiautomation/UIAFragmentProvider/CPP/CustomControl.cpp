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
#include "UIAProviders.h"


// CustomListControl class.
//
CustomListControl::CustomListControl(HWND hwnd) :
    m_selectedIndex(-1), m_pListProvider(NULL), m_controlHwnd(hwnd)
{
    // Initialize the list items.
    AddItem(Status_Online, L"Fred");
    AddItem(Status_Offline, L"Prakash");
    AddItem(Status_Online, L"Kim");
    AddItem(Status_Online, L"Sandra");
    AddItem(Status_Offline, L"Silvio");
    SelectItem(0);
}

// Destructor.
//
CustomListControl::~CustomListControl()
{
    if (m_pListProvider != NULL)
    {
        m_pListProvider->Release();
    }
    m_itemCollection.clear();
}

// Adds an item to the end of the list.
//
bool CustomListControl::AddItem(ContactStatus status, WCHAR* name)
{
    if ((name == NULL) || (GetCount() >= MaxItems))
    {
        return false;
    }
    int id = CreateUniqueId();
    CustomListItem* newItem = new (std::nothrow) CustomListItem(this, id, name);
    if (newItem != NULL)
    {
        newItem->SetStatus(status);

        // Add to collection.
        m_itemCollection.push_back(newItem);  
        SelectItem(0);

        // Raise UI Automation event.
        ListItemProvider* itemProvider = newItem->GetListItemProvider();
        itemProvider->NotifyItemAdded();
        return true;
    }

    return false;
}

// Removes the selected item.
//
bool CustomListControl::RemoveSelected()
{
    int index = GetSelectedIndex();
    LISTITERATOR itemToDelete = GetItemAt(index);

    // Don't allow deletion of the last remaining item. This is just to
    // simplify the logic of the sample.
    if (GetCount() == 1)
    {
        return false;
    }
    CustomListItem* pItem = static_cast<CustomListItem*>(*itemToDelete);

    // Raise event.
    ListItemProvider* itemProvider = pItem->GetListItemProvider();
    itemProvider->NotifyItemRemoved();

    // Remove from list.
    m_itemCollection.erase(itemToDelete);
    // Delete object.
    delete pItem;

    // Select at the same index; if we deleted the bottom item, 
    // the index will be decremented.
    SelectItem(index);   
    return true;
}

LISTITERATOR CustomListControl::GetItemAt(int index)
{
    return m_itemCollection.begin() + index;
}


// Gets the UI Automation provider for the list; creates it if necessary.
//
ListProvider* CustomListControl::GetListProvider()
{
    if (m_pListProvider == NULL)
    {
        m_pListProvider = new (std::nothrow) ListProvider(this);   
    }
    return m_pListProvider;
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

// Sets the selected item and forces refresh.
//
void CustomListControl::SelectItem(int index)
{
    m_selectedIndex = index;
    if (m_selectedIndex >= static_cast<int>(m_itemCollection.size()))
    {
        m_selectedIndex = static_cast<int>(m_itemCollection.size()) - 1;  
    }
    InvalidateRect(m_controlHwnd, NULL, false);

    // Raise UI Automation event

    if (m_pListProvider != NULL)
    {
        ListItemProvider* itemProvider = m_pListProvider->GetItemProviderByIndex(m_selectedIndex);
        if (itemProvider != NULL)
        {
            itemProvider->NotifyElementSelected();
        }
    }
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

// Creates a unique identifier within this instance of the control.
//
int CustomListControl::CreateUniqueId()
{
    static int uniqueId;
    return uniqueId++;
};

// Gets the HWND of the control.
//
HWND CustomListControl::GetHwnd()
{
    return m_controlHwnd;
}


// Registers the control class.
//
void RegisterListControl(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = ControlWndProc;
    wc.hInstance        = hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName    = L"CONTACTLIST";
    RegisterClass(&wc);
}


// Helper function.
CustomListControl* GetControl(HWND hwnd)
{
    return reinterpret_cast<CustomListControl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCustomList));
            break;
        }

    case WM_DESTROY:
        {
            // Destroy the control so interfaces are released.
            CustomListControl* pCustomList = GetControl(hwnd);
            delete pCustomList;
            break;
        }

    case WM_GETOBJECT:
        {
            // Register the control with UI Automation.
            // If the lParam matches the RootObjectId, send back the list provider.
            if (static_cast<long>(lParam) == static_cast<long>(UiaRootObjectId))
            {
                // Get the control.
                CustomListControl* pCustomList = GetControl(hwnd);
                // Return its associated UI Automation provider.
                LRESULT lresult = UiaReturnRawElementProvider(
                    hwnd, wParam, lParam, pCustomList->GetListProvider());
                return lresult;
            }
            return 0;
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

            int ItemHeight = pCustomList->ItemHeight;

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
                    itemRect.top = clientRect.top + 2 + ItemHeight * i;
                    itemRect.right = clientRect.right - 2;
                    itemRect.bottom = itemRect.top + ItemHeight;

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
                    CustomListItem* pItem = static_cast<CustomListItem*>(*item);

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
            CustomListControl* pCustomList = GetControl(hwnd);
            if (pCustomList != NULL)
            {
                pCustomList->SetIsFocused(true);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;        
        }
    case WM_KILLFOCUS:
        {
            CustomListControl* pCustomList = GetControl(hwnd);
            pCustomList->SetIsFocused(false); 
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

    case WM_DELETEITEM:
        {
            CustomListControl* pCustomList = GetControl(hwnd);
            pCustomList->RemoveSelected();
            break;
        }

    case CUSTOMLB_ADDITEM:
        {
            CustomListControl* pCustomList = GetControl(hwnd);
            pCustomList->AddItem((ContactStatus)wParam, (WCHAR*)lParam);
            break;
        }

    case WM_GETDLGCODE:
        {
            // Trap arrow keys.
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
            break;
        }

    case WM_LBUTTONDOWN:
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            // Get the item under the cursor. This is -1 if the user clicked on a blank space.
            int y = HIWORD(lParam);
            int newItem = pCustomList->IndexFromY(y);

            // Set the focus to the control regardless of whether the selection is valid.
            SetFocus(hwnd);
            if (newItem >= 0)
            {
                pCustomList->SelectItem(newItem);
            }

            break;
        }

    case WM_KEYDOWN:
        // Move the selection with up/down arrows.
        {
            // Retrieve the control.
            CustomListControl* pCustomList = GetControl(hwnd);

            // Ignore keystrokes if listbox does not have focus.
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


// CustomListItem class 
//
// Constructor.
CustomListItem::CustomListItem(CustomListControl* pOwner, int id, WCHAR* name)
{
    m_pOwnerControl = pOwner;
    m_Id = id;
    m_name = _wcsdup(name);
    m_pListItemProvider = NULL;
}

// Destructor.
CustomListItem::~CustomListItem()
{
    free(m_name);
    if (m_pListItemProvider != NULL)
    {
        m_pListItemProvider->Release();
    }
}

// Gets the index of this item in the collection.
//
int CustomListItem::GetItemIndex()
{
    for (int i = 0; i < m_pOwnerControl->GetCount(); i++)
    {
        LISTITERATOR item = m_pOwnerControl->GetItemAt(i);
        CustomListItem* pItem = static_cast<CustomListItem*>(*item);
        if (pItem == this)
        {
            return i;
        }
    }
    // Item not found; shouldn't happen.
    return -1;
}

// Gets the UI Automation provider for the list item; creates it if necessary.
//
ListItemProvider* CustomListItem::GetListItemProvider()
{
    if (m_pListItemProvider == NULL)
    {
        m_pListItemProvider = new (std::nothrow) ListItemProvider(this);  
    }
    return m_pListItemProvider;
}

// Gets the custom list control that holds this item.
//
CustomListControl* CustomListItem::GetOwnerList()
{
    return m_pOwnerControl;
}

// Gets the status (online/offline) of this contact.
//
ContactStatus CustomListItem::GetStatus()
{
    return m_status;
}

// Sets the status (online/offline) of this contact.
//
void CustomListItem::SetStatus(ContactStatus status)
{
    m_status = status;
}

// Gets the name of the contact.
//
WCHAR* CustomListItem::GetName()
{
    return m_name;
}

// Gets the Id of the contact.
//
int CustomListItem::GetId()
{
    return m_Id;
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


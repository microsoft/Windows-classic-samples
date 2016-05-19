
/*************************************************************************************************
 * Description: Implementation of the ListItemProvider class, which implements a 
 * UI Automation provider for a list item in a custom control.
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

#define INITGUID
#include "UIAProviders.h"

extern UiaIds AutoIds;

ListItemProvider::ListItemProvider(CustomListItem* pControl): m_refCount(1)
{
    m_pListItemControl = pControl;
    m_pListControl = pControl->GetOwnerList();
}

ListItemProvider::~ListItemProvider()
{
}


// IUnknown implementation.

IFACEMETHODIMP_(ULONG) ListItemProvider::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) ListItemProvider::Release()
{
    long val = InterlockedDecrement(&m_refCount);
    if (val == 0)
    {
        delete this;
    }
    return val;
}

IFACEMETHODIMP ListItemProvider::QueryInterface(REFIID riid, void** ppInterface)
{
    if(riid == __uuidof(IUnknown))                              *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    else if(riid == __uuidof(IRawElementProviderSimple))        *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    else if(riid == __uuidof(IRawElementProviderFragment))      *ppInterface =static_cast<IRawElementProviderFragment*>(this);
    else if(riid == __uuidof(ISelectionItemProvider))           *ppInterface =static_cast<ISelectionItemProvider*>(this);
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }
    (static_cast<IUnknown*>(*ppInterface))->AddRef();
    return S_OK;
}


// IRawElementProviderSimple implementation
//
// Implementation of IRawElementProviderSimple::GetProviderOptions.
//
IFACEMETHODIMP ListItemProvider::get_ProviderOptions(ProviderOptions* pRetVal)
{
    *pRetVal = ProviderOptions_ServerSideProvider;
    return S_OK;
}

// Implementation of IRawElementProviderSimple::GetPatternProvider.
// Gets the object that supports the specified pattern.
//
IFACEMETHODIMP ListItemProvider::GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal)
{
    if (patternId == AutoIds.SelectionItemPattern)
    {
        *pRetVal =static_cast<IRawElementProviderSimple*>(this);
        AddRef();
    }
    else
    {
        *pRetVal = NULL;   
    }
    return S_OK;
}
// Implementation of IRawElementProviderSimple::GetPropertyValue.
// Gets custom properties. Because list items are not directly hosted in an HWND, 
// more properties should be supported here than for the list box itself. 
//
IFACEMETHODIMP ListItemProvider::GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal)
{
    if (propertyId == AutoIds.AutomationIdProperty)
    {
        pRetVal->vt = VT_BSTR;
        int Id = m_pListItemControl->GetId();
        // Convert int to BSTR.
        WCHAR idString[3];
        swprintf_s(idString, 3, L"%d", Id);
        pRetVal->bstrVal = SysAllocString(idString);
    }
    else if (propertyId == AutoIds.NameProperty)
    {
        pRetVal->vt = VT_BSTR;
        pRetVal->bstrVal = SysAllocString(m_pListItemControl->GetName());
    }
    else if (propertyId == AutoIds.ControlTypeProperty)
    {
        pRetVal->vt = VT_I4;
        pRetVal->lVal = AutoIds.ListItemControlType;
    }
    // HasKeyboardFocus is true if the list has focus, and this item is selected.
    else if(propertyId == AutoIds.HasKeyboardFocusProperty)
    {
        int myIndex = m_pListItemControl->GetItemIndex();
        BOOL hasFocus = ((m_pListControl->GetSelectedIndex() == myIndex) 
            && (m_pListControl->GetIsFocused() == true));
        pRetVal->vt = VT_BOOL;
        pRetVal->boolVal = hasFocus ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (propertyId == AutoIds.IsControlElementProperty)
    {
        pRetVal->vt = VT_BOOL;
        pRetVal->boolVal = VARIANT_TRUE;
    }
    else if (propertyId == AutoIds.IsContentElementProperty)
    {
        pRetVal->vt = VT_BOOL;
        pRetVal->boolVal = VARIANT_TRUE;
    }
    else if (propertyId == AutoIds.IsKeyboardFocusableProperty)
    {
        pRetVal->vt = VT_BOOL;
        pRetVal->boolVal = VARIANT_TRUE;  
    }
    else if (propertyId == AutoIds.ItemStatusProperty)  
    {
        pRetVal->vt = VT_BSTR;
        if (m_pListItemControl->GetStatus() == Status_Online)
        {
            pRetVal->bstrVal = SysAllocString(L"Online");
        }
        else
        {
            pRetVal->bstrVal = SysAllocString(L"Offline");
        }
    }
    else
    {
        pRetVal->vt = VT_EMPTY;
    }
    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_HostRawElementProvider.
// Gets the UI Automation provider for the host window. 
// Return NULL. because the list items are not directly hosted in a window.
//
IFACEMETHODIMP ListItemProvider::get_HostRawElementProvider(IRawElementProviderSimple** pRetVal)
{
    *pRetVal = NULL; 
    return S_OK; 
}

// IRawElementProviderFragment implementation.
//
// Implementation of IRawElementProviderFragment::Navigate.
// Enables UI Automation to locate the element in the tree.
//
IFACEMETHODIMP ListItemProvider::Navigate(NavigateDirection direction, IRawElementProviderFragment ** pRetVal)
{
    IRawElementProviderFragment* pFrag = NULL;
    switch(direction)
    {
    case NavigateDirection_Parent:
        pFrag = m_pListControl->GetListProvider(); 
        break;

    case NavigateDirection_NextSibling:
        {
            int myIndex = m_pListItemControl->GetItemIndex();
            if (myIndex == m_pListControl->GetCount() - 1)
            {
                pFrag = NULL;
                break;
            }
            LISTITERATOR nextIter = m_pListControl->GetItemAt(myIndex + 1);
            CustomListItem* pNext = (CustomListItem*)(*nextIter);
            pFrag = pNext->GetListItemProvider();
            break;
        }

    case NavigateDirection_PreviousSibling:  
        {
            int myIndex = m_pListItemControl->GetItemIndex();
            if (myIndex <= 0) 
            {
                pFrag = NULL;
                break;
            }
            LISTITERATOR nextIter = m_pListControl->GetItemAt(myIndex - 1);
            CustomListItem* pPrev = static_cast<CustomListItem*>(*nextIter);
            pFrag = pPrev->GetListItemProvider();
            break;
        }
    }
    *pRetVal = pFrag;
    if (pFrag != NULL) 
    {   
        pFrag->AddRef();
    }
    return S_OK;
}

// Implementation of IRawElementProviderFragment::GetRuntimeId.
// Gets the runtime identifier. This is an array consisting of UiaAppendRuntimeId, 
// which makes the ID unique among instances of the control, and the Automation Id.
//
IFACEMETHODIMP ListItemProvider::GetRuntimeId(SAFEARRAY ** pRetVal)
{
    int id = m_pListItemControl->GetId();
    int rId[] = { UiaAppendRuntimeId, id };

    SAFEARRAY *psa = SafeArrayCreateVector(VT_I4, 0, 2);
    for (LONG i = 0; i < 2; i++)
    {
        SafeArrayPutElement(psa, &i, &(rId[i]));
    }
    *pRetVal = psa;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::get_BoundingRectangle.
// Gets the bounding rectangle of the item, in screen coordinates.
//
IFACEMETHODIMP ListItemProvider::get_BoundingRectangle(UiaRect * pRetVal)
{
    IRawElementProviderFragment* pParent = m_pListControl->GetListProvider();
    int myIndex = m_pListItemControl->GetItemIndex();
    UiaRect parentRect;

    HRESULT hr = pParent->get_BoundingRectangle(&parentRect);
    if (SUCCEEDED(hr))
    {
        pRetVal->left = parentRect.left;
        pRetVal->top = parentRect.top + (m_pListControl->ItemHeight * myIndex);
        pRetVal->width = parentRect.width;
        pRetVal->height = m_pListControl->ItemHeight;
    }
    return hr;
}


// Implementation of IRawElementProviderFragment::GetEmbeddedFragmentRoots.
// Retrieves any fragment roots that may be hosted in this element.
//
IFACEMETHODIMP ListItemProvider::GetEmbeddedFragmentRoots(SAFEARRAY ** pRetVal)
{
    *pRetVal = NULL;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::SetFocus.
// Responds to the control receiving focus through a UI Automation request.
//
IFACEMETHODIMP ListItemProvider::SetFocus()
{
    Select();
    return S_OK;
}

// Implementation of IRawElementProviderFragment::get_FragmentRoot.
// Retrieves the root element of this fragment.
//
IFACEMETHODIMP ListItemProvider::get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal)
{
    IRawElementProviderFragmentRoot* pRoot = this->m_pListControl->GetListProvider();
    if (pRoot == NULL)
    {
        return E_FAIL;
    }
    pRoot->AddRef();
    *pRetVal = pRoot;
    return S_OK;
}


// ISelectionItemProvider implementation.

// Implementation of ISelectionItemProvider::Select.
// Responds to a request by UI Automation to select the item.
//
IFACEMETHODIMP ListItemProvider::Select()
{
    int index = m_pListItemControl->GetItemIndex();
    m_pListControl->SelectItem(index);

    // Force refresh even when app doesn't have focus.
    InvalidateRect(m_pListControl->GetHwnd(), NULL, false); 
    return S_OK;
}

// Implementation of ISelectionItemProvider::AddToSelection.
// Responds to a request by UI Automation to add the item to the selection.
// Because this is a single-selection list box, the call is equivalent to Select().
//
IFACEMETHODIMP ListItemProvider::AddToSelection()
{
    Select();
    return S_OK;
}

// Implementation of ISelectionItemProvider::RemoveFromSelection.
// Responds to a request by UI Automation to remove the item from the selection.
// One and only one item must always be selected, so this is not implemented.
//
IFACEMETHODIMP ListItemProvider::RemoveFromSelection()
{
    return UIA_E_INVALIDOPERATION;
}

// Implementation of ISelectionItemProvider::get_IsSelected.
// Advises whether the item is selected.
//
IFACEMETHODIMP ListItemProvider::get_IsSelected(BOOL *pRetVal)
{
    *pRetVal = 
        (m_pListItemControl->GetItemIndex() == m_pListControl->GetSelectedIndex());
    return S_OK;
}

// Implementation of ISelectionItemProvider::get_SelectionContainer.
// Returns the UI Automation provider for the list control.
//
IFACEMETHODIMP ListItemProvider::get_SelectionContainer(
    IRawElementProviderSimple **pRetVal)
{
    IRawElementProviderSimple* pParent = 
        static_cast<IRawElementProviderSimple*>(m_pListControl->GetListProvider());
    pParent->AddRef();
    *pRetVal = pParent;
    return S_OK;
}

// Raises an event when an item is added to the list.
//
void ListItemProvider::NotifyItemAdded()
{
    if (UiaClientsAreListening())
    {
        UiaRaiseStructureChangedEvent(this, StructureChangeType_ChildAdded, NULL, 0);
    }
}

// Raises an event when an item is removed from the list.
//
// StructureType_ChildRemoved is unusual in that it is raised on the parent provider,
// since the child provider may not exist anymore, but it uses the child's runtime ID.
void ListItemProvider::NotifyItemRemoved()
{
    if (UiaClientsAreListening())
    {
        ListProvider* listProvider = m_pListControl->GetListProvider();

        // Construct the partial runtime ID for the removed child
        int id = m_pListItemControl->GetId();
        int rId[] = { UiaAppendRuntimeId, id };

        UiaRaiseStructureChangedEvent(listProvider, StructureChangeType_ChildRemoved, rId, ARRAYSIZE(rId));
    }
}

// Raises an event when a list item is selected.
//
void ListItemProvider::NotifyElementSelected()
{
    if (UiaClientsAreListening())
    {
        UiaRaiseAutomationEvent(this, AutoIds.ElementSelectedEvent);
    }
}



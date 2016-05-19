
/*************************************************************************************************
 * Description: Implementation of the ListProvider class, which implements which implements a 
 * UI Automation provider for a custom list control.
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

#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <ole2.h>
#include "UIAProviders.h"
#include "CustomControl.h"

HFONT GetFont(LONG ht);
UiaIds AutoIds;

ListProvider::ListProvider(CustomListControl* pControl): 
    m_refCount(1), m_pControl(pControl)
{
    m_controlHwnd = pControl->GetHwnd();

    // Initialize identifiers from GUIDs.
    InitIds();
}

ListProvider::~ListProvider()
{
}


// Retrieves the UI Automation provider for a list item.
//
ListItemProvider* ListProvider::GetItemProviderByIndex(int index)
{
    if ((index < 0) || (index >= m_pControl->GetCount()))
    {
        return NULL;
    }
    LISTITERATOR iter = m_pControl->GetItemAt(index);
    CustomListItem* pItem = static_cast<CustomListItem*>(*iter);
    if (pItem == NULL)
    {
        return NULL;
    }
    return pItem->GetListItemProvider();  
}

// Looks up identifiers. To use UiaLookupId, you must link to UIAutomationcore.lib.
//
void ListProvider::InitIds()
{
    static bool inited = false;
    if (!inited)
    {
        inited = true;
        AutoIds.LocalizedControlTypeProperty = UiaLookupId(AutomationIdentifierType_Property, &LocalizedControlType_Property_GUID);
        AutoIds.AutomationIdProperty = UiaLookupId(AutomationIdentifierType_Property, &AutomationId_Property_GUID);
        AutoIds.NameProperty = UiaLookupId(AutomationIdentifierType_Property, &Name_Property_GUID);
        AutoIds.HasKeyboardFocusProperty = UiaLookupId(AutomationIdentifierType_Property, &HasKeyboardFocus_Property_GUID);
        AutoIds.IsControlElementProperty = UiaLookupId(AutomationIdentifierType_Property, &IsControlElement_Property_GUID);
        AutoIds.IsContentElementProperty = UiaLookupId(AutomationIdentifierType_Property, &IsContentElement_Property_GUID);
        AutoIds.IsKeyboardFocusableProperty = UiaLookupId(AutomationIdentifierType_Property, &IsKeyboardFocusable_Property_GUID);
        AutoIds.ItemStatusProperty = UiaLookupId(AutomationIdentifierType_Property, &ItemStatus_Property_GUID);

        AutoIds.ControlTypeProperty = UiaLookupId(AutomationIdentifierType_Property, &ControlType_Property_GUID);
        AutoIds.SelectionPattern = UiaLookupId(AutomationIdentifierType_Pattern, &Selection_Pattern_GUID);
        AutoIds.SelectionItemPattern = UiaLookupId(AutomationIdentifierType_Pattern, &SelectionItem_Pattern_GUID);
        AutoIds.ListControlType = UiaLookupId(AutomationIdentifierType_ControlType, &List_Control_GUID);
        AutoIds.ListItemControlType = UiaLookupId(AutomationIdentifierType_ControlType, &ListItem_Control_GUID);
        AutoIds.ElementSelectedEvent = UiaLookupId(AutomationIdentifierType_Event, &SelectionItem_ElementSelectedEvent_Event_GUID);
    }
}

// Raises an event when a list item is selected.
//

// IUnknown implementation.
//
IFACEMETHODIMP_(ULONG) ListProvider::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) ListProvider::Release()
{
    long val = InterlockedDecrement(&m_refCount);
    if (val == 0)
    {
        delete this;
    }
    return val;
}

IFACEMETHODIMP ListProvider::QueryInterface(REFIID riid, void** ppInterface)
{
    if(riid == __uuidof(IUnknown))                              *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    else if(riid == __uuidof(IRawElementProviderSimple))        *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    else if(riid == __uuidof(IRawElementProviderFragment))      *ppInterface =static_cast<IRawElementProviderFragment*>(this);
    else if(riid == __uuidof(IRawElementProviderFragmentRoot))  *ppInterface =static_cast<IRawElementProviderFragmentRoot*>(this);
    else if(riid == __uuidof(ISelectionProvider))               *ppInterface =static_cast<ISelectionProvider*>(this);
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
// Implementation of IRawElementProviderSimple::get_ProviderOptions.
// Gets UI Automation provider options.
//
IFACEMETHODIMP ListProvider::get_ProviderOptions(ProviderOptions* pRetVal)
{
    *pRetVal = ProviderOptions_ServerSideProvider;
    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_PatternProvider.
// Gets the object that supports ISelectionPattern.
//
IFACEMETHODIMP ListProvider::GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal)
{
    *pRetVal = NULL;
    if (patternId == AutoIds.SelectionPattern)
    {
        *pRetVal =static_cast<IRawElementProviderSimple*>(this);
        AddRef();  
    }
    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_PropertyValue.
// Gets custom properties.
//
IFACEMETHODIMP ListProvider::GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal)
{
    // Although it is hard-coded for the purposes of this sample, localizable 
    // text should be stored in, and loaded from, the resource file (.rc). 
    if (propertyId == AutoIds.LocalizedControlTypeProperty)
    {
        pRetVal->vt = VT_BSTR;
        pRetVal->bstrVal = SysAllocString(L"contact list");
    }
    else if (propertyId == AutoIds.ControlTypeProperty)
    {
        pRetVal->vt = VT_I4;
        pRetVal->lVal = AutoIds.ListControlType;
    }
    else if (propertyId == AutoIds.IsKeyboardFocusableProperty)
    {
        pRetVal->vt = VT_BOOL;
        pRetVal->boolVal = VARIANT_TRUE;
    }
    // else pRetVal is empty, and UI Automation will attempt to get the property from
    //  the HostRawElementProvider, which is the default provider for the HWND.
    // Note that the Name property comes from the Caption property of the control window, 
    //  if it has one.
    else
    {
        pRetVal->vt = VT_EMPTY;
    }
    return S_OK;
}

// Implementation of IRawElementProviderSimple::get_HostRawElementProvider.
// Gets the default UI Automation provider for the host window. This provider 
// supplies many properties.
//
IFACEMETHODIMP ListProvider::get_HostRawElementProvider(IRawElementProviderSimple** pRetVal)
{
    if (m_controlHwnd == NULL)
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    HRESULT hr = UiaHostProviderFromHwnd(m_controlHwnd, pRetVal); 
    return hr;
}


// IRawElementProviderFragment implementation
//
// Implementation of IRawElementProviderFragment::Navigate.
// Enables UI Automation to locate the element in the tree.
// Navigation to the parent is handled by the host window provider.
//
IFACEMETHODIMP ListProvider::Navigate(NavigateDirection direction, IRawElementProviderFragment** pRetVal)
{
    CustomListControl* pListControl = this->m_pControl;
    CustomListItem* pDest = NULL;
    IRawElementProviderFragment* pFrag = NULL;
    LISTITERATOR iter;
    switch(direction)
    {
      case NavigateDirection_FirstChild:  
          iter = pListControl->GetItemAt(0);
          pDest = (CustomListItem*)(*iter);
          pFrag = pDest->GetListItemProvider();
          break;
      case NavigateDirection_LastChild:  
          iter = pListControl->GetItemAt(pListControl->GetCount()-1);
          pDest = (CustomListItem*)(*iter);
          pFrag = pDest->GetListItemProvider();
          break;
    }
    if (pFrag != NULL) 
    {
        pFrag->AddRef();   
    }
    *pRetVal = pFrag;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::GetRuntimeId.
// UI Automation gets this value from the host window provider, so supply NULL here.
//
IFACEMETHODIMP ListProvider::GetRuntimeId(SAFEARRAY** pRetVal)
{
    *pRetVal = NULL;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::get_BoundingRectangle.
//
// Retrieves the screen location and size of the control. Controls hosted in
// Win32 windows can return an empty rectangle; UI Automation will
// retrieve the rectangle from the HWND provider. However, the method is
// implemented here so that it can be used by the list items to calculate
// their own bounding rectangles.
//
// UI Spy uses the bounding rectangle to draw a red border around the element.
//
IFACEMETHODIMP ListProvider::get_BoundingRectangle(UiaRect* pRetVal)
{
    RECT rect;
    GetClientRect(m_controlHwnd, &rect);
    InflateRect(&rect, -2, -2);
    POINT upperLeft;
    upperLeft.x = rect.left;  
    upperLeft.y = rect.top;
    ClientToScreen(m_controlHwnd, &upperLeft);

    pRetVal->left = upperLeft.x;
    pRetVal->top = upperLeft.y;
    pRetVal->width = rect.right - rect.left;
    pRetVal->height = rect.bottom - rect.top;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::GetEmbeddedFragmentRoots.
// Retrieves other fragment roots that may be hosted in this one.
//
IFACEMETHODIMP ListProvider::GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal)
{
    *pRetVal = NULL;
    return S_OK;
}

// Implementation of IRawElementProviderFragment::SetFocus.
// Responds to the control receiving focus through a UI Automation request.
// For HWND-based controls, this is handled by the host window provider.
//
IFACEMETHODIMP ListProvider::SetFocus()
{
    return S_OK;
}

// Implementation of IRawElementProviderFragment::get_FragmentRoot.
// Retrieves the root element of this fragment.
//
IFACEMETHODIMP ListProvider::get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal)
{
    *pRetVal = static_cast<IRawElementProviderFragmentRoot*>(this);
    AddRef();  
    return S_OK;
}

// IRawElementProviderFragmentRoot implementation
//
// Implementation of IRawElementProviderFragmentRoot::ElementProviderFromPoint.
// Retrieves the IRawElementProviderFragment interface for the item at the specified 
// point (in client coordinates).
// UI Spy uses this to determine what element is under the cursor when Ctrl is pressed.
//
IFACEMETHODIMP ListProvider::ElementProviderFromPoint(double x, double y, IRawElementProviderFragment** pRetVal)
{
    POINT pt;
    pt.x = (LONG)x;
    pt.y = (LONG)y;
    ScreenToClient(m_controlHwnd, &pt);
    int itemIndex = m_pControl->IndexFromY(pt.y);
    ListItemProvider* pItem = GetItemProviderByIndex(itemIndex);  
    if (pItem != NULL)
    {
        *pRetVal = static_cast<IRawElementProviderFragment*>(pItem);
        pItem->AddRef();
    }
    else 
    {
        *pRetVal = NULL;
    }
    return S_OK;
}

// Implementation of IRawElementProviderFragmentRoot::GetFocus.
// Retrieves the provider for the list item that is selected when the control gets focus.
//
IFACEMETHODIMP ListProvider::GetFocus(IRawElementProviderFragment** pRetVal)
{
    *pRetVal = NULL;
    ListItemProvider* pItem = GetItemProviderByIndex(m_pControl->GetSelectedIndex()); 
    if (pItem != NULL)
    {
        pItem->AddRef();
        *pRetVal = pItem;
    }
    return S_OK;
}


// ISelectionProvider implementation
//
// Implementation of ISelectionProvider::GetSelection.
// Gets the provider(s) for the items(s) selected in the list box. 
// In this case, only a single item can be selected.
//
IFACEMETHODIMP ListProvider::GetSelection(SAFEARRAY** pRetVal)
{
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UNKNOWN, 0, 1);
    int index = m_pControl->GetSelectedIndex(); 
    ListItemProvider* pItem = GetItemProviderByIndex(index); 
    if (pItem != NULL)
    {
        LONG i = 0;
        SafeArrayPutElement(psa, &i, pItem);
    }
    *pRetVal = psa;
    return S_OK;
}

// Implementation of ISelectionProvider::get_CanSelectMultiple.
//
IFACEMETHODIMP ListProvider::get_CanSelectMultiple(BOOL *pRetVal)
{
    *pRetVal = FALSE;
    return S_OK;
}
// Implementation of ISelectionProvider::get_IsSelectionRequired.
//
IFACEMETHODIMP ListProvider::get_IsSelectionRequired(BOOL *pRetVal)
{
   *pRetVal = TRUE;
   return S_OK;
}


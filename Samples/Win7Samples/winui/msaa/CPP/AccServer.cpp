/*************************************************************************************************
* Description: Implementation of the accessible object.
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
#include "AccServer.h"

AccServer::AccServer(HWND hwnd, CustomListControl* pOwnerControl):
    m_pControl(pOwnerControl), m_hwnd(hwnd), m_refCount(1), m_enumCount(0)
{
    // Create a standard window-based IAccessible object to handle default actions.
    CreateStdAccessibleObject(hwnd, OBJID_CLIENT, IID_PPV_ARGS(&m_pStdAccessibleObject));
}

AccServer::~AccServer()
{
    if (m_pStdAccessibleObject != NULL)
    {
        m_pStdAccessibleObject->Release();
    }
}

// Set the state of the control.
//
void AccServer::SetControlIsAlive(bool alive)
{
    m_controlIsAlive = alive;
}

// IUnknown methods.
//
IFACEMETHODIMP_(ULONG) AccServer::AddRef()
{
    return ++m_refCount;
}

IFACEMETHODIMP_(ULONG) AccServer::Release()
{
    if (--m_refCount <= 0)
    {
        delete this;
        return 0;             
    }
    return m_refCount;
}

IFACEMETHODIMP AccServer::QueryInterface(REFIID riid, void** ppInterface)
{
    if (riid == __uuidof(IUnknown))   
    {
        *ppInterface = static_cast<IUnknown*>(static_cast<IAccessible*>(this));
    }
    else if (riid == __uuidof(IAccessible)) 
    {
        *ppInterface = static_cast<IAccessible*>(this);
    }
    else if (riid == __uuidof(IDispatch))   
    {
        *ppInterface = static_cast<IDispatch*>(this);
    }
    else if (riid == __uuidof(IEnumVARIANT))    
    {
        *ppInterface = static_cast<IEnumVARIANT*>(this);
    }
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }
    (static_cast<IUnknown*>(*ppInterface))->AddRef();
    return S_OK;
  }


// IDispatch methods.
// Under Oleacc.dll v. 2, these don't have to be implemented.
// However, COM requires that any out parameters be cleared.

IFACEMETHODIMP AccServer::GetTypeInfoCount(UINT* pctinfo)
{
    *pctinfo = 0;
    return E_NOTIMPL;
}

IFACEMETHODIMP AccServer::GetTypeInfo(UINT /*itinfo*/, LCID /*lcid*/, ITypeInfo** pptinfo)
{
    *pptinfo = NULL;
    return E_NOTIMPL;
}

IFACEMETHODIMP AccServer::GetIDsOfNames(REFIID /*riid*/, OLECHAR** rgszNames, UINT /*cNames*/,
                                                   LCID /*lcid*/, DISPID* rgdispid)
{
    *rgszNames = NULL;
    *rgdispid = 0;
    return E_NOTIMPL;
}

IFACEMETHODIMP AccServer::Invoke(DISPID /*dispidMember*/, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/,
                                            DISPPARAMS* pdispparams, VARIANT* pvarResult,
                                            EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    pdispparams = NULL;
    pvarResult->vt = VT_EMPTY;
    pexcepinfo = NULL;
    puArgErr = NULL;
    return E_NOTIMPL;
}


// IEnumVARIANT methods

IFACEMETHODIMP AccServer::Next( 
        ULONG celt,          // Number of elements to return.
        VARIANT *rgVar,      // Array of returned elements.
        ULONG *pCeltFetched) // Number actually returned.   
{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    ULONG childCount = static_cast<ULONG>(m_pControl->GetCount());
    if (pCeltFetched != NULL)
    {
        *pCeltFetched = 0;
    }
    if (!rgVar) 
    {
        return E_INVALIDARG;
    }
    ULONG fetched = 0;
    for (ULONG x = 0; x < celt; x++)
    {
        if (++m_enumCount <= childCount)
        {
            rgVar[x].vt = VT_I4;
            // Return the index, because in this example the list members always have sequential
            // child IDs. In most cases, the value would be a nonsequential ID or an IDispatch pointer.
            rgVar[x].lVal = m_enumCount;
            fetched++;
        }
        else
        {
            break;
        }
    }
    if (pCeltFetched != NULL)
    {
        *pCeltFetched = fetched;
    }

    //
    // Return S_FALSE if we grabbed fewer items than requested
    //
    return((fetched < celt) ? S_FALSE : S_OK);
}

    
IFACEMETHODIMP AccServer::Skip(ULONG celt)
{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    ULONG childCount = static_cast<ULONG>(m_pControl->GetCount());

    m_enumCount += celt;
    if (m_enumCount > childCount)
    {
        m_enumCount = childCount;
    }

    //
    // We return S_FALSE if at the end
    //
    return((m_enumCount >= childCount) ? S_FALSE : S_OK);
}
    
IFACEMETHODIMP AccServer::Reset()
{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    m_enumCount = 0;
    return S_OK;
}
    
IFACEMETHODIMP AccServer::Clone(IEnumVARIANT **ppEnum)
{
    *ppEnum = NULL;
    AccServer* pAcc = new (std::nothrow) AccServer(m_hwnd, m_pControl);
    HRESULT hr = (pAcc != NULL) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        pAcc->m_enumCount = m_enumCount;
        *ppEnum = pAcc;
    }
    return hr;
}
        
// IAccessible methods.

// Gets the parent object.
//
IFACEMETHODIMP AccServer::get_accParent( 
    IDispatch **ppdispParent)
{
    *ppdispParent = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return m_pStdAccessibleObject->get_accParent(ppdispParent);
}

// Gets the count of child objects or elements.
//
IFACEMETHODIMP AccServer::get_accChildCount( 
    long *pcountChildren)
{
    *pcountChildren = 0;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    *pcountChildren = m_pControl->GetCount(); 
    return S_OK;
}


// Gets a child object. Because the list items in this control are elements,
// not objects, returns S_FALSE.
//
IFACEMETHODIMP AccServer::get_accChild( 
    VARIANT varChild,
    IDispatch **ppdispChild)
{
    *ppdispChild = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    return S_FALSE;     
}

// Get the name of the control or one of its children.

IFACEMETHODIMP AccServer::get_accName( 
    VARIANT varChild,
    BSTR *pszName)

{
    *pszName = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        *pszName = NULL;
        return E_INVALIDARG;
    }
    // For the control itself, let the standard accessible object return the name
    // assigned by the application. This is either the "caption" property or, if
    // there is no caption, the text of any label.
    if (varChild.lVal == CHILDID_SELF)
    {
        return m_pStdAccessibleObject->get_accName(varChild, pszName);          
    }
    else
    {
        LISTITERATOR item = m_pControl->GetItemAt(varChild.lVal - 1);
        CustomListControlItem* pItem = static_cast<CustomListControlItem*>(*item);
        *pszName = SysAllocString(pItem->GetName());
        if (!pszName)
        {
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}

// Get the value of the control or one of its children.
// Not implemented for a list box.

IFACEMETHODIMP AccServer::get_accValue( 
    VARIANT /*varChild*/,
    BSTR *pszValue)
{
    *pszValue = NULL;   
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return DISP_E_MEMBERNOTFOUND;
}


// Get the description of the control or one of its children.
// Note that the descriptive strings given here are not typical;
// see Description Property in the documentation for information
// about when this property should be supported.

IFACEMETHODIMP AccServer::get_accDescription( 
    VARIANT varChild,
    BSTR *pszDescription)
{
    *pszDescription = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    if (varChild.lVal == CHILDID_SELF)
    {
        *pszDescription = SysAllocString(L"List of contacts.");         
    }
    else
    {
        *pszDescription = SysAllocString(L"A contact.");            
    }
    return S_OK;
}


// Get the role of the control or one of its children.

IFACEMETHODIMP AccServer::get_accRole( 
    VARIANT varChild,
    VARIANT *pvarRole)
{
    pvarRole->vt = VT_EMPTY;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        pvarRole->vt = VT_EMPTY;
        return E_INVALIDARG;
    }
    pvarRole->vt = VT_I4;
    if (varChild.lVal == CHILDID_SELF)
    {
        pvarRole->lVal = ROLE_SYSTEM_LIST;
    }
    else
    {
        pvarRole->lVal = ROLE_SYSTEM_LISTITEM;
    }
    return S_OK;
}


// Gets the state of the control or one of its children.

IFACEMETHODIMP AccServer::get_accState( 
    VARIANT varChild,
    VARIANT *pvarState)
{
    pvarState->vt = VT_EMPTY;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        pvarState->vt = VT_EMPTY;
        return E_INVALIDARG;
    }
    if (varChild.lVal == CHILDID_SELF)
    {
        return m_pStdAccessibleObject->get_accState(varChild, pvarState);
    }
    else  // For list items.
    {
        DWORD flags = STATE_SYSTEM_SELECTABLE | STATE_SYSTEM_FOCUSABLE;
        int index = static_cast<int>(varChild.lVal - 1);
        if (index == m_pControl->GetSelectedIndex())
        {
            flags |= STATE_SYSTEM_SELECTED;
            if (GetFocus() == m_hwnd)
            {
                flags |= STATE_SYSTEM_FOCUSED;
            }
        }
        pvarState->vt = VT_I4;
        pvarState->lVal = flags; 
    }
    return S_OK;
}

// Get a help string for the control or one of its children.
// For simplicity, the string is not localized.

IFACEMETHODIMP AccServer::get_accHelp( 
    VARIANT varChild,
    BSTR *pszHelp)
{
    *pszHelp = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        *pszHelp = NULL;
        return E_INVALIDARG;
    }
    if (varChild.lVal == CHILDID_SELF)
    {
        *pszHelp = SysAllocString(L"Contact list.");
    }
    else
    {
        int index = static_cast<int>(varChild.lVal - 1);
        LISTITERATOR item = m_pControl->GetItemAt(index);
        CustomListControlItem* pItem = (CustomListControlItem*)(*item);
        if (pItem->GetStatus() == Status_Online)
        {
            *pszHelp = SysAllocString(L"Online contact.");
        }
        else 
        {
            *pszHelp = SysAllocString(L"Offline contact.");
        }
    }
    return S_OK;
}

// Get a help file for the control or one of its children.
//
IFACEMETHODIMP AccServer::get_accHelpTopic( 
    BSTR *pszHelpFile,
    VARIANT /*varChild*/,
    long * /*pidTopic*/)
{
    *pszHelpFile = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return S_FALSE;
}

// Get a keyboard shortcut for the control.
//
IFACEMETHODIMP AccServer::get_accKeyboardShortcut( 
    VARIANT varChild,
    BSTR *pszKeyboardShortcut)
{
    *pszKeyboardShortcut = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return m_pStdAccessibleObject->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}


// Get the element that has the keyboard focus.

IFACEMETHODIMP AccServer::get_accFocus(VARIANT *pvarChild)
{
    pvarChild->vt = VT_EMPTY;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    HRESULT hr = m_pStdAccessibleObject->get_accFocus(pvarChild); 
    // If the HWND does not have the focus, the variant type is set to VT_EMPTY.
    if ((pvarChild->vt != VT_I4) || (FAILED(hr)))
    {
        return hr;
    }
    else
    {
        int index = m_pControl->GetSelectedIndex();
        if (index < 0)
        {
            pvarChild->lVal = CHILDID_SELF;
        }
        else
        {
            // Convert to 1-based index for child ID.
            pvarChild->lVal = index + 1;
        }
    }
    return S_OK;
}



// Get the index of the selected child. 
//
IFACEMETHODIMP AccServer::get_accSelection(VARIANT *pvarChildren)
{
    pvarChildren->vt = VT_EMPTY;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    int childID = m_pControl->GetSelectedIndex() + 1; // Convert from 0-based.
    if (childID <= 0)
    {
        pvarChildren->vt = VT_EMPTY;
    }
    else 
    {
        pvarChildren->vt = VT_I4;
        pvarChildren->lVal = childID;
    }
    return S_OK;
}

// Get a description of the default action.

IFACEMETHODIMP AccServer::get_accDefaultAction( 
    VARIANT varChild,
    BSTR *pszDefaultAction)
{
    *pszDefaultAction = NULL;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        *pszDefaultAction = NULL;
        return E_INVALIDARG;
    }
    if (varChild.lVal == CHILDID_SELF)
    {
        *pszDefaultAction = NULL;
        return DISP_E_MEMBERNOTFOUND;
    }
    else
    {
        *pszDefaultAction = SysAllocString(L"Double-click");
    }
    return S_OK;
}

// Select an item. Allow only single selection.

IFACEMETHODIMP AccServer::accSelect( 
    long flagsSelect, VARIANT varChild)
{
    // Check parameters. We don't support the following:
    // SELFLAG_NONE
    // SELFLAG_ADDSELECTION
    // SELFLAG_REMOVESELECTION
    // SELFLAG_EXTENDSELECTION
    // SELFLAG_VALID
    DWORD allowedFlags = SELFLAG_TAKEFOCUS | SELFLAG_TAKESELECTION;
    if ((flagsSelect | allowedFlags) != allowedFlags)
    {
        return E_INVALIDARG;
    }
    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    // Move the focus to the list box.
    SetFocus(m_hwnd);

    // Move the selection if called on to do so.
    if (((flagsSelect & (SELFLAG_TAKESELECTION | SELFLAG_TAKEFOCUS)) != 0) 
        && (varChild.lVal != CHILDID_SELF))
    {
        int selection = static_cast<int>(varChild.lVal) - 1;
        m_pControl->SelectItem(selection);
    }
    return S_OK;
}

// Get the location of the control or the list item.

IFACEMETHODIMP AccServer::accLocation( 
    long *pxLeft,
    long *pyTop,
    long *pcxWidth,
    long *pcyHeight,
    VARIANT varChild)
{
    *pxLeft = 0;
    *pyTop = 0;
    *pcxWidth = 0;
    *pcyHeight = 0;
    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if (varChild.lVal == CHILDID_SELF)
    {
        return m_pStdAccessibleObject->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
    }
    else
    {
        RECT rect;
        if (m_pControl->GetItemScreenRect(varChild.lVal - 1, &rect) == FALSE)
        {
            return E_INVALIDARG;
        }
        else
        {
            *pxLeft = rect.left;
            *pyTop = rect.top;
            *pcxWidth = rect.right - rect.left;
            *pcyHeight = rect.bottom - rect.top;
            return S_OK;    
        }
    }
}

// Navigate through the tree.

IFACEMETHODIMP AccServer::accNavigate( 
    long navDir,
    VARIANT varStart,
    VARIANT *pvarEndUpAt)
{
    // Default value.
    pvarEndUpAt->vt = VT_EMPTY;

    if ((varStart.vt != VT_I4) || (varStart.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    switch (navDir)
    {
    case NAVDIR_FIRSTCHILD:
        if ((varStart.lVal == CHILDID_SELF) && (m_pControl->GetCount() > 0))
        {
            pvarEndUpAt->vt = VT_I4;
            pvarEndUpAt->lVal = 1;
        }
        else  
        {
            return S_FALSE;
        }
        break;

    case NAVDIR_LASTCHILD:
        if ((varStart.lVal == CHILDID_SELF) && (m_pControl->GetCount() > 0))
        {
            pvarEndUpAt->vt = VT_I4;
            pvarEndUpAt->lVal = m_pControl->GetCount();
        }
        else    
        {
            return S_FALSE;
        }
        break;

    case NAVDIR_NEXT:   
    case NAVDIR_DOWN:
        if (varStart.lVal != CHILDID_SELF)
        {
            pvarEndUpAt->vt = VT_I4;
            pvarEndUpAt->lVal = varStart.lVal + 1;
            // Out of range.
            if (pvarEndUpAt->lVal > m_pControl->GetCount())
            {
                pvarEndUpAt->vt = VT_EMPTY;
                return S_FALSE;
            }
        }
        else  // Call through to method on standard container.
        {
            return m_pStdAccessibleObject->accNavigate(navDir, varStart, pvarEndUpAt);
        }
        break;

    case NAVDIR_PREVIOUS:
    case NAVDIR_UP:
        if (varStart.lVal != CHILDID_SELF)
        {
            pvarEndUpAt->vt = VT_I4;
            pvarEndUpAt->lVal = varStart.lVal - 1;
            // Out of range.
            if (pvarEndUpAt->lVal < 1)
            {
                pvarEndUpAt->vt = VT_EMPTY;
                return S_FALSE;
            }
        }
        else  // Call through to method on standard container.
        {
            return m_pStdAccessibleObject->accNavigate(navDir, varStart, pvarEndUpAt);
        }
        break;

        // Unsupported directions.
    case NAVDIR_LEFT:
    case NAVDIR_RIGHT:
        if (varStart.lVal == CHILDID_SELF)
        {
            return m_pStdAccessibleObject->accNavigate(navDir, varStart, pvarEndUpAt);
        }
        else 
        {
            pvarEndUpAt->vt = VT_EMPTY;
            return S_FALSE;
        }
        break;
    }
    return S_OK;
}

IFACEMETHODIMP AccServer::accHitTest( 
    long xLeft,
    long yTop,
    VARIANT *pvarChild) 

{
    pvarChild->vt = VT_EMPTY;
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    // Does the control window contain the point?
    // Note: don't use WindowFromPoint, as it may return a transparent window such as 
    // a group box.
    RECT controlRect;
    GetWindowRect(m_hwnd, &controlRect);
    BOOL inWindow = ((xLeft >= controlRect.left) && (xLeft <= controlRect.right)
        && (yTop >= controlRect.top) && (yTop <= controlRect.bottom));

    // Not in our window.
    if (!inWindow)
    {
        return S_FALSE;
    }
    else  // In our window; return list item, or self if in blank space.
    {
        pvarChild->vt = VT_I4;
        POINT pt;
        pt.x = xLeft;
        pt.y = yTop;
        ScreenToClient(m_hwnd, &pt);
        int index = m_pControl->IndexFromY(pt.y);
        if (index >= 0)
        {
            pvarChild->lVal = index + 1;
        }
        else
        {
            pvarChild->lVal = CHILDID_SELF;

        }
        return S_OK;
    }
}

IFACEMETHODIMP AccServer::accDoDefaultAction( 
    VARIANT varChild) 

{
    if ((varChild.vt != VT_I4) || (varChild.lVal > m_pControl->GetCount()))
    {
        return E_INVALIDARG;
    }
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    if (varChild.lVal != CHILDID_SELF)
    {
        // Because our sample action is to open a dialog box (thus blocking), 
        // do it indirectly. First select the item.
        if (SUCCEEDED(accSelect(SELFLAG_TAKESELECTION, varChild)))
        {
            PostMessage(m_hwnd, CUSTOMLB_DEFERDOUBLECLICK, 0, 0);
        }
    }
    return S_OK;
}

IFACEMETHODIMP AccServer::put_accName( 
    VARIANT /*varChild*/,
    BSTR /*szName*/) 

{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return E_NOTIMPL;
}

IFACEMETHODIMP AccServer::put_accValue( 
    VARIANT /*varChild*/,
    BSTR /*szValue*/) 

{
    if (!m_controlIsAlive) 
    { 
        return RPC_E_DISCONNECTED; 
    }

    return E_NOTIMPL;
}


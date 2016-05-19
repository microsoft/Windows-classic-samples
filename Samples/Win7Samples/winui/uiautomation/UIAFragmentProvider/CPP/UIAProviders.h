/*************************************************************************************************
* Description: Declarations for the sample UI Autoamtion provider implementations.
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

#include <UIAutomationCore.h>
#include <UIAutomationCoreAPI.h>
#include <assert.h>
#include "CustomControl.h"

class ListItemProvider;

// Various identifiers that have to be looked up.
typedef struct UiaIdentifiers
{
    PROPERTYID    LocalizedControlTypeProperty;
    PROPERTYID    AutomationIdProperty;
    PROPERTYID    NameProperty;
    PROPERTYID    ControlTypeProperty;
    PROPERTYID    HasKeyboardFocusProperty;
    PROPERTYID    IsControlElementProperty;
    PROPERTYID    IsContentElementProperty;
    PROPERTYID    IsKeyboardFocusableProperty;
    PROPERTYID    ItemStatusProperty;
    PATTERNID     SelectionPattern;
    PATTERNID     SelectionItemPattern;
    CONTROLTYPEID ListControlType;
    CONTROLTYPEID ListItemControlType;
    EVENTID       ElementSelectedEvent;
} UiaIds;

class ListProvider : public IRawElementProviderSimple, 
    public IRawElementProviderFragment, 
    public IRawElementProviderFragmentRoot, 
    public ISelectionProvider
{
public:

    // Constructor/destructor.
    ListProvider(CustomListControl* pControl);

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

    // IRawElementProviderSimple methods
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions * pRetVal);
    IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
    IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );
    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple ** pRetVal );

    // IRawElementProviderFragment methods
    IFACEMETHODIMP Navigate(NavigateDirection direction, IRawElementProviderFragment ** pRetVal );
    IFACEMETHODIMP GetRuntimeId(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP get_BoundingRectangle(UiaRect * pRetVal );
    IFACEMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP get_FragmentRoot( IRawElementProviderFragmentRoot * * pRetVal );

    // IRawElementProviderFragmenRoot methods
    IFACEMETHODIMP ElementProviderFromPoint(double x, double y, IRawElementProviderFragment ** pRetVal );
    IFACEMETHODIMP GetFocus(IRawElementProviderFragment ** pRetVal );

    // ISelectionProvider methods
    IFACEMETHODIMP GetSelection(SAFEARRAY * *pRetVal);
    IFACEMETHODIMP get_CanSelectMultiple(BOOL *pRetVal);
    IFACEMETHODIMP get_IsSelectionRequired(BOOL *pRetVal);

    // Various methods.
    void InitIds();
    ListItemProvider* GetItemProviderByIndex(int index);

private:
    virtual ~ListProvider();

    // Ref counter for this COM object.
    ULONG m_refCount;

    // Parent control.
    HWND m_controlHwnd;
    CustomListControl* m_pControl;
};

class ListItemProvider : public IRawElementProviderSimple, 
    public IRawElementProviderFragment, 
    public ISelectionItemProvider
{
public:

    // Constructor / destructor
    ListItemProvider(CustomListItem* pControl); 

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

    // IRawElementProviderSimple methods
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions * pRetVal);
    IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
    IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );
    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple ** pRetVal );

    // IRawElementProviderFragment methods
    IFACEMETHODIMP Navigate(NavigateDirection direction, IRawElementProviderFragment ** pRetVal );
    IFACEMETHODIMP GetRuntimeId(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP get_BoundingRectangle(UiaRect * pRetVal );
    IFACEMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP get_FragmentRoot( IRawElementProviderFragmentRoot * * pRetVal );

    // ISelectionItemProvider methods
    IFACEMETHODIMP Select();
    IFACEMETHODIMP AddToSelection();
    IFACEMETHODIMP RemoveFromSelection();
    IFACEMETHODIMP get_IsSelected(BOOL *pRetVal);
    IFACEMETHODIMP get_SelectionContainer(IRawElementProviderSimple **pRetVal);

    // Various methods
    void NotifyItemAdded();
    void NotifyItemRemoved();
    void NotifyElementSelected();

private:
    virtual ~ListItemProvider();

    // Ref Counter for this COM object
    ULONG m_refCount;

    // Pointers to the owning item control and list control.
    CustomListItem* m_pListItemControl;
    CustomListControl* m_pListControl;

};

/*************************************************************************************************
* Description: Declarations for the accessible object.
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
#include <oleacc.h>
#include "CustomControl.h"

class AccServer :
    public IAccessible, public IEnumVARIANT
{
private:
    ULONG               m_refCount;             // The COM reference count.
    IAccessible*        m_pStdAccessibleObject; // The standard server for the HWND.
    CustomListControl*  m_pControl;             // The control served by this instance.
    HWND                m_hwnd;                 // The control's HWND.
    bool                m_controlIsAlive;       // Flag for when control goes away.
    ULONG               m_enumCount;            // Current count for EnumVARIANT::Next.

    virtual ~AccServer();

public:
    AccServer(HWND, CustomListControl*);
    void SetControlIsAlive(bool alive);

    // IUnknown methods.
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

    // IDispatch methods.
    IFACEMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    IFACEMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    IFACEMETHODIMP GetIDsOfNames(REFIID riid, __in_ecount(cNames)
        OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    IFACEMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
        WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr);
    
    // IAccessible methods
    IFACEMETHODIMP get_accParent(IDispatch **ppdispParent);
    IFACEMETHODIMP get_accChildCount(long *pcountChildren);
    IFACEMETHODIMP get_accChild(VARIANT varChild, IDispatch **ppdispChild);
    IFACEMETHODIMP get_accName(VARIANT varChild, BSTR *pszName);
    IFACEMETHODIMP get_accValue(VARIANT varChild, BSTR *pszValue);
    IFACEMETHODIMP get_accDescription(VARIANT varChild, BSTR *pszDescription);
    IFACEMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);
    IFACEMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);
    IFACEMETHODIMP get_accHelp(VARIANT varChild, BSTR *pszHelp);
    IFACEMETHODIMP get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
    IFACEMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
    IFACEMETHODIMP get_accFocus(VARIANT *pvarChild);
    IFACEMETHODIMP get_accSelection(VARIANT *pvarChildren);
    IFACEMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
    IFACEMETHODIMP accSelect(long flagsSelect, VARIANT varChild);
    IFACEMETHODIMP accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
    IFACEMETHODIMP accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
    IFACEMETHODIMP accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
    IFACEMETHODIMP accDoDefaultAction(VARIANT varChild);
    IFACEMETHODIMP put_accName(VARIANT varChild, BSTR szName);
    IFACEMETHODIMP put_accValue(VARIANT varChild, BSTR szValue);

    // IEnumVARIANT methods.

    IFACEMETHODIMP Next(ULONG celt, VARIANT *rgVar, ULONG *pCeltFetched);
    IFACEMETHODIMP Skip(ULONG celt);
    IFACEMETHODIMP Reset();
    IFACEMETHODIMP Clone(IEnumVARIANT **ppEnum);


};

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SysLCESub_H_
#define __SysLCESub_H_

#include "resource.h"       // main symbols

BSTR GuidToBstr(REFGUID guid);
class CComSpy;

////////////////////////////////////////////////////////////////////////////
// CSysLCESub
class ATL_NO_VTABLE CSysLCESub : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public ICOMSysLCE
{
private:
    LPCWSTR    EventName( EventEnum e );

protected:
    BSTR m_bstrSubscriptionID;
    CComSpy * m_pSpy;

public:
    CSysLCESub() :
      m_bstrSubscriptionID(NULL),
      m_pSpy(NULL)
    {
    }

    void SetSpyObj(CComSpy * pSpy)
    {
        m_pSpy = pSpy;
    }

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CSysLCESub)
    COM_INTERFACE_ENTRY(ICOMSysLCE)
END_COM_MAP()

    // Virtual Methods
public:
    virtual EventEnum EventType() = 0;
    virtual REFCLSID EventCLSID() = 0;
    virtual REFIID EventIID() = 0;

    // ICOMSysLCESub Methods
public:
    STDMETHOD(GetEventType)(__out EventEnum* e) { *e = EventType(); return S_OK; }
    STDMETHOD(GetEventClass)(__out LPGUID guid) { *guid = EventCLSID(); return S_OK; }
    STDMETHOD(Install)(__in BSTR* PropertyName, __in VARIANT PropertyValue);
    STDMETHOD(Uninstall)();
};

#endif //__SysLCESub_H_

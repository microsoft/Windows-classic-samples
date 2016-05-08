//-----------------------------------------------------------------------------
// File: WebHelper.h
// Desc: Class for opening a browser window and sending data via HTTP PUSH.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include <exdisp.h>
#include <exdispid.h>


/******************************************************************************
 *
 *  WebHelper Class
 *  
 *  The WebHelper class hosts the InternetExplorer control and has a helper
 *  function for submitting HTTP POST data (see OpenURLWithData).
 *
 *  The InternetExplorer controls sends events to the client through the
 *  DWebBrowserEvents2 dispinterface. We use this to catch the "browser
 *  window closed" event. 
 *
 *****************************************************************************/

// DispatchCallback defines a callback for hooking the events from the
// browser control. (This is just to separate the browser code from the
// application logic.)

struct DispatchCallback
{
    virtual void OnDispatchInvoke(DISPID  dispIdMember) = 0;
};


class WebHelper: public IDispatch
{
private:
    IWebBrowser2        *m_pBrowser;
    IConnectionPoint    *m_pCP;         // Connection point to receive events from the control.
    DWORD               m_dwCookie;     // Connection point identifier.

    DispatchCallback    *m_pDispatchCB; // Callback to handle browser events.

    HWND    m_hwnd;

public:

    WebHelper() : m_pBrowser(NULL), m_pCP(NULL), m_hwnd(NULL), m_pDispatchCB(NULL)
    {
    }

    ~WebHelper()
    {
        Exit();
    }

    HRESULT Init(DispatchCallback *pCallback);
    void    Exit();
    HRESULT OpenURLWithData(const WCHAR *pURL, const BYTE *pPostData, DWORD cbData);
    
    // IUnknown  methods
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        if (ppv == NULL)
        {
            return E_POINTER;
        }
        if (riid == IID_IUnknown)
        {
            *ppv = (IUnknown*)this;
        }
        else if (riid == IID_IDispatch)
        {
            *ppv = (IDispatch*)this;
        }
        else
        {
            // NOTE: We specifically do *not* expose DWebBrowserEvents2,
            // instead the caller must go through IDispatch.
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    // We implement fake ref counting, because our lifetime always exceeds the
    // lifetime of the browser control.
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 2; }
    
    // IDispatch methods

    // GetIDsOfNames: Not implemented
    STDMETHODIMP GetIDsOfNames(REFIID,
        OLECHAR FAR* FAR*,  
        unsigned int, LCID, DISPID FAR*)
    {
        return E_NOTIMPL;
    }

    // GetTypeInfo: Not implemented
    STDMETHODIMP GetTypeInfo(unsigned int,         
        LCID, ITypeInfo FAR* FAR*)
    {
        return E_NOTIMPL;
    }

    // GetTypeInfoCount: Always return 0 (no type info)
    STDMETHODIMP GetTypeInfoCount(unsigned int FAR* pctinfo)
    {
        if (pctinfo == NULL)
        {
            return E_POINTER;
        }
        else
        {
            *pctinfo = 0;
            return S_OK;
        }
    }

    // Invoke:
    STDMETHODIMP Invoke( 
        DISPID  dispIdMember, REFIID, LCID, WORD,              
        DISPPARAMS FAR*, VARIANT FAR*, EXCEPINFO FAR*, unsigned int FAR*    
    )
    {
        if (m_pDispatchCB)
        {
            m_pDispatchCB->OnDispatchInvoke(dispIdMember);
        }

        return S_OK;
    }        

};
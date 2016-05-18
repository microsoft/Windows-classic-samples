//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  EditSession.h
//
//          CEditSessionBase declaration.
//
//////////////////////////////////////////////////////////////////////

#ifndef EDITSESSION_H
#define EDITSESSION_H

#include "TextService.h"

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(CExtentMonitorTextService *pTextService, ITfContext *pContext)
    {
        _cRef = 1;
        _pContext = pContext;
        _pContext->AddRef();

        _pTextService = pTextService;
        _pTextService->AddRef();
    }
    virtual ~CEditSessionBase()
    {
        _pContext->Release();
        _pTextService->Release();
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
    {
        if (ppvObj == NULL)
            return E_INVALIDARG;

        *ppvObj = NULL;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_ITfEditSession))
        {
            *ppvObj = (ITfLangBarItemButton *)this;
        }

        if (*ppvObj)
        {
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return ++_cRef;
    }
    STDMETHODIMP_(ULONG) Release(void)
    {
        LONG cr = --_cRef;

        assert(_cRef >= 0);

        if (_cRef == 0)
        {
            delete this;
        }

        return cr;
    }

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

protected:
    ITfContext *_pContext;
    CExtentMonitorTextService *_pTextService;

private:
    LONG _cRef;     // COM ref count
};

#endif // EDITSESSION_H

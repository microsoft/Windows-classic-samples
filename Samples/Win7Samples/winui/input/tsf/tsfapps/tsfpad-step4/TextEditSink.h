#pragma once

#include "private.h"

#define TES_INVALID_COOKIE  ((DWORD)(-1))

class CTextEditor;

class CTextEditSink : public ITfTextEditSink
{
public:
    CTextEditSink(CTextEditor *pEditor);
    virtual ~CTextEditSink() {};

    //
    // IUnknown methods
    //
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    //
    // ITfTextEditSink
    //
    STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

private:
    long _cRef;
    ITfContext *_pic;
    DWORD _dwEditCookie;
    CTextEditor *_pEditor;
};


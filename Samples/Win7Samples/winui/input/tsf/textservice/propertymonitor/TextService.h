//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  TextService.h
//
//          CPropertyMonitorTextService declaration.
//
//////////////////////////////////////////////////////////////////////

#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

class CLangBarItemButton;
class CPropertyPopupWindow;

class CPropertyMonitorTextService : public ITfTextInputProcessor,
                     public ITfThreadMgrEventSink,
                     public ITfTextEditSink,
                     public ITfThreadFocusSink
{
public:
    CPropertyMonitorTextService();
    ~CPropertyMonitorTextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext *pContext);
    STDMETHODIMP OnPopContext(ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // CClassFactory factory callback
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    ITfThreadMgr *_GetThreadMgr() { return _pThreadMgr; }

    // dump all properties
    void DumpProperties(ITfContext *pContext);
    void _DumpProperties(TfEditCookie ec, ITfContext *pContext);

    CPropertyPopupWindow *_GetPopupWindow() {return _pPopupWindow;}

private:
    // initialize and uninitialize ThreadMgrEventSink.
    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    // initialize TextEditSink.
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);

    // initialize and uninitialize LanguageBar Item.
    BOOL _InitLanguageBar();
    void _UninitLanguageBar();

    // initialize and uninitalize the thread focus sink.
    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    void _DumpPropertyInfo(REFGUID rguid);
    HRESULT _GetTextLengthInRange(TfEditCookie ec, ITfRange *prange, LONG *pcch);
    HRESULT _GetTextExtent(TfEditCookie ec, ITfRange *prange, LONG *pacp, LONG *pcch);
    void _DumpPropertyRange(REFGUID rguid, TfEditCookie ec, ITfProperty *pprop, ITfRange *prange);
    void _DumpRange(TfEditCookie ec, ITfRange *prange);
    void _DumpVariant(VARIANT *pvar);
    BOOL _IsDisplayAttributeProperty(REFGUID rguid);
    void _ShowPopupWindow();

    //
    // state
    //
    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;

    IStream *_pMemStream;
    ITfDisplayAttributeMgr *_pDisplayAttributeMgr;
    ITfCategoryMgr *_pCategoryMgr;
    CPropertyPopupWindow *_pPopupWindow;

    // The cookie of ThreadMgrEventSink
    DWORD _dwThreadMgrEventSinkCookie;

    //
    // private variables for TextEditSink
    //
    ITfContext   *_pTextEditSinkContext;
    DWORD _dwTextEditSinkCookie;

    DWORD _dwThreadFocusCookie;

    CLangBarItemButton *_pLangBarItem;

    LONG _cRef;     // COM ref count
};


#endif // TEXTSERVICE_H

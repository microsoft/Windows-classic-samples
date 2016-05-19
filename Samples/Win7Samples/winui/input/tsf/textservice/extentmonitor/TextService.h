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
//          CExtentMonitorTextService declaration.
//
//////////////////////////////////////////////////////////////////////

#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

class CLangBarItemButton;
class CExtentPopupWindow;
class CExtentVisualWindow;
class CRangeExtentViewer;
class CRangeFromPointViewer;


// DumpExtent EventIDs
#define  DE_EVENTID_ACTIVATE              0
#define  DE_EVENTID_ONSETFOCUS            1
#define  DE_EVENTID_ONENDEDIT             2
#define  DE_EVENTID_ONLAYOUTCHANGE        3
#define  DE_EVENTID_FROMLANGUAGEBAR       4

class CExtentMonitorTextService : public ITfTextInputProcessor,
                     public ITfThreadMgrEventSink,
                     public ITfTextEditSink,
                     public ITfTextLayoutSink,
                     public ITfThreadFocusSink
{
public:
    CExtentMonitorTextService();
    ~CExtentMonitorTextService();

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

    // ITfTextLayoutSink
    STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView);

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // CClassFactory factory callback
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    ITfThreadMgr *_GetThreadMgr() { return _pThreadMgr; }

    CExtentPopupWindow *_GetPopupWindow() {return _pPopupWindow;}

    void DumpExtentFocusContext(UINT nEventId);
    void _EnsurePopupWindow();
    void _DumpExtent(TfEditCookie ec, ITfContext *pContext, UINT nEventId);

    void _EnsureExtentVisualWindows();
    void _UpdateExtentVisualWindows();
    void _HideExtentVisualWindows();
    BOOL IsShownExtentVisualWindows() {return _fIsShownExtentVisualWindows;}

    void _EnsureRangeExtentViewer();
    void _UpdateRangeExtentViewer();
    void _HideRangeExtentViewer();
    BOOL IsShownRangeExtentViewer() {return _fIsShownRangeExtentViewer;}

    void GetRectFromPointOnFocusDocument(POINT pt);
    void _GetRectFromPoint(TfEditCookie ec, ITfContext *pContext, POINT pt);

    void _EnsureRangeFromPointViewer();
    void _UpdateRangeFromPointViewer();
    void _HideRangeFromPointViewer();
    BOOL IsShownRangeFromPointViewer() {return _fIsShownRangeFromPointViewer;}

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

    void DumpExtent(ITfContext *pContext, UINT nEventId);

    void _DumpRange(TfEditCookie ec, ITfRange *prange);


    HRESULT GetNonTransitoryDim(ITfDocumentMgr *pDocMgr, ITfDocumentMgr **ppDim);

    //
    // state
    //
    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;

    IStream *_pMemStream;
    ITfDisplayAttributeMgr *_pDisplayAttributeMgr;
    ITfCategoryMgr *_pCategoryMgr;
    CExtentPopupWindow *_pPopupWindow;

    // Extent Visual windows.
    CExtentVisualWindow *_pExtentVisualWinodowStartPos;
    CExtentVisualWindow *_pExtentVisualWinodowEndPos;
    CExtentVisualWindow *_pExtentVisualWinodowSelection;
    BOOL _fIsShownExtentVisualWindows;
    RECT _rcStartPos;
    RECT _rcEndPos;
    RECT _rcSelection;
    RECT _rcRanges[256];

    CRangeExtentViewer *_pRangeExtentViewer;
    BOOL _fIsShownRangeExtentViewer;
    RECT _rcView;

    CRangeFromPointViewer *_pRangeFromPointViewer;
    BOOL _fIsShownRangeFromPointViewer;

    // The cookie of ThreadMgrEventSink
    DWORD _dwThreadMgrEventSinkCookie;

    //
    // private variables for TextEditSink
    //
    ITfContext   *_pTextEditSinkContext;
    DWORD _dwTextEditSinkCookie;
    DWORD _dwTextLayoutSinkCookie;

    DWORD _dwThreadFocusCookie;

    CLangBarItemButton *_pLangBarItem;

    LONG _cRef;     // COM ref count
};


#endif // TEXTSERVICE_H

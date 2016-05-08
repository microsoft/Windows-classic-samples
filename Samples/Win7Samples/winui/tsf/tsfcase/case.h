//
// case.h
//
// CCaseTextService declaration.
//

#ifndef CASE_H
#define CASE_H

class CLangBarItemButton;
class CSnoopWnd;

class CCaseTextService : public ITfTextInputProcessor,
                         public ITfThreadMgrEventSink,
                         public ITfTextEditSink,
                         public ITfThreadFocusSink,
                         public ITfKeyEventSink
{
public:
    CCaseTextService();
    ~CCaseTextService();

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

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten);

    // CClassFactory factory callback
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    // server registration
    static BOOL RegisterProfiles();
    static void UnregisterProfiles();
    static BOOL RegisterCategories(BOOL fRegister);
    static BOOL RegisterServer();
    static void UnregisterServer();

    // language bar menu handlers
    static void _Menu_HelloWord(CCaseTextService *_this);
    static void _Menu_FlipSel(CCaseTextService *_this);
    static void _Menu_FlipDoc(CCaseTextService *_this);
    static void _Menu_ShowSnoopWnd(CCaseTextService *_this);
    static void _Menu_FlipKeys(CCaseTextService *_this);

    ITfThreadMgr *_GetThreadMgr() { return _pThreadMgr; }
    TfClientId _GetClientId() { return _tfClientId; }
    BOOL _IsSnoopWndVisible() { return _fShowSnoop; }
    BOOL _IsKeyFlipping() { return _fFlipKeys; }

private:
    // init methods
    BOOL _InitLanguageBar();
    BOOL _InitThreadMgrSink();
    BOOL _InitSnoopWnd();
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);
    BOOL _InitKeystrokeSink();
    BOOL _InitPreservedKey();

    // uninit methods
    void _UninitLanguageBar();
    void _UninitThreadMgrSink();
    void _UninitSnoopWnd();
    void _UninitKeystrokeSink();
    void _UninitPreservedKey();

    //
    // state
    //

    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;

    BOOL _fShowSnoop; // hide/show the snoop window popup
    CSnoopWnd *_pSnoopWnd;

    BOOL _fFlipKeys;

    CLangBarItemButton *_pLangBarItem;

    DWORD _dwThreadMgrEventSinkCookie;
    DWORD _dwThreadFocusSinkCookie;
    DWORD _dwTextEditSinkCookie;
    ITfContext *_pTextEditSinkContext;

    LONG _cRef;     // COM ref count
};


#endif // CASE_H

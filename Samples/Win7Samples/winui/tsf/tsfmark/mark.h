//
// mark.h
//
// CMarkTextService declaration.
//

#ifndef MARK_H
#define MARK_H

class CLangBarItemButton;

class CMarkTextService : public ITfTextInputProcessor,
                         public ITfDisplayAttributeProvider,
                         public ITfCreatePropertyStore,
                         public ITfThreadMgrEventSink,
                         public ITfTextEditSink,
                         public ITfCompositionSink,
                         public ITfCleanupContextDurationSink,
                         public ITfCleanupContextSink,
                         public ITfCompartmentEventSink,
                         public ITfKeyEventSink
{
public:
    CMarkTextService();
    ~CMarkTextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();

    // ITfDisplayAttributeProvider
    STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo);

    // ITfCreatePropertyStore
    STDMETHODIMP IsStoreSerializable(REFGUID guidProperty, ITfRange *pRange, ITfPropertyStore *pPropertyStore, BOOL *pfSerializable);
    STDMETHODIMP CreatePropertyStore(REFGUID guidProperty, ITfRange *pRange, ULONG cb, IStream *pStream, ITfPropertyStore **ppStore);

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext *pContext);
    STDMETHODIMP OnPopContext(ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

    // ITfCleanupContextDurationSink
    STDMETHODIMP OnStartCleanupContext();
    STDMETHODIMP OnEndCleanupContext();

    // ITfCleanupContextSink
    STDMETHODIMP OnCleanupContext(TfEditCookie ecWrite, ITfContext *pContext);

    // ITfCompartmentEventSink
    STDMETHODIMP OnChange(REFGUID rguidCompartment);

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
    static void _Menu_OnComposition(CMarkTextService *_this);
    static void _Menu_OnSetGlobalCompartment(CMarkTextService *_this);
    static void _Menu_OnSetCaseProperty(CMarkTextService *_this);
    static void _Menu_OnViewCaseProperty(CMarkTextService *_this);
    static void _Menu_OnViewCustomProperty(CMarkTextService *_this);
    static void _Menu_OnSetCustomProperty(CMarkTextService *_this);

    // keystroke handlers
    HRESULT _HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleReturn(TfEditCookie ec, ITfContext *pContext);

    // callbacks for CCompositionEditSession
    BOOL _IsComposing()
    {
        return _pComposition != NULL;
    }
    ITfComposition *_GetComposition()
    {
        return _pComposition;
    }
    void _SetComposition(ITfComposition *pComposition)
    {
        _pComposition = pComposition;
    }
    void _TerminateComposition(TfEditCookie ec)
    {
        if (_pComposition != NULL)
        {
            _ClearCompositionDisplayAttributes(ec);
            _pComposition->EndComposition(ec);
            SafeReleaseClear(_pComposition);
        }
    }
    void _TerminateCompositionInContext(ITfContext *pContext);
    void _ClearCompositionDisplayAttributes(TfEditCookie ec);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec);

    // messages for the worker wnd
    enum { WM_DISPLAY_PROPERTY = WM_USER + 1 };
    // property methods
    void _ViewCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _ViewCustomProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCustomProperty(TfEditCookie ec, ITfContext *pContext);

private:
    // init methods
    BOOL _InitLanguageBar();
    BOOL _InitThreadMgrSink();
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);
    BOOL _InitDisplayAttributeGuidAtom();
    BOOL _InitCleanupContextDurationSink();
    BOOL _InitCleanupContextSink(ITfContext *pContext);
    BOOL _InitContextCompartment(ITfContext *pContext);
    BOOL _InitGlobalCompartment();
    BOOL _InitWorkerWnd();
    BOOL _InitKeystrokeSink();

    // uninit methods
    void _UninitLanguageBar();
    void _UninitThreadMgrSink();
    void _UninitCleanupContextDurationSink();
    void _UninitCleanupContextSink(ITfContext *pContext);
    void _UninitCompartment(ITfContext *pContext);
    void _UninitGlobalCompartment();
    void _UninitWorkerWnd();
    void _UninitKeystrokeSink();

    // property methods
    void _RequestPropertyEditSession(ULONG ulCallback);
    static LRESULT CALLBACK _WorkerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    //
    // state
    //

    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;
    
    ITfComposition *_pComposition; // pointer to an ongoing composition, or NULL if there is none

    BOOL _fCleaningUp;

    TfGuidAtom _gaDisplayAttribute;

    CLangBarItemButton *_pLangBarItem;

    DWORD _dwThreadMgrEventSinkCookie;
    DWORD _dwThreadFocusSinkCookie;
    DWORD _dwTextEditSinkCookie;
    DWORD _dwGlobalCompartmentEventSinkCookie;

    ITfContext *_pTextEditSinkContext;

    HWND _hWorkerWnd;
    // buffers for the popup property displays
    WCHAR _achDisplayText[64];
    WCHAR _achDisplayPropertyText[64];

    LONG _cRef;     // COM ref count
};


#endif // MARK_H

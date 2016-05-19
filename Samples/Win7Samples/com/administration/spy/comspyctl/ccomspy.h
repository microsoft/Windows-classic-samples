// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __COMSPY_H_
#define __COMSPY_H_

#include "resource.h"       // main symbols

#include "comsvcs.h"

#define IF_AUDIT_DO(x)                        \
    if (m_pSpy->Audit())                    \
        m_pSpy->GetAuditObj()->x        


/////////////////////////////////////////////////////////////////////////////
// CComSpy
class ATL_NO_VTABLE CComSpy : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComSpy, &CLSID_ComSpy>,
    public CComControl<CComSpy>,
    public CStockPropImpl<CComSpy, IComSpy, &IID_IComSpy, &LIBID_COMSPYCTLLib>,
    public IProvideClassInfo2Impl<&CLSID_ComSpy, NULL, &LIBID_COMSPYCTLLib>,
    public IPersistStreamInitImpl<CComSpy>,
    public IPersistStorageImpl<CComSpy>,
    public IQuickActivateImpl<CComSpy>,
    public IOleControlImpl<CComSpy>,
    public IOleObjectImpl<CComSpy>,
    public IOleInPlaceActiveObjectImpl<CComSpy>,
    public IViewObjectExImpl<CComSpy>,
    public IOleInPlaceObjectWindowlessImpl<CComSpy>,
    public IDataObjectImpl<CComSpy>,
    public ISpecifyPropertyPagesImpl<CComSpy>,
    public IPropertyBag
{

private:
    HRESULT EnableAudit(BOOL bEnable);
    MapStringToAppInfo m_map;
    CAppInfo *m_pSysAppInfo;    //system app that manages system events
    BOOL m_bLogToFile;
    HANDLE m_hFile;
    CComBSTR m_sLogFile;
    int m_cEvents;
    HWND m_hWndList;
    BOOL m_bShowGridLines;    
    BOOL m_bAudit;
    HFONT m_hFont;
    long m_nWidth[NUMBER_COLUMNS];
    BOOL m_bShowOnScreen;
    BOOL m_bCSV;
    HMENU m_hMenuDebug; //the Debug menu
    CComPtr<IComSpyAudit> m_spSqlAudit;

public:

    BOOL Audit(){return m_bAudit;}
    IComSpyAudit * GetAuditObj(){return m_spSqlAudit;}
    CComPtr<IFontDisp> m_pFont;
    CContainedWindow m_ctlSysListView32;

    CComSpy();
    ~CComSpy() 
    {
        m_hWndList = NULL;    

    }
    void FinalRelease()
    {
    }
    

    bool ShouldScroll(int nIndex);
    HRESULT ShutdownApplication(LPCWSTR pwszApplicationName);
    bool AddEventToList(LONGLONG perfCount, LPCWSTR pwszEvent, LPCWSTR pwszApplication);
    bool AddParamValueToList(LPCWSTR pwszParamName, LPCWSTR pwszValue);


    DECLARE_REGISTRY_RESOURCEID(IDR_COMSPY)

    BEGIN_COM_MAP(CComSpy)
        COM_INTERFACE_ENTRY(IComSpy)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
        COM_INTERFACE_ENTRY_IMPL(IOleControl)
        COM_INTERFACE_ENTRY_IMPL(IOleObject)
        COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_IMPL(IDataObject)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY(IPropertyBag)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CComSpy)
        PROP_ENTRY_TYPE_EX( "LogFile", DISPID_LOGFILE, CLSID_ComSpyPropPage, IID_IComSpy, VT_BSTR)
        PROP_ENTRY_TYPE_EX( "ShowGridLines", DISPID_GRIDLINES, CLSID_ComSpyPropPage, IID_IComSpy, VT_BOOL)
        PROP_ENTRY_TYPE_EX( "Audit", DISPID_AUDIT, CLSID_ComSpyPropPage, IID_IComSpy, VT_BOOL)
        //PROP_ENTRY_EX( "ColWidth", DISPID_COLWIDTH, CLSID_ComSpyPropPage, IID_IComSpy)    
        PROP_PAGE(CLSID_StockFontPage)
    END_PROPERTY_MAP()


    BEGIN_MSG_MAP(CComSpy)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)

    ALT_MSG_MAP(1)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
        COMMAND_ID_HANDLER(ID_LOG, OnLogToFile)
        COMMAND_ID_HANDLER(ID_CHOOSE_LOG_FILE_NAME, OnChooseLogFile)
        COMMAND_ID_HANDLER(ID_SAVE, OnSave)
        COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
        COMMAND_ID_HANDLER(ID_OPTIONS_GRID_LINES, OnToggleGridLines)    
        COMMAND_ID_HANDLER(ID_AUDIT, OnToggleAudit)    
        COMMAND_ID_HANDLER(ID_SHOW_ON_SCREEN, OnToggleShowOnScreen)
        COMMAND_ID_HANDLER(IDM_ABOUT, OnAbout)
        MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
        COMMAND_ID_HANDLER(IDM_SELECT_APPLICATIONS, OnSelectApplications)
        COMMAND_ID_HANDLER(ID_CHOOSEFONT, OnChooseFont)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_RANGE_HANDLER(ID_DEBUG_BEGIN, ID_DEBUG_END, OnDebugApplication)
    END_MSG_MAP()

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLogToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChooseLogFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleGridLines(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleAudit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSelectApplications(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChooseFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleShowOnScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDebugApplication(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);




    STDMETHOD(Close)(DWORD dwSaveOption);
        
    STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip)
    {
        IOleInPlaceObjectWindowlessImpl<CComSpy>::SetObjectRects(prcPos, prcClip);
        int cx, cy;
        cx = prcPos->right - prcPos->left;
        cy = prcPos->bottom - prcPos->top;
        ::SetWindowPos(m_ctlSysListView32.m_hWnd, NULL, 0,
            0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

        
        return S_OK;
    }

// IViewObjectEx
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(L"IViewObjectExImpl::GetViewStatus\n");
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

// IComSpy
public:
    bool AddRunningAspsToDebugMenu(HMENU hMenu);
    STDMETHOD(ChooseLogFile)(/*[out]*/ BSTR * sLogFileName, /*[out]*/ BOOL * bCanceled);
    STDMETHOD(get_ShowOnScreen)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_ShowOnScreen)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_Audit)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_Audit)(/*[in]*/ BOOL newVal);
    STDMETHOD(ChooseFont)();
    STDMETHOD(get_LogToFile)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_LogToFile)(/*[in]*/ BOOL newVal);
    STDMETHOD(About)();
    STDMETHOD(ClearAllEvents)();
    STDMETHOD(SaveToFile)();
    STDMETHOD(SelectApplications)();
    HFONT CreateHFontFromIFont(__in IFont* pFontDisp);
    STDMETHOD(get_ColWidth)(short nColumn, /*[out, retval]*/ long *pVal);
    STDMETHOD(put_ColWidth)(short nColumn, /*[in]*/ long newVal);
    STDMETHOD(get_ShowGridLines)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_ShowGridLines)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_LogFile)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_LogFile)(/*[in]*/ BSTR newVal);
    HRESULT OnDrawAdvanced(ATL_DRAWINFO& di);
    HRESULT STDMETHODCALLTYPE putref_Font(__in IFontDisp* pFontDisp);

    STDMETHOD(Read)(LPCWSTR pwszPropName,VARIANT* pVar,IErrorLog* pErrorLog )
    {
        return S_OK;
    }

    STDMETHOD(Write)(LPCWSTR pwszPropName,VARIANT* pVar)
    {
        return S_OK;
    }

    HRESULT IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap);
    HRESULT IPersistStreamInit_Save(LPSTREAM pStm, BOOL /* fClearDirty */,
        const ATL_PROPMAP_ENTRY* pMap);


};

#endif //__COMSPY_H_

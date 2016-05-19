// MainDlg.h : Declaration of the CMainDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include "stdafx.h"

typedef enum MEDIATYPE
{
    AUDIO       = 0,
    PHOTO       = 1,
    VIDEO       = 2,
    PLAYLIST    = 3,
    OTHER       = 4
};

struct NODEPARAM
{
    WCHAR* szNodeName;
    WCHAR* szAttrName;
};


/////////////////////////////////////////////////////////////////////////////
// CMainDlg
class CMainDlg : 
    public CAxDialogImpl<CMainDlg>
{
public:
    CMainDlg();
    ~CMainDlg();

    enum { IDD = IDD_MAINDLG };

BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER     (WM_INITDIALOG,                     OnInitDialog)
    MESSAGE_HANDLER     (WM_DESTROY,                        OnDestroy)
    COMMAND_ID_HANDLER  (IDOK,                              OnOK)
    COMMAND_ID_HANDLER  (IDCANCEL,                          OnCancel)
    COMMAND_HANDLER     (IDC_MEDIATYPELIST, CBN_SELCHANGE,  OnChangeMediaType)
    COMMAND_HANDLER     (IDC_LIBLIST,       CBN_SELCHANGE,  OnChangeLib)
    NOTIFY_HANDLER      (IDC_TREE,          NM_CLICK,       OnClickTree)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnChangeMediaType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnChangeLib(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClickTree(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled);

private:
    CAxWindow                       *m_pView; 
    CComPtr<IWMPPlayer4>            m_spPlayer;
    CComPtr<IWMPMediaCollection2>   m_spMC;
    HWND                            m_hLibTree;
    HWND                            m_hDetailList;
    MEDIATYPE                       m_mtCurMediaType;
    CComBSTR                        m_bstrMediaType;
    int                             m_cSchemaCount;
    NODEPARAM*                      m_pnpNodeParams;
    HTREEITEM                       m_hPreNode;
    
private:
    HRESULT CreateWmpOcx();
    void BuildLibTree();
    HRESULT UpdateCurMC();
    void AddSubNode(MEDIATYPE mediaType);
    void ShowQueryResult(IWMPQuery* pQuery);
};


// CWMPRemoteHost: implement IServiceProvider and IWMPRemoteMediaServices
// to provide a remote host
class CWMPRemoteHost : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IServiceProvider,
    public IWMPRemoteMediaServices
{
public:
    CWMPRemoteHost(){}
    ~CWMPRemoteHost(){}

    DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWMPRemoteHost)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IWMPRemoteMediaServices)
END_COM_MAP()

    // IServiceProvider
    STDMETHOD(QueryService)(REFGUID /*guidService*/, REFIID riid, void **ppv)
    {
        return QueryInterface(riid, ppv);
    }
    // IWMPRemoteMediaServices
    STDMETHOD(GetServiceType)(BSTR *pbstrType)
    {
        *pbstrType = ::SysAllocString(L"Remote");
        return S_OK;
    }
    STDMETHOD(GetApplicationName)(BSTR *pbstrName)
    {
        *pbstrName = ::SysAllocString(L"Test Application");
        return S_OK;
    }
    STDMETHOD(GetScriptableObject)(BSTR * /*pbstrName*/, IDispatch ** /*ppDispatch*/)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(GetCustomUIMode)(BSTR * /*pbstrFile*/)
    {
        return E_NOTIMPL;
    }
};

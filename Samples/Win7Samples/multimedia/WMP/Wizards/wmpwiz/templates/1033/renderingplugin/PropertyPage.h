/////////////////////////////////////////////////////////////////////////////
//
// C[!output Safe_root]PropPage.h : Declaration of C[!output Safe_root]PropPage
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __C[!output SAFE_ROOT]PROPPAGE_H_
#define __C[!output SAFE_ROOT]PROPPAGE_H_

#include "resource.h"

// {[!output CLASSIDPROPPAGE]}
DEFINE_GUID(CLSID_[!output Safe_root]PropPage, [!output DEFINEGUIDPROPPAGE]);

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage
class ATL_NO_VTABLE C[!output Safe_root]PropPage :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<C[!output Safe_root]PropPage, &CLSID_[!output Safe_root]PropPage>,
    public IPropertyPageImpl<C[!output Safe_root]PropPage>,
    public CDialogImpl<C[!output Safe_root]PropPage>
{
public:
            C[!output Safe_root]PropPage(); 
    virtual ~C[!output Safe_root]PropPage(); 
    

    enum {IDD = IDD_[!output SAFE_ROOT]PROPPAGE};
    

DECLARE_REGISTRY_RESOURCEID(IDR_[!output SAFE_ROOT]PROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(C[!output Safe_root]PropPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
    
END_COM_MAP()

BEGIN_MSG_MAP(C[!output Safe_root]PropPage)
    CHAIN_MSG_MAP(IPropertyPageImpl<C[!output Safe_root]PropPage>)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_HANDLER(IDC_BLUE, BN_CLICKED, OnClickedBlue)
    COMMAND_HANDLER(IDC_GREEN, BN_CLICKED, OnClickedGreen)
    COMMAND_HANDLER(IDC_RED, BN_CLICKED, OnClickedRed)
END_MSG_MAP()

    STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk);
    STDMETHOD(Apply)(void);

    LRESULT (OnInitDialog)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT (OnClickedBlue)(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT (OnClickedGreen)(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT (OnClickedRed)(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
private:
    CComPtr<I[!output Safe_root]> m_sp[!output Safe_root];  // pointer to plug-in interface
    COLORREF            m_Color; // Last button clicked.
};

#endif // __C[!output Safe_root]PropPage_H_

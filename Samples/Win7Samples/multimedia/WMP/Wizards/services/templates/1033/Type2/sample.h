/////////////////////////////////////////////////////////////////////////////
//
// [!output root].h : Implementation of C[!output Safe_root]
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __[!output SAFE_ROOT]_H_
#define __[!output SAFE_ROOT]_H_

#include "resource.h"       // main symbols
#include "wmp.h"
#include "subscriptionservices.h"
#include <atlwin.h>

// {[!output CLASSID]}
DEFINE_GUID(CLSID_[!output Safe_root], [!output DEFINEGUID]);

// Content Distributor ID
const WCHAR kwszContentDistributorID[] = L"[!output CONTENTDISTRIBUTOR]";

//**********************************************************************
class ATL_NO_VTABLE C[!output Safe_root] : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<C[!output Safe_root], &CLSID_[!output Safe_root]>,
    public IWMPSubscriptionService2
{
public:
    C[!output Safe_root]()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_[!output SAFE_ROOT])

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(C[!output Safe_root])
        COM_INTERFACE_ENTRY(IWMPSubscriptionService)
        COM_INTERFACE_ENTRY(IWMPSubscriptionService2)
    END_COM_MAP()

public:
    // IWMPSubscriptionService methods
    STDMETHODIMP allowPlay(HWND hwnd, IWMPMedia *pMedia, BOOL *pfAllowPlay);
    STDMETHODIMP allowCDBurn(HWND hwnd, IWMPPlaylist *pPlaylist, BOOL *pfAllowBurn);
    STDMETHODIMP allowPDATransfer(HWND hwnd, IWMPPlaylist *pPlaylist, BOOL *pfAllowTransfer);
    STDMETHODIMP startBackgroundProcessing(HWND hwnd);

    // IWMPSubscriptionService2 methods
    STDMETHODIMP stopBackgroundProcessing(void);
    STDMETHODIMP serviceEvent(WMPSubscriptionServiceEvent event);
    STDMETHODIMP deviceAvailable(BSTR bstrDeviceName, IWMPSubscriptionServiceCallback *pCB);
    STDMETHODIMP prepareForSync(BSTR bstrFilename, BSTR bstrDeviceName, IWMPSubscriptionServiceCallback *pCB);
};

//**********************************************************************
// The following classes define our popup dialogs used to display
// the list of items in the playlist passed to allowCDBurn() and
// allowPDATransfer().
//**********************************************************************

//**********************************************************************
template <class T>
class CAllowBaseDialog : public CDialogImpl<T>
{
protected:
    CComPtr<IWMPPlaylist> m_spPlaylist;
public:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled, int iResourceID);
    LRESULT OnRemoveMediaFromPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled, int iResourceID);
};

//**********************************************************************
class CAllowBurnDialog :
    public CAllowBaseDialog<CAllowBurnDialog>
{
public:
    enum { IDD = IDD_ALLOWBURN };
    
    BEGIN_MSG_MAP(CAllowBurnDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDC_TRANSFER_REMOVE, OnRemoveMediaFromPlaylist)
        COMMAND_ID_HANDLER(IDYES, OnYes)
        COMMAND_ID_HANDLER(IDNO, OnNo)
    END_MSG_MAP()

    void SetPlaylist(IWMPPlaylist *pPlaylist) { m_spPlaylist = pPlaylist; }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
    {
        return CAllowBaseDialog<CAllowBurnDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled, IDC_ALLOWBURN_LISTBOX);
    }
    
    LRESULT OnRemoveMediaFromPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        return CAllowBaseDialog<CAllowBurnDialog>::OnRemoveMediaFromPlaylist(wNotifyCode, wID, hWndCtl, bHandled, IDC_ALLOWBURN_LISTBOX);
    }
    
    LRESULT OnYes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        EndDialog(IDYES);
        bHandled = TRUE;
        return 0;
    }
    
    LRESULT OnNo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        EndDialog(IDNO);
        bHandled = TRUE;
        return 0;
    }
};

//**********************************************************************
class CAllowTransferDialog :
    public CAllowBaseDialog<CAllowTransferDialog>
{
public:
    enum { IDD = IDD_ALLOWTRANSFER };
    
    BEGIN_MSG_MAP(CAllowTransferDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDC_TRANSFER_REMOVE, OnRemoveMediaFromPlaylist)
        COMMAND_ID_HANDLER(IDYES, OnYes)
        COMMAND_ID_HANDLER(IDNO, OnNo)
    END_MSG_MAP()

    void SetPlaylist(IWMPPlaylist *pPlaylist) { m_spPlaylist = pPlaylist; }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
    {
        return CAllowBaseDialog<CAllowTransferDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled, IDC_ALLOWPDA_LISTBOX);
    }
    
    LRESULT OnRemoveMediaFromPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        return CAllowBaseDialog<CAllowTransferDialog>::OnRemoveMediaFromPlaylist(wNotifyCode, wID, hWndCtl, bHandled, IDC_ALLOWPDA_LISTBOX);
    }
    
    LRESULT OnYes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        EndDialog(IDYES);
        bHandled = TRUE;
        return 0;
    }
    
    LRESULT OnNo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
    {
        EndDialog(IDNO);
        bHandled = TRUE;
        return 0;
    }
};

#endif //__[!output SAFE_ROOT]_H_

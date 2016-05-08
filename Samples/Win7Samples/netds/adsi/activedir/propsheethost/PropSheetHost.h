//***************************************************************************
//    THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
//    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//    PARTICULAR PURPOSE.
//
//    Copyright Microsoft Corporation. All Rights Reserved.
//***************************************************************************

//***************************************************************************
//
//    File:          PropSheetHost.h
//
//***************************************************************************

#pragma once

//***************************************************************************
//    #include statements
//***************************************************************************

#include "stdafx.h"

//***************************************************************************
//    data type definitions
//***************************************************************************

#define VIEW_POINTER_OFFSET   GWLP_USERDATA

#ifndef CFSTR_DS_PARENTHWND
    #define CFSTR_DS_PARENTHWND_W L"DsAdminParentHwndClipFormat"
    #define CFSTR_DS_PARENTHWND_A "DsAdminParentHwndClipFormat"

    #ifdef UNICODE
        #define CFSTR_DS_PARENTHWND CFSTR_DS_PARENTHWND_W
    #else
        #define CFSTR_DS_PARENTHWND CFSTR_DS_PARENTHWND_A
    #endif //UNICODE
#endif //CFSTR_DS_PARENTHWND

#ifndef CFSTR_DS_PROPSHEETCONFIG
    #define CFSTR_DS_PROPSHEETCONFIG_W L"DsPropSheetCfgClipFormat"
    #define CFSTR_DS_PROPSHEETCONFIG_A "DsPropSheetCfgClipFormat"

    #ifdef UNICODE
        #define CFSTR_DS_PROPSHEETCONFIG CFSTR_DS_PROPSHEETCONFIG_W
    #else
        #define CFSTR_DS_PROPSHEETCONFIG CFSTR_DS_PROPSHEETCONFIG_A
    #endif //UNICODE
#endif //CFSTR_DS_PROPSHEETCONFIG


#ifndef WM_ADSPROP_SHEET_CREATE
    #define WM_ADSPROP_SHEET_CREATE (WM_USER + 1108)
#endif


#ifndef WM_DSA_SHEET_CREATE_NOTIFY
    #define WM_DSA_SHEET_CREATE_NOTIFY (WM_USER + 6)
#endif


#ifndef WM_DSA_SHEET_CLOSE_NOTIFY
    #define WM_DSA_SHEET_CLOSE_NOTIFY (WM_USER + 5) 
#endif


#ifndef DSA_SEC_PAGE_INFO
    typedef struct _DSA_SEC_PAGE_INFO
    {
        HWND hwndParentSheet;
        DWORD offsetTitle;
        DSOBJECTNAMES dsObjectNames;
    } DSA_SEC_PAGE_INFO, *PDSA_SEC_PAGE_INFO;
#endif //DSA_SEC_PAGE_INFO

#ifndef PROPSHEETCFG
    typedef struct _PROPSHEETCFG
    {  
        LONG_PTR lNotifyHandle;  
        HWND hwndParentSheet;  
        HWND hwndHidden;  
        WPARAM wParamSheetClose;
    } PROPSHEETCFG, *PPROPSHEETCFG;
#endif //PROPSHEETCFG

#define PROP_SHEET_HOST_ID 0xCDCDCDCD

#define PROP_SHEET_PREFIX_ADMIN L"admin"
#define PROP_SHEET_PREFIX_SHELL L"shell"

/**************************************************************************

   CPropSheetHost class definition

**************************************************************************/

class CPropSheetHost : public IDataObject
{
private:
    HWND m_hwndParent;
    HWND m_hwndHidden;
    DWORD m_ObjRefCount;
    CComPtr<IADs> m_spADObject;
    ATOM m_cfDSPropSheetConfig;
    ATOM m_cfDSObjectNames;
    ATOM m_cfDSDispSpecOptions;
    CSimpleArray<HPROPSHEETPAGE> m_rgPageHandles;
    LPTSTR m_szHiddenWindowClass;
    HINSTANCE m_hInst;
    LPWSTR m_pwszPrefix;

public:
    CPropSheetHost(HINSTANCE hInstance, HWND hwndParent = NULL);
    ~CPropSheetHost();
   
public:
    HRESULT SetObject(LPCWSTR pwszADsPath);
    HRESULT SetObject(IADs *pads);
    void Run();

private:
    HWND _CreateHiddenWindow();
    static LRESULT CALLBACK _HiddenWindowProc(HWND, UINT, WPARAM, LPARAM);
    void _CreatePropertySheet();
    static BOOL CALLBACK _AddPagesCallback(HPROPSHEETPAGE hPage, LPARAM lParam);
    HRESULT _AddPagesForObject(IADs *padsObject);
    void _CreateSecondaryPropertySheet(DSA_SEC_PAGE_INFO *pDSASecPageInfo);
    HRESULT _GetDSDispSpecOption(FORMATETC *pFormatEtc, STGMEDIUM *pStgMedium);
    HRESULT _GetDSObjectNames(FORMATETC *pFormatEtc, STGMEDIUM *pStgMedium);
    HRESULT _GetDSPropSheetConfig(FORMATETC *pFormatEtc, STGMEDIUM *pStgMedium);
    HRESULT _ExtractSecPageInfo(WPARAM wParam, PDSA_SEC_PAGE_INFO *ppSecPageInfo);

public:
    //IUnknown methods
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(DWORD) AddRef();
    STDMETHODIMP_(DWORD) Release();

    //IDataObject methods
    STDMETHODIMP GetData(FORMATETC*, STGMEDIUM*);
    STDMETHODIMP GetDataHere(FORMATETC*, STGMEDIUM*);
    STDMETHODIMP QueryGetData(FORMATETC*);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
    STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
    STDMETHODIMP EnumFormatEtc(DWORD, IEnumFORMATETC**);
    STDMETHODIMP DAdvise(FORMATETC*, DWORD, IAdviseSink*, LPDWORD);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);
};


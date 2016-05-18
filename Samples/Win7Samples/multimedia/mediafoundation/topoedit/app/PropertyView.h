// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedobj.h"
#include "resource.h"
#include <tedutil.h>

class CPropertyEditWindow;

class CPropertyController : public ITedPropertyController
{
public:
    CPropertyController(CPropertyEditWindow* pView);
    ~CPropertyController();
    
    HRESULT STDMETHODCALLTYPE ClearProperties();
    HRESULT STDMETHODCALLTYPE AddPropertyInfo(ITedPropertyInfo* pPropertyInfo);
        
    CPropertyEditWindow* GetWindow() const;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
private:
    LONG m_cRef;
    CPropertyEditWindow* m_pView;
    CAtlArray<ITedPropertyInfo*> m_arrPropertyInfo;
};

#define IDC_OKBUTTON 1001
#define IDC_ADDBUTTON 1002

class CPropertyEditWindow : public CWindowImpl<CPropertyEditWindow>
{
public:
    CPropertyEditWindow();
    ~CPropertyEditWindow();

    void ShowProperties(ITedPropertyInfo* pPropertyInfo);
    void ClearProperties();
    
    void OnFinalMessage(HWND hwnd);

    // Message Handlers //
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
protected:
    BEGIN_MSG_MAP(CPropertyEditWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        
        COMMAND_HANDLER(IDC_OKBUTTON, BN_CLICKED, OnOK)
        COMMAND_HANDLER(IDC_ADDBUTTON, BN_CLICKED, OnAdd)
    END_MSG_MAP()

    HRESULT CreateTitle(DWORD dwPropertyInfoIndex, RECT& labelRect);
    HRESULT CreatePropertyInterfaceForIndex(DWORD dwPropertyInfoIndex, DWORD dwIndex, RECT& labelRect);
    HRESULT CreatePropertyLabel(DWORD dwPropertyInfoIndex, CAtlStringW strLabelText, RECT& rectLabel);
    HRESULT CreatePropertyEdit(DWORD dwPropertyInfoIndex, CAtlStringW strInitialText, RECT& rectLabel, bool fReadOnly);
    HRESULT CreatePropertyTooltip(DWORD dwPropertyInfoIndex, HWND hWndParent, CAtlStringW strLabelText, RECT& rectTooltip);
    void ResizeChildren();
    CAtlStringW GetTextForVartype(VARTYPE vt);
    LRESULT RelayMessageToTooltipControl(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
private:
    typedef struct tagPropertyInfoDisplay
    {
        CAtlArray<CStatic*> m_arrLabels;
        CAtlArray<CEdit*> m_arrEdits;
        CAtlArray<VARTYPE> m_arrVartypes;
        CAtlArray<CToolTipControl*> m_arrTooltips;
    } PropertyInfoDisplay;
    
    CAtlArray<ITedPropertyInfo*> m_arrPropertyInfo;
    CAtlArray<PropertyInfoDisplay*> m_arrPropertyInfoDisplay;
    CAtlArray<CStatic*> m_arrTitles;
    CButton m_OKButton;
    CButton m_AddButton;
    HFONT m_hLabelFont;
    RECT m_lastRect;

    // Constants
    static const int ms_LabelWidth;
    static const int ms_LabelHeight;
    static const int ms_EditWidth;
    static const int ms_ButtonWidth;
    static const int ms_ButtonHeight;
    static const int ms_MarginWidth;
};

class CPropertyAddDialog : public CDialogImpl<CPropertyAddDialog>
{
public:
    enum { IDD = IDD_ADDPROPERTY };

    void AddPropertyCategory(CAtlString strCategory, TED_ATTRIBUTE_CATEGORY Category, DWORD dwIndex);
    
    DWORD GetChosenCategory() const { return m_dwChosenCategory; }
    CAtlString GetChosenProperty() const { return m_strChosenProperty; }
    CAtlString GetValue() const { return m_strValue; }
    
protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCategoryChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    BEGIN_MSG_MAP( CPropertyAddDialog )
       MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

       COMMAND_HANDLER(IDOK, 0, OnOK)
       COMMAND_HANDLER(IDCANCEL, 0, OnCancel)
       COMMAND_HANDLER(IDC_PROPERTYCATEGORY, CBN_SELCHANGE, OnCategoryChange)
    END_MSG_MAP()

private:
    CAtlArray<CAtlString> m_arrCategories;
    CAtlArray<TED_ATTRIBUTE_CATEGORY> m_arrCategoryIDs;
    CAtlArray<DWORD> m_arrCategoryIndex;
    DWORD m_dwChosenCategory;
    CAtlString m_strChosenProperty;
    CAtlString m_strValue;
    DWORD m_nChosenIndex;

    HWND m_hPropertyCategoryCombo;
    HWND m_hPropertyNameCombo;
    HWND m_hPropertyValueEdit;
};

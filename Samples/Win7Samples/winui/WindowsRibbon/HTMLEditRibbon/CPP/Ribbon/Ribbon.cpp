// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "../stdafx.h"
#include "../MainFrm.h"

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
#include "../RibbonRes/ribbonres.h"
#include "../HTMLEdDoc.h"
#include "../HTMLEdView.h"
#include "../HTMLEdit.h"

#include "Ribbon.h"
#include "font.h"
#include "MRU.h"

/////////////////////////////////////////////////////////////////////////////
// CRibbonBar 

BEGIN_MESSAGE_MAP(CRibbonBar, CToolBar)
    //{{AFX_MSG_MAP(CRibbonBar)
    ON_WM_NCCALCSIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSize CRibbonBar::CalcFixedLayout(BOOL fStretch, BOOL fHortz)
{
    CSize size = CToolBar::CalcFixedLayout(fStretch, fHortz);
    size.cy = m_ulRibbonHeight;
    return size;
}

// This is the Ribbon implementation that is done by an application.
// Applications have to implement IUIApplication and IUICommandHandler to set up communication with the Windows Ribbon.
class CApplication
    : public IUIApplication
    , public IUICommandHandler
{
public:

    // Static method to create an instance of the object.
    __checkReturn static HRESULT CreateInstance(__in CMainFrame* pFrame, __deref_out_opt CApplication **ppApplication)
    {
        if (!pFrame || !ppApplication)
        {
            return E_POINTER;
        }

        *ppApplication = NULL;
        HRESULT hr = S_OK;
       
        CApplication* pApplication = new CApplication();

        if (pApplication != NULL)
        {
            pApplication->m_pMainFrame = pFrame;
            // pApplication->m_cRef is 1.
            *ppApplication = pApplication;  
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        return hr;
    }

    // IUnknown methods.
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv)
        {
            return E_POINTER;
        }

        if (iid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IUnknown*>(static_cast<IUIApplication*>(this));
        }
        else if (iid == __uuidof(IUIApplication))
        {
            *ppv = static_cast<IUIApplication*>(this);
        }
        else if (iid == __uuidof(IUICommandHandler))
        {
            *ppv = static_cast<IUICommandHandler*>(this);
        }
        else 
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    // IUIApplication methods.
    STDMETHODIMP OnViewChanged(UINT32 /*nViewID*/, 
        __in UI_VIEWTYPE /*typeID*/, 
        __in IUnknown* pView, 
        UI_VIEWVERB verb, 
        INT32 /*uReasonCode*/)
    {
        HRESULT hr;

        // The Ribbon size has changed.
        if (verb == UI_VIEWVERB_SIZE)
        {
            IUIRibbon* pRibbon;
            pView->QueryInterface(IID_PPV_ARGS(&pRibbon));
            if (!pRibbon)
            {
                return E_FAIL;
            }

            UINT ulRibbonHeight;
            // Get the Ribbon height.
            hr = pRibbon->GetHeight(&ulRibbonHeight);

            pRibbon->Release();
            if (FAILED(hr))
            {
                return hr;
            }

            // Update the Ribbon bar so that the main frame can recalculate the child layout.
            m_pMainFrame->m_RibbonBar.SetRibbonHeight(ulRibbonHeight);
            m_pMainFrame->RecalcLayout();
        }

        return S_OK;
    }

    STDMETHODIMP OnCreateUICommand(UINT32 /*nCmdID*/, 
       __in UI_COMMANDTYPE /*typeID*/,
       __deref_out IUICommandHandler** ppCommandHandler)
    {
        // This application uses one command handler for all ribbon commands.
        return QueryInterface(IID_PPV_ARGS(ppCommandHandler));
    }

    STDMETHODIMP OnDestroyUICommand(UINT32 /*commandId*/, 
        __in UI_COMMANDTYPE /*typeID*/,  
        __in_opt  IUICommandHandler* /*commandHandler*/)
    {        
        return E_NOTIMPL;
    }

    // IUICommandHandler methods.

    // User action callback, with transient execution parameters.
    STDMETHODIMP Execute(UINT nCmdID,
        UI_EXECUTIONVERB /*verb*/, 
        __in_opt const PROPERTYKEY* key,
        __in_opt const PROPVARIANT* ppropvarValue,
        __in_opt IUISimplePropertySet* /*pCommandExecutionProperties*/)
    {       
        HRESULT hr = S_OK;
        switch(nCmdID)
        {
        case IDC_FONT:
            {
                if (key != NULL && ppropvarValue != NULL)
                {
                    IPropertyStore* pPropertyStore = NULL;
                    UIPropertyToInterface(*key, *ppropvarValue, &pPropertyStore);
                    if (pPropertyStore)
                    {
                        // Set font to the HTML editor.
                        hr = SetFont(m_pMainFrame, pPropertyStore);
                        pPropertyStore->Release();
                    }
                }
                break;
            }
        case IDC_MRULIST:
            {
                if (key != NULL && UI_PKEY_SelectedItem == *key)
                {
                    UINT uSelectedMRUItem = 0xffffffff;
                    if (ppropvarValue != NULL && SUCCEEDED(UIPropertyToUInt32(*key, *ppropvarValue, &uSelectedMRUItem)))
                    {
                        ASSERT(uSelectedMRUItem < RECENT_FILE_COUNT);
                        ::SendMessage(*m_pMainFrame, WM_COMMAND, uSelectedMRUItem + ID_FILE_MRU_FILE1, 0);
                    }
                }
                break;
            }
        default:
            ::SendMessage(*m_pMainFrame, WM_COMMAND, nCmdID, 0);
        }
        return hr;
    }

    STDMETHODIMP UpdateProperty(UINT32 nCmdID, 
        __in REFPROPERTYKEY key,
        __in_opt  const PROPVARIANT *currentValue,
        __out PROPVARIANT *newValue) 
    {   
        HRESULT hr = E_NOTIMPL;
        if(UI_PKEY_Enabled == key)
        {
            return UIInitPropertyFromBoolean(UI_PKEY_Enabled, GetStatus(nCmdID), newValue);
        }

        switch(nCmdID)
        {
        case IDC_MRULIST:
            if (UI_PKEY_Label == key)
            {
                WCHAR label[MAX_PATH];
                LoadStringW(AfxGetResourceHandle(), IDS_RECENTFILES, label, _countof(label));
                hr = UIInitPropertyFromString(UI_PKEY_Label, label, newValue);
            }
            else if (UI_PKEY_RecentItems == key)
            {
                hr = PopulateRibbonRecentItems(theApp.GetRecentFileList(), newValue);
            }
            break;
        case IDC_FONT:
            if (UI_PKEY_FontProperties == key && currentValue != NULL)
            {
                IPropertyStore* pPropertyStore;
                UIPropertyToInterface(UI_PKEY_FontProperties, *currentValue, &pPropertyStore);
                if (pPropertyStore)
                {
                    // Update the Ribbon font control.
                    hr = UpdateFont(m_pMainFrame, pPropertyStore);
                    pPropertyStore->Release();
                }
            }
            break;
        case IDC_VIEWSOURCE:
            if (UI_PKEY_BooleanValue == key)
            {
                hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, !(GetStatus(nCmdID) & OLECMDF_LATCHED), newValue);
            }
            break;
        default:
            if (UI_PKEY_BooleanValue == key)
            {
                hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, GetStatus(nCmdID) & OLECMDF_LATCHED, newValue);
            }
            break;
        }
        return hr;
    }

private:
    CApplication()
        : m_cRef(1) 
        , m_pMainFrame(NULL)
    {
    }

    BOOL GetStatus(UINT32 nCmdID)
    {
        CHTMLEdView *pView = (CHTMLEdView*)m_pMainFrame->GetActiveView();
        if (!pView)
            return 0;

        return pView->GetCommandStatus(nCmdID);
    }

private:
    LONG m_cRef;                        // Reference count.
    CMainFrame* m_pMainFrame;
};

__checkReturn HRESULT InitRibbon(__in CMainFrame* pMainFrame, __deref_out_opt IUnknown** ppFramework)
{
    if (ppFramework == NULL)
    {
        return E_POINTER;
    }

    // Create the IUIFramework instance.
    IUIFramework* pFramework = NULL;
    HRESULT hr = ::CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFramework));
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Instantiate the CApplication object.
    CApplication* pAppObject = NULL;
    IUIApplication* pApplication = NULL;
    hr = CApplication::CreateInstance(pMainFrame, &pAppObject);
    
    if (FAILED(hr))
    {
        goto done;
    }

#pragma warning( disable : 6011)    // pAppObject cannot be NULL.
    hr = pAppObject->QueryInterface(IID_PPV_ARGS(&pApplication));
#pragma warning( default : 6011)    

    if (FAILED(hr))
    {
        goto done;
    }

    hr = pFramework->Initialize(*pMainFrame, pApplication);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pFramework->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
    if (FAILED(hr))
    {
        goto done;
    }

    // Return IUIFramework interface to the caller.
    hr = pFramework->QueryInterface(ppFramework);

done:

    if (pApplication)
    {
        pApplication->Release();
    }

    if (pAppObject)
    {
        pAppObject->Release();
    }

    if (pFramework)
    {
        pFramework->Release();
    }

    return hr;
}

void DestroyRibbon(__in IUnknown* pFramework) 
{
    IUIFramework* pUIFramework;
    pFramework->QueryInterface(IID_PPV_ARGS(&pUIFramework));

    ASSERT(pFramework != NULL);

    // Destroy the Ribbon.
    pUIFramework->Destroy();
    pUIFramework->Release();
}

HRESULT SetModes(__in IUnknown* pFramework, UINT modes)
{
    IUIFramework* pUIFramework;
    pFramework->QueryInterface(IID_PPV_ARGS(&pUIFramework));
    if (pUIFramework == NULL)
    {
        return E_FAIL;
    }

    // Change the modes.
    HRESULT hr = pUIFramework->SetModes(modes);
    pUIFramework->Release();

    return hr;
}

HRESULT RibbonInvalidate(__in IUnknown* pFramework)
{
    IUIFramework* pUIFramework;
    pFramework->QueryInterface(IID_PPV_ARGS(&pUIFramework));
    if (pUIFramework == NULL)
    {
        return E_FAIL;
    }

    // Invalidate the Font control properties.
    HRESULT hr = pUIFramework->InvalidateUICommand(IDC_FONT, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_FontProperties);

    // Invalidate the control values, for example, the checked value of toggle buttons and enabled state.
    hr = pUIFramework->InvalidateUICommand(0, UI_INVALIDATIONS_STATE|UI_INVALIDATIONS_VALUE, NULL);

    UNREFERENCED_PARAMETER(hr);

    pUIFramework->Release();
    return S_OK;
}



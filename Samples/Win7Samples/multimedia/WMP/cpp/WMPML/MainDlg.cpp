// MainDlg.cpp : Implementation of CMainDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "MainDlg.h"
#include "QueryDlg.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

// Node string for Advanced Query node
WCHAR* g_wszAdvQuery = L"Advanced Query";
// Node string for blank metadata
WCHAR* g_szUnknown = L"Unknown";

// Media types
WCHAR* g_pwszMediaTypes[] = {
    L"audio",
    L"photo",
    L"video",
    L"playlist",
    L"other"
};

// Note that attribute values retrieved by using the Player object model
// might be returned in a different form or format than you see in
// in Windows Media Player. For example, the Rating attribute returns a 
// value between 0 and 99 when using the SDK, rather than a "star" rating.
// (For details about how to convert the 0-99 value to "stars", see 
// UserRating Attribute in the Attribute Reference section of the documentation.

// Params to build Audio tree
NODEPARAM g_pAudioNodes[] = 
{
    { L"Artist", L"Artist" },
    { L"Album", L"WM/AlbumTitle" },
    { L"Songs", NULL },
    { L"Genre", L"Genre" },
    { L"Year", L"ReleaseDateYear" },
    { L"Rating", L"UserRating" }
};

// Params to build Photo tree
NODEPARAM g_pPhotoNodes[] = 
{
    { L"All Pictures", NULL },
    { L"Keywords", L"Comment" },
    { L"Date Taken", L"DateTakenYear" },
    { L"Rating", L"UserRating" },
    { L"Folder", L"Folder" }
};

// Params to build Video tree
NODEPARAM g_pVideoNodes[] = 
{
    { L"All Video", NULL },
    { L"Actors", L"Author" },
    { L"Genre", L"Genre" },
    { L"Rating", L"UserRating" }
};

// Params to build Playlist tree
NODEPARAM g_pPlaylistNodes[] = 
{
    { L"My Playlists", NULL },
};

// Params to build Other tree
NODEPARAM g_pOtherNodes[] = 
{
    { L"Other Media", NULL },
    { L"Folder", L"Folder" }
};

// Launch the dialog when the application launchs
extern "C" int WINAPI wWinMain(
    HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/,
    LPWSTR /*lpCmdLine*/, 
    int /*nShowCmd*/)
{
    CoInitialize(NULL);
    _Module.Init(ObjectMap, hInstance, NULL);
    CMainDlg dlg;
    dlg.DoModal();
    CoUninitialize();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainDlg

/***********************************************************************
* Constructor
***********************************************************************/
CMainDlg::CMainDlg()
{
    m_pView = NULL;
    // Initial selection is Audio
    m_mtCurMediaType = AUDIO;
    m_cSchemaCount = sizeof(g_pAudioNodes) / sizeof(g_pAudioNodes[0]);
    m_pnpNodeParams = g_pAudioNodes;
    m_bstrMediaType = g_pwszMediaTypes[m_mtCurMediaType];
    
    // Use prenode to keep track current clicking on the tree
    m_hPreNode = NULL;
}

/***********************************************************************
* Destructor
***********************************************************************/
CMainDlg::~CMainDlg()
{
}

/***********************************************************************
* OnInitDialog
* 
* Initialize member variables of handlers
* Initialize WMP OCX
***********************************************************************/
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::InitCommonControls();
    AtlAxWinInit();
    
    // dialog items
    m_hLibTree = GetDlgItem(IDC_TREE);
    m_hDetailList = GetDlgItem(IDC_DETAILLIST);
    
    // Create a WMP OCX
    CreateWmpOcx();
    
    // Fill in combo box for media type
    HWND    hMediaTypeList = GetDlgItem(IDC_MEDIATYPELIST);
    SendMessage(hMediaTypeList, CB_ADDSTRING, 0, (LPARAM)L"Music");
    SendMessage(hMediaTypeList, CB_ADDSTRING, 0, (LPARAM)L"Pictures");
    SendMessage(hMediaTypeList, CB_ADDSTRING, 0, (LPARAM)L"Video");
    SendMessage(hMediaTypeList, CB_ADDSTRING, 0, (LPARAM)L"Playlists");
    SendMessage(hMediaTypeList, CB_ADDSTRING, 0, (LPARAM)L"Other Media");
    SendMessage(hMediaTypeList, CB_SETCURSEL, 0, (LPARAM)m_mtCurMediaType);
    
    // Use IWMPLibraryServices to enumerate the libries and fill in combo box
    HRESULT                         hr = E_POINTER;
    LONG                            lLibCount = 0;
    HWND                            hLibList = GetDlgItem(IDC_LIBLIST);
    CComPtr<IWMPLibraryServices>    spLibSvc;
    
    if(m_spPlayer != NULL)
    {
        hr = m_spPlayer->QueryInterface(&spLibSvc);
    }
    if(SUCCEEDED(hr) && spLibSvc)
    {
        hr = spLibSvc->getCountByType(wmpltAll, &lLibCount);
    }
    if(SUCCEEDED(hr))
    {
        for(LONG i = 0; i < lLibCount; i++)
        {
            CComPtr<IWMPLibrary>    spLib;
            CComBSTR                bstrLibName;
            
            hr = spLibSvc->getLibraryByType(wmpltAll, i , &spLib);
            if(SUCCEEDED(hr) && spLib)
            {
                hr = spLib->get_name(&bstrLibName);
            }
            if(SUCCEEDED(hr))
            {
                SendMessage(hLibList, CB_ADDSTRING, 0, (LPARAM)bstrLibName.m_str);
            }
        }
    }
    SendMessage(hLibList, CB_SETCURSEL, 0, 0);
    
    // Set WordWheel text to Search...
    SetDlgItemText(IDC_WORDWHEEL, L"Search...");
    
    // Set m_spMC to correct mediaCollection of currently selected library
    UpdateCurMC();

    // Now we try to build the tree
    BuildLibTree();
    
    return 0;
}

/***********************************************************************
* OnDestroy
* Release WMP OCX
***********************************************************************/
LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // Release WMP OCX
    m_spMC = NULL;
    m_spPlayer = NULL;

    if(m_pView != NULL)
    {
        delete m_pView;
        m_pView = NULL;
    }
    return 0;
}

/***********************************************************************
* OnOK
* User closes the dialog
***********************************************************************/
LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnCancel
* User closes the dialog
***********************************************************************/
LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnChangeMediaType
* Called when selection in MediaType combo box is changed
***********************************************************************/
LRESULT CMainDlg::OnChangeMediaType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // We use m_pnpNodeParams as a pointer to current schema parameter
    // This function simple change what it points and call BuildTree()
    m_mtCurMediaType = (MEDIATYPE)SendDlgItemMessage(IDC_MEDIATYPELIST, CB_GETCURSEL, 0, 0);

    switch(m_mtCurMediaType)
    {
    case AUDIO:
        m_cSchemaCount = sizeof(g_pAudioNodes) / sizeof(g_pAudioNodes[0]);
        m_pnpNodeParams = g_pAudioNodes;
        break;
    case PHOTO:
        m_cSchemaCount = sizeof(g_pPhotoNodes) / sizeof(g_pPhotoNodes[0]);
        m_pnpNodeParams = g_pPhotoNodes;
        break;
    case VIDEO:
        m_cSchemaCount = sizeof(g_pVideoNodes) / sizeof(g_pVideoNodes[0]);
        m_pnpNodeParams = g_pVideoNodes;
        break;
    case PLAYLIST:
        m_cSchemaCount = sizeof(g_pPlaylistNodes) / sizeof(g_pPlaylistNodes[0]);
        m_pnpNodeParams = g_pPlaylistNodes;
        break;
    case OTHER:
        m_cSchemaCount = sizeof(g_pOtherNodes) / sizeof(g_pOtherNodes[0]);
        m_pnpNodeParams = g_pOtherNodes;
        break;
    default:
        break;
    }
    
    m_bstrMediaType = g_pwszMediaTypes[m_mtCurMediaType];
    
    BuildLibTree();
    return 0;
}

/***********************************************************************
* OnChangeLib
* Called when selection in Library combo box is changed
***********************************************************************/
LRESULT CMainDlg::OnChangeLib(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // Set m_spMC to correct mediaCollection of currently selected library
    UpdateCurMC();
    
    BuildLibTree();

    return 0;
}

/***********************************************************************
* OnClickTree
* Called when user clicks on the tree view
***********************************************************************/
LRESULT CMainDlg::OnClickTree(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
    POINT p;
    
    TVHITTESTINFO   hitInfo;
    HTREEITEM       hNode = NULL;
    TVITEM          tvi;
    WCHAR           szVal[MAX_PATH];

    HRESULT hr = S_OK;
    
    // Find out the hit point
    GetCursorPos(&p);
    ::ScreenToClient(m_hLibTree, &p);
    hitInfo.pt = p;
    hNode = TreeView_HitTest(m_hLibTree, &hitInfo);
    
    // Proceed only when user clicks on the node label
    if(hNode && hitInfo.flags == TVHT_ONITEMLABEL && m_spMC)
    {
        tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
        tvi.hItem = hNode;
        tvi.pszText = szVal;
        tvi.cchTextMax = MAX_PATH - 1;

        if(TreeView_GetItem(m_hLibTree, &tvi))
        {
            // Reset Detail List
            SendMessage(m_hDetailList, LB_RESETCONTENT, 0, 0);
            
            CComPtr<IWMPQuery>              spQuery;
            
            // If the param is -1, it's a Advanced Query node
            // So we bring out the Query dialog for more conditions
            if(tvi.lParam < 0)
            {
                CQueryDlg dlg(m_spMC);
                dlg.DoModal();
                hr = dlg.GetQuery(&spQuery);
                SetDlgItemText(IDC_DETAILTEXT, g_wszAdvQuery);
            }
            else if(m_hPreNode != hNode)
            // Otherwise the query condition is based on current schema node
            {
                WCHAR*      szAttName = NULL;
                if(tvi.lParam < m_cSchemaCount)
                {
                    szAttName = m_pnpNodeParams[tvi.lParam].szAttrName;
                }
                
                // If szAttrName is not NULL, build a query for that attribute
                if(szAttName)
                {
                    m_spMC->createQuery(&spQuery);
                    if(spQuery)
                    {
                        spQuery->addCondition(
                            CComBSTR(szAttName),
                            CComBSTR(L"Equals"),
                            CComBSTR(tvi.pszText)
                       );
                    }
                }
                
                // Show current node path
                if(szAttName && TreeView_GetParent(m_hLibTree, tvi.hItem))
                {
                    WCHAR szText[MAX_PATH];
                    StringCbPrintf(szText, sizeof(szText), L"%s > %s", szAttName, tvi.pszText);
                    SetDlgItemText(IDC_DETAILTEXT, szText);
                }
                else
                {
                    SetDlgItemText(IDC_DETAILTEXT, tvi.pszText);
                }
            }

            if(SUCCEEDED(hr))
            {            
                // Now we use the query to get stringCollection and show it.
                ShowQueryResult(spQuery);
            }
            
            // Remember current node
            m_hPreNode = hNode;
        }
    }
    
    // Let tree view do its job for UI of the tree
    bHandled = FALSE;
    return SUCCEEDED(hr)?0:1;
}


/***********************************************************************
                Private functions
***********************************************************************/

/***********************************************************************
* CreateWmpOcx
* Create a hidden WMP OCX
***********************************************************************/
HRESULT CMainDlg::CreateWmpOcx()
{
    CComPtr<IAxWinHostWindow>           spHost;
    RECT                                rect = {0, 0, 0, 0};
    HRESULT                             hr = E_FAIL;
    CComPtr<IObjectWithSite>            spHostObject;
    CComObject<CWMPRemoteHost>          *pRemoteHost = NULL;
    IUnknown                            *punkSite = NULL;

    m_pView = new CAxWindow();
    hr = m_pView ? S_OK : AtlHresultFromLastError();
    
    if(SUCCEEDED(hr) && m_pView)
    {
        m_pView->Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE);
        if(::IsWindow(m_pView->m_hWnd))
        {
            hr = m_pView->QueryHost(IID_IObjectWithSite, (void **)&spHostObject);
        }
        
        if(SUCCEEDED(hr) && spHostObject.p)
        {
            hr = CComObject<CWMPRemoteHost>::CreateInstance(&pRemoteHost);
            if(pRemoteHost)
            {
                hr = ((IUnknown *)(IWMPRemoteMediaServices *)pRemoteHost)->QueryInterface(IID_IUnknown, (void **)&punkSite);
            }
        }
        
        if(SUCCEEDED(hr) && punkSite)
        {
            hr = spHostObject->SetSite(punkSite);
            punkSite->Release();
            punkSite = NULL;
        }
        
        if(SUCCEEDED(hr))
        {
            hr = m_pView->QueryHost(&spHost);
        }
        
        if(SUCCEEDED(hr) && spHost.p)
        {
            hr = spHost->CreateControl(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}", m_pView->m_hWnd, 0);
        }

        if(SUCCEEDED(hr))
        {
            hr = m_pView->QueryControl(&m_spPlayer);
        }
    }
    
    return hr;
}


/***********************************************************************
* BuildLibTree
* Build the tree view for current library. 
* Try to simulate the Windows Media Player library
***********************************************************************/
void CMainDlg::BuildLibTree()
{
    TreeView_DeleteAllItems(m_hLibTree);

    HTREEITEM hParentNode = NULL;
    TVINSERTSTRUCT tvins;
    tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvins.hParent = NULL;
    tvins.hInsertAfter = TVI_ROOT;
    
    // Add the first node as Advanced Query so we can click on it
    tvins.item.lParam = (LPARAM)-1;
    tvins.item.pszText = g_wszAdvQuery;
    tvins.item.cchTextMax = lstrlen(tvins.item.pszText) + 1;
    hParentNode = TreeView_InsertItem(m_hLibTree, &tvins);
    
    // Now add nodes for schemas
    for(int iSchemaIndex = 0; iSchemaIndex < m_cSchemaCount; iSchemaIndex++)
    {
        tvins.hInsertAfter = TVI_ROOT;
        tvins.hParent = TVI_ROOT;
        tvins.item.lParam = (LPARAM)iSchemaIndex;
        tvins.item.pszText = m_pnpNodeParams[iSchemaIndex].szNodeName;
        tvins.item.cchTextMax = lstrlen(tvins.item.pszText) + 1;
        hParentNode = TreeView_InsertItem(m_hLibTree, &tvins);
    
        // If fEnumValues is true, we enumerate the values related to szNodeName
        // and add those values as subnodes 
        if(hParentNode && m_pnpNodeParams[iSchemaIndex].szAttrName)
        {
            HRESULT                         hr = E_FAIL;
            CComBSTR                        bstrAttrName = m_pnpNodeParams[iSchemaIndex].szAttrName;
            CComPtr<IWMPStringCollection>   spSC;
            LONG                            lCount = 0;
            
            hr = m_spMC->getStringCollectionByQuery(
                bstrAttrName,
                NULL,
                m_bstrMediaType,
                bstrAttrName,
                VARIANT_TRUE,
                &spSC);
            if(SUCCEEDED(hr) && spSC)
            {
                hr = spSC->get_count(&lCount);
            }
            if(SUCCEEDED(hr))
            {
                tvins.hInsertAfter = TVI_LAST;
                tvins.hParent = hParentNode;
                tvins.item.lParam = (LPARAM)iSchemaIndex;
                for(LONG i = 0; i < lCount; i++)
                {
                    CComBSTR                bstrVal;
                    hr = spSC->item(i, &bstrVal);
                    if(SUCCEEDED(hr))
                    {
                        tvins.item.pszText = bstrVal.Length() ? bstrVal : g_szUnknown;
                        tvins.item.cchTextMax = lstrlen(tvins.item.pszText);
                        TreeView_InsertItem(m_hLibTree, &tvins);
                    }
                }
            }
    
        }        
    }
}

/***********************************************************************
* UpdateCurMC
* Each library in IWMPLibraryServices has its own mediaCollection.
* This application only shows schema tree for current library
* So we need to update m_spMC for current library
***********************************************************************/
HRESULT CMainDlg::UpdateCurMC()
{
    HRESULT                         hr = S_OK;
    LONG                            lLidID = -1;
    CComPtr<IWMPLibrary>            spLib;
    CComPtr<IWMPLibraryServices>    spLibSvc;
            
    lLidID      = SendDlgItemMessage(IDC_LIBLIST, CB_GETCURSEL, 0, 0);
    
    if(m_spPlayer != NULL)
    {
        hr = m_spPlayer->QueryInterface(&spLibSvc);
    }
    
    if(spLibSvc && lLidID >= 0)
    {
        hr = spLibSvc->getLibraryByType(wmpltAll, lLidID, &spLib);
    }
    
    if(SUCCEEDED(hr) && spLib)
    {
        m_spMC = NULL;
        CComPtr<IWMPMediaCollection>    spTmpMC;
        hr = spLib->get_mediaCollection(&spTmpMC);
        if(SUCCEEDED(hr) && spTmpMC)
        {
            spTmpMC->QueryInterface(&m_spMC);
        }
        
    }
    
    return hr;
}

/***********************************************************************
* ShowQueryResult
* Run the query to get stringCollection and show it in the listbox
***********************************************************************/
void CMainDlg::ShowQueryResult(IWMPQuery* pQuery)
{
    if(m_spMC)
    {
        CComPtr<IWMPStringCollection>   spSC;
        // Now we use the query to get stringCollection and show it.
        m_spMC->getStringCollectionByQuery(
            CComBSTR(L"Title"),
            pQuery,
            m_bstrMediaType,
            CComBSTR(L"Title"),
            VARIANT_TRUE,
            &spSC);
        
        if(spSC)
        {
            LONG    lCount = 0;
            spSC->get_count(&lCount);
            for(LONG i = 0; i < lCount; i++)
            {
                CComBSTR    bstrVal;
                spSC->item(i, &bstrVal);
                if(bstrVal.m_str)
                {
					SendMessage(m_hDetailList, LB_ADDSTRING, 0, (LPARAM)bstrVal.m_str);
                }
            }
        }
    }
}


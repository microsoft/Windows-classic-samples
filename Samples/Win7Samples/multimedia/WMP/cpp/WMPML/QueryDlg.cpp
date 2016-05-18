// QueryDlg.cpp : Implementation of CQueryDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "QueryDlg.h"

// Commonly used metadata names
WCHAR* g_pszAttrNames[] = 
{
    L"Abstract",
    L"AcquisitionTime",
    L"AlbumArtistSortOrder",
    L"AlbumID",
    L"AlbumIDAlbumArtist",
    L"AlbumTitleSortOrder",
    L"AudioBitrate",
    L"Author",
    L"AuthorSortOrder",
    L"AverageLevel",
    L"Bitrate",
    L"BuyNow",
    L"BuyTickets",
    L"CallLetters",
    L"CDTrackEnabled",
    L"Comment",
    L"Copyright",
    L"CurrentBitrate",
    L"Description",
    L"Duration",
    L"DVDID",
    L"Event",
    L"FileSize",
    L"FileType",
    L"FourCC",
    L"FrameRate",
    L"Frequency",
    L"IsVBR",
    L"LeadPerformer",
    L"Location",
    L"MediaType",
    L"ModifiedBy",
    L"MoreInfo",
    L"PeakValue",
    L"Provider",
    L"ProviderLogoURL",
    L"ProviderURL",
    L"RadioBand",
    L"RadioFormat",
    L"RatingOrg",
    L"RecordingTime",
    L"ReleaseDate",
    L"RequestState",
    L"SourceURL",
    L"SyncState",
    L"SyncOnly",
    L"Title",
    L"TitleSortOrder",
    L"TotalDuration",
    L"TrackingID",
    L"UserCustom1",
    L"UserCustom2",
    L"UserEffectiveRating",
    L"UserLastPlayedTime",
    L"UserPlayCount",
    L"UserPlaycountAfternoon",
    L"UserPlaycountEvening",
    L"UserPlaycountMorning",
    L"UserPlaycountNight",
    L"UserPlaycountWeekday",
    L"UserPlaycountWeekend",
    L"UserRating",
    L"UserServiceRating",
    L"VideoBitrate",
    L"WM/AlbumArtist",
    L"WM/AlbumTitle",
    L"WM/Category",
    L"WM/Composer",
    L"WM/Conductor",
    L"WM/ContentDistributor",
    L"WM/ContentGroupDescription",
    L"WM/Director",
    L"WM/EncodingTime",
    L"WM/Genre",
    L"WM/GenreID",
    L"WM/InitialKey",
    L"WM/Language",
    L"WM/Lyrics",
    L"WM/MCDI",
    L"WM/MediaClassPrimaryID",
    L"WM/MediaClassSecondaryID",
    L"WM/MediaOriginalBroadcastDateTime",
    L"WM/MediaOriginalChannel",
    L"WM/MediaStationName",
    L"WM/Mood",
    L"WM/OriginalAlbumTitle",
    L"WM/OriginalArtist",
    L"WM/OriginalLyricist",
    L"WM/ParentalRating",
    L"WM/PartOfSet",
    L"WM/Period",
    L"WM/Producer",
    L"WM/ProtectionType",
    L"WM/Provider",
    L"WM/ProviderRating",
    L"WM/ProviderStyle",
    L"WM/Publisher",
    L"WM/SubscriptionContentID",
    L"WM/SubTitle",
    L"WM/SubTitleDescription",
    L"WM/TrackNumber",
    L"WM/UniqueFileIdentifier",
    L"WM/VideoHeight",
    L"WM/VideoWidth",
    L"WM/WMCollectionGroupID",
    L"WM/WMCollectionID",
    L"WM/WMContentID",
    L"WM/Writer",
    L"WM/Year",
    L"WM/Lyrics_Synchronised",
    L"WM/Picture",
    L"WM/UserWebURL"
};

// Supported query operators
WCHAR* g_pszQueryOperators[] = 
{\
    L"BeginsWith",
    L"Contains",
    L"Equals",
    L"GreaterThan",
    L"GreaterThanOrEquals",
    L"LessThan",
    L"LessThanOrEquals",
    L"NotBeginsWith",
    L"NotContains",
    L"NotEquals"
};

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg

/***********************************************************************
* Constructor
***********************************************************************/
CQueryDlg::CQueryDlg(IWMPMediaCollection2* pMC)
{
    m_spMC = pMC;
    
    if(m_spMC.p)
    {
        m_spMC->createQuery(&m_spQuery);
    }
}

/***********************************************************************
* Destructor
***********************************************************************/
CQueryDlg::~CQueryDlg()
{
}

/***********************************************************************
* OnInitDialog
***********************************************************************/
LRESULT CQueryDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_hQueryList = GetDlgItem(IDC_QUERYLIST);
    
    // Add headers to Query Condition List
    LRESULT     dwStyle = 0;
    LVCOLUMN    lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT;
    lvc.iSubItem = 0;
    lvc.pszText = L"Logic";
    lvc.cx = 40;
    ListView_InsertColumn(m_hQueryList, 0, &lvc);
    lvc.pszText = L"Name";
    lvc.cx = 240;
    ListView_InsertColumn(m_hQueryList, 1, &lvc);
    lvc.pszText = L"Operator";
    lvc.cx = 120;
    ListView_InsertColumn(m_hQueryList, 2, &lvc);
    lvc.pszText = L"Value";
    lvc.cx = 300;
    ListView_InsertColumn(m_hQueryList, 3, &lvc);
    ::SendMessage(m_hQueryList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);

    // Add common metadata names to the dropdown lists
    HWND hQueryName = GetDlgItem(IDC_QUERYNAME);
    for(int i = 0; i < sizeof(g_pszAttrNames) / sizeof(g_pszAttrNames[0]); i++)
    {
        ::SendMessage(hQueryName,  CB_ADDSTRING, 0, (LPARAM)g_pszAttrNames[i]);
    }
    ::SendMessage(hQueryName,  CB_SETCURSEL, 46, 0);
    
    // Add query operators to the dropdown lists
    HWND    hQueryOpr = GetDlgItem(IDC_QUERYOPR);
    for(int i = 0; i < sizeof(g_pszQueryOperators) / sizeof(g_pszQueryOperators[0]); i++)
    {
        ::SendMessage(hQueryOpr, CB_ADDSTRING, 0, (LPARAM)g_pszQueryOperators[i]);
    }
    ::SendMessage(hQueryOpr, CB_SETCURSEL, 1, 0);

    return 0;
}

/***********************************************************************
* OnCancel
* User closes the dialog
***********************************************************************/
LRESULT CQueryDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // When it's cancelled, only return NULL query pointer
    if(m_spQuery)
    {
        m_spQuery.Release();
        m_spQuery = NULL;
    }
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnAddCondition
* Called when use clicks on Add a Condition button
***********************************************************************/
LRESULT CQueryDlg::OnAddCondition(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
    HRESULT hr = S_OK;

    if(m_spQuery)
    {
        CComBSTR bstrAttr, bstrOperator, bstrValue;


        GetDlgItemText(IDC_QUERYNAME,  bstrAttr.m_str);
        GetDlgItemText(IDC_QUERYOPR,   bstrOperator.m_str);
        GetDlgItemText(IDC_QUERYVAL,   bstrValue.m_str);

        hr = (m_spQuery)->addCondition(bstrAttr, bstrOperator, bstrValue);
        
        if(SUCCEEDED(hr))
        {
            // Show the new condition
            // Column 0: attribute name
            LVITEM lvi;
            int index;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = ListView_GetItemCount(m_hQueryList);
            lvi.iSubItem = 0;

            lvi.pszText = L"AND";
            lvi.cchTextMax = lstrlen(lvi.pszText);
            index = ListView_InsertItem(m_hQueryList, &lvi);
            ListView_SetItemText(m_hQueryList, index, 1, bstrAttr);
            ListView_SetItemText(m_hQueryList, index, 2, bstrOperator);
            ListView_SetItemText(m_hQueryList, index, 3, bstrValue);
        }
    }

    return SUCCEEDED(hr)?0:1;
}

/***********************************************************************
* OnAddGroup
* Called when use clicks on Add a Group button
***********************************************************************/
LRESULT CQueryDlg::OnAddGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    HRESULT hr = S_OK;
    if(m_spQuery)
    {
        hr = (m_spQuery)->beginNextGroup();
        
        if(SUCCEEDED(hr))
        {
            // Show the new condition
            // Column 0: attribute name
            LVITEM lvi;
            int index;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = ListView_GetItemCount(m_hQueryList);
            lvi.iSubItem = 0;
            lvi.pszText = L"OR";
            lvi.cchTextMax = lstrlen(lvi.pszText);
            index = ListView_InsertItem(m_hQueryList, &lvi);
        }
    }
    return SUCCEEDED(hr)?0:1;
}

/***********************************************************************
* OnShowSC
* Called when Show StringCollection button is clicked. 
* Close the dialog
***********************************************************************/
LRESULT CQueryDlg::OnShowSC(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnStartOver
* Removes all existing conditions
***********************************************************************/
LRESULT CQueryDlg::OnStartOver(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // We basically re-create a new query
    if(m_spQuery.p)
    {
         m_spQuery.Release();
         m_spQuery = NULL;
    }

    if(m_spMC.p)
    {
        m_spMC->createQuery(&m_spQuery);
    }
    
    // Clean up the condition list
    ListView_DeleteAllItems(m_hQueryList);
    
    return 0;
}

/***********************************************************************
* GetQuery
* Returns the query result
***********************************************************************/
HRESULT CQueryDlg::GetQuery(IWMPQuery** ppQuery)
{
    HRESULT hr = S_OK;

    if(m_spQuery.p && ppQuery)
    {
        m_spQuery.CopyTo(ppQuery);
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

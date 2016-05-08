// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedtransformdialog.h"

#include <dmo.h>
#include <assert.h>

DMOCategory CTedTransformDialog::ms_categories[9] = 
{
    { MFT_CATEGORY_AUDIO_DECODER,   IDS_CATEGORY_AUDIO_DECODERS },
    { MFT_CATEGORY_AUDIO_EFFECT,    IDS_CATEGORY_AUDIO_EFFECTS },
    { MFT_CATEGORY_AUDIO_ENCODER,   IDS_CATEGORY_AUDIO_ENCODERS },
    { MFT_CATEGORY_DEMULTIPLEXER,   IDS_CATEGORY_DEMUX },
    { MFT_CATEGORY_MULTIPLEXER,     IDS_CATEGORY_MUX },
    { MFT_CATEGORY_OTHER,           IDS_CATEGORY_OTHER },
    { MFT_CATEGORY_VIDEO_DECODER,   IDS_CATEGORY_VIDEO_DECODERS },
    { MFT_CATEGORY_VIDEO_EFFECT,    IDS_CATEGORY_VIDEO_EFFECTS },
    { MFT_CATEGORY_VIDEO_ENCODER,   IDS_CATEGORY_VIDEO_ENCODERS }
};

CTedTransformDialog::~CTedTransformDialog()
{
    for(DWORD i = 0; i < m_Activates.GetCount(); i++)
    {
        m_Activates[i]->Release();
    }
}

IMFActivate* CTedTransformDialog::GetChosenActivate() const 
{
    assert(m_nChosenIndex < m_Activates.GetCount());

    return m_Activates.GetAt(m_nChosenIndex);
}

CAtlStringW CTedTransformDialog::GetChosenName() const 
{
    assert(m_nChosenIndex < m_strNames.GetCount());

    return m_strNames.GetAt(m_nChosenIndex);
}

LRESULT CTedTransformDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_hList = GetDlgItem(IDC_TRANSFORMTREE);

    for(int i = 0; i < 9; i++) 
    {
        PopulateCategory(ms_categories[i]);
    }

    return 0;
}

LRESULT CTedTransformDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HTREEITEM hSelectedItem = TreeView_GetSelection(m_hList);

    TVITEM selectedItem;
    selectedItem.mask = TVIF_PARAM;
    selectedItem.hItem = hSelectedItem;

    TreeView_GetItem(m_hList, &selectedItem);
    
    m_nChosenIndex = (unsigned int) selectedItem.lParam;

    return 0;
}

LRESULT CTedTransformDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    HTREEITEM hSelectedItem = TreeView_GetSelection(m_hList);

    TVITEM selectedItem;
    selectedItem.mask = TVIF_PARAM;
    selectedItem.hItem = hSelectedItem;

    TreeView_GetItem(m_hList, &selectedItem);
    
    m_nChosenIndex = (unsigned int) selectedItem.lParam;
    
    EndDialog(IDOK);
    
    return 0;
}

LRESULT CTedTransformDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    EndDialog(IDCANCEL);

    return 0;
}

void CTedTransformDialog::PopulateCategory(DMOCategory& category) 
{
    TVINSERTSTRUCT treeInserter;
    treeInserter.hParent = TVI_ROOT;
    treeInserter.hInsertAfter = TVI_FIRST;

    CAtlString strName = LoadAtlString(category.m_nID);
    TVITEM item;
    item.mask = TVIF_TEXT;
    item.pszText = strName.GetBuffer();
    item.cchTextMax = strName.GetLength();

    treeInserter.item = item;
    HTREEITEM hBaseItem = (HTREEITEM) TreeView_InsertItem(m_hList, &treeInserter);

    assert(hBaseItem != NULL);

    treeInserter.hParent = hBaseItem;
    item.mask = TVIF_TEXT | TVIF_PARAM;

    DMO_PARTIAL_MEDIATYPE mediaType;
    CComPtr<IEnumDMO> spDMOList;

    mediaType.type = GUID_NULL;
    mediaType.subtype = GUID_NULL;
    
    IMFActivate** ppActivates = NULL;
    UINT32 cMFTs = 0;
    ::MFTEnumEx(category.m_GUID, MFT_ENUM_FLAG_ALL, NULL, NULL, &ppActivates, &cMFTs);

    for(DWORD i = 0; i < cMFTs; i++)
    {
        m_Activates.Add(ppActivates[i]);

        LPWSTR pszName = NULL;
        ppActivates[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &pszName, NULL);
        
        m_strNames.Add(CAtlStringW(pszName));
        item.pszText = m_strNames.GetAt(m_strNames.GetCount() - 1).GetBuffer();
        item.lParam = (LPARAM)  m_Activates.GetCount() - 1;

        treeInserter.item = item;

        TreeView_InsertItem(m_hList, &treeInserter);
        
        CoTaskMemFree(pszName);
    }

    CoTaskMemFree(ppActivates);
}
        
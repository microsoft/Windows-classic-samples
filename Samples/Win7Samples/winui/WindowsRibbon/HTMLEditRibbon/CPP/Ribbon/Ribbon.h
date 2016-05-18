// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(RIBBON_H__F563231A_2848_47E6_B261_15B56A698695__INCLUDED_)
#define RIBBON_H__F563231A_2848_47E6_B261_15B56A698695__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Forward declaration.
class CMainFrame;

// Declaration of CRibbonBar class which inherits from CToolBar. 
// This placeholder bar helps the MFC CMainFrame to calculate the view layout whenever the Ribbon is shown or hidden.
class CRibbonBar: public CToolBar
{
public:
    CRibbonBar()
        : m_ulRibbonHeight(0)
    {
    }


    // WM_NCCALCSIZE handler.
    void OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* /*lpncsp*/) 
    {
        // Do nothing.
    }

    // Override CalcFixedLayout.
    CSize CalcFixedLayout(BOOL fStretch, BOOL fHortz);

    // Set the Ribbon height.
    void SetRibbonHeight(UINT ulRibbonHeight)
    {
        m_ulRibbonHeight = ulRibbonHeight;
    }

    DECLARE_MESSAGE_MAP()

private:
    UINT m_ulRibbonHeight;
};

// Initialize Ribbon.
__checkReturn HRESULT InitRibbon(__in CMainFrame* pMainFrame, __deref_out IUnknown** ppFramework);

// Destroy Ribbon.
void DestroyRibbon(__in IUnknown* pFramework);

// Set Ribbon modes.
HRESULT SetModes(__in IUnknown* pFramework, UINT modes);

// Invalidate Ribbon controls.
HRESULT RibbonInvalidate(__in IUnknown* pFramework);

#endif // !defined(RIBBON_H__F563231A_2848_47E6_B261_15B56A698695__INCLUDED_)

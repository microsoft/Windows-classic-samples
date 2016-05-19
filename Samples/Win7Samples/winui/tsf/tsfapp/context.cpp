/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Context.cpp

   Description:   ITfContextOwnerCompositionSink Implementation

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "TSFEdit.h"

/**************************************************************************

   CTSFEditWnd::OnStartComposition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
{
    OutputDebugString(TEXT("CTSFEditWnd::OnStartComposition\n"));

    *pfOk = TRUE;

    if(m_cCompositions >= MAX_COMPOSITIONS)
    {
        //can't handle any more compositions
        *pfOk = FALSE;
        return S_OK;
    }

    m_cCompositions++;

    //increment the reference count
    pComposition->AddRef();

    //find an empty slot to put the composition pointer in
    ULONG i;
    for(i = 0; i < MAX_COMPOSITIONS; i++)
    {
        if(NULL == m_rgCompositions[i])
        {
            m_rgCompositions[i] = pComposition;
            break;
        }
    }
    
    _UpdateStatusBar();

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::OnUpdateComposition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew)
{
    OutputDebugString(TEXT("CTSFEditWnd::OnUpdateComposition\n"));

    _UpdateStatusBar();

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::OnEndComposition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::OnEndComposition(ITfCompositionView *pComposition)
{
    OutputDebugString(TEXT("CTSFEditWnd::OnEndComposition\n"));

    //find the composition pointer in the array
    ULONG i;
    for(i = 0; i < MAX_COMPOSITIONS; i++)
    {
        if(pComposition == m_rgCompositions[i])
        {
            m_rgCompositions[i]->Release();
            m_rgCompositions[i] = NULL;

            m_cCompositions--;
            break;
        }
    }

    _UpdateStatusBar();

    return S_OK;
}


/**************************************************************************

	CTSFEditWnd::_TerminateAllCompositions()

**************************************************************************/

HRESULT CTSFEditWnd::_TerminateAllCompositions(void)
{
    HRESULT hr;
    ITfContextOwnerCompositionServices *pCompServices;
    
    //get the ITfContextOwnerCompositionServices interface pointer
    hr = m_pContext->QueryInterface(IID_ITfContextOwnerCompositionServices, (LPVOID*)&pCompServices);
    if(SUCCEEDED(hr))
    {
        //passing NULL terminates all compositions
        hr = pCompServices->TerminateComposition(NULL);

        pCompServices->Release();
    }

    return hr;
}


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   srengext.h 
*       This file contains the declaration of the CSampleSRExtension class.
*       This implements the custom interface ISampleSRExtension.
*       When an app QI's for this from the reco context, SAPI will
*       look for the ExtensionCLSID field in the engine object token, and
*       create this object and then QI for the requested interface.
******************************************************************************/

#pragma once

#include "stdafx.h" 
#include "SampleSrEngine.h"
#include "resource.h"

class ATL_NO_VTABLE CSampleSRExtension : 
public CComObjectRootEx<CComMultiThreadModel>,
public CComCoClass<CSampleSRExtension, &CLSID_SampleSRExtension>,
public ISampleSRExtension,
public ISpDisplayAlternates,
public ISpEnginePronunciation
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SRENGEXT)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()
        
BEGIN_COM_MAP(CSampleSRExtension)
    COM_INTERFACE_ENTRY(ISampleSRExtension)
    COM_INTERFACE_ENTRY(ISpDisplayAlternates)
    COM_INTERFACE_ENTRY(ISpEnginePronunciation)
END_COM_MAP()
        
    HRESULT FinalConstruct()
    {
        // Fail if CRecoExt is created as a non-aggregate object.
        // SAPI creates CRecoExt as an aggregate.
        if(GetControllingUnknown() == dynamic_cast<ISampleSRExtension *>(this) )
        {
            return E_FAIL;
        }

        // QI for this interface when dealing with SAPI 5.1.
        // It must be QI'd for in the FinalConstruct and is not released.
        // We don't QI for SAPI 5.2 ISpPrivateEngineCallEx interface until later.
        return OuterQueryInterface(IID__ISpPrivateEngineCall, (void **)&m_pEngineCall);
    }

    void FinalRelease()
    {
        // Don't release IID__ISpPrivateEngineCall here.
    }

    STDMETHODIMP ExamplePrivateEngineCall(void); // Just a test method


    // ISpDisplayAlternates methods
    STDMETHODIMP GetDisplayAlternates(
        const SPDISPLAYPHRASE *pPhrase, 
        ULONG cRequestCount, 
        SPDISPLAYPHRASE **ppCoMemPhrases,
        ULONG *pcPhrasesReturned);

    STDMETHODIMP SetFullStopTrailSpace(ULONG ulTrailSpace);


    // ISpEnginePronunciation methods
    STDMETHODIMP Normalize( 
        LPCWSTR pszWord,
        LPCWSTR pszLeftContext,
        LPCWSTR pszRightContext,
        WORD LangID,
        SPNORMALIZATIONLIST *pNormalizationList);
        
    STDMETHODIMP GetPronunciations( 
        LPCWSTR pszWord,
        LPCWSTR pszLeftContext,
        LPCWSTR pszRightContext,
        WORD LangID,
        SPWORDPRONUNCIATIONLIST *pEnginePronunciationList);

private:
    _ISpPrivateEngineCall *m_pEngineCall;

};

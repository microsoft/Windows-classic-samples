// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   srengext.cpp 
*       This file contains the implementation of the CSampleSRExtension class.
*       This implements the custom interface ISampleSRExtension.
*       When an app QI's for this from the reco context, SAPI will
*       look for the ExtensionCLSID field in the engine object token, and
*       create this object and then QI for the requested interface.
******************************************************************************/

#include "stdafx.h"
#include "srengext.h"

/****************************************************************************
* CSampleSRExtension::ExamplePrivateEngineCall *
*----------------------------------------------*
*   Description:
*       This method shows an example of calling back to the main engine object.
*       When CallEngine is called, the data supplied will get passed by SAPI
*       to the ISpSREngine::PrivateCall method in CSrEngine.
*   Return: 
*       S_OK
*       FAILED(hr)
*****************************************************************************/   
STDMETHODIMP CSampleSRExtension::ExamplePrivateEngineCall(void)
{
    // We can use this method to pass data to and from the actual engine class, via the context

    // We can query back to SAPI to find both the reco context, and,
    // an IID__ISpPrivateEngineCall interface which can be used to call
    // back to the main engine object.
    static BYTE Data[4] = { 1, 2, 3, 4 };
    HRESULT hr = S_OK;

    // Try and query for new interface. Gets released on exit {don't keep persistent pointer}.
    CComPtr<ISpPrivateEngineCallEx> cpEngineCallEx;
    OuterQueryInterface(IID_ISpPrivateEngineCallEx, (void **)&cpEngineCallEx); // Ignore error

    if (cpEngineCallEx)
    {
        void *pCoMemOutFrame = NULL;
        ULONG ulOutFrameSize;
        hr = cpEngineCallEx->CallEngineSynchronize( (void*)Data, sp_countof(Data), &pCoMemOutFrame, &ulOutFrameSize );
        ::CoTaskMemFree( pCoMemOutFrame );
    }
    else
    {
        // Else use old interface.
        hr = m_pEngineCall->CallEngine( (void*)Data, sp_countof(Data) );
    }

    return hr;
}

/****************************************************************************
* CSampleSRExtension::GetDisplayAlternates *
*----------------------------------------------*
*   Description:
*       This method is supposed to generate display alternates for the
*       specified phrase. It's called by Windows Vista Speech Recognition
*       System before the dictation result is inserted into a document.
*
*       The input phrase contains the left context, the phrase recognized by
*       the engine and the right context. Left context is the word which is
*       right before the cursor at the time of recognition. It's the first
*       token. The right context is the word which is right after the cursor,
*       and it is the last token. All the other tokens are the phrase that is
*       recognized by the engine
*
*       This method should return at least one valid alternate in order to
*       enable Windows Vista to dictate using this engine.
*   Return: 
*       S_OK
*       FAILED(hr)
*****************************************************************************/   
STDMETHODIMP CSampleSRExtension::GetDisplayAlternates(
    const SPDISPLAYPHRASE *pPhrase, 
    ULONG cRequestCount, 
    SPDISPLAYPHRASE **ppCoMemPhrases,
    ULONG *pcPhrasesReturned)
{
    HRESULT hr = S_OK;

    // Initialize pointers to NULL.
    memset(ppCoMemPhrases, 0, sizeof(*ppCoMemPhrases) * cRequestCount);

    // Return cRequestCount display alternates (though they'll be all same)
    *pcPhrasesReturned = cRequestCount;

    for (unsigned int p=0; p<*pcPhrasesReturned; p++)
    {
        // Only one buffer will be allocated (by CoTaskMemAlloc) to store all the
        // SPDISPLAYPHRASE, the SPDISPLAYTOKEN structures that are pointed from
        // the SPDISPLAYPHRASE, and all the strings. This buffer will be freed
        // by the caller.

        // First calculate the size of the memory needed
        size_t cbPhraseSize = sizeof(SPDISPLAYPHRASE);

        // Add the size needed to store the SPDISPLAYTOKEN structures
        cbPhraseSize += pPhrase->ulNumTokens * sizeof(SPDISPLAYTOKEN);

        // Finally add the size needed to store the strings
        for (unsigned int t=0; t<pPhrase->ulNumTokens; t++)
        {
            if (pPhrase->pTokens[t].pszDisplay != NULL)
            {
                cbPhraseSize += (wcslen(pPhrase->pTokens[t].pszDisplay) + 1) * sizeof(WCHAR);
            }

            if (pPhrase->pTokens[t].pszLexical != NULL)
            {
                cbPhraseSize += (wcslen(pPhrase->pTokens[t].pszLexical) + 1) * sizeof(WCHAR);
            }
        }

        // Allocate the memory
        ppCoMemPhrases[p] = (SPDISPLAYPHRASE *)CoTaskMemAlloc(cbPhraseSize);
        if (ppCoMemPhrases[p] == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

        SPDISPLAYPHRASE *pCoMemPhrase = ppCoMemPhrases[p];
        pCoMemPhrase->ulNumTokens = pPhrase->ulNumTokens;

        // The tokens are appended right after the phrase
        pCoMemPhrase->pTokens = (SPDISPLAYTOKEN *)(pCoMemPhrase + 1);

        // String table begins right after the tokens
        LPWSTR pStringTable = (LPWSTR)(pCoMemPhrase->pTokens + pCoMemPhrase->ulNumTokens);
        LPWSTR pEndOfStringTable = (LPWSTR)(((BYTE*)pCoMemPhrase) + cbPhraseSize);

        // Now fill the tokens and strings
        for (unsigned int t=0; t<pPhrase->ulNumTokens; t++)
        {
            // Copy the token's display attributes
            pCoMemPhrase->pTokens[t].bDisplayAttributes = pPhrase->pTokens[t].bDisplayAttributes;

            // If current token is the left context
            if (t == 0)
            {
                // If the left context is not empty (we're not at the beginning of the line or paragraph)
                if (pPhrase->pTokens[t].pszDisplay != NULL)
                {
                    // Insert a space after it
                    pCoMemPhrase->pTokens[t].bDisplayAttributes |= SPAF_ONE_TRAILING_SPACE;
                }
            }
            else
            {
                // If it's not the right context
                if (t<pPhrase->ulNumTokens-1)
                {
                    // Insert a space after it
                    pCoMemPhrase->pTokens[t].bDisplayAttributes |= SPAF_ONE_TRAILING_SPACE;
                }
            }

            // Copy the token's display string
            if (pPhrase->pTokens[t].pszDisplay != NULL)
            {
                // Get the length of the token's display string
                size_t length = wcslen(pPhrase->pTokens[t].pszDisplay);
                // Copy it to the string table
                if (wcscpy_s(pStringTable, pEndOfStringTable - pStringTable, pPhrase->pTokens[t].pszDisplay))
                {
                    hr = E_OUTOFMEMORY;
                    goto CleanUp;
                }
                // Adjust the output token's display string to point to the string in the string table
                pCoMemPhrase->pTokens[t].pszDisplay = pStringTable;
                // Advance the string table pointer
                pStringTable += length + 1;
            }
            else
            {
                ppCoMemPhrases[p]->pTokens[t].pszDisplay = NULL;
            }

            // Copy the token's lexical string
            if (pPhrase->pTokens[t].pszLexical != NULL)
            {
                // Get the length of the token's lexical string
                size_t length = wcslen(pPhrase->pTokens[t].pszLexical);
                // Copy it to the string table
                if (wcscpy_s(pStringTable, pEndOfStringTable - pStringTable, pPhrase->pTokens[t].pszLexical))
                {
                    hr = E_OUTOFMEMORY;
                    goto CleanUp;
                }
                // Adjust the output token's lexical string to point to the string in the string table
                pCoMemPhrase->pTokens[t].pszLexical = pStringTable;
                // Advance the string table pointer
                pStringTable += length + 1;
            }
            else
            {
                ppCoMemPhrases[p]->pTokens[t].pszLexical = NULL;
            }
        }
    }

CleanUp:
    if (FAILED(hr))
    {
        for (unsigned int p=0; p<*pcPhrasesReturned; p++)
        {
            CoTaskMemFree(ppCoMemPhrases[p]);
            ppCoMemPhrases[p] = NULL;
        }

        *pcPhrasesReturned = 0;
    }

    return hr;
}

/****************************************************************************
* CSampleSRExtension::SetFullStopTrailSpace *
*----------------------------------------------*
*   Description:
*       This is used by the application to specify the number of spaces after
*       full stop puctuations.
*   Return: 
*       S_OK
*****************************************************************************/   
STDMETHODIMP CSampleSRExtension::SetFullStopTrailSpace(ULONG ulTrailSpace)
{
    // Do nothing.
    return S_OK;
}

/****************************************************************************
* CSampleSRExtension::Normalize *
*----------------------------------------------*
*   Description:
*       This method is supposed to return the list of normalized forms for a
*       given word, but it's not supported by this sample engine.
*   Return: 
*       S_NOTSUPPORTED
*****************************************************************************/   
STDMETHODIMP CSampleSRExtension::Normalize( 
    LPCWSTR pszWord,
    LPCWSTR pszLeftContext,
    LPCWSTR pszRightContext,
    WORD LangID,
    SPNORMALIZATIONLIST *pNormalizationList)
{
    // This functionality is not supported
    pNormalizationList = NULL;

    return S_NOTSUPPORTED;
}
    
/****************************************************************************
* CSampleSRExtension::GetPronunciations *
*----------------------------------------------*
*   Description:
*       This method is supposed to return all the pronunciations the engine
*       knows about for a given word, but it's not supported by this sample
*       engine.
*   Return: 
*       S_NOTSUPPORTED
*****************************************************************************/   
STDMETHODIMP CSampleSRExtension::GetPronunciations( 
    LPCWSTR pszWord,
    LPCWSTR pszLeftContext,
    LPCWSTR pszRightContext,
    WORD LangID,
    SPWORDPRONUNCIATIONLIST *pEnginePronunciationList)
{
    // This functionality is not supported
    pEnginePronunciationList->ulSize = 0;
    pEnginePronunciationList->pvBuffer = 0;
    pEnginePronunciationList->pFirstWordPronunciation = 0;

    return S_NOTSUPPORTED;
}

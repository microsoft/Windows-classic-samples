// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   resultcontainer.h 
*       This module contains the definition of CResultContainer.  
*       CResultContainer makes all of the recognition-object-
*       specific SAPI5 calls
******************************************************************************/
#pragma once

#include "DictationRun.h"

// Forward definition
class CDictationRun;

// List node for the result container's list of owner DictationRuns
typedef struct OWNERNODE
{
    CDictationRun   *pOwner;
    OWNERNODE       *pNext;
}   OWNERNODE;

// Constant for alternate retrieval
#define ALT_REQUEST_COUNT 15   

/******************************************************************************
* CResultContainer *
*------------------*
*   Description:
*       CResultContainer is the class that handles the recognition result
*       object that corresponds to one or more CDictationRun's (see
*       DictationRun.h) that hold phrase elements from that result.
*       All SAPI5 calls on recognition result objects are made from
*       the appropriate instance of CResultContainer
*******************************************************************************/
class CResultContainer
{
public:

    CResultContainer( ISpRecoResult &rResult, 
        CDictationRun &rFirstOwner, 
        CPhraseReplacement &rPhraseReplacement );
    ~CResultContainer();

    HRESULT AddOwner( CDictationRun &rNewOwner );
    void DeleteOwner( CDictationRun &rOldOwner );

    CPhraseReplacement *GetPhraseReplacement() 
    { return m_pPhraseReplacement; }

    // These methods that behave exactly like the methods of 
    // the same name in ISpRecoResult, except the arguments
    // to these take into account possible ITN replacement
    HRESULT SpeakAudio( 
        ULONG ulStartElement,
        ULONG cElements );
    HRESULT Serialize( 
        SPSERIALIZEDRESULT **ppResultBlock)
    { return m_cpRecoResult->Serialize( ppResultBlock );}

    // These methods are closely related to ISpRecoResult 
    // methods, altered slightly for the purposes of this app
    HRESULT GetAlternatesText( 
        ULONG ulStartElement,
        ULONG cElements,
        ULONG ulRequestCount,
        __out_ecount_part(ulRequestCount, *pcPhrasesReturned) WCHAR **ppszCoMemText,
        __out ULONG *pcPhrasesReturned );
    HRESULT GetAltInfo(
        ULONG ulStartElement,
        ULONG cElements,
        ULONG ulAlternateIndex,
        ULONG *pulStartInParent,
        ULONG *pcEltsInParent,
        ULONG *pcEltsInAlt = NULL,
        bool fReturnPostReplIndices = true);
    HRESULT GetAltText(
        ULONG ulStartElement,
        ULONG cElements,
        ULONG ulAlternateIndex,
        __deref_out WCHAR **ppwszCoMemText,
        __out_opt BYTE *pbDisplayAttributes );
    HRESULT ChooseAlternate( 
        ULONG ulStartElement,
        ULONG cElements,
        ULONG ulAlternateIndex,
        BYTE  *pbDisplayAttributes );

private:

    HRESULT GetAlternates( 
        ULONG ulStartElement,
        ULONG cElements,
        ULONG ulRequestCount,
        __out_opt ULONG *pcPhrasesReturned );

    // Called whenever the RecoResult has changed as a result of some
    // owner committing an alternate
    HRESULT NotifyOwnersOfCommit( 
        ULONG ulStartElement, 
        ULONG cElements, 
        ULONG ulAlternateIndex );

private:

    CComPtr<ISpRecoResult>      m_cpRecoResult;
    CPhraseReplacement          *m_pPhraseReplacement;
    OWNERNODE                   *m_pOwnerListHead;

    // Information about the last-requested set of alternates.
    // Note that m_ulStartOfLastRequestedAlts and m_cElementsInLastRequestedAlts
    // are indices with ITN replacements figured in
    ISpPhraseAlt *              m_apLastRequestedPhraseAlts[ALT_REQUEST_COUNT];
    ULONG                       m_cLastRequestedAltsReturned;
    ULONG                       m_ulStartOfLastRequestedAlts;
    ULONG                       m_cElementsInLastRequestedAlts;
};  // class CResultContainer



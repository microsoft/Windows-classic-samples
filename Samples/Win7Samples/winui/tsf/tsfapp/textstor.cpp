/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          TextStor.cpp

   Description:   ITextStoreACP Implementation

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "TSFEdit.h"
#include "DataObj.h"
#include "Globals.h"

#include <tsattrs.h>

/**************************************************************************
   global variables
**************************************************************************/

/**************************************************************************

   CTSFEditWnd::AdviseSink()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::AdviseSink(REFIID riid, IUnknown *pUnknown, DWORD dwMask)
{
    OutputDebugString(TEXT("CTSFEditWnd::AdviseSink \n"));

    HRESULT     hr;
    IUnknown    *punkID;

    //Get the "real" IUnknown pointer. This needs to be done for comparison purposes.
    hr = pUnknown->QueryInterface(IID_IUnknown, (LPVOID*)&punkID);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = E_INVALIDARG;
    
    //see if this advise sink already exists
    if(punkID == m_AdviseSink.punkID)
    {
        //this is the same advise sink, so just update the advise mask
        m_AdviseSink.dwMask = dwMask;
        
        hr = S_OK;
    }
    else if(NULL != m_AdviseSink.punkID)
    {
        //only one advise sink is allowed at a time
        hr = CONNECT_E_ADVISELIMIT;
    }
    else if(IsEqualIID(riid, IID_ITextStoreACPSink))
    {
        //set the advise mask
        m_AdviseSink.dwMask = dwMask;

        /*
        Set the IUnknown pointer. This is used for comparison in 
        UnadviseSink and future calls to this method.
        */
        m_AdviseSink.punkID = punkID;

        //AddRef this because it will get released below and it needs to be kept
        punkID->AddRef();

        //get the ITextStoreACPSink interface
        pUnknown->QueryInterface(IID_ITextStoreACPSink, (LPVOID*)&m_AdviseSink.pTextStoreACPSink);

        //get the ITextStoreACPServices interface
        pUnknown->QueryInterface(IID_ITextStoreACPServices, (LPVOID*)&m_pServices);

        hr = S_OK;
    }
    
    //this isn't needed anymore
    punkID->Release();

    return hr;
}

/**************************************************************************

   CTSFEditWnd::UnadviseSink()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::UnadviseSink(IUnknown *pUnknown)
{
    OutputDebugString(TEXT("CTSFEditWnd::UnadviseSink \n"));

    HRESULT     hr;
    IUnknown    *punkID;

    /*
    Get the "real" IUnknown pointer. This needs to be done for comparison 
    purposes.
    */
    hr = pUnknown->QueryInterface(IID_IUnknown, (LPVOID*)&punkID);
    if(FAILED(hr))
    {
        return hr;
    }

    //find the advise sink
    if(punkID == m_AdviseSink.punkID)
    {
        //remove the advise sink from the list
        _ClearAdviseSink(&m_AdviseSink);

        if(m_pServices)
        {
            m_pServices->Release();
            m_pServices = NULL;
        }

        hr = S_OK;
    }
    else
    {
        hr = CONNECT_E_NOCONNECTION;
    }

    punkID->Release();
    
    return hr;
}

/**************************************************************************

   CTSFEditWnd::RequestLock()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
{
    OutputDebugString(TEXT("CTSFEditWnd::RequestLock \n"));

    if(NULL == m_AdviseSink.pTextStoreACPSink)
    {
        return E_UNEXPECTED;
    }
    
    if(NULL == phrSession)
    {
        return E_INVALIDARG;
    }

    *phrSession = E_FAIL;

    if(m_fLocked)
    {
        //the document is locked

        if(dwLockFlags & TS_LF_SYNC)
        {
            /*
            The caller wants an immediate lock, but this cannot be granted because 
            the document is already locked.
            */
            *phrSession = TS_E_SYNCHRONOUS;
            return S_OK;
        }
        else    
        {
            //the request is asynchronous 

            /*
            The only type of asynchronous lock request this application 
            supports while the document is locked is to upgrade from a read 
            lock to a read/write lock. This scenario is referred to as a lock 
            upgrade request. 
            */
            if(((m_dwLockType & TS_LF_READWRITE) == TS_LF_READ) && 
                ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE))
            {
                m_fPendingLockUpgrade = TRUE;

                *phrSession = TS_S_ASYNC;

                return S_OK;
            }

        }
        return E_FAIL;
    }

    //lock the document
    _LockDocument(dwLockFlags);
    
    //call OnLockGranted
    *phrSession = m_AdviseSink.pTextStoreACPSink->OnLockGranted(dwLockFlags);

    //unlock the document
    _UnlockDocument();

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetStatus()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetStatus(TS_STATUS *pdcs)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetStatus \n"));

    if(NULL == pdcs)
    {
        return E_INVALIDARG;
    }

    /*
    Can be zero or:
    TS_SD_READONLY  // if set, document is read only; writes will fail
    TS_SD_LOADING   // if set, document is loading, expect additional inserts
    */
    pdcs->dwDynamicFlags = 0;

    /*
    Can be zero or:
    TS_SS_DISJOINTSEL   // if set, the document supports multiple selections
    TS_SS_REGIONS       // if clear, the document will never contain multiple regions
    TS_SS_TRANSITORY    // if set, the document is expected to have a short lifespan
    TS_SS_NOHIDDENTEXT  // if set, the document will never contain hidden text (for perf)
    */
    pdcs->dwStaticFlags = TS_SS_REGIONS;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::QueryInsert()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::QueryInsert(  LONG acpTestStart, 
                                        LONG acpTestEnd, 
                                        ULONG cch, 
                                        LONG *pacpResultStart, 
                                        LONG *pacpResultEnd)
{
    OutputDebugString(TEXT("CTSFEditWnd::QueryInsert\n"));

    LONG   lTextLength;

    lTextLength = GetWindowTextLength(m_hwndEdit);

    //make sure the parameters are within range of the document
    if( (acpTestStart > acpTestEnd) || 
        (acpTestEnd > lTextLength))
    {
        return E_INVALIDARG;
    }

    //set the start point to the given start point
    *pacpResultStart = acpTestStart;

    //set the end point to the given end point
    *pacpResultEnd = acpTestEnd;

    return S_OK;
}

/**************************************************************************

    CTSFEditWnd::_TestInsert()

    This method is similar to QueryInsert except this method assumes the 
    insertion will actually happen, so the document length would get 
    expanded to fit the inserted text. QueryInsert doesn't allow the ranges 
    to go outside of the existing text.

**************************************************************************/

STDMETHODIMP CTSFEditWnd::_TestInsert(  LONG acpTestStart, 
                                        LONG acpTestEnd, 
                                        ULONG cch, 
                                        LONG *pacpResultStart, 
                                        LONG *pacpResultEnd)
{
    //make sure the parameters are within range of the document
    if(acpTestStart > acpTestEnd)
    {
        return E_INVALIDARG;
    }

    //set the start point after the insertion
    *pacpResultStart = acpTestStart;

    //set the end point after the insertion
    *pacpResultEnd = acpTestStart + cch;
    
    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetSelection()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetSelection( ULONG ulIndex, 
                                        ULONG ulCount, 
                                        TS_SELECTION_ACP *pSelection, 
                                        ULONG *pcFetched)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetSelection \n"));

    //verify pSelection
    if(NULL == pSelection)
    {
        return E_INVALIDARG;
    }

    //verify pcFetched
    if(NULL == pcFetched)
    {
        return E_INVALIDARG;
    }

    *pcFetched = 0;

    //does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    //check the requested index
    if(TF_DEFAULT_SELECTION == ulIndex)
    {
        ulIndex = 0;
    }
    else if (ulIndex > 1)
    {
        /*
        The index is too high. This app only supports one selection.
        */
        return E_INVALIDARG;
    }

    _GetCurrentSelection();
    
    //find out which end of the selection the caret (insertion point) is
    POINT   pt;
    LRESULT lPos;
    GetCaretPos(&pt);
    lPos = ::SendMessage(m_hwndEdit, EM_POSFROMCHAR, m_acpStart, 0);

    //if the caret position is the same as the start character, then the selection end is the start of the selection
    m_ActiveSelEnd = ((pt.x == LOWORD(lPos) && pt.y == HIWORD(lPos)) ? TS_AE_START : TS_AE_END);

    pSelection[0].acpStart = m_acpStart;
    pSelection[0].acpEnd = m_acpEnd;
    pSelection[0].style.fInterimChar = m_fInterimChar;
    if(m_fInterimChar)
    {
        /*
        fInterimChar will be set when an intermediate character has been 
        set. One example of when this will happen is when an IME is being 
        used to enter characters and a character has been set, but the IME 
        is still active.
        */
        pSelection[0].style.ase = TS_AE_NONE;
    }
    else
    {
        pSelection[0].style.ase = m_ActiveSelEnd;
    }

    *pcFetched = 1;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::SetSelection()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::SetSelection( ULONG ulCount, 
                                        const TS_SELECTION_ACP *pSelection)
{
    OutputDebugString(TEXT("CTSFEditWnd::SetSelection \n"));

    //verify pSelection
    if(NULL == pSelection)
    {
        return E_INVALIDARG;
    }

    if(ulCount > 1)
    {
        //this implementaiton only supports a single selection
        return E_INVALIDARG;
    }
    
    //does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    m_acpStart = pSelection[0].acpStart;
    m_acpEnd = pSelection[0].acpEnd;
    m_fInterimChar = pSelection[0].style.fInterimChar;
    if(m_fInterimChar)
    {
        /*
        fInterimChar will be set when an intermediate character has been 
        set. One example of when this will happen is when an IME is being 
        used to enter characters and a character has been set, but the IME 
        is still active.
        */
        m_ActiveSelEnd = TS_AE_NONE;
    }
    else
    {
        m_ActiveSelEnd = pSelection[0].style.ase;
    }

    //if the selection end is at the start of the selection, reverse the parameters
    LONG    lStart = m_acpStart;
    LONG    lEnd = m_acpEnd;

    if(TS_AE_START == m_ActiveSelEnd)
    {
        lStart = m_acpEnd;
        lEnd = m_acpStart;
    }

    m_fNotify = FALSE;
    
    ::SendMessage(m_hwndEdit, EM_SETSEL, lStart, lEnd);

    m_fNotify = TRUE;
    
    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetText()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetText(  LONG acpStart, 
                                    LONG acpEnd, 
                                    WCHAR *pchPlain, 
                                    ULONG cchPlainReq, 
                                    ULONG *pcchPlainOut, 
                                    TS_RUNINFO *prgRunInfo, 
                                    ULONG ulRunInfoReq, 
                                    ULONG *pulRunInfoOut, 
                                    LONG *pacpNext)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetText\n"));

    //does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    BOOL    fDoText = cchPlainReq > 0;
    BOOL    fDoRunInfo = ulRunInfoReq > 0;
    LONG    cchTotal;
    HRESULT hr = E_FAIL;

    if(pcchPlainOut)
    {
        *pcchPlainOut = 0;
    }

    if(fDoRunInfo)
    {
        *pulRunInfoOut = 0;
    }

    if(pacpNext)
    {
        *pacpNext = acpStart;
    }

    //get all of the text
    LPWSTR  pwszText;
    hr = _GetText(&pwszText, &cchTotal);
    if(FAILED(hr))
    {
        return hr;
    }
    
    //validate the start pos
    if((acpStart < 0) || (acpStart > cchTotal))
    {
        hr = TS_E_INVALIDPOS;
    }
    else
    {
        //are we at the end of the document
        if(acpStart == cchTotal)
        {
            hr = S_OK;
        }
        else
        {
            ULONG    cchReq;

            /*
            acpEnd will be -1 if all of the text up to the end is being requested. 
            */

            if(acpEnd >= acpStart)
            {
                cchReq = acpEnd - acpStart;
            }
            else
            {
                cchReq = cchTotal - acpStart;
            }

            if(fDoText)
            {
                if(cchReq > cchPlainReq)
                {
                    cchReq = cchPlainReq;
                }

                //extract the specified text range
                LPWSTR  pwszStart = pwszText + acpStart;
    
                if(pchPlain && cchPlainReq)
                {
                    //the text output is not NULL terminated
                    CopyMemory(pchPlain, pwszStart, cchReq * sizeof(WCHAR));
                }
            }

            //it is possible that only the length of the text is being requested
            if(pcchPlainOut)
            {
                *pcchPlainOut = cchReq;
            }

            if(fDoRunInfo)
            {
                /*
                Runs are used to separate text characters from formatting characters. 
        
                In this example, sequences inside and including the <> are treated as 
                control sequences and are not displayed.  

                Plain text = "Text formatting."
                Actual text = "Text <B><I>formatting</I></B>." 
        
                If all of this text were requested, the run sequence would look like this:

                prgRunInfo[0].type = TS_RT_PLAIN;   //"Text "
                prgRunInfo[0].uCount = 5;

                prgRunInfo[1].type = TS_RT_HIDDEN;  //<B><I>
                prgRunInfo[1].uCount = 6;

                prgRunInfo[2].type = TS_RT_PLAIN;   //"formatting"
                prgRunInfo[2].uCount = 10;
        
                prgRunInfo[3].type = TS_RT_HIDDEN;  //</B></I>
                prgRunInfo[3].uCount = 8;

                prgRunInfo[4].type = TS_RT_PLAIN;   //"."
                prgRunInfo[4].uCount = 1;

                TS_RT_OPAQUE is used to indicate characters or character sequences 
                that are in the document, but are used privately by the application
                and do not map to text.  Runs of text tagged with TS_RT_OPAQUE should
                NOT be included in the pchPlain or cchPlainOut [out] parameters.
                */
        
                /*
                This implementation is plain text, so the text only consists of one run.
                If there were multiple runs, it would be an error to have consecuative runs
                of the same type.
                */
                *pulRunInfoOut = 1;
                prgRunInfo[0].type = TS_RT_PLAIN;
                prgRunInfo[0].uCount = cchReq;
            }

            if(pacpNext)
            {
                *pacpNext = acpStart + cchReq;
            }

            hr = S_OK;
        }
    }

    GlobalFree(pwszText);

    return hr;
}

/**************************************************************************

   CTSFEditWnd::SetText()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::SetText(  DWORD dwFlags, 
                                    LONG acpStart, 
                                    LONG acpEnd, 
                                    const WCHAR *pchText, 
                                    ULONG cch, 
                                    TS_TEXTCHANGE *pChange)
{
    OutputDebugString(TEXT("CTSFEditWnd::SetText \n"));

    HRESULT hr;

    /*
    dwFlags can be:
    TS_ST_CORRECTION
    */
    
    if(dwFlags & TS_ST_CORRECTION)
    {
        OutputDebugString(TEXT("\tTS_ST_CORRECTION\n"));
    }

    //set the selection to the specified range
    TS_SELECTION_ACP    tsa;
    tsa.acpStart = acpStart;
    tsa.acpEnd = acpEnd;
    tsa.style.ase = TS_AE_START;
    tsa.style.fInterimChar = FALSE;

    hr = SetSelection(1, &tsa);

    if(SUCCEEDED(hr))
    {
        //call InsertTextAtSelection
        hr = InsertTextAtSelection(TS_IAS_NOQUERY, pchText, cch, NULL, NULL, pChange);
    }

    return hr;
}

/**************************************************************************

   CTSFEditWnd::GetFormattedText()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetFormattedText( LONG acpStart, 
                                            LONG acpEnd, 
                                            IDataObject **ppDataObject)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetFormattedText \n"));

    if(NULL == ppDataObject)
    {
        return E_INVALIDARG;
    }
    
    *ppDataObject = NULL;
    
    //does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    HRESULT     hr;

    CTSFDataObject *pdo = new CTSFDataObject;
    if(NULL != pdo)
    {
        //get the text
        ULONG   cchOut;

        if(-1 == acpEnd)
        {
            //get the length of all of the text
            hr = GetText(acpStart, acpEnd, NULL, 0, &cchOut, NULL, 0, NULL, NULL);
        }
        else
        {
            cchOut = acpEnd - acpStart;
            hr = S_OK;
        }

        if(SUCCEEDED(hr))
        {
            LPWSTR  pwszTemp = (LPWSTR)GlobalAlloc(GPTR, (cchOut + 1) * sizeof(WCHAR));
            if(NULL != pwszTemp)
            {
                hr = GetText(acpStart, acpEnd, pwszTemp, cchOut, &cchOut, NULL, 0, NULL, NULL);
                pwszTemp[cchOut] = '\0';
                if(SUCCEEDED(hr))
                {
                    //set the text in the data object
                    hr = pdo->_SetText(pwszTemp);
                    if(SUCCEEDED(hr))
                    {
                        //get the IID_IDataObject interface
                        hr = pdo->QueryInterface(IID_IDataObject, (LPVOID*)ppDataObject);
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        //release the interface
        pdo->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }


    return hr;
}

/**************************************************************************

   CTSFEditWnd::GetEmbedded()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetEmbedded(  LONG acpPos, 
                                        REFGUID rguidService, 
                                        REFIID riid, IUnknown **ppunk)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetEmbedded \n"));

    //this implementation doesn't support embedded objects
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFEditWnd::QueryInsertEmbedded()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::QueryInsertEmbedded(  const GUID *pguidService, 
                                                const FORMATETC *pFormatEtc, 
                                                BOOL *pfInsertable)
{
    OutputDebugString(TEXT("CTSFEditWnd::QueryInsertEmbedded \n"));

    //this implementation doesn't support embedded objects
    *pfInsertable = FALSE;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::InsertEmbedded()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::InsertEmbedded(   DWORD dwFlags, 
                                            LONG acpStart, 
                                            LONG acpEnd, 
                                            IDataObject *pDataObject, 
                                            TS_TEXTCHANGE *pChange)
{
    OutputDebugString(TEXT("CTSFEditWnd::InsertEmbedded \n"));

    //this implementation doesn't support embedded objects
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFEditWnd::RequestSupportedAttrs()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::RequestSupportedAttrs(    DWORD dwFlags, 
                                                    ULONG cFilterAttrs, 
                                                    const TS_ATTRID *paFilterAttrs)
{
    OutputDebugString(TEXT("CTSFEditWnd::RequestSupportedAttrs \n"));

    _ClearRequestedAttributes();

    int i;

    for(i = 0; i < NUM_SUPPORTED_ATTRS; i++)
    {
        ULONG x;

        for(x = 0; x < cFilterAttrs; x++)
        {
            if(IsEqualGUID(*m_rgAttributes[i].attrid, paFilterAttrs[x]))
            {
                m_rgAttributes[i].dwFlags = ATTR_FLAG_REQUESTED;
                if(dwFlags & TS_ATTR_FIND_WANT_VALUE)
                {
                    m_rgAttributes[i].dwFlags |= ATTR_FLAG_DEFAULT;
                }
                else
                {
                    //just copy the default value into the regular value
                    VariantCopy(&m_rgAttributes[i].varValue, &m_rgAttributes[i].varDefaultValue);
                }
            }
        }
    }

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::RequestAttrsAtPosition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::RequestAttrsAtPosition(   LONG acpPos, 
                                                    ULONG cFilterAttrs, 
                                                    const TS_ATTRID *paFilterAttrs, 
                                                    DWORD dwFlags)
{
    OutputDebugString(TEXT("CTSFEditWnd::RequestAttrsAtPosition \n"));

    int cch = GetWindowTextLength(m_hwndEdit);

    if(acpPos < 0 || acpPos > cch)
    {
        return TS_E_INVALIDPOS;
    }

    _ClearRequestedAttributes();

    /*
    This app doesn't maintain per-character attributes, so just return the default attributes.
    */
    int i;

    for(i = 0; i < NUM_SUPPORTED_ATTRS; i++)
    {
        ULONG x;

        for(x = 0; x < cFilterAttrs; x++)
        {
            if(IsEqualGUID(*m_rgAttributes[i].attrid, paFilterAttrs[x]))
            {
                m_rgAttributes[i].dwFlags = ATTR_FLAG_REQUESTED;
                if(dwFlags & TS_ATTR_FIND_WANT_VALUE)
                {
                    m_rgAttributes[i].dwFlags |= ATTR_FLAG_DEFAULT;
                }
                else
                {
                    //just copy the default value into the regular value
                    VariantCopy(&m_rgAttributes[i].varValue, &m_rgAttributes[i].varDefaultValue);
                }
            }
        }
    }

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::RequestAttrsTransitioningAtPosition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::RequestAttrsTransitioningAtPosition(  LONG acpPos, 
                                                                ULONG cFilterAttrs, 
                                                                const TS_ATTRID *paFilterAttrs, 
                                                                DWORD dwFlags)
{
    OutputDebugString(TEXT("CTSFEditWnd::RequestAttrsTransitioningAtPosition \n"));

    return E_NOTIMPL;
}

/**************************************************************************

   CTSFEditWnd::FindNextAttrTransition()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::FindNextAttrTransition(   LONG acpStart, 
                                                    LONG acpHalt, 
                                                    ULONG cFilterAttrs, 
                                                    const TS_ATTRID *paFilterAttrs, 
                                                    DWORD dwFlags, 
                                                    LONG *pacpNext, 
                                                    BOOL *pfFound, 
                                                    LONG *plFoundOffset)
{
    OutputDebugString(TEXT("CTSFEditWnd::FindNextAttrTransition \n"));

    return E_NOTIMPL;
}

/**************************************************************************

   CTSFEditWnd::RetrieveRequestedAttrs()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::RetrieveRequestedAttrs(   ULONG ulCount, 
                                                    TS_ATTRVAL *paAttrVals, 
                                                    ULONG *pcFetched)
{
    OutputDebugString(TEXT("CTSFEditWnd::RetrieveRequestedAttrs \n"));

    ULONG   uFetched = 0;
    int     i;

    for(i = 0; i < NUM_SUPPORTED_ATTRS && ulCount; i++)
    {
        if(m_rgAttributes[i].dwFlags & ATTR_FLAG_REQUESTED)
        {
            paAttrVals->varValue.vt = VT_EMPTY;
            
            //copy the attribute ID
            CopyMemory(&paAttrVals->idAttr, m_rgAttributes[i].attrid, sizeof(GUID));
            
            //this app doesn't support overlapped attributes
            paAttrVals->dwOverlapId = 0; 

            if (m_rgAttributes[i].dwFlags & ATTR_FLAG_DEFAULT)
            {
                VariantCopy(&paAttrVals->varValue, &m_rgAttributes[i].varDefaultValue);
            }
            else
            {
                VariantCopy(&paAttrVals->varValue, &m_rgAttributes[i].varValue);
            }

            paAttrVals++;
            uFetched++;
            ulCount--;

            //remove the item from the requested state
            VariantClear(&m_rgAttributes[i].varValue);
            m_rgAttributes[i].dwFlags = ATTR_FLAG_NONE;
        }
    }

    *pcFetched = uFetched;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetEndACP()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetEndACP(LONG *pacp)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetEndACP \n"));

    //does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    if(NULL == pacp)
    {
        return E_INVALIDARG;
    }

    _GetCurrentSelection();
    
    *pacp = m_acpEnd;
    
    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetActiveView()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetActiveView(TsViewCookie *pvcView)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetActiveView \n"));

    //this app only supports one view, so this can be constant
    *pvcView = EDIT_VIEW_COOKIE;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetACPFromPoint()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetACPFromPoint(  TsViewCookie vcView, 
                                            const POINT *pt, 
                                            DWORD dwFlags, 
                                            LONG *pacp)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetACPFromPoint \n"));

    return E_NOTIMPL;
}

/**************************************************************************

    CTSFEditWnd::GetTextExt()

    If the text spans multiple lines, the result is the rectangle that 
    contains all of the requested characters.

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetTextExt(   TsViewCookie vcView, 
                                        LONG acpStart, 
                                        LONG acpEnd, 
                                        RECT *prc, 
                                        BOOL *pfClipped)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetTextExt \n"));

    if(NULL == prc || NULL == pfClipped)
    {
        return E_INVALIDARG;
    }

    *pfClipped = FALSE;
    ZeroMemory(prc, sizeof(RECT));

    if(EDIT_VIEW_COOKIE != vcView)
    {
        return E_INVALIDARG;
    }

    //does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    //is this an empty request?
    if(acpStart == acpEnd)
    {
        return E_INVALIDARG;
    }

    LONG        lTextLength;
    LONG        lTemp;
    RECT        rc;
    DWORD       dwStart;
    DWORD       dwEnd;
    HDC         hdc;
    HFONT       hfont;
    TEXTMETRIC  tm;
    LONG        lLineHeight;
    LPWSTR      pwszText;
    HRESULT     hr;

    hr = _GetText(&pwszText);
    if(FAILED(hr))
    {
        return hr;
    }
    
    lTextLength = (LONG)SendMessage(m_hwndEdit, WM_GETTEXTLENGTH, 0, 0);

    //are the start and end reversed?
    if(acpStart > acpEnd)
    {
        lTemp = acpStart;
        acpStart = acpEnd;
        acpEnd = lTemp;
    }
    
    //request to the end of the text?
    if(-1 == acpEnd)
    {
        acpEnd = lTextLength - 1;
    }

    hdc = GetDC(m_hwndEdit);
    hfont = (HFONT)SendMessage(m_hwndEdit, WM_GETFONT, 0, 0);
    hfont = (HFONT)SelectObject(hdc, hfont);

    //get the position of the start character
    dwStart = (DWORD)SendMessage(m_hwndEdit, EM_POSFROMCHAR, acpStart, 0);
    rc.left = LOWORD(dwStart);
    rc.top = HIWORD(dwStart);

    //get the position of the last character
    /*
    The character offset passed to this method is inclusive. For example, if 
    the first character is being requested, acpStart will be 0 and acpEnd will 
    be 1. If the last character is requested, acpEnd will not equal a valid 
    character, so EM_POSFROMCHAR fails. If the next character is on another 
    line, EM_POSFROMCHAR won't return a valid value. To work around this, get 
    the position of the beginning of the end character, calculate the width of 
    the end character and add the width to the rectangle.
    */
    acpEnd--;
    dwEnd = (DWORD)SendMessage(m_hwndEdit, EM_POSFROMCHAR, acpEnd, 0);
    
    //calculate the width of the last character
    SIZE    size;
    GetTextExtentPoint32(hdc, pwszText + acpEnd, 1, &size);
    rc.right = LOWORD(dwEnd) + size.cx;
    rc.bottom = HIWORD(dwEnd);

    //calculate the line height
    GetTextMetrics(hdc, &tm);
    lLineHeight = tm.tmHeight;

    SelectObject(hdc, hfont);
    ReleaseDC(m_hwndEdit, hdc);

    /*
    If the text range spans multiple lines, expand the rectangle to include all 
    of the requested text. 
    */
    if(rc.bottom > rc.top)
    {
        DWORD   dwMargins;
        RECT    rcEdit;

        GetClientRect(m_hwndEdit, &rcEdit);
        
        dwMargins = (DWORD)SendMessage(m_hwndEdit, EM_GETMARGINS, 0, 0);
        
        //set the left point of the rectangle to the left margin of the edit control
        rc.left = LOWORD(dwMargins);

        //set the right member to the width of the edit control less both the right margin
        rc.right = rc.right - HIWORD(dwMargins);
    }

    //add the line height to the bottom of the rectangle
    rc.bottom += lLineHeight;

    *prc = rc;
    
    //if any part of the text rectangle is not visible, set *pfClipped to TRUE
    GetClientRect(m_hwndEdit, &rc);

    if( (prc->left < rc.left) ||
        (prc->top < rc.top) ||
        (prc->right > rc.right) ||
        (prc->bottom > rc.bottom))
    {
        *pfClipped = TRUE;
    }

    //convert the rectangle to screen coordinates
    MapWindowPoints(m_hwndEdit, NULL, (LPPOINT)prc, 2);

    GlobalFree(pwszText);

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetScreenExt()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetScreenExt(TsViewCookie vcView, RECT *prc)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetScreenExt \n"));

    if(NULL == prc)
    {
        return E_INVALIDARG;
    }

    ZeroMemory(prc, sizeof(RECT));

    if(EDIT_VIEW_COOKIE != vcView)
    {
        return E_INVALIDARG;
    }

    //no lock is necessary for this method.

    GetClientRect(m_hwndEdit, prc);
    MapWindowPoints(m_hwndEdit, NULL, (LPPOINT)prc, 2);

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::GetWnd()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetWnd(TsViewCookie vcView, HWND *phwnd)
{
    OutputDebugString(TEXT("CTSFEditWnd::GetWnd \n"));

    if(EDIT_VIEW_COOKIE == vcView)
    {
        *phwnd = _GetWindow();

        return S_OK;
    }

    return E_INVALIDARG;
}

/**************************************************************************

   CTSFEditWnd::InsertTextAtSelection()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::InsertTextAtSelection(    DWORD dwFlags, 
                                                    const WCHAR *pwszText, 
                                                    ULONG cch, 
                                                    LONG *pacpStart, 
                                                    LONG *pacpEnd, 
                                                    TS_TEXTCHANGE *pChange)
{
    OutputDebugString(TEXT("CTSFEditWnd::InsertTextAtSelection \n"));

    LONG    lTemp;
    
    //does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }

    //verify pwszText
    if(NULL == pwszText)
    {
        return E_INVALIDARG;
    }

    //verify pacpStart
    if(NULL == pacpStart)
    {
        pacpStart = &lTemp;
    }

    //verify pacpEnd
    if(NULL == pacpEnd)
    {
        pacpEnd = &lTemp;
    }

    LONG    acpStart;
    LONG    acpOldEnd;
    LONG    acpNewEnd;
    
    _GetCurrentSelection();

    acpOldEnd = m_acpEnd;
    _TestInsert(m_acpStart, m_acpEnd, cch, &acpStart, &acpNewEnd);

    if(dwFlags & TS_IAS_QUERYONLY)
    {
        *pacpStart = acpStart;
        *pacpEnd = acpOldEnd;
        return S_OK;
    }

    LPWSTR  pwszCopy;

    pwszCopy = (LPWSTR)GlobalAlloc(GMEM_FIXED, (cch + 1) * sizeof(WCHAR));
    if(NULL == pwszCopy)
    {
        return E_OUTOFMEMORY;
    }

    //pwszText will most likely not be NULL terminated
    CopyMemory(pwszCopy, pwszText, cch * sizeof(WCHAR));
    pwszCopy[cch] = 0;

    //don't notify TSF of text and selection changes when in response to a TSF action
    m_fNotify = FALSE;
    
    //insert the text
    ::SendMessage(m_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)pwszCopy);

    //set the selection
    ::SendMessage(m_hwndEdit, EM_SETSEL, acpStart, acpNewEnd);

    m_fNotify = TRUE;

    _GetCurrentSelection();

    if(!(dwFlags & TS_IAS_NOQUERY))
    {
        *pacpStart = acpStart;
        *pacpEnd = acpNewEnd;
    }

    //set the TS_TEXTCHANGE members
    pChange->acpStart = acpStart;
    pChange->acpOldEnd = acpOldEnd;
    pChange->acpNewEnd = acpNewEnd;

    GlobalFree(pwszCopy);

    //defer the layout change notification until the document is unlocked
    m_fLayoutChanged = TRUE;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::InsertEmbeddedAtSelection()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::InsertEmbeddedAtSelection(    DWORD dwFlags, 
                                                        IDataObject *pDataObject, 
                                                        LONG *pacpStart, 
                                                        LONG *pacpEnd, 
                                                        TS_TEXTCHANGE *pChange)
{
    OutputDebugString(TEXT("CTSFEditWnd::InsertEmbeddedAtSelection \n"));

    return E_NOTIMPL;
}



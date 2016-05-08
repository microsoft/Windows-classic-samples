//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  TextEditSink.cpp
//
//          ITfTextEditSink implementation.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// OnEndEdit
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    DumpExtent(pContext, DE_EVENTID_ONENDEDIT);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnLayoutChange
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView)
{
    DumpExtent(pContext, DE_EVENTID_ONLAYOUTCHANGE);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitTextEditSink
//
// Init a text edit sink on the topmost context of the document.
// Always release any previous sink.
//----------------------------------------------------------------------------

BOOL CExtentMonitorTextService::_InitTextEditSink(ITfDocumentMgr *pDocMgr)
{
    ITfSource *pSource;
    BOOL fRet;

    // clear out any previous sink first

    if (_pTextEditSinkContext)
    {
        if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
        {
            if (_dwTextEditSinkCookie != TF_INVALID_COOKIE)
                pSource->UnadviseSink(_dwTextEditSinkCookie);
            if (_dwTextLayoutSinkCookie != TF_INVALID_COOKIE)
                pSource->UnadviseSink(_dwTextLayoutSinkCookie);
            pSource->Release();
        }

        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
        _dwTextEditSinkCookie = TF_INVALID_COOKIE;
        _dwTextLayoutSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == NULL)
        return TRUE; // caller just wanted to clear the previous sink

    // setup a new sink advised to the topmost context of the document

    if (pDocMgr->GetBase(&_pTextEditSinkContext) != S_OK)
        return FALSE;

    if (_pTextEditSinkContext == NULL)
        return TRUE; // empty document, no sink possible

    fRet = FALSE;

    if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        if (pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwTextEditSinkCookie) == S_OK)
        {
            fRet = TRUE;
        }
        else
        {
            _dwTextEditSinkCookie = TF_INVALID_COOKIE;
        }

        if (pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &_dwTextLayoutSinkCookie) == S_OK)
        {
            fRet = TRUE;
        }
        else
        {
            _dwTextLayoutSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (fRet == FALSE)
    {
        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
    }

    return fRet;
}

//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  ThreadFocusSink.cpp
//
//          ITfThreadFocusSink interface
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "PopupWindow.h"

//+---------------------------------------------------------------------------
//
// OnSetThreadFocus
//
//----------------------------------------------------------------------------

STDAPI CPropertyMonitorTextService::OnSetThreadFocus()
{
    if (_pPopupWindow)
    {
       _pPopupWindow->Show();
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKillThreadFocus
//
//----------------------------------------------------------------------------

STDAPI CPropertyMonitorTextService::OnKillThreadFocus()
{
    if (_pPopupWindow)
    {
       _pPopupWindow->Hide();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitThreadFocusSink
//
//----------------------------------------------------------------------------

BOOL CPropertyMonitorTextService::_InitThreadFocusSink()
{
    ITfSource *pSource;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusCookie);
        pSource->Release();
    }

    return TRUE;
}
 
//+---------------------------------------------------------------------------
//
// _UninitThreadFocusSink
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_UninitThreadFocusSink()
{
    ITfSource *pSource;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_dwThreadFocusCookie);
        pSource->Release();
    }
}
 
 



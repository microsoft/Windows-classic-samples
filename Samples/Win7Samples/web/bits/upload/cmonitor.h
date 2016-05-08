//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cdialog.h
//
//  Purpose:
//  Defines the class CSimpleDialog, which implements the 
//  IBackgroundCopyCallback interface.
//
//----------------------------------------------------------------------------

#pragma once

#include <bits.h>

class CMonitor : public IBackgroundCopyCallback
{
    LONG m_lRefCount;

public:
    CMonitor() { m_lRefCount = 0;};
    ~CMonitor() {};

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG   STDMETHODCALLTYPE AddRef();
    ULONG   STDMETHODCALLTYPE Release();

    // IBackgroundCopyCallback methods
    HRESULT STDMETHODCALLTYPE JobTransferred(IBackgroundCopyJob* pJob);
    HRESULT STDMETHODCALLTYPE JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError);
    HRESULT STDMETHODCALLTYPE JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved);

private:
    HRESULT ProcessReply(IBackgroundCopyJob* pJob);
};


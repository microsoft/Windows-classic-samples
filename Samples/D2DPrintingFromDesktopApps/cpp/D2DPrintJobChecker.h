//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef _DIRECT2D_PRINT_JOB_CHECKER_
#define _DIRECT2D_PRINT_JOB_CHECKER_

#ifndef UNICODE
#define UNICODE
#endif

// DirectX header files.
#include <d2d1_1.h>
#include <xpsobjectmodel_1.h>
#include <DocumentTarget.h>

class D2DPrintJobChecker : public IPrintDocumentPackageStatusEvent
{
public:
    D2DPrintJobChecker();
    ~D2DPrintJobChecker();

    // Implement virtual functions from interface IUnknown.
    virtual
    HRESULT STDMETHODCALLTYPE
    QueryInterface(
                REFIID  iid,
        _Out_   void**  ppvObject
        );
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // Implement virtual functions from interface IDispatch.
    virtual STDMETHODIMP
        GetTypeInfoCount(
            _Out_ UINT *pctinfo
        );
    virtual STDMETHODIMP
    GetTypeInfo(
        UINT iTInfo,
        LCID lcid,
        _Outptr_result_maybenull_ ITypeInfo **ppTInfo
        );
    virtual STDMETHODIMP
    GetIDsOfNames(
        _In_                        REFIID      riid,
        _In_reads_(cNames)          LPOLESTR*   rgszNames,
        _In_range_(0, 16384)        UINT        cNames,
                                    LCID        lcid,
        __out_ecount_full(cNames)   DISPID*     rgDispId
        );
    virtual STDMETHODIMP
    Invoke(
        DISPID          dispIdMember,
        REFIID          riid,
        LCID            lcid,
        WORD            wFlags,
        DISPPARAMS*     pDispParams,
        VARIANT*        pVarResult,
        EXCEPINFO*      pExcepInfo,
        UINT*           puArgErr
        );

    // Implement virtual functions from interface IPrintDocumentPackageStatusEvent.
    virtual STDMETHODIMP
    PackageStatusUpdated(
        _In_ PrintDocumentPackageStatus* packageStatus
        );

    // New functions in D2DPrintJobChecker.
    HRESULT Initialize(
        _In_ IPrintDocumentPackageTarget* documentPackageTarget
        );
    PrintDocumentPackageStatus GetStatus();
    HRESULT WaitForCompletion();
    static void OutputPackageStatus(
        _In_ PrintDocumentPackageStatus packageStatus
        );

private:
    void ReleaseResources();

private:
    PrintDocumentPackageStatus m_documentPackageStatus;
    DWORD m_eventCookie;
    ULONG m_refcount;
    HANDLE m_completionEvent;
    CRITICAL_SECTION m_criticalSection;
    IConnectionPoint* m_connectionPoint;
    bool m_isInitialized;
};

#endif  // _DIRECT2D_PRINT_JOB_CHECKER_

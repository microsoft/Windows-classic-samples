//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "D2DPrintJobChecker.h"

#include <Strsafe.h>

D2DPrintJobChecker::D2DPrintJobChecker() :
    m_eventCookie(0),
    m_refcount(1),
    m_completionEvent(nullptr),
    m_connectionPoint(nullptr),
    m_isInitialized(false)
{
    PrintDocumentPackageStatus status = {0};
    m_documentPackageStatus = status;

    // Initialize the critical section, one time only.
    InitializeCriticalSection(&m_criticalSection);
}

D2DPrintJobChecker::~D2DPrintJobChecker()
{
    ReleaseResources();

    // Release resources used by the critical section object.
    DeleteCriticalSection(&m_criticalSection);
}

// Initialize this print job checker by registering a print document package target.
// This is required before the application can use this print job checker to monitor a print job.
HRESULT D2DPrintJobChecker::Initialize(
    _In_ IPrintDocumentPackageTarget* documentPackageTarget
    )
{
    // Application can only initialize this job checker once.
    if (m_isInitialized)
    {
        return E_FAIL;
    }

    HRESULT hr = (documentPackageTarget != nullptr) ? S_OK : E_INVALIDARG;

    IConnectionPointContainer* connectionPointContainer = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = documentPackageTarget->QueryInterface(IID_PPV_ARGS(&connectionPointContainer));
    }

    if (SUCCEEDED(hr))
    {
        hr = connectionPointContainer->FindConnectionPoint(
            __uuidof(IPrintDocumentPackageStatusEvent),
            &m_connectionPoint
            );
    }

    IUnknown* unknownEvent = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = this->QueryInterface(IID_PPV_ARGS(&unknownEvent));
    }

    if (SUCCEEDED(hr))
    {
        hr = m_connectionPoint->Advise(unknownEvent, &m_eventCookie);
    }

    // Create an event handle for print job completion.
    if (SUCCEEDED(hr))
    {
        m_completionEvent = ::CreateEvent(
            nullptr, // The handle cannot be inherited by child processes.
            TRUE,    // The event object requires the use of the ResetEvent function to set the event state to nonsignaled.
            FALSE,   // The initial state of the event object is nonsignaled.
            nullptr  // Name of the event object.
            );
        hr = (!m_completionEvent) ? HRESULT_FROM_WIN32(::GetLastError()) : hr;
    }

    if (unknownEvent)
    {
        unknownEvent->Release();
        unknownEvent = nullptr;
    }
    if (connectionPointContainer)
    {
        connectionPointContainer->Release();
        connectionPointContainer = nullptr;
    }

    if (SUCCEEDED(hr))
    {
        m_isInitialized = true;
    }
    else
    {
        ReleaseResources();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE
D2DPrintJobChecker::QueryInterface(
    REFIID iid,
    _Out_ void** ppvObject
    )
{
    if (iid == __uuidof(IUnknown) ||
        iid == __uuidof(IPrintDocumentPackageStatusEvent))
    {
        *ppvObject = static_cast<IPrintDocumentPackageStatusEvent*>(this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE
D2DPrintJobChecker::AddRef()
{
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG STDMETHODCALLTYPE
D2DPrintJobChecker::Release()
{
    ULONG res = (ULONG)InterlockedDecrement(&m_refcount);
    if (0 == res)
    {
        delete this;
    }
    return res;
}

HRESULT D2DPrintJobChecker::GetTypeInfoCount(
    _Out_ UINT* /* pctinfo */
    )
{
    return E_NOTIMPL;
}

HRESULT D2DPrintJobChecker::GetTypeInfo(
    UINT /* iTInfo */,
    LCID /* lcid */,
    _Outptr_result_maybenull_ ITypeInfo** /* ppTInfo */
    )
{
    return E_NOTIMPL;
}

HRESULT D2DPrintJobChecker::GetIDsOfNames(
    _In_                        REFIID     /* riid */,
    _In_reads_(cNames)          LPOLESTR*  /* rgszNames */,
    _In_range_(0, 16384)        UINT       cNames,
                                LCID       /* lcid */,
    __out_ecount_full(cNames)   DISPID*    /* rgDispId */
    )
{
    UNREFERENCED_PARAMETER(cNames);
    return E_NOTIMPL;
}

HRESULT D2DPrintJobChecker::Invoke(
    DISPID          /* dispIdMember */,
    REFIID          /* riid */,
    LCID            /* lcid */,
    WORD            /* wFlags */,
    DISPPARAMS*     /* pDispParams */,
    VARIANT*        /* pVarResult */,
    EXCEPINFO*      /* pExcepInfo */,
    UINT*           /* puArgErr */
    )
{
    return E_NOTIMPL;
}

// Callback to indicate that the status of the package has changed.
STDMETHODIMP D2DPrintJobChecker::PackageStatusUpdated(
    _In_ PrintDocumentPackageStatus* packageStatus
    )
{
    HRESULT hr = (packageStatus == nullptr) ? E_INVALIDARG : S_OK;

    if (SUCCEEDED(hr))
    {
        // The package status update operation is guarded with a critical section,
        // since PackageStatusUpdated may be called in a very short time slot.
        EnterCriticalSection(&m_criticalSection);
        m_documentPackageStatus = *packageStatus;
        LeaveCriticalSection(&m_criticalSection);
    }

    if (SUCCEEDED(hr))
    {
        // Signal the print job complete event.
        if (PrintDocumentPackageCompletion_InProgress != m_documentPackageStatus.Completion)
        {
            if (!SetEvent(m_completionEvent))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
            }
        }
    }

#if defined(_DEBUG)
    OutputPackageStatus(m_documentPackageStatus);
#endif

    return S_OK;
}

// Return print document package status.
PrintDocumentPackageStatus D2DPrintJobChecker::GetStatus()
{
    return(m_documentPackageStatus);
}

// Wait for job completion event to be signaled; in the meanwhile keep pumping messages to avoid deadlocks.
//
// This is an example for deadlock: when user print to a " Print To File" printer, the winspool.dll will try to pop
// up a modal dialog asking for file name. In order to show modal dialog, winspool will call EnableWindow(FALSE) to
// disable this window. Therefore, the winspool thread is waiting for us to process the message of disabling the
// window, while our thread is waiting for the winspool thread to complete.
HRESULT D2DPrintJobChecker::WaitForCompletion()
{
    // Return successfully if this job checker is not initialized.
    if (!m_isInitialized)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;

    bool isWaiting = true;
    while (isWaiting)
    {
        DWORD waitResult = ::MsgWaitForMultipleObjects(
            1,                  // Number of object handles in the array.
            &m_completionEvent, // Array of object handles.
            FALSE,              // Function returns when the state of any one of the objects is set to signaled.
            INFINITE,           // Function will return only when the specified objects are signaled.
            QS_ALLINPUT         // Any message is in the queue.
            );
        if (waitResult == WAIT_OBJECT_0 + 1)
        {
            // This is not the desired completion event.
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                ::DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT)
            {
                isWaiting = false;
                break;
            }
        }
        else
        {
            isWaiting = false;
        }
    }

    return hr;
}

// Output debug message for the print document package status.
void D2DPrintJobChecker::OutputPackageStatus(
    _In_ PrintDocumentPackageStatus packageStatus
    )
{
    switch (packageStatus.Completion)
    {
    case PrintDocumentPackageCompletion_Completed:
        OutputDebugString(L"PrintDocumentPackageCompletion_Completed; ");
        break;

    case PrintDocumentPackageCompletion_Canceled:
        OutputDebugString(L"PrintDocumentPackageCompletion_Canceled; ");
        break;

    case PrintDocumentPackageCompletion_Failed:
        OutputDebugString(L"PrintDocumentPackageCompletion_Failed; ");
        break;

    case PrintDocumentPackageCompletion_InProgress:
        OutputDebugString(L"PrintDocumentPackageCompletion_InProgress; ");
        break;

    default:
        OutputDebugString(L"PrintDocumentPackageStatus unknown! ");
        break;
    }
    WCHAR messageBuffer[256] = {0};
    StringCchPrintf(
        messageBuffer,
        ARRAYSIZE(messageBuffer),
        L"\tjobID:%3d, currentDoc:%3d, currentPage:%3d, currentPageTotal:%3d, packageStatus:0x%08X\n",
        packageStatus.JobId,
        packageStatus.CurrentDocument,
        packageStatus.CurrentPage,
        packageStatus.CurrentPageTotal,
        packageStatus.PackageStatus
        );
    OutputDebugString(messageBuffer);
}

// Release resources.
void D2DPrintJobChecker::ReleaseResources()
{
    if (m_connectionPoint != nullptr)
    {
        if (m_eventCookie != 0)
        {
            m_connectionPoint->Unadvise(m_eventCookie);
        }
        m_connectionPoint->Release();
        m_connectionPoint = nullptr;
    }

    if (m_completionEvent != nullptr)
    {
        ::CloseHandle(m_completionEvent);
        m_completionEvent = nullptr;
    }

    return;
}
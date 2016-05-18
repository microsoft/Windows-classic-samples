//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cmonitor.cpp
//
//  Purpose:
//  This module implements the CMonitor class, which is 
//  used in the sample to respond to job callbacks.
// 
//  Take special note of the CMonitor::JobTransferred() method. It executes
//  the important step of calling Job->Complete(), which causes the file
//  upload to be effectively completed. 
//  
//  Also note the treatment of Upload-reply jobs. If the job was created
//  with the type BG_JOB_TYPE_UPLOAD_REPLY, then additional processing is 
//  needed to actually read the reply and do something with it.
//  For the client to see upload replies, the IIS virtual directory
//  needs to be configured to process notifications, and the processing
//  application needs to post data back (see the script newupload.asp).
//
//  The script configure.js, included with this sample, provides a convenient
//  way of configuring the IIS upload virtual directory to work properly
//  with this sample.
//
//----------------------------------------------------------------------------


#include <windows.h>
#include <crtdbg.h>
#include <unknwn.h>
#include <strsafe.h>
#include <bits.h>

#include "util.h"
#include "main.h"
#include "cmonitor.h"


//----------------------------------------------------------------------------
// CMonitor's IUnknown methods 
//----------------------------------------------------------------------------

HRESULT CMonitor::QueryInterface(REFIID riid, LPVOID* ppvObj) 
{
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IBackgroundCopyCallback))
    {
        *ppvObj = this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

ULONG CMonitor::AddRef() 
{
    if (m_lRefCount == 0) // first time this function is called
    {
        g_hSafeToExitEvent = CreateEvent( 
             NULL,   // default security attributes
             FALSE,  // auto-reset event object
             FALSE,  // initial state is nonsignaled
             NULL);  // unnamed object

        // in case of failure just act as we didn't create the event
        if (!g_hSafeToExitEvent)
        {
            g_hSafeToExitEvent = INVALID_HANDLE_VALUE;
        }
    }

    return InterlockedIncrement(&m_lRefCount);
}

ULONG CMonitor::Release() 
{
    ULONG ulCount = InterlockedDecrement(&m_lRefCount);

    if (ulCount == 0)
    {
        if (g_hSafeToExitEvent != INVALID_HANDLE_VALUE)
        {
            SetEvent(g_hSafeToExitEvent);
        }
    }

    return ulCount;
}


//----------------------------------------------------------------------------
// CMonitor's IBackgroundCopyJob methods
//
// Note that the implementation of these methods need to be thread-safe
//
//----------------------------------------------------------------------------

HRESULT CMonitor::JobTransferred(IBackgroundCopyJob* pJob)
{
    HRESULT hr;
    GUID    guid;
    BG_JOB_TYPE JobType;


    hr = pJob->GetType(&JobType);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    if (JobType == BG_JOB_TYPE_UPLOAD_REPLY)
    {
        hr = ProcessReply(pJob);
        if (FAILED(hr))
        {
            goto cleanup;
        }
    }

    hr = pJob->Complete();
    if (FAILED(hr))
    {
        // BITS probably was unable to rename one or more of the 
        // temporary files. We won't do anything about this
        goto cleanup;
    }

    hr = pJob->GetId(&guid);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    g_pDialog->AddStatusMessage(L"Notification received (BG_NOTIFY_JOB_TRANSFERRED). File was uploaded! (Job ID %s)", ConvertGuidToString(guid));
    g_pDialog->AddStatusMessage(L"END OF UPLOAD PROCESS");


cleanup:

    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Notification received (BG_NOTIFY_JOB_TRANSFERRED), but an error occured. Error id is 0x%X", hr);
    }

    //If you do not return S_OK, BITS continues to call this callback.
    return S_OK;
}


HRESULT CMonitor::JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError)
{
    HRESULT hr;
    GUID    guid;
    HRESULT hrErrorId;
    WCHAR  *pwszErrorDescription   = NULL;
    WCHAR  *pwszContextDescription = NULL;
    BG_ERROR_CONTEXT Context;


    //
    // Collect information about the failed job
    //

    hr = pJob->GetId(&guid);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pError->GetError(&Context, &hrErrorId);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pError->GetErrorContextDescription(MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), &pwszContextDescription);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pError->GetErrorDescription(MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), &pwszErrorDescription);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //
    // Cancel the job because the error is not recoverable
    //
    hr = pJob->Cancel();

    g_pDialog->AddStatusMessage(L"Notification received (BG_NOTIFY_JOB_ERROR). (Job ID %s)\r\n"
        L"   Error context: %s"
        L"   Error id: Ox%X\r\n"
        L"   Error description: %s"
        L"   This error is not recoverable and the job will be cancelled.",
        ConvertGuidToString(guid),
        pwszContextDescription,
        hrErrorId,
        pwszErrorDescription
    );
    g_pDialog->AddStatusMessage(L"END OF UPLOAD PROCESS -- FAILED");

cleanup:

    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Notification received (BG_NOTIFY_JOB_ERROR), but the error description could not be retrieved and the job was not cancelled. Error id is 0x%X", hr);
    }

    // safe to call these if pointer is NULL
    CoTaskMemFree(pwszContextDescription);
    CoTaskMemFree(pwszErrorDescription);

    //If you do not return S_OK, BITS continues to call this callback.
    return S_OK;
}

HRESULT CMonitor::JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved)
{
    HRESULT      hr;
    GUID         guid;
    BG_JOB_STATE State;
    WCHAR        *pwszErrorDescription   =    NULL;
    WCHAR        *pwszContextDescription =    NULL;
    HRESULT hrErrorId;
    BG_ERROR_CONTEXT Context;
    
    hr = pJob->GetState(&State);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"BITS Upload Sample was unable to get the BITS job state. ", hr);
        return S_OK;
    }

    if (State == BG_JOB_STATE_TRANSIENT_ERROR)
    {
        CSmartComPtr<IBackgroundCopyError> Error;

        hr = pJob->GetId(&guid);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = pJob->GetError(&Error);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = Error->GetError(&Context, &hrErrorId);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = Error->GetErrorContextDescription(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &pwszContextDescription);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = Error->GetErrorDescription(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &pwszErrorDescription);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        g_pDialog->AddStatusMessage(L"Job entered a transient error state. (Job ID %s)\r\n"
            L"   Error context: %s"
            L"   Error id: 0x%X\r\n"
            L"   Error description: %s"
            L"   BITS will retry to upload the file.",
            ConvertGuidToString(guid),
            pwszContextDescription,
            hrErrorId,
            pwszErrorDescription
        );
    }

cleanup:

    // safe to call these if pointer is NULL
    CoTaskMemFree(pwszContextDescription);
    CoTaskMemFree(pwszErrorDescription);

    return S_OK;
}

//----------------------------------------------------------------------------
// CMonitor's private methods
//----------------------------------------------------------------------------

HRESULT CMonitor::ProcessReply(IBackgroundCopyJob* pJob)
{
    HRESULT hr;
    CSmartComPtr<IBackgroundCopyJob2> Job2;
    BYTE  *pReply    = NULL;
    WCHAR *pTextData = NULL;
    UINT64 ReplySize;
    GUID   guid;

    //
    // Initialize our Job2 smart object. Note the use of the & operator
    // 
    hr = pJob->QueryInterface(__uuidof(IBackgroundCopyJob2), (void**)&Job2);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pJob->GetId(&guid);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = Job2->GetReplyData(&pReply, &ReplySize);
    if (hr == S_OK)
    {
        // check if we got the reply data.
        // As we are expecting to receive a Unicode file as a reply, check that at least
        // 2 bytes were received. 2 bytes is the size of the unicode prologue
        if (pReply && (ReplySize > 2))   
        {
            // allocate a buffer to copy the data
            // we need to do this because we need to add a \0 at the end of the
            // data stream before we can treat it as a WCHAR string
            pTextData = reinterpret_cast<WCHAR *>(new BYTE[static_cast<size_t>(ReplySize)]);
            if (!pTextData)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            // now here's something tricky: we want to add a \0 to the end of the string,
            // but we also want to skip 2 bytes at the beguinning of the string.
            // Note that overall we have enough space by allocating just ReplySize

            // initialize our buffer with zeros
            memset(pTextData, 0, static_cast<size_t>(ReplySize));

            // copy the relevant data to the buffer, skipping 2 bytes because of the Unicode prologue
            // note that pTextData is WCHAR* and pReply is BYTE*
            memcpy(pTextData, (pReply + 2), (static_cast<size_t>(ReplySize) - 2));

            g_pDialog->AddStatusMessage(L"The server sent the following data as a response to the uploaded file: (Job ID %s)\r\n"
                                        L"\r\n%s", ConvertGuidToString(guid), pTextData);

        }
        else
        {
            g_pDialog->AddStatusMessage(L"No reply data was received. (Job ID %s)", ConvertGuidToString(guid));
        }
    }

cleanup:

    if (pReply)
    {
        CoTaskMemFree(pReply);
        pReply = NULL;
    }

    if (pTextData)
    {
        delete [] reinterpret_cast<BYTE *>(pTextData);
        pTextData = NULL;
    }

    return hr;
}

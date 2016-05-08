// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "DOWNLOADS.h"
#include "stdafx.h"

void _cdecl _tmain(int argc, LPWSTR* argv)
{	
    GUID guidJob;
    HRESULT hr;
	IBackgroundCopyManager *pQueueMgr;
	IBackgroundCopyJob* pJob = NULL;
    CNotifyInterface *pNotify;

    if (argc != 3)
    {
        wprintf(L"Usage:");
        wprintf(L"%s", argv[0]);
        wprintf(L"[remote name] [local name]\n");
        goto finished;
    }

    //Specify the COM threading model.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        //The impersonation level must be at least RPC_C_IMP_LEVEL_IMPERSONATE.
        hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
             RPC_C_AUTHN_LEVEL_CONNECT,
             RPC_C_IMP_LEVEL_IMPERSONATE,
             NULL, EOAC_NONE, 0);

        if (SUCCEEDED(hr))
        {

	        // Connect to BITS
            hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
                CLSCTX_LOCAL_SERVER,
                __uuidof(IBackgroundCopyManager),
                (void **)&pQueueMgr);

	        if (FAILED(hr))
	        {
		        // Failed to connect
                wprintf(L"Failed to connect to BITS.Error: 0x%x\n",hr);
		        goto done;
	        }
        }
        else
        {
            //Failed to impersonate
            wprintf(L"CoInitializeSecurity Failed. Error:0x%x\n",hr);
            goto done;
        }
    }
    else
    {
        wprintf(L"CoInitializeEx Failed. Error:0x%x",hr);
        goto done;
    }

	// Create a Job
    wprintf(L"Creating Job...\n");
    hr = pQueueMgr->CreateJob(L"P2PSample",
         BG_JOB_TYPE_DOWNLOAD,
         &guidJob,
         &pJob);
	
    // Free Resources
    pQueueMgr->Release();

	if(FAILED(hr))
    {   
        wprintf(L"Create Job failed with error: %x\n",hr);
    	goto done;
    }
    
    // Set the File Completed Call
    pNotify = new CNotifyInterface();
    if (pNotify)
    {
        hr = pJob->SetNotifyInterface(pNotify);
        if (SUCCEEDED(hr))
        {
            hr = pJob->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | 
                                BG_NOTIFY_JOB_ERROR);
        }

        // Free resouces
        pNotify->Release();
        pNotify = NULL;

        if (FAILED(hr))
        {
            wprintf(L"Unable to register callbacks\nError: %x\n",hr);
            wprintf(L"Cancelling job\n");
            goto cancel;
        }
    }
    else
    {
        wprintf(L"Could not create the Notification Interface\n");
        wprintf(L"Cancelling job\n");
        goto cancel;
    }
	// Add a File
	// Replace parameters with variables that contain valid paths.
    wprintf(L"Adding File to Job\n");
	hr = pJob->AddFile(argv[1], argv[2]);

    if(FAILED(hr))
    {   
        wprintf(L"Add File failed with error: %x\n",hr);
    	goto cancel;
    }

	//Resume the job
    wprintf(L"Resuming Job...\n");
	hr = pJob->Resume();
	if (FAILED(hr))
	{
		// Resume Failed
        wprintf(L"Resume failed with error: %x\n",hr);
        wprintf(L"Cancelling Job\n");
		goto cancel;
	}    

    // Wait for QuitMessage from CallBack
    DWORD dwLimit = GetTickCount() + (15 * 60 * 1000);  // set 15 minute limit
    while (dwLimit > GetTickCount())
    {
         MSG msg;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        { 
            // If it is a quit message, exit.
            if (msg.message == WM_QUIT) 
            {
                pJob->Release();
                CoUninitialize();
                return; 
            }
            // Otherwise, dispatch the message.
            DispatchMessage(&msg); 
        } // End of PeekMessage while loop
    }

done:
    CoUninitialize();
finished:
	return;
cancel:
    pJob->Cancel();
    pJob->Release();
    goto done;
}

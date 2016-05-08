// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "PEERCACHING.h"
#include "stdafx.h"

HRESULT InDomain(bool *b)
{
    DWORD s = 0;

    // get the computer name and domain

    LPWSTR name = NULL;
    NETSETUP_JOIN_STATUS status;

    *b = false;

    s = NetGetJoinInformation( NULL, &name, &status );
    if (s != 0)
    {
       return HRESULT_FROM_WIN32(s);
    }
    NetApiBufferFree(name);
    name = NULL;

    if (status == NetSetupDomainName)
    {
        *b = true;
    }

    return S_OK;
}

void _cdecl _tmain(int argc, LPWSTR* argv)
{	
    GUID guidJob;
    HRESULT hr;
	IBackgroundCopyManager *pQueueMgr;
	IBackgroundCopyJob* pJob = NULL;
    IBitsPeerCacheAdministration* pCacheAdmin = NULL;
    IBackgroundCopyJobHttpOptions* pHttp = NULL;
    IBackgroundCopyJob4* pJob4 = NULL;
    IEnumBitsPeers* pPeers = NULL;
    IBitsPeer* pPeer = NULL;
    LPWSTR pwszPeerName = NULL;
    CNotifyInterface *pNotify;
    ULONG ulCount = 0;
	bool b;

    //
    // BITS requires that you run in a domain for peercaching
    // to be used.  This will be important in the following
    // examples.
    if (FAILED(InDomain(&b)))
    {
        wprintf(L"Unable to determine if joined to domain\n");
        return;
    }

    //
    // Use Group Policy (gpedit) to enable peer caching.
    if (!b)
    {
        wprintf(L"Peer Caching is enabled only in a domain environment\n");
        return;
    }

    if (argc != 3)
    {
        wprintf(L"Usage:");
        wprintf(L"%s", argv[0]);
        wprintf(L"[remote name] [local name]\n");
        return;
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

    // Find out who is in the neighborhood
    hr = pQueueMgr->QueryInterface(
        __uuidof(IBitsPeerCacheAdministration), 
        (void **)&pCacheAdmin);
    
	if(FAILED(hr))
    {   
        wprintf(L"IBitsPeerCacheAdministration failed with error: %x\n",hr);
    }
    else
    {
        pCacheAdmin->EnumPeers( &pPeers );
        if (pPeers)
        {
            hr = pPeers->GetCount( &ulCount );
            
            if(FAILED(hr))
            {   
                wprintf(L"pPeers->GetCount failed with error: %x\n",hr);
            }
            else
            {
                wprintf(L"Peers count: %li\n", ulCount);
                while( S_OK == pPeers->Next( 1, &pPeer, &ulCount ))
                {
                    pPeer->GetPeerName( &pwszPeerName );
                    wprintf(L"Neighbor: %s\n", pwszPeerName);
                    pPeer->Release();
                }
            }

            // Free the resource
            pPeers->Release();
        }

        // Free up resource
        pCacheAdmin->Release();
    }

	// Create a Job
    wprintf(L"Creating Job...\n");
    hr = pQueueMgr->CreateJob(L"P2PSample",
        BG_JOB_TYPE_DOWNLOAD,
        &guidJob,
        &pJob);

	if(FAILED(hr))
    {   
        wprintf(L"Create Job failed with error: %x\n",hr);
	    pQueueMgr->Release();
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
                                BG_NOTIFY_JOB_ERROR | 
                                BG_NOTIFY_FILE_TRANSFERRED );
        }
        pNotify->Release();
        pNotify = NULL;
        if (FAILED(hr))
        {
            wprintf(L"Unable to register callbacks\nError: %x\n",hr);
            wprintf(L"Cancelling job\n");
	        pQueueMgr->Release();
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
    	goto done;
    }

	// Get the IBitsPeerCacheAdministration interface     
    hr = pQueueMgr->QueryInterface(__uuidof(IBitsPeerCacheAdministration), 
        (void **)&pCacheAdmin);

    // We don't need this again
	pQueueMgr->Release();
	
    if (!FAILED(hr))
    {
	    // Enable local caching for this job
	    hr = pCacheAdmin->SetConfigurationFlags(BG_ENABLE_PEERCACHING_CLIENT
            || BG_ENABLE_PEERCACHING_SERVER);

        // Free the resource
        pCacheAdmin->Release();

	    if (FAILED(hr))
	    {
		    //Log the error
            wprintf(L"SetConfigurationFlags Failed with error: %x\n",hr);
	    }

        hr = pJob->QueryInterface(__uuidof(IBackgroundCopyJob4), 
            (void **)&pJob4);
        
        if (!FAILED(hr))
        {
            pJob4->SetPeerCachingFlags(
                BG_JOB_ENABLE_PEERCACHING_CLIENT | BG_JOB_ENABLE_PEERCACHING_SERVER);

            pJob4->Release();
        }
    }
	
	// Get the IBackgroundCopyJobHttpOptions interface     
    hr = pJob->QueryInterface(__uuidof(IBackgroundCopyJobHttpOptions), 
        (void **)&pHttp);

    if (pHttp)
    {
        // Say it's ok to get it elsewhere, but tell us later
        hr = pHttp->SetSecurityFlags(BG_HTTP_REDIRECT_POLICY_ALLOW_REPORT);

        // Free resources
        pHttp->Release();

	    if (FAILED(hr))
	    {
		    //Log the error
            wprintf(L"SetSecurityFlags failed with error: %x\n",hr);
	    }
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
            // the job
            if (msg.message == WM_QUIT)
            {
                pJob->Release();
                goto done; 
            }
            // Otherwise, dispatch the message.
            DispatchMessage(&msg); 
        } // End of PeekMessage while loop
    }

done:
    CoUninitialize();
	return;
cancel:
    pJob->Cancel();
    pJob->Release();
    goto done;
}

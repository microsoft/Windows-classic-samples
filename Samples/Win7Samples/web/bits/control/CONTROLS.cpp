#include "stdafx.h"
#include <string.h>
#include <windows.h>
#include <string.h>
#include <limits.h>
#include <bits3_0.h>
#include <comdef.h>
#include <Userenv.h>
#include <winerror.h>
#include <objbase.h>
#include <tchar.h>
#include <strsafe.h>

void _cdecl _tmain( int argc, LPWSTR argv[])
{	
    GUID guidJob;
    HRESULT hr;
	IBackgroundCopyManager *pQueueMgr;
	IBackgroundCopyJob* pJob = NULL;
	
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
        		// FAiled to connect
                wprintf(L"CoCreateInstance failed with error: %x\n",hr);
        		goto done;
        	}

        	// Create a Job
            hr = pQueueMgr->CreateJob(L"MyJobName",
                BG_JOB_TYPE_DOWNLOAD,
                &guidJob,
                &pJob);    	

        	if(FAILED(hr))
            {
                wprintf(L"CreateJob failed with error: %x\n",hr);
                pQueueMgr->Release();
            	goto done;
            }

        	// Get the pointer to cache control
            IBitsPeerCacheAdministration* pCacheControl = NULL;
            hr = pQueueMgr->QueryInterface(__uuidof(IBitsPeerCacheAdministration), (void **) &pCacheControl);
        	if (FAILED(hr))
        	{
                wprintf(L"pJob->QueryInterface failed with error: %x\n",hr);
                wprintf(L"Cancelling Job\n");
        		goto cancel;
        	}

            // Free Resources
            pQueueMgr->Release();

        	// Add a File	
        	hr = pJob->AddFile(argv[1], argv[2]);
        	if (FAILED(hr))
        	{
                wprintf(L"AddFile failed with error: %x\n",hr);
                wprintf(L"Cancelling Job\n");
        		goto cancel;
        	}

        	// Enable local caching for this job
        	hr = pCacheControl->SetConfigurationFlags(BG_ENABLE_PEERCACHING_CLIENT
                || BG_ENABLE_PEERCACHING_SERVER);

        	if (FAILED(hr))
        	{
                wprintf(L"SetPolicyFlags failed with error: %x\n",hr);
                wprintf(L"Cancelling Job\n");
                pCacheControl->Release();
        		goto cancel;
        	}

        	// Set Max Cache Size to 2% of disk
            hr = pCacheControl->SetMaximumCacheSize(2);
        	if (FAILED(hr))
        	{
        		// Resume Failed
                wprintf(L"SetMaximumCacheSize failed with error: %x\n",hr);
                wprintf(L"Cancelling Job\n");
        		goto cancel;
        	}

            // Free resources
            pCacheControl->Release();
        	
        	//Resume the job
        	hr = pJob->Resume();
        	if (FAILED(hr))
        	{
        		// Resume Failed
                wprintf(L"Resume failed with error: %x\n",hr);
                wprintf(L"Cancelling Job\n");
        		goto cancel;
        	}
        	
        	//Wait for completion
        	BG_JOB_STATE dwStatus = (BG_JOB_STATE)0;
            DWORD dwLimit = GetTickCount() + (5 * 60 * 1000);  // set 5 minute limit
        	while (dwLimit > GetTickCount())
            {
                hr = pJob->GetState( &dwStatus);
        	    if (FAILED(hr))
                {
                    wprintf(L"Job->GetState failed with error: %x\n",hr);
                    wprintf(L"Cancelling Job\n");
                    goto cancel;
                }

        		//If the transfer is complete or enters an error state, exit
        		if (dwStatus & (BG_JOB_STATE_TRANSFERRED|BG_JOB_STATE_ERROR))
        		{
        			break;
        		}
                Sleep(1000);
            }
        	
        	if (dwStatus & BG_JOB_STATE_TRANSFERRED)
        	{
        		hr = pJob->Complete();
        	    if (FAILED(hr))
                {
                    wprintf(L"Job->Complete failed with error: %x\n",hr);
                }
        	}
        	else 
        	{
        		hr = pJob->Cancel();
        	    if (FAILED(hr))
                {
                    wprintf(L"Job->Cancel failed with error: %x\n",hr);
                }
        	}
                
            pJob->Release();
        }
        
done:       
        CoUninitialize();
    }
	return;
cancel:
    pJob->Cancel();
    pJob->Release();
    goto done;
}

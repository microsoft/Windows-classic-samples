// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"


int _tmain()
{
	HRESULT hr = S_OK;
	GUID guidJob;
	IBackgroundCopyJob5* pBackgroundCopyJob5;	
	IBackgroundCopyJob* pBackgroundCopyJob;
	IBackgroundCopyManager* pQueueMgr;
	BITS_JOB_PROPERTY_VALUE propval;

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
			hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
				CLSCTX_LOCAL_SERVER,
				__uuidof(IBackgroundCopyManager),
				(void **)&pQueueMgr);

			if (FAILED(hr))
			{
				// Failed to connect to BITS
				wprintf(L"Failed to connect to BITS with error %x\n",hr);
				goto done;
			}

			// Create a Job
			wprintf(L"Creating Job...\n");

			hr = pQueueMgr->CreateJob(L"TransferPolicy",
				BG_JOB_TYPE_DOWNLOAD,
				&guidJob,
				(IBackgroundCopyJob **)&pBackgroundCopyJob);

			if(FAILED(hr))
			{   
				wprintf(L"Failed to Create Job, error = %x\n",hr);
				goto cancel;
			}

			wprintf(L" Job is succesfully created ...\n");

			// Set Transfer Policy for the job
			propval.Dword = BITS_COST_STATE_USAGE_BASED | BITS_COST_STATE_OVERCAP_THROTTLED| BITS_COST_STATE_BELOW_CAP| BITS_COST_STATE_CAPPED_USAGE_UNKNOWN| BITS_COST_STATE_UNRESTRICTED;

			hr = pBackgroundCopyJob->QueryInterface( 
				__uuidof(IBackgroundCopyJob5), 
				reinterpret_cast<void**>(&pBackgroundCopyJob5) 
				);

			if(FAILED(hr))
			{   
				wprintf(L"Failed to Create Job, error = %x\n",hr);
				goto cancel;
			}
			pBackgroundCopyJob5->SetProperty(BITS_JOB_PROPERTY_ID_COST_FLAGS, propval);

			// get Transfer Policy for the new job
			BITS_JOB_PROPERTY_VALUE actual_propval;

			wprintf(L"Getting TransferPolicy Property ...\n");

			hr = pBackgroundCopyJob5->GetProperty( BITS_JOB_PROPERTY_ID_COST_FLAGS, &actual_propval );
			if (FAILED(hr))
			{
				//SetSSLSecurityFlags Failed
				wprintf(L"GetProperty failed with error %x\n",hr);
				goto cancel;
			}

			DWORD job_transferpolicy = actual_propval.Dword;
			wprintf(L"get TransferPolicy Property returned %#x\n", job_transferpolicy);
		}
done:
		CoUninitialize();
	}
	return 1;

cancel:
	pBackgroundCopyJob->Cancel();
	pBackgroundCopyJob->Release();
	goto done;
}


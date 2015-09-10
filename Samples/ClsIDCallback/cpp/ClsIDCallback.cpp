// ClsIDSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NotifyInterfaceImp.h"
#include "utils.h"

DOWNLOAD_FILE FileList[] =
{
	{
		L"http://download.microsoft.com/download/D/2/2/D22D16C3-7637-41D3-99DA-10E7CEBAD290/SQL2008UpgradeTechnicalReferenceGuide.docx",
		L"C:\\BitsSample\\SQL2008UpgradeTechnicalReferenceGuide.docx"
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	MSG		msg;
	HRESULT	hr = S_OK;
	bool bRun = true;
	BOOL bRet = FALSE; 

	FormatObjectPath();
	if (_wcsicmp(argv[1], L"UnRegServer") == 0)
	{
		UnRegisterServer();
		return 0;
	}

	if (_wcsicmp(argv[1], L"RegServer") == 0)
	{
		RegisterServer();
		return 0;
	}

	//Specify the COM threading model.
	hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (_wcsicmp(argv[1], L"Demo") == 0)
	{
		HRESULT result = S_OK;
		GUID guidJob;
		IBackgroundCopyJob5* pBackgroundCopyJob5 = NULL;	
		IBackgroundCopyJob* pBackgroundCopyJob = NULL;
		IBackgroundCopyManager* pQueueMgr = NULL;
		BITS_JOB_PROPERTY_VALUE propval;

		result = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
			CLSCTX_LOCAL_SERVER,
			__uuidof(IBackgroundCopyManager),
			(void **)&pQueueMgr);

		if (FAILED(result))
		{
			// Failed to connect to BITS
			wprintf(L"Failed to connect to BITS with error %x\n",result);
			goto cancel;
		}

		// Create a Job
		wprintf(L"Creating Job...\n");

		result = pQueueMgr->CreateJob(L"BITS_CLSID_SAMPLE",
			BG_JOB_TYPE_DOWNLOAD,
			&guidJob,
			(IBackgroundCopyJob **)&pBackgroundCopyJob);


		if(FAILED(result))
		{   
			wprintf(L"Failed to Create Job, error = %x\n",result);
			goto cancel;
		}

		result = pBackgroundCopyJob->QueryInterface( 
			__uuidof(IBackgroundCopyJob5), 
			reinterpret_cast<void**>(&pBackgroundCopyJob5) 
			);

		if(FAILED(result))
		{   
			wprintf(L"Failed to Get the Job Interface, error = %x\n",result);
			goto cancel;
		}

		// Set the CLS ID.
		propval.ClsID = CLSID_CNotifyInterfaceImp;

		wprintf(L"Setting CLSID Callback Property ...\n");

		result = pBackgroundCopyJob5->SetProperty(BITS_JOB_PROPERTY_NOTIFICATION_CLSID, propval);
		if(FAILED(result))
		{   
			wprintf(L"Failed to Set the CLSID, error = %x\n",result);
			goto cancel;
		}

		// get CLSID for the new job
		BITS_JOB_PROPERTY_VALUE actual_propval;

		result = pBackgroundCopyJob5->GetProperty( BITS_JOB_PROPERTY_NOTIFICATION_CLSID, &actual_propval );
		if (FAILED(result))
		{
			wprintf(L"GetProperty failed with error %x\n",result);
			goto cancel;
		}

		// actual_propval.ClsID will contain the CLSID registered for the Job.

		wprintf(L"Setting notification flags ...\n");
		// Set appropriate notify flags for the Job
		result = pBackgroundCopyJob5->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR);
		if(FAILED(result))
		{
			wprintf(L"Failed to SetNotifyFlags the Job, error = %x\n",result);
			goto cancel;
		}

		wprintf(L"Adding Download files ...\n");
		// Now add one or more files to the Job using AddFile() and Resume() the job.
		result = pBackgroundCopyJob5->AddFile(FileList[0].RemoteFile, FileList[0].LocalFile);

		if( FAILED(result) )
		{
			wprintf(L"Error: Unable to add remote file to the download job (error %08X).\n",result);
			goto cancel;
		}

		// Start the download
		result = pBackgroundCopyJob5->Resume();

		if(FAILED(result))
		{
			wprintf(L"Error: Unable to resume the download job (error %08X).\n",result);
			goto cancel;
		}
		wprintf(L"Download started, terminating the process.\n");
		wprintf(L"BITS should start a new process when transfer is done.\n");

		// It is OK to terminate the application after this. BITS will instantiate the 
		// CLSID to deliver the registered callbacks of the job.

		goto done;

		// NOTE: In actual scenario please do not call Cancel() until the Job is done with the download.
cancel:
		if(pBackgroundCopyJob)
		{
			pBackgroundCopyJob->Cancel();
			pBackgroundCopyJob->Release();
		}

done:
		bRun = false;
	}

	if (bRun)
	{
		DWORD dwCookie_ExeObj01 = 0;
		DWORD dwCookie_CNotifyInterfaceImp = 0;

		g_dwMainThreadID = GetCurrentThreadId();

		RegisterClassObject<CNotifyInterfaceImp_Factory>(CLSID_CNotifyInterfaceImp, &dwCookie_CNotifyInterfaceImp);

		::CoResumeClassObjects();

		while( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{ 
			if (bRet == -1)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} 

		::CoRevokeClassObject(dwCookie_CNotifyInterfaceImp);
	}

	::CoUninitialize();

	return 0;
}


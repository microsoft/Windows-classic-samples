// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#pragma once

#undef __REQUIRED_RPCNDR_H_VERSION__
#include <bits3_0.h>
#include <stdio.h>
#include <tchar.h>
#include <lm.h>
#include <iostream>

using namespace std;

BOOL GetYesNoAnswer(BOOL *b)
{
    WCHAR ans[4];
    wprintf(L"Please enter yes or no:\n");
    wcin >> ans ;

    if (0 == _wcsicmp(ans,L"YES"))
    {
        *b = true;
        return true;
    }
    if (0 == _wcsicmp(ans, L"NO"))
    {
        *b = false;
        return true;
    }

    return false;
}

class CNotifyInterface : public IBackgroundCopyCallback2
{
  LONG m_lRefCount;

public:
  //Constructor, Destructor
  CNotifyInterface() {m_lRefCount = 1;};
  ~CNotifyInterface() {};

  //IUnknown
  HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj);
  ULONG __stdcall AddRef();
  ULONG __stdcall Release();

  //IBackgroundCopyCallback2 methods
  HRESULT __stdcall JobTransferred(IBackgroundCopyJob* pJob);
  HRESULT __stdcall JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError);
  HRESULT __stdcall JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved);
  HRESULT __stdcall FileTransferred(IBackgroundCopyJob* pJob, IBackgroundCopyFile * pFile );
};

HRESULT CNotifyInterface::QueryInterface(REFIID riid, LPVOID* ppvObj) 
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

ULONG CNotifyInterface::AddRef() 
{
  return InterlockedIncrement(&m_lRefCount);
}

ULONG CNotifyInterface::Release() 
{
  ULONG  ulCount = InterlockedDecrement(&m_lRefCount);

  if(0 == ulCount) 
  {
    delete this;
  }

  return ulCount;
}

HRESULT CNotifyInterface::JobTransferred(IBackgroundCopyJob* pJob)
{
    HRESULT hr;
    
    wprintf(L"Job transferred. Completing Job...\n");
    hr = pJob->Complete();
    if (FAILED(hr))
    {
        //BITS probably was unable to rename one or more of the 
        //temporary files. See the Remarks section of the IBackgroundCopyJob::Complete 
        //method for more details.
        wprintf(L"Job Completion Failed with error %x\n", hr);
    }

    PostMessage(NULL,WM_QUIT,NULL,NULL);

    //If you do not return S_OK, BITS continues to call this callback.
    return S_OK;
}

HRESULT CNotifyInterface::JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved)
{
    return S_OK;
}

HRESULT CNotifyInterface::JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError)
{
    HRESULT hr;
    WCHAR* pszJobName = NULL;
    WCHAR* pszErrorDescription = NULL;

    //Use pJob and pError to retrieve information of interest. For example,
    //if the job is an upload reply, call the IBackgroundCopyError::GetError method 
    //to determine the context in which the job failed. If the context is 
    wprintf(L"Job entered error state...\n");
    hr = pJob->GetDisplayName(&pszJobName);
    if (FAILED(hr))
    {
        wprintf(L"Unable to get job name\n");
    }
    hr = pError->GetErrorDescription(GetThreadLocale(), &pszErrorDescription);
    if (FAILED(hr))
    {
        wprintf(L"Unable to get error description\n");
    }
    if (pszJobName && pszErrorDescription)
    {
        wprintf(L"Job %s ",pszJobName);
        wprintf(L"encountered the following error:\n");
        wprintf(L"%s\n",pszErrorDescription);
    }

    CoTaskMemFree(pszJobName);
    CoTaskMemFree(pszErrorDescription);

    PostMessage(NULL,WM_QUIT,NULL,NULL);

    //If you do not return S_OK, BITS continues to call this callback.
    return S_OK;
}

HRESULT CNotifyInterface::FileTransferred(IBackgroundCopyJob* pJob,IBackgroundCopyFile * pFile)
{
    IBackgroundCopyFile3 *pFile3;   
    LPWSTR pwszTempName;
    HRESULT hr;
    BOOL b;
    
    // Validate the downloaded File. Like checking the hash of the file. Or any other validation.
    printf("File transferred\n");
    hr = pFile->QueryInterface(__uuidof(IBackgroundCopyFile3), (void **)&pFile3);
    if (FAILED(hr))
    {
        wprintf(L"Unable to get IBackgroundCopyFile3 interface\n");
        return S_OK;
    }
    pFile3->GetTemporaryName(&pwszTempName);
    
    wprintf(L"Temporary location of the downloaded file: ");
    wprintf(L"%s",pwszTempName);
    wprintf(L"Is this a valid file?");
    while (1)
    {
       if(GetYesNoAnswer(&b))
       {
        pFile3->SetValidationState(b);
        break;
       }
    }

    CoTaskMemFree(pwszTempName);

    //If you do not return S_OK, BITS continues to call this callback.
    return S_OK;
}
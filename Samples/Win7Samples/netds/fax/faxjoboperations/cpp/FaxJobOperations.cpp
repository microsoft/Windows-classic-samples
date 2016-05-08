//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#include "FaxJobOperations.h"
#include <faxcomex_i.c>

//+---------------------------------------------------------------------------
//
//  function:   GiveUsage
//
//  Synopsis:   prints the usage of the application
//
//  Arguments:  [AppName] - Name of the application whose usage has to be printed
//
//  Returns:    void
//
//----------------------------------------------------------------------------
void GiveUsage(LPTSTR AppName)
{
        _tprintf( TEXT("Usage : %s \n \
                                /s Fax Server Name \n "),AppName);
        _tprintf( TEXT("Usage : %s /? -- this message\n"),AppName);
}

//+---------------------------------------------------------------------------
//
//  function:   IsOSVersionCompatible
//
//  Synopsis:   finds whether the target OS supports this functionality.
//
//  Arguments:  [dwVersion] - Minimum Version of the OS required for the Sample to run.
//
//  Returns:    bool - true if the Sample can run on this OS
//
//----------------------------------------------------------------------------

bool IsOSVersionCompatible(DWORD dwVersion)
{
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)    ;
        if( !bOsVersionInfoEx  )
        {
                osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
                if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
                        return false;
        }
        bOsVersionInfoEx = (osvi.dwMajorVersion >= dwVersion );
        return (bOsVersionInfoEx == TRUE);
}

//+---------------------------------------------------------------------------
//
//  function:   DisplayFaxJob
//
//  Synopsis:   Prints the Job Id
//
//  Arguments:  [pFaxJob] - Fax Job object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------
template<class JobClass>
bool DisplayFaxJob(JobClass* pFaxJob)
{
        bool retVal = false;
        HRESULT hr = S_OK;
        BSTR bstrJobId = NULL;
        BSTR bstrSubId = NULL;
        BSTR bstrSubj = NULL;
        IFaxSender* pFaxSender;
        BSTR bstrSenderName = NULL;

        //check for NULL
        if(pFaxJob == NULL) 
        { 
                _tprintf(_T("DisplayFaxJob: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxJob->get_Id(&bstrJobId);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Id failed. Error %x \n"), hr);
                goto Exit;
        }
        hr = pFaxJob->get_SubmissionId(&bstrSubId);
        if(FAILED(hr))
        {
                _tprintf(_T("get_SubmissionId failed. Error %x \n"), hr);
                goto Exit;
        }
        hr = pFaxJob->get_Subject(&bstrSubj);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Subject failed. Error %x \n"), hr);
                goto Exit;
        }
        hr = pFaxJob->get_Sender(&pFaxSender);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Sender failed. Error %x \n"), hr);
                goto Exit;
        }
        hr = pFaxSender->get_Name(&bstrSenderName);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Name failed. Error %x \n"), hr);
                goto Exit;
        }
        _tprintf(_T("Job Id: %s \t"), bstrJobId);
        _tprintf(_T("Submission Id: %s \t"), bstrSubId);
        _tprintf(_T("Subject: %s \t"), bstrSubj);
        _tprintf(_T("SenderName: %s \t"), bstrSenderName);
        _tprintf(_T("\n"));

        retVal = true;
Exit:
        if(bstrJobId)
                SysFreeString(bstrJobId);
        if(bstrSubId)
                SysFreeString(bstrSubId);
        if(bstrSubj)
                SysFreeString(bstrSubj);
        if(bstrSenderName)
                SysFreeString(bstrSenderName);

        return retVal;
}
//+---------------------------------------------------------------------------
//
//  function:   DisplayFaxJob
//
//  Synopsis:   Prints the Job Id
//
//  Arguments:  [pFaxJob] - Fax Job object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------
template<class JobClass>
bool CompareJob(JobClass* pFaxJob, LPTSTR lptstrJobId)
{
        bool retVal = false;
        HRESULT hr = S_OK;
        BSTR bstrJobId = NULL;
        BSTR bstrSubId = NULL;
        BSTR bstrSubj = NULL;
        BSTR bstrSenderName = NULL;

        //check for NULL
        if(pFaxJob == NULL || lptstrJobId == NULL) 
        { 
                _tprintf(_T("DisplayFaxJob: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxJob->get_Id(&bstrJobId);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Id failed. Error %x \n"), hr);
                goto Exit;
        }

        if(_tcscmp(CharLower(lptstrJobId), CharLower(bstrJobId)) == 0)
        {
                _tprintf(_T("Found matching job \n"));
                retVal = true;
        }
Exit:
        if(bstrJobId)
            SysFreeString(bstrJobId);

        return retVal;
}
//+---------------------------------------------------------------------------
//
//  function:   EnumerateFaxJobs
//
//  Synopsis:   Enumerates the Fax Jobs in the Incoming or Outbox folders
//
//  Arguments:  [pIterator] - Fax Job Iterator Object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

template<class IteratorClass, class JobClass>
bool EnumerateFaxJobs(IteratorClass* pIterator, const IID* piid, LPTSTR lptstrJobId, bool bCancelJob)
{
        HRESULT hr = S_OK;
        IUnknown* pIUnknown =NULL;
        IEnumVARIANT* pIEnumVariant = NULL;
        DWORD numberOfJobs = 10;
        DWORD numberOfJobsReturned = 0;
        VARIANT* variantJobsArray = NULL;
        bool bRetVal = false;
        //check for NULL
        if(pIterator == NULL) 
        { 
                _tprintf(_T("EnumerateFaxJobs: Parameter passed is NULL"));
                goto Exit;
        }

        //check for NULL
        if((bCancelJob == true ) && (lptstrJobId == NULL)) 
        { 
                _tprintf(_T("EnumerateFaxJobs: CancelJob: Parameter passed is NULL"));
                goto Exit;
        }

        hr = pIterator->get__NewEnum(&pIUnknown);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get IUnknown. Error %x \n"), hr);
                goto Exit;
        }
        hr = pIUnknown->QueryInterface(&pIEnumVariant);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get IEnumVariant. Error %x \n"), hr);
                goto Exit;
        }

        //allocate the jobs array
        variantJobsArray = new VARIANT[numberOfJobs];
        if(NULL == variantJobsArray)
        {
                _tprintf(_T("EnumerateFaxJobs: new failed \n"));
                goto Exit;
        }
        INIT_VARIANT_ARRAY(variantJobsArray, numberOfJobs);
        if(bCancelJob == false)
                _tprintf(__T("Enumerating Jobs... \n"));
        while(1)
        {
                hr = pIEnumVariant->Next(numberOfJobs,
                                variantJobsArray,
                                &numberOfJobsReturned);
                if(FAILED(hr))
                {
                        _tprintf(_T("EnumerateFaxJobs: Next failed. Error %x \n"), hr);
                        goto Exit;
                }

                for(DWORD i =0; i< numberOfJobsReturned; i++)
                {
                        JobClass* pFaxJob = NULL;
                        if(VT_DISPATCH != variantJobsArray[i].vt)
                        {
                                _tprintf(_T("Unexpected variant. Expected=%d, Actual=%d \n"),
                                                VT_DISPATCH, variantJobsArray[i].vt);
                                goto Exit;
                        }
                        hr = variantJobsArray[i].pdispVal->QueryInterface(*piid,
                                        (void**)&pFaxJob);
                        if(FAILED(hr))
                        {
                                _tprintf(__T("QueryInterface for Job interface failed. Error %x \n"), hr);
                                goto Exit;
                        }

                        if(bCancelJob == true)
                        {
                                bool bResult = CompareJob(pFaxJob, lptstrJobId);
                                if(bResult)
                                {
                                        hr = pFaxJob->Cancel();
                                        if(FAILED(hr))
                                        {     
                                                _tprintf(_T("Cancel Job failed. Error %x \n"), hr);
                                                goto Exit;
                                        }
                                        _tprintf(_T("Job cancelled successfuly \n\n"));
                                        bRetVal = true;
                                        goto Exit;
                                }

                        }
                        else
                                DisplayFaxJob(pFaxJob);                          
                }        
                CLEAR_VARIANT_ARRAY(variantJobsArray, numberOfJobs);
                if(S_FALSE == hr)
                {
                        //no more fax jobs                        
                        break;
                }
        }
        if(bCancelJob == false)
                bRetVal =  true;
Exit:        
        if(NULL != variantJobsArray)
        {
                delete[] variantJobsArray;
        }
        return bRetVal;
}



//+---------------------------------------------------------------------------
//
//  function:   EnumOutbox
//
//  Synopsis:   Displays the jobs present in the Outbox Folder
//
//  Arguments:  [pFaxFolders] - Fax Folders object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

bool EnumOutbox(IFaxOutgoingQueue* pFaxOutgoingQueue)
{   
        //check for NULL
        if(pFaxOutgoingQueue == NULL) 
        { 
                _tprintf(_T("EnumOutbox: Parameter passed is NULL"));
                return false;
        }        

        IFaxOutgoingJobs* pFaxOutgoingJobs;
        HRESULT hr = pFaxOutgoingQueue->GetJobs(&pFaxOutgoingJobs);
        if(FAILED(hr))
        {
                _tprintf(_T("GetJobs failed. Error %x \n"), hr);
                return false;
        }
        if(!EnumerateFaxJobs <IFaxOutgoingJobs, IFaxOutgoingJob>(pFaxOutgoingJobs, &IID_IFaxOutgoingJob, NULL, false))
        {
                _tprintf(_T("Failed to enumerate \n"));
                return false;
        }
        return true;
}

//+---------------------------------------------------------------------------
//
//  function:   EnumOutbox
//
//  Synopsis:   Displays the jobs present in the Outbox Folder
//
//  Arguments:  [pFaxFolders] - Fax Folders object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

bool CancelJob(LPTSTR lptstrJobId, IFaxOutgoingQueue* pFaxOutgoingQueue)
{   
        //check for NULL
        if(pFaxOutgoingQueue == NULL) 
        { 
                _tprintf(_T("EnumOutbox: Parameter passed is NULL"));
                return false;
        }        
        IFaxOutgoingJobs* pFaxOutgoingJobs;
        HRESULT hr = pFaxOutgoingQueue->GetJobs(&pFaxOutgoingJobs);
        if(FAILED(hr))
        {
                _tprintf(_T("GetJobs failed. Error %x \n"), hr);
                return false;
        }
        if(!EnumerateFaxJobs<IFaxOutgoingJobs, IFaxOutgoingJob>(pFaxOutgoingJobs, &IID_IFaxOutgoingJob, lptstrJobId, true))
        {
                _tprintf(_T("Failed to enumerate \n"));
                return false;
        }
        return true;
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        bool bConnected = false;
        size_t argSize = 0;
        LPTSTR lptstrServerName = NULL;        
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("This sample is compatible with Windows Vista"));
                bRetVal = false;
                goto Exit1;
        }

        //introducing an artifical scope here so that the COM objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer = NULL;
                IFaxOutgoingQueue* pFaxOutgoingQueue;            
                IFaxFolders* pFaxFolders;

                int argcount = 0;
                int count = 0;

#ifdef UNICODE
                argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
                argv = argvA;
#endif      

                // check for commandline switches
                for (argcount=1; argcount<argc; argcount++)
                {
                        if(argcount + 1 < argc)
                        {
                                hr = StringCbLength(argv[argcount + 1],1024 * sizeof(TCHAR),&argSize);
                                if(!FAILED(hr))
                                {
                                        if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-'))
                                        {                
                                                switch (towlower(argv[argcount][1]))
                                                {
                                                        case 's':
                                                                if(lptstrServerName == NULL)
                                                                {
                                                                        //servername parameter
                                                                        lptstrServerName = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrServerName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrServerName, 0, (argSize+1) * sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrServerName,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;

                                                        case '?':
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit;                
                                                        default:
                                                                break;
                                                }//switch
                                        }//if
                                }//if failed
                        }//(argcount + 1 < argc)
                }//for
                //initialize COM
                hr = CoInitialize(NULL);
                if(FAILED(hr))
                {
                        //failed to init com
                        _tprintf(_T("Failed to init com. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = CoCreateInstance (CLSID_FaxServer, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxServer), 
                            (void **)&pFaxServer);
                if(FAILED(hr))
                {
                        //CoCreateInstance failed.
                        _tprintf(_T("CoCreateInstance failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                //connect to fax server.
                BSTR bstrServerName = SysAllocString(lptstrServerName);
                if(lptstrServerName != NULL && bstrServerName == NULL)
                {
                        _tprintf(_T("SysAllocString failed. \n"));
                        bRetVal = false;
                        goto Exit;
                }
                hr = pFaxServer->Connect(bstrServerName);
                if(FAILED(hr))
                {
                        _tprintf(_T("Connect failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                bConnected = true;

                FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
                hr = pFaxServer->get_APIVersion(&enumFaxAPIVersion);
                if(FAILED(hr))
                {
                        //get_APIVersion failed.
                        _tprintf(_T("get_APIVersion failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if (enumFaxAPIVersion < fsAPI_VERSION_3) 
                {
                        bRetVal = false;
                        _tprintf(_T("This sample is compatible with Windows Vista"));
                        goto Exit;
                }         

                //Now lets get the folders object
                hr = pFaxServer->get_Folders(&pFaxFolders);
                if(FAILED(hr))
                {
                        _tprintf(_T("Getting Folders failed Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //get outgoingqueue
                hr = pFaxFolders->get_OutgoingQueue(&pFaxOutgoingQueue);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_OutgoingQueue failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                bool bQuit = false;
                TCHAR cOption ; 
                TCHAR strJobId[1024]  = {0};
                int iResult = -1;
                while(bQuit == false)
                {
                        VARIANT_BOOL bQueue = VARIANT_TRUE;
                        //Block queue
                        hr = pFaxOutgoingQueue->put_Paused(bQueue);
                        if(FAILED(hr))
                        {
                                _tprintf(_T("put_Paused failed. Error %x \n"), hr);                    
                                bRetVal = false;
                                goto Exit;
                        }                    

                        hr = pFaxOutgoingQueue->put_Blocked(bQueue);
                        if(FAILED(hr))
                        {
                                _tprintf(_T("put_Blocked failed. Error %x \n"), hr);                    
                                bRetVal = false;
                                goto Exit;
                        }        
                        hr = pFaxOutgoingQueue->Save();
                        if(FAILED(hr))
                        {
                                _tprintf(_T("Save failed. Error %x \n"), hr);                    
                                bRetVal = false;
                                goto Exit;
                        }        
                        _tprintf(_T("\nOutgoing Queue is paused. \n"));        
                        _tprintf(_T("Outgoing Queue is blocked. \n"));    
                        //Print all outgoing jobs
                        _tprintf(_T("Printing list of Outgoing jobs ....\n"));

                        if(!EnumOutbox(pFaxOutgoingQueue))
                        {
                                _tprintf(_T("Failed to enumerate \n"));
                                bRetVal =  false;
                        }

                        _tprintf(_T("Enter 'c' to cancel a job \n"));    
                        _tprintf(_T("Enter 'q' to quit \n"));    
                        fflush(stdin);
                        iResult = _tscanf_s(_T("%c"), &cOption);
                        if(iResult == 0 || iResult == EOF)
                        {
                                _tprintf(_T("_tscanf failed. Return Value %d\n"), iResult);                    
                                bRetVal = false;
                                goto Exit;
                        }

input:
                        switch(cOption)
                        {
                                case 'c':
                                        _tprintf(_T("\nEnter 'i' to enter Job id \n"));    
                                        _tprintf(_T("Enter 'q' to quit \n"));    
                                        fflush(stdin);
                                        iResult = _tscanf_s(_T("%c"), &cOption);
                                        if(iResult == 0 || iResult == EOF)
                                        {
                                                _tprintf(_T("_tscanf failed. Return Value %d\n"), iResult);                    
                                                bRetVal = false;
                                                goto Exit;
                                        }

input2:      
                                        switch(cOption)
                                        {
                                                case 'i':
                                                        _tprintf(_T("Enter Job id \n"));
                                                        fflush(stdin);
                                                        iResult = _tscanf_s(_T("%s"), strJobId);
                                                        if(iResult == 0 || iResult == EOF)
                                                        {
                                                                _tprintf(_T("_tscanf failed. Return Value %d\n"), iResult);                    
                                                                bRetVal = false;
                                                                goto Exit;
                                                        }
                                                        CancelJob(strJobId, pFaxOutgoingQueue);
                                                        break;
                                                case 'q' :
                                                        goto quit;
                                                        break;
                                                default:
                                                        _tprintf(_T("Invalid Option. Enter 'i' to enter Job id or 'q' to quit \n"));
                                                        fflush(stdin);
                                                        iResult = _tscanf_s(_T("%c"), &cOption);
                                                        if(iResult == 0 || iResult == EOF)
                                                        {
                                                                _tprintf(_T("_tscanf failed. Return Value %d\n"), iResult);                    
                                                                bRetVal = false;
                                                                goto Exit;
                                                        }
                                                        goto input2;
                                        }
                                        break;
                                case 'q' :
quit:                                   bQuit = true;
                                        break;
                                default:
                                        _tprintf(_T("Invalid Option. Enter again \n"));    
                                        fflush(stdin);
                                        iResult = _tscanf_s(_T("%c"), &cOption);
                                        if(iResult == 0 || iResult == EOF)
                                        {
                                                _tprintf(_T("_tscanf failed. Return Value %d\n"), iResult);                    
                                                bRetVal = false;
                                                goto Exit;
                                        }
                                        goto input;
                        }

                }
                //unblock queue
                VARIANT_BOOL bQueue = VARIANT_FALSE;
                hr = pFaxOutgoingQueue->put_Paused(bQueue);
                if(FAILED(hr))
                {
                        _tprintf(_T("put_Paused failed. Error %x \n"), hr);                    
                        bRetVal = false;
                        goto Exit;
                }

                hr = pFaxOutgoingQueue->put_Blocked(bQueue);
                if(FAILED(hr))
                {
                        _tprintf(_T("put_Blocked failed. Error %x \n"), hr);                    
                        bRetVal = false;
                        goto Exit;
                }
                hr = pFaxOutgoingQueue->Save();
                if(FAILED(hr))
                {
                        _tprintf(_T("Save failed. Error %x \n"), hr);                    
                        bRetVal = false;
                        goto Exit;
                }    
                _tprintf(_T("Outgoing Queue is resumed. \n"));        
                _tprintf(_T("Outgoing Queue is unblocked. \n\n"));    

Exit:
                if(bConnected)
                {
                        pFaxServer->Disconnect();
                }
                if(lptstrServerName)
                        free(lptstrServerName);
                if(bstrServerName)
                        SysFreeString(bstrServerName);

        }
        CoUninitialize();
Exit1:
        return bRetVal;

}

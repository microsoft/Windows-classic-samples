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

#include "SendFax.h"
#include <faxcomex_i.c>

VARIANT g_vVariant;

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
            /s Fax Server Name \n \
            /d DocumentPath (can have multiple documents separated by semicolons. test1.txt;test2.doc \n \
            /n Fax Number \n"),AppName);
        _tprintf( TEXT("Usage : %s /? -- help message\n"),AppName);
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
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
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
//  function:   PrintJobStatus
//
//  Synopsis:   prints the jobs status
//
//  Arguments:  [pFaxOutgoingJob] - FaxOutgoingJob object pointing to the fax that was sent. 
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool PrintJobStatus(IFaxOutgoingJob* pFaxOutgoingJob)
{
        HRESULT hr = S_OK;
        bool bRetVal = false;
        long lDeviceId = 0;
        FAX_JOB_STATUS_ENUM faxStatus;
        FAX_PRIORITY_TYPE_ENUM faxPriority;

        if (pFaxOutgoingJob == NULL) 
        {
                _tprintf(_T("PrintJobStatus: Parameter passed is NULL"));
                goto Exit;
        }

        hr = pFaxOutgoingJob->get_DeviceId(&lDeviceId);
        if(FAILED(hr))
        {
                _tprintf(_T("get_DeviceId failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;
        }

        _tprintf(_T("Device Id: %d \n"),lDeviceId );
        hr = pFaxOutgoingJob->get_Status(&faxStatus);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Status failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;
        }

        if(faxStatus == fjsPENDING)
                _tprintf(TEXT("Status : Pending \n") );
        if(faxStatus == fjsINPROGRESS)
                _tprintf(TEXT("Status : In Progress \n") );
        if(faxStatus == fjsFAILED)
                _tprintf(TEXT("Status : Failed\n") );
        if(faxStatus == fjsPAUSED)
                _tprintf(TEXT("Status : Paused \n") );
        if(faxStatus == fjsNOLINE)
                _tprintf(TEXT("Status : No Line \n") );
        if(faxStatus == fjsRETRYING)
                _tprintf(TEXT("Status : Retrying \n") );
        if(faxStatus == fjsRETRIES_EXCEEDED)
                _tprintf(TEXT("Status : Retries Exceeded \n") );
        if(faxStatus == fjsCOMPLETED)
                _tprintf(TEXT("Status : Completed \n") );
        if(faxStatus == fjsCANCELED)
                _tprintf(TEXT("Status : Canceled \n") );
        if(faxStatus == fjsCANCELING)
                _tprintf(TEXT("Status : Canceling \n") );
        if(faxStatus == fjsROUTING)
                _tprintf(TEXT("Status : Routing \n") );


        hr = pFaxOutgoingJob->get_Priority(&faxPriority);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Priority failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;
        }
        if(faxPriority == fptLOW)
                _tprintf(TEXT("Priority : Low\n") );
        if(faxPriority == fptNORMAL)
                _tprintf(TEXT("Priority : Normal\n") );
        if(faxPriority == fptHIGH)
                _tprintf(TEXT("Priority : High\n") );

        bRetVal = true;
Exit:
        return bRetVal;

}
//+---------------------------------------------------------------------------
//
//  function:   DecodeToDocArray
//
//  Synopsis:   Creates a VARIANT Array of Docs from the inputDocListString
//
//  Arguments:  [inputDocListString] - The list of documents in string format separated by semicolon
//              [numDocuments] -    The number of documents to be sent
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------

bool DecodeToDocArray(const LPTSTR inputDocListString, int* numDocuments)
{
        SAFEARRAY * pSafeArray;
        SAFEARRAYBOUND aDim[1];    
        LPTSTR* docArray = NULL;
        BSTR* pbstrArray = NULL;
        bool bRetVal = false;
	LPTSTR docListString = NULL;
        if(NULL == inputDocListString)
        {
                return false;
        }

        HRESULT hr = S_OK;      
        int numDocs = 0;
        int count =0;

        size_t iStrLen = 0;
        hr = StringCchLength(inputDocListString,  2056 , &iStrLen);

        if(FAILED(hr))
        {
                _tprintf(_T("StringCchLength of inputDocListString failed.  Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        docListString = (TCHAR*) malloc(sizeof(TCHAR) * (iStrLen+1));
        if(docListString == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc of inputDocListString failed. Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        memset(docListString, 0, iStrLen+1);
        hr = StringCchCopy(docListString, (iStrLen+1),inputDocListString );
        if(FAILED(hr))
        {
                _tprintf(_T("StringCchLength of inputDocListString failed. Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        //Get the number of Docs first.
        size_t i = 0;
        while(i < iStrLen)
        {
                if(docListString[i] == ';')
                {
                        numDocs= numDocs + 1;
                }
                i++;
        }
        numDocs = numDocs+1;
        docArray = new LPTSTR [numDocs];        
        if(docArray == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("New of docArray failed. Error 0x%x"),hr);
                bRetVal = false;
                goto Exit;
        } 
        memset(docArray, 0, numDocs);

        i = 0;
        size_t end = 0;
        size_t start = 0;
        while(i <= iStrLen)
        {
                if(docListString[i] == ';' || i == iStrLen)
                {
                        end = i;
                        size_t len = end - start ;
                        TCHAR* strTemp = &docListString[start];
                        docArray[count] = (TCHAR*)malloc((len+1) * sizeof(TCHAR));
                        if(docArray[count] == NULL)
                        {
                                hr = E_OUTOFMEMORY;
                                _tprintf(_T("malloc of docArray[count] failed. Error 0x%x"), hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        memset(docArray[count], 0, len+1);
                        hr = StringCchCopyN(docArray[count], len+1, strTemp, len);
                        if(FAILED(hr))
                        {
                                _tprintf(_T("StringCchCopyN of docArray on count %i failed. Error 0x%x "), count, hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        start = i+1;
                        count++;
                }            
                i++;
        }

        *numDocuments = numDocs;

        VariantInit(&g_vVariant);        
        aDim[0].lLbound = 0;
        aDim[0].cElements = numDocs;    

        g_vVariant.vt = VT_ARRAY | VT_BSTR;
        pSafeArray = SafeArrayCreate(VT_BSTR, 1, aDim);   
        SafeArrayAccessData(pSafeArray, (void**)&pbstrArray);   
        for(int nCount = 0; nCount < numDocs ; nCount++)
        {
                pbstrArray[nCount] = SysAllocString(docArray[nCount]);
                if(pbstrArray[nCount] == NULL)
                {
                    _tprintf(_T("SysAllocString for count %d failed."), nCount);
                    bRetVal = false;
                    goto Exit;
                }
        }
        SafeArrayUnaccessData(pSafeArray);
        g_vVariant.parray = pSafeArray;    
        bRetVal = true;

Exit:       
        for(int j=0; j < numDocs; j++)
        {
            if(docArray != NULL)
            {
                if( docArray[j] != NULL)
                        free( docArray[j]);
            }
        }

        if(docArray != NULL)
                delete[] docArray; 
 
        if(docListString)
                free(docListString);
        return bRetVal;

}

int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrNumber = NULL;
        LPTSTR lptstrDocListString = NULL;
        BSTR bstrServerName = NULL;
        bool bConnected = false;
        size_t argSize = 0;        
        long lErrorBodyFile = -1;
        int numDocs=0;
        LPTSTR* docListArray = NULL;
        bool bResult = false;
        SAFEARRAY * pSafeArray =NULL;
        long lIndex = 0;
        long lUBound;
        BSTR bstrJobID = NULL;

        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("OS Version does not support this feature"));
                bRetVal = false;
                goto Exit1;
        }

        //introducing an artifical scope here so that the COm objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer = NULL;                
                IFaxDocument2* pFaxDoc;
                IFaxRecipients* pFaxToRecipients;
                IFaxSender* pFaxSender;
                IFaxAccount* pFaxAccount;
                IFaxRecipient* pFaxRecipient;
                IFaxAccountFolders* pFaxFolders;
                IFaxAccountOutgoingQueue* pFaxOutgoingQueue;
                IFaxOutgoingJob* pFaxOutgoingJob;


                int argcount = 0;

#ifdef UNICODE
                argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
                argv = argvA;
#endif
                if (argc == 1)
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

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
                                                                        lptstrServerName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrServerName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrServerName, 0, (argSize+1)* sizeof(TCHAR));
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
                                                        case 'd':
                                                                if(lptstrDocListString == NULL)
                                                                {
                                                                        lptstrDocListString = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));                        
                                                                        if(lptstrDocListString == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrDocListString: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrDocListString, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrDocListString,argSize +1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrDocListString: StringCchCopyN failed. Error %x \n"), hr);
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
                                                        case 'n':
                                                                if(lptstrNumber ==NULL)
                                                                {
                                                                        lptstrNumber = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrNumber == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrNumber: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrNumber, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrNumber, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrNumber: StringCchCopyN failed. Error %x \n"), hr);
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
                                }
                        }
                }//for

                if (!lptstrDocListString || !lptstrNumber)
                {
                        _tprintf( TEXT("Missing Args. \n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

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
                        _tprintf(_T("CoCreateInstance for FaxServer failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }             
               
                //connect to fax server.
                bstrServerName = SysAllocString(lptstrServerName);
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
                        _tprintf(_T("get_APIVersion failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if (enumFaxAPIVersion < fsAPI_VERSION_3) 
                {
                        bRetVal = false;
                        _tprintf(_T("OS Version does not support this feature"));
                        goto Exit;
                }     

                bResult = DecodeToDocArray(lptstrDocListString, &numDocs);
                if(bResult == false)
                {
                        _tprintf(_T("DecodeToDocArray failed. Error \n"));
                        bRetVal = false;
                        goto Exit;
                }

                //Create the fax document object.
                hr = CoCreateInstance (CLSID_FaxDocument, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxDocument), 
                            (void **)&pFaxDoc);                
                if(FAILED(hr))
                {
                        _tprintf(_T("CoCreateInstance for faxdocument failed. Error 0x%x \n"), hr);
                        return false;
                }

                //Set the Bodies
                hr = pFaxDoc->put_Bodies(g_vVariant);
                if(FAILED(hr))
                {
                        _tprintf(_T("put_Bodies failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //put to recipients    
                hr = pFaxDoc->get_Recipients(&pFaxToRecipients);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_Recipients failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = pFaxToRecipients->Add(BSTR(lptstrNumber),
                                BSTR(RECIPIENT_NAME),
                                &pFaxRecipient);
                if(FAILED(hr))
                {
                        _tprintf(_T("pFaxToRecipients->Add failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //load the default sender info before calling connected submit.
                hr = pFaxDoc->get_Sender(&pFaxSender);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_Sender failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = pFaxSender->LoadDefaultSender();
                if(FAILED(hr))
                {
                        _tprintf(_T("LoadDefaultSender failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                VARIANT jobIds;
                hr = pFaxDoc->ConnectedSubmit2(pFaxServer, &jobIds, &lErrorBodyFile);
                if (FAILED(hr) || (-1 != lErrorBodyFile))
                {
                        _tprintf(_T("ConnectedSubmit2 failed. Error %x \n"), hr);
                        _tprintf(_T("Error Index File %d \n"), lErrorBodyFile);
                        bRetVal = false;
                        goto Exit;
                }

                //Get the Outgoing job object
                // Get the upper bound of the array or job IDs.
                SafeArrayGetUBound(jobIds.parray, 1, &lUBound);
                for (lIndex = 0; lIndex <= lUBound; lIndex++)
                {                        
                        SafeArrayGetElement(jobIds.parray , &lIndex, LPVOID(&bstrJobID));
                        _tprintf (_T("Job ID : %s\n"), bstrJobID);
                }

                hr = pFaxServer->get_CurrentAccount(&pFaxAccount);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetCurrentAccount failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }    

                hr = pFaxAccount->get_Folders(&pFaxFolders);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_Folders failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                hr = pFaxFolders->get_OutgoingQueue(&pFaxOutgoingQueue);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_OutgoingQueue failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = pFaxOutgoingQueue->GetJob(bstrJobID, &pFaxOutgoingJob);
                if(FAILED(hr))
                {
                        _tprintf(_T("getJob failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                // Print the job status
                if(PrintJobStatus(pFaxOutgoingJob) == false)
                {
                        _tprintf(_T("PrintJobStatus failed. \n"));
                        bRetVal = false;
                        goto Exit;
                }

Exit:              
                if(bConnected)
                {
                        pFaxServer->Disconnect();
                }
                if(lptstrServerName)
                        free(lptstrServerName);
                if(lptstrDocListString)
                        free(lptstrDocListString);
                if(lptstrNumber)
                        free(lptstrNumber);
                if(bstrServerName)
                        SysFreeString(bstrServerName);
                if(bstrJobID)
                        SysFreeString(bstrJobID);
                pSafeArray = g_vVariant.parray;
                SafeArrayDestroy(pSafeArray);
        }        
        CoUninitialize();
Exit1:
        return bRetVal;
}


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

#include "FoldersEnum.h"
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
            /s Fax Server Name \n \
            /o <EnumInbox>/<EnumOutbox>/<EnumSentItems>/<EnumIncoming> \n "),AppName);
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
        BSTR bstrOutput = NULL;
        //check for NULL
        if(pFaxJob == NULL) 
        { 
                _tprintf(_T("DisplayFaxJob: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxJob->get_Id(&bstrOutput);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Id failed. Error %x \n"), hr);
                goto Exit;
        }
        _tprintf(_T("Job Id: %s \n"), bstrOutput);
        if(bstrOutput)
                SysFreeString(bstrOutput);
        retVal = true;
Exit:
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
bool EnumerateFaxJobs(IteratorClass* pIterator, const IID* piid)
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
                        JobClass* pFaxJob;
                        if(VT_DISPATCH != variantJobsArray[i].vt)
                        {
                                _tprintf(_T("Unexpected variant. Expected=%d, Actual=%d \n"),
                                                VT_DISPATCH, variantJobsArray[i].vt);
                                goto Exit;
                        }
                        hr = variantJobsArray[i].pdispVal->QueryInterface(*piid,
                                        (void**)&pFaxJob);
                        DisplayFaxJob(pFaxJob);

                        if(FAILED(hr))
                        {
                                _tprintf(__T("QueryInterface for Job interface failed. Error %x \n"), hr);
                                goto Exit;
                        }
                }        
                CLEAR_VARIANT_ARRAY(variantJobsArray, numberOfJobs);
                if(S_FALSE == hr)
                {
                        //no more fax jobs                        
                        break;
                }
        }
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
//  function:   DisplayFaxMessage
//
//  Synopsis:   Prints the Mesage Id
//
//  Arguments:  [pFaxJob] - Fax Message object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

template<class MsgClass>
bool DisplayFaxMessage(MsgClass* pFaxMsg)
{
        bool retVal = false;
        HRESULT hr = S_OK;
        BSTR bstrOutput = NULL;
        if(pFaxMsg == NULL) 
        { 
                _tprintf(_T("DisplayFaxMessage: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxMsg->get_Id(&bstrOutput);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Id failed. Error %x \n"), hr);
                goto Exit;
        }
        _tprintf(_T("Message Id: %s \n"), bstrOutput);
        if(bstrOutput)
                SysFreeString(bstrOutput);
        retVal = true;
Exit:
        return retVal;
}
//+---------------------------------------------------------------------------
//
//  function:   EnumerateFaxMessages
//
//  Synopsis:   Enumerates the Fax Messages in the Inbox or Sent Items folders
//
//  Arguments:  [pIterator] - Fax Message Iterator Object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

template<class IteratorClass, class MsgClass>
bool EnumerateFaxMessages(IteratorClass* pIterator)
{
        HRESULT hRes = S_OK;   
        bool bRetVal = false;
        //check for NULL
        if(pIterator == NULL) 
        { 
                _tprintf(_T("EnumerateFaxJobs: Parameter passed is NULL"));
                goto Exit;
        }
        hRes = pIterator->MoveFirst();
        if (FAILED(hRes))
        {
                _tprintf(_T("EnumerateFaxMessages: MoveFirst failed. Error %x \n"), hRes);
                goto Exit;
        }

        _tprintf(__T("Enumerating Messages... \n"));
        while(1)
        {
                VARIANT_BOOL vAtEOF;      
                hRes = pIterator->get_AtEOF(&vAtEOF);
                if (FAILED(hRes))
                {
                        _tprintf(_T("get_AtEOF failed. Error %x \n"), hRes);
                        goto Exit;
                }
                if (VARIANT_TRUE == vAtEOF)
                {
                        //end of msg list
                        break;
                }  
                MsgClass* pMessage;
                hRes = pIterator->get_Message(&pMessage);
                DisplayFaxMessage(pMessage);

                if (FAILED(hRes))
                {
                        _tprintf(_T("get_Message failed. Error %x \n"), hRes);
                        goto Exit;
                }        
                hRes = pIterator->MoveNext();
                if (FAILED(hRes))
                {
                        _tprintf(_T("MoveNext failed. Error %x \n"), hRes);
                        goto Exit;
                }
        }        
        bRetVal =  true;
Exit:
        return bRetVal;
}
//+---------------------------------------------------------------------------
//
//  function:   EnumInbox
//
//  Synopsis:   Displays the messages present in the inbox
//
//  Arguments:  [pFaxFolders] - Fax Folders object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

bool EnumInbox(IFaxAccountFolders* pFaxFolders)
{
        //check for NULL
        if(pFaxFolders == NULL) 
        { 
                _tprintf(_T("EnumInbox: Parameter passed is NULL"));
                return false;
        }
        IFaxAccountIncomingArchive* pFaxInbox;
        HRESULT hr = pFaxFolders->get_IncomingArchive(&pFaxInbox);
        if(FAILED(hr))
        {
                _tprintf(_T("GetIncomingArchive failed. Error %x \n"), hr);
                return false;
        }
        IFaxIncomingMessageIterator* incomingMsgIterator;

        //Function will retieve 100 messages
        hr = pFaxInbox->GetMessages(100, &incomingMsgIterator);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get Msg Iterator. Error %x \n"), hr);
                return false;
        }

        if(!EnumerateFaxMessages <IFaxIncomingMessageIterator,IFaxIncomingMessage>(incomingMsgIterator))
        {
                _tprintf(_T("Failed to enumerate \n"));
                return false;
        }
        return true;
}

//+---------------------------------------------------------------------------
//
//  function:   EnumSentItems
//
//  Synopsis:   Displays the messages present in the Sent Items
//
//  Arguments:  [pFaxFolders] - Fax Folders object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

bool EnumSentItems(IFaxAccountFolders* pFaxFolders)
{
        //check for NULL
        if(pFaxFolders == NULL) 
        { 
                _tprintf(_T("EnumSentItems: Parameter passed is NULL"));
                return false;
        }
        IFaxAccountOutgoingArchive* pFaxOutbox;
        HRESULT hr = pFaxFolders->get_OutgoingArchive(&pFaxOutbox);
        if(FAILED(hr))
        {
                _tprintf(_T("GetIncomingArchive failed. Error %x \n"), hr);
                return false;
        }
        IFaxOutgoingMessageIterator* outgoingMsgIterator;
        //Function will retieve 100 messages
        hr = pFaxOutbox->GetMessages(100, &outgoingMsgIterator);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get Msg Iterator Error %x \n"), hr);
                return false;
        }
        if(!EnumerateFaxMessages <IFaxOutgoingMessageIterator, IFaxOutgoingMessage>(outgoingMsgIterator))
        {
                _tprintf(_T("Failed to enumerate \n"));
                return false;
        }
        return true;
}

//+---------------------------------------------------------------------------
//
//  function:   EnumIncoming
//
//  Synopsis:   Displays the jobs present in the Incoming Folder
//
//  Arguments:  [pFaxFolders] - Fax Folders object
//
//  Returns:    bool - true if the function was successful
//
//----------------------------------------------------------------------------

bool EnumIncoming(IFaxAccountFolders* pFaxFolders)
{
        //check for NULL
        if(pFaxFolders == NULL) 
        { 
                _tprintf(_T("EnumIncoming: Parameter passed is NULL"));
                return false;
        }
        IFaxAccountIncomingQueue* pFaxIncomingQueue;
        HRESULT hr = pFaxFolders->get_IncomingQueue(&pFaxIncomingQueue);
        if(FAILED(hr))
        {
                _tprintf(_T("get_IncomingQueue failed. Error %x \n"), hr);
                return false;
        }
        IFaxIncomingJobs* pFaxIncomingJobs;
        hr = pFaxIncomingQueue->GetJobs(&pFaxIncomingJobs);
        if(FAILED(hr))
        {
                _tprintf(_T("GetJobs failed. Error %x \n"), hr);
                return false;
        }
        if(!EnumerateFaxJobs<IFaxIncomingJobs, IFaxIncomingJob>(pFaxIncomingJobs, &IID_IFaxIncomingJob))
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

bool EnumOutbox(IFaxAccountFolders* pFaxFolders)
{   
        //check for NULL
        if(pFaxFolders == NULL) 
        { 
                _tprintf(_T("EnumOutbox: Parameter passed is NULL"));
                return false;
        }
        IFaxAccountOutgoingQueue* pFaxOutgoingQueue;
        HRESULT hr = pFaxFolders->get_OutgoingQueue(&pFaxOutgoingQueue);
        if(FAILED(hr))
        {
                _tprintf(_T("get_OutgoingQueue failed. Error %x \n"), hr);
                return false;
        }
        IFaxOutgoingJobs* pFaxOutgoingJobs;
        hr = pFaxOutgoingQueue->GetJobs(&pFaxOutgoingJobs);
        if(FAILED(hr))
        {
                _tprintf(_T("GetJobs failed. Error %x \n"), hr);
                return false;
        }
        if(!EnumerateFaxJobs<IFaxOutgoingJobs, IFaxOutgoingJob>(pFaxOutgoingJobs, &IID_IFaxOutgoingJob))
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
        LPTSTR lptstrServerName = NULL;        
        LPTSTR lptstrOption = NULL;
        bool bConnected = false;
        size_t argSize = 0;
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
                IFaxAccount* pFaxAccount;
                IFaxAccountFolders* pFaxFolders;
                BSTR bstrServerName = NULL;

                int argcount = 0;
                int count = 0;

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
                                                        case 'o':
                                                                if(lptstrOption == NULL)
                                                                {
                                                                        //option parameter (list or reassign)
                                                                        lptstrOption = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));                       
                                                                        if(lptstrOption == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrOption: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrOption, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrOption,argSize+1, argv[argcount+1], argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrOption: StringCchCopyN failed. Error %x \n"), hr);
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

                if (!lptstrOption)
                {
                        _tprintf( TEXT("Missing args.\n") );
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
                        _tprintf(_T("CoCreateInstance failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                //connect to fax server.
                bstrServerName = SysAllocString(lptstrServerName);
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


                hr = pFaxServer->get_CurrentAccount(&pFaxAccount);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetCurrentAccount failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }


                //Now that we have got the account object lets get the folders object
                hr = pFaxAccount->get_Folders(&pFaxFolders);
                if(FAILED(hr))
                {
                        _tprintf(_T("Getting FaxAccountFolders failed Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //if reassign message option is selected
                if(_tcscmp(_T("enumoutbox"), CharLower(lptstrOption))==0)
                {
                        if(EnumOutbox(pFaxFolders)== false)
                        {                                
                                _tprintf(_T("EnumOutbox failed \n"));
                                bRetVal = false;            

                        }            
                }
                if(_tcscmp(_T("enuminbox"), CharLower(lptstrOption)) ==0)
                {
                        if(EnumInbox(pFaxFolders)== false)
                        {                                
                                _tprintf(_T("EnumInbox failed \n"));
                                bRetVal = false;            

                        }            
                }

                if(_tcscmp(_T("enumincoming"), CharLower(lptstrOption)) == 0)
                {
                        if(EnumIncoming(pFaxFolders)== false)
                        {                               
                                _tprintf(_T("EnumIncoming failed \n"));
                                bRetVal = false;            

                        }            
                }

                if(_tcscmp(_T("enumsentitems"), CharLower(lptstrOption))==0)
                {
                        if(EnumSentItems(pFaxFolders)== false)
                        {
                                _tprintf(_T("EnumSentItems failed \n"));
                                bRetVal = false;            

                        }            
                }

Exit:
                if(bConnected)
                {
                        pFaxServer->Disconnect();
                }
                if(lptstrServerName)
                        free(lptstrServerName);
                if(lptstrOption)
                        free(lptstrOption);
                if(bstrServerName)
                        SysFreeString(bstrServerName);

        }
        CoUninitialize();
Exit1:
        return bRetVal;

}

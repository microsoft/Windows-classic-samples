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

#include "FaxReassign.h"
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
                      /o <List/Reassign> Message option \n \
                      /i Message Id. Used if Reassign option \n \
                      /r Recipients in the form \"domain1\\user1;domain1\\user2\" \n"),AppName);
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
//  function:   checkIfReassignableFaxPresent
//
//  Synopsis:   Check if there are faxes that can be reassigned in the current account
//
//  Arguments:  [pIncomingMsgIterator] - Iterator to the messages in inbox folder
//
//  Returns:    int: number of fax that can be reassigned
//
//----------------------------------------------------------------------------
int checkIfReassignableFaxPresent(IFaxIncomingMessageIterator* pIncomingMsgIterator)
{
        int iCount =0;
        HRESULT hr = S_OK;

        if(pIncomingMsgIterator == NULL)
        {
                _tprintf(_T("checkIfReassignableFaxPresent: Parameter passed is NULL"));
                return false;
        }

        //Go to first message
        hr = pIncomingMsgIterator->MoveFirst();
        if (FAILED(hr))
        {
                _tprintf(_T("Failed to enumerate the first message. Error 0x%x \n"), hr);
                goto Exit;
        }

        //Count the number of Reassignable messages
        while(1)
        {
                VARIANT_BOOL vAtEOF;
                IFaxIncomingMessage* pIncomingMessage = NULL;
                hr = pIncomingMsgIterator->get_AtEOF(&vAtEOF);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_AtEOF failed. Error 0x%x \n"), hr);
                        goto Exit;
                }
                if (VARIANT_TRUE == vAtEOF)
                {            
                        break;
                }

                //Get the current message
                hr = pIncomingMsgIterator->get_Message(&pIncomingMessage);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_Message failed Error 0x%x \n"), hr);
                        goto Exit;
                }
                IFaxIncomingMessage2* pIncomingMessage2;
                pIncomingMessage2 = (IFaxIncomingMessage2*)pIncomingMessage;        
                VARIANT_BOOL vWasReassigned;
                //Was the msg reassigned?
                hr = pIncomingMessage2->get_WasReAssigned(&vWasReassigned);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_WasReAssigned failed Error 0x%x \n"), hr);
                        goto Exit;
                }
                if (VARIANT_TRUE != vWasReassigned)
                {
                        //if yes, increase count
                        iCount++;            
                }

                hr = pIncomingMsgIterator->MoveNext();
                if (FAILED(hr))
                { 
                        _tprintf(_T("MoveNext failed Error 0x%x \n"), hr);
                        goto Exit;
                }
        }    
        return iCount;
Exit:
        return -1;
}



//+---------------------------------------------------------------------------
//
//  function:   FaxAccountIncomingArchive
//
//  Synopsis:   Get the incoming archive folder of the current account
//
//  Arguments:  [pFaxFolders] - List of folders for the current account
//
//  Returns:    [IFaxIncomingMessageIterator]: Iterator to the messages in inbox folder
//
//  Modifies:
//
//----------------------------------------------------------------------------
IFaxIncomingMessageIterator* FaxAccountIncomingArchive(IFaxAccountFolders* pFaxFolders)
{
        if(pFaxFolders == NULL)
        {
                _tprintf(_T("FaxAccountIncomingArchive: Parameter passed is NULL"));
                return false;
        }
        IFaxAccountIncomingArchive* pFaxInbox = NULL;
        IFaxIncomingMessageIterator* pIncomingMsgIterator = NULL;
        HRESULT hr = S_OK;
        //Initialize MsgArchive Object
        hr = pFaxFolders->get_IncomingArchive(&pFaxInbox);
        if(FAILED(hr))
        {
                _tprintf(_T("GetIncomingArchive failed. Error = 0x%x \n"), hr);
                return NULL;
        }
        //Initialize Msg Iterator
        hr = pFaxInbox->GetMessages(NUM_MSGS, &pIncomingMsgIterator);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get Msg Iterator. Error = 0x%x \n"), hr);
                return NULL;
        }    
        return pIncomingMsgIterator;
}
//+---------------------------------------------------------------------------
//
//  function:   hasReassignPermission
//
//  Synopsis:   Check if the current user has ReAssign Permission
//
//  Arguments:  [pFaxServer] - Fax Server object
//
//  Returns:    bool: true if it has reassign permissions
//
//  Modifies:
//
//----------------------------------------------------------------------------
bool hasReassignPermission(IFaxServer2* pFaxServer )
{
        if(pFaxServer == NULL)
        {
                _tprintf(_T("hasReassignPermission: Parameter passed is NULL"));
                return false;
        }
        IFaxSecurity2* pFaxSecurity2 = NULL;
        HRESULT hr = S_OK;
        //Get the Security Object
        hr = pFaxServer->get_Security2(&pFaxSecurity2);
        FAX_ACCESS_RIGHTS_ENUM_2 enumFaxRights;
        //Get the Access Rights of the user
        hr = pFaxSecurity2->get_GrantedRights(&enumFaxRights);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed get_GrantedRights. Error 0x%x \n"), hr);
                return false;
        }    
        if((enumFaxRights & far2MANAGE_RECEIVE_FOLDER) == far2MANAGE_RECEIVE_FOLDER )
                return true;
        else
        {                
                return false;        
        }
}


//+---------------------------------------------------------------------------
//
//  function:   getUnassignedMsg
//
//  Synopsis:   Get unassigned msgs
//
//  Arguments:  [pIncomingMsgIterator] - Iterator to the messages in inbox folder
//                [pCount] - Referenced variable containing the number of reassignable faxes.
//
//  Returns:    TCHAR**: Array of strings containing the mesg ids of reassignable faxes
//
//  Modifies:
//
//----------------------------------------------------------------------------
TCHAR** getUnassignedMsg(IFaxIncomingMessageIterator* pIncomingMsgIterator, int* pCount)
{
        if((pIncomingMsgIterator == NULL) || (pCount == NULL))
        {
                _tprintf(_T("getUnassignedMsg: Parameter passed is NULL"));
                return false;
        }
        //Get the number of reassignable messages
        int count = checkIfReassignableFaxPresent(pIncomingMsgIterator);
        HRESULT hr = S_OK;
        TCHAR** strArrMsgID = NULL;
        int i = 0;

        //allocate memory for msgs
        strArrMsgID = (TCHAR**) malloc(count * sizeof(TCHAR*));
        if(strArrMsgID ==NULL)
        {
                hr = HRESULT_FROM_WIN32(GetLastError());
                _tprintf(_T("malloc failed. Error 0x%x \n"), hr);
                goto Exit;
        }
        memset(strArrMsgID, 0, count * sizeof(TCHAR*));

        //Goto first Msg
        hr = pIncomingMsgIterator->MoveFirst();
        if (FAILED(hr))
        {
                _tprintf(_T("Failed to enumerate the first message. Error 0x%x \n"), hr);
                goto Exit;
        }
        //Loop thru all msgs

        while(1)
        {
                VARIANT_BOOL vAtEOF;
                IFaxIncomingMessage* pIncomingMessage = NULL;
                hr = pIncomingMsgIterator->get_AtEOF(&vAtEOF);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_AtEOF failed. Error 0x%x \n"), hr);
                        goto Exit;
                }
                if (VARIANT_TRUE == vAtEOF)
                {            
                        break;
                }
                hr = pIncomingMsgIterator->get_Message(&pIncomingMessage);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_Message failed. Error = 0x%x \n"), hr);
                        goto Exit;
                }
                IFaxIncomingMessage2* pIncomingMessage2;
                pIncomingMessage2 = (IFaxIncomingMessage2*)pIncomingMessage;        
                VARIANT_BOOL vWasReassigned;
                hr = pIncomingMessage2->get_WasReAssigned(&vWasReassigned);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_WasReAssigned failed. Error = 0x%x \n"), hr);
                        goto Exit;
                }
                //if not reassigned
                if (VARIANT_TRUE != vWasReassigned)
                {
                        hr = pIncomingMessage2->get_Id(&strArrMsgID[i]);
                        if (FAILED(hr))
                        {
                                _tprintf(_T("get_id failed. Error = 0x%x \n"), hr);
                                goto ExitonError;
                        }            
                        i++;
                }      
                hr = pIncomingMsgIterator->MoveNext();
                if (FAILED(hr))
                {
                        _tprintf(_T("MoveNext failed. Error = 0x%x \n"), hr);
                        goto Exit;
                }
        }   
        goto Exit;
ExitonError:
        for    (int j =0;j< i;j++)
                SysFreeString(strArrMsgID[j]);
        free(strArrMsgID);
        *pCount = 0;
        strArrMsgID = NULL;        
Exit:        
        *pCount = i;
        return strArrMsgID;
}

//+---------------------------------------------------------------------------
//
//  function:   Reassign
//
//  Synopsis:   Reassign the Msg
//
//  Arguments:  [pIncomingMsgIterator] - Iterator to the messages in inbox folder
//                [strMsgId] - Id of the message to be reassigned
//                [strRecipients] - Recipients to whom the message is to be assigned.            
//
//  Returns:    bool : true if reassign was successful
//
//----------------------------------------------------------------------------
bool Reassign(IFaxIncomingMessageIterator* pIncomingMsgIterator, TCHAR* strMsgId, TCHAR* strRecipients )
{
        TCHAR strRecep[RECIPIENT_SIZE] = {0};
        HRESULT hr = S_OK;
        bool bRetVal = false;

        if(pIncomingMsgIterator == NULL && strMsgId != NULL && strRecipients != NULL) 
        {
                _tprintf(_T("Reassign: Parameter passed is NULL"));
                return false;
        }

        BSTR bstrSubject = SysAllocString(SUBJECT);
        BSTR bstrSenderName = SysAllocString(SENDER_NAME);
        BSTR bstrRecipientList = SysAllocString(strRecipients);
        BSTR bstrSenderFaxNumber = SysAllocString(SENDER_FAXNUMBER);

        //Goto first Msg
        hr = pIncomingMsgIterator->MoveFirst();
        if (FAILED(hr))
        {
                _tprintf(_T("Failed to enumerate the first message. Error 0x%x \n"), hr);
                bRetVal = false;
                goto Exit;
        }
        while(1)
        {        
                VARIANT_BOOL vAtEOF;
                IFaxIncomingMessage* pIncomingMessage = NULL;
                hr = pIncomingMsgIterator->get_AtEOF(&vAtEOF);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_AtEOF failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                if (VARIANT_TRUE == vAtEOF)
                {  
                        _tprintf(_T("Reassign Message Id not found \n"));
                        break;
                }
                //Get current Msg
                hr = pIncomingMsgIterator->get_Message(&pIncomingMessage);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_Message failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                IFaxIncomingMessage2* pIncomingMessage2;
                pIncomingMessage2 = (IFaxIncomingMessage2*)pIncomingMessage;        
                TCHAR* strCurrMsgId;                
                hr = pIncomingMessage2->get_Id(&strCurrMsgId);
                if (FAILED(hr))
                {
                        _tprintf(_T("get_Id failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if(_tcscmp(strCurrMsgId, strMsgId) == 0)
                {
                        //Set the Msg Parameters                        
                        hr = pIncomingMessage2->put_Subject(bstrSubject);
                        if (FAILED(hr))
                        {
                                _tprintf(_T("put_Subject failed. Error 0x%x \n"), hr);
                                bRetVal = false;
                                goto Exit;
                        }                        
                        hr = pIncomingMessage2->put_SenderName(bstrSenderName);
                        if (FAILED(hr))
                        {
                                _tprintf(_T("put_SenderName failed. Error 0x%x \n"), hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        
                        hr = pIncomingMessage2->put_Recipients(bstrRecipientList);
                        if (FAILED(hr))
                        {
                                _tprintf(_T("put_Recipients failed. Error 0x%x \n"), hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        
                        hr = pIncomingMessage2->put_SenderFaxNumber(bstrSenderFaxNumber);    
                        if (FAILED(hr))
                        {
                                _tprintf(_T("put_SenderFaxNumber failed. Error 0x%x \n"), hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        hr = pIncomingMessage2->ReAssign();
                        if (FAILED(hr))
                        {
                                _tprintf(_T("ReAssign failed. Error 0x%x \n"), hr);
                                bRetVal = false;
                                goto Exit;
                        }
                        _tprintf(_T("ReAssign was successful. \n"), hr);                        
                        bRetVal = true;
                        break;
                }

                //Next Msg
                hr = pIncomingMsgIterator->MoveNext();
                if (FAILED(hr))
                {
                        _tprintf(_T("MoveNext failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
        }        
Exit:
    if(bstrRecipientList)
        SysFreeString(bstrRecipientList);
    if(bstrSenderFaxNumber)
        SysFreeString(bstrSenderFaxNumber);
    if(bstrSenderName)
        SysFreeString(bstrSenderName);
    if(bstrSubject)
        SysFreeString(bstrSubject);
        return bRetVal;
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrMsgId = NULL;    
        LPTSTR lptstrOption = NULL;
        LPTSTR lptstrRecipient = NULL;
        bool bConnected = false;
        TCHAR** pstrFaxMsgIds = NULL;
        size_t argSize = 0;


        //introducing an artifical scope here so that the COM objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer = NULL;
                IFaxAccount* pFaxAccount;
                IFaxAccountFolders* pFaxFolders;
                IFaxIncomingMessageIterator* pIncomingMsgIterator;

                int argcount = 0;
                int count = 0;

                //Check is OS is Vista
                bool bVersion = IsOSVersionCompatible(VISTA);
                if(bVersion == false)
                {
                        _tprintf(_T("OS Version does not support this feature"));
                        bRetVal = false;
                        goto Exit1;
                }
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
                                                                //servername parameter
                                                                //handling the case " /s fax1 /s fax2 "
                                                                if(lptstrServerName == NULL)
                                                                {
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
                                                                                _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                                //handling the case " /s fax1 /s fax2 "
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
                                                                                _tprintf(_T("lptstrOption: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                        case 'i':
                                                                //handling the case " /s fax1 /s fax2 "
                                                                if(lptstrMsgId == NULL)
                                                                {
                                                                        //message id parameter
                                                                        lptstrMsgId = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrMsgId == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrMsgId: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrMsgId, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrMsgId,argSize+1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrMsgId: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                        case 'r':
                                                                //handling the case " /s fax1 /s fax2 "
                                                                if(lptstrRecipient == NULL)
                                                                {
                                                                        //recipient parameter
                                                                        lptstrRecipient = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrRecipient == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrRecipient: malloc failed. Error 0x%x \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrRecipient, 0, argSize+1* sizeof(TCHAR));

                                                                        hr = StringCchCopyN(lptstrRecipient,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrRecipient: StringCchCopyN failed. Error 0x%x \n"), hr);
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

                if ((lptstrOption == NULL) || (( _tcscmp(_T("list"), CharLower(lptstrOption)) != 0 ) && ((lptstrRecipient==NULL) || (lptstrMsgId ==NULL))))
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
                        _tprintf(_T("Failed to init com. Error 0x%x \n"), hr);
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
                BSTR serverName(lptstrServerName);
                hr = pFaxServer->Connect(serverName);
                if(FAILED(hr))
                {
                        _tprintf(_T("Connect failed. Error 0x%x \n"), hr);
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
                        _tprintf(_T("Feature not available on this version of the Fax API"));
                        goto Exit;
                }     
                hr = pFaxServer->get_CurrentAccount(&pFaxAccount);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetCurrentAccount failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }        

                //Now that we have got the account object lets get the folders object
                hr = pFaxAccount->get_Folders(&pFaxFolders);
                if(FAILED(hr))
                {
                        _tprintf(_T("Getting FaxAccountFolders failed Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //if reassign message option is selected
                if(_tcscmp(_T("reassign"), CharLower(lptstrOption))==0)
                {
                        if(hasReassignPermission(pFaxServer))
                        {
                                pIncomingMsgIterator = FaxAccountIncomingArchive(pFaxFolders);
                                if(pIncomingMsgIterator != NULL)
                                {
                                        if(!Reassign(pIncomingMsgIterator, lptstrMsgId, lptstrRecipient))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                        else
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;            
                                        }
                                }
                        }
                        else
                        {
                                _tprintf(_T("User doesn't have reassign permission \n"));
                                bRetVal = false;    

                        }
                }

                //if list message ids option is selected
                if(_tcscmp(_T("list"), CharLower(lptstrOption)) ==0)
                {
                        if(hasReassignPermission(pFaxServer))
                        {
                                pIncomingMsgIterator = FaxAccountIncomingArchive(pFaxFolders);
                                if(pIncomingMsgIterator != NULL)
                                {
                                        pstrFaxMsgIds = getUnassignedMsg(pIncomingMsgIterator, &count);            
                                        if(pstrFaxMsgIds == NULL)
                                        {
                                                _tprintf(_T("No reassignable faxes present \n"));
                                        }
                                        else
                                        {
                                                _tprintf(_T("Printing Msg Ids of reassignable faxes \n"));
                                                for(int i = 0; i<count; i++)
                                                {
                                                        _tprintf(_T("Msg Id of Message Number %d is %s \n"), i, pstrFaxMsgIds[i]);
                                                }
                                        }
                                }
                                else
                                {
                                        //we dont want to log any error here as the error will be logged in the function itself
                                        bRetVal = false;
                                }
                        }
                        else
                        {
                                _tprintf(_T("User doesn't have reassign permission \n"));
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
                if(lptstrRecipient)
                        free(lptstrRecipient);
                if(lptstrMsgId)
                        free(lptstrMsgId);    
                if(pstrFaxMsgIds)
                {
                        for    (int j =0;j< count;j++)
                        {
                                if(pstrFaxMsgIds[j])
                                        SysFreeString(pstrFaxMsgIds[j]);
                        }
                        free(pstrFaxMsgIds);
                        pstrFaxMsgIds = NULL;       
                }
        }    
        CoUninitialize();
Exit1:
        return bRetVal;
}

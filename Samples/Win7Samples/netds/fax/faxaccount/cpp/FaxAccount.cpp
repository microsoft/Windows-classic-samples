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

#include "FaxAccount.h"
#include <faxcomex_i.c>

#define ARR_SIZE(a) (sizeof(a)/sizeof(a[0]))

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
            /o <Add/Delete/Validate/Enum> Account option \n \
            /a Account Name Only if the option is Add/Delete or Validate \n"),AppName);
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
//  function:   DisplayFaxAccount
//
//  Synopsis:   prints the display name of a Fax Account
//
//  Arguments:  [pFaxAcc] - FaxAccount object whose display name is to be printed
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool DisplayFaxAccount(IFaxAccount* pFaxAcc)
{
        HRESULT hr = S_OK;
        BSTR bstrAccName;

        //check for NULL
        if(pFaxAcc == NULL)
        { 
                _tprintf(_T("DisplayFaxAccount: Parameter passed is NULL"));
                return false;
        }
        //Get the AccountName from pFaxAcc
        hr = pFaxAcc->get_AccountName(&bstrAccName);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get acc name. Error 0x%x \n"), hr);
                return false;
        }
        _tprintf(_T("AccountName: %s \n"), bstrAccName);
        if(bstrAccName)
            SysFreeString(bstrAccName);
        return true;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxEnumAccounts
//
//  Synopsis:   Enumerates the list of accounts
//
//  Arguments:  [pFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
//                [bCheck] - if set to true then is verifies if the account with name lptstrAccName is present.
//                [lptstrAccName] - name of the account that is to be verified.
//                [pbFound] - used to return the result of whether the account is present or not
//
//  Returns:    bool: true if passed successfully
//
//  Modifies:    pbFound : if the account with the name lptstrAccName is found then pbFound is set to true.
//
//----------------------------------------------------------------------------
bool FaxEnumAccounts(IFaxAccountSet* pFaxAccSet, bool bCheck = false, LPTSTR lptstrAccName= NULL, bool* pbFound = NULL)
{
        HRESULT hr = S_OK;
        IFaxAccounts* pFaxAccounts;
        if(pFaxAccSet == NULL )
        { 
                _tprintf(_T("FaxEnumAccounts: Parameter passed is NULL"));
                return false;
        }
        if(bCheck && (lptstrAccName == NULL || pbFound == NULL))
        { 
                _tprintf(_T("FaxEnumAccounts: bCheck is True So, parameters can't be NULL"));
                return false;
        }

        //Get all the Accounts 
        hr = pFaxAccSet->GetAccounts(&pFaxAccounts);
        if(FAILED(hr))
        {
                _tprintf(_T("GetAccounts failed. Error 0x%x \n"), hr);
                return false;
        }

        long numOfAccs;
        //Total number of accounts
        hr = pFaxAccounts->get_Count(&numOfAccs);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Count failed. Error 0x%x \n"), hr);
                return false;
        }
        else
        {
                _tprintf(_T("Number of accounts: %d \n"), numOfAccs);
        }

        //start enumerating each account.
        IUnknown* pUnknown;
        hr = pFaxAccounts->get__NewEnum(&pUnknown);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get IUnknown Error. 0x%x \n"), hr);
                return false;
        }

        //now get the variant interface.
        IEnumVARIANT* pIEnumVariant;
        hr = pUnknown->QueryInterface(&pIEnumVariant);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to get IEnumVariant. Error 0x%x \n"), hr);
                return false;
        }
        VARIANT acc_variant;
        VariantInit(&acc_variant);
        while(1)
        {
                DWORD numReturned = 0;
                hr = pIEnumVariant->Next(1, &acc_variant, &numReturned);
                if(FAILED(hr))
                {
                        _tprintf(_T("Next failed. Error 0x%x \n"), hr);
                        return false;
                }
                if(S_FALSE == hr)
                {
                        //enumeration is done
                        _tprintf(_T("Enumeration of accounts done. \n"));
                        break;
                }
                //Get the Account object
                IFaxAccount* pFaxAccount;
                hr = acc_variant.pdispVal->QueryInterface(IID_IFaxAccount, (void**)&pFaxAccount);
                if(FAILED(hr))
                {
                        _tprintf(_T("QueryInterface for account failed. Error 0x%x \n"), hr);
                        return false;
                }
                //if check that a account is present.
                if(bCheck)
                {
                        BSTR bstrAccName;
                        hr = pFaxAccount->get_AccountName(&bstrAccName);
                        if(FAILED(hr))
                        {
                                _tprintf(_T("Failed to get acc name. Error 0x%x \n"), hr);
                                return false;
                        }
                        if(_tcsicmp(bstrAccName, lptstrAccName) == 0)
                        {
                                if(NULL != pbFound)
                                {
                                        *pbFound = true;
                                }
                        }
                        if(bstrAccName)
                            SysFreeString(bstrAccName);
                }
                //Display the current account info.
                DisplayFaxAccount(pFaxAccount);
                VariantClear(&acc_variant);
        }
        return true;
}


//+---------------------------------------------------------------------------
//
//  function:   AddAccount
//
//  Synopsis:   Adds a Fax Account
//
//  Arguments:  [pFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
//              [lptstrAccName] - name of the account to be added. Must be a valid NT/Domain user.
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool AddAccount(IFaxAccountSet* accSet, LPTSTR lptstrAccName)
{
        bool flag = false;
        if(accSet == NULL || lptstrAccName == NULL)
        { 
                _tprintf(_T("AddAccount: Parameter passed is NULL"));
                return false;
        }
        //first enum the existing accounts
        if(!FaxEnumAccounts(accSet))
        {
                //enum failed 
                _tprintf(_T("FaxEnumAccounts failed \n"));
        }

        //now add the account
        HRESULT hr = S_OK;
        IFaxAccount* pFaxAccount;
        BSTR bstrAccountName = SysAllocString(lptstrAccName);
        if(bstrAccountName == NULL)
        {
                //getting the account failed!!!
                _tprintf(_T("SysAllocString failed. Error \n"));
                goto Exit;
        }

        hr = accSet->AddAccount(bstrAccountName, &pFaxAccount);
        if(FAILED(hr))
        {
                _tprintf(_T("Failed to add account. Error 0x%x \n"), hr);
                goto Exit;
        }
        _tprintf(_T(" Account was added. \n"));
        //Display Info on added account.
        DisplayFaxAccount(pFaxAccount);
        flag = true;
Exit:
        if(bstrAccountName)
                SysFreeString(bstrAccountName);
        return flag;
}

//+---------------------------------------------------------------------------
//
//  function:   DeleteAccount
//
//  Synopsis:   Deletes a Fax Account
//
//  Arguments:  [pFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
//              [lptstrAccName] - name of the account to be deleted. 
//
//  Returns:    bool: true if passed successfully
//----------------------------------------------------------------------------
bool DeleteAccount(IFaxAccountSet* pFaxAccSet, LPTSTR lptstrAccountName)
{ 
        bool flag = false;
        if(pFaxAccSet == NULL || lptstrAccountName == NULL)
        { 
                _tprintf(_T("DeleteAccount: Parameter passed is NULL"));
                return false;
        }
        //now lets enumerate the existing accounts first.
        if(!FaxEnumAccounts(pFaxAccSet))
        {
                //FaxEnumAccounts failed
                _tprintf(_T("FaxEnumAccounts failed before deleting \n"));
        }
        HRESULT hr = S_OK;
        BSTR bstrAccountName = SysAllocString(lptstrAccountName);
        if(bstrAccountName == NULL)
        {
                //getting the account failed!!!
                _tprintf(_T("SysAllocString failed. Error \n"));
                goto Exit;
        }
        hr = pFaxAccSet->RemoveAccount(bstrAccountName);
        if(FAILED(hr))
        {
                _tprintf(_T("RemoveAccount failed. Error 0x%x \n"), hr);
                goto Exit;
        }
        //now enumerate to see if account exists
        bool bFound = false;
        if(!FaxEnumAccounts(pFaxAccSet, true, lptstrAccountName, &bFound))
        {
                //we can properly validate if this call fails, hence log an error
                _tprintf(false, _T("FaxEnumAccounts failed during validation \n"));
                goto Exit;
        }
        if(bFound)
        {
                //we just deleted the account but still the enumeration shows the account. hecen log error
                _tprintf(false, _T("Account exists after deleteion \n"));
                goto Exit;
        }
        _tprintf(_T(" Account was deleted. \n"));
        flag = true;
Exit:
        if(bstrAccountName)
                SysFreeString(bstrAccountName);
        return flag;
}

//+---------------------------------------------------------------------------
//
//  function:   GetAccountInfo
//
//  Synopsis:   Get the account object.
//
//  Arguments:  [pFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
//              [lptstrAccountName] - Account whose info is to be printed.
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool GetAccountInfo(IFaxAccountSet* pFaxAccSet, LPTSTR lptstrAccountName)
{
        IFaxAccount* pFaxAccount;
        HRESULT hr = S_OK;
        bool flag = false;

        if(pFaxAccSet == NULL || lptstrAccountName == NULL)
        { 
                _tprintf(_T("GetAccountInfo: Parameter passed is NULL"));
                return false;
        }
        BSTR bstrAccountName = SysAllocString(lptstrAccountName);
        if(bstrAccountName == NULL)
        {
                //getting the account failed!!!
                _tprintf(_T("SysAllocString failed. Error \n"));
                goto Exit;
        }

        //Get the account with the name lptstrAccountName
        hr = pFaxAccSet->GetAccount(bstrAccountName, &pFaxAccount);
        if(FAILED(hr))
        {
                //getting the account failed!!!
                _tprintf(_T("GetAccount failed. Error 0x%x \n"), hr);
                goto Exit;
        }
        DisplayFaxAccount(pFaxAccount);
        flag = true;
Exit:
        if(bstrAccountName)
                SysFreeString(bstrAccountName);
        return flag;
}



int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrAccountName = NULL;
        LPTSTR lptstrOption = NULL;
        BSTR bstrServerName = NULL;
        bool bConnected = false;
        size_t argSize = 0;
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
                IFaxAccountSet* pFaxAccountSet;

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
                                                                //handling the case " /s fax1 /s fax2 "

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

                                                                        lptstrOption = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));                        
                                                                        if(lptstrOption == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrOption: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrOption, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrOption,argSize +1, argv[argcount+1],argSize);
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
                                                        case 'a':
                                                                //handling the case " /s fax1 /s fax2 "

                                                                if(lptstrAccountName == NULL)
                                                                {
                                                                        lptstrAccountName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrAccountName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrAccountName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrAccountName, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrAccountName, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrAccountName: StringCchCopyN failed. Error 0x%x \n"), hr);
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

                if (!lptstrOption || ( _tcscmp(_T("enum"), CharLower(lptstrOption)) != 0 ) && !lptstrAccountName)
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
                bstrServerName = SysAllocString(lptstrServerName);
                hr = pFaxServer->Connect(bstrServerName);
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

                //lets also get the account set since that is the basis for all account relates operations
                hr = pFaxServer->get_FaxAccountSet(&pFaxAccountSet);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_FaxAccountSet failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //if Enum Account option is selected
                if(_tcscmp(_T("enum"), CharLower(lptstrOption)) == 0)
                {
                        if(!FaxEnumAccounts(pFaxAccountSet))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }

                //if Add Account option is selected
                if(_tcscmp(_T("add"), CharLower(lptstrOption)) == 0)
                {
                        if(!AddAccount(pFaxAccountSet, lptstrAccountName))
                        {
                                bRetVal = false;
                        }
                }

                //if Delete Account option is selected
                if(_tcscmp(_T("delete"), CharLower(lptstrOption)) == 0)
                {
                        if(!DeleteAccount(pFaxAccountSet, lptstrAccountName))
                        {
                                bRetVal = false;
                        }
                }

                //if validate account option is selected
                if(_tcscmp(_T("validate"), CharLower(lptstrOption))== 0)
                {
                        if(!GetAccountInfo(pFaxAccountSet, lptstrAccountName))
                        {
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
                if(lptstrAccountName)
                        free(lptstrAccountName);
                if(bstrServerName)
                        SysFreeString(bstrServerName);
        }
        CoUninitialize();
Exit1:
        return bRetVal;
}

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

#include "ServerConfig.h"
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
            /o <PersonalCoverPage/Branding/IncomingFaxesPublic/AutoCreateAccount> Account option \n \
            /v value to be set \"0\" or \"1\" \n"),AppName);
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
//  function:   PrintGeneralConfig
//
//  Synopsis:   prints the Server Configuration (PersonalCoverPages, Branding, IncomingFaxPublic and AutoCreateOnConnect)
//
//  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool PrintGeneralConfig(IFaxConfiguration* pFaxConfiguration)
{
        HRESULT  hr = S_OK;
        bool bRetVal = true;    
        VARIANT_BOOL varBool = VARIANT_FALSE;


        //check for NULL
        if (pFaxConfiguration == NULL) 
        {
                _tprintf(_T("PrintGeneralConfig: Parameter passed is NULL"));
                goto Exit;
        }


        _tprintf(_T("\n Logging General Config details....\n \n"));

        hr = pFaxConfiguration->Refresh();
        if(FAILED(hr))
        {
                _tprintf(_T("Refresh failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        hr = pFaxConfiguration->get_AllowPersonalCoverPages(&varBool);
        if(FAILED(hr))
        {
                _tprintf(_T("get_AllowPersonalCoverPages failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        if(varBool == VARIANT_TRUE)
                _tprintf(_T("AllowPersonalCoverPages = true \n"));
        else
                _tprintf(_T("AllowPersonalCoverPages = false \n"));

        hr = pFaxConfiguration->get_AutoCreateAccountOnConnect(&varBool);
        if(FAILED(hr))
        {
                _tprintf(_T("get_AutoCreateAccountOnConnect failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        if(varBool == VARIANT_TRUE)
                _tprintf(_T("AutoCreateAccountOnConnect = true \n"));
        else
                _tprintf(_T("AutoCreateAccountOnConnect = false \n"));

        hr = pFaxConfiguration->get_Branding(&varBool);
        if(FAILED(hr))
        {
                _tprintf(_T("get_Branding failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        if(varBool == VARIANT_TRUE)
                _tprintf(_T("Branding = true \n"));
        else
                _tprintf(_T("Branding = false \n"));

        hr = pFaxConfiguration->get_IncomingFaxesArePublic(&varBool);
        if(FAILED(hr))
        {
                _tprintf(_T("get_IncomingFaxesArePublic failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        if(varBool == VARIANT_TRUE)
                _tprintf(_T("IncomingFaxesArePublic = true \n"));
        else
                _tprintf(_T("IncomingFaxesArePublic = false \n")); 

Exit: 
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   setIncomingFaxesArePublic
//
//  Synopsis:   sets the valus of IncomingFaxArePublic according to bState value
//
//  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
//              [bState] -    bool value set to true or false
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool setIncomingFaxesArePublic(IFaxConfiguration* pFaxConfiguration, VARIANT_BOOL bState )
{
        HRESULT hr = S_OK;
        bool bRetVal = false;
        //check for NULL
        if (pFaxConfiguration == NULL) 
        {
                _tprintf(_T("PrintGeneralConfig: Parameter passed is NULL"));
                goto Exit;
        }

        pFaxConfiguration->Refresh();
        VARIANT vState;
        VariantInit(&vState);
        vState.boolVal = bState;
        //Set the configuration object
        hr = pFaxConfiguration->put_IncomingFaxesArePublic(vState.boolVal);
        if(FAILED(hr))
        {
                _tprintf(_T("put_IncomingFaxesArePublic Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        //Save it
        hr = pFaxConfiguration->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setIncomingFaxesArePublic: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        bRetVal = true;
Exit:
        return bRetVal;
}
//+---------------------------------------------------------------------------
//
//  function:   setAllowPersonalCoverPages
//
//  Synopsis:   sets the valus of AllowPersonalCoverPages according to bState value
//
//  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
//              [bState] -    bool value set to true or false
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool setAllowPersonalCoverPages(IFaxConfiguration* pFaxConfiguration, VARIANT_BOOL bState )
{
        HRESULT hr = S_OK;
        bool bRetVal = false;
        //check for NULL
        if (pFaxConfiguration == NULL) 
        {
                _tprintf(_T("PrintGeneralConfig: Parameter passed is NULL"));
                goto Exit;
        }

        pFaxConfiguration->Refresh();
        VARIANT vState;
        VariantInit(&vState);
        vState.boolVal = bState;
        //Set the configuration object
        hr = pFaxConfiguration->put_AllowPersonalCoverPages(vState.boolVal);
        if(FAILED(hr))
        {
                _tprintf(_T("put_AllowPersonalCoverPages Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        //Save it
        hr = pFaxConfiguration->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setAllowPersonalCoverPages: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        bRetVal = true;
Exit:
        return bRetVal;
}
//+---------------------------------------------------------------------------
//
//  function:   setBranding
//
//  Synopsis:   sets the valus of Branding according to bState value
//
//  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
//                [bState] -    bool value set to true or false
//
//  Returns:    bool: true if passed successfully
//
//  Modifies:
//
//----------------------------------------------------------------------------
bool setBranding(IFaxConfiguration* pFaxConfiguration, VARIANT_BOOL bState )
{
        HRESULT hr = S_OK;
        bool bRetVal = false;
        //check for NULL
        if (pFaxConfiguration == NULL) 
        {
                _tprintf(_T("PrintGeneralConfig: Parameter passed is NULL"));
                goto Exit;
        }

        pFaxConfiguration->Refresh();
        VARIANT vState;
        VariantInit(&vState);
        vState.boolVal = bState;
        //Set the configuration object
        hr = pFaxConfiguration->put_Branding(vState.boolVal);
        if(FAILED(hr))
        {
                _tprintf(_T("put_Branding Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        //Save it
        hr = pFaxConfiguration->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setBranding: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        bRetVal = true;
Exit:
        return bRetVal;
}


//+---------------------------------------------------------------------------
//
//  function:   setAutoCreateAccountOnConnect
//
//  Synopsis:   sets the valus of AutoCreateAccountonConnect according to bState value
//
//  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
//                [bState] -    bool value set to true or false
//
//  Returns:    bool: true if passed successfully
//
//  Modifies:
//
//----------------------------------------------------------------------------
bool setAutoCreateAccountOnConnect(IFaxConfiguration* pFaxConfiguration, VARIANT_BOOL bState )
{
        HRESULT hr = S_OK;
        bool bRetVal = false;
        //check for NULL
        if (pFaxConfiguration == NULL) 
        {
                _tprintf(_T("PrintGeneralConfig: Parameter passed is NULL"));
                goto Exit;
        }

        pFaxConfiguration->Refresh();
        VARIANT vState;
        VariantInit(&vState);
        vState.boolVal = bState;
        //Set the configuration object
        hr = pFaxConfiguration->put_AutoCreateAccountOnConnect(vState.boolVal);
        if(FAILED(hr))
        {
                _tprintf(_T("put_AutoCreateAccountOnConnect Failed. Error. 0x%x \n"), hr);                
                goto Exit;
        }
        //Save it
        hr = pFaxConfiguration->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setAutoCreateAccountOnConnect: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        bRetVal = true;
Exit:
        return bRetVal;
}

int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrValue = NULL;
        LPTSTR lptstrOption = NULL;
        BSTR bstrServerName = NULL;
        bool bConnected = false;
        size_t argSize = 0;
        VARIANT_BOOL vbState = VARIANT_FALSE;
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
                IFaxConfiguration* pFaxConfiguration;

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
                                                        case 'v':
                                                                if(lptstrValue == NULL)
                                                                {
                                                                        lptstrValue = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrValue == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrValue: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrValue, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrValue, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrValue: StringCchCopyN failed. Error 0x%x \n"), hr);
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

                if ((lptstrOption == NULL) || (lptstrValue == NULL ) || ((_tcscmp(_T("0"), lptstrValue) != 0) && (_tcscmp(_T("1"), lptstrValue) != 0)))
                {
                        _tprintf( TEXT("Missing/Invalid Value.\n") );
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
                        _tprintf(_T("OS Version does not support this feature"));
                        goto Exit;
                }         

                hr = pFaxServer->get_Configuration(&pFaxConfiguration);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetCurrentAccount failed. Error 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if (_tcscmp(_T("0"), lptstrValue) == 0)
                {
                        vbState = VARIANT_FALSE;
                }
                if (_tcscmp(_T("1"), lptstrValue) == 0)
                {
                        vbState = VARIANT_TRUE;
                }

                _tprintf(_T("Current Configuration. \n \n"));
                if(!PrintGeneralConfig(pFaxConfiguration))
                {
                        //we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = false;
                }

                //if PersonalCoverPages option is selected
                if(_tcscmp(_T("personalcoverpage"), CharLower(lptstrOption)) == 0)
                {
                        if(!setAllowPersonalCoverPages(pFaxConfiguration, vbState))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }
                //if Branding option is selected
                if(_tcscmp(_T("branding"), CharLower(lptstrOption)) == 0)
                {
                        if(!setBranding(pFaxConfiguration, vbState))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }
                //if IncomingFaxArePublic option is selected
                if(_tcscmp(_T("incomingfaxespublic"), CharLower(lptstrOption)) == 0)
                {
                        if(!setIncomingFaxesArePublic(pFaxConfiguration, vbState))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }
                //if AutoCreateAccount option is selected
                if(_tcscmp(_T("autocreateaccount"), CharLower(lptstrOption)) == 0)
                {
                        if(!setAutoCreateAccountOnConnect(pFaxConfiguration, vbState))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }
                _tprintf(_T("Current Server settings after the changes... \n"));
                if(!PrintGeneralConfig(pFaxConfiguration))
                {
                        //we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = false;
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
                if(lptstrValue)
                        free(lptstrValue);
                if(bstrServerName)
                        SysFreeString(bstrServerName);
        }
        CoUninitialize();
Exit1:
        return bRetVal;

}

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

#include "SendToFaxRecipient.h"

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
            /o <cansendtofax> or <sendtofax> \n \
            /d Document that is to be sent as Fax  \n"),AppName);
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
//  function:   CorrectString
//
//  Synopsis:   Replaces the semicolon separated string with space separated string
//
//  Arguments:  [inputDocListString] - The list of documents in string format separated by semicolon
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------

bool CorrectString(LPTSTR inputDocListString)
{      
        bool bRetVal = false;
        if(NULL == inputDocListString)
        {
                return false;
        }

        HRESULT hr = S_OK;      
        size_t iStrLen = 0;

        hr = StringCchLength( inputDocListString,  2056 , &iStrLen);
        if(FAILED(hr))
        {
                _tprintf(_T("StringCchLength of inputDocListString failed.  Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
  
        //Replace ; with space
        size_t i = 0;
        while(i < iStrLen)
        {                            
                if(inputDocListString[i] == ';')
                {
                    (inputDocListString)[i] = ' ';
                }
                i++;
        }
        bRetVal = true;
Exit:        
        return bRetVal;

}

int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrOption = NULL;
        LPTSTR lptstrDoc = NULL;
        bool bConnected = false;
        bool bResult = false;
        int argcount = 0;
        size_t argSize = 0;
        DWORD dwRet = 0;
        bool bFlag = false;
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("This sample is compatible with Windows Vista"));
                bRetVal = false;
                goto Exit;
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
                                                                hr = StringCchCopyN(lptstrOption,argSize+1, argv[argcount+1],argSize);
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
                                                case 'd':
                                                        if(lptstrDoc == NULL)
                                                        {                                                                  

                                                                lptstrDoc = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));                        
                                                                if(lptstrDoc == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrDoc: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                memset(lptstrDoc, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrDoc,argSize +1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrDoc: StringCchCopyN failed. Error %x \n"), hr);
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

        if ((lptstrOption == NULL) || ((lptstrDoc == NULL) && ( _tcscmp(_T("sendtofax"), CharLower(lptstrOption)) == 0 ))) 
        {
                _tprintf( TEXT("Invalid Value.\n") );
                GiveUsage(argv[0]);
                bRetVal = false;
                goto Exit;
        }

        if( _tcscmp(_T("sendtofax"), CharLower(lptstrOption)) == 0 )
        {
            bFlag = CorrectString(lptstrDoc);
            if(bFlag == true)
            {
                dwRet = SendToFaxRecipient(SEND_TO_FAX_RECIPIENT_ATTACHMENT,lptstrDoc);  
                if(dwRet != 0)
                {
                        _tprintf(_T("SendToFaxRecipient: failed. Error %d \n"), dwRet);
                        bRetVal = false;
                        goto Exit;
                }
                _tprintf(_T("SendToFaxRecipient was successful"));
            }
            else
            {
                    _tprintf(_T("Replacement of semicolon with space failed.\n"));
                    bRetVal = false;
                    goto Exit;
            }

        }
        if( _tcscmp(_T("cansendtofax"), CharLower(lptstrOption)) == 0 )
        {
                if(CanSendToFaxRecipient() == FALSE)
                {
                        _tprintf(_T("CanSendToFaxRecipient: failed. Error %d \n"), GetLastError());
                        bRetVal = false;
                        goto Exit;
                }
                _tprintf(_T("CanSendToFaxRecipient was successful"));
        }

Exit:
        if(lptstrDoc)
                free(lptstrDoc);
        if(lptstrOption)
                free(lptstrOption);               
        return bRetVal;
}

/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzMigrate.cpp

Abstract:

    Defines the entry point for the console application.

 History:

****************************************************************************/


#include "stdafx.h"
#include "AzMStore.h"
#include "Shellapi.h"

// Function prototype declarations

void DisplayUsage();
void run();
int parseCommandLine();
void DisplayUsage();

/*++

Routine description:

    This method runs most of the migration code

Arguments: NONE

Return Value:NONE


--*/

void run() {

    HRESULT hr;

    CAzMStore sourceStore(CAzGlobalOptions::m_bstrSourceStoreName),destStore(CAzGlobalOptions::m_bstrDestStoreName);

    hr=sourceStore.InitializeStore(false);

    if (FAILED(hr))
        goto lError1;

    hr=destStore.InitializeStore(true);

    if (FAILED(hr)) {

        if (CAzGlobalOptions::m_bOverWrite==true) {

            hr=destStore.OverWriteStore();
    
            CAzLogging::Log(hr,_TEXT("Overwriting AzMan Authornization Store"),COLE2T(CAzGlobalOptions::m_bstrDestStoreName));

            if (FAILED(hr))
                goto lError1;

        } else {

            goto lError1;
        }
    }

    if (CAzGlobalOptions::m_bSpecificApp==false)
        hr=sourceStore.OpenAllApps();
    else {
        for (vector<CComBSTR>::iterator iter=CAzGlobalOptions::m_bstrAppNames.begin() ; iter !=CAzGlobalOptions::m_bstrAppNames.end() ; iter++)

        hr=sourceStore.OpenApp(*iter);

        if (FAILED(hr))
            goto lError1;

    }

    if (FAILED(hr))
        goto lError1;

    hr=destStore.Copy(sourceStore);

lError1:
    CAzLogging::Close();
}


/*++

Routine description:

    This method parses the command line arguments and sets the appropriate
    members in the CAzGlobalOptions class

Return Value:

    Returns success - 0
            failure - -1
            help - 1

--*/


int parseCommandLine() {

    int iRet    =   0;
    int iNumArgs, iNumToBeProcessed;

    LPTSTR lpstrCommandArgs=GetCommandLine();
    
    LPTSTR *rgCmdArgs=CommandLineToArgvW(lpstrCommandArgs,&iNumArgs);

    // To take care of no command line arguments
    if (iNumArgs <= 1 ) {
        iRet = -1;
        goto Cleanup;
    }

    // Check if one argument only and that is the /? argument
    if (0==_tcsncicmp(rgCmdArgs[1],CAzGlobalOptions::HELPTAG , CAzGlobalOptions::HELPTAG_LEN)) {
          DisplayUsage();
          iRet = 1;
          goto Cleanup;
    }

    // if more than one argument but not the required 2 arguments
    if (iNumArgs < 3 ) {
        iRet = -1;
        goto Cleanup;
    }

    CAzGlobalOptions::m_bstrDestStoreName.Attach(CComBSTR(rgCmdArgs[1]).Copy());

    CAzGlobalOptions::m_bstrSourceStoreName.Attach(CComBSTR(rgCmdArgs[2]).Copy());

    CAzGlobalOptions::setDefaults();

    iNumToBeProcessed = iNumArgs-3;

    for (int i = 3 ; i < iNumArgs ; i++) {
        
        // Checking for /logfile
        if (0==_tcsncicmp(rgCmdArgs[i],CAzGlobalOptions::LOGFILETAG,CAzGlobalOptions::LOGFILETAG_LEN)) {

            _TCHAR *strRightPart=_tcschr(rgCmdArgs[i],_TCHAR('='));

            if (NULL==strRightPart) {
                iRet = -1;
                goto Cleanup;
            }

            CAzLogging::Initialize(CAzLogging::LOG_LOGFILE,&strRightPart[1]);

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i],CAzGlobalOptions::APPNAMETAG,CAzGlobalOptions::APPNAMETAG_LEN)) {

            //Checking for /application flag

            LPTSTR strTmp;

            _TCHAR *strRightPart=_tcschr(rgCmdArgs[i],_TCHAR('='));

            if (NULL==strRightPart) {
                iRet = -1;
                goto Cleanup;
            }
            CAzGlobalOptions::m_bSpecificApp=true;

            _TCHAR *strAppNames =_tcstok_s(&strRightPart[1],_TEXT(","),&strTmp);

            while (strAppNames!=NULL) {

                CAzGlobalOptions::m_bstrAppNames.push_back(CComBSTR(strAppNames).Copy());

                /* While there are tokens in "string" */
                /* Get next token: */

                strAppNames =_tcstok_s(NULL,_TEXT(","),&strTmp);

                }

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i] , CAzGlobalOptions::OVERWRITETAG , CAzGlobalOptions::OVERWRITETAG_LEN)) {

            //Checking for /overwrite flag

            CAzGlobalOptions::m_bOverWrite=true;

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i] , CAzGlobalOptions::IGNOREMEMBERSTAG,CAzGlobalOptions::IGNOREMEMBERSTAG_LEN)) {

            //Checking for /IGNOREMEMBERS flag

            CAzGlobalOptions::m_bIgnoreMembers=true;

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i] , CAzGlobalOptions::IGNOREPOLICYADMINSTAG , CAzGlobalOptions::IGNOREPOLICYADMINSTAG_LEN)) {

            //Checking for /IGNOREPOLICYADMIN flag

            CAzGlobalOptions::m_bIgnorePolicyAdmins=true;

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i] , CAzGlobalOptions::VERBOSETAG , CAzGlobalOptions::VERBOSETAG_LEN)) {

            CAzGlobalOptions::m_bVerbose=true;

            CAzLogging::Initialize(CAzLogging::LOG_TRACE);

            iNumToBeProcessed--;

        } else if (0==_tcsncicmp(rgCmdArgs[i] , CAzGlobalOptions::HELPTAG , CAzGlobalOptions::HELPTAG_LEN)) {

            DisplayUsage();

            iNumToBeProcessed--;
            iRet = 1;
            goto Cleanup;
        }
    }
    // Some additional parameters exist which donot match
    // hence these are invalid flags.
    if (0 != iNumToBeProcessed)
    {
        iRet = -1;
        goto Cleanup;
    }

    iRet = 0;

Cleanup:
    
    if (rgCmdArgs)
    {
        LocalFree(rgCmdArgs);
    }

    return iRet;
}

/*++

Routine description:

    This method displays the usage details of the tool

Arguments: NONE

Return Value: NONE


--*/

void DisplayUsage() {

    wcout << _TEXT("\tAzMigrate <destination store> <source store> [flags]\n") << endl ;

    wcout << _TEXT("\tFlags:")<<endl << _TEXT("\t/o : If destination store exists, it would be overwritten")<< endl;

    wcout << _TEXT("\t/a=[application name1,application name2,....] : Migrate specified applications only to destination store")<< endl;

    wcout << _TEXT("\t/l=[log file name] : Log all the operations performed during migration into specified log file")<< endl;

    wcout << _TEXT("\t/ip : Ignore all Policy assignments")<< endl;

    wcout << _TEXT("\t/im : Ignore all members")<< endl;

    wcout << _TEXT("\t/v : Verbose mode")<< endl;

    wcout << _TEXT("\t/? : Help")<< endl;
}



int __cdecl main(int argc, __in_ecount(argc) _TCHAR* argv[])
{
    int iReturnValue=0;
    HRESULT     hr;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED); 

    if (FAILED(hr))
    {

        wcout << _TEXT("Failed to initialize COM. CoInitializeEx return ") << CAzLogging::getMsgBuf(hr)  << endl;
        goto Done;
    }
        
    int iStatus = parseCommandLine();

    if (iStatus < 0) {

        wcout << _TEXT("Invalid command line arguments") << endl;

        DisplayUsage();

        iReturnValue = -1;

        goto Done;
    }
    
    if (iStatus==0) {

        run();

        iReturnValue = CAzLogging::MIGRATE_SUCCESS ? 0 : -1;

        if (CAzLogging::MIGRATE_SUCCESS)
            wcout << _TEXT("Successful Migration") << endl;
        else
            wcout << _TEXT("Errors in Migration") << endl;
    }

Done:
    CoUninitialize();

    return iReturnValue;
}


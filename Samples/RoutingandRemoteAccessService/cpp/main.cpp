// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"
#include "InterfaceConfiguration.h"
#include "ServerConfiguration.h"
#include "VpnConnection.h"

//********************************************************************************************
// Function: MprApiSampleUsage
//
// Description: Prints the application usage.
//
//********************************************************************************************
VOID MprApiSampleUsage()
{
    wprintf(L"Usage:\n");
    wprintf(L"\t MprApiSample.exe [RemoteAccesServerName]\n");
}

//********************************************************************************************
// Function: Main
//
// Description: The main function, prompts for a input, to enable the user to play with the Sample SDK
//
//********************************************************************************************

int __cdecl wmain(_In_ int argc, _In_reads_(argc) PWSTR argv[])
{
    int userChoice = -1;
    LPWSTR rrasServerName = NULL;
    int optionCount = 0;
    BOOL exitSample = FALSE;
    BOOL invalidChoice = FALSE;
    DWORD status = ERROR_SUCCESS;

    typedef VOID(*MPRAPI_TEST_FUNC)(LPWSTR);
   
    typedef struct _MPRAPI_SAMPLE
    {
        MPRAPI_TEST_FUNC testFunc;
        LPWSTR sampleName;
    }MPRAPI_SAMPLE;
       
    static MPRAPI_SAMPLE sampleList[] = {
        {SetCustomIpsecConfigurationOnInterface, L"Configure custom IPSec configuration on demand dial interface."},
        {RemoveCustomIpsecConfigurationFromInterface, L"Remove custom IPSec configuration from demand dial interface."},
        {SetCustomIpsecConfigurationOnServer, L"Configure custom IPSec configuration on Remote access server"},
        {RemoveCustomIpsecConfigurationFromServer, L"Remove custom IPSec configuration from Remote access server"},
        {EnumerateVpnConnections, L"Enumerate VPN connections"},
        {NULL, L"Exit"}
        };

    if (argc > 1)
    {
        if ((_wcsicmp(argv[1], L"/?") == 0) ||
            (_wcsicmp(argv[1], L"-help") == 0) ||
            (_wcsicmp(argv[1], L"?") == 0))
        {
            MprApiSampleUsage();
            return 0;
        }
        else
        {
            // Remote access server name is passed as an input from commandline
            rrasServerName = argv[1];
        }
    }
    
    status = LoadMprApiLibrary();
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"LoadMprApiLibrary failed: %u\n", status);
        goto Done;
    }
    
    //Get user choice option to play with the MprAPI sample SDK
    while (!exitSample)
    {
        optionCount = ARRAYSIZE(sampleList);
        
        for(int i=0; i < optionCount; i++)
        {
            wprintf(L"   %i. %s\n", (i + 1), sampleList[i].sampleName);
        }
        
        wprintf(L"---------------------------------------------------------\n");
        wprintf(L"Enter a choice (1-%i): ", optionCount);

        do
        {
            wscanf_s(L"%i", &userChoice);
            FlushCurrentLine();
            invalidChoice = (userChoice < 0 || userChoice > optionCount);
            if (invalidChoice)
            {
                wprintf(L"Invalid Choice '%d'. Please enter a choice between 1 and %d: ", userChoice, optionCount);
            }
        }
        while (invalidChoice);
        
        if (sampleList[userChoice - 1].testFunc != NULL)
            sampleList[userChoice - 1].testFunc(rrasServerName);
        else
            exitSample = TRUE;
    }
    ReleaseMprApiLibrary();
    
Done:
    wprintf(L"Mpr API Sample SDK exited\n");
    return status;
}

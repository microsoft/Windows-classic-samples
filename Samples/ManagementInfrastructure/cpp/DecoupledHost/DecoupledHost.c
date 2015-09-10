//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <mi.h>
#include <strsafe.h>
#include <assert.h>

#define STR_NAMESPACE       L"-Namespace"
#define STR_PROVIDERNAME    L"-ProviderName"
#define STR_PROVIDERPATH    L"-ProviderPath"
#define MI_MAIN_FUNCNAME    "MI_Main"

//
// DecoupledHostArgument structure holds all
// requested parameter
//
typedef struct _DecoupledHostArgument
{
    LPWSTR lpNamespace;
    LPWSTR lpProviderName;
    LPWSTR lpProviderPath;
}DecoupledHostArgument;

//
// Shutdown the provider by closing hosted provider,
// closing the application and releasing the library handle.
//
// Argument:
//  pApplication        - MI_Application object
//  pHostedProvider     - The started decoupled host provider object
//  hProviderModule     - The provider DLL handle
//
void ShutdownDecoupledHost(
    MI_Application *pApplication,
    MI_HostedProvider *pHostedProvider,
    HMODULE hProviderModule)
{
    MI_Result result;
    BOOL bFreeLibrary;

    fprintf(stdout, "%s\r\n", "Trying to shutdown decoupled provider.");

    // closing hosted provider
    result = MI_HostedProvider_Close(pHostedProvider);
    fprintf(stdout, "%s %d.\r\n", "MI_HostedProvider_Close returned", result);

    result = MI_Application_Close(pApplication);
    fprintf(stdout, "%s %d.\r\n", "MI_Application_Close returned", result);

    bFreeLibrary = FreeLibrary(hProviderModule);
    fprintf(stdout, "%s %d.\r\n", "Free provider module result = ", bFreeLibrary);
}

//
// Parse the commandline and return the parsing result
// to caller.
//
// Argument:
//  argc        - number of arguments
//  argv        - arguments value array
//  pArgument   - parsed result output through pArgument
//
// Return Value:
//  TRUE means input arguments are valid, otherwise invalid
//  and printf help usage.
//
BOOL ParseArgument(__in int argc,
    __in_ecount(argc) LPCWSTR argv[],
    __out DecoupledHostArgument * pArgument)
{
    int i;
    memset(pArgument, 0, sizeof(DecoupledHostArgument));
    // argv[0] is the application name
    if (argc < 2)
    {
        return FALSE;
    }

    for(i = 1; i < argc; i++)
    {
        LPCWSTR lpCurrArg = argv[i];
        if (_wcsicmp(lpCurrArg, STR_NAMESPACE) == 0)
        {
            // read namespace argument value
            if (i + 1 < argc)
            {
                pArgument->lpNamespace = _wcsdup(argv[++i]);
            }
            else
            {
                return FALSE;
            }
        }
        else if (_wcsicmp(lpCurrArg, STR_PROVIDERNAME) == 0)
        {
            // read providername argument value
            if (i + 1 < argc)
            {
                pArgument->lpProviderName = _wcsdup(argv[++i]);
            }
            else
            {
                return FALSE;
            }
        }
        else if (_wcsicmp(lpCurrArg, STR_PROVIDERPATH) == 0)
        {
            // read providerpath argument value
            if (i + 1 < argc)
            {
                pArgument->lpProviderPath = _wcsdup(argv[++i]);
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}


//
// cleanup the memory of parsed argument
//
// Argument:
//  pArgument   - parsed result output through pArgument
//
void CleanupArgument(__inout DecoupledHostArgument * pArgument)
{
    if (pArgument->lpNamespace)
    {
        free(pArgument->lpNamespace);
    }
    if (pArgument->lpProviderName)
    {
        free(pArgument->lpProviderName);
    }
    if (pArgument->lpProviderPath)
    {
        free(pArgument->lpProviderPath);
    }
    memset(pArgument, 0, sizeof(DecoupledHostArgument));
}

//
// Host the decoupled provider in current process
//
// Argument:
//  pArgument   - argument object used to host provider
//
// Return Value:
//  hosting result
//
int HostDecoupledProvider(__in DecoupledHostArgument * pArgument)
{
    HMODULE hProvider = NULL;
    MI_MainFunction mi_main = NULL;
    MI_Application application = MI_APPLICATION_NULL;
    MI_HostedProvider hostedProvider = MI_HOSTEDPROVIDER_NULL;
    MI_Result result = MI_RESULT_OK;
    
    // load provider DLL
    hProvider = LoadLibraryExW(pArgument->lpProviderPath, NULL, 0);
    if(hProvider == NULL)
    {
        DWORD dwErrorCode = GetLastError();
        fwprintf(stderr, L"%s %s %s %d.\r\n",
            L"Failed to load library",
            pArgument->lpProviderPath,
            L"with error code",
            dwErrorCode);
        return dwErrorCode;
    }

    // query MI_Main function from provider DLL
    mi_main = (MI_MainFunction)GetProcAddress(hProvider, MI_MAIN_FUNCNAME);
    if(mi_main == NULL)
    {
        DWORD dwErrorCode = GetLastError();
        fprintf(stderr, "%s %s %s %d.\r\n",
            "Cannot find procedure",
            MI_MAIN_FUNCNAME,
            "from the provider module with error code",
            dwErrorCode);
        return dwErrorCode;
    }

    // initialize MI_Application object
    result = MI_Application_Initialize(0, NULL, NULL, &application);
    if(result != MI_RESULT_OK)
    {
        fprintf(stderr, "%s %d.\r\n",
            "Failed to initialize MI_Application with error code",
            result);
        return result;
    }

    // host the given provider as decoupled provider
    result = MI_Application_NewHostedProvider(&application,
        pArgument->lpNamespace,
        pArgument->lpProviderName,
        mi_main,
        NULL,
        &hostedProvider);
    if(result != MI_RESULT_OK)
    {
        fprintf(stderr, "%s %d.\r\n",
            "Failed to host decoupled provider with error code",
            result);
        return result;
    }

    // Successfully hosted the provider as a decoupled one,
    // now wait for the exit / quit command
    {
        BOOL quit = FALSE;
        char inputBuffer[MAX_PATH];
        do
        {
            fprintf(stdout, "\r\n%s \r\n",
            "enter 'exit' or 'quit' to terminate the decoupled host process.");
            {
                size_t finalLength = 0;
                while (finalLength == 0)
                {
                    _cgets_s(inputBuffer, sizeof(inputBuffer)/sizeof(char), &finalLength);
                }
                inputBuffer[MAX_PATH -1] = '\0';
                if ((_stricmp(inputBuffer, "exit") == 0) || (_stricmp(inputBuffer, "quit") == 0))
                {
                    // Free all resources
                    ShutdownDecoupledHost(&application, &hostedProvider, hProvider);
                    quit = TRUE;
                }
            }
        }
        while(!quit);
    }
    return 0;
}

//
// Main method for dcuphost.exe
//
// Usage: dcuphost.exe  -Namespace <namespace>
//                      -ProviderName <provider name>
//                      -ProviderPath <provider dll path>
// NOTE:
//  providername and namespace has to be the same value of
//  registering the provider through Register-CimProvider.exe
//
int __cdecl wmain (int argc, _In_count_(argc) LPWSTR argv[])
{
    DecoupledHostArgument argument;
    int result;

    // parse commandline to get namespace, providername, and providerpath
    if (!ParseArgument(argc, argv, &argument))
    {
        fprintf(stderr, "%s\r\n",
            "Usage: DcupHost.exe -Namespace <namespace>"\
            " -ProviderName <provider name> -ProviderPath <provider dll path>");

        CleanupArgument(&argument);
        return ERROR_INVALID_PARAMETER;
    }

    result = HostDecoupledProvider(&argument);

    // clean up the memory
    CleanupArgument(&argument);
    return result;
}

// Copyright (C) Microsoft Corporation.  All rights reserved. 
//
// sample code demonstrates how to retrieve "cmd.exe dir" results 
// by calliing shell client API

#include <windows.h>
#include <iostream>

#include "ShellClient.h"
#include "ParamsParser.h"

using namespace std;

// connection
int __cdecl wmain(int argc, __in_ecount(argc) wchar_t * argv[])
{
    CParamsParser * pParser = new CParamsParser();
    if (NULL == pParser)
    {
        wprintf(L"Could not allocate CParamsParser\n");
        return ERROR_OUTOFMEMORY;
    }
    if (!pParser->ParseCommandLine(argc, argv))
    {
        wprintf(L"ParseCommandLine failed\n");
        delete pParser;
        return (int)-1;
    }

    CShellClient * pShellClient = new CShellClient();
    if (!pShellClient)
    {
        wprintf(L"out of memory");
        delete pParser;
        return ERROR_OUTOFMEMORY;
    }

    pShellClient->Setup(pParser->connection,
                        pParser->authenticationMechanism,
                        pParser->username,
                        pParser->password);
    pShellClient->Execute(pParser->resourceUri,
                          pParser->commandLine,
                          pParser->sendData,
                          pParser->count);
    delete pShellClient;
    delete pParser;

    return ERROR_SUCCESS;
}
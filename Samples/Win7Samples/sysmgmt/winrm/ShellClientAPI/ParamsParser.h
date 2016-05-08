/******************************************************************************
 * <copyright file="ParamsParser.h" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#ifndef _PARAMSPARSER_H_
#define _PARAMSPARSER_H_

#include <windows.h>
#include <iostream>
#include <string.h>
#include <strsafe.h>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>

/*------------------------------------------------------------------------
Implements a class to get input parameters for shell client API calls
  ------------------------------------------------------------------------*/
class CParamsParser
{
public:

    PWSTR connection;
    PWSTR authentication;
    PWSTR username;
    PWSTR password;
    PWSTR resourceUri;
    PWSTR commandLine;
    PSTR sendData;
    PWSTR countStr;

    DWORD authenticationMechanism;
    DWORD count;

    CParamsParser();
    ~CParamsParser();

    bool ParseCommandLine(int argc, __in_ecount(argc) wchar_t * argv[]);
    static void PrintUsage(PCWSTR program);

private:


};

#endif
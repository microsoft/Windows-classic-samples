/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiModule.cpp

Abstract:

    Module functions for ISAPI tools library

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>

//
// Implement the project global module name
//

CHAR    g_szModuleName[MAX_PATH+1] = "*UnknownIsapi*";

VOID
InitializeIsapiTools(
    CHAR *  szModuleName
    )
/*++

Purpose:

    Initializes IsapiTools so that time function work
    correctly, and so that debug output contains the
    module name.

Arguments:

    szModuleName - The name of the dll (ie. "MyIsapi", without ".dll")

Returns:

    None

--*/
{
    //
    // Set the module name
    //

    if ( szModuleName != NULL )
    {
        strncpy_s( g_szModuleName, sizeof(g_szModuleName), szModuleName, MAX_PATH );

        g_szModuleName[MAX_PATH] = '\0';
    }

    //
    // Initialize the time tools
    //

    InitializeIsapiTime();

    return;
}
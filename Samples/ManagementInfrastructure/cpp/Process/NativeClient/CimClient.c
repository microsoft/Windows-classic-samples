//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//


#include <windows.h>
#include <strsafe.h>
#include <mi.h>
#include "utilities.h"
#include "operations.h"

int __cdecl wmain(int argc, __in_ecount(argc) wchar_t* argv[])
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Application miApplication = MI_APPLICATION_NULL;
    wchar_t selection;
    wchar_t protocolSelection;
    wchar_t _machineName[MAX_PATH];
    wchar_t *machineName = NULL;
    wchar_t namespaceName[MAX_PATH];
    wchar_t className[MAX_PATH];
    wchar_t *protocol = NULL;

    MI_UNREFERENCED_PARAMETER(argc);
    MI_UNREFERENCED_PARAMETER(argv);

    /* It is recommended to only have one MI_Application per process, although
     * multiple are allowed.  Some caching takes place within an application.
     * An MI_Application always needs to be closed via MI_Application_Close().
     */
    miResult = MI_Application_Initialize(0, NULL, NULL, &miApplication);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Application_Initialize failed, error = %s\n", MI_Result_To_String(miResult));
        return -1;
    }

    do
    {
        /* Helper API to retrieve the user selection */
        selection = GetUserSelection(
                    L"Do you want local operation, or remote machine?\n"
                    L"\t[1] Local machine\n"
                    L"\t[2] Remote machine\n"
                    L"\t[0] Quit sample application\n",
                    L"012");
        if (selection != L'0')
        {
            if (selection == L'2')
            {
                /* If remote is selected then the default remoting protocol to use is WS-Man */
                GetUserInputString(L"Enter computer name", _machineName, sizeof(_machineName)/sizeof(_machineName[0]), L"localhost");
                machineName = _machineName;
            }
            else if (selection == L'1')
            {
                /* If local is selected then the default local protocol to use is WMI-DCOM */
                machineName = NULL;
            }

            /* Override default protocol, where local is WMI DCOM and remote is WS-Man */
            protocolSelection = GetUserSelection(
                        L"Do you want to override the default protocol?\n"
                        L"\t[1] Override with WMI DCOM\n"
                        L"\t[2] Override with WS-Management\n"
                        L"\t[0] Use default\n",
                        L"012");
            if (protocolSelection == L'1')
            {
                protocol = L"WMIDCOM";
            }
            else if (selection == L'2')
            {
                /* WINRM is the Microsoft implementation of the WS-Man protocol */
                protocol = L"WINRM";
            }
            else
            {
                /* This will use the default based on the computer name */
                protocol = NULL;
            }

            /* Retrieve the namespace that all operations will be run against.  */
            GetUserInputString(L"Enter CIM namespace", namespaceName, sizeof(namespaceName)/sizeof(namespaceName[0]), L"root/standardcimv2/samples");

            /* Retrieve the class name that all operations will be run on.  Note that a couple 
             * of operations will not actually use this, namely enumerations and subscriptions.
             */
            GetUserInputString(L"Enter CIM class name", className, sizeof(className)/sizeof(className[0]), L"MSFT_WindowsProcess");

            /* This is where we make a choice of which operation to carry out */
            Do_Operation(&miApplication, machineName, protocol, namespaceName, className);
        }

    } while (selection != L'0');

    /* MI_Application_Close() will block until all operations and sessions are fully closed.
     */
    miResult = MI_Application_Close(&miApplication);
    if (miResult != MI_RESULT_OK)
    {
        /* Most common failures are invalid parameter or out of memory.  
         * Invalid parameter is likely to be a programming error.
         * Out of memory (unlikely, but possible) will cause things to be shut down as best
         * it can be.  The API should not be called again in this case though.
         */
        wprintf(L"MI_Application_Close failed, error = %s\n", MI_Result_To_String(miResult));
    }

    return miResult;
}

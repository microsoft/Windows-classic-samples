// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*

    File Replication Sample
    Server System Service

    FILE: FileRepServer.cpp
    
    PURPOSE: Provides file replication services.
    
    USAGE: 
        FileRepService

    FUNCTIONS:
        main() - calls the StartServiceCtrlDispatcher function to
            connect to the SCM and start the control dispatcher thread.
        ServiceMain() - the entry point for the service.
        ServiceCtrl() - this function is called by the dispatcher thread.
            It handles the control code passed to it.
        ServiceStart() - performs actual initialization and starts the service.
            Makes the RPC server listen for calls.

    COMMENTS:

*/

// Common definitions
#include "common.h"

#define SECURITY_WIN32

#include <process.h>
#include <tchar.h>
#include <rpc.h>
#include <Ntdsapi.h>
#include <Ntsecapi.h>
#include <Security.h>
#include <Secext.h>
#include <Dsgetdc.h>
#include <Lm.h>

// Contains declarations for system service functions.
#include "Service.h"

#ifdef PROF
#include "Prof.h"
#endif

#ifdef DEBUG1
#include "DbgMsg.h"
#endif

/*
    FUNCTION: main()

    PURPOSE: main() calls StartServiceCtrlDispatcher to register the
        main service thread.    When this call returns,
        the service has stopped.

    PARAMETERS:
        dwArgc - number of command line arguments
        lpszArgv - array of command line arguments

    RETURN VALUE:
        none

    COMMENTS:
*/
VOID _cdecl main(int argc, char **argv){
    RPC_STATUS status;

    int nNumArgs;
    LPWSTR *szArgList = CommandLineToArgvW(GetCommandLine(), &nNumArgs);

    if (NULL == szArgList) {
        _tprintf(TEXT("FileRep main: CommandLineToArgW failed"));
        exit(EXIT_FAILURE);
    }

    // Allow the user to override settings with command line switches.
    for (int i = 1; i < nNumArgs; i++) {
        // Well-formed argument switches start with '/' or '-' and are
        // two characters long.
        if (((*szArgList[i] == TEXT('-')) || (*szArgList[i] == TEXT('/'))) && _tcsclen(szArgList[i]) == 2) {

            switch (_totlower(*(szArgList[i]+1))) {

                case TEXT('f'):
		  bNoFileIO = true;
		  break;
                    
                case TEXT('h'):
                case TEXT('?'):
                default:
                    exit(EXIT_SUCCESS);
            }
        }
        else {
	  _tprintf(TEXT("Bad arguments.\n\n"));
	  exit(EXIT_FAILURE);
        }
    }

    // Service initialization
    if (!StartFileRepServer()) {
      return;
    } else {
      bServerListening = TRUE;

      // RpcMgmtWaitServerListen() will block until the server has
      // stopped listening.
      status = RpcMgmtWaitServerListen();
      if (status != RPC_S_OK){
        AddToMessageLogProcFailureEEInfo(TEXT("ServiceStart: RpcMgmtWaitServerListen"), status);
        return;
      }
    }
    
    return;
}

// end FileRepServer.cpp

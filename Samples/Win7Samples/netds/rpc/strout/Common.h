// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*														                */
/*                      Microsoft RPC strout sample                    */
/*                                                                      */
/*  FILE    :   common.h                                                */
/*                                                                      */
/*  PURPOSE :   Definitions used in the program                         */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef __COMMON_STROUT__   // If this file has be included before      
#define __COMMON_STROUT__   // Don't include it again                   

#include <windows.h>        // The CommandLineToArgvW is defined here   
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>          // Included to support UNICODE/ANSI         

// Return values from the program to the OS
#define EXECUTION_FAILED    -1
#define EXECUTION_OK        0

// Exception handler macro
#define DO_EXCEPTION        1       // Execute the exception block

// Macro for printing out error message and exit the program if an      
// error occured                                                        
#define EXIT_IF_FAIL(x, string){	\
    ((x) != RPC_S_OK)?_tprintf(TEXT("%s returned with error: %d\n"), \
    TEXT(string), (x)),exit(x):0;} 

// Common type definition and defines used in the program
#define PROTOCOL_SEQUENCE   TEXT("ncacn_ip_tcp")
#define END_POINT           TEXT("8765")
#define	IGNORE_CHAR         TEXT('=')
#define NULL_CHAR           TEXT('\0')

#endif  // ifndef __COMMON_STROUT__ 




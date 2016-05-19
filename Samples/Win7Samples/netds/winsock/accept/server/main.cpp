// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: main.cpp
//
// Description:
//             This file contains the functions for parsing the command-line
// arguments and the main function.

#include "common.h"

// g_AcceptContext will hold all the global variables used across all
// the files.
AcceptContext g_AcceptContext;


/*
    This function converts a given address family into its corresponding string
    representation for display purposes.
*/
const char *AFImage(BYTE addressFamily)
{
    char *szRetVal;

    // return the printable string equivalent of the corresponding 
    // address family.
    switch (addressFamily)
    {
        case AF_UNSPEC : szRetVal = "AF_UNSPEC";
                         break;
        case AF_INET   : szRetVal = "AF_INET";
                         break;
        case AF_INET6  : szRetVal = "AF_INET6";
                         break;
        default        : szRetVal = "Unrecognized";
                         break;
    }
    
    return szRetVal;
}



/*
    This function converts a given type of accept into its corresponding string
    representation for display purposes.
*/
const char *AcceptTypeImage(BYTE typeOfAccept)
{
    char *szRetVal;

    // return a string equivalent of the accept type for displaying
    // to the user of his choices.
    switch (typeOfAccept)
    {
        case NON_BLOCKING_ACCEPT : szRetVal = "NON_BLOCKING_ACCEPT";
                                   break;
        case ASYNC_SELECT_ACCEPT : szRetVal = "ASYNC_SELECT_ACCEPT";
                                   break;
        default                  : szRetVal = "Unrecognized";
                                   break;
    }
    
    return szRetVal;
}



/*
    This function prints the available command-line options, the arguments
    expected by each of them and the valid input values and the default 
    values for each them.
*/
void PrintUsage(char *szProgramName)
{
    printf("\n\n"
           "Usage:\n"
           "------\n"
           "   %s <options> \n\n"
           "where <options> is one or more of the following: \n\n"
           "   -a <0|4|6>      Address Family: 0 for Either\n"
           "                                   4 for IPv4\n"
           "                                   6 for IPv6\n"
           "                   Default: %d\n\n"
           "   -i <interface>  Interface address\n"
           "                   Default: %s\n\n"
           "   -e <endpoint>   Port number\n"
           "                   Default: %s\n\n"
           "   -t <1|2>        Type of Accept: 1 for Non-blocking accept\n"
           "                                   2 for Accept with WSAAsyncSelect\n"
           "                   Default: %d\n\n"
           "\n",
           szProgramName,
           DEFAULT_ADDRESS_FAMILY,
          ( DEFAULT_INTERFACE == NULL ? "NULL" : DEFAULT_INTERFACE),
           DEFAULT_PORT,
           DEFAULT_TYPE_OF_ACCEPT
           );

    return;
}

/*
    This function parses the given input arguments and fills up the
    corresponding fields in the g_AcceptContext structure.
*/
BOOL ParseArguments(int argc, char *argv[])
{
    // holds the return value from this function.
    // TRUE indicates that all the supplied arguments are valid. 
    // FALSE indicates incorrect or insufficient number of arguments.
    BOOL retVal = FALSE;

    // loop index to go over the command-line arguments one by one.
    int i;
    
    printf("Entering ParseArguments()\n");

    // fill up the default arguments and let the user options override these.
    g_AcceptContext.addressFamily = DEFAULT_ADDRESS_FAMILY;
    g_AcceptContext.szInterface = DEFAULT_INTERFACE;
    g_AcceptContext.szPort = DEFAULT_PORT;
    g_AcceptContext.typeOfAccept = DEFAULT_TYPE_OF_ACCEPT;
    

    // process each argument in the argv list.
    for (i = 1; i < argc ; i++)
    {
        char firstChar = argv[i][0];

        // make sure the option begins with a hyphen or a forward slash.
        if (!(firstChar == '-' || firstChar == '/'))
        {
            printf("ERROR: Option has to begin with - or / : %s\n", argv[i]);
            PrintUsage(argv[0]);
            goto CLEANUP;
        }

        // process the option.
        switch(argv[i][1])
        {
            case 'a' :
            
                // Address Family. 
                // should be -a 0 or -a 4 or -a 6
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Argument 0/4/6 needed for -a option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract and validate the AF number.
                switch(atoi(argv[i+1]))
                {
                    // Unspecified. 
                    case 0:
                      g_AcceptContext.addressFamily = AF_UNSPEC;
                      break;

                    // IPv4.
                    case 4:
                      g_AcceptContext.addressFamily = AF_INET;
                      break;                      

                    // IPv6.
                    case 6:
                      g_AcceptContext.addressFamily = AF_INET6;
                      break;                      

                    // Invalid value.
                    default:
                      printf("ERROR: Invalid address family. Must be 0/4/6\n");
                      PrintUsage(argv[0]);
                      goto CLEANUP;
                }

                // indicate that we have processed the next argument as well.
                i++; 

                // AF was fine. continue.
                break;

            case 'i' :

                // Interface to listen on.
                // should be -i <interface>
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Interface needed for -i option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // make sure the input string length is less than
                // the INET6_ADDRSTRLEN, the maximum valid IP address length.
                if (FAILED(StringCchLength(argv[i+1],INET6_ADDRSTRLEN, NULL)))
                {
                    printf("ERROR: Interface string too long. "
                           "can't exceed %d characters\n", 
                           INET6_ADDRSTRLEN);
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // remember the interface string.
                g_AcceptContext.szInterface = argv[i+1];

                // indicate that we have processed the next argument as well.
                i++; 

                // continue.            
                break;

            case 'e' :
            
                // Endpoint or Port.
                // should be -e <port number>
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Port number needed for -e option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // make sure the input string length is less than
                // the maximum length for a service name.
                if (FAILED(StringCchLength(argv[i+1], NI_MAXSERV, NULL)))
                {
                    printf("ERROR: Port number too long. "
                           "can't exceed %d characters\n", 
                           NI_MAXSERV);

                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // remember the port number string.
                g_AcceptContext.szPort = argv[i+1];

                // indicate that we have processed the next argument as well.
                i++; 

                // continue.            
                break;
                
            case 't' :
           
                // Type of Accept.
                // should be -t 1 or -t 2
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Argument 1 or 2 needed for -t option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract the type value
                g_AcceptContext.typeOfAccept = (BYTE) atoi(argv[i+1]);

                // indicate that we have processed the next argument as well.
                i++; 

                // validate the accept type.
                if (!(g_AcceptContext.typeOfAccept == 1 ||
                      g_AcceptContext.typeOfAccept == 2))
                {
                    printf("ERROR: Invalid accept type: %d. Must be 1 or 2\n",
                            g_AcceptContext.addressFamily);
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // Accept type was fine. continue.
                break;
                
            case 'h' : // help
            case '?' : // help
                        PrintUsage(argv[0]);
                        goto CLEANUP;                        
            default  : 
                        printf("ERROR: Unrecognized option: %s\n", argv[i]);
                        PrintUsage(argv[0]);
                        goto CLEANUP;                        
        }
    }

    // echo the final list of values that'll be used.
    // remember, these need not be the same as the input arguments.
    // rather, this is what we'll use inside our program.
    printf("Parsed input arguments. The following values will be used : \n");
    printf("\tAddress Family = %s\n", AFImage(g_AcceptContext.addressFamily));
    printf("\tInterface = %s\n",g_AcceptContext.szInterface);
    printf("\tPort = %s\n", g_AcceptContext.szPort);
    printf("\tType of Accept = %s\n", 
                            AcceptTypeImage(g_AcceptContext.typeOfAccept));  

    // all went well, signal that we can proceed.
    retVal = TRUE;

CLEANUP:    

    printf("Exiting ParseArguments()\n");
    return retVal;
}


/*
    This function is the entry point for this program.
    Based on the command-line arguments, it invokes the suitable functions.
*/
int _cdecl main(int argc, char *argv[])
{
    // holds the return value from this function.
    // 0 indicates success, non-zero indicates failure.
    int retVal;

    WSADATA wsaData;

    printf("Entering main()\n");

    // parse and validate the given arguments and determine if we should
    // continue the execution or return error.
    if (ParseArguments(argc, argv) == FALSE)
    {
        // error input. return a non-zero error code.
        retVal = 1;
        goto CLEANUP;
    }

    // call WSAStartup before calling any of the Winsock API functions.
    retVal = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retVal != 0)
    {
        printf("WSAStartup failed. Error = %d\n", retVal);
        goto CLEANUP;        
    }

    // Depending on the command-line options given, create one or more
    // listening sockets on the requested interface(s).
    CreateListeningSockets();

    // Depending on the type of accept requested, call the suitable function.
    switch(g_AcceptContext.typeOfAccept)
    {
        case NON_BLOCKING_ACCEPT:
            NonBlockingAcceptMain();
            break;

        case ASYNC_SELECT_ACCEPT:
            AsyncSelectAcceptMain();
            break;

        default:
            // some error. return a non-zero error code.
            retVal = 1;
            break;
    }

    // we may not come here as per the current implementation since
    // the XXXAcceptMain functions themselves are waiting forever for
    // connections or data. But in case we add a timeout option in future
    // we might come here and so we'll cleanup everything properly.

    // Close all the listening sockets and remove them from the global
    // list. In case there are some accepted sockets still in the list,
    // (due to some error), they'll also be closed as well.
    DestroyListeningSockets();

    // Inform Winsock that we're done with all the Winsock APIs.
    WSACleanup();
  
CLEANUP:

    printf("Exiting main()\n");
    return retVal;
}



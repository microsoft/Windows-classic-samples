/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright 1991-2000  Microsoft Corporation

Module Name:

    snmputil.c

Abstract:

    Sample SNMP Management API usage for Windows NT.

    This file is an example of how to code management applications using
    the SNMP Management API for Windows NT.  It is similar in operation to
    the other commonly available SNMP command line utilities.

    Extensive comments have been included to describe its structure and
    operation.  See also "Microsoft Windows/NT SNMP Programmer's Reference".

Created:

    28-Jun-1991

Revision History:


--*/


// General notes:
//   Microsoft's SNMP Management API for Windows NT is implemented as a DLL
// that is linked with the developer's code.  These APIs (examples follow in
// this file) allow the developer's code to generate SNMP queries and receive
// SNMP traps.  A simple MIB compiler and related APIs are also available to
// allow conversions between OBJECT IDENTIFIERS and OBJECT DESCRIPTORS.


// Necessary includes.

#include <windows.h>

#include <stdio.h>
#include <string.h>

#include <snmp.h>
#include <mgmtapi.h>
#include <strsafe.h>

// Constants used in this example.

#define GET     1
#define GETNEXT 2
#define WALK    3
#define TRAP    4

#define TIMEOUT 6000 /* milliseconds */
#define RETRIES 3


// Main program.
INT _CRTAPI1 main(
    IN int  argumentCount,
    IN char *argumentVector[])
    {
    INT                operation;
    LPSTR              agent = NULL;
    LPSTR              community = NULL;
    RFC1157VarBindList variableBindings;
    LPSNMP_MGR_SESSION session = NULL;

    INT        timeout = TIMEOUT;
    INT        retries = RETRIES;

    BYTE       requestType;
    AsnInteger errorStatus;
    AsnInteger errorIndex;


    // Parse command line arguments to determine requested operation.

    // Verify number of arguments...
    if      (argumentCount < 5 && argumentCount != 2)
        {
        printf("Error:  Incorrect number of arguments specified.\n");
        printf(
"\nusage:  snmputil [get|getnext|walk] agent community oid [oid ...]\n");
        printf(
  "        snmputil trap\n");

        return 1;
        }

    // Get/verify operation...
    argumentVector++;
    argumentCount--;
    if      (!strcmp(*argumentVector, "get"))
        operation = GET;
    else if (!strcmp(*argumentVector, "getnext"))
        operation = GETNEXT;
    else if (!strcmp(*argumentVector, "walk"))
        operation = WALK;
    else if (!strcmp(*argumentVector, "trap"))
        operation = TRAP;
    else
        {
        printf("Error:  Invalid operation, '%s', specified.\n",
               *argumentVector);

        return 1;
        }

    if (operation != TRAP)
        {
        if (argumentCount < 4)
            {
            printf("Error:  Incorrect number of arguments specified.\n");
            printf(
"\nusage:  snmputil [get|getnext|walk] agent community oid [oid ...]\n");
            printf(
  "        snmputil trap\n");

            return 1;
            }

        // Get agent address...
        argumentVector++;
        argumentCount--;
        agent = (LPSTR)SnmpUtilMemAlloc((UINT)(strlen(*argumentVector) + 1));
        if (agent != NULL)
            StringCchCopyA(agent, strlen(*argumentVector) + 1, *argumentVector);
        else
            {
            printf("Error: SnmpUtilMemAlloc failed to allocate memory.\n");
            return 1;
            }

        // Get agent community...
        argumentVector++;
        argumentCount--;
        community = (LPSTR)SnmpUtilMemAlloc((UINT)(strlen(*argumentVector) + 1));
        if (community != NULL)
            StringCchCopyA(community, strlen(*argumentVector) + 1, *argumentVector);
        else
            {
            printf("Error: SnmpUtilMemAlloc failed to allocate memory.\n");
            SnmpUtilMemFree(agent);
            return 1;
            }

        // Get oid's...
        variableBindings.list = NULL;
        variableBindings.len = 0;

        while(--argumentCount)
            {
            AsnObjectIdentifier reqObject;
            RFC1157VarBind * tmpVb;

            argumentVector++;

            // Convert the string representation to an internal representation.
            if (!SnmpMgrStrToOid(*argumentVector, &reqObject))
                {
                printf("Error: Invalid oid, %s, specified.\n", *argumentVector);
                SnmpUtilMemFree(agent);
                SnmpUtilMemFree(community);
                SnmpUtilVarBindListFree(&variableBindings);
                return 1;
                }
            else
                {
                // Since sucessfull, add to the variable bindings list.
                variableBindings.len++;
                if ((tmpVb = (RFC1157VarBind *)SnmpUtilMemReAlloc(
                    variableBindings.list, sizeof(RFC1157VarBind) *
                    variableBindings.len)) == NULL)
                    {
                    printf("Error: Error allocating oid, %s.\n",
                           *argumentVector);
                    SnmpUtilMemFree(agent);
                    SnmpUtilMemFree(community);
                    SnmpUtilOidFree(&reqObject);
                    variableBindings.len--;
                    SnmpUtilVarBindListFree(&variableBindings);

                    return 1;
                    }
                variableBindings.list = tmpVb;

                variableBindings.list[variableBindings.len - 1].name =
                    reqObject; // NOTE!  structure copy
                variableBindings.list[variableBindings.len - 1].value.asnType =
                    ASN_NULL;
                }
            } // end while()

        // Make sure only one variable binding was specified if operation
        // is WALK.
        if (operation == WALK && variableBindings.len != 1)
            {
            printf("Error: Multiple oids specified for WALK.\n");
            SnmpUtilMemFree(agent);
            SnmpUtilMemFree(community);
            SnmpUtilVarBindListFree(&variableBindings);
            return 1;
            }

        // Establish a SNMP session to communicate with the remote agent.  The
        // community, communications timeout, and communications retry count
        // for the session are also required.

        if ((session = SnmpMgrOpen(agent, community, timeout, retries)) == NULL)
            {
            printf("error on SnmpMgrOpen %d\n", GetLastError());
            SnmpUtilMemFree(agent);
            SnmpUtilMemFree(community);
            SnmpUtilVarBindListFree(&variableBindings);

            return 1;
            }

        } // end if(TRAP)


    // Determine and perform the requested operation.

    if      (operation == GET || operation == GETNEXT)
        {
        // Get and GetNext are relatively simple operations to perform.
        // Simply initiate the request and process the result and/or
        // possible error conditions.


        if (operation == GET)
            requestType = ASN_RFC1157_GETREQUEST;
        else
            requestType = ASN_RFC1157_GETNEXTREQUEST;


        // Request that the API carry out the desired operation.

        if (!SnmpMgrRequest(session, requestType, &variableBindings,
                            &errorStatus, &errorIndex))
            {
            // The API is indicating an error.

            printf("error on SnmpMgrRequest %d\n", GetLastError());
            }
        else
            {
            // The API succeeded, errors may be indicated from the remote
            // agent.

            if (errorStatus > 0)
                {
                printf("Error: errorStatus=%d, errorIndex=%d\n",
                       errorStatus, errorIndex);
                }
            else
                {
                // Display the resulting variable bindings.

                UINT i;
                
                for(i=0; i < variableBindings.len; i++)
                    {
                    char *string = NULL;
                    
                    if (SnmpMgrOidToStr(&variableBindings.list[i].name, &string))
                        {
                        printf("Variable = %s\n", string);
                        if (string) SnmpUtilMemFree(string);
                        }
                    printf("Value    = ");
                    SnmpUtilPrintAsnAny(&variableBindings.list[i].value);

                    printf("\n");
                    } // end for()
                }
            }

        // Free the variable bindings that have been allocated.
        
        SnmpUtilVarBindListFree(&variableBindings);
        
        // Free other allocated resources 

        SnmpUtilMemFree(agent);
        SnmpUtilMemFree(community);


        }
    else if (operation == WALK)
        {
        // Walk is a common term used to indicate that all MIB variables
        // under a given OID are to be traversed and displayed.  This is
        // a more complex operation requiring tests and looping in addition
        // to the steps for get/getnext above.
        
        
        AsnObjectIdentifier root;
        AsnObjectIdentifier tempOid;
        
        
        if (!SnmpUtilOidCpy(&root, &variableBindings.list[0].name))
            {
            printf("error on SnmpUtilOidCpy\n");
            SnmpUtilMemFree(agent);
            SnmpUtilMemFree(community);
            SnmpUtilVarBindListFree(&variableBindings);
            return 1;
            }
        
        requestType = ASN_RFC1157_GETNEXTREQUEST;
        
        
        while(1)
            {
            if (!SnmpMgrRequest(session, requestType, &variableBindings,
                &errorStatus, &errorIndex))
                {
                // The API is indicating an error.
                
                printf("error on SnmpMgrRequest %d\n", GetLastError());
                
                break;
                }
            else
                {
                // The API succeeded, errors may be indicated from the remote
                // agent.
                
                
                // Test for end of subtree or end of MIB.
                
                if (errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ||
                    SnmpUtilOidNCmp(&variableBindings.list[0].name,
                    &root, root.idLength))
                    {
                    printf("End of MIB subtree.\n\n");
                    
                    break;
                    }
                
                
                // Test for general error conditions or sucesss.
                
                if (errorStatus > 0)
                    {
                    printf("Error: errorStatus=%d, errorIndex=%d \n",
                        errorStatus, errorIndex);
                    
                    break;
                    }
                else
                    {
                    // Display resulting variable binding for this iteration.
                    
                    char *string = NULL;
                    
                    if (SnmpMgrOidToStr(&variableBindings.list[0].name, &string))
                        {
                        printf("Variable = %s\n", string);
                        if (string) SnmpUtilMemFree(string);
                        }
                    printf("Value    = ");
                    SnmpUtilPrintAsnAny(&variableBindings.list[0].value);
                    
                    printf("\n");
                    }
                } // end if()
            
            
            // Prepare for the next iteration.  Make sure returned oid is
            // preserved and the returned value is freed.
            
            if (!SnmpUtilOidCpy(&tempOid, &variableBindings.list[0].name))
                {
                printf("error on SnmpUtilOidCpy\n");
                SnmpUtilOidFree(&root);
                SnmpUtilMemFree(agent);
                SnmpUtilMemFree(community);
                SnmpUtilVarBindListFree(&variableBindings);
                return 1;
                }
            
            SnmpUtilVarBindFree(&variableBindings.list[0]);
            
            if (!SnmpUtilOidCpy(&variableBindings.list[0].name, &tempOid))
                {
                printf("error on SnmpUtilOidCpy\n");
                SnmpUtilOidFree(&tempOid);
                SnmpUtilOidFree(&root);
                SnmpUtilMemFree(agent);
                SnmpUtilMemFree(community);
                SnmpUtilVarBindListFree(&variableBindings);
                return 1;
                }
            variableBindings.list[0].value.asnType = ASN_NULL;
            
            SnmpUtilOidFree(&tempOid);
            
        } // end while()
        
        
        // Free the variable bindings that have been allocated.
        
        SnmpUtilVarBindListFree(&variableBindings);
        
        // Free other allocated resources

        SnmpUtilOidFree(&root);
        SnmpUtilMemFree(agent);
        SnmpUtilMemFree(community);
        
        
    }
    else if (operation == TRAP)
        {
        // Trap handling can be done two different ways: event driven or
        // polled.  The following code illustrates the steps to use event
        // driven trap reception in a management application.


        HANDLE hNewTraps = NULL;


        if (!SnmpMgrTrapListen(&hNewTraps))
            {
            printf("error on SnmpMgrTrapListen %d\n", GetLastError());
            return 1;
            }
        else
            {
            printf("snmputil: listening for traps...\n");
            }


        while(1)
            {
            DWORD dwResult;

            if ((dwResult = WaitForSingleObject(hNewTraps, INFINITE))
                == WAIT_FAILED)
                {
                printf("error on WaitForSingleObject %d\n",
                       GetLastError());
                }
            else if (dwResult != WAIT_OBJECT_0)
                {
                    printf("hNewTraps is not signaled\n");
                }
            else if (!ResetEvent(hNewTraps))
                {
                printf("error on ResetEvent %d\n", GetLastError());
                }
            else
                {
                AsnObjectIdentifier enterprise;
                AsnNetworkAddress   agentAddress;
                AsnNetworkAddress   sourceAddress;
                AsnInteger          genericTrap;
                AsnInteger          specificTrap;
                AsnOctetString      communityTrap;
                AsnTimeticks        timeStamp;
                RFC1157VarBindList  variableBindingsTrap;

                UINT i;
                
                while(SnmpMgrGetTrapEx(
                        &enterprise,
                        &agentAddress,
                        &sourceAddress,
                        &genericTrap,
                        &specificTrap,
                        &communityTrap,
                        &timeStamp,
                        &variableBindingsTrap))
                    {

                    LPSTR string = NULL;

                    printf("Incoming Trap:\n"
                           "  generic    = %d\n"
                           "  specific   = %d\n"
                           "  timeStamp  = %u\n",
                           genericTrap,
                           specificTrap,
                           timeStamp);

                    if (SnmpMgrOidToStr(&enterprise, &string))
                        {
                        printf ("  enterprise = %s\n", string);
                        if (string) 
                            SnmpUtilMemFree(string);
                        }
                    SnmpUtilOidFree(&enterprise);

                    if (agentAddress.length == 4) 
                        {
                        printf ("  agent      = %d.%d.%d.%d\n",
                             (int)agentAddress.stream[0],
                             (int)agentAddress.stream[1],
                             (int)agentAddress.stream[2],
                             (int)agentAddress.stream[3]);
                        }
                    if (agentAddress.dynamic) 
                        {
                        SnmpUtilMemFree(agentAddress.stream);
                        }

                    if (sourceAddress.length == 4) 
                        {
                        printf ("  source IP  = %d.%d.%d.%d\n",
                             (int)sourceAddress.stream[0],
                             (int)sourceAddress.stream[1],
                             (int)sourceAddress.stream[2],
                             (int)sourceAddress.stream[3]);
                        }
                    else if (sourceAddress.length == 10) 
                        {
                        printf ("  source IPX = %.2x%.2x%.2x%.2x."
                                "%.2x%.2x%.2x%.2x%.2x%.2x\n",
                             (int)sourceAddress.stream[0],
                             (int)sourceAddress.stream[1],
                             (int)sourceAddress.stream[2],
                             (int)sourceAddress.stream[3],
                             (int)sourceAddress.stream[4],
                             (int)sourceAddress.stream[5],
                             (int)sourceAddress.stream[6],
                             (int)sourceAddress.stream[7],
                             (int)sourceAddress.stream[8],
                             (int)sourceAddress.stream[9]);
                        }
                    if (sourceAddress.dynamic) 
                        {
                        SnmpUtilMemFree(sourceAddress.stream);
                        }

                    if (communityTrap.length)
                        {
                        string = (LPSTR)SnmpUtilMemAlloc (communityTrap.length + 1);
                        if (string)
                            {
                            memcpy (string, communityTrap.stream, communityTrap.length);
                            string[communityTrap.length] = '\0';
                            printf ("  community  = %s\n", string);
                            SnmpUtilMemFree(string);
                            }
                        }
                    if (communityTrap.dynamic) 
                        {
                        SnmpUtilMemFree(communityTrap.stream);
                        }

                    for(i=0; i < variableBindingsTrap.len; i++)
                        {

                        string = NULL;
                        if (SnmpMgrOidToStr(&variableBindingsTrap.list[i].name, &string))
                            {
                            printf("  variable   = %s\n", string);
                            if (string) SnmpUtilMemFree(string);
                            }

                        printf("  value      = ");
                        SnmpUtilPrintAsnAny(&variableBindingsTrap.list[i].value);
                        } // end for()
                    printf("\n");


                    SnmpUtilVarBindListFree(&variableBindingsTrap);
                    }

                dwResult = GetLastError(); // check for errors...

                if ((dwResult != NOERROR) && (dwResult != SNMP_MGMTAPI_NOTRAPS))
                    {
                    printf("error on SnmpMgrGetTrap %d\n", dwResult);
                    }
                }

            } // end while()


        } // end if(operation)

    if (operation != TRAP)
        {
        // Close SNMP session with the remote agent.

        if (!SnmpMgrClose(session))
            {
            printf("error on SnmpMgrClose %d\n", GetLastError());

            return 1;
            }
        }


    // Let the command interpreter know things went ok.

    return 0;

    } // end main()
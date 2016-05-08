/********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for working with contacts using
    the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include <p2p.h>
#include <stdio.h>
#include <strsafe.h>
#include "SignIn.h"
#include "EndpointInfo.h"
#include "Presence.h"
#include "pnm.h"
#include "contacts.h"
#include "Publication.h"
#include "invite.h"
#include "Events.h"


//-----------------------------------------------------------------------------
// Function:    PrintMenu
// Purpose:     Prints a menu of options to the user
// Parameters:  None
//
void PrintMenu()
{
    wprintf(L"\n"
           L"Sign In and Endpoint Info                 Presence\n"
           L"  1.  Sign In                               16. Get presence information\n"
           L"  2.  Sign Out                              17. Set presence information\n"                         
           L"  3.  Display Sign In Options\n"
           L"  4.  Set our endpoint name               Publication\n"
           L"  5.  Get our endpoint name                 18. Subscribe to a PNM endpoint\n"
           L"  6.  Display endpoint info                 19. Unsubscribe from a PNM endpoint\n"
           L"                                            20. Publish an object\n"
           L"People Near Me                              21. Delete a published object\n"
           L"  7.  Enumerate PNM\n"                  
           L"  8.  Add a PNM endpoint to contacts      Invitations\n"
           L"                                            22. Register this Application\n"
           L"Contacts                                    23. Unregister this Application\n"
           L"  9.  Export \"Me\" Contact                   24. Display Registered Applications\n"
           L"  10. Parse Contact                         25. Send Invitation\n"
           L"  11. Import Contact\n"
           L"  12. Delete Contact\n"
           L"  13. Display Contacts\n"
           L"  14. Set contact watch state\n"
           L"  15. Set watch permissions\n"
           L"\n"
           L"  Q. Quit\n"
           L"> ");


}

//-----------------------------------------------------------------------------
// Function:    PrintMenu
// Purpose:     Prints a menu, retrieves the user selection, calls the handler, loops.
//              Returns when "Quit" is selected from the menu
// Parameters:  None
//
void RunMenu()
{
    HRESULT hr;
    WCHAR wzBuff[INPUT_BUFSIZE];
    int nInput;

    // Continuously show the menu to user and respond to their request
    //
    while (TRUE)
    {
        PrintMenu();

        fflush(stdin);
        hr = StringCbGets(wzBuff, sizeof(wzBuff));

        if (FAILED(hr))
        {
            wprintf (L"Invalid input, expected a number between 1 and 25 or Q to quit.\n");
            continue;
        }

        if  ((wcsncmp(wzBuff, L"Q", INPUT_BUFSIZE) == 0) || (wcsncmp(wzBuff, L"q", INPUT_BUFSIZE) == 0))
        {
            // break out of while loop.
            //
            break;
        }

        // since wtoi can't distinguish between 0 and an error, note that valid values start at 1, not 0    
        nInput = _wtoi(wzBuff);
        if (nInput < 1  || nInput > 25)
        {
            printf ("Invalid input, expected a number between 1 and 25 or Q to quit.\n");
            continue;
        }

        switch (nInput)
        {
            case 1:
                SignIn();
                break;

            case 2:
                SignOut();
                break;

            case 3:
                SignInOptions();
                break;

            case 4:
                SetEndpointName();
                break;

            case 5:
                GetEndpointName();
                break;

            case 6:
                DisplayEndpointInformation();
                break;

            case 7:
                EnumeratePeopleNearMe();
                break;

            case 8:
                AddEndpointAsContact();
                break;

            case 9:
                ExportContact();
                break;

            case 10:
                ParseContact();
                break;
            
            case 11:
                ImportContact();
                break;
            
            case 12:
                DeleteContact();
                break;

            case 13:
                EnumContacts();
                break;
            
            case 14:
                SetWatching();
                break;

            case 15:
                SetWatchPermissions();
                break;

            case 16:
                GetPresenceInformation();
                break;
                    
            case 17:
                SetPresenceInformation();
                break;

            case 18:
                SubscribeEndpointData();
                break;

            case 19:
                UnsubscribeEndpointData();
                break;

            case 20:
                PublishEndpointObject();
                break;

            case 21:
                DeleteEndpointObject();
                break;
                                    
            case 22:
                RegisterApplication();
                break;

            case 23:
                UnregisterApplication();
                break;

            case 24:
                DisplayApplicationRegistrations();
                break;

            case 25:
                SendInvite();
                break;

            default:
                wprintf(L"Invalid selection.\n");
                break;

        }

        //Pause so that our output doesn't scroll
        //off the screen before the user gets a change to
        //read it.
         wprintf(L"\n\nPress <ENTER> to continue.\n");
        fflush(stdin);
        (void)StringCbGets(wzBuff, sizeof(wzBuff));
    }
}


int __cdecl wmain(int argc, __in_ecount(argc) wchar_t *argv[])
{
    HRESULT hr;
    BOOL fPeerCollabRunning = FALSE;

    hr = PeerCollabStartup(PEER_COLLAB_VERSION);

    if (FAILED(hr))
    {
        goto exit;
    }

    //PeerCollabStartup was successful.  Must call PeerCollabShutdown before we terminate
    fPeerCollabRunning = TRUE;

    if (argc > 1)
    {
        if (wcscmp(argv[1], L"/invite") == 0)
        {
            // When we regsiter ourselves to be launched in response to an 
            // invite we specify that the command line argument "/invite"
            // should be used
            //
            DisplayAppLauchInfo();
        }
        else if (wcscmp(argv[1], L"/showevents") == 0) 
        {
            //Run the event handler.  This will return once the
            //process running the interactive menu exits
            printf("----------------------------------------------\n");
            printf("Interactive menu is running in original window\n");
            printf("\n");
            printf("Collaboration events will print to this window\n");
            printf("----------------------------------------------\n");
            EventHandler();

            hr = S_OK;
            goto exit;
        }
    }

    {
        //Launch a second process that appears in a seperate console window.
        //That process will print collaboration events as they occur.
        //
        //We will use this console window for the interactive menu.

        //Launch ourselves as a second process with the "/showevents" command
        //line option.
        WCHAR cmdLine[512];
        STARTUPINFO startupInfo = {0};
        PROCESS_INFORMATION procInfo = {0};

        ZeroMemory( &startupInfo, sizeof(startupInfo) );
        startupInfo.cb = sizeof(startupInfo);
        ZeroMemory( &procInfo, sizeof(procInfo) );

        StringCchPrintf(cmdLine, celems(cmdLine), L"%s /showevents", argv[0]);

        if (!CreateProcess(argv[0], cmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &procInfo))
        {
            printf("Unable to lauch child process.\n");
            goto exit;
        }

        //Close the handles from CreateProcess.  We do not need them
        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);

        // Run the menu and allow the user to exercise the collaboration
        // sample interactively
        //
        RunMenu();
    }

exit:

    if (fPeerCollabRunning != FALSE)
    {
        PeerCollabShutdown();
    }

    return SUCCEEDED(hr)?0:-1;
}
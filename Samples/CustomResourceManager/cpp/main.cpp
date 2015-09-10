//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include <stdio.h>
#include <iostream>
#include "CSecInfo.h"
#include "windows.h"
#include <AclUI.h>
  
#pragma warning(push)  
// C4127: conditional expression is constant (disabled for our input loop)
#pragma warning(disable : 4127)   

int __cdecl wmain(
    _In_ int argc,
    _In_reads_(argc) PCWSTR argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    
    CSecInfo cSecInfo;
    BOOL bResult;
    if ( cSecInfo.m_bFailedToConstruct != FALSE)
    {
        wprintf(L"Couldn't construct our class. Exiting.\n");
        return 1;
    }
    
    int choice;
    do {
        // Print the following:
        // ------------------------------------
        // 1. Contoso forums
        // 2.      Sports
        // 3.              Favorite team
        // 4.              Upcoming events
        // 5.      Movies
        // 6.              2012 releases
        // 7.              Classics
        // 8.      Hobbies
        // 9.              Learning to cook
        // 10.             Snowboarding
        // 11. Help
        // ------------------------------------
        wprintf(L"------------------------------------\n");
        wprintf(L"1. %s\n",         cSecInfo.GetResource(0)->GetName());
        wprintf(L"2. \t%s\n",       cSecInfo.GetResource(1)->GetName());
        wprintf(L"3. \t\t%s\n",     cSecInfo.GetResource(2)->GetName());
        wprintf(L"4. \t\t%s\n",     cSecInfo.GetResource(3)->GetName());
        wprintf(L"5. \t%s\n",       cSecInfo.GetResource(4)->GetName());
        wprintf(L"6. \t\t%s\n",     cSecInfo.GetResource(5)->GetName());
        wprintf(L"7. \t\t%s\n",     cSecInfo.GetResource(6)->GetName());
        wprintf(L"8. \t%s\n",       cSecInfo.GetResource(7)->GetName());
        wprintf(L"9. \t\t%s\n",     cSecInfo.GetResource(8)->GetName());
        wprintf(L"10. \t\t%s\n",    cSecInfo.GetResource(9)->GetName());
        wprintf(L"11. Help\n");
        wprintf(L"------------------------------------\n");
        wprintf(L"Input a number to view/edit the security descriptor "
            L"for one of the above: ");
        std::cin >> choice;

        if ( choice >= 1 && choice <= 10 )
        {
            cSecInfo.SetCurrentObject(choice-1);
            wprintf(L"You chose %s\n\n", 
                cSecInfo.GetResource(choice-1)->GetName());
        }
        else if ( choice == 11 )
        {
            wprintf(L"\n"
                L"This resource manager example models a set of forums"
                L". On the top level, there is a single FORUMS object "
                L"(%s). It has three children, which are SECTIONs (%s,"
                L" %s, and %s), and they each have two children, which"
                L" are TOPICs. Inheritable ACEs follow this inheritance"
                L" hierarchy. Choose a number (1-%d) to edit any of "
                L"their security descriptors.\n\n",
                cSecInfo.GetResource(CONTOSO_FORUMS)->GetName(), 
                cSecInfo.GetResource(SPORTS)->GetName(),
                cSecInfo.GetResource(MOVIES)->GetName(), 
                cSecInfo.GetResource(HOBBIES)->GetName(),
                NUMBER_OF_RESOURCES);
            continue;
        }
        else
        {
            break;
        }

        // Suppress warning about the param being 0
        #pragma warning(suppress: 6387)
        bResult = EditSecurity(nullptr, &cSecInfo);
        if ( !bResult )
        {
            wprintf(L"EditSecurity error %d\n", GetLastError());
            break;
        }

    } while ( true );

    return 0;
}

#pragma warning(pop)  

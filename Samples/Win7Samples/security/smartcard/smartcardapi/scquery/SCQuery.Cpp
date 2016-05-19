/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

 Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.

 Module:      SCQuery.cpp

 Abstract:    Main program of SCQuery for Smart Card SDK sample. See
              ReadMe.txt for more detail information about this sample.

 Environment: Win32 console, C++ w/SEH, UNICODE ready.

------------------------------------------------------------------------------*/

////////////////////
//
// INCLUDE
//

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <winscard.h>

#include "SCCommon.h"

///////////////
//
// Prototype
//

LONG ListReaders (IN SCARDCONTEXT hContext, IN LPCTSTR lpmszReaderGroups);
LONG ListCards   (IN SCARDCONTEXT hContext, IN LPCBYTE lpszATR);
LONG ListGroups  (IN SCARDCONTEXT hContext);
LONG QueryCards  (IN SCARDCONTEXT hContext, IN LPCTSTR lpmszReaderGroups);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : _tmain

 Synopsis : Entry point of SCQuery.

 Parameter: Standard ANSI C command line parameters.

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

extern "C" int __cdecl _tmain (int argc, _TCHAR  * argv[])
{
    LONG lResult;
    SCARDCONTEXT hContext = NULL;

    __try
    {
        //
        // Establish context with the resource manager.
        //
        lResult = SCardEstablishContext(SCARD_SCOPE_USER,
                                        NULL,
                                        NULL,
                                        &hContext);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        //
        // List readers
        //
        lResult = ListReaders(hContext, NULL);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        //
        // List cards
        //
        lResult = ListCards(hContext, NULL);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        //
        // List groups
        //
        lResult = ListGroups(hContext);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        //
        // Query cards
        //
        lResult = QueryCards(hContext, NULL);
    }

    __finally
    {
        //
        // Don't forget to release the context handle if established.
        //
        if (hContext != NULL)
        {
            LONG lReturn = SCardReleaseContext(hContext);

            //
            // If successful so far, then capture the SCardReleaseContext()
            // return code; otherwise, don't bother
            //
            if (lResult == SCARD_S_SUCCESS)
            {
                lResult = lReturn;
            }
        }
    }

    //
    // Inform user if an error had occurred.
    //
    if (lResult != SCARD_S_SUCCESS)
    {
        _tprintf(_T("\nError [0x%lx]: Program terminated abnormally.\n"),
                 lResult);
    }

    return((int) lResult);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : ListReaders

 Synopsis : Print out a formatted list of registered Smart Card readers
            associated with the specified reader groups to STDOUT.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - IN LPCTSTR lpmszReaderGroups

              Pointer to multi-string reader group names, or NULL to specify all
              readers known to the system.

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG ListReaders (IN SCARDCONTEXT hContext,
                  IN LPCTSTR lpmszReaderGroups)
{
    LONG lResult;
    LPTSTR lpmszReaderNames;

    __try
    {
        //
        // Get the list of registered readers associated with the specified
        // group(s).
        // Note: The buffer is automatically allocated and must be freed
        //       by SCFree().
        //
        lResult = SCListReaders(hContext,
                                lpmszReaderGroups,
                                (LPTSTR *) &lpmszReaderNames);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        DWORD  dwNumReaders   = 0;
        LPTSTR lpszReaderName = lpmszReaderNames;

        _tprintf(_T("\n"));
        _tprintf(_T("Registered Reader(s)\n"));
        _tprintf(_T("====================\n"));

        //
        // Walk through the list of readers and print out some information.
        // Note: The list of readers are in a multi-string structure.
        //
        while (*lpszReaderName != _T('\0'))
        {
            _tprintf(_T("%02d: %s\n"), ++dwNumReaders, lpszReaderName);
            lpszReaderName += lstrlen(lpszReaderName) + 1;
        }

        //
        // Inform the user if no reader was found.
        //
        if (dwNumReaders == 0)
        {
            _tprintf(_T("No registered reader was found for the specified "
                        _T("reader group(s) [%s].\n")), lpmszReaderGroups);
        }
    }

    __finally
    {
        //
        // Don't forget to release memory, if allocated.
        //
        if (lpmszReaderNames != NULL)
        {
            LONG lReturn = SCFree((LPVOID) lpmszReaderNames);

            //
            // If successful so far, then capture the return code
            // from SCFree(); otherwise, don't bother.
            //
            if (lResult == SCARD_S_SUCCESS)
            {
                lResult = lReturn;
            }
        }
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : ListCards

 Synopsis : Print out a formatted list of registered Smart Cards associated with
            the specified ATR string to STDOUT.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - IN LPCBYTE lpszATR

              ATR string of the card to list, or NULL to return all cards known
              to the system.

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG ListCards (IN SCARDCONTEXT hContext,
                IN LPCBYTE lpszATR)
{
    LONG lResult;
    LPTSTR lpmszCardNames;

    __try
    {
        //
        // Get the list of registered cards associated with the specified ATR.
        // Note: The buffer is automatically allocated and must be freed
        //       by SCFree().
        //
        lResult = SCListCards(hContext,
                              lpszATR,
                              &lpmszCardNames);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        DWORD dwNumCards = 0;
        LPTSTR lpszCardName = lpmszCardNames;

        _tprintf(_T("\n"));
        _tprintf(_T("Registered Card(s)\n"));
        _tprintf(_T("==================\n"));

        //
        // Walk through the list of cards and print out some information.
        // Note: The list of cards are in a multi-string structure.
        //
        while (*lpszCardName != _T('\0'))
        {
            _tprintf(_T("%02d: %s\n"), ++dwNumCards, lpszCardName);
            lpszCardName += lstrlen(lpszCardName) + 1;
        }

        //
        // Inform the user if no card was found.
        //
        if (dwNumCards == 0)
        {
            _tprintf(_T("No registered Smart Card was found for the "
                        _T("specified ATR [%s].\n")), lpszATR);
        }
    }

    __finally
    {
        //
        // Don't forget to release memory, if allocated.
        //
        if (lpmszCardNames != NULL)
        {
            LONG lReturn = SCFree((LPVOID) lpmszCardNames);

            //
            // If successful so far, then capture the return code
            // from SCFree(); otherwise, don't bother.
            //
            if (lResult == SCARD_S_SUCCESS)
            {
                lResult = lReturn;
            }
        }
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : ListGroups

 Synopsis : Print out a formatted list of registered reader groups to STDOUT.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG ListGroups (IN SCARDCONTEXT hContext)
{
    LONG lResult;
    LPTSTR lpmszGroupNames;

    __try
    {
        //
        // Get the list of registered reader groups.
        // Note: The buffer is automatically allocated and must be freed
        //       by SCFree().
        //
        lResult = SCListGroups(hContext,
                               (LPTSTR *) &lpmszGroupNames);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        DWORD  dwNumGroups   = 0;
        LPTSTR lpszGroupName = lpmszGroupNames;

        _tprintf(_T("\n"));
        _tprintf(_T("Registered Group(s)\n"));
        _tprintf(_T("===================\n"));

        //
        // Walk through the list of groups and print out some information.
        // Note: The list of cards are in a multi-string structure.
        //
        while (*lpszGroupName != TEXT('\0'))
        {
            _tprintf(_T("%02d: %s\n"), ++dwNumGroups, lpszGroupName);
            lpszGroupName += lstrlen(lpszGroupName) + 1;
        }

        //
        // Inform the user if no group was found.
        //
        if (dwNumGroups == 0)
        {
            _tprintf(_T("No registered reader group was found.\n"));
        }
    }

    __finally
    {
        //
        // Don't forget to release memory, if allocated.
        //
        if (lpmszGroupNames != NULL)
        {
            LONG lReturn = SCFree((LPVOID) lpmszGroupNames);

            //
            // If successful so far, then capture the return code
            // from SCFree(); otherwise, don't bother.
            //
            if (lResult == SCARD_S_SUCCESS)
            {
                lResult = lReturn;
            }
        }
    }

    return lResult;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : QueryCards

 Synopsis : Print out a formatted list of card found in the specified reader
            group to STDOUT.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - IN LPCTSTR lpmszReaderGroups

              Pointer to multi-string reader group names, or NULL to specify all
              readers known to the system.

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG QueryCards (IN SCARDCONTEXT hContext,
                 IN LPCTSTR lpmszReaderGroups)
{
    LONG   lResult          = SCARD_S_SUCCESS;
    LPTSTR lpmszReaderNames = NULL;

    __try
    {
        //
        // Get the list of registered readers associated with the specified
        // group(s).
        // Note: The buffer is automatically allocated and must be freed
        //       by SCFree().
        //
        lResult = SCListReaders(hContext,
                                lpmszReaderGroups,
                                (LPTSTR *) &lpmszReaderNames);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        _tprintf(_T("\n"));
        _tprintf(_T("Card in reader\n"));
        _tprintf(_T("==============\n"));

        //
        // Note that MAXIMUM_SMARTCARD_READERS (10) only refers to maximum # of
        // non-PnP readers. The resource manager is capable of handling any number
        // of PnP readers, in addition to the 10 non-PnP readers (your physical
        // system is the limiting factor). Make appropriate changes as necessary
        // to accomodate more readers.
        //

        DWORD  dwNumReaders   = 0;
        LPTSTR lpszReaderName = lpmszReaderNames;
        SCARD_READERSTATE rsReaders[MAXIMUM_SMARTCARD_READERS];

        ZeroMemory((LPVOID)rsReaders, sizeof(rsReaders));

        //
        // Prepare state array
        //
        while ((*lpszReaderName != _T('\0')) &&
               (dwNumReaders < MAXIMUM_SMARTCARD_READERS))
        {
            rsReaders[dwNumReaders].szReader = (LPCTSTR)lpszReaderName;
            rsReaders[dwNumReaders].dwCurrentState = SCARD_STATE_UNAWARE;

            dwNumReaders++;
            lpszReaderName += lstrlen(lpszReaderName) + 1;
        }

        if (dwNumReaders == 0)
        {
            _tprintf(_T("No registered reader was found for the specified "
                        _T("reader group(s) [%s].\n")), lpmszReaderGroups);
            __leave;
        }

        //
        // Now check state of each reader
        //
        lResult = SCardGetStatusChange(hContext,
                                       INFINITE,
                                       rsReaders,
                                       dwNumReaders);
        if (lResult != SCARD_S_SUCCESS)
        {
            __leave;
        }

        for (DWORD dwIndex = 0; dwIndex < dwNumReaders; dwIndex++)
        {
            //
            // Print out reader name
            //
            _tprintf(_T("%02d: %s\n"), dwIndex + 1, rsReaders[dwIndex].szReader);

            //
            // If a card is present in this reader, print ATR and
            // any extra data from the Select File command.
            //
            if (!(rsReaders[dwIndex].dwEventState & SCARD_STATE_PRESENT))
            {
                _tprintf(_T("    No Smart Card found in this reader.\n"));
                continue;
            }

            SCARDHANDLE hCard  = NULL;
            LPBYTE lpbResponse = NULL;

            __try
            {
                DWORD dwActiveProtocol;

                //
                // Connect to the card
                //
                lResult = SCardConnect(hContext,
                                       rsReaders[dwIndex].szReader,
                                       SCARD_SHARE_SHARED,
                                       SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                       &hCard,
                                       &dwActiveProtocol);
                if (lResult != SCARD_S_SUCCESS)
                {
                    __leave;
                }

                DWORD dwReaderLen = 0;
                DWORD dwState;
                DWORD dwProtocol;
                BYTE  atr[MAXIMUM_ATTR_STRING_LENGTH];
                DWORD dwAtrLen;

                //
                // Get status of card
                //
                lResult = SCardStatus(hCard,
                                      NULL,
                                      &dwReaderLen,
                                      &dwState,
                                      &dwProtocol,
                                      atr,
                                      &dwAtrLen);
                if (lResult != SCARD_S_SUCCESS)
                {
                    __leave;
                }

                //
                // Print out ATR bytes of card
                //
                _tprintf(_T("    ATR = "), dwIndex + 1);
                for (DWORD i = 0; i < dwAtrLen; i++)
                {
                    _tprintf(_T("%02x "), (DWORD) atr[i]);
                }

                _tprintf(_T("\n"));

                BYTE filename[2] = {0x3f, 0x00};
                DWORD dwExtraBytes;

                //
                // Select Master File (0x3f00)
                //
                lResult = SCSelectFile(hCard,
                                       filename,
                                       &dwExtraBytes);
                if (lResult != SCARD_S_SUCCESS)
                {
                    __leave;
                }

                //
                // Any extra data?
                //
                if (dwExtraBytes)
                {
                    //
                    // Yes, so retrieve it
                    //
                    lResult = SCGetResponse(hCard, dwExtraBytes, &lpbResponse);

                    if (lResult == SCARD_S_SUCCESS)
                    {
                        //
                        // Print out MF file control information
                        //
                        _tprintf(_T("    FCI = "), dwIndex + 1);
                        for (DWORD i = 0; i < dwExtraBytes; i++)
                        {
                            _tprintf(_T("%02x "), (DWORD) lpbResponse[i]);
                        }

                        _tprintf(_T("\n"));
                    }
                }

                _tprintf(_T("\n"));
            }

            __finally
            {
                LONG lReturn;

                //
                // Don't forget to release the memory block if it had been
                // allocated by SCGetResponse().
                //
                if (lpbResponse != NULL)
                {
                    lReturn = SCFree((LPVOID) lpbResponse);

                    //
                    // If successful so far, then capture the return code
                    // from SCFree(); otherwise, don't bother.
                    //
                    if (lResult == SCARD_S_SUCCESS)
                    {
                        lResult = lReturn;
                    }
                }

                //
                // Don't forget to disconnect the card.
                //
                if (hCard != NULL)
                {
                    lReturn = SCardDisconnect(hCard, SCARD_LEAVE_CARD);

                    //
                    // If successful so far, then capture the SCardDisconnect()
                    // return code; otherwise, don't bother.
                    //
                    if (lResult == SCARD_S_SUCCESS)
                    {
                        lResult = lReturn;
                    }
                }
            }
        }
    }

    __finally
    {
        LONG lReturn;

        //
        // Don't forget to release memory, if allocated.
        //
        if (lpmszReaderNames != NULL)
        {
            lReturn = SCFree((LPVOID) lpmszReaderNames);

            //
            // If successful so far, then capture the return code
            // from SCFree(); otherwise, don't bother.
            //
            if (lResult == SCARD_S_SUCCESS)
            {
                lResult = lReturn;
            }
        }
    }

    return lResult;
}


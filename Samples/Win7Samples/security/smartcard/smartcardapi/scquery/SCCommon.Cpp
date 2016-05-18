/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

 Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.

 Module:      SCCommon.cpp

 Abstract:    Common routines for used by Smart Card enbled application.

 Environment: Win32 console, C++ w/SEH

------------------------------------------------------------------------------*/

///////////////
//
// INCLUDE
//

#include <tchar.h>
#include <assert.h>
#include <windows.h>
#include <winscard.h>

#include "sccommon.h"


///////////////
//
// Internal Functions
//

static LONG scSelectFile (IN SCARDHANDLE hCard,
                          IN DWORD dwClassCode,
                          IN LPBYTE lpbFileName,
                          OUT LPDWORD lpdwExtraBytes);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCMalloc

 Synopsis : Allocate a block of memory and initialize to all 0s.

 Parameter: - IN DWORD dwSize

              Size of memory to allocate in bytes.

 Return   : Pointer to allocated memory block or NULL. Memory allocated must be
            freed with SCFree().

------------------------------------------------------------------------------*/

LPVOID SCMalloc (IN DWORD dwSize)
{
    return(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize));
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCFree

 Synopsis : Free a block of memory previously allocated by SCMalloc().

 Parameter: - IN LPVOID lpMemory

              Pointer to memory block to be freed.

 Return   : SCARD_S_SUCCESS or error code

-----------------------------------------------------------------------------*/

LONG SCFree (IN LPVOID lpMemory)
{
    LONG lResult;

    //
    // Parameters sanity check.
    //
    assert(lpMemory != NULL);

    if (HeapFree(GetProcessHeap(), 0, lpMemory))
    {
        lResult = SCARD_S_SUCCESS;
    }
    else
    {
        lResult = GetLastError();
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCListReaders

 Synopsis : Return a list of registered Smart Card readers associated with the
            specified reader groups.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - IN LPCTSTR lpmszReaderGroups

              Pointer to multi-string reader group names, or NULL to list all
              readers known to the system.

            - OUT LPTSTR * lplpmszReaderNames

              Receives a pointer to a block of memory containing list of
              registered reader names. This block of memory must be freed
              with scFree().

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG SCListReaders (IN SCARDCONTEXT hContext,
                    IN LPCTSTR lpmszReaderGroups,
                    OUT LPTSTR * lplpmszReaderNames)
{
    LONG  lResult;
    DWORD dwReaders;

    //
    // Parameters sanity check.
    //
    assert(lplpmszReaderNames != NULL);

    //
    // Initialize returned info.
    //
    * lplpmszReaderNames = NULL;

    //
    // First find the required buffer length.
    //
    lResult = SCardListReaders(hContext,
                               lpmszReaderGroups,
                               NULL,         // NULL to indicate we want to
                               &dwReaders);  // know the length of the buffer
    if (lResult != SCARD_S_SUCCESS)
    {
        return lResult;
    }

    //
    // Allocate memory.
    //
    LPTSTR lpmszReaderNames = (LPTSTR) SCMalloc(dwReaders * sizeof(_TCHAR));

    if (lpmszReaderNames == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Now actually get the list of reader names.
    //
    lResult = SCardListReaders(hContext,
                               lpmszReaderGroups,
                               lpmszReaderNames,
                               &dwReaders);
    if (lResult == SCARD_S_SUCCESS)
    {
        //
        // Successful, so return pointer to reader names.
        //
        *lplpmszReaderNames = lpmszReaderNames;
    }
    else
    {
        //
        // Error occurred, so free memory.
        //
        SCFree((LPVOID) lpmszReaderNames);
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCListCards

 Synopsis : Return a list of registered Smart Cards associated with the
            specified ATR string.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - IN LPCBYTE lpszATR

              ATR string of the card to list, or NULL to return all cards known
              to the system.

            - OUT LPTSTR * lplpmszCardNames

              Receives a pointer to a block of memory containing list of
              registered card names. This block of memory must be freed
              with scFree().

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG SCListCards (IN SCARDCONTEXT hContext,
                  IN LPCBYTE lpszATR,
                  OUT LPTSTR * lplpmszCardNames)
{
    LONG  lResult;
    DWORD dwCards;

    //
    // Parameters sanity check.
    //
    assert(lplpmszCardNames != NULL);

    //
    // Initialize returned info.
    //
    * lplpmszCardNames = NULL;

    //
    // First find the required buffer length.
    //
    lResult = SCardListCards(hContext,
                             lpszATR,
                             NULL,
                             0,
                             NULL,       // NULL to indicate we want to
                             &dwCards);  // know the length of the buffer
    if (lResult != SCARD_S_SUCCESS)
    {
        return lResult;
    }

    //
    // Allocate memory.
    //
    LPTSTR lpmszCardNames = (LPTSTR) SCMalloc(dwCards * sizeof(_TCHAR));

    if (lpmszCardNames == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Now actually get the list of card names.
    //
    lResult = SCardListCards(hContext,
                             lpszATR,
                             NULL,
                             0,
                             lpmszCardNames,
                             &dwCards);
    if (lResult == SCARD_S_SUCCESS)
    {
        //
        // Successful, so return pointer to card names.
        //
        *lplpmszCardNames = lpmszCardNames;
    }
    else
    {
        //
        // Error occurred, so free memory.
        //
        SCFree((LPVOID) lpmszCardNames);
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCListGroups

 Synopsis : Return a list of registered reader groups.

 Parameter: - IN SCARDCONTEXT hContext

              Resource manager context returned by SCardEstablishContext(), or
              NULL if the query is not directed towards a specific context.

            - OUT LPTSTR * lplpmszGroupNames

              Receives a pointer to a block of memory containing list of
              registered reader group names. This block of memory must be freed
              with scFree().

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG SCListGroups (IN SCARDCONTEXT hContext,
                   OUT LPTSTR * lplpmszGroupNames)
{
    LONG  lResult;
    DWORD dwGroups;

    //
    // Parameters sanity check.
    //
    assert(lplpmszGroupNames != NULL);

    //
    // Initialize returned info.
    //
    * lplpmszGroupNames = NULL;

    //
    // First find the required buffer length.
    //
    lResult = SCardListReaderGroups(hContext,
                                    NULL,        // NULL to indicate we want to
                                    &dwGroups);  // know the length of the buffer
    if (lResult != SCARD_S_SUCCESS)
    {
        return lResult;
    }

    //
    // Allocate memory.
    //
    LPTSTR lpmszGroupNames = (LPTSTR) SCMalloc(dwGroups * sizeof(_TCHAR));

    if (lpmszGroupNames == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Now actually get the list of group names.
    //
    lResult = SCardListReaderGroups(hContext,
                                    lpmszGroupNames,
                                    &dwGroups);
    if (lResult == SCARD_S_SUCCESS)
    {
        //
        // Successful, so return pointer to group names.
        //
        *lplpmszGroupNames = lpmszGroupNames;
    }
    else
    {
        //
        // Error occurred, so free memory.
        //
        SCFree((LPVOID) lpmszGroupNames);
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCGetAttrib

 Synopsis : Get the current reader attributes for the given card handle. It does
            not affect the state of the reader, driver, or card.


 Parameter: - IN SCARDHANDLE hCard

              Card handle returned by SCardConnect()

            - IN DWORD dwAttrId

              Identifier for the attribute to get

            - OUT LPBYTE * lplpbAttr

              Receives a pointer to a block of memory containing the requested
              attribute.  This block of memory must be freed with SCFree().

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG SCGetAttrib (IN SCARDHANDLE hCard,
                  IN DWORD dwAttrId,
                  OUT LPBYTE * lplpbAttr)
{
    LONG  lResult;
    DWORD dwAttrLen;

    //
    // Parameters sanity check.
    //
    assert(lplpbAttr != NULL);

    //
    // Initialize returned info.
    //
    * lplpbAttr = NULL;

    //
    // First find the required buffer length.
    //

    lResult = SCardGetAttrib(hCard,
                             dwAttrId,
                             NULL,         // NULL to indicate we want to
                             &dwAttrLen);  // know the length of the buffer
    if (lResult != SCARD_S_SUCCESS)
    {
        return lResult;
    }

    //
    // Allocate memory.
    //
    LPBYTE lpbAttr = (LPBYTE) SCMalloc(dwAttrLen);

    if (lpbAttr == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Now actually get the attribute.
    //
    lResult = SCardGetAttrib(hCard,
                             dwAttrId,
                             lpbAttr,
                             &dwAttrLen);
    if (lResult == SCARD_S_SUCCESS)
    {
        //
        // Successful, so return pointer to attribute.
        //
        *lplpbAttr = lpbAttr;
    }
    else
    {
        //
        // Error occurred, so free memory.
        //
        SCFree((LPVOID) lpbAttr);
    }

    return lResult;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCGetResponse

 Synopsis : Get the response data from the card.

 Parameter: - IN SCARDHANDLE hCard

              Card handle returned by SCardConnect()

            - IN DWORD dwLength

              Length of response data to retrieve in bytes

            - OUT LPBYTE * lplpbResponse

              Receives a pointer to a block of memory containing the requested
              response data from the card. This block of memory must be freed
              with SCFree().

 Return   : SCARD_S_SUCCESS or error code

------------------------------------------------------------------------------*/

LONG SCGetResponse (IN SCARDHANDLE hCard,
                    IN DWORD dwLength,
                    OUT LPBYTE * lplpbResponse)
{
    LONG lResult;

    //
    // Parameters sanity check.
    //
    assert(lplpbResponse != NULL);

    //
    // APDU response data length cannot be larger than 256
    //
    if (dwLength > 256)
    {
        return SCARD_E_INVALID_PARAMETER;
    }

    //
    // Initialize returned info.
    //
    *lplpbResponse = NULL;

    //
    // Allocate memory.
    //
    LPBYTE lpbResponse = (LPBYTE) SCMalloc(dwLength + 2);

    if (lpbResponse == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Construct the Get Response APDU.
    // Note that when dwLength is casted to BYTE, 256 would be converted to 0,
    // which will be the correct value to indicate 256 bytes.
    //
    DWORD dwStatusLen = dwLength + 2;
    BYTE apdu[5] = {0xc0, 0xc0, 0x00, 0x00, (BYTE) dwLength};

    //
    // Send APDU to card.
    //
    lResult = SCardTransmit(hCard,
                            SCARD_PCI_T0,
                            apdu,
                            sizeof(apdu),
                            NULL,
                            lpbResponse,
                            &dwStatusLen);
    if (lResult == SCARD_S_SUCCESS)
    {
        //
        // Did we get all the data?
        //
        if (dwStatusLen == dwLength + 2)
        {
            //
            // Is the data good?
            //
            if (lpbResponse[dwLength] == 0x90 && lpbResponse[dwLength + 1] == 0x00)
            {
                //
                // Successful, so return pointer to response data.
                //
                *lplpbResponse = (LPBYTE) lpbResponse;
            }
            else
            {
                //
                // Error occurred, so free memory
                //
                SCFree(lpbResponse);

                //
                // and return SW1 and SW2 as return code
                //
                lResult = MAKELONG(MAKEWORD(lpbResponse[dwLength + 1],
                                            lpbResponse[dwLength]), 0x000);
            }
        }
        else
        {
            //
            // Error occurred, so free memory
            //
            SCFree(lpbResponse);

            //
            // and return SW1 and SW2 as return code
            //
            lResult = MAKELONG(MAKEWORD(lpbResponse[1], lpbResponse[0]), 0x000);
        }
    }
    else
    {
        //
        // Error occurred, so free memory before returning.
        //
        SCFree(lpbResponse);
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Function : SCSelectFile

 Synopsis : Select a file on the Smart Card.

 Parameter: - IN SCARDHANDLE hCard

              Card handle returned by SCardConnect()

            - IN LPBYTE lpbFileName

              Pointer to 2-byte filename to select on the card

            - OUT LPDWORD lpdwExtraBytes

              Pointer to a DWORD to receive number of extra bytes available from
              the card as a consequent of this operation.

 Return   : SCARD_S_SUCCESS or error code

 Remarks  : Value for APDU Class byte when used for Select File command varies
            among different cards. Some cards expect it to be 0xC0, while others
            insist on 0x00. To properly handle this inconsistency, we will try
            both 0xC0 and 0x00, if necessary.

------------------------------------------------------------------------------*/

LONG SCSelectFile (IN SCARDHANDLE hCard,
                   IN LPBYTE lpbFileName,
                   OUT LPDWORD lpdwExtraBytes)
{
    LONG lResult;

    //
    // Parameters sanity check.
    //
    assert(lpbFileName != NULL);
    assert(lpdwExtraBytes != NULL);

    //
    // Start with 0xC0.
    //
    lResult = scSelectFile(hCard, 0xC0, lpbFileName, lpdwExtraBytes);
    if (lResult == 0x6e00)
    {
        //
        // Try 0x00.
        //
        lResult = scSelectFile(hCard, 0x00, lpbFileName, lpdwExtraBytes);
    }

    return lResult;
}


/*++++++++++++++++++++++++++++++++++++++

 Function : scSelectFile

 Synopsis : Internal. Select a file on the Smart Card.

 Parameter: - IN SCARDHANDLE hCard

              Card handle returned by SCardConnect()

            - IN DWORD dwClassCode

              Must be 0xC0 or 0x00

            - IN LPBYTE lpbFileName

              Pointer to 2-byte filename to select on the card

            - OUT LPDWORD lpdwExtraBytes

              Pointer to a DWORD to receive number of extra bytes available from
              the card as a consequent of this operation.

 Return   : SCARD_S_SUCCESS or error code

----------------------------------------*/

static LONG scSelectFile (IN SCARDHANDLE hCard,
                          IN DWORD dwClassCode,
                          IN LPBYTE lpbFileName,
                          OUT LPDWORD lpdwExtraBytes)
{
    LONG lResult;

    //
    // Initialize returned info.
    //
    *lpdwExtraBytes = 0;

    //
    // Construct the Select File APDU
    //
    BYTE  status[2];
    DWORD dwStatusLength = sizeof(status);
    BYTE apdu[7] = {(BYTE) dwClassCode, 0xa4, 0x00, 0x00, 0x02,
        *lpbFileName, *(lpbFileName + 1)};

    //
    // Send APDU to card
    //
    lResult = SCardTransmit(hCard,
                            SCARD_PCI_T0,
                            apdu,
                            sizeof(apdu),
                            NULL,
                            status,
                            &dwStatusLength);

    //
    // Sanity check
    //
    assert(dwStatusLength == sizeof(status));

    //
    // If API successful but card operation failed, then
    // return SW1 and SW2 as error code
    //
    if (lResult == SCARD_S_SUCCESS)
    {
        if (!(status[0] == 0x90 && status[1] == 0x00))
        {
            if (status[0] == 0x61)
            {
                //
                // Successful, but there might be extra data from the card
                //
                *lpdwExtraBytes = (DWORD) status[1];
            }
            else
            {
                //
                // Card error, so return SW1 and SW2 as error
                //
                lResult = MAKELONG(MAKEWORD(status[1], status[0]), 0x0000);
            }
        }
    }

    return lResult;
}


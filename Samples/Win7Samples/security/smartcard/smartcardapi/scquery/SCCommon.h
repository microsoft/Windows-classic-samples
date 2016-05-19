/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

 Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.

 Filename:    SCCommon.h

 Abstract:    Common routines for used by Smart Card enbled application.

 Environment: Win32 console, C++ w/SEH

------------------------------------------------------------------------------*/

#if !defined (INC_SCCOMMON_H)
    #define INC_SCCOMMON_H

//
// C interface
//
    #ifdef __cplusplus
extern "C"
{
    #endif


    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

     Function : SCMalloc

     Synopsis : Allocate a block of memory and initialize to all 0s.

     Parameter: - IN DWORD dwSize

                  Size of memory to allocate in bytes.

     Return   : Pointer to allocated memory block or NULL. Memory allocated must be
                freed with SCFree().

    ------------------------------------------------------------------------------*/

    LPVOID SCMalloc (IN DWORD dwSize);


    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

     Function : SCFree

     Synopsis : Free a block of memory previously allocated by SCMalloc().

     Parameter: - IN LPVOID lpMemory

                  Pointer to memory block to be freed.

     Return   : SCARD_S_SUCCESS or error code

    ------------------------------------------------------------------------------*/

    LONG SCFree (IN LPVOID lpMemory);


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
                  with SCFree().

     Return   : SCARD_S_SUCCESS or error code

    ------------------------------------------------------------------------------*/

    LONG SCListReaders (IN SCARDCONTEXT hContext,
                        IN LPCTSTR lpmszReaderGroups,
                        OUT LPTSTR * lplpmszReaderNames);


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
                  with SCFree().

     Return   : SCARD_S_SUCCESS or error code

    ------------------------------------------------------------------------------*/

    LONG SCListCards (IN SCARDCONTEXT hContext,
                      IN LPCBYTE lpszATR,
                      OUT LPTSTR * lplpmszCardNames);


    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

     Function : SCListGroups

     Synopsis : Return a list of registered reader groups.

     Parameter: - IN SCARDCONTEXT hContext

                  Resource manager context returned by SCardEstablishContext(), or
                  NULL if the query is not directed towards a specific context.

                - OUT LPTSTR * lplpmszGroupNames

                  Receives a pointer to a block of memory containing list of
                  registered reader group names. This block of memory must be freed
                  with SCFree().

     Return   : SCARD_S_SUCCESS or error code

    ------------------------------------------------------------------------------*/

    LONG SCListGroups (IN SCARDCONTEXT hContext,
                       OUT LPTSTR * lplpmszGroupNames);


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
                      OUT LPBYTE * lplpbAttr);


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
                        OUT LPBYTE * lplpbResponse);


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

    ------------------------------------------------------------------------------*/

    LONG SCSelectFile (IN SCARDHANDLE hCard,
                       IN LPBYTE lpbFileName,
                       OUT LPDWORD lpdwExtraBytes);

    #ifdef __cplusplus
}
    #endif

#endif


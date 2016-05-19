/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Utils.c

Abstract:

    This c file includes sample code for packing WSAQUERYSET2 into
    a single buffer.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#include "utils.h"

// Calculate the size of a string, in bytes, including number of bytes
// required to round up to the next aligned boundary
//
size_t RoundedStringSize(PCWSTR pcwzString)
{
    if (pcwzString == NULL)
    {
        return 0;
    }
    else
    {
        size_t cch = wcslen(pcwzString) + 1;
        size_t cb = sizeof(WCHAR) * cch;
        return ROUND_UP_COUNT(cb, ALIGN_LPVOID);
    }
}

// Utility routine to return the number of bytes required to store a 
// full WSAQUERYSET2 in a single buffer
//
size_t GetWSAQuerySet2Size(__in  WSAQUERYSET2  *pQuerySet)
{
    ULONG   i = 0;
    size_t  cbTotal = sizeof(WSAQUERYSET2);

    // calculate the string sizes
    cbTotal += RoundedStringSize(pQuerySet->lpszServiceInstanceName);
    cbTotal += RoundedStringSize(pQuerySet->lpszComment);
    cbTotal += RoundedStringSize(pQuerySet->lpszContext);
    cbTotal += RoundedStringSize(pQuerySet->lpszQueryString);

    if (pQuerySet->lpVersion != NULL)
    {
        cbTotal += sizeof(WSAVERSION);
        cbTotal = ROUND_UP_COUNT(cbTotal, ALIGN_LPVOID);
    }
    
    if (pQuerySet->lpNSProviderId != NULL)
    {
        cbTotal += sizeof(GUID);
        cbTotal = ROUND_UP_COUNT(cbTotal, ALIGN_LPVOID);
    }

    if (pQuerySet->lpafpProtocols != NULL && pQuerySet->dwNumberOfProtocols > 0)
    {
        cbTotal += pQuerySet->dwNumberOfProtocols * sizeof(AFPROTOCOLS);
    }

    if (pQuerySet->lpcsaBuffer != NULL && pQuerySet->dwNumberOfCsAddrs > 0)
    {
        cbTotal += pQuerySet->dwNumberOfCsAddrs * sizeof(CSADDR_INFO);

        for (i = 0; i < pQuerySet->dwNumberOfCsAddrs; i++)
        {
            cbTotal += pQuerySet->lpcsaBuffer[i].LocalAddr.iSockaddrLength;
            cbTotal = ROUND_UP_COUNT(cbTotal, ALIGN_LPVOID);
            cbTotal += pQuerySet->lpcsaBuffer[i].RemoteAddr.iSockaddrLength;
            cbTotal = ROUND_UP_COUNT(cbTotal, ALIGN_LPVOID);
        }
    }

    if (pQuerySet->lpBlob != NULL)
    {
        cbTotal += sizeof(BLOB);
        cbTotal += pQuerySet->lpBlob->cbSize;
        cbTotal = ROUND_UP_COUNT(cbTotal, ALIGN_LPVOID);
    }

    return cbTotal;
}


// Utility routine to serialize data into a buffer
//
// ppDest:   Gets updated to point to ppBuffer
// pvSrc:    Location to copy data from
// cbSrc:    Number of bytes to copy from the buffer
// ppBuffer: Location to copy the data to.  The pointer is updated to point to the end
//           of the buffer (ready for the next copy into it).
//
void SerializeData(__out void **ppDest, 
                   __in  const void *pvSrc, 
                         size_t cbSrc, 
                   __out_ecount(cbSrc) BYTE **ppBuffer)
{
    if (pvSrc != NULL && cbSrc > 0)
    {
        // Point the destination pointer to the buffer
        *ppDest = (void*) *ppBuffer;

        // copy the string
        CopyMemory(*ppDest, pvSrc, cbSrc);

        // move the buffer pointer out past the string
        *ppBuffer += cbSrc;

        // move the buffer pointer to safe alignment boundary
        *ppBuffer  = (PBYTE) ROUND_UP_POINTER(*ppBuffer, ALIGN_LPVOID);
    }
    else
    {
        *ppDest = NULL;
    }
}

// Utility routine to serialize a string into a buffer
//
void SerializeString(__out WCHAR **ppDest, 
                     PCWSTR pcwzSrc, 
                     __out BYTE **ppBuffer)
{
    if (pcwzSrc != NULL)
    {
        size_t cbSrc = (wcslen(pcwzSrc) + 1) * sizeof(WCHAR);
        SerializeData(ppDest, pcwzSrc, cbSrc, ppBuffer);
    }
    else
    {
        *ppDest = NULL;
    }
}

// Utility routine to serialize the array of addresses into a buffer
//
void SerializeAddresses(__out CSADDR_INFO **ppDestAddrs, 
                        __in_ecount(cAddr) CSADDR_INFO *pSrcAddrs, 
                        DWORD cAddr, 
                        __out BYTE **ppBuffer)
{
    DWORD i = 0;

    if (pSrcAddrs != NULL && cAddr > 0)
    {
        // Serialize the outer level array of addresss
        SerializeData(ppDestAddrs, pSrcAddrs, sizeof(CSADDR_INFO) * cAddr, ppBuffer);
            
        // loop through and serialize the inner pointers
        for (i = 0; i < cAddr; i++)
        {
            CSADDR_INFO *pAddr = &((*ppDestAddrs)[i]);

            SerializeData(&pAddr->LocalAddr.lpSockaddr, pSrcAddrs[i].LocalAddr.lpSockaddr, 
                          pSrcAddrs[i].LocalAddr.iSockaddrLength, ppBuffer);

            SerializeData(&pAddr->RemoteAddr.lpSockaddr, pSrcAddrs[i].RemoteAddr.lpSockaddr, 
                          pSrcAddrs[i].RemoteAddr.iSockaddrLength, ppBuffer);
        }
    }
    else
    {
        *ppDestAddrs = NULL;
    }
}


// Utility routine to pack a WSAQUERYSET2 into a single continuous buffer
//
INT BuildSerializedQuerySet2(
    __in                               PWSAQUERYSET2  pQuerySet,
                                       size_t         cbSerializedQuerySet,
    __out_bcount(cbSerializedQuerySet) PBYTE          pbSerializedQuerySet)
{
    PBYTE pbCurrData   = NULL;
    PWSAQUERYSET2 pQS2 = (PWSAQUERYSET2) pbSerializedQuerySet;
 
    // Verify the output buffer is big enough.  This is ** CRITICAL ** because this function
    // does not continue to track the amount of memory consumed in the buffer as it is written to.
    //
    if (cbSerializedQuerySet < GetWSAQuerySet2Size(pQuerySet))
    {
        return WSA_INVALID_PARAMETER;
    }

    // Do a shallow copy to copy non-pointer fields.
    //
    *pQS2 = *pQuerySet;

    // Initial the pbCurrData pointer which holds all the variable length (pointer) data
    //
    pbCurrData = ((PBYTE) pQS2) + sizeof(WSAQUERYSET2);

    // Serialize all the string fields
    //
    SerializeString(&pQS2->lpszServiceInstanceName, pQuerySet->lpszServiceInstanceName, &pbCurrData);
    SerializeString(&pQS2->lpszComment, pQuerySet->lpszComment, &pbCurrData);
    SerializeString(&pQS2->lpszContext, pQuerySet->lpszContext, &pbCurrData);
    SerializeString(&pQS2->lpszQueryString, pQuerySet->lpszQueryString, &pbCurrData);

    // Serialize the address field
    SerializeAddresses(&pQS2->lpcsaBuffer, pQuerySet->lpcsaBuffer, pQuerySet->dwNumberOfCsAddrs, &pbCurrData);

    // Serialize the outer blob structure
    SerializeData(&pQS2->lpBlob, pQuerySet->lpBlob, sizeof(BLOB), &pbCurrData);

    // Serialize the data pointed to by the blob
    if (pQuerySet->lpBlob != NULL)
    {
        SerializeData(&pQS2->lpBlob->pBlobData, pQuerySet->lpBlob->pBlobData, pQuerySet->lpBlob->cbSize, &pbCurrData);
    }
    
    // Serialize other pointer fields
    SerializeData(&pQS2->lpVersion, pQuerySet->lpVersion, sizeof(WSAVERSION), &pbCurrData);
    SerializeData(&pQS2->lpNSProviderId, pQuerySet->lpNSProviderId, sizeof(GUID), &pbCurrData);
    SerializeData(&pQS2->lpafpProtocols, pQuerySet->lpafpProtocols, 
                  pQuerySet->dwNumberOfProtocols * sizeof(AFPROTOCOLS), &pbCurrData);

    return NO_ERROR;
}


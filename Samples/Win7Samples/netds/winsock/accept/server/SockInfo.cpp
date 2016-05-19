// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: SockInfo.cpp
//
// Description:
//             This file contains the functions for allocating and freeing
// the SOCK_INFO structures and for adding and deleting these structures from
// the global list.

#include "common.h"

/*
    This function allocates a SOCK_INFO structure on the heap and
    initializes the contents with suitable initial values and returns
    the allocated memory.
*/
PSOCK_INFO AllocAndInitSockInfo()
{
    PSOCK_INFO pNewSockInfo;

    // allocate a new structure. this should be freed by calling FreeSockInfo
    // by the caller of this function.
    pNewSockInfo = (PSOCK_INFO) malloc(sizeof(SOCK_INFO));
    if (pNewSockInfo == NULL)
    {
        printf("AllocAndInitSockInfo: malloc returned NULL.\n");
        goto CLEANUP;
    }

    printf("Allocated SockInfo at %p\n", pNewSockInfo);

    // Initialize the SOCK_INFO structure with the suitable initial values
    // for each field. 
    memset(pNewSockInfo,0,sizeof(SOCK_INFO));
    pNewSockInfo->sock = INVALID_SOCKET;

CLEANUP:
    return pNewSockInfo;
}


/*
    This function frees the SOCK_INFO structure allocated by 
    AllocAndInitSockInfo. 
*/
void FreeSockInfo(PSOCK_INFO pSockInfo)
{
    // free the SOCK_INFO structure allocated earlier by AllocAndInitSockInfo
    free(pSockInfo);
    printf("Freed SockInfo at %p\n", pSockInfo);
    return;
}

/*
    This function adds a given sockinfo structure to head of the given list.
*/
void AddSockInfoToList(PSOCK_INFO *ppHead, PSOCK_INFO pNewSockInfo)
{
    // add the new sock info at the head, for sake of simiplicity as we
    // don't care about the order of these structures.

    // this is going to be the first node.
    pNewSockInfo->prev = NULL;

    // the earlier list follows this node.
    pNewSockInfo->next = *ppHead;

    // this node is the previous node for the earlier head node.
    if (*ppHead != NULL)
        (*ppHead)->prev = pNewSockInfo;

    // the new head is this new node, as we inserted at the head.
    *ppHead = pNewSockInfo;            
    
    printf("Added SockInfo %p to list\n", pNewSockInfo);
    return;
}
   
/*
    This function removes a given sockinfo structure from the given list
    and frees the memory also.
*/
void DeleteSockInfoFromList(PSOCK_INFO *ppHead, PSOCK_INFO pDelSockInfo)
{
    // make the previous and the next nodes to point to each other,
    // insteade of pointing to pDelSockInfo.
    if (pDelSockInfo->prev != NULL)
        pDelSockInfo->prev->next = pDelSockInfo->next;
    if (pDelSockInfo->next != NULL)
        pDelSockInfo->next->prev = pDelSockInfo->prev;

    // if the head node is being deleted, make the next node as the head.
    if (*ppHead == pDelSockInfo)
        *ppHead = pDelSockInfo->next;

    // now, pDelSockInfo can be safely deleted as nobody points to it. 
    FreeSockInfo(pDelSockInfo);
    
    printf("Deleted and freed SockInfo %p\n", pDelSockInfo);
    return;
}
   


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*
    FILE: Resources.h
    
    PURPOSE: Declarations for resource management routines and
        data structures.

*/
#pragma once

typedef struct {
    LONG Value;
    LONG Bound;
} Counter;

/*
    FUNCTION: CounterCreate

    PURPOSE: Creates a counter.

*/
Counter * CounterCreate(UINT Bound);

/*
    FUNCTION: CounterDelete

    PURPOSE: Deletes a counter.

*/
VOID CounterDelete(Counter * pCounter);

/*
    FUNCTION: CounterIncrement

    PURPOSE: Attempts to increment a counter.  If the
        incremented value is aboove the corresponding bound, does not
        increment and returns FALSE, otherwise increments and returns
        TRUE.

    RETURN VALUE:
        TRUE - icremented
        FALSE - not incremented, bound exceeded

    COMMENTS:

*/
BOOL CounterIncrement(Counter *pCounter);

/*
    FUNCTION: CounterDecrement

    PURPOSE: Decrements a couner.

    RETURN VALUE: none

    COMMENTS:

*/
VOID CounterDecrement(Counter *pCounter);

/*
    FUNCTION: CountersCreate

    PURPOSE: Creates nCounters counters that can be used to keep track of
        the resource use by several users or user groups and returns a
        pointer to the structure containing them.

    PARAMETERS:
        Counters - Array of pointers to counters.  It
      is allocate dwithin this call and needs to be freed
      with the call to CountersDelete.
        nCounters - number of counters to create.
        Bounds - array of bound values.

    RETURN VALUE:
        Pointer to the Counterst structure.

    COMMENTS:

*/
BOOL CountersCreate(Counter *Counters[], UINT nCounters, UINT * Bounds);

/*
    FUNCTION: CountersDelete

    PURPOSE: Deallocates an array of counter pointers
      and all the counters in it.

    RETURN VALUE:
      TRUE - success
      FALSE - failure

    COMMENTS:

*/
VOID CountersDelete(Counter *pCounters[], UINT n);

/*
    FUNCTION: CountersCheckForNonzero

    PURPOSE: Checks if any counter in the array
      is nonzero.

    PARAMETERS: none

    RETURN VALUE:
      TRUE - some counter is nonzero
      FALSE - otherwise

    COMMENTS: This function is not thread-safe.

*/
BOOL CountersCheckForNonzero(Counter *pCounters[], UINT n);

typedef struct _QueueNode {
    VOID *pData;
    _QueueNode *pNext;
} QueueNode;

typedef struct {
    QueueNode *pFirst;
    QueueNode *pLast;
    LPCRITICAL_SECTION lpCriticalSection;
} Queue;

/*
    FUNCTION: QueueCreate

    PURPOSE: Creates a queue.

*/
Queue * QueueCreate(BOOL NoCritSec);

/*
    FUNCTION: QueueDelete

    PURPOSE: Deletes a queue.

    NOTE: It will deallocate all node data and all nodes if the
    queue is not empty.  No error checks are performed.
*/
VOID QueueDelete(Queue * pQueue);

/*
    FUNCTION: QueueisEmpty

    PURPOSE: Returns TRUE if a queue has an elt. in it and false
    otherwise.

*/
BOOL QueueIsEmpty(Queue * pQueue);

/*
    FUNCTION: QueueAdd

    PURPOSE: Adds an element to the queue.

*/
VOID QueueAdd(Queue *pQueue, VOID *pData, BOOL EnterCritSec);

/*
    FUNCTION: QueueRemove

    PURPOSE: Removes an element from the queue.

*/
VOID * QueueRemove(Queue *pQueue);

/*
    FUNCTION: QueueRemoveData

    PURPOSE: Removes an element from the queue with a given data filed.

*/
VOID * QueueRemoveData(Queue *pQueue, VOID *pData);

/*
    FUNCTION: QueueMoveToBackNoLock

    PURPOSE: Moves an element to the back of the queue.

    COMMENTS:

*/
VOID QueueMoveToBack(Queue *pQueue, QueueNode *pNode, QueueNode *pPrevNode);

/*
    FUNCTION: QueuesCreate

    PURPOSE: Creates new queues for an array of pointers to queues.

    RETURN VALUE:
      TRUE - success
      FALSE - failure

    COMMENTS:

*/
BOOL QueuesCreate(Queue *Queues[], UINT n, BOOL NoCritSec);

/*
    FUNCTION: QueuesDelete

    PURPOSE: Deletes all queues with addresses in an array of pointers
    to queues.

*/
VOID QueuesDelete(Queue *Queues[], UINT n);

typedef struct _QueueHashNode {
    VOID *pKey;
    VOID *pValue;
} QueueHashNode;

/*
    FUNCTION: QueueHashAdd

    PURPOSE: Adds an element to the hash.

    PARAMETERS:
      pQueue - The queue of QueuehashNodes
      Sid - Sid for the user for whom we create a node
      pValue - Value associated with the Sid

*/
VOID QueueHashAdd(Queue *pQueue, PSID Sid, VOID *pValue, BOOL EnterCritSec);

/*
    FUNCTION: QueueHashLookup

    PURPOSE: Returns the element of the hash queue that
    corresponds to a given Sid.

*/
VOID * QueueHashLookup(Queue *pQueue, PSID Sid, BOOL EnterCritSec);

/*
    FUNCTION: QueueHashAddToSubqueue

    PURPOSE: The function assumes that values in the
    hash are queues.  It checks if a queue for SID pSID
    already exists, and if necessary creates a new queue
    for the SID.  It then adds the Data to the queue
    corresponding to a SID.

*/
VOID QueueHashAddToSubqueue(Queue *pHashQueue, PSID pSID, VOID * pData);

/*
    FUNCTION: QueueHashRemoveFromSubqueue

    PURPOSE: The function assumes that values in the
    hash are queues.  It removes an element from the
    first nonempty queue that it can find and puts
    that queue into the end of the hash queue.

*/
VOID * QueueHashRemoveFromSubqueue(Queue *pQueue);

/*
    FUNCTION: QueueHashIncrementCounter

    PURPOSE: The function assumes that values in the
    hash are counters.  It checks if a counter for SID pSID
    already exists, and if necessary creates a new counter
    for the SID.  It then attempts to increment that counter.

*/
BOOL QueueHashIncrementCounter(Queue *pHashQueue, PSID pSID);

/*
    FUNCTION: QueueHashDecrementCounter

    PURPOSE: The function assumes that values in the
    hash are counters.  It checks if a counter for SID pSID
    already exists, and if necessary creates a new counter
    for the SID.  It then decrements that counter.

*/
VOID QueueHashDecrementCounter(Queue *pQueue, PSID pSID);

// end Resources.h

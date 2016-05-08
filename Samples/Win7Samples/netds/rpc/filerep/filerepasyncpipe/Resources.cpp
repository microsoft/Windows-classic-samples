// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*
   

    FILE: Resources.cpp
    
    PURPOSE: Declarations for resource management routines and
        data structures.
    
    COMMENTS:  See Resources.h for function and data-type info.

*/

#include "common.h"

#include "Resources.h"
#include "Service.h"

#ifdef DEBUG2
#include "DbgMsg.h"
#endif

Counter * CounterCreate(UINT Bound) {
    Counter * pCounter;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CounterCreate\n"));
#endif

    pCounter = (Counter *)AutoHeapAlloc(sizeof(Counter));

    if (pCounter == NULL) {
        return NULL;
    }

    pCounter->Value = 0;
    pCounter->Bound = Bound;    

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CounterCreate\n"));
#endif

    return pCounter;
}

VOID CounterDelete(Counter * pCounter) {

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CounterDelete\n"));
#endif

    ASSERT(pCounter != NULL);

    AutoHeapFree(pCounter);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CounterDelete\n"));
#endif
}

BOOL CounterIncrement(Counter *pCounter) {
    BOOL ret = FALSE;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CounterIncrement\n"));
#endif

    ASSERT(pCounter != NULL);

    // Check for the bound.
    if (InterlockedIncrement(&pCounter->Value) > pCounter->Bound) {
        ret = FALSE;
        InterlockedDecrement(&pCounter->Value);
    }
    else {
        ret = TRUE;
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CounterIncrement\n"));
#endif

    return ret;
}

VOID CounterDecrement(Counter *pCounter) {

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CounterDecrement\n"));
#endif

    ASSERT(pCounter != NULL);

    InterlockedDecrement(&pCounter->Value);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CounterDecrement\n"));
#endif

}

BOOL CountersCreate(Counter *pCounters[], UINT n, UINT * Bounds) {
    BOOL ret = FALSE;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CountersCreate\n"));
#endif

    for (UINT i=0; i<n; i++) {
        if ((pCounters[i] = CounterCreate(Bounds[i])) == NULL) {
            CountersDelete(pCounters, i-1);
            ret = FALSE;
            break;
        }
    }
    ret = TRUE;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CountersCreate\n"));
#endif

    return ret;
}

VOID CountersDelete(Counter *pCounters[], UINT n) {

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> CountersDelete\n"));
#endif

    for (UINT i=0; i<n; i++) {
        ASSERT(pCounters[i] != NULL);
        CounterDelete(pCounters[i]);
        pCounters[i] = NULL;
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- CountersDelete\n"));
#endif
}

BOOL CountersCheckForNonzero(Counter *pCounters[], UINT n) {
    for (UINT i=0; i<n; i++) {
        ASSERT(pCounters[i] != NULL);
        if (pCounters[i]->Value != 0) {
            return TRUE;
        }
    }
    return FALSE;
}

Queue * QueueCreate(BOOL NoCritSec) {
    Queue * pQueue;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueCreate\n"));
#endif

    pQueue = (Queue *)AutoHeapAlloc(sizeof(Queue));

    if (pQueue == NULL) {
        AddToMessageLog(TEXT("QueueCreate: AutoHeapAlloc failed"));
        return NULL;
    }

    pQueue->pFirst = NULL;
    pQueue->pLast = NULL;    

    if (NoCritSec) {
        pQueue->lpCriticalSection = NULL;
    }
    else {
        pQueue->lpCriticalSection = (LPCRITICAL_SECTION) AutoHeapAlloc(sizeof(CRITICAL_SECTION));
        if (InitializeCriticalSectionAndSpinCount(pQueue->lpCriticalSection, 100) == 0) {
            AddToMessageLogProcFailure(TEXT("QueueCreate: InitializeCriticalSectionAndSpinCount"), GetLastError());
            QueueDelete(pQueue);
            return NULL;
        }
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueCreate\n"));
#endif

    return pQueue;
}

VOID QueueDelete(Queue * pQueue) {
    QueueNode *pNextNode;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueDelete\n"));
#endif

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL) {
        DeleteCriticalSection(pQueue->lpCriticalSection);
        AutoHeapFree(pQueue->lpCriticalSection);
    }
    
    if (pQueue->pFirst != NULL) {
        for (QueueNode *pNode = pQueue->pFirst; pNode != NULL; pNode = pNextNode) {
            pNextNode = pNode->pNext;
            AutoHeapFree(pNode->pData);
            AutoHeapFree(pNode);
        }
    }
    
    AutoHeapFree(pQueue);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueDelete\n"));
#endif
}

BOOL QueueIsEmpty(Queue * pQueue) {
    BOOL Ret = FALSE;

    ASSERT(pQueue != NULL);    

    if (pQueue->lpCriticalSection != NULL) EnterCriticalSection(pQueue->lpCriticalSection);

    Ret = (pQueue->pFirst == NULL);

    ASSERT((!Ret) || (pQueue->pFirst == pQueue->pLast));

    if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);

    return Ret;
}

VOID QueueAdd(Queue *pQueue, VOID *pData, BOOL EnterCritSec) {

    ASSERT(pQueue != NULL);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueAdd\n"));
#endif

    QueueNode * pNode = (QueueNode *) AutoHeapAlloc(sizeof(QueueNode));
    if (pNode == NULL) {
        AddToMessageLog(TEXT("QueueAdd: AutoHeapAlloc failed"));
        return;
    }

    pNode->pNext = NULL;
    pNode->pData = pData;

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) EnterCriticalSection(pQueue->lpCriticalSection);

    if (pQueue->pFirst == NULL) {
        //ASSERT(pQueue->pLast == NULL);
        pQueue->pFirst = pNode;
        pQueue->pLast = pNode;
    }
    else {
        ASSERT(pQueue->pLast->pNext == NULL);
        pQueue->pLast->pNext = pNode;
        pQueue->pLast = pNode;
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueAdd\n"));
#endif

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) LeaveCriticalSection(pQueue->lpCriticalSection);
}

VOID * QueueRemove(Queue *pQueue) {
    VOID * pData;
    QueueNode *pFuturepFirst;
    VOID * pToFree;

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL) EnterCriticalSection(pQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueRemove\n"));
#endif

    if (pQueue->pFirst == NULL) {
        ASSERT(pQueue->pFirst == pQueue->pLast);

        if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);
        return NULL;
    }

    pData = pQueue->pFirst->pData;
    pFuturepFirst = pQueue->pFirst->pNext;

    if (pFuturepFirst == NULL) {
        pQueue->pLast = NULL;
    }

    pToFree = pQueue->pFirst;
    pQueue->pFirst = pFuturepFirst;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueRemove\n"));
#endif

    if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);

    AutoHeapFree(pToFree);

    return pData;
}

VOID * QueueRemoveData(Queue *pQueue, VOID *pData) {
    QueueNode *pFuturepFirst = NULL;
    VOID *pToFree = NULL;
    QueueNode *pNodeToFree = NULL;
    QueueNode *pPrevNode = NULL; 

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL) EnterCriticalSection(pQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueRemoveData\n"));
#endif

    // Find a node to remove.
    // Walk the queue of active client requests
    for (QueueNode *pNode = pQueue->pFirst; pNode != NULL; pNode = pNode->pNext) {
        if (pNode->pData == pData) {
	  // If this is not the first node
	  if (pPrevNode) {
	    pPrevNode->pNext = pNode->pNext;
	    if (pQueue->pLast == pNode) {
	      ASSERT(pNode->pNext == NULL);
	      pQueue->pLast = pPrevNode;
	    }
	  }
	  else {
	    pQueue->pFirst = pNode->pNext;
	  }
	  pToFree = pNode;
	  break;
        }

	pPrevNode = pNode;
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueRemoveData\n"));
#endif

    if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);

    AutoHeapFree(pToFree);

    return pData;
}

VOID QueueMoveToBack(Queue *pQueue, QueueNode *pNode, QueueNode *pPrevNode) {

    ASSERT(pQueue != NULL);
    ASSERT(pNode != NULL);

    if (pQueue->lpCriticalSection != NULL) EnterCriticalSection(pQueue->lpCriticalSection);

    // If it is the first element.
    if (pPrevNode == NULL) {
        pQueue->pFirst = pNode->pNext;
    }
    // If it has a predecessor in the queue
    else {
        pPrevNode->pNext = pNode->pNext;
    }
    
    // If it is the last element.
    if (pNode->pNext == NULL) {
        if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);
        return;
    }
    
    // Put the subqueue onto the back of the queue.
    pNode->pNext = NULL;
    if (QueueIsEmpty(pQueue)) {
        pQueue->pFirst = pNode;
        pQueue->pLast = pNode;
    }
    else {
        pQueue->pLast->pNext = pNode;
        pQueue->pLast = pNode;
    }

    if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);
}

BOOL QueuesCreate(Queue *Queues[], UINT n, BOOL NoCritSec) {
    BOOL ret = FALSE;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueuesCreate\n"));
#endif

    for (UINT i=0; i<n; i++) {
        if ((Queues[i] = QueueCreate(NoCritSec)) == NULL) {
            QueuesDelete(Queues, i-1);
            ret = FALSE;
            break;
        }
    }

    ret = TRUE;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueuesCreate\n"));
#endif

    return ret;
}

VOID QueuesDelete(Queue *Queues[], UINT n) {

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueuesDelete\n"));
#endif

    for (UINT i=0; i<n; i++) {
        QueueDelete(Queues[i]);
        Queues[i] = NULL;
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueuesDelete\n"));
#endif

}

VOID QueueHashAdd(Queue *pQueue, PSID Sid, VOID *pValue, BOOL EnterCritSec) {
    QueueHashNode *pQueueHashNode;
    DWORD SidLength;

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) EnterCriticalSection(pQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashAdd\n"));
#endif

    pQueueHashNode = (QueueHashNode *) AutoHeapAlloc(sizeof(QueueHashNode));

    SidLength = GetLengthSid(Sid);

    // We need to copy the key so that if the original
    // copy gets deallocated we still have one.
    if ((pQueueHashNode->pKey = AutoHeapAlloc(SidLength)) == NULL) {
        AddToMessageLog(TEXT("QueueHashAdd: AutoHeapAlloc failed"));

        if (pQueue->lpCriticalSection != NULL && EnterCritSec) LeaveCriticalSection(pQueue->lpCriticalSection);
        return;
    }

    if (CopySid(SidLength, pQueueHashNode->pKey, Sid) == 0) {
        AddToMessageLogProcFailure(TEXT("QueueHashAdd: CopySid"), GetLastError());

        if (pQueue->lpCriticalSection != NULL && EnterCritSec) LeaveCriticalSection(pQueue->lpCriticalSection);
        return;    
    }

    pQueueHashNode->pValue = pValue;

    QueueAdd(pQueue, (VOID *) pQueueHashNode, FALSE);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashAdd\n"));
#endif

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) LeaveCriticalSection(pQueue->lpCriticalSection);
}

VOID * QueueHashLookup(Queue *pQueue, PSID Sid, BOOL EnterCritSec) {
    VOID * pValue;
    QueueNode *pNextNode;

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) EnterCriticalSection(pQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashLookup\n"));
#endif

    pValue = NULL;

    for (QueueNode *pNode = pQueue->pFirst; pNode != NULL; pNode = pNextNode) {
        pNextNode = pNode->pNext;
        if (EqualSid(((QueueHashNode *)(pNode->pData))->pKey, Sid) == TRUE) {
            pValue = ((QueueHashNode *)(pNode->pData))->pValue;
            break;
        }
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashLookup\n"));
#endif

    if (pQueue->lpCriticalSection != NULL && EnterCritSec) LeaveCriticalSection(pQueue->lpCriticalSection);

    return pValue;
}

VOID QueueHashAddToSubqueue(Queue *pHashQueue, PSID pSID, VOID * pData) {

    ASSERT(pHashQueue != NULL);

    if (pHashQueue->lpCriticalSection != NULL) EnterCriticalSection(pHashQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashAddToSubqueue\n"));
#endif

    // Put the request onto the appropriate queue.

    // Lookup the subqueue for the user's sid.
    Queue * pQueue = (Queue *) QueueHashLookup(pHashQueue, pSID, FALSE);
    // If the subqueue does not exist, create it.
    if (pQueue == NULL) {
        if ((pQueue = QueueCreate(TRUE)) == NULL) {
            AddToMessageLog(TEXT("QueueHashAddToSubqueue: QueueCreate failed"));
            if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);
            return;
	}
        QueueHashAdd(pHashQueue, pSID, pQueue, FALSE);
    }
    // Add the request to the subqueue
    QueueAdd(pQueue, pData, FALSE);   

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashAddToSubqueue\n"));
#endif

    if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);
}

VOID * QueueHashRemoveFromSubqueue(Queue *pQueue) {
    Queue * pSubQueue;
    QueueNode *pPrevNode;
    VOID * pData;

    ASSERT(pQueue != NULL);

    if (pQueue->lpCriticalSection != NULL) EnterCriticalSection(pQueue->lpCriticalSection);

    pSubQueue = NULL;

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashRemoveFromSubqueue\n"));
#endif

    // If the hash queue is not empty.
    if (pQueue->pFirst != NULL) {
        // Find the first nonempty subqueue.
        pPrevNode = NULL;
        for (QueueNode *pNode = pQueue->pFirst; pNode != NULL; pNode = pNode->pNext) {

            // Check if it has elements in it.
            if (!QueueIsEmpty((Queue*) ((QueueHashNode *)pNode->pData)->pValue)) {
                // Select it.  We will extract an element off this subqueue
                // and put the subqueue into the end of the hash queue.
                pSubQueue = (Queue*) ((QueueHashNode *)pNode->pData)->pValue;

                // Take it off the hash queue and put it
                // on the back.
                QueueMoveToBack(pQueue, pNode, pPrevNode);

                break;
            }

            pPrevNode = pNode;
        }
        if (pSubQueue == NULL) {
            // We did not find any nonemoty subqueues.
            pData = NULL;
        }
    }
    else {
        // There are no nonempty subqueues.
        pData = NULL;
    }

    if (pSubQueue != NULL) {
        // Extract an element from the subqueue.
        pData = QueueRemove(pSubQueue);
    }

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashRemoveFromSubqueue\n"));
#endif

    if (pQueue->lpCriticalSection != NULL) LeaveCriticalSection(pQueue->lpCriticalSection);

    return pData;
}

BOOL QueueHashIncrementCounter(Queue *pHashQueue, PSID pSID) {
    Counter *pCounter;
    BOOL ret;

    ASSERT(pHashQueue != NULL);

    if (pHashQueue->lpCriticalSection != NULL) EnterCriticalSection(pHashQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashIncrementCounter\n"));
#endif

    // Put the counter onto the appropriate queue.
    // Lookup the counter for the user's sid.
    pCounter = (Counter *) QueueHashLookup(pHashQueue, pSID, FALSE);
    // If the counter does not exist, create it.
    if (pCounter == NULL) {
        if ((pCounter = CounterCreate(MaxUserReqs)) == NULL) {
            AddToMessageLog(TEXT("QueueHashIncrementCounter: CounterCreate failed"));
            if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);
            return FALSE;
        }
        QueueHashAdd(pHashQueue, pSID, pCounter, FALSE);
    }
    // Attempt to increment the counter
    ret = CounterIncrement(pCounter);   

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashIncrementCounter\n"));
#endif

    if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);

    return ret;
}

VOID QueueHashDecrementCounter(Queue *pHashQueue, PSID pSID) {
    Counter *pCounter;

    ASSERT(pHashQueue != NULL);

    if (pHashQueue->lpCriticalSection != NULL) EnterCriticalSection(pHashQueue->lpCriticalSection);

#ifdef DEBUG2
    DbgMsgRecord(TEXT("-> QueueHashDecrementCounter\n"));
#endif

    // Put the counter onto the appropriate queue.
    // Lookup the counter for the user's sid.
    pCounter = (Counter *) QueueHashLookup(pHashQueue, pSID, FALSE);
    // If the counter does not exist, create it.
    if (pCounter == NULL) {
        if ((pCounter = CounterCreate(MaxUserReqs)) == NULL) {
            AddToMessageLog(TEXT("QueueHashIncrementCounter: CounterCreate failed"));
            if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);
            return;
        }
        QueueHashAdd(pHashQueue, pSID, pCounter, FALSE);
    }
    // Decrement the counter.
    CounterDecrement(pCounter);   

#ifdef DEBUG2
    DbgMsgRecord(TEXT("<- QueueHashDecrementCounter\n"));
#endif

    if (pHashQueue->lpCriticalSection != NULL) LeaveCriticalSection(pHashQueue->lpCriticalSection);
}

// end Resources.cpp









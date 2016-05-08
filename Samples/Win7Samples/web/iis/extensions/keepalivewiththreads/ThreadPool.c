/* Copyright (c) 1997-2002 Microsoft Corporation

	Module Name:
	
		ThreadPool.c

	Abstract:

		Work queue management functions.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <httpext.h>
#include "threadpool.h"

/* Structure to create simple linked list */

typedef struct {
	EXTENSION_CONTROL_BLOCK *pECB;	/* Data for list entry */
	DWORD dwNextEntry;							/* Pointer to next entry */
} ECB_QUEUE_ENTRY;

/* Array that is a simple linked list */

ECB_QUEUE_ENTRY ECBqueue[WORK_QUEUE_ENTRIES];

/* Index of next ECBqueue entry to use, and last Entry in use. */

DWORD dwCurrentEntry, dwLastEntry;

/* Flag to indicate that there are no other entries in the ECBqueue */

BOOL fQueueEmpty;

/*
	Description:

		Initialize our thread pool
*/

BOOL InitThreadPool(void)
{
	DWORD i;
	DWORD dwThreadID;

	/* Create Semaphore in nonsignaled state */

	if ((hWorkSem = CreateSemaphore(NULL, 0, 0x7fffffff, NULL)) == NULL)
		return FALSE;

	InitializeCriticalSection(&csQueueLock);

	fQueueEmpty = TRUE;

	/* Create Pool Threads */

	for (i = 0; i < POOL_THREADS; i++)
		if (CreateThread(NULL, 0, WorkerFunction, (LPVOID)i, 0, &dwThreadID) == NULL)
			return FALSE;

	/* Clear work queue */

	ZeroMemory(ECBqueue, WORK_QUEUE_ENTRIES * sizeof(ECB_QUEUE_ENTRY));

	return TRUE;
}

/*
	Description:

		Add single work unit to the queue

	Arguments:

		pECB - pointer to the extension control block

	Returns:

		TRUE if the unit was successfully queued
		FALSE if queue is full
*/

BOOL AddWorkQueueEntry(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	DWORD i;

	for (i = 0; i < WORK_QUEUE_ENTRIES; i++) {

		if (ECBqueue[i].pECB == NULL) {

			if (fQueueEmpty) {

				dwCurrentEntry = i;

				fQueueEmpty = FALSE;

			} else {

				ECBqueue[dwLastEntry].dwNextEntry = i;
			}

			ECBqueue[i].pECB = pECB;

			dwLastEntry = i;

			return TRUE;
		}
	}

	/* If no NULL queue entry found, indicate failure */

	return FALSE;
}

BOOL GetWorkQueueEntry(OUT EXTENSION_CONTROL_BLOCK **ppECB)
{
	if ((*ppECB = ECBqueue[dwCurrentEntry].pECB) == NULL) {

		return FALSE;

	} else {

		ECBqueue[dwCurrentEntry].pECB = NULL;

		if (dwCurrentEntry == dwLastEntry)	/* If this is only pending item */
			fQueueEmpty = TRUE;
		else
			dwCurrentEntry = ECBqueue[dwCurrentEntry].dwNextEntry;
	}

	return TRUE;
}

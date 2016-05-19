/* Copyright (c) 1997-2002  Microsoft Corporation

	Module Name:

		ThreadPool.h
*/

/* Number of threads in pool */

#define POOL_THREADS 2

/* Number of entries in ECBqueue */

#define WORK_QUEUE_ENTRIES 2

/* Global critical section to control access to ECB queue */

CRITICAL_SECTION csQueueLock;

/* 
	Semaphore to wait on in worker thread; each time an ECB is added to the 
	ECBqueue by HttpExtensionProc, the semaphore must be released once
*/

HANDLE hWorkSem;

/*
	These functions will add/retrieve an ECB to/from the linked list.
	ENTER csQueueLock BEFORE CALLING AND LEAVE csQueueLock AFTER
	RETURNING FROM THESE FUNCTIONS!!!
*/

BOOL AddWorkQueueEntry(EXTENSION_CONTROL_BLOCK *);
BOOL GetWorkQueueEntry(EXTENSION_CONTROL_BLOCK **ppECB);

/* This function initializes the thread pool */

BOOL InitThreadPool(void);

/* Function that threads in pool run */

DWORD WINAPI WorkerFunction(LPVOID); 

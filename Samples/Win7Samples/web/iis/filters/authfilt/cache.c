/*
Copyright (c) 1996-2002  Microsoft Corporation

This program is released into the public domain for any purpose.

Module Name:

    cache.c

Abstract:

    This module implements a simple user cache.  The cached users are kept
    in an LRU sorted list.  If there will be a large number of simultaneous
    users, then a sorted array would be more appropriate.
*/

#include <windows.h>
#include <httpfilt.h>
#include "authfilt.h"

// Constants

// The maximum number of users we will cache.  If there will be a large number
// of simultaneous users, bump this value

#define MAX_CACHED_USERS 100

// The position after which we'll move a cache entry to the front of the list

#define LIST_REORDER_THRESHOLD 6

// Cached user structure

typedef struct _USER_INFO
{
	LIST_ENTRY  ListEntry;  // Double linked list entry

	CHAR  achUserName[SF_MAX_USERNAME];   // External username and password
	CHAR  achPassword[SF_MAX_PASSWORD];
	CHAR  achNTUserName[SF_MAX_USERNAME]; // account and password to map user to
	CHAR  achNTUserPassword[SF_MAX_PASSWORD];

} USER_INFO, *PUSER_INFO;

// Globals

// Circular double linked list of cached users

LIST_ENTRY CacheListHead;

// Critical section protects cache list

CRITICAL_SECTION csCacheLock;

// Indicates whether we are initialized

BOOL fCacheInitialized = FALSE;

// Number of items in the cache

DWORD cCacheItems = 0;

/*
Description:

	Initializes the cache module

Return Value:

	TRUE if initialized successfully, FALSE on error
*/

BOOL InitializeCache(VOID)
{
	if (fCacheInitialized)
		return TRUE;

	InitializeCriticalSection(&csCacheLock);

	CacheListHead.Blink = CacheListHead.Flink = &CacheListHead;

	fCacheInitialized = TRUE;

	return TRUE;
}

/*
Description:

	Checks to see if a user is in the cache and returns the user properties
	if found

Arguments:

	pszUserName       - Case insensitive username to find
	pfFound           - Set to TRUE if the specified user was found
	pszPassword       - Receives password for specified user if found
	pszNTUser         - Receives the NT Username to map this user to
	pszNTUserPassword - Receives the NT Password for pszNTUser

	Note: pszPassword and pszNTUserPassword must be at least SF_MAX_PASSWORD
	characters.  pszNTUser must be at least SF_MAX_USERNAME characters.

Return Value:

	TRUE if no errors occurred.
*/

BOOL LookupUserInCache(CHAR *pszUserName, BOOL *pfFound, CHAR *pszPassword, CHAR *pszNTUser, CHAR *pszNTUserPassword)
{
	LIST_ENTRY *pEntry;
	USER_INFO *pUser;
	DWORD cPosition = 0;

	/* Search the cache for the specified user */

	EnterCriticalSection(&csCacheLock);

	for (pEntry = CacheListHead.Flink; pEntry != &CacheListHead; pEntry = pEntry->Flink)
	{
		pUser = CONTAINING_RECORD(pEntry, USER_INFO, ListEntry);

		if (!_stricmp( pszUserName, pUser->achUserName))
			goto Found;

		cPosition++;
	}

	LeaveCriticalSection(&csCacheLock);

	/* Not Found */

	*pfFound = FALSE;

	return TRUE;

Found:

	/* Copy out the user properties */

	strcpy_s(pszPassword, sizeof(pszPassword),       pUser->achPassword);
	strcpy_s(pszNTUser,  sizeof(pszNTUser),        pUser->achNTUserName);
	strcpy_s(pszNTUserPassword, sizeof(pszNTUserPassword), pUser->achNTUserPassword);

	/*
		Move this user entry to the front of the list as we're probably going
	  to get subsequent requests for this user.  Note we only move it
	  if it's not already near the front
	*/

	if (cPosition > LIST_REORDER_THRESHOLD)
	{
		/* Remove from the old position... */

		pEntry->Blink->Flink = pEntry->Flink;
		pEntry->Flink->Blink = pEntry->Blink;

		/* ...and insert it at the beginning of the list */

		pEntry->Blink = &CacheListHead;
		pEntry->Flink = CacheListHead.Flink;

		CacheListHead.Flink->Blink = pEntry;
		CacheListHead.Flink = pEntry;
	}

	LeaveCriticalSection(&csCacheLock);

	*pfFound = TRUE;

	return TRUE;
}

/*
	Description:

		Adds the specified user to the cache

	Arguments:

		pszUserName - Username to add
		pszPassword - Contains the external password for this user
		pszNTUser   - Contains the NT user name to use for this user
		pszNTUserPassword - Contains the password for NTUser

	Return Value:

		TRUE if no errors occurred.
*/

BOOL AddUserToCache(CHAR *pszUserName, CHAR *pszPassword, CHAR *pszNTUser, CHAR *pszNTUserPassword)
{
	LIST_ENTRY *pEntry;
	USER_INFO *pUser;

	/* Check our parameters before adding them to the cache */

	if (strlen(pszUserName) > SF_MAX_USERNAME ||
			strlen(pszPassword) > SF_MAX_PASSWORD ||
			strlen(pszNTUser) > SF_MAX_USERNAME ||
			strlen(pszNTUserPassword) > SF_MAX_PASSWORD)
	{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
	}

	/* Search the cache for the specified user to make sure there are no duplicates */

	EnterCriticalSection(&csCacheLock);

	for (pEntry = CacheListHead.Flink; pEntry != &CacheListHead; pEntry = pEntry->Flink)
	{
		pUser = CONTAINING_RECORD(pEntry, USER_INFO, ListEntry);

		if (!_stricmp(pszUserName, pUser->achUserName))
			goto Found;
	}

	/* Allocate a new cache item and put it at the head of the list */

	pUser = (USER_INFO *)LocalAlloc(LPTR, sizeof(USER_INFO));

	if (!pUser)
	{
		LeaveCriticalSection(&csCacheLock);

		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	pUser->ListEntry.Flink = CacheListHead.Flink;
	pUser->ListEntry.Blink = &CacheListHead;

	CacheListHead.Flink->Blink = &pUser->ListEntry;
	CacheListHead.Flink = &pUser->ListEntry;

Found:

	/* Set the various fields */

	strcpy_s(pUser->achUserName, sizeof(pUser->achUserName), pszUserName);
	strcpy_s(pUser->achPassword, sizeof(pUser->achPassword), pszPassword);
	strcpy_s(pUser->achNTUserName, sizeof(pUser->achNTUserName), pszNTUser);
	strcpy_s(pUser->achNTUserPassword, sizeof(pUser->achNTUserPassword), pszNTUserPassword);

	cCacheItems++;

	/* If there are too many cached users, remove the least recently used one now */

	if (cCacheItems > MAX_CACHED_USERS)
	{
		pEntry = CacheListHead.Blink;

		pEntry->Blink->Flink = &CacheListHead;
		CacheListHead.Blink  = pEntry->Blink;

		LocalFree(CONTAINING_RECORD(pEntry, USER_INFO, ListEntry));

		cCacheItems--;
	}

	LeaveCriticalSection(&csCacheLock);

	return TRUE;
}

/*
	Description:

		Terminates the cache module and frees any allocated memory
*/

VOID TerminateCache(VOID)
{
	LIST_ENTRY *pEntry;
	LIST_ENTRY *pEntryNext;
	USER_INFO *pUser;

	if (!fCacheInitialized)
		return;

	EnterCriticalSection(&csCacheLock);

	/* Free all of the cache entries */

	for (pEntry = CacheListHead.Flink; pEntry != &CacheListHead; pEntry  = pEntryNext)
	{
		pUser = CONTAINING_RECORD(pEntry, USER_INFO, ListEntry);

		pEntryNext = pEntry->Flink;

		/* Remove this entry from the list and free it */

		pEntry->Blink->Flink = pEntry->Flink;
		pEntry->Flink->Blink = pEntry->Blink;

		LocalFree(pUser);
	}

	cCacheItems = 0;

	LeaveCriticalSection(&csCacheLock);

	DeleteCriticalSection(&csCacheLock);

	fCacheInitialized = FALSE;
}

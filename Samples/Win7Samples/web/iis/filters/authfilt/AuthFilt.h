/* Copyright (c) 1996-2002 Microsoft Corporation

Module Name:

    authfilt.h

Abstract:

    This module contains the common definitions for the authentication filter
    sample
*/

#ifndef _AUTHFILT_H_
#define _AUTHFILT_H_

// Constants

#define ISWHITE(ch) ((ch) && ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r'))

#if DBG
	#define DEST							buff
	#define DbgWrite(x)				{                                 \
                              char buff[256];                 \
                              swprintf_s x;                     \
                              OutputDebugString( buff );      \
														}
#else
	#define DbgWrite(x)      /* nothing */
#endif

// Database routines

BOOL InitializeUserDatabase(VOID);

BOOL ValidateUser(CHAR *pszUserName, CHAR *pszPassword, BOOL *pfValid);

BOOL LookupUserInDb(
    IN CHAR *pszUser,
    OUT BOOL *pfFound,
    OUT CHAR *pszPassword,
    OUT CHAR *pszNTUser,
    OUT CHAR *pszNTUserPassword
    );

VOID TerminateUserDatabase(VOID);

// Cache routines

BOOL InitializeCache(VOID);

BOOL LookupUserInCache(
    CHAR *pszUserName,
    BOOL *pfFound,
    CHAR *pszPassword,
    CHAR *pszNTUser,
    CHAR *pszNTUserPassword
    );

BOOL AddUserToCache(
    CHAR *pszUserName,
    CHAR *pszPassword,
    CHAR *pszNTUser,
    CHAR *pszNTUserPassword
    );

VOID TerminateCache(VOID);

#endif //_AUTHFILT_H_

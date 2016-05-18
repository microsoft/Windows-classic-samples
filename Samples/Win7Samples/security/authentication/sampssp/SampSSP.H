/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1992 - 2000.  Microsoft Corporation.  All rights reserved.
----------------------------------------------------------------------------*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif // SECURITY_WIN32
#include <sspi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern CRITICAL_SECTION DllCritSect;

typedef struct _SEC_CONTEXT {
    struct _SEC_CONTEXT * Next;
    ULONG ContextId;
    ULONG CredentialId;
    ULONG Nonce;
    ULONG ContextFlags;
    enum {
        Idle,
        FirstInit,
        FirstAccept,
        SecondInit,
        SecondAccept
    } State;
} SEC_CONTEXT, *PSEC_CONTEXT;

typedef struct _CREDENTIAL {
    struct _CREDENTIAL *Next;
    ULONG CredentialId;
    ULONG Use;
    ULONG * AuthData;
} CREDENTIAL, *PCREDENTIAL;

typedef enum {
    Negotiate,
    Challenge,
    ChallengeResponse,
    ReAuthenticate
} MESSAGE_TYPE;

#define MESSAGE_SIZE 10

typedef struct _MESSAGE {
    MESSAGE_TYPE MessageType;
    UCHAR Buffer[MESSAGE_SIZE];
} MESSAGE, *PMESSAGE;

typedef struct _SIGNATURE {
    ULONG MessageId;
    ULONG Tag;
} SIGNATURE, *PSIGNATURE;



#define PACKAGE_NAME            L"SAMPSSP"
#define PACKAGE_COMMENT         L"Sample package for testing Security Providers"
#define PACKAGE_CAPABILITIES    (SECPKG_FLAG_TOKEN_ONLY | \
                                 SECPKG_FLAG_MULTI_REQUIRED | \
                                 SECPKG_FLAG_CONNECTION | \
                                 SECPKG_FLAG_INTEGRITY | \
                                 SECPKG_FLAG_PRIVACY)
#define PACKAGE_VERSION         1
//
// This is the number we enter in
// Registry\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Rpc\SecurityService
// for example: 123 = REG_SZ "sampssp.dll"
//
#define PACKAGE_RPCID           123
#define PACKAGE_MAXTOKEN        sizeof(MESSAGE)
#define PACKAGE_SIGNATURE_SIZE  sizeof(SIGNATURE)
#define PACKAGE_SIGNATURE_TAG   0xbaadf00d



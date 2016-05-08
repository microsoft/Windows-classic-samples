/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1987 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:
    subauth.c

Abstract:
    Sample Kerberos SubAuthentication Package.

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

--*/

#define _UNICODE
#define UNICODE
#include <windef.h>
#include <windows.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <subauth.h>
#include <stdio.h>

static BOOL WriteLogFile(LPTSTR String);

NTSTATUS
NTAPI
Msv1_0SubAuthenticationFilter (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
)
/*++

Routine Description:

    The subauthentication routine does client/server specific authentication
    of a user. The credentials of the user are passed in addition to all the
    information from SAM defining the user. This routine decides whether to
    let the user log on.


Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.

    Flags - Flags describing the circumstances of the logon.

        MSV1_0_PASSTHRU -- This is a PassThru authenication.  (i.e., the
            user isn't connecting to this machine.)
        MSV1_0_GUEST_LOGON -- This is a retry of the logon using the GUEST
            user account.

    UserAll -- The description of the user as returned from SAM.

    WhichFields -- Returns which fields from UserAllInfo are to be written
        back to SAM.  The fields will only be written if MSV returns success
        to it's caller.  Only the following bits are valid.

        USER_ALL_PARAMETERS - Write UserAllInfo->Parameters back to SAM.  If
            the size of the buffer is changed, the old buffer must be deleted 
            using MIDL_user_free() and reallocate the
            buffer using MIDL_user_allocate().

    UserFlags -- Returns UserFlags to be returned from LsaLogonUser in the
        LogonProfile.  The following bits are currently defined:


            LOGON_GUEST -- This was a guest logon
            LOGON_NOENCRYPTION -- The caller didn't specify encrypted credentials

        SubAuthentication packages should restrict themselves to returning
        bits in the high order byte of UserFlags.  However, this convention
        isn't enforced giving the SubAuthentication package more flexibility.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

    LogoffTime - Receives the time at which the user should log off the
        system.  This time is specified as a GMT relative NT system time.

    KickoffTime - Receives the time at which the user should be kicked
        off the system. This time is specified as a GMT relative system
        time.  Specify, a full scale positive number if the user isn't to
        be kicked off.

Return Value:

    STATUS_SUCCESS: if there was no error.

    STATUS_NO_SUCH_USER: The specified user has no account.
    STATUS_WRONG_PASSWORD: The password was invalid.

    STATUS_INVALID_INFO_CLASS: LogonLevel is invalid.
    STATUS_ACCOUNT_LOCKED_OUT: The account is locked out
    STATUS_ACCOUNT_DISABLED: The account is disabled
    STATUS_ACCOUNT_EXPIRED: The account has expired.
    STATUS_PASSWORD_MUST_CHANGE: Account is marked as Password must change
        on next logon.
    STATUS_PASSWORD_EXPIRED: The Password is expired.
    STATUS_INVALID_LOGON_HOURS - The user is not authorized to log on at
        this time.
    STATUS_INVALID_WORKSTATION - The user is not authorized to log on to
        the specified workstation.

--*/
{
    NTSTATUS Status;
    SYSTEMTIME CurrentTime;
    WCHAR buf[256];
    PNETLOGON_LOGON_IDENTITY_INFO Identity =
    (PNETLOGON_LOGON_IDENTITY_INFO)LogonInformation;
    
    Status = STATUS_SUCCESS;
    
    *Authoritative = TRUE;
    *UserFlags = 0;
    *WhichFields = 0;


    GetLocalTime( &CurrentTime );

    if (!Identity) {
    WriteLogFile(TEXT("No identity\r\n"));
    return Status;
    }
    
    
    swprintf_s(buf, RTL_NUMBER_OF(buf),
         L"%02d/%02d/%d %02d:%02d:%02d: Logon (level=%d) %wZ\\%wZ (%wZ) from %wZ\r\n",
         CurrentTime.wMonth, CurrentTime.wDay, CurrentTime.wYear,
         CurrentTime.wHour, CurrentTime.wMinute, CurrentTime.wSecond,
         LogonLevel,
         &Identity->LogonDomainName, &Identity->UserName,
         &UserAll->FullName, &Identity->Workstation);
    WriteLogFile(buf);

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:
    case NetlogonNetworkInformation:

    //
    // If you care you can determine what to do here
    //
        *Authoritative = FALSE;

    if (LogoffTime) {
        LogoffTime->HighPart = 0x7FFFFFFF;
        LogoffTime->LowPart = 0xFFFFFFFF;
    }

    if (KickoffTime) {
        KickoffTime->HighPart = 0x7FFFFFFF;
        KickoffTime->LowPart = 0xFFFFFFFF;
    }
        break;

    default:
        return STATUS_INVALID_INFO_CLASS;
    }

    return Status;
}

#define LOGFILE L"C:\\lastlog.txt"

static BOOL
WriteLogFile(
    LPWSTR String
    )
{
    HANDLE hFile;
    DWORD dwBytesWritten;

    hFile = CreateFile(LOGFILE,
               GENERIC_WRITE,
               0,
               NULL,
               OPEN_ALWAYS,
               FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
               NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    return FALSE;

    SetFilePointer(hFile, 0, NULL, FILE_END);

    WriteFile(hFile,
          String,
          (lstrlen(String) * sizeof(WCHAR)),
          &dwBytesWritten,
          NULL);

    CloseHandle(hFile);

    return TRUE;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include "ntsecapi.h"
#include "dcomperm.h"

/*---------------------------------------------------------------------------*\
 * NAME: GetRunAsPassword                                                    *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Retrieves the password for the given application.            *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszAppID - The Application ID you wish to modify                         *
 *  (e.g. "{99999999-9999-9999-9999-00AA00BBF7C7}")                          *
 *                                                                           *
 *  tszPassword - Password of the user you have specified in the RunAs       *
 *  named value under the AppID registry key.                                *
 *                                                                           *
 * --------------------------------------------------------------------------*
 *  RETURNS: WIN32 Error Code                                                *
\*---------------------------------------------------------------------------*/
DWORD GetRunAsPassword (
    LPTSTR tszAppID,
    LPTSTR tszPassword
    )
{
    LSA_OBJECT_ATTRIBUTES objectAttributes             = {0};
    HANDLE                hPolicy                      = NULL;
    LSA_UNICODE_STRING    lsaKeyString                 = {0};
    PLSA_UNICODE_STRING   lsaPasswordString            = {0};
    WCHAR                 wszKey [4 + GUIDSTR_MAX + 1] = {0};
    WCHAR                 wszAppID [GUIDSTR_MAX + 1]   = {0};
    DWORD                 dwReturnValue                = ERROR_SUCCESS;

#ifndef UNICODE
    STR2UNI (wszAppID, tszAppID);
#else
    StringCchCopy (wszAppID, RTL_NUMBER_OF(wszAppID), tszAppID);
#endif

    StringCchCopyW(wszKey, RTL_NUMBER_OF(wszKey), L"SCM:");
    StringCchCatW(wszKey, RTL_NUMBER_OF(wszKey), wszAppID);

    lsaKeyString.Length = (USHORT) ((wcslen (wszKey) + 1) * sizeof (WCHAR));
    lsaKeyString.MaximumLength = sizeof(wszKey);
    lsaKeyString.Buffer = wszKey;

    // Open the local security policy
    objectAttributes.Length = sizeof (LSA_OBJECT_ATTRIBUTES);

    dwReturnValue = LsaOpenPolicy (NULL,
                                 &objectAttributes,
                                 POLICY_GET_PRIVATE_INFORMATION,
                                 &hPolicy);

    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    // Read the user's password
    dwReturnValue = LsaRetrievePrivateData (hPolicy,
                                          &lsaKeyString,
                                          &lsaPasswordString);
                                          
    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

#ifndef UNICODE
    UNI2STR (tszPassword, lsaPasswordString->Buffer);
#else
    StringCbCopyW(tszPassword, lsaPasswordString->Length, lsaPasswordString->Buffer);
#endif

CLEANUP:

    if(hPolicy) LsaClose (hPolicy);

    if (lsaPasswordString)
		LsaFreeMemory(lsaPasswordString);

    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: SetRunAsPassword                                                    *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Sets the RunAs password for an AppID. Note that if you       *
 * have specified the RunAs named value to "Interactive User" you do not     *
 * need to set the RunAs password.                                           *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszAppID - The Application ID you wish to modify                         *
 *  (e.g. "{99999999-9999-9999-9999-00AA00BBF7C7}")                          *
 *                                                                           *
 *  tszPrincipal - Name of the principal you have specified in the RunAs     *
 *  named value under the AppID registry key                                 *
 *                                                                           *
 *  tszPassword - Password of the user you have specified in the RunAs       *
 *  named value under the AppID registry key.                                *
 * --------------------------------------------------------------------------*
 *  RETURNS: WIN32 Error Code                                                *
\*---------------------------------------------------------------------------*/
DWORD SetRunAsPassword (
    LPTSTR tszAppID,
    LPTSTR tszPrincipal,
    LPTSTR tszPassword
    )
{
    LSA_OBJECT_ATTRIBUTES objectAttributes             = {0};
    HANDLE                hPolicy                      = NULL;
    LSA_UNICODE_STRING    lsaKeyString                 = {0};
    LSA_UNICODE_STRING    lsaPasswordString            = {0};
    WCHAR                 wszKey [4 + GUIDSTR_MAX + 1] = {0};
    WCHAR                 wszAppID [GUIDSTR_MAX + 1]   = {0};
    WCHAR                 wszPassword [256]            = {0};
    DWORD                 dwReturnValue                = ERROR_SUCCESS;

#ifndef UNICODE
    STR2UNI (wszAppID, tszAppID);
    STR2UNI (wszPassword, tszPassword);
#else
    StringCchCopy (wszAppID, RTL_NUMBER_OF(wszAppID), tszAppID);
    StringCchCopy (wszPassword, RTL_NUMBER_OF(wszPassword), tszPassword);
#endif

    StringCchCopyW(wszKey, RTL_NUMBER_OF(wszKey), L"SCM:");
    StringCchCatW(wszKey, RTL_NUMBER_OF(wszKey), wszAppID);

    lsaKeyString.Length = (USHORT) ((wcslen (wszKey) + 1) * sizeof (WCHAR));
    lsaKeyString.MaximumLength = sizeof(wszKey);
    lsaKeyString.Buffer = wszKey;

    lsaPasswordString.Length = (USHORT) ((wcslen (wszPassword) + 1) * sizeof (WCHAR));
    lsaPasswordString.Buffer = wszPassword;
    lsaPasswordString.MaximumLength = lsaPasswordString.Length;

    // Open the local security policy
    objectAttributes.Length = sizeof (LSA_OBJECT_ATTRIBUTES);

    dwReturnValue = LsaOpenPolicy (NULL,
                                 &objectAttributes,
                                 POLICY_CREATE_SECRET,
                                 &hPolicy);

    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    // Store the user's password
    dwReturnValue = LsaStorePrivateData (hPolicy,
                                       &lsaKeyString,
                                       &lsaPasswordString);

    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;


    dwReturnValue = SetAccountRights (tszPrincipal, _T("SeBatchLogonRight"));
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

CLEANUP:

    if(hPolicy) LsaClose (hPolicy);

    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: SetAccountRights                                                    *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Sets the account right for a given user.                     *
\*---------------------------------------------------------------------------*/
DWORD SetAccountRights (
    LPTSTR tszUser,
    LPTSTR tszPrivilege
    )
{
    LSA_HANDLE            hPolicy            = NULL;
    LSA_OBJECT_ATTRIBUTES objectAttributes   = {0};
    PSID                  psidPrincipal      = NULL;
    LSA_UNICODE_STRING    lsaPrivilegeString = {0};
    WCHAR                 wszPrivilege [256] = {0};
    DWORD                dwReturnValue       = ERROR_SUCCESS;

#ifdef _UNICODE
    StringCchCopy (wszPrivilege, RTL_NUMBER_OF(wszPrivilege),tszPrivilege);
#else
    STR2UNI (wszPrivilege, tszPrivilege);
#endif

    dwReturnValue = LsaOpenPolicy (NULL,
                                   &objectAttributes,
                                   POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                                   &hPolicy);
                                   
    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    dwReturnValue = GetPrincipalSID (tszUser, &psidPrincipal);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    lsaPrivilegeString.Length = (USHORT) (wcslen (wszPrivilege) * sizeof (WCHAR));
    lsaPrivilegeString.MaximumLength = (USHORT) (lsaPrivilegeString.Length + sizeof (WCHAR));
    lsaPrivilegeString.Buffer = wszPrivilege;

    dwReturnValue = LsaAddAccountRights (hPolicy,
                                         psidPrincipal,
                                         &lsaPrivilegeString,
                                         1);

    dwReturnValue = LsaNtStatusToWinError(dwReturnValue); 

    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;
    

CLEANUP:

    if(psidPrincipal) free (psidPrincipal);
    if(hPolicy) LsaClose (hPolicy);

    return dwReturnValue;
}

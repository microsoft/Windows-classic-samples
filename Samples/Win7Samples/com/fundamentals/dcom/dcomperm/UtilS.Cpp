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
#include "ntsecapi.h"
#include "dcomperm.h"

#define VERSION_MAJOR_W2K_XP_W2K3 5 
#define VERSION_MINOR_W2K         0 
#define VERSION_MINOR_XP          1 
#define VERSION_MINOR_W2K3        2 


/*---------------------------------------------------------------------------*\
 * NAME: GetCurrentUserSID                                                   *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Returns a SID for the current user.                          *
\*---------------------------------------------------------------------------*/
DWORD GetCurrentUserSID (
    PSID *pSid
    )
{
    TOKEN_USER  *ptokenUser = NULL;
    HANDLE      hToken = NULL;
    DWORD       cbToken = 0;
    DWORD       cbSid = 0;
    DWORD       dwReturnValue = ERROR_SUCCESS;

    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
            dwReturnValue = GetLastError();
            goto CLEANUP;
    }


    GetTokenInformation (hToken,
                         TokenUser,
                         ptokenUser,
                         0,
                         &cbToken);

    ptokenUser = (TOKEN_USER *) malloc (cbToken);

 if(!ptokenUser)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }
	   
 if (!GetTokenInformation (hToken,
                             TokenUser,
                             ptokenUser,
                             cbToken,
                             &cbToken))
    {
            dwReturnValue = GetLastError();
            goto CLEANUP;
    }


    cbSid = GetLengthSid (ptokenUser->User.Sid);
    *pSid = (PSID) malloc (cbSid);

 if(!*pSid)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }
 
    memcpy (*pSid, ptokenUser->User.Sid, cbSid);
    CloseHandle (hToken);


CLEANUP:

    if (ptokenUser)
    	free (ptokenUser);
    
    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: ConstructWellKnownSID                                               *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: This method converts some designated well-known identities   *
 * to a SID.                                                                 *
\*---------------------------------------------------------------------------*/
BOOL ConstructWellKnownSID(
    LPTSTR tszPrincipal,
    PSID *pSid
    )
{

    BOOL fRetVal = FALSE;
    PSID psidTemp = NULL;
    BOOL fUseWorldAuth = FALSE;
    
    SID_IDENTIFIER_AUTHORITY SidAuthorityNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidAuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;

    DWORD dwSubAuthority;

    // Look for well-known English names
    if (_tcsicmp (tszPrincipal, _T("Administrators")) == 0) 
    {
        dwSubAuthority = DOMAIN_ALIAS_RID_ADMINS;
    } 
    else if (_tcsicmp (tszPrincipal, _T("Power Users")) == 0) 
    {
        dwSubAuthority = DOMAIN_ALIAS_RID_POWER_USERS;
    } 
    else if (_tcsicmp (tszPrincipal, _T("Everyone")) == 0) 
    {
        dwSubAuthority = SECURITY_WORLD_RID;
        fUseWorldAuth = TRUE;
    } 
    else if (_tcsicmp (tszPrincipal, _T("System")) == 0) 
    {
        dwSubAuthority = SECURITY_LOCAL_SYSTEM_RID;
    } 
    else if (_tcsicmp (tszPrincipal, _T("Self")) == 0) 
    {
        dwSubAuthority = SECURITY_PRINCIPAL_SELF_RID;
    } 
    else if (_tcsicmp (tszPrincipal, _T("Anonymous")) == 0) 
    {
        dwSubAuthority = SECURITY_ANONYMOUS_LOGON_RID;
    } 
    else if (_tcsicmp (tszPrincipal, _T("Interactive")) == 0) 
    {
        dwSubAuthority = SECURITY_INTERACTIVE_RID;
    } 
    else 
    {
        return FALSE;
    }


    if(dwSubAuthority == DOMAIN_ALIAS_RID_ADMINS ||
       dwSubAuthority == DOMAIN_ALIAS_RID_POWER_USERS)
    {
        if(!AllocateAndInitializeSid (
            &SidAuthorityNT, 
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            dwSubAuthority,
            0, 0, 0, 0, 0, 0,
            &psidTemp
            )) return FALSE;
    }
    else
    {

        if(!AllocateAndInitializeSid (
            fUseWorldAuth ? &SidAuthorityWorld : &SidAuthorityNT, 
            1, 
            dwSubAuthority,
            0, 0, 0, 0, 0, 0, 0, 
            &psidTemp
            )) return FALSE;

    }

    if(IsValidSid(psidTemp))
    {
        DWORD cbSid = GetLengthSid(psidTemp);
        *pSid = (PSID) malloc(cbSid);
        if(pSid)
        {
            if(!CopySid(cbSid, *pSid, psidTemp))
            {
                free(*pSid);
                *pSid = NULL;
            }
            else
            {
                fRetVal = TRUE;
            }
        }
        FreeSid(psidTemp);
    }


    return fRetVal;
}

/*---------------------------------------------------------------------------*\
 * NAME: GetPrincipalSID                                                     *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Creates a SID for the supplied principal.                    *
\*---------------------------------------------------------------------------*/
DWORD GetPrincipalSID (
    LPTSTR tszPrincipal,
    PSID *pSid
    )
{
    DWORD        cbSid              = 0;
    TCHAR        tszRefDomain [256] = {0};
    DWORD        cbRefDomain        = 0;
    DWORD        dwReturnValue      = ERROR_SUCCESS;
    SID_NAME_USE snu;

    cbSid = 0;
    cbRefDomain = 255;

    if(ConstructWellKnownSID(tszPrincipal, pSid)) return ERROR_SUCCESS;

    LookupAccountName (NULL,
                       tszPrincipal,
                       *pSid,
                       &cbSid,
                       tszRefDomain,
                       &cbRefDomain,
                       &snu);

    dwReturnValue = GetLastError();
    if (dwReturnValue != ERROR_INSUFFICIENT_BUFFER) goto CLEANUP;

    dwReturnValue = ERROR_SUCCESS;

    *pSid = (PSID) malloc (cbSid);
    if(!pSid)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }

    cbRefDomain = 255;
    
    if (!LookupAccountName (NULL,
                            tszPrincipal,
                            *pSid,
                            &cbSid,
                            tszRefDomain,
                            &cbRefDomain,
                            &snu))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

CLEANUP:

    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: SystemMessage                                                       *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Configures a system message for the given error code.        *
\*---------------------------------------------------------------------------*/
LPTSTR SystemMessage (
    LPTSTR tszBuffer,
    DWORD cbBuffer,
    HRESULT hr
    )
{
    LPTSTR   tszMessage = NULL;

    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   hr,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &tszMessage,
                   0,
                   NULL);

    _stprintf_s (tszBuffer, cbBuffer, _T("%s(%lx)\n"), tszMessage, hr);
    
    LocalFree (tszMessage);
    return tszBuffer;
}

/*---------------------------------------------------------------------------*\
 * NAME: IsLegacySecurityModel                                               *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Returns a boolean value of TRUE if this system is running    *
 * a legacy security model.                                                  *
\*---------------------------------------------------------------------------*/
BOOL IsLegacySecurityModel ()
{
    BOOL fReturnValue = FALSE;
    OSVERSIONINFOEX VersionInfo = {sizeof(VersionInfo)};

    if(GetVersionEx ((LPOSVERSIONINFO)&VersionInfo))
    {

        if(VersionInfo.dwMajorVersion == VERSION_MAJOR_W2K_XP_W2K3)
        {
            if(VersionInfo.dwMinorVersion < VERSION_MINOR_XP)
            {
                fReturnValue = TRUE;
            }
            else if(VersionInfo.dwMinorVersion == VERSION_MINOR_XP)
            {
                if(VersionInfo.wServicePackMajor < 2)
                {
                    fReturnValue = TRUE;
                }
            }
            else if(VersionInfo.dwMinorVersion == VERSION_MINOR_W2K3)
            {
                if(VersionInfo.wServicePackMajor < 1)
                {
                    fReturnValue = TRUE;
                }
            }

        }
        else if(VersionInfo.dwMajorVersion < VERSION_MAJOR_W2K_XP_W2K3)
        {
            fReturnValue = TRUE;
        }

    }

    return fReturnValue;
}



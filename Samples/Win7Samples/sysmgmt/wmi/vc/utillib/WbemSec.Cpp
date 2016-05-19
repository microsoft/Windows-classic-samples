//***************************************************************************

//

//  WBEMSEC.CPP

//

//  Purpose: Provides some security helper functions.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

//#undef _WIN32_WINNT
//#define _WIN32_WINNT 0x0400
#include "precomp.h"
#include <wbemidl.h>
#include "wbemsec.h"

//***************************************************************************
//
//  InitializeSecurity(DWORD dwAuthLevel, DWORD dwImpLevel)
//
//  DESCRIPTION:
//
//  Initialize DCOM security.  The authentication level is typically
//  RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  and the impersonation level is typically
//  RPC_C_IMP_LEVEL_IMPERSONATE.  
//  When using asynchronous WMI callbacks remotely in an environment where the "Local System" account 
//  has no network identity (such as non-Kerberos domains), the authentication level of 
//  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to 
//  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
//	use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.
//
//  RETURN VALUE:
//
//  see description.
//
//***************************************************************************

HRESULT InitializeSecurity(DWORD dwAuthLevel, DWORD dwImpLevel)
{
    // Initialize security
    // ===================

	DWORD dwCapabilities;

	if (dwAuthLevel == RPC_C_AUTHN_LEVEL_NONE) 
        dwCapabilities = EOAC_NONE;
	else
        dwCapabilities = EOAC_SECURE_REFS;


    return CoInitializeSecurity
		(NULL, -1, NULL, NULL,
        dwAuthLevel, dwImpLevel,
        NULL, 
		dwCapabilities, 
		0);
}

//***************************************************************************
//
//  bool bIsNT
//
//  DESCRIPTION:
//
//  Returns true if running windows NT.
//
//  RETURN VALUE:
//
//  see description.
//
//***************************************************************************

bool bIsNT(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;           // should never happen
    return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}


//***************************************************************************
//
//  SCODE ParseAuthorityUserArgs
//
//  DESCRIPTION:
//
//	NOTE: For Windows XP or later, consider calling CredUIParseUserName() instead of this method.
//
//  Examines the Authority and User argument and determines the authentication
//  type and possibly extracts the domain name from the user arugment in the
//  NTLM case.  For NTLM, the domain can be at the end of the authentication
//  string, or in the front of the user name, ex:  "TestDomain\JoeUser"
//
//  PARAMETERS:
//
//  ConnType            Returned with the connection type, ie wbem, ntlm
//  AuthArg             Output, contains the domain name
//  UserArg             Output, user name
//  Authority           Input
//  User                Input
//
//  RETURN VALUE:
//
//  S_OK                all is well
//  else error listed in WBEMSVC.H
//
//***************************************************************************


SCODE ParseAuthorityUserArgs(BSTR & AuthArg, BSTR & UserArg,BSTR & Authority,BSTR & User)
{

    // Determine the connection type by examining the Authority string

    if(!(Authority == NULL || wcslen(Authority) == 0 || !_wcsnicmp(Authority, L"NTLMDOMAIN:",11)))
        return E_INVALIDARG;

    // The ntlm case is more complex.  There are four cases
    // 1)  Authority = NTLMDOMAIN:name" and User = "User"
    // 2)  Authority = NULL and User = "User"
    // 3)  Authority = "NTLMDOMAIN:" User = "domain\user"
    // 4)  Authority = NULL and User = "domain\user"

    // first step is to determine if there is a backslash in the user name somewhere between the
    // second and second to last character

    WCHAR * pSlashInUser = NULL;
    if(User)
    {
        WCHAR * pEnd = User + wcslen(User) - 1;
        for(pSlashInUser = User; pSlashInUser <= pEnd; pSlashInUser++)
            if(*pSlashInUser == L'\\')      // dont think forward slash is allowed!
                break;
        if(pSlashInUser > pEnd)
            pSlashInUser = NULL;
    }

    if(Authority && wcslen(Authority) > 11)
    {
        if(pSlashInUser)
            return E_INVALIDARG;

        AuthArg = SysAllocString(Authority + 11);
		if (!AuthArg)
			return E_OUTOFMEMORY;
        if(User) 
		{
			UserArg = SysAllocString(User);
			if (!UserArg)
				return E_OUTOFMEMORY;
		}

        return S_OK;
    }
    else if(pSlashInUser)
    {
        INT_PTR iDomLen = min(MAX_PATH-1, pSlashInUser-User);
        WCHAR cTemp[MAX_PATH];
        wcsncpy_s(cTemp, User, iDomLen);
        cTemp[iDomLen] = 0;
        AuthArg = SysAllocString(cTemp);
		if (!AuthArg)
			return E_OUTOFMEMORY;
        if(wcslen(pSlashInUser+1))
		{
            UserArg = SysAllocString(pSlashInUser+1);
			if (!UserArg)
				return E_OUTOFMEMORY;
		}

    }
    else
        if(User) 
		{
			UserArg = SysAllocString(User);
			if (!UserArg)
				return E_OUTOFMEMORY;
		}

    return S_OK;
}


//***************************************************************************
//
//  SCODE GetAuthImp
//
//  DESCRIPTION:
//
//  Gets the authentication and impersonation levels for a current interface.
//
//  PARAMETERS:
//
//  pFrom               the interface to be tested.
//  pdwAuthLevel    Set to the authentication level
//  pdwImpLevel    Set to the impersonation level
//  RETURN VALUE:
//
//  S_OK                all is well
//  else error listed in WBEMSVC.H
//
//***************************************************************************

SCODE GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel)
{

    if(pFrom == NULL || pdwAuthLevel == NULL || pdwImpLevel == NULL)
        return WBEM_E_INVALID_PARAMETER;

    IClientSecurity * pFromSec = NULL;
    SCODE sc = pFrom->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        DWORD dwAuthnSvc, dwAuthzSvc, dwCapabilities;
        sc = pFromSec->QueryBlanket(pFrom, &dwAuthnSvc, &dwAuthzSvc,
                                            NULL,
                                            pdwAuthLevel, pdwImpLevel,
                                            NULL, &dwCapabilities);

        // Special case of going to a win9x share level box

        if (sc == 0x800706d2)
        {
            *pdwAuthLevel = RPC_C_AUTHN_LEVEL_NONE;
            *pdwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            sc = S_OK;
        }
        pFromSec->Release();
    }
    return sc;
}
//***************************************************************************
//
//  SCODE SetInterfaceSecurity
//
//  DESCRIPTION:
//
//  This routine is used by clients in order to set the security settings of a particular 
//  interface proxy. NOTE that setting the security blanket on the interface is not always needed:
//  simple client applications that execute in their own process can typically just call 
//	CoInitializeSecurity( NULL, -1, NULL, NULL, 
//						RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
//						RPC_C_IMP_LEVEL_IMPERSONATE, 
//						NULL, 
//						EOAC_NONE, 
//						NULL );
//  before calling out to WMI.  
//  However, for clients that reside in DLLs and do not control 
//  the hosting process' COM security setting (MMC snap-ins, Visual Studio extensions and such), 
//  it is necessary to set security on WMI interface proxies by calling 
//  CoSetProxyBlanket() directly or through this helper.  
//
//	Another case where setting proxy security for WMI connection is needed is when 
//  credentials different from the client process user context must be specified.
//
//
//  PARAMETERS:
//
//  pInterface       Interface to be set
//  pDomain          Input, domain
//  pUser            Input, user name
//  pPassword        Input, password.
//  pFrom            Input, if not NULL, then the authentication level of this interface
//                      is used
//  bAuthArg          If pFrom is NULL, then this is the authentication level
//  RETURN VALUE:
//
//  S_OK                all is well
//  else error listed in WBEMSVC.H
//
//***************************************************************************

HRESULT SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pAuthority, LPWSTR pUser,
                             LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel)
{

    SCODE sc;
    if(pInterface == NULL)
        return E_INVALIDARG;

    // If we are lowering the security, no need to deal with the identification info

    if(dwAuthLevel == RPC_C_AUTHN_LEVEL_NONE)
        return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    // If we are doing trivial case, just pass in a null authentication structure which is used
    // if the current logged in user's credentials are OK.

    if((pAuthority == NULL || wcslen(pAuthority) < 1) &&
        (pUser == NULL || wcslen(pUser) < 1) &&
        (pPassword == NULL || wcslen(pPassword) < 1))
            return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, NULL, EOAC_NONE);

    // If user, or Authority was passed in, the we need to create an authority argument for the login

    COAUTHIDENTITY  authident;
    BSTR AuthArg = NULL, UserArg = NULL;
    sc = ParseAuthorityUserArgs(AuthArg, UserArg, pAuthority, pUser);
	size_t * intReturnValue = NULL;
    if(sc != S_OK)
        return sc;

    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));
    if(bIsNT())
    {
        if(UserArg)
        {
            authident.UserLength = (ULONG)wcslen(UserArg);
            authident.User = (USHORT*)UserArg;
        }
        if(AuthArg)
        {
            authident.DomainLength = (ULONG)wcslen(AuthArg);
            authident.Domain = (USHORT*)AuthArg;
        }
        if(pPassword)
        {
            authident.PasswordLength = (ULONG)wcslen(pPassword);
            authident.Password = (USHORT*)pPassword;
        }
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    }
    else
    {
        char szUser[MAX_PATH], szAuthority[MAX_PATH], szPassword[MAX_PATH];
		ZeroMemory(szUser, sizeof(szUser));
		ZeroMemory(szAuthority, sizeof(szAuthority));
		ZeroMemory(szPassword, sizeof(szPassword));

		size_t written = 0;

        // Fill in the indentity structure

        if(UserArg)
        {
			written = wcstombs_s(intReturnValue, szUser, sizeof(szUser), UserArg, sizeof(szUser));
            //written = wcstombs(szUser, UserArg, sizeof(szUser));
			if (written < sizeof(szUser) - 1)
			{
				//we could retry by calling wcstombs to find out required buffer size, but this is a simple sample
				sc = E_INVALIDARG;
				goto cleanup;
			}
	
            authident.UserLength = (ULONG)strlen(szUser);
            authident.User = (USHORT*)szUser;
        }
        if(AuthArg)
        {
            written = wcstombs_s(intReturnValue, szAuthority, sizeof(szAuthority), AuthArg, sizeof(szAuthority));
			//written = wcstombs(szAuthority, AuthArg, sizeof(szAuthority));
			if (written < sizeof(szAuthority) - 1)
			{
				//we could retry by calling wcstombs to find out required buffer size, but this is a simple sample
				sc = E_INVALIDARG;
				goto cleanup;
			}
            authident.DomainLength = (ULONG)strlen(szAuthority);
            authident.Domain = (USHORT*)szAuthority;
        }
        if(pPassword)
        {
            written = wcstombs_s(intReturnValue, szPassword, sizeof(szPassword), pPassword, sizeof(szPassword));
			//written = wcstombs(szPassword, pPassword, sizeof(szPassword));
			if (written < sizeof(szPassword) - 1)
			{
				//we could retry by calling wcstombs to find out required buffer size, but this is a simple sample
				sc = E_INVALIDARG;
				goto cleanup;
			}
            authident.PasswordLength = (ULONG)strlen(szPassword);
            authident.Password = (USHORT*)szPassword;
        }
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    }
    sc = CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, &authident, EOAC_NONE);

cleanup:
    if(UserArg)
        SysFreeString(UserArg);
    if(AuthArg)
        SysFreeString(AuthArg);
    return sc;
}

/******************************************************************************
 *
 *	Name:	CheckAccess
 *				
 *  Description:	Allow provider to evaluate permissions against a security descriptor
 *
 *  This method should be called by WMI providers in scenarios where
 *			they cannot or should not impersonate the client. This happens in two scenarios:
 *			a) when the providers access resources that are not protected by ACL's
 *			b) when the client connects at the impersonation level of RPC_C_IMP_LEVEL_IDENTIFY
 *
 *****************************************************************************/

HRESULT CheckAccess (SECURITY_DESCRIPTOR *a_SecurityDescriptor ,
					DWORD a_Access , 
					GENERIC_MAPPING *a_Mapping)
{
	HRESULT t_Result = S_OK ;

	HANDLE t_Token = NULL ;

	BOOL t_Status = OpenThreadToken (

		GetCurrentThread () ,
		TOKEN_QUERY ,
		TRUE ,
		& t_Token 										
	) ;

	DWORD t_LastError = GetLastError () ;
	if ( ! t_Status)
	{
		//the thread token should always be available

		switch ( t_LastError )
		{
			case E_ACCESSDENIED:
			{
				return 	WBEM_E_ACCESS_DENIED ;
			}
			break ;

			default:
			{
				return WBEM_E_FAILED ;
			}
			break ;
		}
	}
	

	DWORD t_Access = 0 ;
	BOOL t_AccessStatus = FALSE ;
	PRIVILEGE_SET *t_PrivilegeSet = NULL ;
	DWORD t_PrivilegeSetSize = 0 ;

	MapGenericMask (

		& a_Access ,
		a_Mapping
	) ;

	t_Status = AccessCheck (

		a_SecurityDescriptor ,
		t_Token,
		a_Access ,
		a_Mapping ,
		NULL ,
		& t_PrivilegeSetSize ,
		& t_Access ,
		& t_AccessStatus
	) ;

	if (!t_Status || !t_AccessStatus )
	{
		DWORD t_LastError = GetLastError () ;
		if ( t_LastError == ERROR_INSUFFICIENT_BUFFER )
		{
			t_PrivilegeSet = ( PRIVILEGE_SET * ) new BYTE [ t_PrivilegeSetSize ] ;
			if ( t_PrivilegeSet )
			{				
				t_Status = AccessCheck (
					a_SecurityDescriptor ,
					t_Token,
					a_Access ,
					a_Mapping ,
					t_PrivilegeSet ,
					& t_PrivilegeSetSize ,
					& t_Access ,
					& t_AccessStatus
				) ;

				if ( !t_Status || !t_AccessStatus )
				{
					t_Result = WBEM_E_ACCESS_DENIED ;
				}

				delete [] ( BYTE * ) t_PrivilegeSet ;
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
		else
		{
			t_Result = WBEM_E_ACCESS_DENIED;
		}

	}

	CloseHandle ( t_Token ) ;	


	return t_Result ;
}



/******************************************************************************
 *
 *	Name:	GetCurrentImpersonationLevel
 *
 *	
 *  Description:
 *
 *			Get COM impersonation level of caller.	
 *
 *****************************************************************************/

DWORD GetCurrentImpersonationLevel ()
{
	DWORD t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;

    HANDLE t_ThreadToken = NULL ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread() ,
		TOKEN_QUERY,
		TRUE,
		&t_ThreadToken
	) ;

    if ( t_Status )
    {
		SECURITY_IMPERSONATION_LEVEL t_Level = SecurityAnonymous ;
		DWORD t_Returned = 0 ;

		t_Status = GetTokenInformation (

			t_ThreadToken ,
			TokenImpersonationLevel ,
			& t_Level ,
			sizeof ( SECURITY_IMPERSONATION_LEVEL ) ,
			& t_Returned
		) ;

		CloseHandle ( t_ThreadToken ) ;

		if ( t_Status == FALSE )
		{
			t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
		}
		else
		{
			switch ( t_Level )
			{
				case SecurityAnonymous:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
				}
				break ;

				case SecurityIdentification:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY ;
				}
				break ;

				case SecurityImpersonation:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
				}
				break ;

				case SecurityDelegation:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_DELEGATE ;
				}
				break ;

				default:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
				}
				break ;
			}
		}
	}
	else
	{
        ULONG t_LastError = GetLastError () ;

        if ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN )
        {
            t_ImpersonationLevel = RPC_C_IMP_LEVEL_DELEGATE ;
        }
        else 
		{
			if ( t_LastError == ERROR_CANT_OPEN_ANONYMOUS )
			{
				t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
			}
			else
			{
				t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
			}
		}
    }

	return t_ImpersonationLevel ;
}



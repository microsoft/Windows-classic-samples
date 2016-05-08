/*++

Copyright (C)  Microsoft Corporation

Module Name:

	Globals.cpp

Abstract:


History:

--*/

#include <PreComp.h>
#include <wbemprov.h>

#include <initguid.h>
#ifndef INITGUID
#define INITGUID
#endif

#include "Globals.h"

/******************************************************************************
 *
 *	Name:
 *
 *		s_LocksInProgress
 *		s_ObjectsInProgress
 *
 *  Description:
 *
 *		Keep a count of outstanding global objects. Used for debugging purposes.
 *
 *****************************************************************************/

LONG Provider_Globals :: s_LocksInProgress ;
LONG Provider_Globals :: s_ObjectsInProgress ;

/******************************************************************************
 *
 *	Name:	Global_Startup ()
 *
 *	
 *  Description:
 *
 *		Global function to handle initialization.
 *		Place user code here.
 *
 *****************************************************************************/

HRESULT Provider_Globals :: Global_Startup ()
{
	HRESULT t_Result = S_OK ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	Global_Shutdown ()
 *
 *	
 *  Description:
 *
 *		Global function to handle uninitialization.
 *		Place user code here.
 *
 *****************************************************************************/

HRESULT Provider_Globals :: Global_Shutdown ()
{
	HRESULT t_Result = S_OK ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	CreateInstance
 *
 *	
 *  Description:
 *
 *			Wrapper function for CoCreateInstance/CoGetClassObject.
 *			Useful for specific security context and debugging purposes.
 *
 *****************************************************************************/

HRESULT Provider_Globals :: CreateInstance ( 

	const CLSID &a_ReferenceClsid ,
	LPUNKNOWN a_OuterUnknown ,
	const DWORD &a_ClassContext ,
	const UUID &a_ReferenceInterfaceId ,
	void **a_ObjectInterface
)
{
	HRESULT t_Result = S_OK ;

#if 1

/*
 *	Use standard implementation
 */

	 t_Result = CoCreateInstance (
  
		a_ReferenceClsid ,
		a_OuterUnknown ,
		a_ClassContext ,
		a_ReferenceInterfaceId ,
		( void ** )  a_ObjectInterface
	);

#else

/*
 *	Tweak configuration settings for call.
 */

	COAUTHIDENTITY t_AuthenticationIdentity ;
	ZeroMemory ( & t_AuthenticationIdentity , sizeof ( t_AuthenticationIdentity ) ) ;

	t_AuthenticationIdentity.User = NULL ; 
	t_AuthenticationIdentity.UserLength = 0 ;
	t_AuthenticationIdentity.Domain = NULL ; 
	t_AuthenticationIdentity.DomainLength = 0 ; 
	t_AuthenticationIdentity.Password = NULL ; 
	t_AuthenticationIdentity.PasswordLength = 0 ; 
	t_AuthenticationIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE ; 

	COAUTHINFO t_AuthenticationInfo ;
	ZeroMemory ( & t_AuthenticationInfo , sizeof ( t_AuthenticationInfo ) ) ;

    t_AuthenticationInfo.dwAuthnSvc = RPC_C_AUTHN_DEFAULT ;
    t_AuthenticationInfo.dwAuthzSvc = RPC_C_AUTHZ_DEFAULT ;
    t_AuthenticationInfo.pwszServerPrincName = NULL ;
    t_AuthenticationInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY; //RPC_C_AUTHN_LEVEL_CONNECT ;
    t_AuthenticationInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE  ;
    t_AuthenticationInfo.dwCapabilities = EOAC_NONE ;
    t_AuthenticationInfo.pAuthIdentityData = NULL ;

	COSERVERINFO t_ServerInfo ;
	ZeroMemory ( & t_ServerInfo , sizeof ( t_ServerInfo ) ) ;

	t_ServerInfo.pwszName = NULL ;
    t_ServerInfo.dwReserved2 = 0 ;
    t_ServerInfo.pAuthInfo = & t_AuthenticationInfo ;

	IClassFactory *t_ClassFactory = NULL ;

	t_Result = CoGetClassObject (

		a_ReferenceClsid ,
		a_ClassContext ,
		& t_ServerInfo ,
		IID_IClassFactory ,
		( void ** )  & t_ClassFactory
	) ;
 
	if ( SUCCEEDED ( t_Result ) )
	{
		t_Result = t_ClassFactory->CreateInstance (

			a_OuterUnknown ,
			a_ReferenceInterfaceId ,
			a_ObjectInterface 
		);	

		t_ClassFactory->Release () ;
	}

#endif

	return t_Result ;
}


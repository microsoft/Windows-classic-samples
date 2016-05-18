/*++

Copyright (C)  Microsoft Corporation

Module Name:

	XXXX

Abstract:


History:

--*/

#include <precomp.h>
#include <objbase.h>
#include <sddl.h>
#include <strsafe.h>


#include "Globals.h"
#include "Service.h"

#define MASK_CLIENT_ACCESS_BIND 1
static GENERIC_MAPPING s_ClientAccessMapping = {
	0,
	0,
	STANDARD_RIGHTS_REQUIRED | MASK_CLIENT_ACCESS_BIND,
	STANDARD_RIGHTS_REQUIRED | MASK_CLIENT_ACCESS_BIND
};

/******************************************************************************
 *
 *	Name:	g_Contact_Properties
 *
 *	
 *  Description:
 *
 *			Static snapshot of contact data
 *	
 *****************************************************************************/

Object_Property g_Contact_Properties [] = {

		0 , 21 , L"ID" , L"0000000000001" ,
		0 , 8 , L"LastName" ,  L"Mandelshtam" ,
		0 , 8 , L"FirstName" , L"Osip" ,
		0 , 8 , L"HomePhone" , L"2069424877" ,
		0 , 8 , L"WorkPhone" , L"4258649459" ,
		1 , 0 , L"NA" , L"NA" ,
		0 , 21 , L"ID" , L"0000000000002" ,
		0 , 8 , L"LastName" ,  L"Pasternak" ,
		0 , 8 , L"FirstName" , L"Boris" ,
		0 , 8 , L"HomePhone" , L"4259434877" ,
		0 , 8 , L"WorkPhone" , L"4258964949" ,
		1 , 0 , L"NA" , L"NA" ,
		0 , 21 , L"ID" , L"0000000000003" ,
		0 , 8 , L"LastName" ,  L"Akhmatova" ,
		0 , 8 , L"FirstName" , L"Anna" ,
		0 , 8 , L"HomePhone" , L"7186724877" ,
		0 , 8 , L"WorkPhone" , L"2128640059" ,
		1 , 0 , L"NA" , L"NA" ,
		2 , 0 , L"NA" , L"NA"

} ;

/******************************************************************************
 *
 *	Name:	g_Process_Properties
 *
 *	
 *  Description:
 *
 *			Static snapshot of process data
 *	
 *****************************************************************************/

Object_Property g_Process_Properties [] = {

0 , 8 , L"Caption" , L"System Idle Process" ,
0 , 8 , L"CreationClassName" , L"Win32_Process" ,
0 , 8 , L"CSCreationClassName" , L"Win32_ComputerSystem" ,
0 , 8 , L"CSName" , L"STEVM_1" ,
0 , 8 , L"Description" , L"System Idle Process" ,
0 , 8 , L"Handle" , L"0" ,
0 , 19 , L"HandleCount" , L"0" ,
0 , 21 , L"KernelModeTime" , L"1484250781250" ,
0 , 8 , L"Name" , L"System Idle Process" ,
0 , 8 , L"OSCreationClassName" , L"Win32_OperatingSystem" ,
0 , 8 , L"OSName" , L"Microsoft Windows Whistler Server|C:\\WINNT|\\Device\\Harddisk0\\Partition1" ,
0 , 21 , L"OtherOperationCount" , L"0" ,
0 , 21 , L"OtherTransferCount" , L"0" ,
0 , 19 , L"PageFaults" , L"1" ,
0 , 19 , L"PageFileUsage" , L"0" ,
0 , 19 , L"ParentProcessId" , L"0" ,
0 , 19 , L"PeakPageFileUsage" , L"0" ,
0 , 21 , L"PeakVirtualSize" , L"0" ,
0 , 19 , L"PeakWorkingSetSize" , L"20480" ,
0 , 19 , L"Priority" , L"0" ,
0 , 21 , L"PrivatePageCount" , L"0" ,
0 , 19 , L"ProcessId" , L"0" ,
0 , 19 , L"QuotaNonPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPeakNonPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPeakPagedPoolUsage" , L"0" ,
0 , 21 , L"ReadOperationCount" , L"0" ,
0 , 21 , L"ReadTransferCount" , L"0" ,
0 , 19 , L"SessionId" , L"0" ,
0 , 19 , L"ThreadCount" , L"2" ,
0 , 21 , L"UserModeTime" , L"0" ,
0 , 21 , L"VirtualSize" , L"0" ,
0 , 8 , L"WindowsVersion" , L"5.1.2496" ,
0 , 21 , L"WorkingSetSize" , L"20480" ,
0 , 21 , L"WriteOperationCount" , L"0" ,
0 , 21 , L"WriteTransferCount" , L"0" ,
1 , 0 , L"NA" , L"NA" ,
0 , 8 , L"Caption" , L"System" ,
0 , 8 , L"CreationClassName" , L"Win32_Process" ,
0 , 8 , L"CSCreationClassName" , L"Win32_ComputerSystem" ,
0 , 8 , L"CSName" , L"STEVM_1" ,
0 , 8 , L"Description" , L"System" ,
0 , 8 , L"Handle" , L"4" ,
0 , 19 , L"HandleCount" , L"1650" ,
0 , 21 , L"KernelModeTime" , L"617031250" ,
0 , 19 , L"MaximumWorkingSetSize" , L"1413120" ,
0 , 19 , L"MinimumWorkingSetSize" , L"0" ,
0 , 8 , L"Name" , L"System" ,
0 , 8 , L"OSCreationClassName" , L"Win32_OperatingSystem" ,
0 , 8 , L"OSName" , L"Microsoft Windows Whistler Server|C:\\WINNT|\\Device\\Harddisk0\\Partition1" ,
0 , 21 , L"OtherOperationCount" , L"29386" ,
0 , 21 , L"OtherTransferCount" , L"508628" ,
0 , 19 , L"PageFaults" , L"7085" ,
0 , 19 , L"PageFileUsage" , L"0" ,
0 , 19 , L"ParentProcessId" , L"0" ,
0 , 19 , L"PeakPageFileUsage" , L"0" ,
0 , 21 , L"PeakVirtualSize" , L"2043904" ,
0 , 19 , L"PeakWorkingSetSize" , L"831488" ,
0 , 19 , L"Priority" , L"8" ,
0 , 21 , L"PrivatePageCount" , L"32768" ,
0 , 19 , L"ProcessId" , L"4" ,
0 , 19 , L"QuotaNonPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPeakNonPagedPoolUsage" , L"0" ,
0 , 19 , L"QuotaPeakPagedPoolUsage" , L"0" ,
0 , 21 , L"ReadOperationCount" , L"92" ,
0 , 21 , L"ReadTransferCount" , L"542740" ,
0 , 19 , L"SessionId" , L"0" ,
0 , 19 , L"ThreadCount" , L"50" ,
0 , 21 , L"UserModeTime" , L"0" ,
0 , 21 , L"VirtualSize" , L"1884160" ,
0 , 8 , L"WindowsVersion" , L"5.1.2496" ,
0 , 21 , L"WorkingSetSize" , L"221184" ,
0 , 21 , L"WriteOperationCount" , L"10189" ,
0 , 21 , L"WriteTransferCount" , L"54440072" ,
1 , 0 , L"NA" , L"NA" ,
2 , 0 , L"NA" , L"NA"
} ;

/******************************************************************************
 *
 *	Name:	Set_Uint64
 *
 *	
 *  Description:
 *
 *		Sets a 64bit value into an instance	
 *
 *****************************************************************************/

HRESULT Set_Uint64 (

	IWbemClassObject *a_Instance , 
	wchar_t *a_Name ,
	const UINT64 &a_Uint64
)
{
	wchar_t t_String [ 64 ] ;
	StringCbPrintf(t_String, sizeof(t_String), L"%I64u" , a_Uint64 ) ;

	VARIANT t_Variant ;
	VariantInit ( & t_Variant ) ;
	t_Variant.vt = VT_BSTR ;
	t_Variant.bstrVal = SysAllocString ( t_String ) ;
	if ( t_Variant.bstrVal )
	{
		HRESULT t_Result = a_Instance->Put ( a_Name , 0 , & t_Variant , 0 ) ;

		VariantClear ( & t_Variant ) ;

		return t_Result ;
	}
	else
	{
		return WBEM_E_OUT_OF_MEMORY ;
	}
}

/******************************************************************************
 *
 *	Name:	Set_Uint32
 *
 *	
 *  Description:
 *
 *		Sets a 32bit value into an instance	
 *
 *****************************************************************************/

HRESULT Set_Uint32 ( 

	IWbemClassObject *a_Instance , 
	wchar_t *a_Name ,
	const DWORD &a_Uint32
)
{
	VARIANT t_Variant ;
	VariantInit ( & t_Variant ) ;
	t_Variant.vt = VT_I4 ;
	t_Variant.lVal = a_Uint32 ;

	return a_Instance->Put ( a_Name , 0 , & t_Variant , 0 ) ;
}

/******************************************************************************
 *
 *	Name:	Set_String
 *
 *	
 *  Description:
 *
 *		Sets a string value into an instance	
 *
 *****************************************************************************/

HRESULT Set_String ( 

	IWbemClassObject *a_Instance , 
	wchar_t *a_Name ,
	wchar_t *a_String
)
{
	VARIANT t_Variant ;
	VariantInit ( & t_Variant ) ;
	t_Variant.vt = VT_BSTR ;
	t_Variant.bstrVal = SysAllocString ( a_String ) ;
	if ( t_Variant.bstrVal )
	{
		HRESULT t_Result = a_Instance->Put ( a_Name , 0 , & t_Variant , 0 ) ;

		VariantClear ( & t_Variant ) ;

		return t_Result ;
	}
	else
	{
		return WBEM_E_OUT_OF_MEMORY ;
	}
}

/******************************************************************************
 *
 *	Name:	SetProperty
 *
 *	
 *  Description:
 *
 *			Set an instance property based on it's type specified in the static
 *			process data definition.	
 *
 *****************************************************************************/

HRESULT SetProperty ( 

	IWbemClassObject *a_Instance , 
	Object_Property &a_Property
)
{
	HRESULT t_Result = S_OK ;

	switch ( a_Property.m_Type )
	{
		case CIM_STRING:
		{
			t_Result = Set_String ( 

				a_Instance , 
				a_Property.m_Name ,
				a_Property.m_Value
			) ;
		}
		break ;

		case CIM_UINT32:
		{
			ULONG t_Integer ;
			swscanf_s ( a_Property.m_Value ,  L"%lu" , & t_Integer ) ;

			t_Result = Set_Uint32 ( 

				a_Instance , 
				a_Property.m_Name ,
				t_Integer
			) ;
		}
		break ;

		case CIM_UINT64:
		{
			UINT64 t_Integer ;
			swscanf_s ( a_Property.m_Value , L"%I64u" , & t_Integer ) ;

			t_Result = Set_Uint64 ( 

				a_Instance , 
				a_Property.m_Name ,
				t_Integer
			) ;
		}
		break ;

		default:
		{
			WBEM_E_INVALID_PARAMETER ;
		}
		break ;
	}

	return t_Result ;
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



/******************************************************************************
 *
 *	Name:	CProvider_IWbemServices
 *
 *	
 *  Description:
 *
 *		Constructor for object. Initialize variables to NULL. Increment global object count
 *	
 *****************************************************************************/

CProvider_IWbemServices :: CProvider_IWbemServices () : 

	m_ReferenceCount ( 0 ) , 
	m_User ( NULL ) ,
	m_Locale ( NULL ) ,
	m_Namespace ( NULL ) ,
	m_CoreService ( NULL ) ,
	m_ComputerName  ( NULL ) ,
	m_OperatingSystemVersion ( NULL ) ,
	m_Win32_ProcessEx_Object ( NULL ),
	m_ContactInfo_Object (NULL)
{
	InitializeCriticalSection ( & m_CriticalSection ) ;

	InterlockedIncrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:	~CProvider_IWbemServices
 *
 *	
 *  Description:
 *
 *		Constructor for object. Uninitialize variables . Decrement global object count
 *	
 *****************************************************************************/

CProvider_IWbemServices :: ~CProvider_IWbemServices ()
{
	DeleteCriticalSection ( & m_CriticalSection ) ;

	if ( m_User ) 
	{
		SysFreeString ( m_User ) ;
	}

	if ( m_Locale ) 
	{
		SysFreeString ( m_Locale ) ;
	}

	if ( m_Namespace ) 
	{
		SysFreeString ( m_Namespace ) ;
	}

	if ( m_CoreService ) 
	{
		m_CoreService->Release () ;
	}

	if ( m_Win32_ProcessEx_Object ) 
	{
		m_Win32_ProcessEx_Object->Release () ;
	}
	
	if ( m_ContactInfo_Object ) 
	{
		m_ContactInfo_Object->Release () ;
	}

	if ( m_ComputerName ) 
	{
		SysFreeString ( m_ComputerName ) ;
	}

	if ( m_OperatingSystemVersion ) 
	{
		SysFreeString ( m_OperatingSystemVersion ) ;
	}

	InterlockedDecrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:		AddRef
 *
 *	
 *  Description:
 *
 *		Perform Locked increment. Keep com object Alive.
 *
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemServices :: AddRef ( void )
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

/******************************************************************************
 *
 *	Name:
 *
 *		Release
 *
 *  Description:
 *
 *		Perform Locked decrement. Attempt to destroy com object.
 *	
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemServices :: Release ( void )
{
	LONG t_Reference ;
	if ( ( t_Reference = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
	{
/*
 *	No more outstanding references, delete the object.
 */
		delete this ;
	}

	return t_Reference ;
}

/******************************************************************************
 *
 *	Name:
 *
 *	
 *  Description:
 *
 *	
 *****************************************************************************/

STDMETHODIMP CProvider_IWbemServices :: QueryInterface (

	REFIID a_Riid , 
	LPVOID FAR *a_Void 
) 
{
/*
 *	Just clean up out parameter first.
 */

	*a_Void = NULL ;

	if ( a_Riid == IID_IUnknown )
	{
		*a_Void = ( LPVOID ) this ;
	}
	else if ( a_Riid == IID_IWbemServices )
	{
/*
 *	Make sure we support event consumer interface IWbemServices
 */

		*a_Void = ( LPVOID ) ( IWbemServices * ) this ;		
	}	
	else if ( a_Riid == IID_IWbemPropertyProvider )
	{
		*a_Void = ( LPVOID ) ( IWbemPropertyProvider * ) this ;		
	}	
	else if ( a_Riid == IID_IWbemProviderInit )
	{
		*a_Void = ( LPVOID ) ( IWbemProviderInit * ) this ;		
	}	
	else if ( a_Riid == IID_IWbemShutdown )
	{
		*a_Void = ( LPVOID ) ( IWbemShutdown * ) this ;		
	}	

	if ( *a_Void )
	{
		( ( LPUNKNOWN ) *a_Void )->AddRef () ;

		return ResultFromScode ( S_OK ) ;
	}
	else
	{
		return ResultFromScode ( E_NOINTERFACE ) ;
	}
}

/******************************************************************************
 *
 *	Name:	CancelAsyncCall
 *
 *	
 *  Description:
 *
 *			Allow provider to determine if call was cancelled.	
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: CancelAsyncCall ( 
		
	IWbemObjectSink *a_Sink
)
{
	HRESULT t_Result = WBEM_E_NOT_AVAILABLE ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	GetObjectAsync
 *
 *	
 *  Description:
 *
 *			Allow provider to respond to a query for a specific object	
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: GetObjectAsync ( 
		
	const BSTR a_ObjectPath ,
	long a_Flags , 
	IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
	HRESULT t_Result = WBEM_E_NOT_FOUND ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	PutClassAsync
 *
 *	
 *  Description:
 *
 *			Not required for instance providers.,	
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: PutClassAsync ( 
		
	IWbemClassObject *a_Object , 
	long a_Flags ,
	IWbemContext FAR *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
 	 return WBEM_E_NOT_FOUND ;
}

/******************************************************************************
 *
 *	Name:	DeleteClassAsync
 *
 *	
 *  Description:
 *
 *			Not required for instance providers.,	
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: DeleteClassAsync ( 
		
	const BSTR a_Class ,
	long a_Flags,
	IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
 	 return WBEM_E_NOT_AVAILABLE ;
}

/******************************************************************************
 *
 *	Name:	CreateClassEnumAsync
 *
 *	
 *  Description:
 *
 *			Not required for instance providers.,	
 *
 *****************************************************************************/


SCODE CProvider_IWbemServices :: CreateClassEnumAsync (

	const BSTR a_SuperClass ,
	long a_Flags ,
	IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
	return WBEM_E_NOT_FOUND ;
}

/******************************************************************************
 *
 *	Name:	PutInstanceAsync
 *
 *	
 *  Description:
 *
 *			Allow provider to create/update a specific object
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: PutInstanceAsync ( 
		
	IWbemClassObject *a_Instance , 
	long a_Flags ,
    IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
	HRESULT t_Result = WBEM_E_NOT_AVAILABLE ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	DeleteInstanceAsync
 *
 *	
 *  Description:
 *
 *			Allow provider to delete a specific object
 *
 *****************************************************************************/
        
HRESULT CProvider_IWbemServices :: DeleteInstanceAsync (
 
	const BSTR a_ObjectPath ,
    long a_Flags ,
    IWbemContext *a_Context ,
    IWbemObjectSink *a_Sink	
)
{
	HRESULT t_Result = WBEM_E_NOT_AVAILABLE ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	CreateInstanceEnumAsync
 *
 *	
 *  Description:
 *
 *			This provider enumerates instances of Win32_ProcessEx and ContactInfo classes.
 *			
 *			To access Win32_ProcessEx objects, this provider impersonates the client (modeling the
 *			scenario where protected OS objects would surfaced by a provider - even though
 *			in this sample the objects are in fact hard-coded and not protected).
 *
 *			ContactInfo objects surfaced by this provider that are not protected by ACL's, so
 *			the provider performs access checks only allowing certain groups to view ContactInfo data
 *			(acts as an Identity-level provider)
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: CreateInstanceEnumAsync (

 	const BSTR a_Class ,
	long a_Flags ,
	IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{
	//Impersonate the client
	HRESULT hr = S_OK ;

	if ( _wcsicmp ( a_Class , L"ContactInfo" ) == 0 ) 
	{
		//Grant access for built-in admins, local system, local admins, local service and network service.
		PSECURITY_DESCRIPTOR secDescriptor = NULL;
		BOOL bRes = ConvertStringSecurityDescriptorToSecurityDescriptor //this function is only available on Windows 2000 and above
			( L"O:BAG:BAD:(A;;0x10000001;;;BA)(A;;0x10000001;;;SY)(A;;0x10000001;;;LA)(A;;0x10000001;;;SY)(A;;0x10000001;;;S-1-5-20)(A;;0x10000001;;;S-1-5-19)",
			SDDL_REVISION_1,
			(PSECURITY_DESCRIPTOR *) &secDescriptor,
			NULL);

		if (! bRes)
		{
			hr = WBEM_E_ACCESS_DENIED;
			a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;
			return hr ;
		}

		hr = CoImpersonateClient () ;
		if ( FAILED ( hr ) )
		{
			LocalFree(secDescriptor);

			a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;
			return hr ;
		}	


		// perform an access check. 
		
		hr = CheckAccess((SECURITY_DESCRIPTOR *)secDescriptor, 
							MASK_CLIENT_ACCESS_BIND,
							&s_ClientAccessMapping);

		LocalFree(secDescriptor);

		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		if (FAILED(hr))
		{
			hr = WBEM_E_ACCESS_DENIED;
			a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;
			return hr ;
		}		
		

 		//Enumerate the instances.
		hr = CreateInstanceEnumAsync_Contacts ( 

			m_ContactInfo_Object ,
			a_Flags ,
			a_Context , 
			a_Sink
		) ;

	}
	else if ( _wcsicmp ( a_Class , L"Win32_ProcessEx" ) == 0 ) 
	{
		hr = CoImpersonateClient () ;
		if ( FAILED ( hr ) )
		{
			a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;
			return hr ;
		}	

		//	Check to see if call is at the RPC_C_IMP_LEVEL_IDENTIFY level. If that's the case,
		//  the provider will not be able to impersonate the client to access the protected Win32_ProcessEx resources.
		//  However, the provider can decide perform an access check to see if the client is sufficiently privileged.
		//  
		//  Please keep in mind that if in the MOF file __Win32Provider instabce sets the HostingModel 
		//  to Decoupled:Com (the default), the original client impersonation level will be lowered to
		//  RPC_C_IMP_LEVEL_IDENTIFY before calling into provider. To allow original client 
		//  impersonation level through to provider, set the HostingModel to 
		//  Decoupled:Com:FoldIdentity(FALSE}.
		//  See comments in the MOF file for more info.
	

		DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
		if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
		{
			//	Revert before we perform any operations	
			CoRevertToSelf () ;


			hr = WBEM_E_ACCESS_DENIED;
			a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;
			return hr ;
		}

 		//Enumerate the instances.
		hr = CreateInstanceEnumAsync_Process ( 

			m_Win32_ProcessEx_Object ,
			a_Flags ,
			a_Context , 
			a_Sink
		) ;

		CoRevertToSelf () ;	
	}
	else
	{
		hr = WBEM_E_INVALID_CLASS ;
	}

/*
 *	Inform WMI of status of call.
 */


	a_Sink->SetStatus ( 0 , hr , NULL , NULL ) ;

	return hr ;
}

/******************************************************************************
 *
 *	Name:	ExecQueryAsync
 *
 *	
 *  Description:
 *
 *			Enumerate instances based on a WQL query.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: ExecQueryAsync ( 
		
	const BSTR a_QueryFormat, 
	const BSTR a_Query, 
	long a_Flags, 
	IWbemContext *a_Context ,
	IWbemObjectSink *a_Sink
) 
{

	return WBEM_E_NOT_AVAILABLE ;


}

/******************************************************************************
 *
 *	Name:	ExecMethodAsync
 *
 *	
 *  Description:
 *
 *			Allow provider to execute a method.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: ExecMethodAsync ( 

    const BSTR a_ObjectPath ,
    const BSTR a_MethodName ,
    long a_Flags ,
    IWbemContext *a_Context ,
    IWbemClassObject *a_InParams ,
	IWbemObjectSink *a_Sink
) 
{
	HRESULT t_Result = WBEM_E_NOT_AVAILABLE ;
	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	GettProperty
 *	
 *	
 *  Description:
 *
 *		Place holder for property provider method. Currently not used.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: GetProperty (

    long a_Flags ,
    const BSTR a_Locale ,
    const BSTR a_ClassMapping ,
    const BSTR a_InstanceMapping ,
    const BSTR a_PropertyMapping ,
    VARIANT *a_Value
)
{
	if ( _wcsicmp ( a_PropertyMapping , L"ExtraProperty1" ) == 0 )
	{
	}
	else 
	{
		if ( _wcsicmp ( a_PropertyMapping , L"ExtraProperty2" ) == 0 )
		{
		}
		else
		{
		}
	}
	
	return S_OK ;
}

/******************************************************************************
 *
 *	Name:	PutProperty
 *	
 *	
 *  Description:
 *
 *		Place holder for property provider method. Currently not used.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: PutProperty (

    long a_Flags ,
    const BSTR a_Locale ,
    const BSTR a_ClassMapping ,
    const BSTR a_InstanceMapping ,
    const BSTR a_PropertyMapping ,
    const VARIANT *a_Value
)
{
	if ( _wcsicmp ( a_PropertyMapping , L"ExtraProperty1" ) == 0 )
	{
	}
	else 
	{
		if ( _wcsicmp ( a_PropertyMapping , L"ExtraProperty2" ) == 0 )
		{
		}
		else
		{
		}
	}

	return S_OK ;
}

/******************************************************************************
 *
 *	Name:		Initialize
 *
 *	
 *  Description:
 *
 *				Wmi calls this optional interface to inform the provider of
 *				configuration information associated within the client, locale
 *				and namespace. Provider retains this information for future use.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: Initialize (

	LPWSTR a_User,
	LONG a_Flags,
	LPWSTR a_Namespace,
	LPWSTR a_Locale,
	IWbemServices *a_CoreService,         // For anybody
	IWbemContext *a_Context,
	IWbemProviderInitSink *a_Sink     // For init signals
)
{
	HRESULT t_Result = S_OK ;
	
      if ( m_User ) 

      {

            SysFreeString ( m_User ) ;

            m_User = NULL ;

      }

 

      if ( m_Locale ) 

      {

            SysFreeString ( m_Locale ) ;

            m_Locale = NULL ;

      }

 

      if ( m_Namespace ) 

      {

            SysFreeString ( m_Namespace ) ;

            m_Namespace  = NULL ;

      }

 

      if ( m_CoreService ) 

      {

            m_CoreService->Release () ;

            m_CoreService = NULL ;

      }

 

      if ( m_Win32_ProcessEx_Object ) 

      {

            m_Win32_ProcessEx_Object->Release () ;

            m_Win32_ProcessEx_Object = NULL ;

      }

	  if ( m_ContactInfo_Object ) 

      {

            m_Win32_ProcessEx_Object->Release () ;

            m_Win32_ProcessEx_Object = NULL ;

      }



      if ( m_ComputerName ) 

      {

            SysFreeString ( m_ComputerName ) ;

            m_ComputerName = NULL ;

      }

 

      if ( m_OperatingSystemVersion ) 

      {

            SysFreeString ( m_OperatingSystemVersion ) ;

            m_OperatingSystemVersion = NULL ;

      }


/*
 *	Impersonate and check impersonation level.
 */

	t_Result = CoImpersonateClient () ;
	if ( SUCCEEDED ( t_Result ) )
	{
		if ( GetCurrentImpersonationLevel () == RPC_C_IMP_LEVEL_IDENTIFY )
		{
			CoRevertToSelf () ;
		}
	}

	if ( a_CoreService ) 
	{
		m_CoreService = a_CoreService ;
		m_CoreService->AddRef () ;
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_User ) 
		{
			m_User = SysAllocString ( a_User ) ;
			if ( m_User == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_Locale ) 
		{
			m_Locale = SysAllocString ( a_Locale ) ;
			if ( m_Locale == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_Namespace ) 
		{
			m_Namespace = SysAllocString ( a_Namespace ) ;
			if ( m_Namespace == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}

	BSTR t_Class;
	
	if ( SUCCEEDED ( t_Result ) ) 
	{
		t_Class = SysAllocString ( L"Win32_ProcessEx" ) ;
		if ( !t_Class ) 
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
		else
		{
			t_Result = m_CoreService->GetObject (

				t_Class ,
				0 ,
				a_Context ,
				& m_Win32_ProcessEx_Object ,
				NULL 
			) ;			

			SysFreeString ( t_Class ) ;
		}
	}

	if ( SUCCEEDED ( t_Result ) ) 
	{

		t_Class = SysAllocString( L"ContactInfo" );
		if ( !t_Class ) 
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
		else
		{
			t_Result = m_CoreService->GetObject (

				t_Class ,
				0 ,
				a_Context ,
				& m_ContactInfo_Object ,
				NULL 
			) ;

			SysFreeString ( t_Class ) ;
		}
		
	}

	if ( SUCCEEDED ( t_Result ) ) 
	{
		m_ComputerName = SysAllocStringLen ( NULL , MAX_COMPUTERNAME_LENGTH + 1 ) ;
		if ( m_ComputerName ) 
		{
			DWORD t_Length = MAX_COMPUTERNAME_LENGTH + 1 ;
			if (! GetComputerName ( m_ComputerName , & t_Length ))
			{
				t_Result = WBEM_E_FAILED ;
			}

		}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
	}

	if ( SUCCEEDED ( t_Result ) )
	{	
		OSVERSIONINFO t_VersionInfo ;
		t_VersionInfo.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;
		if ( GetVersionEx ( & t_VersionInfo ) )
		{
			m_OperatingSystemVersion  = SysAllocStringLen ( NULL , _MAX_PATH ) ;
			if ( m_OperatingSystemVersion )
			{
				StringCbPrintfW(m_OperatingSystemVersion, SysStringByteLen(m_OperatingSystemVersion),
					L"%d.%d.%hu", t_VersionInfo.dwMajorVersion , t_VersionInfo.dwMinorVersion , LOWORD ( t_VersionInfo.dwBuildNumber ) ) ;
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
		else
		{
			t_Result = WBEM_E_FAILED ;
		}
	}

	CoRevertToSelf () ;

	a_Sink->SetStatus ( t_Result , 0 ) ;

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Shutdown
 *
 *	
 *  Description:
 *
 *				Optional interface and method that informs provider of object implementation
 *				being released by WMI. Shutdown is not guaranteed to be called on system
 *				shutdown.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: Shutdown (

	LONG a_Flags ,
	ULONG a_MaxMilliSeconds ,
	IWbemContext *a_Context
)
{
	HRESULT t_Result = S_OK ;
	return t_Result ;
}


/******************************************************************************
 *
 *	Name:	CreateInstanceEnumAsync_Contacts
 *
 *	
 *  Description:
 *
 *		Enumerate all static instances of the ContactInfo class.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: CreateInstanceEnumAsync_Contacts (

	IWbemClassObject *a_ClassObject ,
	long a_Flags , 
	IWbemContext __RPC_FAR *a_Context,
	IWbemObjectSink FAR *a_Sink
)
{
	HRESULT t_Result = S_OK ;

/*
 *	Count the instances
 */

	ULONG t_Count = 0 ; 

	Object_Property *t_Property = g_Contact_Properties ;
	while ( SUCCEEDED ( t_Result ) && t_Property )
	{
		BOOL t_Loop = TRUE ;
		while ( t_Loop )
		{
			switch ( t_Property->m_State )
			{
				case 0:
				{
					t_Property ++ ;
				}
				break ;

				case 1:
				{
					t_Loop = FALSE ;
					t_Property ++ ;
					t_Count ++ ;
				}
				break ;

				case 2:
				{
					t_Loop = FALSE ;
					t_Property = NULL ;
				} ;

				default:
				{
				}
				break ;
			}
		}
	}

	if ( t_Count )
	{

/*
 *	Create the array of instances
 */

		ULONG t_Index = 0 ;

		IWbemClassObject **t_ObjectArray = new IWbemClassObject * [ t_Count ] ;
		if ( t_ObjectArray )
		{
			for ( ULONG t_ObjectIndex = 0 ; t_ObjectIndex < t_Count ; t_ObjectIndex ++ )
			{
				t_ObjectArray [ t_ObjectIndex ] = NULL ;
			}

			t_Result = a_ClassObject->SpawnInstance ( 

				0 , 
				& t_ObjectArray [ t_Index ] 
			) ;

			Object_Property *t_Property = g_Contact_Properties ;
			while ( SUCCEEDED ( t_Result ) && t_Property )
			{
				BOOL t_Loop = TRUE ;
				while ( t_Loop )
				{
					switch ( t_Property->m_State )
					{
						case 0:
						{
/*
 *	Set property value for the current instance
 */

							SetProperty ( t_ObjectArray [ t_Index ] , *t_Property ) ;
							t_Property ++ ;
						}
						break ;

						case 1:
						{
/*
 *	Create a new instance
 */
							t_Loop = FALSE ;

							t_Index ++ ;

							if ( t_Index < t_Count ) 
							{
								t_Result = a_ClassObject->SpawnInstance ( 

									0 , 
									& t_ObjectArray [ t_Index ] 
								) ;
							}

							t_Property ++ ;
						}
						break ;

						case 2:
						{
/*
 * Break from loop when we've reached the end of the data
 */
							t_Loop = FALSE ;
							t_Property = NULL ;
						} ;

						default:
						{
						}
						break ;
					}
				}
			}

/*
 *	Send the instances to WMI.
 */
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = a_Sink->Indicate ( t_Count , t_ObjectArray ) ;
			}

/*
 *	Discard
 */

			for ( t_Index = 0 ; t_Index < t_Count ; t_Index ++ )
			{
				if ( t_ObjectArray [ t_Index ] )
				{
					t_ObjectArray [ t_Index ]->Release () ;
				}
			}

			delete [] t_ObjectArray ;
		}
	}

	return t_Result ;
}


/******************************************************************************
 *
 *	Name:	CreateInstanceEnumAsync_Process
 *
 *	
 *  Description:
 *
 *		Enumerate all static instances of the Win32_ProcessEx class.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemServices :: CreateInstanceEnumAsync_Process (

	IWbemClassObject *a_ClassObject ,
	long a_Flags , 
	IWbemContext __RPC_FAR *a_Context,
	IWbemObjectSink FAR *a_Sink
)
{
	HRESULT t_Result = S_OK ;

/*
 *	Count the instances
 */

	ULONG t_Count = 0 ; 

	Object_Property *t_Property = g_Process_Properties ;
	while ( SUCCEEDED ( t_Result ) && t_Property )
	{
		BOOL t_Loop = TRUE ;
		while ( t_Loop )
		{
			switch ( t_Property->m_State )
			{
				case 0:
				{
					t_Property ++ ;
				}
				break ;

				case 1:
				{
					t_Loop = FALSE ;
					t_Property ++ ;
					t_Count ++ ;
				}
				break ;

				case 2:
				{
					t_Loop = FALSE ;
					t_Property = NULL ;
				} ;

				default:
				{
				}
				break ;
			}
		}
	}

	if ( t_Count )
	{

/*
 *	Create the array of instances
 */

		ULONG t_Index = 0 ;

		IWbemClassObject **t_ObjectArray = new IWbemClassObject * [ t_Count ] ;
		if ( t_ObjectArray )
		{
			for ( ULONG t_ObjectIndex = 0 ; t_ObjectIndex < t_Count ; t_ObjectIndex ++ )
			{
				t_ObjectArray [ t_ObjectIndex ] = NULL ;
			}

			t_Result = a_ClassObject->SpawnInstance ( 

				0 , 
				& t_ObjectArray [ t_Index ] 
			) ;

			Object_Property *t_Property = g_Process_Properties ;
			while ( SUCCEEDED ( t_Result ) && t_Property )
			{
				BOOL t_Loop = TRUE ;
				while ( t_Loop )
				{
					switch ( t_Property->m_State )
					{
						case 0:
						{
/*
 *	Set property value for the current instance
 */

							SetProperty ( t_ObjectArray [ t_Index ] , *t_Property ) ;
							t_Property ++ ;
						}
						break ;

						case 1:
						{
/*
 *	Create a new instance
 */
							t_Loop = FALSE ;

							t_Index ++ ;

							if ( t_Index < t_Count ) 
							{
								t_Result = a_ClassObject->SpawnInstance ( 

									0 , 
									& t_ObjectArray [ t_Index ] 
								) ;
							}

							t_Property ++ ;
						}
						break ;

						case 2:
						{
/*
 * Break from loop when we've reached the end of the data
 */
							t_Loop = FALSE ;
							t_Property = NULL ;
						} ;

						default:
						{
						}
						break ;
					}
				}
			}

/*
 *	Send the instances to WMI.
 */

			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = a_Sink->Indicate ( t_Count , t_ObjectArray ) ;
			}

/*
 *	Discard
 */


			for ( t_Index = 0 ; t_Index < t_Count ; t_Index ++ )
			{
				if ( t_ObjectArray [ t_Index ] )
				{
					t_ObjectArray [ t_Index ]->Release () ;
				}
			}

			delete [] t_ObjectArray ;
		}
	}

	return t_Result ;
}


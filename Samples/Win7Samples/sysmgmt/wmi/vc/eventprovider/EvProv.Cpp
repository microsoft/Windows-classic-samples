// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  EVPROV.cpp
//
// Description:
//    Sample event provider.
//
// History:
//
// **************************************************************************

#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#include <windows.h>
#include <stdio.h>
#include <objbase.h>

#include <wbemidl.h>

#include "oahelp.inl"
#include "evprov.h"

#include <strsafe.h>
#include <sddl.h>


#define MASK_CLIENT_ACCESS_BIND 1
static GENERIC_MAPPING s_ClientAccessMapping = {
	0,
	0,
	STANDARD_RIGHTS_REQUIRED | MASK_CLIENT_ACCESS_BIND,
	STANDARD_RIGHTS_REQUIRED | MASK_CLIENT_ACCESS_BIND
};


//***************************************************************************
//
//***************************************************************************
// ok

CMyEventProvider::CMyEventProvider()
{
    m_pNs = 0;
    m_pSink = 0;
    m_cRef = 0;
    m_pEventClassDef = 0;
    m_eStatus = Pending;
    m_hThread = 0;
}


//***************************************************************************
//
//***************************************************************************
// ok

CMyEventProvider::~CMyEventProvider()
{
    if (m_hThread)
        CloseHandle(m_hThread);

    if (m_pNs)
        m_pNs->Release();

    if (m_pSink)
        m_pSink->Release();

    if (m_pEventClassDef)
        m_pEventClassDef->Release();        
}


//***************************************************************************
//
//***************************************************************************
// ok

STDMETHODIMP CMyEventProvider::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IWbemEventProvider==riid)
    {
        *ppv = (IWbemEventProvider *) this;
        AddRef();
        return NOERROR;
    }

    if (IID_IWbemProviderInit==riid)
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}



//***************************************************************************
//
//***************************************************************************
// ok

ULONG CMyEventProvider::AddRef()
{
    return ++m_cRef;
}



//***************************************************************************
//
//***************************************************************************
// ok

ULONG CMyEventProvider::Release()
{
    if (0 != --m_cRef)
        return m_cRef;

    // If here, we are shutting down.
    // ==============================

    m_eStatus = PendingStop;

    return 0;
}


//***************************************************************************
//
//***************************************************************************
// ok

HRESULT CMyEventProvider::ProvideEvents( 
    /* [in] */ IWbemObjectSink __RPC_FAR *pSink,
    /* [in] */ long lFlags
    )
{
    // Copy the sink.
    // ==============
    
    m_pSink = pSink;
    m_pSink->AddRef();

    // Create the event thread.
    // ========================
    
    DWORD dwTID;
    
    m_hThread = CreateThread(
        0,
        0,
        CMyEventProvider::EventThread,
        this,
        0,
        &dwTID
        );


    // Wait for provider to be 'ready'.
    // ================================
    
    while (m_eStatus != Running)
        Sleep(100);

    return WBEM_NO_ERROR;
}


//***************************************************************************
//
//  This particular provider, being in a DLL operates via its own thread.  
//
//  In practice, such a provider would probably be implemented within a 
//  separate EXE.
//
//***************************************************************************
// ok

DWORD WINAPI CMyEventProvider::EventThread(LPVOID pArg)
{
    // Make transition to the per-instance method.
    // ===========================================
    
    ((CMyEventProvider *)pArg)->InstanceThread();
    return 0;
}

//***************************************************************************
//
//  Events are generated from here
//
//***************************************************************************
// ok

void CMyEventProvider::InstanceThread()
{
    int nIteration = 0;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    m_eStatus = Running;
        
    while (m_eStatus == Running)
    {
        Sleep(2000);    // Provide an event every two seconds
        
        
        // Generate a new event object.
        // ============================
        
        IWbemClassObject *pEvt = 0;

        HRESULT hRes = m_pEventClassDef->SpawnInstance(0, &pEvt);
        if (hRes != 0)
            continue;   // Failed
            

        // Generate some values to put in the event.
        // =========================================
                
        wchar_t Buf[128];
		StringCbPrintfW(Buf, sizeof(Buf), L"Test Event <%d>", nIteration);
 
        CVARIANT vName(Buf);
        hRes = pEvt->Put(CBSTR(L"Name"), 0, vName, 0);      
		if (FAILED(hRes))
        {
            // If here, delivery failed.  This could be due to an out of memory condition.
			// Do something to report it. 
		}

        CVARIANT vCount((LONG) nIteration);
        hRes = pEvt->Put(CBSTR(L"Value"), 0, vCount, 0);      
		if (FAILED(hRes))
        {
            // If here, delivery failed.  This could be due to an out of memory condition.
			// Do something to report it.
		}

        // Deliver the event to CIMOM.
        // ============================
        
        hRes = m_pSink->Indicate(1, &pEvt);
        
        if (FAILED(hRes))
        {
            // If here, delivery failed.  Do something to report it.
        }

        pEvt->Release();                    
        nIteration++;
    }

    // When we get to here, we are no longer interested in the
    // provider and Release() has long since returned.
    
    m_eStatus = Stopped;
    delete this;
}





//***************************************************************************
//
//***************************************************************************

    // Inherited from IWbemProviderInit
    // ================================

HRESULT CMyEventProvider::Initialize( 
            /* [in] */ LPWSTR pszUser,
            /* [in] */ LONG lFlags,
            /* [in] */ LPWSTR pszNamespace,
            /* [in] */ LPWSTR pszLocale,
            /* [in] */ IWbemServices __RPC_FAR *pNamespace,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemProviderInitSink __RPC_FAR *pInitSink
            )
{
    // We don't care about most of the incoming parameters in this
    // simple sample.  However, we will save the namespace pointer
    // and get our event class definition.
    // ===========================================================
    if(! pNamespace)
	{
	    pInitSink->SetStatus(WBEM_E_FAILED , 0);
	    return WBEM_NO_ERROR;
	}

	m_pNs = pNamespace;
	m_pNs->AddRef();
	   
   
    // Grab the class definition for the event.
    // ======================================
    
    IWbemClassObject *pObj = 0;

    HRESULT hRes = m_pNs->GetObject(
        CBSTR(EVENTCLASS),          
        0,                          
        pCtx,  
        &pObj,
        0
        );

    if (hRes != 0)
	{
	    pInitSink->SetStatus(WBEM_E_FAILED , 0);
	    return WBEM_NO_ERROR;
	}

    m_pEventClassDef = pObj;

    // Tell CIMOM that we're up and running.
    // =====================================

    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    
    return WBEM_NO_ERROR;
}            

// ================================
// Inherited from IWbemEventProviderSecurity. This sample does not analyze the query and
// applies the same policy to all queries.
// ================================

HRESULT CMyEventProvider::AccessCheck(
								WBEM_CWSTR wszQueryLanguage,
								WBEM_CWSTR wszQuery,
								long lSidLength,
								const BYTE* pSid)
{

	HRESULT hr = WBEM_S_NO_ERROR;

	//this is a temporary subscription, and therefore we have the thread token

	//Grant access for built-in admins, local system, local admins, local service and network service.
	PSECURITY_DESCRIPTOR secDescriptor = NULL;
	BOOL bRes = ConvertStringSecurityDescriptorToSecurityDescriptor //this function is only available on Windows 2000 and above
		( "O:BAG:BAD:(A;;0x10000001;;;BA)(A;;0x10000001;;;SY)(A;;0x10000001;;;LA)(A;;0x10000001;;;SY)(A;;0x10000001;;;S-1-5-20)(A;;0x10000001;;;S-1-5-19)",
		SDDL_REVISION_1,
		(PSECURITY_DESCRIPTOR *) &secDescriptor,
		NULL);

	if (! bRes)
	{
		return WBEM_E_ACCESS_DENIED;
	}

	if (pSid == NULL)
	{
       

		hr = CoImpersonateClient () ;
		if ( FAILED ( hr ) )
		{
			LocalFree(secDescriptor);
			return WBEM_E_ACCESS_DENIED ;
		}	

		// perform an access check. 
		
		hr = CheckAccess((SECURITY_DESCRIPTOR *)secDescriptor, 
							MASK_CLIENT_ACCESS_BIND,
							&s_ClientAccessMapping);

		LocalFree(secDescriptor);

		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		if (FAILED(hr))
			return WBEM_E_ACCESS_DENIED;
		else
			return WBEM_S_NO_ERROR;
	}
	else
	{
		//check against the SID passed to us. This op
	
		// Call GetSecurityDescriptorDacl() to obtain the DACL for secDescriptor.
		// Then, retrieve the access mask corresponding to permissions granted
		// by DACL to account denoted in pSid.  Please refer to Security SDK samples 
		// and documentation for more details.

		LocalFree(secDescriptor);

        if(0)	//pSID is not privileged enough to receive events from this proviuder
            return WBEM_E_ACCESS_DENIED;
		else
			return WBEM_S_NO_ERROR;

	}

	return WBEM_S_NO_ERROR;
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

HRESULT CMyEventProvider::CheckAccess (SECURITY_DESCRIPTOR *a_SecurityDescriptor ,
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

		if ( t_LastError ==E_ACCESSDENIED)
			return 	WBEM_E_ACCESS_DENIED ;
		else
			return WBEM_E_FAILED ;		
		
	}
	

	DWORD t_Access = 0 ;
	BOOL t_AccessStatus = FALSE ;
	PRIVILEGE_SET *t_PrivilegeSet = NULL ;
	DWORD t_PrivilegeSetSize = 0 ;

	MapGenericMask (

		& a_Access ,
		a_Mapping
	) ;

	t_Status = ::AccessCheck (

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
				t_Status = ::AccessCheck (
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


//***************************************************************************

//

//  propprov.cpp

//

//  Module: WMI Sample Property Provider

//

//  Purpose: Provider class code.  An object of this class is

//           created by the class factory for each connection.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************


#include <objbase.h>
#include "sample.h"


/******************************************************************************
 *	Name:	GetCurrentImpersonationLevel
 *  Description:
 *			Get COM impersonation level of caller.	
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



//***************************************************************************
//
// CPropPro::CPropPro
// CPropPro::~CPropPro
//
//***************************************************************************

CPropPro::CPropPro()
{
    m_cRef=0;
    InterlockedIncrement(&g_cObj);
    return;
}

CPropPro::~CPropPro(void)
{
    InterlockedDecrement(&g_cObj);
    return;
}

//***************************************************************************
//
//  CPropPro::QueryInterface
//
//  Returns a pointer to supported interfaces.
//
//***************************************************************************

STDMETHODIMP CPropPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;
    
    // This provider only support IUnknown and IWbemPropertyProvider.

    if (IID_IUnknown==riid || IID_IWbemPropertyProvider == riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
 }

//***************************************************************************
//
//  CPropPro::AddRef
//
//  Interface has another user, up the usage count.
//
//***************************************************************************

STDMETHODIMP_(ULONG) CPropPro::AddRef(void)
{
    return ++m_cRef;
}

//***************************************************************************
//
//  CPropPro::Release
//
//  Interface has been released.  Object will be deleted if the
//  usage count is zero.
//
//***************************************************************************

STDMETHODIMP_(ULONG) CPropPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}


//***************************************************************************
//
// CPropPro::PutProperty
// CPropPro::GetProperty
//
// Purpose: PutProperty writes out data and GetProperty returns data.
//
//***************************************************************************

STDMETHODIMP CPropPro::PutProperty(
		    long lFlags,
            const BSTR Locale,						   
            const BSTR ClassMapping,
            const BSTR InstMapping,
            const BSTR PropMapping,
            const VARIANT *pvValue)
{
    
    return WBEM_E_PROVIDER_NOT_CAPABLE;

}

STDMETHODIMP CPropPro::GetProperty(
		    long lFlags,
            const BSTR Locale,						   
			const BSTR ClassMapping,
            const BSTR InstMapping,
            const BSTR PropMapping,
            VARIANT *pvValue)
{

    SCODE sc = WBEM_S_NO_ERROR;
	
	//Impersonate the client
	sc = CoImpersonateClient () ;

	if ( FAILED ( sc ) )
	{
		return sc ;
	}	


	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;
		return WBEM_E_ACCESS_DENIED ;
	}

    // Depending on the InstMapping, return either a hard coded integer or
    // a string.  These mapping strings could be used in a more sophisticated
    // manner!  

	if (pvValue == NULL)
	{
		return WBEM_E_INVALID_PARAMETER;
	}

    if(!_wcsicmp(PropMapping, L"GiveMeANumber!"))
    {
        pvValue->vt = VT_I4;
        pvValue->lVal = 27;
    }
    else
    {
        pvValue->vt = VT_BSTR;
        pvValue->bstrVal = SysAllocString(L"Hello World");
        if(pvValue->bstrVal == NULL)
            sc = WBEM_E_OUT_OF_MEMORY;
    }
    return sc;
}

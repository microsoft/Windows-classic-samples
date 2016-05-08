//***************************************************************************

//

//  METHPROV.CPP

//

//  Module: WMI Method provider sample code

//

//  Purpose: Defines the CMethodPro class.  An object of this class is

//           created by the class factory for each connection.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#define _WIN32_DCOM

#include <objbase.h>
#include "methprov.h"
#include <process.h>
#include <wbemidl.h>
#include <stdio.h>


//***************************************************************************
//
// CMethodPro::CMethodPro
// CMethodPro::~CMethodPro
//
//***************************************************************************

CMethodPro::CMethodPro()
{
    InterlockedIncrement(&g_cObj);
    return;
   
}

CMethodPro::~CMethodPro(void)
{
    InterlockedDecrement(&g_cObj);
    return;
}

//***************************************************************************
//
// CMethodPro::QueryInterface
// CMethodPro::AddRef
// CMethodPro::Release
//
// Purpose: IUnknown members for CMethodPro object.
//***************************************************************************


STDMETHODIMP CMethodPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IWbemServices == riid || IID_IWbemProviderInit==riid)
       if(riid== IID_IWbemServices){
          *ppv=(IWbemServices*)this;
       }

       if(IID_IUnknown==riid || riid== IID_IWbemProviderInit){
          *ppv=(IWbemProviderInit*)this;
       }
    

    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CMethodPro::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CMethodPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}

/***********************************************************************
*                                                                      *
*CMethodPro::Initialize                                                *
*                                                                      *
*Purpose: This is the implementation of IWbemProviderInit. The method  *
* is need to initialize with CIMOM.                                    *
*                                                                      *
***********************************************************************/
STDMETHODIMP CMethodPro::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace, 
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{

  	if(! pNamespace)
	{
	    pInitSink->SetStatus(WBEM_E_FAILED , 0);
	}
	else
	{
		m_pWbemSvcs = pNamespace;
	    m_pWbemSvcs->AddRef();
		pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	}

    
    return WBEM_S_NO_ERROR;
}



/************************************************************************
*                                                                       *      
*CMethodPro::ExecMethodAsync                                            *
*                                                                       *
*Purpose: This is the Async function implementation.                    *
*         The only method supported in this sample is named Echo.  It   * 
*         takes an input string, copies it to the output and returns the* 
*         length.  The mof definition is                                *
*                                                                       *
*    [dynamic: ToInstance, provider("MethProv")]class MethProvSamp      *
*    {                                                                  *
*         [implemented, static]                                         *
*            uint32 Echo([IN]string sInArg="default",                   *
*                [out] string sOutArg);                                 *
*    };                                                                 *
*                                                                       *
************************************************************************/

STDMETHODIMP CMethodPro::ExecMethodAsync(const BSTR ObjectPath, const BSTR MethodName, 
            long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, 
            IWbemObjectSink* pResultSink)
{
	// Do some minimal error checking.  This code only support the
    // method "Echo" as defined in the mof.  A routine could support
    // more than on method

    if(_wcsicmp(MethodName, L"Echo"))
        return WBEM_E_INVALID_PARAMETER;

    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutClass = NULL;    
    IWbemClassObject* pOutParams = NULL;


	// Impersonate the client: in this simple sample, no protected resources are accessed,
	// but the provider normally needs to perform any operations in the security context 
	// of the client
	HRESULT hr = CoImpersonateClient () ;

	if ( FAILED ( hr ) )
	{
		pResultSink->SetStatus ( 0 , hr , NULL , NULL ) ;
		return hr ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hr = WBEM_E_ACCESS_DENIED;
		pResultSink->SetStatus ( 0 , hr , NULL , NULL ) ;
		return hr ;
	}
 

    // Allocate some BSTRs
    
    BSTR ClassName = SysAllocString(L"MethProvSamp");   
	if (!ClassName)
	{
		CoRevertToSelf();
		pResultSink->SetStatus ( 0 , WBEM_E_OUT_OF_MEMORY , NULL , NULL ) ;
		return hr ;
	}
	
    BSTR InputArgName = SysAllocString(L"sInArg");
	if (!InputArgName)
	{
		SysFreeString(ClassName);
		CoRevertToSelf();
		pResultSink->SetStatus ( 0 , WBEM_E_OUT_OF_MEMORY , NULL , NULL ) ;
		return hr ;
	}

    BSTR OutputArgName = SysAllocString(L"sOutArg");
	if (!OutputArgName)
	{
		SysFreeString(ClassName);
		SysFreeString(InputArgName);
		CoRevertToSelf();
		pResultSink->SetStatus ( 0 , WBEM_E_OUT_OF_MEMORY , NULL , NULL ) ;
		return hr ;
	}
    BSTR retValName = SysAllocString(L"ReturnValue");
	if (!retValName)
	{
		SysFreeString(ClassName);
		SysFreeString(InputArgName);
		SysFreeString(OutputArgName);
		CoRevertToSelf();
		pResultSink->SetStatus ( 0 , WBEM_E_OUT_OF_MEMORY , NULL , NULL ) ;
		return hr ;
	}

    // Get the class object, this is hard coded and matches the class
    // in the MOF.  A more sophisticated example would parse the 
    // ObjectPath to determine the class and possibly the instance.

    hr = m_pWbemSvcs->GetObject(ClassName, 0, pCtx, &pClass, NULL);
	if(hr != S_OK)
	{
		 pResultSink->SetStatus(0,hr, NULL, NULL);
		 return WBEM_S_NO_ERROR;
	}
 

    // This method returns values, and so create an instance of the
    // output argument class.

    hr = pClass->GetMethod(MethodName, 0, NULL, &pOutClass);
    pOutClass->SpawnInstance(0, &pOutParams);

    // Copy the input argument into the output object    
    
    VARIANT var;
    VariantInit(&var);    // Get the input argument
    pInParams->Get(InputArgName, 0, &var, NULL, NULL);   

    // put it into the output object

    pOutParams->Put(OutputArgName , 0, &var, 0);      
    long lLen = (long)wcslen(var.bstrVal);    VariantClear(&var);    var.vt = VT_I4;
    var.lVal = lLen;    // special name for return value.
    pOutParams->Put(retValName , 0, &var, 0); 

    // Send the output object back to the client via the sink. Then 
    // release the pointers and free the strings.

    hr = pResultSink->Indicate(1, &pOutParams);    
    pOutParams->Release();
    pOutClass->Release();    
    pClass->Release();    
    SysFreeString(ClassName);
    SysFreeString(InputArgName);    
    SysFreeString(OutputArgName);
    SysFreeString(retValName);     
    
    // all done now, set the status
    hr = pResultSink->SetStatus(0,WBEM_S_NO_ERROR,NULL,NULL);
    return WBEM_S_NO_ERROR;
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


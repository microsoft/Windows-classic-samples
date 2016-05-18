//***************************************************************************

//

//  CLASSPRO.CPP

//

//  Module: WINMGMT class provider sample code

//

//  Purpose: Defines the CClassPro class.  An object of this class is

//           created by the class factory for each connection.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#include <objbase.h>
#include "sample.h"
#include <process.h>
#include <strsafe.h>
#include <windows.h>


InstDef MyDefs[] = {{L"a", 1}, {L"b", 2}, {L"c", 3}};

long glNumInst = sizeof(MyDefs)/sizeof(InstDef);

#define SAMPLE_CLASS_NAME L"ClassProvSamp"

//***************************************************************************
//
// CClassPro::CClassPro
// CClassPro::~CClassPro
//
//***************************************************************************

CClassPro::CClassPro(BSTR ObjectPath, BSTR User, BSTR Password, IWbemContext * pCtx)
{
    m_pNamespace = NULL;
    m_cRef=0;
    InterlockedIncrement(&g_cObj);
    return;
}

CClassPro::~CClassPro(void)
{
    if(m_pNamespace)
        m_pNamespace->Release();
    InterlockedDecrement(&g_cObj);
    return;
}

//***************************************************************************
//
// CClassPro::QueryInterface
// CClassPro::AddRef
// CClassPro::Release
//
// Purpose: IUnknown members for CClassPro object.
//***************************************************************************


STDMETHODIMP CClassPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

    // Since we have dual inheritance, it is necessary to cast the return type

    if(riid== IID_IWbemServices)
       *ppv=(IWbemServices*)this;

    if(IID_IUnknown==riid || riid== IID_IWbemProviderInit)
       *ppv=(IWbemProviderInit*)this;
    

    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;
  
}


STDMETHODIMP_(ULONG) CClassPro::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CClassPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}

/***********************************************************************
*                                                                      *
*   CClassPro::Initialize                                                *
*                                                                      *
*   Purpose: This is the implementation of IWbemProviderInit. The method  *
*   is need to initialize with CIMOM.                                    *
*                                                                      *
***********************************************************************/

STDMETHODIMP CClassPro::Initialize(LPWSTR pszUser, LONG lFlags,
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
		m_pNamespace = pNamespace;
	    m_pNamespace->AddRef();
		pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	}

    
    return WBEM_S_NO_ERROR;
}

//***************************************************************************
//
// CClassPro::CreateClassEnumAsync
//
// Purpose: Asynchronously enumerates the classes this provider supports.  
// Note that this sample only supports one.  
//
//***************************************************************************

SCODE CClassPro::CreateClassEnumAsync(const BSTR Superclass, long lFlags, 
									  IWbemContext  *pCtx, IWbemObjectSink *pHandler)
{
    if(pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

	SCODE sc = S_OK;

	if(SysStringLen(Superclass) == 0)
	{
		IWbemClassObject * pNewClass = NULL;
		sc = CreateClass(m_pNamespace, &pNewClass, SAMPLE_CLASS_NAME, pCtx);
 
		if(sc == S_OK)
		{
			// Send the object to the caller

			pHandler->Indicate(1,&pNewClass);
			pNewClass->Release();
		}
	}
    // Set status

    pHandler->SetStatus(0,sc,NULL, NULL);
	return S_OK;
}

//***************************************************************************
//
// CClassPro::CreateInstanceEnumAsync
//
// Purpose: Asynchronously enumerates the instances.  
//
//***************************************************************************

SCODE CClassPro::CreateInstanceEnumAsync( const BSTR RefStr, long lFlags, IWbemContext *pCtx,
       IWbemObjectSink FAR* pHandler)
{
    SCODE sc;
    int iCnt;
    IWbemClassObject FAR* pNewInst;
  
    // Do a check of arguments and make sure we have pointer to Namespace

    if(pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

    for(iCnt=0; iCnt < glNumInst; iCnt++)
    {
        sc = CreateInst(m_pNamespace,MyDefs[iCnt].pwcKey,
                    MyDefs[iCnt].lValue, &pNewInst, RefStr, pCtx);
 
        if(sc != S_OK)
            break;

        // Send the object to the caller

        pHandler->Indicate(1,&pNewInst);
        pNewInst->Release();
    }

    // Set status

    pHandler->SetStatus(0,sc,NULL, NULL);

    return sc;
}


//***************************************************************************
//
// CClassPro::GetObjectByPathAsync
//
// Purpose: Returns either an instance or a class.
//
//***************************************************************************



SCODE CClassPro::GetObjectAsync(const BSTR ObjectPath, long lFlags,IWbemContext  *pCtx,
                    IWbemObjectSink FAR* pHandler)
{

    SCODE sc;
    IWbemClassObject FAR* pObj;
    BOOL bOK = FALSE;

    // Do a check of arguments and make sure we have pointer to Namespace

    if(ObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

	// If we are being asked for our class, get that
	size_t i1 = SysStringLen(ObjectPath);
	size_t i2 = wcslen(SAMPLE_CLASS_NAME);
	if(i1 >= i2 && !_wcsicmp(&ObjectPath[i1-i2], SAMPLE_CLASS_NAME) )
	{
		IWbemClassObject * pNewClass = NULL;
		sc = CreateClass(m_pNamespace, &pNewClass, SAMPLE_CLASS_NAME, pCtx);
 
		if(sc == S_OK)
		{
			// Send the object to the caller

			pHandler->Indicate(1,&pNewClass);
			pNewClass->Release();
		}
	}
	else if(S_OK ==GetByPath(ObjectPath,&pObj, pCtx))
    {
        pHandler->Indicate(1,&pObj);
        pObj->Release();
        bOK = TRUE;
		sc = S_OK;
    }
	else
		sc = WBEM_E_NOT_FOUND;

    // Set Status

    pHandler->SetStatus(0,sc, NULL, NULL);

    return sc;
}
 
//***************************************************************************
//
// CClassPro::GetByPath
//
// Purpose: Creates an instance given a particular Path value.
//
//***************************************************************************

SCODE CClassPro::GetByPath(BSTR ObjectPath, IWbemClassObject FAR* FAR* ppObj, IWbemContext  *pCtx)
{
    SCODE sc = S_OK;
    
    int iCnt;

    // do a simple path parse.  The path will look something like
    // ClassProvSamp.MyKey="a"
    // Create a test string with just the part between quotes.

    WCHAR wcTest[MAX_PATH+1];
	StringCbCopyW(wcTest, sizeof(wcTest), ObjectPath);
    WCHAR * pwcTest, * pwcCompare = NULL;

    int iNumQuotes = 0;
    for(pwcTest = wcTest; *pwcTest; pwcTest++)
        if(*pwcTest == L'\"')
        {
            iNumQuotes++;
            if(iNumQuotes == 1)
            {
                pwcCompare = pwcTest+1;
            }
            else if(iNumQuotes == 2)
            {
                *pwcTest = NULL;
                break;
            }
        }
        else if(*pwcTest == L'.')
            *pwcTest = NULL;    // issolate the class name.
    if(iNumQuotes != 2)
        return WBEM_E_FAILED;

    // check the instance list for a match.

    for(iCnt = 0; iCnt < glNumInst; iCnt++)
    {
        if(!_wcsicmp(MyDefs[iCnt].pwcKey, pwcCompare))
        {
            sc = CreateInst(m_pNamespace,MyDefs[iCnt].pwcKey,
                    MyDefs[iCnt].lValue, ppObj, wcTest, pCtx);
            return sc;
        }
    }

    return WBEM_E_NOT_FOUND;
}
 
SCODE CClassPro::PutClassAsync( 
            /* [in] */ IWbemClassObject __RPC_FAR *pObject,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemObjectSink __RPC_FAR *pResponseHandler)
{

	VARIANT var;
	VariantInit(&var);
	pObject->Get(L"__Class", 0, &var, NULL, NULL);
	if(!_wcsicmp(SAMPLE_CLASS_NAME, var.bstrVal))
		pResponseHandler->SetStatus(0,S_OK,NULL, NULL);
	else
		pResponseHandler->SetStatus(0,WBEM_E_NOT_FOUND,NULL, NULL);
	return S_OK;
}
 
SCODE CClassPro::DeleteClassAsync( 
            /* [in] */ const BSTR Class,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!_wcsicmp(SAMPLE_CLASS_NAME, Class))
		pResponseHandler->SetStatus(0,S_OK,NULL, NULL);
	else
		pResponseHandler->SetStatus(0,WBEM_E_NOT_FOUND,NULL, NULL);

	return S_OK;
}
SCODE CClassPro::PutInstanceAsync( 
            /* [in] */ IWbemClassObject __RPC_FAR *pInst,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    pResponseHandler->SetStatus(0,S_OK,NULL, NULL);
	return S_OK;
}
SCODE CClassPro::DeleteInstanceAsync( 
            /* [in] */ const BSTR ObjectPath,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    pResponseHandler->SetStatus(0,S_OK,NULL, NULL);
	return S_OK;
}


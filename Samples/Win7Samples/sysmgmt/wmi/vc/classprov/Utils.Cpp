//***************************************************************************

//

//  UTILS.CPP

//

//  Module: WINMGMT class provider sample code

//

//  Purpose: General purpose utilities.  

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#include <objbase.h>
#include "sample.h"


//***************************************************************************
//
// CreateInst
//
// Purpose: Creates a new instance and sets
//          the inital values of the properties.
//
// Return:   S_OK if all is well, otherwise an error code is returned
//
//***************************************************************************

SCODE CreateInst(IWbemServices * pNamespace, LPWSTR pKey, long lVal, 
                                        IWbemClassObject ** pNewInst,
                                        WCHAR * pwcClassName,
										IWbemContext  *pCtx)
{   
    SCODE sc;
    IWbemClassObject * pClass = NULL;
    sc = pNamespace->GetObject(pwcClassName, 0, pCtx, &pClass, NULL);
    if(sc != S_OK)
        return WBEM_E_FAILED;
    sc = pClass->SpawnInstance(0, pNewInst);
    pClass->Release();
    if(FAILED(sc))
        return sc;
    VARIANT v;

    // Set the key property value.

    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(pKey);
	if (v.bstrVal == NULL)
		return WBEM_E_OUT_OF_MEMORY;
    sc = (*pNewInst)->Put(L"MyKey", 0, &v, 0);
    VariantClear(&v);
	if(FAILED(sc))
        return sc;

    // Set the number property value.

    v.vt = VT_I4;
    v.lVal = lVal;
    sc = (*pNewInst)->Put(L"MyValue", 0, &v, 0);
    return sc;
}

SCODE CreateClass( IWbemServices * pNamespace, IWbemClassObject ** pNewClass,
                                             WCHAR * pwcClassName,
											 IWbemContext  *pCtx)
{

    SCODE sc;
    sc = pNamespace->GetObject(NULL, 0, pCtx, pNewClass, NULL);
    if(sc != S_OK)
        return sc;

    VARIANT v;

    // Set the class name.

    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(pwcClassName);
	if (!v.bstrVal)
		return WBEM_E_OUT_OF_MEMORY;
    sc = (*pNewClass)->Put(L"__class", 0, &v, 0);
    VariantClear(&v);


    // Set the key property value.

    v.vt = VT_NULL;
    sc = (*pNewClass)->Put(L"MyKey", 0, &v, CIM_STRING);

    // Set the number property value.

    sc = (*pNewClass)->Put(L"MyValue", 0, &v, CIM_SINT32);

	// Set the key qualifier

	IWbemQualifierSet* pQualSet = NULL;

	sc = (*pNewClass)->GetPropertyQualifierSet(L"MyKey", &pQualSet);
	if(pQualSet)
	{
	    v.vt = VT_BOOL;
		v.boolVal = VARIANT_TRUE;
		sc = pQualSet->Put(L"Key", &v, 0);
		pQualSet->Release();
	}

	// Set the dynamic qualifier

	sc = (*pNewClass)->GetQualifierSet(&pQualSet);
	if(pQualSet)
	{
	    v.vt = VT_BOOL;
		v.boolVal = VARIANT_TRUE;
		sc = pQualSet->Put(L"Dynamic", &v, 0);

	    v.vt = VT_BSTR;
		v.bstrVal = SysAllocString(L"ClassProvSamp");
		if (v.bstrVal)
			sc = pQualSet->Put(L"Provider", &v, 0);
		else
			sc = WBEM_E_OUT_OF_MEMORY;
		VariantClear(&v);

		pQualSet->Release();
	}

    return sc;

}

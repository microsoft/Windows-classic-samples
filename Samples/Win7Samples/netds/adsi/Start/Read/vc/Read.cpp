// Read.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	HRESULT hr;
	IADs *pUsr=NULL;


	CoInitialize(NULL);

	////////////////////////////////////
	// Bind to a directory object
	/////////////////////////////////////
	hr = ADsGetObject(L"WinNT://INDEPENDENCE/Administrator,user", IID_IADs, (void**) &pUsr );
    if ( !SUCCEEDED(hr) )
    {
        return 0;
    }


	///////////////////////////////////
	// Get a single value attribute
	////////////////////////////////////
	VARIANT var;
	VariantInit(&var);


	hr = pUsr->Get(L"FullName", &var );
	if ( SUCCEEDED(hr) )
	{
		printf("FullName: %S\n", V_BSTR(&var) );
		VariantClear(&var);
	}

	if ( pUsr )
	{
		pUsr->Release();
	}



	///////////////////////////////////////////////////////
	// Get a multi value attribute from a service object
	/////////////////////////////////////////////////////////
	IADs *pSvc = NULL;

	hr = ADsGetObject(L"WinNT://INDEPENDENCE/ANDYHAR11/Browser,service", IID_IADs, (void**) &pSvc );
	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}

	hr = pSvc->Get(L"Dependencies", &var );
	if ( SUCCEEDED(hr) )
	{
		LONG lstart, lend;
		SAFEARRAY  *sa = V_ARRAY( &var );
		VARIANT varItem;

		// Get the lower and upper bound
	    hr = SafeArrayGetLBound( sa, 1, &lstart );
                     hr = SafeArrayGetUBound( sa, 1, &lend );
		
		// Now iterate and print the content
		VariantInit(&varItem);
		printf("Getting service dependencies using IADs :\n");
		for ( long idx=lstart; idx < lend; idx++ )
		{
			hr = SafeArrayGetElement( sa, &idx, &varItem );
			printf("%S ", V_BSTR(&varItem));
			VariantClear(&varItem);
		}
		printf("\n");
		
		VariantClear(&var);
	}
	
	// Clean-up
	if ( pSvc )
	{
		pSvc->Release();
	}




	///////////////////////////////////////////////////////////
	// Using IDirectoryObject to get a multivalue attribute
	// Note: NOT all providers support this interface
	////////////////////////////////////////////////////////////
	IDirectoryObject	*pDirObject=NULL;
	ADS_ATTR_INFO		*pAttrInfo=NULL;
	DWORD				dwReturn;
	LPWSTR				pAttrNames[]={L"objectClass" };
	DWORD				dwNumAttr=sizeof(pAttrNames)/sizeof(LPWSTR);

	hr = ADsGetObject(L"LDAP://CN=Administrator,CN=Users,DC=testDom1,DC=testDom2,DC=microsoft,DC=com",
		              IID_IDirectoryObject, 
					  (void**) &pDirObject );

	if ( !SUCCEEDED(hr) )
	{
		return 0;
	}


	
	// Now get the attribute
	hr = pDirObject->GetObjectAttributes( pAttrNames, 
										  dwNumAttr, 
                                          &pAttrInfo, 
                                          &dwReturn );
	if ( SUCCEEDED(hr) )
	{
		printf("Getting the objectClass multivalue attribute using IDirectoryObject :\n");
		for (DWORD val=0; val < pAttrInfo->dwNumValues; val++, pAttrInfo->pADsValues++) 
        {
                printf("  %S\n", pAttrInfo->pADsValues->CaseIgnoreString);
        }
		FreeADsMem(pAttrInfo);
	}
	
	//Clean up
	pDirObject->Release();


	CoUninitialize();
	return 0;
}

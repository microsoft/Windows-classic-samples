// Filter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	HRESULT hr;
	IADsContainer *pCont=NULL;

	CoInitialize(NULL);
	///////////////////////////////////
	// Bind to the object
	/////////////////////////////////
	hr = ADsGetObject(L"WinNT://INDEPENDENCE", IID_IADsContainer, (void**) &pCont );
	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}

	///////////////////////////////////
	// Build variant filter
	/////////////////////////////////
	LPWSTR pszFilter[] = { L"user", L"group" };
	DWORD dwNumber = sizeof( pszFilter ) /sizeof(LPWSTR);
	VARIANT var;

	hr = ADsBuildVarArrayStr( pszFilter, dwNumber, &var );

	if ( !SUCCEEDED(hr) )
	{
		pCont->Release();
		return hr;
	}

	///////////////////////////////////
	// Set the filter
	/////////////////////////////////

	hr = pCont->put_Filter(var);
	VariantClear(&var);

	if (!SUCCEEDED(hr) )
	{
		pCont->Release();
		return hr;
	}

	////////////////////////////////////////////
	// Enumerate the result
	///////////////////////////////////////////////
	IEnumVARIANT *pEnum = NULL;
	hr = ADsBuildEnumerator( pCont, &pEnum );
	pCont->Release(); // no longer needed, since we have the enumerator already
	
	if ( SUCCEEDED(hr) )
	{
		VARIANT var;
		ULONG lFetch;
		IADs *pChild=NULL;
		VariantInit(&var);
		
		while( SUCCEEDED(ADsEnumerateNext( pEnum, 1, &var, &lFetch )) && lFetch == 1 )
		{
			hr = V_DISPATCH(&var)->QueryInterface( IID_IADs, (void**) &pChild );
			if ( SUCCEEDED(hr) )
			{
				BSTR bstrName;
				BSTR bstrClass;
				// Get more information on the child classes
				pChild->get_Name(&bstrName);
				pChild->get_Class(&bstrClass);
				
				printf("%S\t\t(%S)\n", bstrName, bstrClass );
				
				// Clean-up
				SysFreeString(bstrName);
				SysFreeString(bstrClass);
				
				pChild->Release();
			}
			VariantClear(&var);
		}
	}
              
              
              if ( pEnum )
	{
		ADsFreeEnumerator( pEnum );
	}

	CoUninitialize();

	return 0;
}

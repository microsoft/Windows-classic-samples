// Enum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"




int main(int argc, char* argv[])
{
	HRESULT hr;
	IADsContainer *pCont;
	CoInitialize(NULL);
	

	////////////////////////////////
	// Bind to a domain object
	//////////////////////////////////
	hr = ADsGetObject(L"WinNT://INDEPENDENCE", IID_IADsContainer, (void**) &pCont );
	if ( !SUCCEEDED(hr) )
	{
		return 0;
	}

	/////////////////////////////////
	// Enumerate
	/////////////////////////////////
	IEnumVARIANT *pEnum = NULL;
	hr = ADsBuildEnumerator( pCont, &pEnum );
	if ( SUCCEEDED(hr) )
	{
		VARIANT var;
		ULONG   lFetch;
		IADs   *pChild=NULL;
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

				printf("%S\t(%S)\n", bstrName, bstrClass );

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

	pCont->Release();

	CoUninitialize();
	return 0;
}

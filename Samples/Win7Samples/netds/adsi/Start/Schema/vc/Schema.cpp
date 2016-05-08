// Schema.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	IADsContainer *pSchema=NULL;
	HRESULT hr;

	CoInitialize(NULL);

	hr = ADsGetObject(L"WinNT://INDEPENDENCE/Schema", IID_IADsContainer, (void**) &pSchema );

	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}


	////////////// Enumerate Schema objects ///////////////////////////////////
	IEnumVARIANT *pEnum = NULL;
	hr = ADsBuildEnumerator( pSchema, &pEnum );
	pSchema->Release(); // no longer needed, since we have the enumerator already
	
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

              //Release the enumerator.
              if (pEnum != NULL) 
              {
                  ADsFreeEnumerator(pEnum);
              }


	CoUninitialize();



	return 0;
}

// RootDSE.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char* argv[])
{
	IADs    *pRoot=NULL;
	VARIANT varDSRoot;
	HRESULT hr;

	hr = CoInitialize(NULL);	


	/////////////////////////////////////////////////////////////////////////////
	// NOTE: If your client is NT 4.0 or Win 9.x without the DS Client, you 
	// must put the server name in front of the LDAP://.  For example
	// ADsGetObject(L"LDAP://myDC/RootDSE")
	// If your client is Windows 2000, you don't need to specify the servername.
	// The locator service in Windows 2000 will automatically locate the best DC for you
	/////////////////////////////////////////////////////////////////////////////////////

	// Get the Directory Object on the root DSE, to get to the server configuration
	hr = ADsGetObject(L"LDAP://RootDSE",IID_IADs,(void**)&pRoot);
	
	if(SUCCEEDED(hr) && pRoot != NULL)
	{

		hr = pRoot->Get(L"defaultNamingContext",&varDSRoot);
		if(SUCCEEDED(hr))
			printf("\nDefault Naming Context:%S\n",varDSRoot.bstrVal);
		VariantClear(&varDSRoot);

		
		hr = pRoot->Get(L"rootDomainNamingContext",&varDSRoot);
		if(SUCCEEDED(hr))
			printf("\nRoot Domain Naming Context:%S\n",varDSRoot.bstrVal);
		VariantClear(&varDSRoot);		

		hr = pRoot->Get(L"configurationNamingContext",&varDSRoot);
		if(SUCCEEDED(hr))
			printf("\nConfiguration Naming Context :%S\n",varDSRoot.bstrVal);
		VariantClear(&varDSRoot);
		

		hr = pRoot->Get(L"schemaNamingContext",&varDSRoot);
		if(SUCCEEDED(hr))
			printf("\nSchema Naming context :%S\n",varDSRoot.bstrVal);	    
		VariantClear(&varDSRoot);

	}

    	if (pRoot)	
       	pRoot->Release();
	
    	
	CoUninitialize();

	return 0;
}

// Child.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"

#define RETURN_ON_FAILURE(hr) if(!SUCCEEDED(hr)) return hr;


int main(int argc, char* argv[])
{
	HRESULT hr;

	CoInitialize(NULL);

	IADsContainer *pCont=NULL;

	hr = ADsGetObject(L"LDAP://DC=mydomain2,DC=mydomain1,DC=microsoft,DC=com",
		              IID_IADsContainer, 
					  (void**) &pCont );

	RETURN_ON_FAILURE(hr);
	

	/////////////////////////////////////////////////////////////
	// Get the child from the container 
	// Note in the LDAP provider you can go down more than one level
	///////////////////////////////////////////////////////////////
	IDispatch *pDisp = NULL;
	IADs	  *pADs  = NULL;
	hr = pCont->GetObject(L"user", L"CN=Mike Smith, OU=myou1", &pDisp );
	pCont->Release();

	RETURN_ON_FAILURE(hr);

	
	hr = pDisp->QueryInterface( IID_IADs, (void**) &pADs );
	pDisp->Release();		
	RETURN_ON_FAILURE(hr);

	// ... do something with pADs here .
	pADs->Release();


	CoUninitialize();
	return 0;
}

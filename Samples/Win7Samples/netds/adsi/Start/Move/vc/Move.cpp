// Move.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	CoInitialize(NULL);
	

	////////////////////////////////////////////
	// First bind to the destination container
	////////////////////////////////////////////
	HRESULT hr;
	IADsContainer *pCont=NULL;
    hr = ADsGetObject(L"LDAP://OU=trOU,DC=domain1,DC=domain2,DC=microsoft,DC=com",
		              IID_IADsContainer,
					  (void**) &pCont );

	if ( !SUCCEEDED(hr) )
	{
		return 0;
	}

	/////////////////////////////////////////////////
	// Now, move the object to the bound container
	///////////////////////////////////////////////////
	IDispatch *pDisp=NULL;

	hr = pCont->MoveHere(L"LDAP://CN=Mike Smith,OU=srOU,DC=domain1,DC=domain2,DC=microsoft,DC=com", NULL, &pDisp );
	pCont->Release();

	if (SUCCEEDED(hr) )
	{ 
		// You may do other operation here, such as updating attributes
		pDisp->Release();
	}
		                 

	CoUninitialize();
	return 0;
}

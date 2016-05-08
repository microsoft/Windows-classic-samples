// Binding.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	CoInitialize(NULL);

	
	HRESULT hr;
	IADs *pADs = NULL;
	/////////////////////////////////////////
	// Binding as currently logged on user
	////////////////////////////////////////
	hr = ADsGetObject( L"WinNT://INDEPENDENCE/JSmith,user", IID_IADs, (void**) &pADs );

	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}

	//... do some operations here
	pADs->Release(); // Do not forget to release when you're done.


	/////////////////////////////////////////
	// Binding with alternate credentials
	////////////////////////////////////////
	hr = ADsOpenObject(L"WinNT://INDEPENDENCE/JSmith,user", L"Administrator", L"secret", ADS_SECURE_AUTHENTICATION, IID_IADs, (void**) &pADs );
	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}

	//...
	// Make sure you release it after use
	pADs->Release();



	CoUninitialize();
	return 0;
}
